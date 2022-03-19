/*!\file OW_drv.c
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
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


/*!\brief OneWire search state reset
** \param[in,out] pOW - Pointer to OneWire driver instance
**/
static void NONNULL__ OWReset_search(OW_DRV * const pOW)
{
	// reset the search state
	pOW->search_state.lastDiscrepancy = 0;
	pOW->search_state.lastFamilyDiscrepancy = 0;
	pOW->search_state.lastDeviceFlag = false;
}


FctERR NONNULL__ OWInit(OW_Handle_t * const pHandle, const uint8_t idx)
{
	/* Check the parameters */
	if (!IS_OW_DRV_IDX(idx))	{ return ERROR_INSTANCE; }

	OW_DRV * const pDrv = &OWdrv[idx];

	pDrv->phy_inst.inst = pHandle;

	return OWInit_phy(idx);
}

FctERR NONNULL__ OWWrite_bit(const OW_DRV * const pOW, const uint8_t bit)
{
	FctERR err = ERROR_OK;

	if (pOW->pfWriteBit != NULL)	{ err |= pOW->pfWriteBit(pOW, bit); }
	else							{ err = ERROR_INSTANCE; }

	return err;
}


FctERR NONNULL__ OWWrite_byte(const OW_DRV * const pOW, const uint8_t byte)
{
#if OW_CUSTOM_BYTE_HANDLERS
	FctERR err;

	if (pOW->pfWriteByte != NULL)	{ err = pOW->pfWriteByte(pOW, byte); }
	else							{ err = ERROR_INSTANCE; }

	return err;
#else
	FctERR	err;
	uint8_t	data = byte;

	for (int j = 8 ; j ; j--)
	{
		err = OWWrite_bit(pOW, data & 0x01);
		if (err)	{ break; }

		data >>= 1;
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
		err |= OWWrite_byte(pOW, *pByte++);
		if (err)	{ break; }
	}

	return err;
}


FctERR NONNULL__ OWRead_bit(const OW_DRV * const pOW, uint8_t * const pBit)
{
	FctERR err = ERROR_OK;

	if (pOW->pfReadBit != NULL)		{ err |= pOW->pfReadBit(pOW, pBit); }
	else							{ err = ERROR_INSTANCE; }

	return err;
}


FctERR NONNULL__ OWRead_byte(const OW_DRV * const pOW, uint8_t * const pByte)
{
#if OW_CUSTOM_BYTE_HANDLERS
	FctERR err = ERROR_OK;

	if (pOW->pfReadByte != NULL)	{ err |= pOW->pfReadByte(pOW, pByte); }
	else							{ err = ERROR_INSTANCE; }

	return err;
#else
	FctERR	err;
	uint8_t	bit;

	*pByte = 0;

	for (uint8_t mask = 0x01 ; mask ; mask <<= 1)
	{
		err = OWRead_bit(pOW, &bit);
		if (err)	{ break; }

		if (bit) 	{ *pByte |= mask; }
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
		err |= OWRead_byte(pOW, pByte++);
		if (err)	{ break; }
	}

	return err;
}


FctERR NONNULL__ OWReset(const OW_DRV * const pOW)
{
	FctERR err = ERROR_OK;

	if (pOW->pfReset != NULL)	{ err |= pOW->pfReset(pOW); }
	else						{ err = ERROR_INSTANCE; }

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
	OWWrite_byte(pOW, OW__RESUME);	// Resume
}


/*!\brief OneWire search device
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pROM - Pointer to ROM Id structure
** \return FctERR - Error code
**/
static FctERR NONNULLX__(1) OWSearch(OW_DRV * const pOW, OW_ROM_ID_t * const pROMId)
{	/*** https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/187.html ***/
	FctERR search_result = ERROR_BUSOFF;

	// if the last call was not the last one
	if (!pOW->search_state.lastDeviceFlag)
	{
		uint8_t id_bit_number = 1;

		// 1-Wire reset
		if (OWReset(pOW))
		{
			// reset the search
			OWReset_search(pOW);
			return ERROR_BUSOFF;
		}

		// issue the search command
		OWWrite_byte(pOW, OW__SEARCH_ROM);

		// loop to do the search
		uint8_t last_zero = 0, rom_byte_number = 0, rom_byte_mask = 1, crc8 = 0;
		do
		{
			FctERR	err;
			uint8_t	search_direction, id_bit, cmp_id_bit;

			// read a bit and its complement
			err = OWRead_bit(pOW, &id_bit);
			if (err)	{ return err; }
			err = OWRead_bit(pOW, &cmp_id_bit);
			if (err)	{ return err; }

			// check for no devices on 1-wire
			if ((id_bit == 1) && (cmp_id_bit == 1))
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
						search_direction = ((pOW->search_state.ROM_ID.romId[rom_byte_number] & rom_byte_mask) > 0);
					}
					else
					{
						// if equal to last pick 1, if not then pick 0
						search_direction = (id_bit_number == pOW->search_state.lastDiscrepancy);
					}

					// if 0 was picked then record its position in LastZero
					if (search_direction == 0)
					{
						last_zero = id_bit_number;

						// check for Last discrepancy in family
						if (last_zero < 9)
						{
							pOW->search_state.lastFamilyDiscrepancy = last_zero;
						}
					}
				}

				// set or clear the bit in the ROM byte rom_byte_number with mask rom_byte_mask
				if (search_direction == 1)	{ pOW->search_state.ROM_ID.romId[rom_byte_number] |= rom_byte_mask; }
				else						{ pOW->search_state.ROM_ID.romId[rom_byte_number] &= ~rom_byte_mask; }

				// serial number search direction write bit
				OWWrite_bit(pOW, search_direction);

				// increment the byte counter id_bit_number
				// and shift the mask rom_byte_mask
				id_bit_number++;
				rom_byte_mask <<= 1;

				// if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
				if (rom_byte_mask == 0)
				{
					OWCompute_DallasCRC8(&crc8, &pOW->search_state.ROM_ID.romId[rom_byte_number], 1);  // accumulate the CRC
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
		}
		while (rom_byte_number < OW_ROM_ID_SIZE);	// loop until through all ROM bytes 0-7

		// if the search was successful then
		if (!((id_bit_number < 65) || (crc8 != 0)))
		{
			// search successful so set LastDiscrepancy, LastDeviceFlag, search_result
			pOW->search_state.lastDiscrepancy = last_zero;

			// check for last device
			if (pOW->search_state.lastDiscrepancy == 0)	{ pOW->search_state.lastDeviceFlag = true; }

			search_result = ERROR_OK;
		}
	}

	// if no device found then reset counters so next 'search' will be like a first
	if (!pOW->search_state.ROM_ID.romId[0])	{ search_result = ERROR_NOTAVAIL; }
	if (search_result)						{ OWReset_search(pOW); }
	else
	{
		if (pROMId != NULL)	{ memcpy(pROMId->romId, pOW->search_state.ROM_ID.romId, sizeof(OW_ROM_ID_t)); }
	}

	return search_result;
}


FctERR NONNULLX__(1) OWFirst(OW_DRV * const pOW, OW_ROM_ID_t * const pROM)
{
	/* Reset search values */
	OWReset_search(pOW);

	/* Start with searching */
	return OWSearch(pOW, pROM);
}


FctERR NONNULLX__(1) OWNext(OW_DRV * const pOW, OW_ROM_ID_t * const pROM)
{
	/* Leave the search state alone */
	return OWSearch(pOW, pROM);
}


FctERR NONNULL__ OWSearchAll(OW_DRV * const pOW, OW_ROM_ID_t ROMId[], const uint8_t max_nb)
{
	FctERR	err = ERROR_OK;
	uint8_t idx = 0;

	/* Reset search values */
	OWReset_search(pOW);

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
	memcpy(&bak, &pOW->search_state, sizeof(OWSearch_State_t));

	/* Set search to find the same device */
	pOW->search_state.lastDiscrepancy = 64;
	pOW->search_state.lastDeviceFlag = 0;

	if (OWSearch(pOW, NULL))
	{
		/* Check if same device found */
		if (memcmp(bak.ROM_ID.romId, pOW->search_state.ROM_ID.romId, sizeof(OW_ROM_ID_SIZE)))	{ err = ERROR_VALUE; }
	}

	/* Restore backup state */
	memcpy(&pOW->search_state, &bak, sizeof(OWSearch_State_t));

	return err;
}


void NONNULL__ OWTargetSetup(OW_DRV * const pOW, const OW_ROM_type family_code)
{
	/* Set the search state to find SearchFamily type devices */
	memset(pOW->search_state.ROM_ID.romId, 0, sizeof(pOW->search_state.ROM_ID.romId));
	pOW->search_state.ROM_ID.familyCode = family_code;

	pOW->search_state.lastDiscrepancy = 64;
	pOW->search_state.lastFamilyDiscrepancy = 0;
	pOW->search_state.lastDeviceFlag = false;
}


void NONNULL__ OWFamilySkipSetup(OW_DRV * const pOW)
{
	/* Set the Last discrepancy to last family discrepancy */
	pOW->search_state.lastDiscrepancy = pOW->search_state.lastFamilyDiscrepancy;
	pOW->search_state.lastFamilyDiscrepancy = 0;

	/* Check for end of list */
	if (pOW->search_state.lastDiscrepancy == 0) { pOW->search_state.lastDeviceFlag = true; }
}


__WEAK FctERR NONNULL__ OWROMCmd_Control_Sequence(const OW_DRV * const pOW, const OW_ROM_ID_t * const pROM, const void * const pfOriginator)
{
	// TODO: Parameters may need to be adjusted (eg. maybe pass familyCode (OW_ROM_type) as parameter)

	FctERR err = OWReset(pOW);
	if (err)	{ return err; }

	/**\code
	// EXAMPLE
	if (pfOriginator == XXX_Command)
	{
		if (memcmp(pOW->ROM_ID.romId, pROM->romId, sizeof(OW_ROM_ID)))
		{
			OWSkip(pOW);
		}
	}
	\endcode**/

	OWSelect(pOW, pROM);

	return err;
}


FctERR NONNULL__ OWRead_ROM_Id(const OW_DRV * const pOW, OW_ROM_ID_t * const pROM)
{
	FctERR err = OWReset(pOW);
	if (err)	{ return err; }

	OWWrite_byte(pOW, OW__READ_ROM);
	OWRead(pOW, pROM->romId, OW_ROM_ID_SIZE);

	return OWCheck_DallasCRC8(pROM->romId, 7, pROM->romId[7]);
}

