/*!\file OW_dev_eeprom.h
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire eeprom device type
**/
/****************************************************************/
#ifndef __OW_DEV_EEPROM_H
	#define __OW_DEV_EEPROM_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#include "OW_component.h"
/****************************************************************/


/**********************************/
/*** Peripheral defaults setter ***/
/**********************************/

#define OW_EEPROM_SET_DEFAULTS(name, idx)									\
	const uint8_t * const pData = name[idx].scratch_data;					\
	memcpy((uint8_t *) &name[idx].scratch.pData, &pData, sizeof(pData));	\
	memcpy(&name[idx].eep.props, &name##_props, sizeof(OW_eep_props_t));	\
	name[idx].eep.slave_inst = &name##_hal[idx];							\
	name[idx].eep.pScratch = &name[idx].scratch;							\
	OW_EEP_Set_WaitProg(&name[idx].eep, true);	//!< Macro to set working defaults for peripheral \b name on index \b idx


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\enum OW_EEP_cmd
** \brief Commands enum for EEPROM device type
**/
typedef enum PACK__ OW_EEP_cmd {
	OW_EEP__WRITE_SCRATCHPAD = 0x0F,	//!< Write scratchpad command
	OW_EEP__COPY_SCRATCHPAD = 0x55,		//!< Copy scratchpad command
	OW_EEP__READ_SCRATCHPAD = 0xAA,		//!< Read scratchpad command
	OW_EEP__READ_MEMORY = 0xF0			//!< Read memory command
} OW_EEP_cmd;


/*!\enum OW_eep_pages
** \brief Pages values for eeprom type devices
**/
typedef enum PACK__ OW_eep_pages {
	OW_EEP__PAGE0 = 0,			//!< EEPROM Page 0
	OW_EEP__PAGE1,				//!< EEPROM Page 1
	OW_EEP__PAGE2,				//!< EEPROM Page 2
	OW_EEP__PAGE3,				//!< EEPROM Page 3
	OW_EEP__PAGE_ALL = 0xFF		//!< EEPROM All Pages
} OW_eep_pages;


/*!\struct OW_eep_props_t
** \brief OneWire EEPROM properties type
**/
typedef struct OW_eep_props_t {
	const uint32_t	scratchpad_size;	//!< Scratchpad size (in bytes)
	const uint32_t	mem_size;			//!< Memory size (in bytes)
	const uint32_t	page_size;			//!< Page size (in bytes)
	const uint32_t	page_nb;			//!< Number of pages
	const uint32_t	max_write_address;	//!< Maximum write address
	const uint32_t	max_read_address;	//!< Maximum read address
} OW_eep_props_t;


/*!\struct OW_EEP_scratch_t
** \brief OneWire EEPROM scratchpad struct
**/
typedef struct OW_EEP_scratch_t {
	uint8_t			ES;			//!< ES register value
	uint8_t			address;	//!< Address
	uint16_t		nb;			//!< Number of bytes
	uint16_t		iCRC16;		//!< Inverted CRC16
	uint8_t * const	pData;		//!< Pointer to scratchpad data (data shall be defined in device struct with its address copied to this data pointer)
} OW_EEP_scratch_t;


/*!\struct OW_eep_t
** \brief OneWire EEPROM configuration type
**/
typedef struct OW_eep_t {
	OW_slave_t *			slave_inst;		//!< Slave structure
	OW_eep_props_t			props;			//!< EEPROM properties
	OW_EEP_scratch_t *		pScratch;		//!< Pointer to mirrored scratchpad data
	bool					wait_prog;		//!< Wait until eeprom programming is done (otherwise, only checks for command error byte)
} OW_eep_t;


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************

/*!\brief OneWire EEPROM device wait programming end setter
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \param[in] en - Enable state
**/
__INLINE void NONNULL_INLINE__ OW_EEP_Set_WaitProg(OW_eep_t * const pEEP, const bool en) {
	pEEP->wait_prog = en; }


/*!\brief OneWire EEPROM device read scratchpad
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \param[in,out] pScratch - Pointer to scratchpad output data
** \return FctERR - error code
**/
FctERR NONNULL__ OW_EEP_Read_Scratchpad(OW_eep_t * const pEEP, OW_EEP_scratch_t * const pScratch);

/*!\brief OneWire EEPROM device write scratchpad
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \param[in] pData - Pointer to data for transmission
** \param[in] addr - Target memory cell start address
** \param[in] nb - Number of data bytes to transmit
** \return FctERR - error code
**/
FctERR NONNULL__ OW_EEP_Write_Scratchpad(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const uint32_t nb);


/*!\brief OneWire EEPROM device read from memory
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \param[in,out] pData - Pointer to data for reception
** \param[in] addr - Target memory cell start address
** \param[in] nb - Number of data bytes to receive
** \return FctERR - error code
**/
FctERR NONNULL__ OW_EEP_Read_Memory(OW_eep_t * const pEEP, uint8_t * pData, const uint32_t addr, const uint32_t nb);

/*!\brief OneWire EEPROM device write to memory
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \param[in] pData - Pointer to data for transmission
** \param[in] addr - Target memory cell start address
** \param[in] nb - Number of data bytes to transmit
** \return FctERR - error code
**/
FctERR NONNULL__ OW_EEP_Write_Memory(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const uint32_t nb);


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif	/* __OW_DEV_EEPROM_H */
/****************************************************************/