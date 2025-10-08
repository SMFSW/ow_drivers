/*!\file OW_dev_temp.c
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief OneWire temperature sensor device type
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_drv.h"

#include "OW_dev_temp.h"
/****************************************************************/


FctERR NONNULL__ OW_TEMP_Convert_Handler(OW_temp_t * const pTEMP)
{
	FctERR err = ERROR_OK;

	if (!pTEMP->doneConv)
	{
		if (TPSSUP_MS(pTEMP->hStartConv, pTEMP->props->convTimes[pTEMP->resIdx - pTEMP->props->minResIdx] + 1U))	// Add 1ms to max conversion time
		{
			pTEMP->doneConv = true;

			OW_StrongPull_Set(pTEMP->slave_inst->cfg.bus_inst, false);
			OW_set_busy(pTEMP->slave_inst, false);

			err = OW_TEMP_Read_Conversion(pTEMP);
			//err |= OW_TEMP_Start_Conversion(pTEMP);
		}
		else
		{
			err = ERROR_BUSY;
		}
	}

	return err;
}


/****************************************************************/


FctERR NONNULL__ OWAlarmSearch_All(OW_DRV * const pOW, OW_ROM_ID_t ROMId[], const uint8_t max_nb)
{
	OWSearch_SetType(pOW, OW_TEMP__ALARM_SEARCH);
	FctERR err = OWSearch_All(pOW, ROMId, max_nb);
	OWSearch_SetType(pOW, OW__SEARCH_ROM);	// Switch back to default search command

	return err;
}


/****************************************************************/


__STATIC_INLINE FctERR NONNULL_INLINE__ OW_TEMP_Check_CRC_Scratchpad(const OW_temp_t * const pTEMP) {
	return OWCheck_DallasCRC8(pTEMP->scratch.bytes, OW_TEMP_SCRATCHPAD_SIZE - 1, pTEMP->scratch.bytes[OW_TEMP_SCRATCHPAD_SIZE - 1]); }


FctERR NONNULL__ OW_TEMP_Read_Scratchpad(OW_temp_t * const pTEMP)
{
	OW_slave_t * const		pSlave = pTEMP->slave_inst;
	const OW_DRV * const	pDrv = pSlave->cfg.bus_inst;
	FctERR					err = ERROR_OK;

	if (!OW_is_enabled(pSlave))		{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (OW_is_busy(pSlave))			{ err = ERROR_BUSY; }		// Device busy
	if (err != ERROR_OK)			{ goto ret; }

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pDrv, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)	{ goto err; }

	UNUSED_RET OWWrite_byte(pDrv, OW_TEMP__READ_SCRATCHPAD);
	UNUSED_RET OWRead(pDrv, pTEMP->scratch.bytes, OW_TEMP_SCRATCHPAD_SIZE);

	err = OW_TEMP_Check_CRC_Scratchpad(pTEMP);

	err:
	OW_set_busy(pSlave, false);

	ret:
	return err;
}


static FctERR NONNULL__ OW_TEMP_Recall(OW_temp_t * const pTEMP)
{
	OW_slave_t * const		pSlave = pTEMP->slave_inst;
	const OW_DRV * const	pDrv = pSlave->cfg.bus_inst;
	FctERR					err = ERROR_OK;

	if (!OW_is_enabled(pSlave))		{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (OW_is_busy(pSlave))			{ err = ERROR_BUSY; }		// Device busy
	if (err != ERROR_OK)			{ goto ret; }

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pDrv, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)	{ goto err; }

	UNUSED_RET OWWrite_byte(pSlave->cfg.bus_inst, OW_TEMP__RECALL);

	uint8_t done = 0U;
	while (!done)
	{
		OW_Watchdog_Refresh();
		UNUSED_RET OWRead_byte(pDrv, &done);
	}

	err:
	OW_set_busy(pSlave, false);

	ret:
	return err;
}


static FctERR NONNULL__ OW_TEMP_Copy_Scratchpad(OW_temp_t * const pTEMP)
{
	OW_slave_t * const		pSlave = pTEMP->slave_inst;
	const OW_DRV * const	pDrv = pSlave->cfg.bus_inst;
	FctERR					err = ERROR_OK;

	if (!OW_is_enabled(pSlave))		{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (OW_is_busy(pSlave))			{ err = ERROR_BUSY; }		// Device busy
	if (err != ERROR_OK)			{ goto ret; }

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pDrv, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)			{ goto err;	}

	UNUSED_RET OWWrite_byte(pDrv, OW_TEMP__COPY_SCRATCHPAD);

	OW_StrongPull_Set(pTEMP->slave_inst->cfg.bus_inst, true);

	// Wait for bytes to be copied in EEP
	uint8_t cpt = 10U;
	do
	{
		OW_Watchdog_Refresh();
		HAL_Delay(1U);
	}
	while (--cpt);

	OW_StrongPull_Set(pTEMP->slave_inst->cfg.bus_inst, false);

	err:
	OW_set_busy(pSlave, false);

	ret:
	return err;
}


FctERR NONNULL__ OW_TEMP_Write_Scratchpad(OW_temp_t * const pTEMP)
{
	OW_slave_t * const		pSlave = pTEMP->slave_inst;
	const OW_DRV * const	pDrv = pSlave->cfg.bus_inst;
	FctERR					err = ERROR_OK;

	if (!OW_is_enabled(pSlave))		{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (OW_is_busy(pSlave))			{ err = ERROR_BUSY; }		// Device busy
	if (err != ERROR_OK)			{ goto ret; }

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pDrv, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)	{ goto err; }

	UNUSED_RET OWWrite_byte(pDrv, OW_TEMP__WRITE_SCRATCHPAD);
	UNUSED_RET OWWrite(pDrv, &pTEMP->scratch.bytes[2], pTEMP->props->cfgBytes);

	OW_set_busy(pSlave, false);

	err = OW_TEMP_Copy_Scratchpad(pTEMP);
	if (err != ERROR_OK)	{ goto ret; }

	err = OW_TEMP_Recall(pTEMP);
	if (err != ERROR_OK)	{ goto ret; }

	err = OW_TEMP_Read_Scratchpad(pTEMP);
	if (err != ERROR_OK)	{ goto ret; }

	err:
	OW_set_busy(pSlave, false);

	ret:
	return err;
}


FctERR NONNULL__ OW_TEMP_Start_Conversion(OW_temp_t * const pTEMP)
{
	OW_slave_t * const		pSlave = pTEMP->slave_inst;
	const OW_DRV * const	pDrv = pSlave->cfg.bus_inst;
	FctERR					err = ERROR_OK;

	if (!OW_is_enabled(pSlave))		{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (OW_is_busy(pSlave))			{ err = ERROR_BUSY; }		// Device busy
	if (err != ERROR_OK)			{ goto ret; }

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pDrv, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)
	{
		OW_set_busy(pSlave, false);
		goto ret;
	}

	UNUSED_RET OWWrite_byte(pDrv, OW_TEMP__CONVERT_T);

	OW_StrongPull_Set(pTEMP->slave_inst->cfg.bus_inst, true);

	pTEMP->hStartConv = HALTicks();
	pTEMP->doneConv = false;

	// Do not release slave at this stage, conversion is ongoing

	ret:
	return err;
}


FctERR NONNULL__ OW_TEMP_Read_Conversion(OW_temp_t * const pTEMP)
{
	FctERR err = OW_TEMP_Read_Scratchpad(pTEMP);

	if (!err)	{ pTEMP->tempConv = MAKEWORD(pTEMP->scratch.bytes[0], pTEMP->scratch.bytes[1]); }

	return err;
}


FctERR NONNULL__ OW_TEMP_Convert(OW_temp_t * const pTEMP)
{
	FctERR err = OW_TEMP_Start_Conversion(pTEMP);

	if (!err)
	{
		while (TPSINF_MS(pTEMP->hStartConv, pTEMP->props->convTimes[pTEMP->resIdx - pTEMP->props->minResIdx] + 1U))	// Add 1ms to max conversion time
		{
			OW_Watchdog_Refresh();
		}

		pTEMP->doneConv = true;

		OW_StrongPull_Set(pTEMP->slave_inst->cfg.bus_inst, false);
		OW_set_busy(pTEMP->slave_inst, false);

		err = OW_TEMP_Read_Conversion(pTEMP);
	}

	return err;
}

