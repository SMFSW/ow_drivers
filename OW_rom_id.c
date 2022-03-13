/*!\file OW_rom_id.c
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire ROM Id
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_rom_id.h"
/****************************************************************/


uint64_t NONNULL__ OWGetSerialNumber(const OW_ROM_ID_t * const pROM)
{
	uint64_t sn = 0;

	for (int i = sizeof(pROM->serialNumber) - 1 ; i >= 0 ; i--)
	{
		sn = LSHIFT64(sn, 8) | pROM->serialNumber[i];
	}

	return sn;
}


