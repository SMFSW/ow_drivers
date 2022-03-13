/*!\file OW_dev_sn.h
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire serial number device type (every OneWire device inheriting from serial number)
**/
/****************************************************************/
#ifndef __OW_DEV_SN_H
	#define __OW_DEV_SN_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#include "OW_component.h"
/****************************************************************/


/**********************************/
/*** Peripheral defaults setter ***/
/**********************************/

#define OW_SN_SET_DEFAULTS(name, idx, pROM)								\
	name[idx].sn.slave_inst = &name##_hal[idx];							\
	const uint64_t sn = OWGetSerialNumber(pROM);						\
	memcpy((uint8_t *) &name[idx].sn.serial_number, &sn, sizeof(sn));	//!< Macro to set working defaults for peripheral \b name on index \b idx


#define OW_SN_GETTER(name)													\
	/*!\brief name Serial Number getter										\
	** \param[in] pCpnt - Pointer to name peripheral						\
	** \return name peripheral serial number								\
	**/																		\
__INLINE uint64_t NONNULL_INLINE__ name##_SN_Get(name##_t * const pCpnt) {	\
	uint64_t SN;															\
	OW_SN_Get(&pCpnt->sn, &SN);												\
	return SN; }	//!< Macro to generate serial number getter for peripheral \b name


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\struct OW_sn_t
** \brief OneWire Serial Number configuration type
**/
typedef struct OW_sn_t {
	OW_slave_t *	slave_inst;		//!< Slave structure
	const uint64_t	serial_number;	//!< Serial Number
} OW_sn_t;


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************

/*!\brief OneWire serial number getter for sn type device
** \param[in] pSN - Pointer to serial number device type structure
** \param[in,out] pSerialNumber - Pointer to output serial number
** \return FctERR - error code
**/
FctERR NONNULL__ OW_SN_Get(const OW_sn_t * const pSN, uint64_t * const pSerialNumber);


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif	/* __OW_DEV_SN_H */
/****************************************************************/
