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


FctERR NONNULL__ OW_EEP_WriteCycle_Handler(OW_eep_t * const pEEP)
{
	FctERR err = ERROR_OK;

	if (!pEEP->doneWrite)
	{
		if (TPSSUP_MS(pEEP->hStartWrite, pEEP->props->write_cycle_time + 1U))	// Add 1ms to max write time
		{
			pEEP->doneWrite = true;

			OW_StrongPull_Set(pEEP->slave_inst->cfg.bus_inst, false);
			OW_set_busy(pEEP->slave_inst, false);
		}
		else
		{
			err = ERROR_BUSY;
		}
	}

	return err;
}


/****************************************************************/


/*!\brief OneWire EEPROM device copy scratchpad (to memory)
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \return FctERR - error code
**/
__STATIC FctERR NONNULL__ OW_EEP_Copy_Scratchpad(OW_eep_t * const pEEP)
{
	OW_slave_t * const		pSlave = pEEP->slave_inst;
	const OW_DRV * const	pDrv = pSlave->cfg.bus_inst;

	OW_set_busy(pSlave, true);

	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)
	{
		OW_set_busy(pSlave, false);
		goto ret;
	}

	const uint8_t mask_bits = pEEP->props->scratchpad_size - 1U;
	const uint8_t cmd[4] = { OW_EEP__COPY_SCRATCHPAD, LOBYTE(pEEP->scratch.address), HIBYTE(pEEP->scratch.address), pEEP->scratch.ES & mask_bits };
	UNUSED_RET OWWrite(pDrv, cmd, sizeof(cmd));

	pEEP->hStartWrite = HALTicks();
	pEEP->doneWrite = false;

	OW_StrongPull_Set(pEEP->slave_inst->cfg.bus_inst, true);

	// Do not release slave at this stage, copy to eeprom is ongoing

	ret:
	return err;
}


FctERR NONNULL__ OW_EEP_Read_Scratchpad(OW_eep_t * const pEEP)
{
	OW_slave_t * const		pSlave = pEEP->slave_inst;
	const OW_DRV * const	pDrv = pSlave->cfg.bus_inst;
	uWord					crc;
	uint8_t					tmp[3];
	FctERR					err = ERROR_OK;

	if (!OW_is_enabled(pSlave))		{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (OW_is_busy(pSlave))			{ err = ERROR_BUSY; }		// Device busy
	if (!pEEP->doneWrite)			{ err = ERROR_BUSY; }		// Copy in progess
	if (err != ERROR_OK)			{ goto ret; }

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pDrv, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)			{ goto err; }

	UNUSED_RET OWWrite_byte(pDrv, OW_EEP__READ_SCRATCHPAD);
	UNUSED_RET OWRead(pDrv, tmp, sizeof(tmp));

	const uint32_t	mask_bits = pEEP->props->scratchpad_size - 1U;
	const size_t	len = (tmp[2] & mask_bits) + 1U;

	UNUSED_RET OWRead(pDrv, pEEP->scratch.pData, len);
	UNUSED_RET OWRead(pDrv, crc.Byte, sizeof(crc.Byte));

	pEEP->scratch.ES = tmp[2];
	pEEP->scratch.nb = len;
	pEEP->scratch.address = MAKEWORD(tmp[0], tmp[1]);
	pEEP->scratch.crc = MAKEWORD(crc.Byte[0], crc.Byte[1]);

	crc.Word = 0U;
	OWCompute_DallasCRC16(&crc.Word, &((uint8_t) { OW_EEP__READ_SCRATCHPAD }), 1U);
	OWCompute_DallasCRC16(&crc.Word, tmp, sizeof(tmp));
	OWCompute_DallasCRC16(&crc.Word, pEEP->scratch.pData, len);
	crc.Word = ~crc.Word;

	if (crc.Word != pEEP->scratch.crc) { err = ERROR_CRC; }

	err:
	OW_set_busy(pSlave, false);

	ret:
	return err;
}


FctERR NONNULL__ OW_EEP_Write_Scratchpad(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const size_t len)
{
	OW_slave_t * const		pSlave = pEEP->slave_inst;
	const OW_DRV * const	pDrv = pSlave->cfg.bus_inst;
	FctERR					err = ERROR_OK;

	//if (!OW_is_enabled(pSlave))			{ err = ERROR_DISABLED; }	// Peripheral disabled
	//if (OW_is_busy(pSlave))				{ err = ERROR_BUSY; }		// Device busy
	if (len > pEEP->props->scratchpad_size)	{ err = ERROR_OVERFLOW; }	// Scratchpad overflow
	if (err != ERROR_OK)					{ goto ret; }

	// TODO: is this really needed? (for now, scratchpad is always read back after writing), still needs to be set if no read back is done (for copy)
	pEEP->scratch.address = addr;
	pEEP->scratch.nb = len;

	// Write scratchpad
	const uint8_t cmd[3] = { OW_EEP__WRITE_SCRATCHPAD, LOBYTE(addr), HIBYTE(addr) };
	UNUSED_RET memcpy(pEEP->scratch.pData, pData, len);

	uint16_t CRC_data = 0;
	OWCompute_DallasCRC16(&CRC_data, cmd, sizeof(cmd));
	OWCompute_DallasCRC16(&CRC_data, pData, len);
	CRC_data = ~CRC_data;

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pDrv, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)
	{
		OW_set_busy(pSlave, false);
		goto ret;
	}

	UNUSED_RET OWWrite(pDrv, cmd, sizeof(cmd));
	UNUSED_RET OWWrite(pDrv, pEEP->scratch.pData, len);

	// Get returned CRC
	uint8_t CRC_received[2] = { 0 };
	UNUSED_RET OWRead(pDrv, CRC_received, sizeof(CRC_received));

	OW_set_busy(pSlave, false);

	if (CRC_data != MAKEWORD(CRC_received[0], CRC_received[1])) { err = ERROR_CRC; }
	else
	{
		err = OW_EEP_Read_Scratchpad(pEEP);

		// TODO: check data & iCRC ?
	}

	ret:
	return err;
}


FctERR NONNULL__ OW_EEP_Read_Memory(OW_eep_t * const pEEP, uint8_t * pData, const uint32_t addr, const size_t len)
{
	OW_slave_t * const		pSlave = pEEP->slave_inst;
	const OW_DRV * const	pDrv = pSlave->cfg.bus_inst;
	FctERR					err = ERROR_OK;

	if (!OW_is_enabled(pSlave))									{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (OW_is_busy(pSlave))										{ err = ERROR_BUSY; }		// Device busy
	if (addr > pEEP->props->max_read_address)					{ err = ERROR_RANGE; }		// Unknown address
	if ((addr + len) > (pEEP->props->max_read_address + 1U))	{ err = ERROR_OVERFLOW; }	// Bank overflow
	if (err != ERROR_OK)										{ goto ret; }

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pDrv, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)	{ goto err; }

	const uint8_t cmd[3] = { OW_EEP__READ_MEMORY, LOBYTE(addr), HIBYTE(addr) };
	UNUSED_RET OWWrite(pDrv, cmd, sizeof(cmd));
	UNUSED_RET OWRead(pDrv, pData, len);

	err:
	OW_set_busy(pSlave, false);

	ret:
	return err;
}


FctERR NONNULL__ OW_EEP_Write_Memory(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const size_t len)
{
	const OW_slave_t * const	pSlave = pEEP->slave_inst;
	FctERR						err = ERROR_OK;

	if (!OW_is_enabled(pSlave))									{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (OW_is_busy(pSlave))										{ err = ERROR_BUSY; }		// Device busy
	if (addr > pEEP->props->max_write_address)					{ err = ERROR_RANGE; }		// Unknown address
	if ((addr + len) > (pEEP->props->max_write_address + 1U))	{ err = ERROR_OVERFLOW; }	// Bank overflow
	if (!pEEP->doneWrite)										{ err = ERROR_BUSY; }		// Copy in progess
	if (err != ERROR_OK)										{ goto ret; }

	size_t		data_len = len;
	size_t		unaligned_len = addr % pEEP->props->scratchpad_size;
	uint32_t	address = addr - unaligned_len;	// If unaligned write access, adjust address to read aligned bytes first

	while (data_len != 0U)
	{
		while (OW_EEP_WriteCycle_Handler(pEEP) != ERROR_OK)	// Wait for a previous write to complete
		{
			OW_Watchdog_Refresh();
			HAL_Delay(1U);
		}

		const size_t write_len = min(pEEP->props->scratchpad_size - unaligned_len, data_len);

		if (unaligned_len != 0U)
		{
			const size_t read_len = ((write_len + unaligned_len) == pEEP->props->scratchpad_size) ? unaligned_len : pEEP->props->scratchpad_size;

			err = OW_EEP_Read_Memory(pEEP, pEEP->scratch.pData, address, read_len);
			UNUSED_RET memcpy(&pEEP->scratch.pData[unaligned_len], pData, write_len);
			unaligned_len = 0U;	// Reset unaligned_len, further writes will be aligned
		}
		else if (write_len < pEEP->props->scratchpad_size)
		{
			const size_t read_len = pEEP->props->scratchpad_size - write_len;

			err = OW_EEP_Read_Memory(pEEP, &pEEP->scratch.pData[write_len], address + write_len, read_len);
			UNUSED_RET memcpy(pEEP->scratch.pData, pData, write_len);
		}
		else
		{
			UNUSED_RET memcpy(pEEP->scratch.pData, pData, pEEP->props->scratchpad_size);
		}

		if (err != ERROR_OK)	{ goto ret; }

		// Write scratchpad & Check CRC16 or Read scratchpad to see if matching with written values
		err = OW_EEP_Write_Scratchpad(pEEP, pEEP->scratch.pData, address, pEEP->props->scratchpad_size);
		if (err != ERROR_OK)	{ goto ret; }

		// Copy scratchpad
		err = OW_EEP_Copy_Scratchpad(pEEP);
		if (err != ERROR_OK)	{ goto ret; }

		pData += write_len;
		data_len -= write_len;
		address += pEEP->props->scratchpad_size;
	}

	ret:
	return err;
}

