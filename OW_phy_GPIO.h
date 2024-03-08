/*!\file OW_phy_GPIO.h
** \author SMFSW
** \copyright MIT (c) 2021-2024, SMFSW
** \brief OneWire GPIO physical layer
**/
/****************************************************************/
#ifndef __OW_PHY__GPIO_H
	#define __OW_PHY__GPIO_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"
/****************************************************************/
#if defined(HAL_GPIO_MODULE_ENABLED)
/****************************************************************/


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\struct OW_GPIO_HandleTypeDef
** \brief GPIO handle structure definition
**/
typedef struct OW_GPIO_HandleTypeDef {
	GPIO_TypeDef *	port;			//!< Pointer to pin port
	__IO uint32_t *	reg;			//!< Pointer to pin register
	uint32_t		regMask;		//!< Pin register mask
	uint32_t		inputMask;		//!< Set pin as input mask
	uint32_t		outputMask;		//!< Set pin as output mask
	uint32_t		bitMask;		//!< Pin bit mask
} OW_GPIO_HandleTypeDef;


/*!\struct GPIO_HandleTypeDef
** \brief GPIO configuration handle structure definition
**/
typedef struct GPIO_HandleTypeDef {
	GPIO_TypeDef *	GPIOx;			//!< Pointer to pin port
	uint16_t		GPIO_Pin;		//!< Port pin
} GPIO_HandleTypeDef;


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
/*!\brief OneWire physical driver instance on GPIO init
** \param[in] idx - Instance index
** \return FctERR - Error code
**/
FctERR OWInit_GPIO(const uint8_t idx);


/****************************************************************/
#endif

#ifdef __cplusplus
	}
#endif

#endif	/* __OW_PHY__GPIO_H */
/****************************************************************/
