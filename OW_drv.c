/*!\file OW_drv.c
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief GPIO OneWire driver
**/
/****************************************************************/
#include <string.h>

#include "sarmfsw.h"

#include "OW_drv.h"
#include "OW_phy.h"
#include "OW_component.h"
/****************************************************************/
// TODO: see about timings depending baud rate
// TODO: generic instance for OneWire over UART, I2C


OW_DRV OWdrv[OW_BUS_NB] = { 0 };		//!< OWdrv structure


FctERR OWInit(OW_Handle_t * const pHandle, const uint8_t idx)
{
	FctERR err = ERROR_INSTANCE;

	if (IS_OW_DRV_IDX(idx))
	{
		OW_DRV * const pOW = &OWdrv[idx];

		pOW->phy_inst.inst = pHandle;
		pOW->bus_powered = true;				// set to true as should be most of the times, and check is called manually

		OWSearch_SetType(pOW, OW__SEARCH_ROM);	// Set default search command

		err = OWInit_phy(idx);
	}

	return err;
}


FctERR NONNULL__ OWWrite_bit(const OW_DRV * const pOW, const uint8_t bit)
{
	FctERR err = ERROR_INSTANCE;

	if (pOW->pfWriteBit != NULL)	{ err = pOW->pfWriteBit(pOW, bit); }

	return err;
}


FctERR NONNULL__ OWWrite_byte(const OW_DRV * const pOW, const uint8_t byte)
{
#if OW_CUSTOM_BYTE_HANDLERS
	FctERR err = ERROR_INSTANCE;

	if (pOW->pfWriteByte != NULL)	{ err = pOW->pfWriteByte(pOW, byte); }

	return err;
#else
	FctERR	err;
	uint8_t	data = byte;

	for (size_t j = 8U ; j ; j--)
	{
		err = OWWrite_bit(pOW, data & 0x01U);
		if (err != ERROR_OK)	{ break; }

		data >>= 1U;
	}

	return err;
#endif
}


FctERR NONNULL__ OWWrite(const OW_DRV * const pOW, const uint8_t * const pData, const size_t len)
{
	FctERR			err = ERROR_OK;
	const uint8_t *	pByte = pData;

	for (size_t i = len ; i ; i--)
	{
		err = OWWrite_byte(pOW, *pByte++);
		if (err != ERROR_OK)	{ break; }
	}

	return err;
}


FctERR NONNULL__ OWRead_bit(const OW_DRV * const pOW, uint8_t * const pBit)
{
	FctERR err = ERROR_INSTANCE;

	if (pOW->pfReadBit != NULL)		{ err = pOW->pfReadBit(pOW, pBit); }

	return err;
}


FctERR NONNULL__ OWRead_byte(const OW_DRV * const pOW, uint8_t * const pByte)
{
#if OW_CUSTOM_BYTE_HANDLERS
	FctERR err = ERROR_INSTANCE;

	if (pOW->pfReadByte != NULL)	{ err = pOW->pfReadByte(pOW, pByte); }

	return err;
#else
	FctERR	err;
	uint8_t	bit;

	*pByte = 0U;

	for (uint8_t mask = 0x01U ; mask ; mask <<= 1U)
	{
		err = OWRead_bit(pOW, &bit);
		if (err != ERROR_OK)	{ break; }

		if (bit != 0U) 			{ *pByte |= mask; }
	}

	return err;
#endif
}


FctERR NONNULL__ OWRead(const OW_DRV * const pOW, uint8_t * const pData, const size_t len)
{
	FctERR		err = ERROR_OK;
	uint8_t *	pByte = pData;

	for (size_t i = len ; i ; i--)
	{
		err = OWRead_byte(pOW, pByte++);
		if (err != ERROR_OK)	{ break; }
	}

	return err;
}


/*!\brief OneWire bus reset
** \param[in] pOW - Pointer to OneWire driver instance
** \return FctERR - Error code
**/
__STATIC FctERR NONNULL__ OWReset(const OW_DRV * const pOW)
{
	FctERR err = ERROR_INSTANCE;

	if (pOW->pfReset != NULL)	{ err = pOW->pfReset(pOW); }

	return err;
}


void NONNULL__ OWSelect(const OW_DRV * const pOW, const OW_ROM_ID_t * const pROM)
{
	OWWrite_byte(pOW, OW__MATCH_ROM);
	OWWrite(pOW, pROM->romId, sizeof(OW_ROM_ID_t));
}


void NONNULL__ OWSkip(const OW_DRV * const pOW)
{
	OWWrite_byte(pOW, OW__SKIP_ROM);	// Skip ROM
}


void NONNULL__ OWResume(const OW_DRV * const pOW)
{
	OWWrite_byte(pOW, OW__RESUME);		// Resume
}


/*!\brief OneWire search state reset
** \param[in,out] pOW - Pointer to OneWire driver instance
**/
__STATIC_INLINE void NONNULL_INLINE__ OWSearch_Reset(OW_DRV * const pOW)
{
	UNUSED_RET memset(&pOW->search_state, 0, sizeof(OWSearch_State_t));	// reset the search state
}


/*!\brief OneWire search device
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pROMId - Pointer to ROM Id structure
** \return FctERR - Error code
**/
__STATIC FctERR NONNULLX__(1) OWSearch(OW_DRV * const pOW, OW_ROM_ID_t * const pROMId)
{	/*** https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/187.html ***/
	FctERR err = ERROR_BUSOFF;

	// if the last call was not the last one
	if (!pOW->search_state.lastDeviceFlag)
	{
		uint8_t id_bit_number = 1U;

		// 1-Wire reset
		if (OWReset(pOW) != ERROR_OK)
		{
			// reset the search
			OWSearch_Reset(pOW);
			goto ret;
		}

		// issue the search command
		OWWrite_byte(pOW, pOW->search_type);

		// loop to do the search
		uint8_t last_zero = 0U;
		uint8_t	rom_byte_number = 0U;
		uint8_t	rom_byte_mask = 1U;
		uint8_t	crc8 = 0U;
		do
		{
			uint8_t	search_direction;
			uint8_t	id_bit;
			uint8_t	cmp_id_bit;

			// read a bit and its complement
			err = OWRead_bit(pOW, &id_bit);
			if (err != ERROR_OK)	{ goto ret; }
			err = OWRead_bit(pOW, &cmp_id_bit);
			if (err != ERROR_OK)	{ goto ret; }

			// check for no devices on 1-wire
			if ((id_bit == 1U) && (cmp_id_bit == 1U))
			{
				break;
			}
			else
			{
				// all devices coupled have 0 or 1
				if (id_bit != cmp_id_bit)
				{
					search_direction = id_bit;  // bit write value for search
				}
				else
				{
					// if this discrepancy if before the Last Discrepancy
					// on a previous next then pick the same as last time
					if (id_bit_number < pOW->search_state.lastDiscrepancy)
					{
						search_direction = binEval((pOW->search_state.ROM_ID.romId[rom_byte_number] & rom_byte_mask) > 0U);
					}
					else
					{
						// if equal to last pick 1, if not then pick 0
						search_direction = binEval(id_bit_number == pOW->search_state.lastDiscrepancy);
					}

					// if 0 was picked then record its position in LastZero
					if (search_direction == 0U)
					{
						last_zero = id_bit_number;

						// check for Last discrepancy in family
						if (last_zero < 9U)
						{
							pOW->search_state.lastFamilyDiscrepancy = last_zero;
						}
					}
				}

				// set or clear the bit in the ROM byte rom_byte_number with mask rom_byte_mask
				if (search_direction == 1U)	{ SET_BITS(pOW->search_state.ROM_ID.romId[rom_byte_number], rom_byte_mask); }
				else						{ CLR_BITS(pOW->search_state.ROM_ID.romId[rom_byte_number], rom_byte_mask); }

				// serial number search direction write bit
				OWWrite_bit(pOW, search_direction);

				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1U;

				// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0U)
				{
					OWCompute_DallasCRC8(&crc8, &pOW->search_state.ROM_ID.romId[rom_byte_number], 1U);  // accumulate the CRC
					rom_byte_number++;
					rom_byte_mask = 1U;
				}
			}
		}
		while (rom_byte_number < OW_ROM_ID_SIZE);	// loop until through all ROM bytes 0-7

		// if the search was successful
		if (!((id_bit_number < 65U) || (crc8 != 0U)))
		{
			// search successful so set LastDiscrepancy, LastDeviceFlag, err
			pOW->search_state.lastDiscrepancy = last_zero;

			// check for last device
			if (pOW->search_state.lastDiscrepancy == 0U)	{ pOW->search_state.lastDeviceFlag = true; }

			err = ERROR_OK;
		}
	}

	// if no device found then reset counters so next 'search' will be like a first
	if (!pOW->search_state.ROM_ID.romId[0])	{ err = ERROR_NOTAVAIL; }
	if (err != ERROR_OK)					{ OWSearch_Reset(pOW); }
	else
	{
		if (pROMId != NULL)	{ memcpy(pROMId->romId, pOW->search_state.ROM_ID.romId, OW_ROM_ID_SIZE); }
	}

	ret:
	return err;
}


FctERR NONNULLX__(1) OWSearch_First(OW_DRV * const pOW, OW_ROM_ID_t * const pROM)
{
	/* Reset search values */
	OWSearch_Reset(pOW);

	/* Start with searching */
	return OWSearch(pOW, pROM);
}


FctERR NONNULLX__(1) OWSearch_Next(OW_DRV * const pOW, OW_ROM_ID_t * const pROM)
{
	/* Leave the search state alone */
	return OWSearch(pOW, pROM);
}


FctERR NONNULL__ OWSearch_All(OW_DRV * const pOW, OW_ROM_ID_t ROMId[], const uint8_t max_nb)
{
	FctERR	err = ERROR_OK;
	uint8_t idx = 0U;

	/* Reset search values */
	OWSearch_Reset(pOW);

	/* Start with searching */
	do
	{
		err |= OWSearch(pOW, &ROMId[idx++]);
	}
	while ((!err) && (!pOW->search_state.lastDeviceFlag) && (idx < max_nb));

	return err;
}


FctERR NONNULL__ OWVerify(OW_DRV * const pOW)
{
	FctERR				err = ERROR_OK;
	OWSearch_State_t	bak;

	/* Backup current state */
	UNUSED_RET memcpy(&bak, &pOW->search_state, sizeof(OWSearch_State_t));

	/* Set search to find the same device */
	pOW->search_state.lastDiscrepancy = 64U;
	pOW->search_state.lastDeviceFlag = 0U;

	if (OWSearch(pOW, NULL) != ERROR_OK)
	{
		/* Check if same device found */
		if (memcmp(bak.ROM_ID.romId, pOW->search_state.ROM_ID.romId, OW_ROM_ID_SIZE) != 0)	{ err = ERROR_VALUE; }
	}

	/* Restore backup state */
	UNUSED_RET memcpy(&pOW->search_state, &bak, sizeof(OWSearch_State_t));

	return err;
}


void NONNULL__ OWTargetSetup(OW_DRV * const pOW, const OW_ROM_type family_code)
{
	/* Set the search state to find SearchFamily type devices */
	UNUSED_RET memset(pOW->search_state.ROM_ID.romId, 0U, sizeof(pOW->search_state.ROM_ID.romId));
	pOW->search_state.ROM_ID.familyCode = family_code;

	pOW->search_state.lastDiscrepancy = 64U;
	pOW->search_state.lastFamilyDiscrepancy = 0U;
	pOW->search_state.lastDeviceFlag = false;
}


void NONNULL__ OWFamilySkipSetup(OW_DRV * const pOW)
{
	/* Set the Last discrepancy to last family discrepancy */
	pOW->search_state.lastDiscrepancy = pOW->search_state.lastFamilyDiscrepancy;
	pOW->search_state.lastFamilyDiscrepancy = 0U;

	/* Check for end of list */
	if (pOW->search_state.lastDiscrepancy == 0U) { pOW->search_state.lastDeviceFlag = true; }
}


FctERR NONNULL__ OWCheckPowerSupply(OW_DRV * const pOW)
{
	FctERR err = OWReset(pOW);

	if (!err)
	{
		OWSkip(pOW);

		// issue the read power supply command
		OWWrite_byte(pOW, OW__READ_POWER_SUPPLY);

		uint8_t power;
		OWRead_byte(pOW, &power);

		pOW->bus_powered = nbinEval(power);
	}

	return err;
}


FctERR NONNULL__ OWROMCmd_Control_Sequence(const OW_DRV * const pOW, const OW_ROM_ID_t * const pROM, const bool broadcast)
{
	FctERR err = OWReset(pOW);

	if (!err)
	{
		if (broadcast)	{ OWSkip(pOW); }
		else			{ OWSelect(pOW, pROM); }
	}

	return err;
}


FctERR NONNULL__ OWRead_ROM_Id(const OW_DRV * const pOW, OW_ROM_ID_t * const pROM)
{
	FctERR err = OWReset(pOW);

	if (!err)
	{
		OWWrite_byte(pOW, OW__READ_ROM);
		OWRead(pOW, pROM->romId, OW_ROM_ID_SIZE);

		err = OWCheck_DallasCRC8(pROM->romId, 7, pROM->romId[7]);
	}

	return err;
}

