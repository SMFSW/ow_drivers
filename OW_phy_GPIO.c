/*!\file OW_phy_GPIO.c
** \author SMFSW
** \copyright MIT (c) 2021-2024, SMFSW
** \brief OneWire GPIO physical layer
**/
/****************************************************************/
#include "sarmfsw.h"
#include "tick_utils.h"

#include "OW_drv.h"

#include "OW_phy_GPIO.h"
/****************************************************************/
#if defined(HAL_GPIO_MODULE_ENABLED)
/****************************************************************/


/*!\brief OneWire GPIO set as input
** \param[in,out] pGPIO - Pointer to OneWire GPIO handle
**/
__STATIC_INLINE void NONNULL_INLINE__ OW_GPIO_Input(const OW_GPIO_HandleTypeDef * const pGPIO) {
	SET_BITS_VAL(*pGPIO->reg, pGPIO->regMask, pGPIO->inputMask); }

/*!\brief OneWire GPIO set as output
** \param[in,out] pGPIO - Pointer to OneWire GPIO handle
**/
__STATIC_INLINE void NONNULL_INLINE__ OW_GPIO_Output(const OW_GPIO_HandleTypeDef * const pGPIO) {
	SET_BITS_VAL(*pGPIO->reg, pGPIO->regMask, pGPIO->outputMask); }


/*!\brief OneWire bus release
** \param[in,out] pGPIO - Pointer to OneWire GPIO handle
**/
__STATIC_INLINE void NONNULL_INLINE__ OW_GPIO_Depower(const OW_GPIO_HandleTypeDef * const pGPIO)
{
	diInterrupts();
	OW_GPIO_Input(pGPIO);
	enInterrupts();
}


/*!\brief OneWire GPIO read pin
** \param[in,out] pGPIO - Pointer to OneWire GPIO handle
** \return Pin state
** \retval true - Bit logic 1
** \retval false - Bit logic 0
**/
static bool NONNULL__ OW_GPIO_ReadPin(const OW_GPIO_HandleTypeDef * const pGPIO) {
	return binEval(pGPIO->port->IDR & pGPIO->bitMask); }


/*!\brief OneWire GPIO write logic 1
** \param[in,out] pGPIO - Pointer to OneWire GPIO handle
**/
__STATIC_INLINE void NONNULL_INLINE__ OW_GPIO_WriteHigh(const OW_GPIO_HandleTypeDef * const pGPIO)
{
	pGPIO->port->BSRR = pGPIO->bitMask;
	OW_GPIO_Output(pGPIO);
}

/*!\brief OneWire GPIO write logic 0
** \param[in,out] pGPIO - Pointer to OneWire GPIO handle
**/
__STATIC_INLINE void NONNULL_INLINE__ OW_GPIO_WriteLow(const OW_GPIO_HandleTypeDef * const pGPIO)
{
	pGPIO->port->BRR = pGPIO->bitMask;
	OW_GPIO_Output(pGPIO);
}


/*!\brief OneWire GPIO write bit to bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] bit - Bit for transmission
** \return FctERR - Error code
**/
static FctERR NONNULL__ OW_GPIO_Write_bit(const OW_DRV * const pOW, const uint8_t bit)
{
	const OW_GPIO_HandleTypeDef * const	pGPIO = pOW->phy_inst.GPIO_inst;
	uint8_t								delay[2];

	if (bit & 0x01)
	{
		delay[0] = 10;
		delay[1] = 55;
	}
	else
	{
		delay[0] = 65;
		delay[1] = 5;
	}

	/* Set line low */
	diInterrupts();
	OW_GPIO_WriteLow(pGPIO);
	Delay_us(delay[0]);

	/* Bit high */
	OW_GPIO_WriteHigh(pGPIO);
	enInterrupts();

	/* Wait correct amount of time and release the line */
	Delay_us(delay[1]);
	diInterrupts();
	OW_GPIO_Input(pGPIO);
	enInterrupts();

	return ERROR_OK;
}


#if OW_CUSTOM_BYTE_HANDLERS
/*!\brief OneWire GPIO write byte to bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] byte - Byte for transmission
** \return FctERR - Error code
**/
static FctERR NONNULL__ OW_GPIO_Write_byte(const OW_DRV * const pOW, const uint8_t byte)
{
	uint8_t data = byte;

	for (int j = 8 ; j ; j--)
	{
		OW_GPIO_Write_bit(pOW, data & 0x01);
		data >>= 1;
	}

	return ERROR_OK;
}
#endif


/*!\brief OneWire GPIO read bit from bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pBit - Pointer to bit for reception
** \return FctERR - Error code
**/
static FctERR NONNULL__ OW_GPIO_Read_bit(const OW_DRV * const pOW, uint8_t * const pBit)
{
	const OW_GPIO_HandleTypeDef * const	pGPIO = pOW->phy_inst.GPIO_inst;

	/* Line low */
	diInterrupts();
	OW_GPIO_WriteLow(pGPIO);
	Delay_us(3);

	/* Release line */
	OW_GPIO_Input(pGPIO);
	Delay_us(10);
	*pBit = OW_GPIO_ReadPin(pGPIO);

	/* Wait 47us to complete 60us period */
	enInterrupts();
	Delay_us(47);

	return ERROR_OK;
}


#if OW_CUSTOM_BYTE_HANDLERS
/*!\brief OneWire GPIO read byte from bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pData - Pointer to byte for reception
** \return FctERR - Error code
**/
FctERR NONNULL__ OW_GPIO_Read_byte(const OW_DRV * const pOW, uint8_t * const pByte)
{
	FctERR	err;
	uint8_t	bit;

	*pByte = 0;

	for (uint8_t mask = 0x01 ; mask ; mask <<= 1)
	{
		err = OW_GPIO_Read_bit(pOW, &bit);
		if (err)	{ break; }

		if (bit) 	{ *pByte |= mask; }
	}

	return err;
}
#endif


/*!\brief OneWire GPIO bus reset
** \param[in,out] pOW - Pointer to OneWire driver instance
** \return FctERR - Error code
**/
static FctERR NONNULL__ OW_GPIO_Reset(const OW_DRV * const pOW)
{
	const OW_GPIO_HandleTypeDef * const	pGPIO = pOW->phy_inst.GPIO_inst;

	OW_GPIO_Depower(pGPIO);

	// TODO: see if really needed
	// wait until the wire is high... just in case
	int retries = 125;
	do
	{
		if (--retries == 0) { return ERROR_BUSOFF; }
		Delay_us(2);
	} while (!OW_GPIO_ReadPin(pGPIO));

	/* Line low, and wait 480us */
	diInterrupts();
	OW_GPIO_WriteLow(pGPIO);
	enInterrupts();
	Delay_us(480);

	/* Release line and wait for 70us */
	diInterrupts();
	OW_GPIO_Input(pGPIO);
	Delay_us(70);

	/* Check bit value */
	const bool bit = OW_GPIO_ReadPin(pGPIO);
	enInterrupts();

	/* Delay for 410 us */
	Delay_us(410);

	return bit ? ERROR_BUSOFF : ERROR_OK;
}


FctERR OWInit_GPIO(const uint8_t idx)
{
	/* Check the parameters */
	if (!IS_OW_DRV_IDX(idx))			{ return ERROR_INSTANCE; }	// Unknown instance
	if (OWdrv[idx].phy != OW_PHY_GPIO)	{ return ERROR_INSTANCE; }	// Wrong instance type

	OW_DRV * const pOW = &OWdrv[idx];

	init_Delay_Generator();

	pOW->pfReset = OW_GPIO_Reset;
	pOW->pfWriteBit = OW_GPIO_Write_bit;
	pOW->pfReadBit = OW_GPIO_Read_bit;

	#if OW_CUSTOM_BYTE_HANDLERS
	pOW->pfWriteByte = OW_GPIO_Write_byte;
	pOW->pfReadByte = OW_GPIO_Read_byte;
	#endif


	const GPIO_HandleTypeDef * const pHandle = pOW->phy_inst.inst;

	pOW->phy_inst.GPIO_inst = &pOW->GPIO_cfg;	// Setting virtual GPIO instance pointer after backing up GPIO config handle

	pOW->phy_inst.GPIO_inst->port = pHandle->GPIOx;
	pOW->phy_inst.GPIO_inst->bitMask = pHandle->GPIO_Pin;

	// TODO: setting speed to very high may not be needed
	#ifdef GPIO_SPEED_FREQ_VERY_HIGH
	const uint32_t GPIO_Speed = GPIO_SPEED_FREQ_VERY_HIGH;	//!< OneWire GPIO speed
	#else
	const uint32_t GPIO_Speed = GPIO_SPEED_FREQ_HIGH;		//!< OneWire GPIO speed
	#endif

	// TODO: think about initializing GPIO clock (if not already configured by HAL)
	GPIO_InitTypeDef GPIO_InitStruct = {	.Pin = pOW->phy_inst.GPIO_inst->bitMask,
											.Mode = GPIO_MODE_OUTPUT_OD,
											.Pull = GPIO_NOPULL,
											.Speed = GPIO_Speed };

	HAL_GPIO_Init(pOW->phy_inst.GPIO_inst->port, &GPIO_InitStruct);

	// TODO: see about other families initialization
	#if defined(STM32F0)
		uint8_t RegShift;

		if ((GPIO_Pin & 0x00FF) > 0)
		{
			pOW->phy_inst.GPIO_inst->reg = &pOW->phy_inst.GPIO_inst->port->CRL;

			for (pinpos = 0 ; pinpos < 8 ; pinpos++)
			{
				const uint32_t pos = 1U << pinpos;

				/* Get the port pins position */
				const uint32_t currentpin = (uint16_t) (pOW->phy_inst.GPIO_inst->bitMask & pos);

				if (currentpin == pos)
				{
					RegShift = (pinpos * 4);
					pOW->phy_inst.GPIO_inst->regMask = 0x0FU << RegShift;
					break;
				}
			}
		}
		else
		{
			pOW->phy_inst.GPIO_inst->reg = &pOW->phy_inst.GPIO_inst->port->CRH;

			for (pinpos = 0 ; pinpos < 8 ; pinpos++)
			{
				const uint32_t pos = 1U << (pinpos + 8);

				/* Get the port pins position */
				const uint32_t currentpin = (uint16_t) (pOW->phy_inst.GPIO_inst->bitMask & pos);

				if (currentpin == pos)
				{
					RegShift = (pinpos * 4);
					pOW->phy_inst.GPIO_inst->regMask = 0x0FU << RegShift;
					break;
				}
			}
		}

		pOW->phy_inst.GPIO_inst->inputMask = (((GPIO_MODE_ANALOG) << RegShift) & pOW->phy_inst.GPIO_inst->regMask);
		pOW->phy_inst.GPIO_inst->outputMask = (((uint32_t) GPIO_MODE_OUTPUT_OD | (uint32_t) GPIO_SPEED_ONE_WIRE) << RegShift) & pOW->phy_inst.GPIO_inst->regMask;
	#else /*if defined(STM32F3)*/
		const uint8_t	max_pins = 16;	// Maximum pins on a port
		uint8_t			RegShift = 0;

		pOW->phy_inst.GPIO_inst->reg = &pOW->phy_inst.GPIO_inst->port->MODER;

		// Find pin shifting values to get pin index
		for (int pinpos = 0 ; pinpos < max_pins ; pinpos++)
		{
			const uint32_t pos = 1U << pinpos;
			const uint32_t currentpin = pOW->phy_inst.GPIO_inst->bitMask & pos;

			if (pos == currentpin)
			{
				RegShift = (pinpos * 2);
				pOW->phy_inst.GPIO_inst->regMask = GPIO_MODE << RegShift;
			}
		}

		pOW->phy_inst.GPIO_inst->inputMask = (MODE_INPUT << RegShift) & pOW->phy_inst.GPIO_inst->regMask;
		pOW->phy_inst.GPIO_inst->outputMask = (MODE_OUTPUT << RegShift) & pOW->phy_inst.GPIO_inst->regMask;
	#endif

	return ERROR_OK;
}

#endif
