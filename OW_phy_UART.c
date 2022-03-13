/*!\file OW_phy_UART.c
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief OneWire UART physical layer
** \warning Assuming UART instance has already been configured once at init with following parameters:
**			Half duplex configuration
**			Baud rate: any
** 			Data Bits: 8
** 			Parity: None
** 			Stop Bits: 1
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_drv.h"

#include "OW_phy_UART.h"
/****************************************************************/
#if defined(HAL_UART_MODULE_ENABLED)
/****************************************************************/


/*!\brief OneWire UART baud rate setup
** \param[in,out] pUART - Pointer to UART handle
** \param[in] br - Baud rate
** \return FctERR - Error code
**/
static FctERR OW_UART_Set_BR(UART_HandleTypeDef * const pUART, const uint32_t br)
{
	FctERR err = ERROR_OK;

	if (pUART->Init.BaudRate != br)
	{
		pUART->Init.BaudRate = br;
		err = HALERRtoFCTERR(UART_SetConfig(pUART));
	}

	return err;
}


/*!\brief OneWire UART write bit to bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] bit - Bit for transmission
** \return FctERR - Error code
**/
static FctERR NONNULL__ OW_UART_Write_bit(const OW_DRV * const pOW, const uint8_t bit)
{
	const uint32_t	br = 115200;
	const uint32_t	timeout = 2;

	const uint8_t	tx_low = 0x00;
	const uint8_t	tx_high = 0xFF;

	FctERR err = OW_UART_Set_BR(pOW->phy_inst.UART_inst, br);
	if (err)	{ return err; }

	err = HALERRtoFCTERR(HAL_UART_Transmit(pOW->phy_inst.UART_inst, (uint8_t *) ((bit & 0x01) ? &tx_high : &tx_low), 1, timeout));
	if (err)	{ return err; }

	uint8_t rx;
	err = HALERRtoFCTERR(HAL_UART_Receive(pOW->phy_inst.UART_inst, &rx, 1, timeout));
	if (err)	{ return err; }

	if ((bit & 0x01) != ((rx < tx_high) ? 0 : 1))	{ err = ERROR_VALUE; }

	return err;
}


#if OW_CUSTOM_BYTE_HANDLERS
/*!\brief OneWire UART write byte to bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] byte - Byte for transmission
** \return FctERR - Error code
**/
static FctERR NONNULL__ OW_UART_Write_byte(const OW_DRV * const pOW, const uint8_t byte)
{
	FctERR	err;
	uint8_t data = byte;

	for (int j = 8 ; j ; j--)
	{
		err = OW_UART_Write_bit(pOW, data & 0x01);
		if (err)	{ break; }

		data >>= 1;
	}

	return err;
}
#endif


/*!\brief OneWire UART read bit from bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pBit - Pointer to bit for reception
** \return FctERR - Error code
**/
static FctERR NONNULL__ OW_UART_Read_bit(const OW_DRV * const pOW, uint8_t * const pBit)
{
	const uint32_t	br = 115200;
	const uint32_t	timeout = 2;

	const uint8_t	tx_tick = 0xFF;
	const uint8_t	rx_high = 0xFF;

	FctERR err = OW_UART_Set_BR(pOW->phy_inst.UART_inst, br);
	if (err)	{ return err; }

	err = HALERRtoFCTERR(HAL_UART_Transmit(pOW->phy_inst.UART_inst, (uint8_t *) &tx_tick, 1, timeout));
	if (err)	{ return err; }

	uint8_t rx;
	err = HALERRtoFCTERR(HAL_UART_Receive(pOW->phy_inst.UART_inst, &rx, 1, timeout));
	if (err)	{ return err; }

	*pBit = (rx < rx_high) ? 0 : 1;

	return err;
}


#if OW_CUSTOM_BYTE_HANDLERS
/*!\brief OneWire UART read byte from bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pData - Pointer to byte for reception
** \return FctERR - Error code
**/
FctERR NONNULL__ OW_UART_Read_byte(const OW_DRV * const pOW, uint8_t * const pByte)
{
	FctERR	err;
	uint8_t	bit;

	*pByte = 0;

	for (uint8_t mask = 0x01 ; mask ; mask <<= 1)
	{
		err = OW_UART_Read_bit(pOW, &bit);
		if (err)	{ break; }

		if (bit) 	{ *pByte |= mask; }
	}

	return err;
}
#endif


/*!\brief OneWire UART bus reset
** \param[in,out] pOW - Pointer to OneWire driver instance
** \return FctERR - Error code
**/
static FctERR NONNULL__ OW_UART_Reset(const OW_DRV * const pOW)
{
	const uint32_t	br = 9600;
	const uint32_t	timeout = 2;

	const uint8_t	rst_pulse = 0xF0;
	//const uint8_t	detect_low = 0x10;
	const uint8_t	detect_high = 0x90;

	FctERR err = OW_UART_Set_BR(pOW->phy_inst.UART_inst, br);
	if (err)	{ return err; }

	err = HALERRtoFCTERR(HAL_UART_Transmit(pOW->phy_inst.UART_inst, (uint8_t *) &rst_pulse, 1, timeout));
	if (err)	{ return err; }

	uint8_t presence = 0;
	err = HALERRtoFCTERR(HAL_UART_Receive(pOW->phy_inst.UART_inst, &presence, 1, timeout));
	if (err)	{ return err; }	// TODO: in case of timeout, there may be no need of this as presence is set to 0

	//return inRange(presence, detect_low, detect_high) ? ERROR_OK : ERROR_BUSOFF;
	return (presence >= detect_high) ? ERROR_OK : ERROR_BUSOFF;
}


FctERR OWInit_UART(const uint8_t idx)
{
	/* Check the parameters */
	if (!IS_OW_DRV_IDX(idx))			{ return ERROR_INSTANCE; }	// Unknown instance
	if (OWdrv[idx].phy != OW_PHY_UART)	{ return ERROR_INSTANCE; }	// Wrong instance type

	OW_DRV * const pOW = &OWdrv[idx];

	pOW->pfReset = OW_UART_Reset;
	pOW->pfWriteBit = OW_UART_Write_bit;
	pOW->pfReadBit = OW_UART_Read_bit;

	#if OW_CUSTOM_BYTE_HANDLERS
	pOW->pfWriteByte = OW_UART_Write_byte;
	pOW->pfReadByte = OW_UART_Read_byte;
	#endif

	return ERROR_OK;
}

#endif
