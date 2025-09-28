/*!\file MAX31826.h
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief MAX31826:  Digital Temperature Sensor with 1Kb Lockable EEPROM
**/
// cppcheck-suppress-begin misra-c2012-19.2
/****************************************************************/
#ifndef OW_MAX31826_H__
	#define OW_MAX31826_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#include "OW_component.h"
#include "OW_dev_sn.h"
#include "OW_dev_temp.h"
#include "OW_dev_eeprom.h"
/****************************************************************/


#ifndef OW_MAX31826_NB
//! \note Define OW_MAX31826_NB_NB to enable multiple peripherals of this type
#define OW_MAX31826_NB	1U	//!< Number of MAX31826 peripherals
#endif


// *****************************************************************************
// Section: Constants
// *****************************************************************************
#define MAX31826__GRANULARITY		0.0625f		//!< MAX31826 temperature sensor granularity

#define MAX31826_PAGES				16U										//!< MAX31826 Number of Pages
#define MAX31826_PAGE_SIZE			8U										//!< MAX31826 Page Size
#define MAX31826_SCRATCHPAD_SIZE	8U										//!< MAX31826 Scratchpad Size
#define MAX31826_MEMORY_SIZE		(MAX31826_PAGE_SIZE * MAX31826_PAGES)	//!< MAX31826 Maximum size
#define MAX31826_MAX_WRITE_ADDR		MAX31826__LOCK_EEP_HIGH					//!< MAX31826 Maximum write address
#define MAX31826_MAX_READ_ADDR		(MAX31826_MEMORY_SIZE + 2U)				//!< MAX31826 Maximum read address
#define MAX31826_COPY_TIME			25U										//!< MAX31826 Time to copy scratchpad to EEPROM


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\enum _MAX31826_reg_map
** \brief Register map enum of MAX31826
**/
typedef enum PACK__ _MAX31826_reg_map {
	MAX31826__ADDR_PAGE_0 = 0U,			//!< User memory Page 0
	MAX31826__ADDR_PAGE_1 = 0x08U,		//!< User memory Page 1
	MAX31826__ADDR_PAGE_2 = 0x10U,		//!< User memory Page 2
	MAX31826__ADDR_PAGE_3 = 0x18U,		//!< User memory Page 3
	MAX31826__ADDR_PAGE_4 = 0x20U,		//!< User memory Page 4
	MAX31826__ADDR_PAGE_5 = 0x28U,		//!< User memory Page 5
	MAX31826__ADDR_PAGE_6 = 0x30U,		//!< User memory Page 6
	MAX31826__ADDR_PAGE_7 = 0x38U,		//!< User memory Page 7
	MAX31826__ADDR_PAGE_8 = 0x40U,		//!< User memory Page 8
	MAX31826__ADDR_PAGE_9 = 0x48U,		//!< User memory Page 9
	MAX31826__ADDR_PAGE_10 = 0x50U,		//!< User memory Page 10
	MAX31826__ADDR_PAGE_11 = 0x58U,		//!< User memory Page 11
	MAX31826__ADDR_PAGE_12 = 0x60U,		//!< User memory Page 12
	MAX31826__ADDR_PAGE_13 = 0x68U,		//!< User memory Page 13
	MAX31826__ADDR_PAGE_14 = 0x70U,		//!< User memory Page 14
	MAX31826__ADDR_PAGE_15 = 0x78U,		//!< User memory Page 15
	MAX31826__LOCK_EEP_LOW = 0x80U,		//!< Lock EEPROM Low Memory (bytes 00h–3Fh)
	MAX31826__LOCK_EEP_HIGH = 0x81U,	//!< Lock EEPROM High Memory (bytes 40h–7Fh)
} MAX31826_reg;

/*!\enum _MAX31826_cmd
** \brief Commands enum for MAX31826
** \note Unused
**/
typedef enum PACK__ _MAX31826_cmd {
	MAX31826__TEMP_CONVERT_T = OW_TEMP__CONVERT_T,					//!< Initiates temperature conversion
	MAX31826__TEMP_WRITE_SCRATCHPAD = OW_TEMP__WRITE_SCRATCHPAD,	//!< Write data into scratchpad 1 (temperature sensor scratchpad)
	MAX31826__TEMP_READ_SCRATCHPAD = OW_TEMP__READ_SCRATCHPAD,		//!< Read entire scratchpad 1 including the CRC byte (temperature sensor scratchpad)
	MAX31826__EEP_WRITE_SCRATCHPAD = OW_EEP__WRITE_SCRATCHPAD,		//!< Write data into scratchpad 2 (EEPROM scratchpad)
	MAX31826__EEP_COPY_SCRATCHPAD = OW_EEP__COPY_SCRATCHPAD,		//!< Copy data from scratchpad 2 (EEPROM scratchpad) to EEPROM
	MAX31826__EEP_READ_SCRATCHPAD = OW_EEP__READ_SCRATCHPAD,		//!< Read entire scratchpad 2 including the CRC byte (EEPROM scratchpad)
	MAX31826__EEP_READ_MEMORY = OW_EEP__READ_MEMORY					//!< Read EEPROM memory command
} MAX31826_cmd;


/*!\enum _MAX31826_res
** \brief Resolutions enum for MAX31826
**/
typedef enum PACK__ _MAX31826_res {
	MAX31826__RES_12BIT = 0U,	//!< MAX31826 12b resolution
} MAX31826_res;

/*!\enum _MAX31826_eep_area
** \brief EEP areas enum for MAX31826
**/
typedef enum PACK__ _MAX31826_eep_area {
	MAX31826__EEP_LOW = 0U,	//!< MAX31826 low eeprom area (0h-3Fh)
	MAX31826__EEP_HIGH		//!< MAX31826 high eeprom area (40h-7Fh)
} MAX31826_eep_area;


/*!\union uMAX31826_REG__CFG
** \brief Union for configuration register of MAX31826
**/
typedef union PACK__ _uMAX31826_REG__CFG {
	uint8_t Byte;
	struct PACK__ {
		uint8_t			ADx	:4;	//!< Location information
		uint8_t				:1;
		MAX31826_res	Rx	:2;	//!< Resolution
		uint8_t				:1;
	} Bits;
} uMAX31826_REG__CFG;


/*!\struct MAX31826_t
** \brief MAX31826 user interface struct
**/
typedef struct _MAX31826_t {
	/*** device generic peripheral types structures ***/
	OW_sn_t					sn;			//!< Serial Number device type structure
	OW_temp_t				temp;		//!< Temperature Sensor device type structure
	OW_eep_t				eep;		//!< EEPROM device type structure
	/*** device specific variables ***/
	OW_eep_scratch_t		scratch;								//!< Scratchpad structure (EEPROM)
	uint8_t					scratch_data[MAX31826_SCRATCHPAD_SIZE];	//!< Scratchpad data array (EEPROM)
	OW_temp_scratch_t *		pScratch;	//!< Pointer to scratchpad structure (temperature sensor)
	uint8_t					location;	//!< Device location (defined by hardware pin coding)
} MAX31826_t;

extern MAX31826_t MAX31826[OW_MAX31826_NB];	//!< MAX31826 User structure


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
/*!\brief MAX31826 peripheral family code getter
** \return MAX31826 family code
**/
OW_ROM_type MAX31826_Get_FamilyCode(void);


/******************/
/*** Slave init ***/
/******************/
/*!\brief Initialization Sequence for MAX31826 peripheral
** \weak MAX31826 Init sequence may be user implemented if custom initialization sequence needed
** \param[in,out] pCpnt - Pointer to MAX31826 component
** \return FctERR - error code
**/
FctERR NONNULL__ MAX31826_Init_Sequence(MAX31826_t * const pCpnt);

/*!\brief Initialization for MAX31826 peripheral
** \param[in] idx - MAX31826 index
** \param[in] pOW - pointer to OneWire driver instance
** \param[in] pROM - Pointer to ROM Id structure
** \return FctERR - error code
**/
FctERR NONNULL__ MAX31826_Init(const uint8_t idx, OW_DRV * const pOW, const OW_ROM_ID_t * const pROM);

/*!\brief Initialization for MAX31826 peripheral
** \warning In case multiple devices (defined by OW_MAX31826_NB > 1), you shall use MAX31826_Init instead
** \param[in] pROM - Pointer to ROM Id structure
** \return FctERR - error code
**/
FctERR MAX31826_Init_Single(const OW_ROM_ID_t * const pROM);


/*************************************/
/*** Low level access / Procedures ***/
/*************************************/

#ifndef DOXY
/*!\brief MAX31826 Serial Number getter
** \param[in] pCpnt - Pointer to MAX31826 peripheral
** \return MAX31826 peripheral serial number
**/
#endif
OW_SN_GETTER(MAX31826);


/*!\brief MAX31826 EEPROM device write cycle time handler
** \note Non blocking mode: start copy, test copy time, release bus
** \note Handler shall be called periodically in a main like loop
** \param[in,out] pCpnt - Pointer to MAX31826 component
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ MAX31826_WriteCycle_Handler(MAX31826_t * const pCpnt) {
	return OW_EEP_WriteCycle_Handler(&pCpnt->eep); }


/**************************/
/*** Temperature sensor ***/
/**************************/

/*!\brief MAX31826 start temperature conversion
** \param[in,out] pCpnt - Pointer to MAX31826 peripheral
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ MAX31826_Start_Conversion(MAX31826_t * const pCpnt) {
	return OW_TEMP_Start_Conversion(&pCpnt->temp); }

/*!\brief MAX31826 read conversion
** \note Reads last converted value, does not launch any conversion
** \param[in,out] pCpnt - Pointer to MAX31826 peripheral
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ MAX31826_Read_Conversion(MAX31826_t * const pCpnt) {
	return OW_TEMP_Read_Conversion(&pCpnt->temp); }

/*!\brief MAX31826 blocking temperature conversion
** \note Blocking mode: start conversion, wait, read conversion
** \param[in,out] pCpnt - Pointer to MAX31826 peripheral
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ MAX31826_Convert(MAX31826_t * const pCpnt) {
	return OW_TEMP_Convert(&pCpnt->temp); }

/*!\brief MAX31826 non blocking temperature conversion
** \note Non blocking mode: start conversion, test conversion time, read conversion
** \note Handler shall be called periodically in a main like loop
** \param[in,out] pCpnt - Pointer to MAX31826 peripheral
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ MAX31826_Convert_Handler(MAX31826_t * const pCpnt) {
	return OW_TEMP_Convert_Handler(&pCpnt->temp); }


/*!\brief MAX31826 convert last temperature to Celsius degrees
** \param[in,out] pCpnt - Pointer to MAX31826 peripheral
** \return Temperature in Celsius degrees
**/
__INLINE float NONNULL_INLINE__ MAX31826_Get_Temperature_Celsius(const MAX31826_t * const pCpnt) {
	return OW_TEMP_Get_Temperature_Celsius(&pCpnt->temp); }

/*!\brief MAX31826 convert last temperature to Fahrenheit degrees
** \param[in,out] pCpnt - Pointer to MAX31826 peripheral
** \return Temperature in Fahrenheit degrees
**/
__INLINE float NONNULL_INLINE__ MAX31826_Get_Temperature_Fahrenheit(const MAX31826_t * const pCpnt) {
	return OW_TEMP_Get_Temperature_Fahrenheit(&pCpnt->temp); }

/*!\brief MAX31826 convert last temperature to Kelvin degrees
** \param[in,out] pCpnt - Pointer to MAX31826 peripheral
** \return Temperature in Kelvins
**/
__INLINE float NONNULL_INLINE__ MAX31826_Get_Temperature_Kelvin(const MAX31826_t * const pCpnt) {
	return OW_TEMP_Get_Temperature_Kelvin(&pCpnt->temp); }


/**************/
/*** EEPROM ***/
/**************/
/*!\brief MAX31826 read from memory
** \param[in,out] pCpnt - Pointer to MAX31826 component
** \param[in,out] pData - Pointer to data for reception
** \param[in] addr - Target memory cell start address
** \param[in] len - Number of data bytes to receive
** \return FctERR - error code
**/
FctERR NONNULL__ MAX31826_Read_Memory(MAX31826_t * const pCpnt, uint8_t * pData, const uint32_t addr, const size_t len);

/*!\brief MAX31826 write to memory
** \note This function allows writing across banks for convenience.
** 		 Be aware that writing across banks includes programming wait time for each targeted bank (except for single bank or last in the sequence).
** 		 Non blocking write can be achieved by writing each bank at once manually,
** 		 after testing return value of \ref MAX31826_WriteCycle_Handler to ensure eeprom is ready for write operation
** \note In any case, \ref MAX31826_WriteCycle_Handler has to be called somewhere in main like loop to release one wire bus after a write operation.
** \param[in,out] pCpnt - Pointer to MAX31826 component
** \param[in] pData - Pointer to data for transmission
** \param[in] addr - Target memory cell start address
** \param[in] len - Number of data bytes to transmit
** \return FctERR - error code
**/
FctERR NONNULL__ MAX31826_Write_Memory(MAX31826_t * const pCpnt, const uint8_t * pData, const uint32_t addr, const size_t len);

/*!\brief MAX31826 lock memory
** \param[in,out] pCpnt - Pointer to MAX31826 component
** \param[in] area - EEPROM area to lock
** \return FctERR - error code
**/
FctERR NONNULL__ MAX31826_Lock_Memory(MAX31826_t * const pCpnt, const MAX31826_eep_area area);


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif
// cppcheck-suppress-end misra-c2012-19.2
/****************************************************************/

