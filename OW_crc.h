/*!\file OW_crc.h
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire CRC
**/
/****************************************************************/
#ifndef __OW_CRC_H
	#define __OW_CRC_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#ifndef ONEWIRE_CRC8_TABLE
#define ONEWIRE_CRC8_TABLE	0	//!< Select table-lookup method if set to 1 (table enlarges code size by about 250 bytes)
#endif
/****************************************************************/


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************

/*!\brief Dallas CRC8 computation
** \note Function accumulates CRC8 with previous content of given pointer to CRC8 output
** \param[in,out] pCRC8 - Pointer to CRC8 output (note that previous pointer content is used as CRC8 starting seed)
** \param[in] pData - Pointer to data for CRC8 computation
** \param[in] nb - Number of data bytes for CRC8 computation
**/
void NONNULL__ OWCompute_DallasCRC8(uint8_t * const pCRC8, const uint8_t * const pData, const uint16_t nb);

/*!\brief Dallas CRC8 check
** \param[in] pData - Pointer to data for CRC8 computation
** \param[in] nb - Number of data bytes for CRC8 computation
** \param[in] crc8 - CRC8 value to check against
** \return FctERR - Error code
** \retval ERROR_OK - CRC8 check pass
** \retval ERROR_CRC - CRC8 check fail
**/
FctERR NONNULL__ OWCheck_DallasCRC8(const uint8_t * const pData, const uint16_t nb, const uint8_t crc8);


/*!\brief Dallas CRC16 computation
** \note Function accumulates CRC16 with previous content of given pointer to CRC16 output
** \param[in,out] pCRC16 - Pointer to CRC16 output (note that previous pointer content is used as CRC16 starting seed)
** \param[in] pData - Pointer to data for CRC16 computation
** \param[in] nb - Number of data bytes for CRC16 computation
**/
void NONNULL__ OWCompute_DallasCRC16(uint16_t * const pCRC16, const uint8_t * const pData, const uint16_t nb);

/*!\brief Dallas CRC16 check
** \param[in] pData - Pointer to data for CRC16 computation
** \param[in] nb - Number of data bytes for CRC16 computation
** \param[in] icrc16 - Inverted CRC16 value to check against
** \return FctERR - Error code
** \retval ERROR_OK - CRC16 check pass
** \retval ERROR_CRC - CRC16 check fail
**/
FctERR NONNULL__ OWCheck_DallasCRC16(const uint8_t * const pData, const uint16_t nb, const uint16_t icrc16);


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif	/* __OW_CRC_H */
/****************************************************************/
