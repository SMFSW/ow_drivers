/*!\file OW_phy_UART.h
** \author SMFSW
** \copyright MIT (c) 2021-2024, SMFSW
** \brief OneWire UART physical layer
** \warning Assuming UART instance has already been configured once at init with following parameters:
**			Half duplex configuration
**			Baud rate: any
** 			Data Bits: 8
** 			Parity: None
** 			Stop Bits: 1
**/
/****************************************************************/
#ifndef __OW_PHY__UART_H
	#define __OW_PHY__UART_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"
/****************************************************************/
#if defined(HAL_UART_MODULE_ENABLED)
/****************************************************************/


// *****************************************************************************
// Section: Types
// *****************************************************************************


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
/*!\brief OneWire physical driver instance on UART init
** \param[in] idx - Instance index
** \return FctERR - Error code
**/
FctERR NONNULL__ OWInit_UART(const uint8_t idx);


/****************************************************************/
#endif

#ifdef __cplusplus
	}
#endif

#endif	/* __OW_PHY__UART_H */
/****************************************************************/
