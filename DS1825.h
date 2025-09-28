/*!\file DS1825.h
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief DS1825: Programmable Resolution 1-Wire Digital Thermometer With 4-Bit ID
**/
// cppcheck-suppress-begin misra-c2012-19.2
/****************************************************************/
#ifndef OW_DS1825_H__
	#define OW_DS1825_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#include "OW_component.h"
#include "OW_dev_sn.h"
#include "OW_dev_temp.h"
/****************************************************************/


#ifndef OW_DS1825_NB
//! \note Define OW_DS1825_NB_NB to enable multiple peripherals of this type
#define OW_DS1825_NB	1U	//!< Number of DS1825 peripherals
#endif


// *****************************************************************************
// Section: Constants
// *****************************************************************************
#define DS1825__GRANULARITY		0.0625f		//!< DS1825 temperature sensor granularity


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\enum _DS1825_cmd
** \brief Commands enum for DS1825
** \note Unused
**/
typedef enum PACK__ _DS1825_cmd {
	DS1825__CONVERT_T = OW_TEMP__CONVERT_T,					//!< Initiates temperature conversion
	DS1825__WRITE_SCRATCHPAD = OW_TEMP__WRITE_SCRATCHPAD,	//!< Writes data into scratchpad bytes 2, 3, and 4 (Th, Tl, and configuration registers)
	DS1825__COPY_SCRATCHPAD = OW_TEMP__COPY_SCRATCHPAD,		//!< Copies Th, Tl, and configuration register data from the scratchpad to EEPROM
	DS1825__READ_SCRATCHPAD = OW_TEMP__READ_SCRATCHPAD,		//!< Reads the entire scratchpad including the CRC byte
	DS1825__RECALL = OW_TEMP__RECALL,						//!< Recalls Th, Tl, and configuration register data from EEPROM to the scratchpad
	DS1825__ALARM_SEARCH = OW_TEMP__ALARM_SEARCH,			//!< This command allows the master device to determine if any DS1825 experienced an alarm condition during the most recent temperature conversion
} DS1825_cmd;


/*!\struct DS1825_t
** \brief DS1825 user interface struct
**/
typedef struct _DS1825_t {
	/*** device generic peripheral types structures ***/
	OW_sn_t				sn;			//!< Serial Number device type structure
	OW_temp_t			temp;		//!< Temperature Sensor device type structure
	/*** device specific variables ***/
	OW_temp_scratch_t *	pScratch;	//!< Pointer to scratchpad structure
	uint8_t				location;	//!< Device location (defined by hardware pin coding)
} DS1825_t;

extern DS1825_t DS1825[OW_DS1825_NB];	//!< DS1825 User structure


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
/*!\brief DS1825 peripheral family code getter
** \return DS1825 family code
**/
OW_ROM_type DS1825_Get_FamilyCode(void);


/******************/
/*** Slave init ***/
/******************/
/*!\brief Configuration setter callback for DS1825 peripheral
** \weak DS1825 Configuration setter sequence may be user implemented if needed
** \note Function is called at the end of \ref DS1825_Init_Sequence
** \param[in,out] pCpnt - Pointer to DS1825 component
** \return FctERR - error code
**/
FctERR NONNULL__ DS1825_Configuration_Setter_Callback(DS1825_t * const pCpnt);

/*!\brief Initialization Sequence for DS1825 peripheral
** \weak DS1825 Init sequence may be user implemented if custom initialization sequence needed
** \param[in,out] pCpnt - Pointer to DS1825 component
** \return FctERR - error code
**/
FctERR NONNULL__ DS1825_Init_Sequence(DS1825_t * const pCpnt);

/*!\brief Initialization for DS1825 peripheral
** \param[in] idx - DS1825 index
** \param[in] pOW - pointer to OneWire driver instance
** \param[in] pROM - Pointer to ROM Id structure
** \return FctERR - error code
**/
FctERR NONNULL__ DS1825_Init(const uint8_t idx, OW_DRV * const pOW, const OW_ROM_ID_t * const pROM);

/*!\brief Initialization for DS1825 peripheral
** \warning In case multiple devices (defined by OW_DS1825_NB > 1), you shall use DS1825_Init instead
** \param[in] pROM - Pointer to ROM Id structure
** \return FctERR - error code
**/
FctERR DS1825_Init_Single(const OW_ROM_ID_t * const pROM);


/*************************************/
/*** Low level access / Procedures ***/
/*************************************/

#ifndef DOXY
/*!\brief DS1825 Serial Number getter
** \param[in] pCpnt - Pointer to DS1825 peripheral
** \return DS1825 peripheral serial number
**/
#endif
OW_SN_GETTER(DS1825);


/*!\brief DS1825 set conversion resolution
** \param[in,out] pCpnt - Pointer to DS1825 peripheral
** \param[in] resolution - Conversion resolution from \ref OW_temp_res
** \return FctERR - Error code
**/
FctERR NONNULL__ DS1825_Set_Resolution(DS1825_t * const pCpnt, const OW_temp_res resolution);


/*!\brief DS1825 start temperature conversion
** \param[in,out] pCpnt - Pointer to DS1825 peripheral
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ DS1825_Start_Conversion(DS1825_t * const pCpnt) {
	return OW_TEMP_Start_Conversion(&pCpnt->temp); }

/*!\brief DS1825 read conversion
** \note Reads last converted value, does not launch any conversion
** \param[in,out] pCpnt - Pointer to DS1825 peripheral
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ DS1825_Read_Conversion(DS1825_t * const pCpnt) {
	return OW_TEMP_Read_Conversion(&pCpnt->temp); }

/*!\brief DS1825 blocking temperature conversion
** \note Blocking mode: start conversion, wait, read conversion
** \param[in,out] pCpnt - Pointer to DS1825 peripheral
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ DS1825_Convert(DS1825_t * const pCpnt) {
	return OW_TEMP_Convert(&pCpnt->temp); }

/*!\brief DS1825 non blocking temperature conversion
** \note Non blocking mode: start conversion, test conversion time, read conversion
** \note Handler shall be called periodically in a main like loop
** \param[in,out] pCpnt - Pointer to DS1825 peripheral
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ DS1825_Convert_Handler(DS1825_t * const pCpnt) {
	return OW_TEMP_Convert_Handler(&pCpnt->temp); }


/*!\brief DS1825 convert last temperature to Celsius degrees
** \param[in,out] pCpnt - Pointer to DS1825 peripheral
** \return Temperature in Celsius degrees
**/
__INLINE float NONNULL_INLINE__ DS1825_Get_Temperature_Celsius(const DS1825_t * const pCpnt) {
	return OW_TEMP_Get_Temperature_Celsius(&pCpnt->temp); }

/*!\brief DS1825 convert last temperature to Fahrenheit degrees
** \param[in,out] pCpnt - Pointer to DS1825 peripheral
** \return Temperature in Fahrenheit degrees
**/
__INLINE float NONNULL_INLINE__ DS1825_Get_Temperature_Fahrenheit(const DS1825_t * const pCpnt) {
	return OW_TEMP_Get_Temperature_Fahrenheit(&pCpnt->temp); }

/*!\brief DS1825 convert last temperature to Kelvin degrees
** \param[in,out] pCpnt - Pointer to DS1825 peripheral
** \return Temperature in Kelvins
**/
__INLINE float NONNULL_INLINE__ DS1825_Get_Temperature_Kelvin(const DS1825_t * const pCpnt) {
	return OW_TEMP_Get_Temperature_Kelvin(&pCpnt->temp); }


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif
// cppcheck-suppress-end misra-c2012-19.2
/****************************************************************/

