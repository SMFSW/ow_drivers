/*!\file OW_rom_id.h
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire ROM Id
**/
/****************************************************************/
#ifndef __OW_ROM_ID_H
	#define __OW_ROM_ID_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#include "OW_dev_type.h"
/****************************************************************/


// *****************************************************************************
// Section: Constants
// *****************************************************************************
#define OW_ROM_ID_SIZE	8	//!< ROM Id size in bytes


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*! \struct OW_ROM_ID_t
**  \brief One Wire ROM Identification type
**  \note S
**/
typedef struct PACK__ OW_ROM_ID_t {
	union PACK__ {
		uint8_t			romId[OW_ROM_ID_SIZE];	//!< ROM Id bytes array
		struct PACK__ {
			OW_ROM_type	familyCode;				//!< OW device family code
			/*!\note Serial number has LSB located in first byte **/
			uint8_t		serialNumber[6];		//!< OW device serial number
			uint8_t		crc;					//!< OW device ROM Id Dallas CRC8
		};
	};
} OW_ROM_ID_t;


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************

/*!\brief Serial Number getter
** \param[in] pROM - Pointer to ROM Id structure
** \return ROM Id serial number
**/
uint64_t NONNULL__ OWGetSerialNumber(const OW_ROM_ID_t * const pROM);



/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif	/* __OW_ROM_ID_H */
/****************************************************************/
