/*!\file OW_phy_GPIO.c
** \author SMFSW
** \copyright MIT (c) 2021-2026, SMFSW
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
__STATIC bool NONNULL__ OW_GPIO_ReadPin(const OW_GPIO_HandleTypeDef * const pGPIO) {
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
	pGPIO->port->BSRR = LSHIFT(pGPIO->bitMask, 16U);
	OW_GPIO_Output(pGPIO);
}


/*!\brief OneWire GPIO write bit to bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] bit - Bit for transmission
** \return FctERR - Error code
**/
__STATIC FctERR NONNULL__ OW_GPIO_Write_bit(const OW_DRV * const pOW, const uint8_t bit)
{
	const OW_GPIO_HandleTypeDef * const	pGPIO = pOW->phy_inst.GPIO_inst;
	uint8_t								delay[2];

	if (TEST_BITS_VAL(bit, 1U))
	{
		delay[0] = 10U;
		delay[1] = 55U;
	}
	else
	{
		delay[0] = 65U;
		delay[1] = 5U;
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
__STATIC FctERR NONNULL__ OW_GPIO_Write_byte(const OW_DRV * const pOW, const uint8_t byte)
{
	FctERR	err;
	uint8_t data = byte;

	for (size_t j = 8U ; j ; j--)
	{
		err = OW_GPIO_Write_bit(pOW, data & 0x01U);
		if (err != ERROR_OK)	{ break; }

		data >>= 1U;
	}

	return ERROR_OK;
}
#endif


/*!\brief OneWire GPIO read bit from bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pBit - Pointer to bit for reception
** \return FctERR - Error code
**/
__STATIC FctERR NONNULL__ OW_GPIO_Read_bit(const OW_DRV * const pOW, uint8_t * const pBit)
{
	const OW_GPIO_HandleTypeDef * const	pGPIO = pOW->phy_inst.GPIO_inst;

	/* Line low */
	diInterrupts();
	OW_GPIO_WriteLow(pGPIO);
	Delay_us(3U);

	/* Release line */
	OW_GPIO_Input(pGPIO);
	Delay_us(10U);
	*pBit = OW_GPIO_ReadPin(pGPIO);

	/* Wait 47us to complete 60us period */
	enInterrupts();
	Delay_us(47U);

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

	*pByte = 0U;

	for (uint8_t mask = 0x01U ; mask ; mask <<= 1U)
	{
		err = OW_GPIO_Read_bit(pOW, &bit);
		if (err != ERROR_OK)	{ break; }

		if (bit != 0U) 			{ *pByte |= mask; }
	}

	return err;
}
#endif


/*!\brief OneWire GPIO bus reset
** \param[in,out] pOW - Pointer to OneWire driver instance
** \return FctERR - Error code
**/
__STATIC FctERR NONNULL__ OW_GPIO_Reset(const OW_DRV * const pOW)
{
	const OW_GPIO_HandleTypeDef * const	pGPIO = pOW->phy_inst.GPIO_inst;
	FctERR								err = ERROR_BUSOFF;

	OW_GPIO_Depower(pGPIO);

	// TODO: see if really needed
	// wait until the wire is high... just in case
	uintCPU_t retries = 125;
	do
	{
		if (--retries == 0U) { goto ret; }
		Delay_us(2U);
	} while (!OW_GPIO_ReadPin(pGPIO));

	/* Line low, and wait 480us */
	diInterrupts();
	OW_GPIO_WriteLow(pGPIO);
	enInterrupts();
	Delay_us(480U);

	/* Release line and wait for 70us */
	diInterrupts();
	OW_GPIO_Input(pGPIO);
	Delay_us(70U);

	/* Check bit value */
	const bool bit = OW_GPIO_ReadPin(pGPIO);
	enInterrupts();

	/* Delay for 410 us */
	Delay_us(410U);

	if (!bit)	{ err = ERROR_OK; }

	ret:
	return err;
}


FctERR OWInit_GPIO(const uint8_t idx)
{
	FctERR err;

	/* Check the parameters */
	if (!IS_OW_DRV_IDX(idx))				{ err = ERROR_INSTANCE; }	// Unknown instance
	else if (OWdrv[idx].phy != OW_PHY_GPIO)	{ err = ERROR_INSTANCE; }	// Wrong instance type
	else									{ err = ERROR_OK; }
	if (err != ERROR_OK)					{ goto ret; }

	OW_DRV * const	pOW = &OWdrv[idx];
	uintCPU_t		RegShift = 0U;

	err = init_Delay_Generator();

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
		if ((GPIO_Pin & 0x00FFU) > 0U)
		{
			pOW->phy_inst.GPIO_inst->reg = &pOW->phy_inst.GPIO_inst->port->CRL;

			for (uintCPU_t pinpos = 0U ; pinpos < 8U ; pinpos++)
			{
				const uint32_t pos = 1UL << pinpos;

				/* Get the port pins position */
				const uint32_t currentpin = (uint16_t) (pOW->phy_inst.GPIO_inst->bitMask & pos);

				if (currentpin == pos)
				{
					RegShift = (pinpos * 4U);
					pOW->phy_inst.GPIO_inst->regMask = 0x0FU << RegShift;
					break;
				}
			}
		}
		else
		{
			pOW->phy_inst.GPIO_inst->reg = &pOW->phy_inst.GPIO_inst->port->CRH;

			for (uintCPU_t pinpos = 0U ; pinpos < 8U ; pinpos++)
			{
				const uint32_t pos = 1UL << (pinpos + 8U);

				/* Get the port pins position */
				const uint32_t currentpin = (uint16_t) (pOW->phy_inst.GPIO_inst->bitMask & pos);

				if (currentpin == pos)
				{
					RegShift = (pinpos * 4U);
					pOW->phy_inst.GPIO_inst->regMask = 0x0FU << RegShift;
					break;
				}
			}
		}

		pOW->phy_inst.GPIO_inst->inputMask = (((GPIO_MODE_ANALOG) << RegShift) & pOW->phy_inst.GPIO_inst->regMask);
		pOW->phy_inst.GPIO_inst->outputMask = (((uint32_t) GPIO_MODE_OUTPUT_OD | (uint32_t) GPIO_SPEED_ONE_WIRE) << RegShift) & pOW->phy_inst.GPIO_inst->regMask;
	#else /*if defined(STM32F3)*/
		const uintCPU_t	max_pins = 16U;	// Maximum pins on a port

		pOW->phy_inst.GPIO_inst->reg = &pOW->phy_inst.GPIO_inst->port->MODER;

		// Find pin shifting values to get pin index
		for (uintCPU_t pinpos = 0U ; pinpos < max_pins ; pinpos++)
		{
			const uint32_t pos = 1UL << pinpos;
			const uint32_t currentpin = pOW->phy_inst.GPIO_inst->bitMask & pos;

			if (pos == currentpin)
			{
				RegShift = (pinpos * 2U);
				pOW->phy_inst.GPIO_inst->regMask = GPIO_MODE << RegShift;
			}
		}

		pOW->phy_inst.GPIO_inst->inputMask = (MODE_INPUT << RegShift) & pOW->phy_inst.GPIO_inst->regMask;
		pOW->phy_inst.GPIO_inst->outputMask = (MODE_OUTPUT << RegShift) & pOW->phy_inst.GPIO_inst->regMask;
	#endif

	ret:
	return err;
}

#endif
