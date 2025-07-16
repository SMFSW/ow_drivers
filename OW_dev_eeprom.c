/*!\file OW_dev_eeprom.c
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief OneWire eeprom device type
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_drv.h"

#include "OW_dev_eeprom.h"
/****************************************************************/


/*!\brief OneWire EEPROM device copy scratchpad (to memory)
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \return FctERR - error code
**/
__STATIC FctERR NONNULL__ OW_EEP_Copy_Scratchpad(OW_eep_t * const pEEP)
{
	OW_slave_t * const	pSlave = pEEP->slave_inst;

	OW_set_busy(pSlave, true);

	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)			{ goto ret; }

	const uint8_t mask_bits = pEEP->props.scratchpad_size - 1U;
	const uint8_t cmd[4] = { OW_EEP__COPY_SCRATCHPAD, LOBYTE(pEEP->pScratch->address), HIBYTE(pEEP->pScratch->address), pEEP->pScratch->ES & mask_bits };
	OWWrite(pSlave->cfg.bus_inst, cmd, sizeof(cmd));

	if (pEEP->wait_prog)
	{
		uint8_t done = 0U;
		do
		{
			#if defined(HAL_IWDG_MODULE_ENABLED)
				UNUSED_RET HAL_IWDG_Refresh(&hiwdg);
			#endif
			HAL_Delay(2U);
			OWRead(pSlave->cfg.bus_inst, &done, 1U);
			if (done == 0xFFU)	{ err = ERROR_FAILED; }
		}
		while ((done != 0xAAU) && (!err));
	}

	ret:
	OW_set_busy(pSlave, false);

	return err;
}


FctERR NONNULL__ OW_EEP_Read_Scratchpad(OW_eep_t * const pEEP)
{
	OW_slave_t * const	pSlave = pEEP->slave_inst;
	uint16_t			iCRC16 = 0U;
	uint8_t				crc[2];
	uint8_t				tmp[3];
	FctERR				err = ERROR_OK;

	if (!OW_is_enabled(pSlave))		{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (err != ERROR_OK)			{ goto ret; }

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)			{ goto ret; }

	OWWrite_byte(pSlave->cfg.bus_inst, OW_EEP__READ_SCRATCHPAD);
	OWRead(pSlave->cfg.bus_inst, tmp, sizeof(tmp));

	const uint32_t	mask_bits = pEEP->props.scratchpad_size - 1U;
	const size_t	len = (tmp[2] & mask_bits) + 1U;
	OWRead(pSlave->cfg.bus_inst, pEEP->pScratch->pData, len);
	OWRead(pSlave->cfg.bus_inst, crc, sizeof(crc));

	pEEP->pScratch->ES = tmp[2];
	pEEP->pScratch->nb = len;
	pEEP->pScratch->address = MAKEWORD(tmp[0], tmp[1]);
	pEEP->pScratch->iCRC16 = MAKEWORD(crc[0], crc[1]);

	OWCompute_DallasCRC16(&iCRC16, &((uint8_t) { OW_EEP__READ_SCRATCHPAD }), 1U);
	OWCompute_DallasCRC16(&iCRC16, tmp, sizeof(tmp));
	OWCompute_DallasCRC16(&iCRC16, pEEP->pScratch->pData, len);
	iCRC16 = ~iCRC16;

	if (iCRC16 != pEEP->pScratch->iCRC16) { err = ERROR_CRC; }

	ret:
	OW_set_busy(pSlave, false);

	return err;
}


FctERR NONNULL__ OW_EEP_Write_Scratchpad(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const size_t len)
{
	OW_slave_t * const	pSlave = pEEP->slave_inst;
	FctERR				err = ERROR_OK;

	//if (!OW_is_enabled(pSlave))			{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (len > pEEP->props.scratchpad_size)	{ err = ERROR_OVERFLOW; }	// Scratchpad overflow
	if (err != ERROR_OK)					{ goto ret; }

	// TODO: is this really needed? (for now, scratchpad is always read back after writing), still needs to be set if no read back is done (for copy)
	pEEP->pScratch->address = addr;
	pEEP->pScratch->nb = len;

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)	{ goto ret; }

	// Write scratchpad
	const uint8_t cmd[3] = { OW_EEP__WRITE_SCRATCHPAD, LOBYTE(addr), HIBYTE(addr) };
	UNUSED_RET memcpy(pEEP->pScratch->pData, pData, len);

	uint16_t iCRC16_data = 0U;
	OWCompute_DallasCRC16(&iCRC16_data, cmd, sizeof(cmd));
	OWCompute_DallasCRC16(&iCRC16_data, pData, len);
	iCRC16_data = ~iCRC16_data;

	OWWrite(pSlave->cfg.bus_inst, cmd, sizeof(cmd));
	OWWrite(pSlave->cfg.bus_inst, pEEP->pScratch->pData, len);

	// Get 2 Bytes to verify CRC16
	uWord iCRC16_received = { 0 };
	OWRead(pSlave->cfg.bus_inst, iCRC16_received.Byte, sizeof(iCRC16_received.Byte));
	iCRC16_received.Word = MAKEWORD(iCRC16_received.Byte[0], iCRC16_received.Byte[1]);

	if (iCRC16_data != iCRC16_received.Word) { err = ERROR_CRC; }
	else
	{
		err = OW_EEP_Read_Scratchpad(pEEP);

		// TODO: check data & iCRC16 ?

		// If further operations done on bus, set busy again
		//OW_set_busy(pSlave, true);
	}

	ret:
	OW_set_busy(pSlave, false);

	return err;
}


FctERR NONNULL__ OW_EEP_Read_Memory(OW_eep_t * const pEEP, uint8_t * pData, const uint32_t addr, const size_t len)
{
	OW_slave_t * const	pSlave = pEEP->slave_inst;
	FctERR				err = ERROR_OK;

	if (!OW_is_enabled(pSlave))								{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (addr > pEEP->props.max_read_address)				{ err = ERROR_RANGE; }		// Unknown address
	if ((addr + len) > (pEEP->props.max_read_address + 1U))	{ err = ERROR_OVERFLOW; }	// Bank overflow
	if (err != ERROR_OK)									{ goto ret; }

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)	{ goto ret; }

	const uint8_t cmd[3] = { OW_EEP__READ_MEMORY, LOBYTE(addr), HIBYTE(addr) };
	OWWrite(pSlave->cfg.bus_inst, cmd, sizeof(cmd));
	OWRead(pSlave->cfg.bus_inst, pData, len);

	ret:
	OW_set_busy(pSlave, false);

	return err;
}


FctERR NONNULL__ OW_EEP_Write_Memory(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const size_t len)
{
	const OW_slave_t * const	pSlave = pEEP->slave_inst;
	FctERR						err = ERROR_OK;

	if (!OW_is_enabled(pSlave))									{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (addr > pEEP->props.max_write_address)					{ err = ERROR_RANGE; }		// Unknown address
	if ((addr + len) > (pEEP->props.max_write_address + 1U))	{ err = ERROR_OVERFLOW; }	// Bank overflow
	if (err != ERROR_OK)										{ goto ret; }

	size_t		data_len = len;
	size_t		unaligned_len = addr % pEEP->props.scratchpad_size;
	uint32_t	address = addr - unaligned_len;	// If unaligned write access, adjust address to read aligned bytes first

	while (data_len != 0U)
	{
		const size_t write_len = min(pEEP->props.scratchpad_size - unaligned_len, data_len);

		if (unaligned_len != 0U)
		{
			const size_t read_len = ((write_len + unaligned_len) == pEEP->props.scratchpad_size) ? unaligned_len : pEEP->props.scratchpad_size;

			err = OW_EEP_Read_Memory(pEEP, pEEP->pScratch->pData, address, read_len);
			UNUSED_RET memcpy(&pEEP->pScratch->pData[unaligned_len], pData, write_len);
			unaligned_len = 0U;	// Reset unaligned_len, further writes will be aligned
		}
		else if (write_len < pEEP->props.scratchpad_size)
		{
			const size_t read_len = pEEP->props.scratchpad_size - write_len;

			err = OW_EEP_Read_Memory(pEEP, &pEEP->pScratch->pData[write_len], address + write_len, read_len);
			UNUSED_RET memcpy(pEEP->pScratch->pData, pData, write_len);
		}
		else
		{
			UNUSED_RET memcpy(pEEP->pScratch->pData, pData, pEEP->props.scratchpad_size);
		}

		if (err != ERROR_OK)	{ goto ret; }

		// Write scratchpad & Check CRC16 or Read scratchpad to see if matching with written values
		err = OW_EEP_Write_Scratchpad(pEEP, pEEP->pScratch->pData, address, pEEP->props.scratchpad_size);
		if (err != ERROR_OK)	{ goto ret; }

		// Copy scratchpad
		err = OW_EEP_Copy_Scratchpad(pEEP);
		if (err != ERROR_OK)	{ goto ret; }

		pData += write_len;
		data_len -= write_len;
		address += pEEP->props.scratchpad_size;
	}

	ret:
	return err;
}


