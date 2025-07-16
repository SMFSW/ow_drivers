/*!\file OW_rom_id.c
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief OneWire ROM Id
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_rom_id.h"
/****************************************************************/


uint64_t NONNULL__ OWGetSerialNumber(const OW_ROM_ID_t * const pROM)
{
	uint64_t sn = 0;

	for (size_t i = sizeof(pROM->serialNumber) ; i ; i--)
	{
		sn = LSHIFT64(sn, 8U) | pROM->serialNumber[i - 1U];
	}

	return sn;
}


