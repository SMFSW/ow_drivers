/*!\file OW_phy.c
** \author SMFSW
** \copyright MIT (c) 2021-2026, SMFSW
** \brief OneWire physical layer
**/
/****************************************************************/
#include <string.h>

#include "sarmfsw.h"

#include "OW_drv.h"

#include "OW_phy.h"
/****************************************************************/


FctERR OWInit_phy(const uint8_t idx)
{
	FctERR			err = ERROR_OK;
	OW_DRV * const	pOW = &OWdrv[idx];

	/* Check the parameters */
	if (!IS_OW_DRV_IDX(idx))	{ err = ERROR_INSTANCE; }
	if (err != ERROR_OK)		{ goto ret; }

#if defined(HAL_SWPMI_MODULE_ENABLED)
	if (IS_SWPMI_ALL_INSTANCE(pOW->SWPMI_inst->Instance))
	{
		pOW->phy = OW_PHY_SWPMI;
		//err = OWInit_SWPMI(idx);
		err = ERROR_NOTAVAIL;
	}
#endif
#if defined(HAL_UART_MODULE_ENABLED)
	if (IS_UART_INSTANCE(pOW->phy_inst.UART_inst->Instance))
	{
		pOW->phy = OW_PHY_UART;
		err = OWInit_UART(idx);
	}
#endif
#if defined(HAL_I2C_MODULE_ENABLED)
	if (IS_I2C_ALL_INSTANCE(pOW->phy_inst.I2C_inst->Instance))
	{
		pOW->phy = OW_PHY_I2C;
		//err = OWInit_I2C(idx);
		err = ERROR_NOTAVAIL;
	}
#endif
#if defined(HAL_GPIO_MODULE_ENABLED)
	if (IS_GPIO_ALL_INSTANCE(((GPIO_HandleTypeDef *) pOW->phy_inst.inst)->GPIOx))	// cppcheck-suppress misra-c2012-11.5
	{
		pOW->phy = OW_PHY_GPIO;
		err = OWInit_GPIO(idx);
	}
#endif

	ret:
	if (err != ERROR_OK)
	{
		pOW->phy = OW_PHY_NONE;
		pOW->phy_inst.inst = NULL;
	}

	return err;
}

