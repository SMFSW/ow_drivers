/*!\file OW_dev_sn.h
** \author SMFSW
** \copyright MIT (c) 2021-2026, SMFSW
** \brief OneWire serial number device type (every OneWire device inheriting from serial number)
**/
/****************************************************************/
#ifndef OW_DEV_SN_H__
	#define OW_DEV_SN_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#include "OW_component.h"
/****************************************************************/


/**********************************/
/*** Peripheral defaults setter ***/
/**********************************/

#define OW_SN_SET_DEFAULTS(name, idx, pROM)					\
	name[idx].sn.slave_inst = &name##_hal[idx];				\
	name[idx].sn.serial_number = OWGetSerialNumber(pROM);				//!< Macro to set working defaults for peripheral \b name on index \b idx


#define OW_SN_OFFSET(name)	OW_PERIPHERAL_DEV_OFFSET(name, sn)			//!< Macro to get sn structure offset in \b name peripheral structure


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\struct OW_sn_t
** \brief OneWire Serial Number configuration type
**/
typedef struct {
	OW_slave_t *	slave_inst;		//!< Slave structure
	uint64_t		serial_number;	//!< Serial Number
} OW_sn_t;


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************

/*!\brief OneWire serial number getter for sn type device
** \param[in] pSN - Pointer to serial number device type structure
** \return Serial Number
**/
__INLINE uint64_t NONNULL_INLINE__ OW_SN_Get_SN(const OW_sn_t * const pSN) {
	return pSN->serial_number; }


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif
/****************************************************************/
