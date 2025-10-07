/*!\file MAX31826.c
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief MAX31826:  Digital Temperature Sensor with 1Kb Lockable EEPROM
**/
/****************************************************************/
#include "sarmfsw.h"

#include "MAX31826.h"

#if defined(OW_MAX31826)
/****************************************************************/
// std libs
#include <string.h>
/****************************************************************/


static OW_slave_t MAX31826_hal[OW_MAX31826_NB] = { 0 };						//!< MAX31826 Slave structure
MAX31826_t MAX31826[OW_MAX31826_NB] = { 0 };								//!< MAX31826 User structure

static const uint16_t MAX31826_convTimes[] = { 150 };						//!< MAX31826 conversion times (in ms)

static const OW_temp_props_t MAX31826_temp_props = {
	MAX31826_convTimes, OW_TEMP__RES_12BIT, MAX31826__GRANULARITY, 0 };		//!< MAX31826 temperature sensor parameters

static const OW_eep_props_t MAX31826_eep_props = {
	MAX31826_SCRATCHPAD_SIZE, MAX31826_MEMORY_SIZE,
	MAX31826_PAGE_SIZE, MAX31826_PAGES,
	MAX31826_MAX_WRITE_ADDR, MAX31826_MAX_READ_ADDR,
	MAX31826_COPY_TIME };													//!< MAX31826 eeprom parameters

static const OW_ROM_type FAMILY_CODE = OW_TYPE__THERMOMETER__EEPROM_1K;		//!< MAX31826 family code

OW_ROM_type MAX31826_Get_FamilyCode(void) {
	return FAMILY_CODE; }


/****************************************************************/


/*!\brief MAX31826 read scratchpad
** \param[in,out] pCpnt - Pointer to MAX31826 peripheral
** \return FctERR - error code
**/
__STATIC_INLINE float NONNULL_INLINE__ MAX31826_Read_Scratchpad(MAX31826_t * const pCpnt) {
	return OW_TEMP_Read_Scratchpad(&pCpnt->temp); }

#if 0
/*!\brief MAX31826 write scratchpad
** \param[in,out] pCpnt - Pointer to MAX31826 peripheral
** \return FctERR - error code
**/
__STATIC_INLINE float NONNULL_INLINE__ MAX31826_Write_Scratchpad(MAX31826_t * const pCpnt) {
	return OW_TEMP_Write_Scratchpad(&pCpnt->temp); }
#endif


/****************************************************************/


__WEAK FctERR NONNULL__ MAX31826_Init_Sequence(MAX31826_t * const pCpnt)
{
	FctERR err = MAX31826_Read_Scratchpad(pCpnt);

	if (!err)
	{
		pCpnt->location = pCpnt->temp.scratch.configuration.Bits.ADx;
		pCpnt->temp.resIdx = OW_TEMP__RES_12BIT;
	}

	return err;
}

FctERR NONNULL__ MAX31826_Init(const uint8_t idx, OW_DRV * const pOW, const OW_ROM_ID_t * const pROM)
{
	FctERR err = ERROR_DEVICE_UNKNOWN;

	assert_param(IS_OW_PERIPHERAL(MAX31826, idx));

	if (pROM->familyCode == FAMILY_CODE)	// Family code matches
	{
		OW_slave_init(&MAX31826_hal[idx], pOW, pROM);
		OW_SN_SET_DEFAULTS(MAX31826, idx, pROM);
		OW_TEMP_SET_DEFAULTS(MAX31826, idx);
		OW_EEPROM_SET_DEFAULTS(MAX31826, idx);

		err = MAX31826_Init_Sequence(&MAX31826[idx]);
	}

	if (err != ERROR_OK)	{ OW_set_enable(&MAX31826_hal[idx], false); }

	return err;
}

FctERR MAX31826_Init_Single(const OW_ROM_ID_t * const pROM) {
	return MAX31826_Init(0, OW_MAX31826, pROM); }


/****************************************************************/


/*!\brief OneWire EEPROM device copy scratchpad (to memory)
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \return FctERR - error code
**/
__STATIC FctERR NONNULL__ MAX31826_EEP_Copy_Scratchpad(OW_eep_t * const pEEP)
{
	OW_slave_t * const	pSlave = pEEP->slave_inst;

	OW_set_busy(pSlave, true);

	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)
	{
		OW_set_busy(pSlave, false);
		goto ret;
	}

	const uint8_t cmd[2] = { OW_EEP__COPY_SCRATCHPAD, 0xA5U };
	UNUSED_RET OWWrite(pSlave->cfg.bus_inst, cmd, sizeof(cmd));

	pEEP->hStartWrite = HALTicks();
	pEEP->doneWrite = false;

	OW_StrongPull_Set(pEEP->slave_inst->cfg.bus_inst, true);

	// Do not release slave at this stage, copy to eeprom is ongoing

	ret:
	return err;
}


/*!\brief MAX31826 read scratchpad
** \param[in,out] pEEP - Pointer to MAX31826 EEP component
** \return FctERR - error code
**/
__STATIC FctERR NONNULL__ MAX31826_EEP_Read_Scratchpad(OW_eep_t * const pEEP)
{
	OW_slave_t * const		pSlave = pEEP->slave_inst;
	const OW_DRV * const	pDrv = pSlave->cfg.bus_inst;
	uint8_t					crc;
	FctERR					err = ERROR_OK;

	if (!OW_is_enabled(pSlave))		{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (OW_is_busy(pSlave))			{ err = ERROR_BUSY; }		// Device busy
	if (err != ERROR_OK)			{ goto ret; }

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pDrv, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)			{ goto ret; }

	const uint8_t cmd[2] = { OW_EEP__READ_SCRATCHPAD, LOBYTE(pEEP->scratch.address) };
	UNUSED_RET OWWrite(pDrv, cmd, sizeof(cmd));

	UNUSED_RET OWRead(pDrv, pEEP->scratch.pData, pEEP->props->scratchpad_size);
	UNUSED_RET OWRead(pDrv, &crc, sizeof(crc));

	OW_set_busy(pSlave, false);

	pEEP->scratch.crc = crc;

	crc = 0U;
	OWCompute_DallasCRC8(&crc, cmd, sizeof(cmd));
	OWCompute_DallasCRC8(&crc, pEEP->scratch.pData, pEEP->props->scratchpad_size);

	if (crc != pEEP->scratch.crc) { err = ERROR_CRC; }

	ret:
	return err;
}


/*!\brief MAX31826 write scratchpad
** \param[in,out] pEEP - Pointer to MAX31826 EEP component
** \param[in] pData - Pointer to data for transmission
** \param[in] addr - Target memory cell start address
** \param[in] len - Number of data bytes to transmit
** \return FctERR - error code
**/
__STATIC FctERR NONNULL__ MAX31826_EEP_Write_Scratchpad(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const size_t len)
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
	const uint8_t cmd[2] = { OW_EEP__WRITE_SCRATCHPAD, LOBYTE(addr) };
	UNUSED_RET memcpy(pEEP->scratch.pData, pData, len);

	uint8_t CRC_data = 0;
	OWCompute_DallasCRC8(&CRC_data, cmd, sizeof(cmd));
	OWCompute_DallasCRC8(&CRC_data, pData, len);

	OW_set_busy(pSlave, true);

	err = OWROMCmd_Control_Sequence(pDrv, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)	{ goto ret; }

	UNUSED_RET OWWrite(pDrv, cmd, sizeof(cmd));
	UNUSED_RET OWWrite(pDrv, pEEP->scratch.pData, len);

	// Get Byte to verify CRC
	uint8_t CRC_received = 0;
	UNUSED_RET OWRead(pDrv, &CRC_received, sizeof(CRC_received));

	OW_set_busy(pSlave, false);

	if (CRC_data != CRC_received) { err = ERROR_CRC; }
	else
	{
		err = MAX31826_EEP_Read_Scratchpad(pEEP);

		// TODO: check data & iCRC ?

		// If further operations done on bus, set busy again
		//OW_set_busy(pSlave, true);
	}

	ret:
	return err;
}


FctERR NONNULL__ MAX31826_Read_Memory(MAX31826_t * const pCpnt, uint8_t * pData, const uint32_t addr, const size_t len)
{
	OW_eep_t * const		pEEP = &pCpnt->eep;
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
	if (err != ERROR_OK)	{ goto ret; }

	const uint8_t cmd[2] = { OW_EEP__READ_MEMORY, LOBYTE(addr) };
	UNUSED_RET OWWrite(pDrv, cmd, sizeof(cmd));
	UNUSED_RET OWRead(pDrv, pData, len);

	OW_set_busy(pSlave, false);

	ret:
	return err;
}


FctERR NONNULL__ MAX31826_Write_Memory(MAX31826_t * const pCpnt, const uint8_t * pData, const uint32_t addr, const size_t len)
{
	OW_eep_t * const			pEEP = &pCpnt->eep;
	const OW_slave_t * const	pSlave = pEEP->slave_inst;
	FctERR						err = ERROR_OK;

	if (!OW_is_enabled(pSlave))									{ err = ERROR_DISABLED; }	// Peripheral disabled
	if (OW_is_busy(pSlave))										{ err = ERROR_BUSY; }		// Device busy
	if (addr > pEEP->props->max_write_address)					{ err = ERROR_RANGE; }		// Unknown address
	if ((addr + len) > (pEEP->props->max_write_address + 1U))	{ err = ERROR_OVERFLOW; }	// Bank overflow
	if (err != ERROR_OK)										{ goto ret; }

	size_t		data_len = len;
	size_t		unaligned_len = addr % pEEP->props->scratchpad_size;
	uint32_t	address = addr - unaligned_len;	// If unaligned write access, adjust address to read aligned bytes first

	while (data_len != 0U)
	{
		while (MAX31826_WriteCycle_Handler(pCpnt) != ERROR_OK)	// Wait for a previous write to complete
		{
			OW_Watchdog_Refresh();
			HAL_Delay(1U);
		}

		const size_t write_len = min(pEEP->props->scratchpad_size - unaligned_len, data_len);

		if (unaligned_len != 0U)
		{
			const size_t read_len = ((write_len + unaligned_len) == pEEP->props->scratchpad_size) ? unaligned_len : pEEP->props->scratchpad_size;

			err = MAX31826_Read_Memory(pCpnt, pEEP->scratch.pData, address, read_len);
			UNUSED_RET memcpy(&pEEP->scratch.pData[unaligned_len], pData, write_len);
			unaligned_len = 0U;	// Reset unaligned_len, further writes will be aligned
		}
		else if (write_len < pEEP->props->scratchpad_size)
		{
			const size_t read_len = pEEP->props->scratchpad_size - write_len;

			err = MAX31826_Read_Memory(pCpnt, &pEEP->scratch.pData[write_len], address + write_len, read_len);
			UNUSED_RET memcpy(pEEP->scratch.pData, pData, write_len);
		}
		else
		{
			UNUSED_RET memcpy(pEEP->scratch.pData, pData, pEEP->props->scratchpad_size);
		}

		if (err != ERROR_OK)	{ goto ret; }

		// Write scratchpad & Check CRC16 or Read scratchpad to see if matching with written values
		err = MAX31826_EEP_Write_Scratchpad(pEEP, pEEP->scratch.pData, address, pEEP->props->scratchpad_size);
		if (err != ERROR_OK)	{ goto ret; }

		// Copy scratchpad
		err = MAX31826_EEP_Copy_Scratchpad(pEEP);
		if (err != ERROR_OK)	{ goto ret; }

		pData += write_len;
		data_len -= write_len;
		address += pEEP->props->scratchpad_size;
	}

	ret:
	return err;
}


/****************************************************************/


FctERR NONNULL__ MAX31826_Lock_Memory(MAX31826_t * const pCpnt, const MAX31826_eep_area area)
{
	OW_slave_t * const	pSlave = pCpnt->eep.slave_inst;

	OW_set_busy(pSlave, true);

	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);
	if (err != ERROR_OK)			{ goto ret; }

	const uint8_t cmd[2] = { (area == MAX31826__EEP_LOW) ? MAX31826__LOCK_EEP_LOW : MAX31826__LOCK_EEP_HIGH, 0x55U };
	UNUSED_RET OWWrite(pSlave->cfg.bus_inst, cmd, sizeof(cmd));

	OW_set_busy(pSlave, false);

	ret:
	return err;
}


/****************************************************************/
#endif
/****************************************************************/

