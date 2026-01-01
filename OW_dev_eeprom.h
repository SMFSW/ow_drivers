/*!\file OW_dev_eeprom.h
** \author SMFSW
** \copyright MIT (c) 2021-2026, SMFSW
** \brief OneWire eeprom device type
**/
/****************************************************************/
#ifndef OW_DEV_EEPROM_H__
	#define OW_DEV_EEPROM_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#include "OW_component.h"
/****************************************************************/


/**********************************/
/*** Peripheral defaults setter ***/
/**********************************/

#define OW_EEPROM_SET_DEFAULTS(name, idx)					\
	name[idx].eep.slave_inst = &name##_hal[idx];			\
	name[idx].eep.scratch.pData = name[idx].scratch_data;	\
	name[idx].eep.props = &name##_eep_props;				\
	name[idx].eep.doneWrite = true;							//!< Macro to set working defaults for peripheral \b name on index \b idx


#define OW_EEPROM_OFFSET(name)	OW_PERIPHERAL_DEV_OFFSET(name, eep)		//!< Macro to get eep structure offset in \b name peripheral structure


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\enum OW_EEP_cmd
** \brief Commands enum for EEPROM device type
**/
typedef enum PACK__ {
	OW_EEP__WRITE_SCRATCHPAD = 0x0FU,	//!< Write scratchpad command
	OW_EEP__COPY_SCRATCHPAD = 0x55U,	//!< Copy scratchpad command
	OW_EEP__READ_SCRATCHPAD = 0xAAU,	//!< Read scratchpad command
	OW_EEP__READ_MEMORY = 0xF0U			//!< Read memory command
} OW_EEP_cmd;


/*!\enum OW_eep_pages
** \brief Pages values for eeprom type devices
**/
typedef enum PACK__ {
	OW_EEP__PAGE0 = 0U,			//!< EEPROM Page 0
	OW_EEP__PAGE1,				//!< EEPROM Page 1
	OW_EEP__PAGE2,				//!< EEPROM Page 2
	OW_EEP__PAGE3,				//!< EEPROM Page 3
	OW_EEP__PAGE4,				//!< EEPROM Page 4
	OW_EEP__PAGE5,				//!< EEPROM Page 5
	OW_EEP__PAGE6,				//!< EEPROM Page 6
	OW_EEP__PAGE7,				//!< EEPROM Page 7
	OW_EEP__PAGE8,				//!< EEPROM Page 8
	OW_EEP__PAGE_ALL = 0xFFU	//!< EEPROM All Pages
} OW_eep_pages;


/*!\struct OW_eep_props_t
** \brief OneWire EEPROM properties type
**/
typedef struct {
	size_t		scratchpad_size;	//!< Scratchpad size (in bytes)
	size_t		mem_size;			//!< Memory size (in bytes)
	size_t		page_size;			//!< Page size (in bytes)
	uint32_t	page_nb;			//!< Number of pages
	uint32_t	max_write_address;	//!< Maximum write address
	uint32_t	max_read_address;	//!< Maximum read address
	uint8_t		write_cycle_time;	//!< Maximum time for a write cycle
} OW_eep_props_t;


/*!\struct OW_eep_scratch_t
** \brief OneWire EEPROM scratchpad struct
**/
typedef struct {
	uint8_t		ES;			//!< ES register value
	uint16_t	crc;		//!< Scratchpad CRC
	uint32_t	address;	//!< Address
	size_t		nb;			//!< Number of bytes
	uint8_t *	pData;		//!< Pointer to scratchpad data (data shall be defined in device struct with its address copied to this data pointer)
} OW_eep_scratch_t;


/*!\struct OW_eep_t
** \brief OneWire EEPROM configuration type
**/
typedef struct {
	OW_slave_t *			slave_inst;		//!< Slave structure
	const OW_eep_props_t *	props;			//!< EEPROM properties
	OW_eep_scratch_t		scratch;		//!< Scratchpad structure
	uint32_t				hStartWrite;	//!< Write time start
	bool					doneWrite;		//!< Write done status
} OW_eep_t;


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************

/*!\brief OneWire EEPROM device write cycle time handler
** \note Non blocking mode: start copy, test copy time, release bus
** \note Handler shall be called periodically in a main like loop
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \return FctERR - error code
**/
FctERR NONNULL__ OW_EEP_WriteCycle_Handler(OW_eep_t * const pEEP);


/*!\brief OneWire EEPROM device read scratchpad
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \return FctERR - error code
**/
FctERR NONNULL__ OW_EEP_Read_Scratchpad(OW_eep_t * const pEEP);

/*!\brief OneWire EEPROM device write scratchpad
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \param[in] pData - Pointer to data for transmission
** \param[in] addr - Target memory cell start address
** \param[in] len - Number of data bytes to transmit
** \return FctERR - error code
**/
FctERR NONNULL__ OW_EEP_Write_Scratchpad(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const size_t len);


/*!\brief OneWire EEPROM device read from memory
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \param[in,out] pData - Pointer to data for reception
** \param[in] addr - Target memory cell start address
** \param[in] len - Number of data bytes to receive
** \return FctERR - error code
**/
FctERR NONNULL__ OW_EEP_Read_Memory(OW_eep_t * const pEEP, uint8_t * pData, const uint32_t addr, const size_t len);

/*!\brief OneWire EEPROM device write to memory
** \note This function allows writing across banks for convenience.
** 		 Be aware that writing across banks includes programming wait time for each targeted bank (except for single bank or last in the sequence).
** 		 Non blocking write can be achieved by writing each bank at once manually,
** 		 after testing return value of \ref OW_EEP_WriteCycle_Handler to ensure eeprom is ready for write operation
** \note In any case, \ref OW_EEP_WriteCycle_Handler has to be called somewhere in main like loop to release one wire bus after a write operation.
** \param[in,out] pEEP - Pointer to EEPROM device type structure
** \param[in] pData - Pointer to data for transmission
** \param[in] addr - Target memory cell start address
** \param[in] len - Number of data bytes to transmit
** \return FctERR - error code
**/
FctERR NONNULL__ OW_EEP_Write_Memory(OW_eep_t * const pEEP, const uint8_t * pData, const uint32_t addr, const size_t len);


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif
/****************************************************************/
