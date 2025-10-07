/*!\file OW_phy.h
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief OneWire physical layer
** \note Needed symbols may be defined at project level. If globals.h is not used, symbol NO_GLOBALS_HEADER needs to be defined.
**/
/****************************************************************/
#ifndef OW_PHY_H__
	#define OW_PHY_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#ifndef NO_GLOBALS_HEADER
#include "globals.h"
#endif

//#include "OW_phy_SWPM.h"
#include "OW_phy_UART.h"
//#include "OW_phy_I2C.h"
#include "OW_phy_GPIO.h"
/****************************************************************/


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\enum _OWPhy
** \brief OW driver peripheral type
**/
typedef enum _OWPhy {
	OW_PHY_NONE = 0U,	//!< OW without peripheral
	OW_PHY_SWPM,		//!< OW physical peripheral
	OW_PHY_UART,		//!< OW UART physical peripheral
	OW_PHY_I2C,			//!< OW I2C physical peripheral
	OW_PHY_GPIO,		//!< OW GPIO emulated peripheral
	//CAN_PHY_MAX		//!< Max physical peripheral
} OWPhy;


typedef void	OW_Handle_t;	//!< OW Instance for any type of physical driver (SWPM / UART / ...), to be used as pointer of OW_Handle_t

typedef union _OW_phy_u {
#if defined(HAL_SWPMI_MODULE_ENABLED)
	SWPMI_HandleTypeDef *	SWPMI_inst;		//!< Instance for driver (explicitly SWPMI)
#endif
#if defined(HAL_UART_MODULE_ENABLED)
	UART_HandleTypeDef *	UART_inst;		//!< Instance for driver (explicitly UART)
#endif
#if defined(HAL_I2C_MODULE_ENABLED)
	I2C_HandleTypeDef *		I2C_inst;		//!< Instance for driver (explicitly I2C)
#endif
#if defined(HAL_GPIO_MODULE_ENABLED)
	OW_GPIO_HandleTypeDef *	GPIO_inst;		//!< Instance for driver (explicitly GPIO)
#endif
	OW_Handle_t *			inst;			//!< Instance for driver (any type)
} OW_phy_u;


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
/*!\brief OneWire physical driver instance init
** \param[in] idx - Instance index
** \return FctERR - Error code
**/
FctERR OWInit_phy(const uint8_t idx);


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif
/****************************************************************/
