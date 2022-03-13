/*!\file OW_dev_type.h
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire device types
**/
/****************************************************************/
#ifndef __OW_DEV_TYPE_H
	#define __OW_DEV_TYPE_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"
/****************************************************************/


/*!\enum OW_ROM_type
** \brief Shared ROM type enum for One Wire
**/
typedef enum PACK__ OW_ROM_type {							// Description: (iButton refs), device refs
	OW_TYPE__NO_DEVICE							= 0x00,		//!< No device
	/*** MAXIMINTEGRATED AIBU's software library handled ***/
	OW_TYPE__NET_ADDRESS						= 0x01,		//!< 1-Wire net address (registration number) only: (DS1990A), (DS1990R), DS2401, DS2411
	OW_TYPE__MULTIKEY							= 0x02,		//!< Multikey iButton, 1152-bit secure memory: (DS1991)
	OW_TYPE__NVRAM_4K__RTC						= 0x04,		//!< 4Kb NV RAM memory and clock, timer, alarms: (DS1994), DS2404
	OW_TYPE__SWITCH_1							= 0x05,		//!< Single addressable switch: DS2405
	OW_TYPE__NVRAM_4K							= 0x06,		//!< 4Kb NV RAM memory: (DS1993)
	OW_TYPE__NVRAM_1K							= 0x08,		//!< 1Kb NV RAM memory: (DS1992)
	OW_TYPE__EEPROM_1K							= 0x09,		//!< 1Kb EPROM memory: (DS1982), DS2502
	OW_TYPE__NVRAM_16K							= 0x0A,		//!< 16Kb NV RAM memory: (DS1995)
	OW_TYPE__EEPROM_16K							= 0x0B,		//!< 16Kb EPROM memory: (DS1985), DS2505
	OW_TYPE__NVRAM_64K							= 0x0C,		//!< 64Kb NV RAM memory: (DS1996)
	OW_TYPE__EEPROM_64K							= 0x0F,		//!< 64Kb EPROM memory: (DS1986), DS2506
	OW_TYPE__TEMPERATURE_ALARM					= 0x10,		//!< Temperature with alarm trips: (DS1920)
	OW_TYPE__EEPROM_1K__SWITCH_2				= 0x12,		//!< 1Kb EPROM memory, 2-channel addressable switch: DS2406, DS2407
	OW_TYPE__EEPROM_256__OTP_64					= 0x14,		//!< 256-bit EEPROM memory and 64-bit OTP register: (DS1971), DS2430A
	OW_TYPE__NVRAM_4K__WC_COUNT					= 0x1A,		//!< 4Kb NV RAM memory with write cycle counters: (DS1963L)
	OW_TYPE__EEPROM_4096__SWITCH_2				= 0x1C,		//!< 4096-bit EEPROM memory, 2-channel addressable switch: DS28E04-100
	OW_TYPE__NVRAM_4K__EXT_COUNT				= 0x1D,		//!< 4Kb NV RAM memory with external counters: DS2423
	OW_TYPE__COUPLER_2							= 0x1F,		//!< 2-channel addressable coupler for sub-netting: DS2409
	OW_TYPE__ADC								= 0x20,		//!< 4-channel A/D converter (ADC): DS2450
	OW_TYPE__THERMOCRON							= 0x21,		//!< Thermochron® temperature logger: (DS1921G), (DS1921H), (DS1921Z)
	OW_TYPE__EEPROM_4K							= 0x23,		//!< 4Kb EEPROM memory: (DS1973), DS2433
	OW_TYPE__RTC								= 0x24,		//!< Real-time clock (RTC): (DS1904), DS2415
	OW_TYPE__RTC_IT								= 0x27,		//!< RTC with interrupt: DS2417
	OW_TYPE__SWITCH_8							= 0x29,		//!< 8-channel addressable switch: DS2408
	OW_TYPE__POTENTIOMETER						= 0x2C,		//!< 1-channel digital potentiometer: DS2890
	OW_TYPE__EEPROM_1024						= 0x2D,		//!< 1024-bit, 1-Wire EEPROM: (DS1972), DS2431
	OW_TYPE__EEPROM_32K_PROTECTED				= 0x37,		//!< Password-protected 32KB (bytes) EEPROM: (DS1977)
	OW_TYPE__SWITCH_2							= 0x3A,		//!< 2-channel addressable switch: (DS2413)
	OW_TYPE__THERMOCRON__HYGROCHRON				= 0x41,		//!< High-capacity Thermochron (temperature) and Hygrochron™ (humidity) loggers: (DS1922L), (DS1922T), (DS1923), DS2422
	OW_TYPE__THERMOMETER__PIO					= 0x42,		//!< Programmable resolution digital thermometer with sequenced detection and PIO: DS28EA00
	OW_TYPE__EEPROM_20K							= 0x43,		//!< 20Kb 1-Wire EEPROM: DS28EC20
	/*** MAXIMINTEGRATED other family codes ***/
	OW_TYPE__BATTERY_ID__MONITOR_CHIP			= 0x1B,		//!< Battery ID / Monitor chip: DS2436
	OW_TYPE__THERMOMETER						= 0x22,		//!< Econo digital thermometer: DS1822
	OW_TYPE__BATTERY_MONITOR					= 0x26,		//!< Smart battery monitor: DS2438
	OW_TYPE__PROG_RED_THERMOMETER				= 0x28,		//!< Programmable resolution digital thermometer: DS18B20
	OW_TYPE__BATTERY_MONITOR__CHARGE_CONTROL	= 0x2E,		//!< Battery monitor and charge controller: DS2770
	OW_TYPE__HIGH_PRECISION_BATTERY_MONITOR		= 0x30,		//!< High-precision li+ battery monitor: DS2760, DS2761, DS2762
	OW_TYPE__LI_PROTECTION						= 0x31,		//!< Efficient addressable single-cell rechargeable lithium protection ic: DS2720
	OW_TYPE__EEPROM_1K_SHA1						= 0x33,		//!< 1k protected EEPROM with SHA-1: (DS1961S), DS2432
	OW_TYPE__COULOMB_COUNTER					= 0x36,		//!< High precision coulomb counter: DS2740
	OW_TYPE__BATTERY_FUEL_GAUGE					= 0x51,		//!< Multi-chemistry battery fuel gauge: DS2751
	OW_TYPE__SERIAL_ID							= 0x81,		//!< Serial ID Button: (DS1420)
	OW_TYPE__DUAL_PORT__TIME					= 0x84,		//!< Dual port and time: DS2404S
	OW_TYPE__NODE_48							= 0x89,		//!< 48 bit node address chip: (DS1982U), DS2502-E48, DS2502-UNW
	OW_TYPE__UNIQUEWARE_16K						= 0x8B,		//!< 16k add-only uniqueware: (DS1985U), DS2505-UNW
	OW_TYPE__UNIQUEWARE_64K						= 0x8F,		//!< 64k add-only uniqueware: (DS1986U), DS2506-UNW
	OW_TYPE__LCD								= 0xFF,		//!< LCD (Swart)
} OW_ROM_type;


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif
/****************************************************************/
