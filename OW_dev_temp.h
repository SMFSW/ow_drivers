/*!\file OW_dev_temp.h
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire temperature sensor device type
**/
/****************************************************************/
#ifndef __OW_DEV_TEMP_H
	#define __OW_DEV_TEMP_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#include "OW_component.h"
/****************************************************************/


/**********************************/
/*** Peripheral defaults setter ***/
/**********************************/

#define OW_TEMP_SET_DEFAULTS(name, idx)										\
	name[idx].temp.slave_inst = &name##_hal[idx];							\
	memcpy(&name[idx].temp.props, &name##_props, sizeof(OW_temp_props_t));	\
	name[idx].pScratch = (name##_scratch_t *) name[idx].temp.scratch_data;	//!< Macro to set working defaults for peripheral \b name on index \b idx


#define OW_TEMP_OFFSET(name)	OW_PERIPHERAL_DEV_OFFSET(name, temp)		//!< Macro to get temp structure offset in \b name peripheral structure


// *****************************************************************************
// Section: Constants
// *****************************************************************************
#define OW_TEMP_SCRATCHPAD_SIZE	0x09		//!< Temperature sensor Scratchpad Size


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\enum OW_TEMP_cmd
** \brief Commands enum for Temperature Sensor device type
**/
typedef enum PACK__ OW_TEMP_cmd {
	OW_TEMP__CONVERT_T = 0x44,			//!< Initiates temperature conversion
	OW_TEMP__WRITE_SCRATCHPAD = 0x4E,	//!< Writes data into scratchpad bytes 2, 3, and 4 (Th, Tl, and configuration registers)
	OW_TEMP__COPY_SCRATCHPAD = 0x48,	//!< Copies Th, Tl, and configuration register data from the scratchpad to EEPROM
	OW_TEMP__READ_SCRATCHPAD = 0xBE,	//!< Reads the entire scratchpad including the CRC byte
	OW_TEMP__RECALL = 0xB8,				//!< Recalls Th, Tl, and configuration register data from EEPROM to the scratchpad
	OW_TEMP__ALARM_SEARCH = 0xEC,		//!< This command allows the master device to determine if any DS1825 experienced an alarm condition during the most recent temperature conversion
} OW_TEMP_cmd;


/*!\struct OW_temp_props_t
** \brief OneWire Temperature sensor properties type
**/
typedef struct OW_temp_props_t {
	const uint16_t *	convTimes;		//!< Conversion times array (following resolution)
	const uint8_t		maxResIdx;		//!< Maximum resolution index
	const float			granularity;	//!< Granularity
	const uint8_t		cfgBytes;		//!< Number of configuration bytes written to EEPROM
} OW_temp_props_t;


/*!\struct OW_temp_t
** \brief OneWire Temperature sensor configuration type
**/
typedef struct OW_temp_t {
	OW_slave_t *		slave_inst;		//!< Slave structure
	OW_temp_props_t		props;			//!< Temperature sensor properties
	uint8_t				scratch_data[OW_TEMP_SCRATCHPAD_SIZE];
	uint8_t				resIdx;			//!< Resolution index (for arrays)
	int16_t				tempConv;		//!< Temperature
	uint32_t			hStartConv;		//!< Conversion time start
	bool				doneConv;		//!< Conversion done status
} OW_temp_t;


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************

/*!\brief OneWire alarm search for all devices
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] ROMId - Pointer to ROM Ids array
** \param[in] max_nb - Maximum number of devices to search for (most likely number of ROMId array elements)
** \return FctERR - Error code
**/
FctERR NONNULL__ OWAlarmSearch_All(OW_DRV * const pOW, OW_ROM_ID_t ROMId[], const uint8_t max_nb);

/*!\brief OneWire alarm search for all devices
** \param[in,out] pTEMP - Pointer to Temperature device type structure
** \param[in,out] ROMId - Pointer to ROM Ids array
** \param[in] max_nb - Maximum number of devices to search for (most likely number of ROMId array elements)
** \return FctERR - Error code
**/
__INLINE FctERR NONNULL_INLINE__ OW_TEMP_AlarmSearch_All(OW_temp_t * const pTEMP, OW_ROM_ID_t ROMId[], const uint8_t max_nb) {
	OW_DRV * const pOW = pTEMP->slave_inst->cfg.bus_inst;
	return OWAlarmSearch_All(pOW, ROMId, max_nb); }


/*!\brief OneWire Temperature sensor device read scratchpad
** \param[in,out] pTEMP - Pointer to Temperature device type structure
** \return FctERR - error code
**/
FctERR NONNULL__ OW_TEMP_Read_Scratchpad(OW_temp_t * const pTEMP);

/*!\brief OneWire Temperature sensor device write scratchpad
** \param[in,out] pTEMP - Pointer to Temperature device type structure
** \return FctERR - error code
**/
FctERR NONNULL__ OW_TEMP_Write_Scratchpad(OW_temp_t * const pTEMP);


/*!\brief OneWire Temperature sensor device start temperature conversion
** \param[in,out] pTEMP - Pointer to Temperature device type structure
** \return FctERR - error code
**/
FctERR NONNULL__ OW_TEMP_Start_Conversion(OW_temp_t * const pTEMP);

/*!\brief OneWire Temperature sensor device read conversion
** \note Reads last converted value, does not launch any conversion
** \param[in,out] pTEMP - Pointer to Temperature device type structure
** \return FctERR - error code
**/
FctERR NONNULL__ OW_TEMP_Read_Conversion(OW_temp_t * const pTEMP);

/*!\brief OneWire Temperature sensor device blocking temperature conversion
** \note Blocking mode: start conversion, wait, read conversion
** \param[in,out] pTEMP - Pointer to Temperature device type structure
** \return FctERR - error code
**/
FctERR NONNULL__ OW_TEMP_Convert(OW_temp_t * const pTEMP);

/*!\brief OneWire Temperature sensor device non blocking temperature conversion
** \note Non blocking mode: start conversion, test conversion time, read conversion
** \note Handler shall be called periodically in a main like loop
** \param[in,out] pTEMP - Pointer to Temperature device type structure
** \return FctERR - error code
**/
FctERR NONNULL__ OW_TEMP_Convert_Handler(OW_temp_t * const pTEMP);


/*!\brief OneWire Temperature sensor device convert last temperature to Celsius degrees
** \param[in,out] pTEMP - Pointer to Temperature device type structure
** \return Temperature in Celsius degrees
**/
__INLINE float NONNULL__ OW_TEMP_Get_Temperature_Celsius(const OW_temp_t * const pTEMP) {
	return pTEMP->tempConv * pTEMP->props.granularity; }

/*!\brief OneWire Temperature sensor device convert last temperature to Fahrenheit degrees
** \param[in,out] pTEMP - Pointer to Temperature device type structure
** \return Temperature in Fahrenheit degrees
**/
__INLINE float NONNULL__ OW_TEMP_Get_Temperature_Fahrenheit(const OW_temp_t * const pTEMP) {
	return celsius2fahrenheit(OW_TEMP_Get_Temperature_Celsius(pTEMP)); }

/*!\brief OneWire Temperature sensor device convert last temperature to Kelvin degrees
** \param[in,out] pTEMP - Pointer to Temperature device type structure
** \return Temperature in Kelvins
**/
__INLINE float NONNULL__ OW_TEMP_Get_Temperature_Kelvin(const OW_temp_t * const pTEMP) {
	return celsius2kelvin(OW_TEMP_Get_Temperature_Celsius(pTEMP)); }


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif	/* __OW_DEV_TEMP_H */
/****************************************************************/
