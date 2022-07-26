/*!\file OW_dev_temp.c
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire temperature sensor device type
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_drv.h"

#include "OW_dev_temp.h"
/****************************************************************/


FctERR NONNULL__ OWAlarmSearch_All(OW_DRV * const pOW, OW_ROM_ID_t ROMId[], const uint8_t max_nb)
{
	OWSearch_SetType(pOW, OW_TEMP__ALARM_SEARCH);
	FctERR err = OWSearch_All(pOW, ROMId, max_nb);
	OWSearch_SetType(pOW, OW__SEARCH_ROM);	// Switch back to default search command

	return err;
}


/****************************************************************/


__STATIC_INLINE FctERR NONNULL_INLINE__ OW_TEMP_Check_CRC_Scratchpad(OW_temp_t * const pTEMP) {
	return OWCheck_DallasCRC8(pTEMP->scratch_data, OW_TEMP_SCRATCHPAD_SIZE - 1, pTEMP->scratch_data[OW_TEMP_SCRATCHPAD_SIZE - 1]); }


FctERR NONNULL__ OW_TEMP_Read_Scratchpad(OW_temp_t * const pTEMP)
{
	OW_slave_t * const	pSlave = pTEMP->slave_inst;

	if (!OW_is_enabled(pSlave))		{ return ERROR_DISABLED; }	// Peripheral disabled

	OW_set_busy(pSlave, true);
	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);

	if (!err)
	{
		OWWrite_byte(pSlave->cfg.bus_inst, OW_TEMP__READ_SCRATCHPAD);
		OWRead(pSlave->cfg.bus_inst, pTEMP->scratch_data, OW_TEMP_SCRATCHPAD_SIZE);
	}

	OW_set_busy(pSlave, false);

	if (!err)
	{
		err = OW_TEMP_Check_CRC_Scratchpad(pTEMP);
	}

	return err;
}


static FctERR NONNULL__ OW_TEMP_Recall(OW_temp_t * const pTEMP)
{
	OW_slave_t * const	pSlave = pTEMP->slave_inst;

	//if (!OW_is_enabled(pSlave))		{ return ERROR_DISABLED; }	// Peripheral disabled

	OW_set_busy(pSlave, true);
	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);

	if (!err)
	{
		OWWrite_byte(pSlave->cfg.bus_inst, OW_TEMP__RECALL);

		uint8_t done = 0;
		while (!done)
		{
			#if defined(HAL_IWDG_MODULE_ENABLED)
				HAL_IWDG_Refresh(&hiwdg);
			#endif
			OWRead_byte(pSlave->cfg.bus_inst, &done);
		}
	}

	return err;
}


static FctERR NONNULL__ OW_TEMP_Copy_Scratchpad(OW_temp_t * const pTEMP)
{
	OW_slave_t * const	pSlave = pTEMP->slave_inst;

	//if (!OW_is_enabled(pSlave))		{ return ERROR_DISABLED; }	// Peripheral disabled

	OW_set_busy(pSlave, true);
	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);

	if (!err)
	{
		OWWrite_byte(pSlave->cfg.bus_inst, OW_TEMP__COPY_SCRATCHPAD);

		const uint32_t hStartCopy = HALTicks();
		while (TPSINF_MS(hStartCopy, 10))		// Wait for bytes to be copied in EEP
		{
			#if defined(HAL_IWDG_MODULE_ENABLED)
				HAL_IWDG_Refresh(&hiwdg);
			#endif
		}
	}

	return err;
}


FctERR NONNULL__ OW_TEMP_Write_Scratchpad(OW_temp_t * const pTEMP)
{
	OW_slave_t * const	pSlave = pTEMP->slave_inst;

	if (!OW_is_enabled(pSlave))		{ return ERROR_DISABLED; }	// Peripheral disabled

	OW_set_busy(pSlave, true);
	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);

	if (!err)
	{
		OWWrite_byte(pSlave->cfg.bus_inst, OW_TEMP__WRITE_SCRATCHPAD);
		OWWrite(pSlave->cfg.bus_inst, &pTEMP->scratch_data[2], pTEMP->props.cfgBytes);

		err = OW_TEMP_Copy_Scratchpad(pTEMP);
		if (!err)	{ err = OW_TEMP_Recall(pTEMP); }
		if (!err)	{ err = OW_TEMP_Read_Scratchpad(pTEMP); }
	}

	OW_set_busy(pSlave, false);

	return err;
}


FctERR NONNULL__ OW_TEMP_Start_Conversion(OW_temp_t * const pTEMP)
{
	OW_slave_t * const	pSlave = pTEMP->slave_inst;

	if (!OW_is_enabled(pSlave))		{ return ERROR_DISABLED; }	// Peripheral disabled

	OW_set_busy(pSlave, true);
	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);

	if (!err)
	{
		OWWrite_byte(pSlave->cfg.bus_inst, OW_TEMP__CONVERT_T);
		pTEMP->hStartConv = HALTicks();
		pTEMP->doneConv = false;
	}

	OW_set_busy(pSlave, false);

	return err;
}


FctERR NONNULL__ OW_TEMP_Read_Conversion(OW_temp_t * const pTEMP)
{
	FctERR err = OW_TEMP_Read_Scratchpad(pTEMP);

	if (!err)	{ pTEMP->tempConv = MAKEWORD(pTEMP->scratch_data[0], pTEMP->scratch_data[1]); }

	return err;
}


FctERR NONNULL__ OW_TEMP_Convert(OW_temp_t * const pTEMP)
{
	FctERR err = OW_TEMP_Start_Conversion(pTEMP);

	if (!err)
	{
		while (TPSINF_MS(pTEMP->hStartConv, pTEMP->props.convTimes[pTEMP->resIdx]))
		{
			#if defined(HAL_IWDG_MODULE_ENABLED)
				HAL_IWDG_Refresh(&hiwdg);
			#endif
		}

		err = OW_TEMP_Read_Conversion(pTEMP);

		if (!err)	{ pTEMP->doneConv = true; }
	}

	return err;
}


FctERR NONNULL__ OW_TEMP_Convert_Handler(OW_temp_t * const pTEMP)
{
	FctERR err = ERROR_BUSY;

	if (TPSSUP_MS(pTEMP->hStartConv, pTEMP->props.convTimes[pTEMP->resIdx]))
	{
		pTEMP->doneConv = true;

		err = OW_TEMP_Read_Conversion(pTEMP);
		err = OW_TEMP_Start_Conversion(pTEMP);
	}

	return err;
}


