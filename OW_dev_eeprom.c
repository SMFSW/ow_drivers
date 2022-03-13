/*!\file OW_dev_eeprom.c
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire eeprom device type
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_drv.h"

#include "OW_dev_eeprom.h"
/****************************************************************/


/*!\brief OneWire EEPROM device copy scratchpad (to memory)
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \param[in] pScratch - Pointer to scratchpad data
** \return FctERR - error code
**/
static FctERR NONNULL__ OW_EEP_Copy_Scratchpad(OW_eep_t * const pEEP, OW_EEP_scratch_t * const pScratch)
{
	OW_slave_t * const	pSlave = pEEP->slave_inst;

	OW_set_busy(pSlave, true);
	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, OW_EEP_Copy_Scratchpad);

	if (!err)
	{
		const uint8_t mask_bits = pEEP->props.scratchpad_size - 1;
		const uint8_t cmd[4] = { OW_EEP__COPY_SCRATCHPAD, LOBYTE(pScratch->address), HIBYTE(pScratch->address), pScratch->ES & mask_bits };
		OWWrite(pSlave->cfg.bus_inst, cmd, sizeof(cmd));

		// TODO: still need to figure out if waiting would be best (maybe just read at least once for 0xFF)
		// FIXME: in any case, reading once for 0xFF may be needed to ensure copy command is not rejected
		if (pEEP->wait_prog)
		{
			uint8_t done = 0;
			do
			{
				#if defined(HAL_IWDG_MODULE_ENABLED)
					HAL_IWDG_Refresh(&hiwdg);
				#endif
				HAL_Delay(1);
				OWRead(pSlave->cfg.bus_inst, &done, 1);
				if (done == 0xFF)	{ err = ERROR_FAILED; }
			}
			while ((done != 0xAA) && (!err));
		}
	}
	OW_set_busy(pSlave, false);

	return ERROR_OK;
}


FctERR NONNULL__ OW_EEP_Read_Scratchpad(OW_eep_t * const pEEP, OW_EEP_scratch_t * const pScratch)
{
	OW_slave_t * const	pSlave = pEEP->slave_inst;

	if (!OW_is_enabled(pSlave))		{ return ERROR_DISABLED; }	// Peripheral disabled

	OW_set_busy(pSlave, true);
	FctERR	err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, OW_EEP_Read_Scratchpad);
	if (err)	{ return err; }

	uint8_t tmp[3];
	uint8_t crc[2];

	OWWrite_byte(pSlave->cfg.bus_inst, OW_EEP__READ_SCRATCHPAD);
	OWRead(pSlave->cfg.bus_inst, tmp, sizeof(tmp));

	const uint8_t mask_bits = pEEP->props.scratchpad_size - 1;
	const uint8_t nb = (tmp[2] & mask_bits) + 1;
	OWRead(pSlave->cfg.bus_inst, pScratch->pData, nb);
	OWRead(pSlave->cfg.bus_inst, crc, sizeof(crc));
	OW_set_busy(pSlave, false);

	pScratch->ES = tmp[2];
	pScratch->nb = nb;
	pScratch->address = MAKEWORD(tmp[0], tmp[1]);
	pScratch->iCRC16 = MAKEWORD(crc[0], crc[1]);

	uint16_t iCRC16 = 0;
	OWCompute_DallasCRC16(&iCRC16, &((uint8_t) { OW_EEP__READ_SCRATCHPAD }), 1);
	OWCompute_DallasCRC16(&iCRC16, tmp, sizeof(tmp));
	OWCompute_DallasCRC16(&iCRC16, pScratch->pData, nb);
	iCRC16 = ~iCRC16;

	return (iCRC16 == pScratch->iCRC16) ? ERROR_OK : ERROR_CRC;;
}


FctERR NONNULL__ OW_EEP_Write_Scratchpad(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const uint32_t nb)
{
	OW_slave_t * const	pSlave = pEEP->slave_inst;

	//if (!OW_is_enabled(pSlave))		{ return ERROR_DISABLED; }	// Peripheral disabled

	// TODO: is this really needed? (for now, scratchpad is always read back after writing), still needs to be set if no read back is done (for copy)
	pEEP->pScratch->address = addr;
	pEEP->pScratch->nb = nb;

	OW_set_busy(pSlave, true);
	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, OW_EEP_Write_Scratchpad);

	if (!err)
	{
		// Write scratchpad
		const uint8_t cmd[3] = { OW_EEP__WRITE_SCRATCHPAD, LOBYTE(addr), HIBYTE(addr) };
		memcpy(pEEP->pScratch->pData, pData, nb);

		uint16_t iCRC16_data = 0;
		OWCompute_DallasCRC16(&iCRC16_data, cmd, sizeof(cmd));
		OWCompute_DallasCRC16(&iCRC16_data, pData, nb);
		iCRC16_data = ~iCRC16_data;

		OWWrite(pSlave->cfg.bus_inst, cmd, sizeof(cmd));
		OWWrite(pSlave->cfg.bus_inst, pEEP->pScratch->pData, nb);

		// Get 2 Bytes to verify CRC16
		uWord iCRC16_received = { 0 };
		OWRead(pSlave->cfg.bus_inst, iCRC16_received.Byte, sizeof(iCRC16_received.Byte));
		iCRC16_received.Word = MAKEWORD(iCRC16_received.Byte[0], iCRC16_received.Byte[1]);
		err = (iCRC16_data == iCRC16_received.Word) ? ERROR_OK : ERROR_CRC;

		if (!err)
		{
			err = OW_EEP_Read_Scratchpad(pEEP, pEEP->pScratch);

			// TODO: check data & iCRC16 ?

			// If further operations done on bus, set busy again
			//OW_set_busy(pSlave, true);
		}
	}

	OW_set_busy(pSlave, false);

	return err;
}


FctERR NONNULL__ OW_EEP_Read_Memory(OW_eep_t * const pEEP, uint8_t * pData, const uint32_t addr, const uint32_t nb)
{
	OW_slave_t * const	pSlave = pEEP->slave_inst;

	if (!OW_is_enabled(pSlave))							{ return ERROR_DISABLED; }	// Peripheral disabled
	if (addr > pEEP->props.max_read_address)			{ return ERROR_RANGE; }		// Unknown address
	if ((addr + nb) > pEEP->props.max_read_address + 1)	{ return ERROR_OVERFLOW; }	// Bank overflow

	OW_set_busy(pSlave, true);
	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, OW_EEP_Read_Memory);

	if (!err)
	{
		const uint8_t cmd[3] = { OW_EEP__READ_MEMORY, LOBYTE(addr), HIBYTE(addr) };
		OWWrite(pSlave->cfg.bus_inst, cmd, sizeof(cmd));
		OWRead(pSlave->cfg.bus_inst, pData, nb);
	}
	OW_set_busy(pSlave, false);

	return err;
}


FctERR NONNULL__ OW_EEP_Write_Memory(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const uint32_t nb)
{
	OW_slave_t * const	pSlave = pEEP->slave_inst;
	FctERR				err = ERROR_OK;

	if (!OW_is_enabled(pSlave))								{ return ERROR_DISABLED; }	// Peripheral disabled
	if (addr > pEEP->props.max_write_address)				{ return ERROR_RANGE; }		// Unknown address
	if ((addr + nb) > pEEP->props.max_write_address + 1)	{ return ERROR_OVERFLOW; }	// Bank overflow

	uint8_t misaligned = addr % pEEP->props.scratchpad_size;
	uint8_t nb_data = nb;
	uint16_t address = addr - misaligned;

	while (nb_data)
	{
		const uint8_t current_nb = min(pEEP->props.scratchpad_size - misaligned, nb_data);

		// If unaligned write access, read aligned bytes first
		if (misaligned)
		{
			err = OW_EEP_Read_Memory(pEEP, pEEP->pScratch->pData, address, pEEP->props.scratchpad_size);
			memcpy(&pEEP->pScratch->pData[misaligned], pData, current_nb);
			misaligned = 0;		// Erase misaligned to skip this step for next writes
		}
		else if (current_nb < pEEP->props.scratchpad_size)
		{
			const uint8_t remainder_nb = pEEP->props.scratchpad_size - current_nb;

			err = OW_EEP_Read_Memory(pEEP, &pEEP->pScratch->pData[current_nb], address + current_nb, remainder_nb);
			memcpy(pEEP->pScratch->pData, pData, current_nb);
		}
		else
		{
			memcpy(pEEP->pScratch->pData, pData, pEEP->props.scratchpad_size);
		}

		if (!err)
		{
			// Write scratchpad & Check CRC16 or Read scratchpad to see if matching with written values
			err = OW_EEP_Write_Scratchpad(pEEP, pEEP->pScratch->pData, address, pEEP->props.scratchpad_size);

			// Copy scratchpad
			if (!err)	{ err = OW_EEP_Copy_Scratchpad(pEEP, pEEP->pScratch); }
		}

		pData += current_nb;
		nb_data -= current_nb;
		address += current_nb;
	}

	return err;
}


