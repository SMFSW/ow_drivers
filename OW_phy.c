/*!\file OW_phy.c
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
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
	/* Check the parameters */
	if (!IS_OW_DRV_IDX(idx))	{ return ERROR_INSTANCE; }

	OW_DRV * const pOW = &OWdrv[idx];

#if defined(HAL_SWPMI_MODULE_ENABLED)
	if (IS_SWPMI_ALL_INSTANCE(pOW->SWPMI_inst->Instance))
	{
		pOW->phy = OW_PHY_SWPMI;
		//return OWInit_SWPMI(idx);
	}
#endif
#if defined(HAL_UART_MODULE_ENABLED)
	if (IS_UART_INSTANCE(pOW->phy_inst.UART_inst->Instance))
	{
		pOW->phy = OW_PHY_UART;
		return OWInit_UART(idx);
	}
#endif
#if defined(HAL_I2C_MODULE_ENABLED)
	if (IS_I2C_ALL_INSTANCE(pOW->phy_inst.I2C_inst->Instance))
	{
		pOW->phy = OW_PHY_I2C;
		//return OWInit_I2C(idx);
	}
#endif
#if defined(HAL_GPIO_MODULE_ENABLED)
	if (IS_GPIO_ALL_INSTANCE(((GPIO_HandleTypeDef *) pOW->phy_inst.inst)->GPIOx))
	{
		pOW->phy = OW_PHY_GPIO;
		return OWInit_GPIO(idx);
	}
#endif

	pOW->phy = OW_PHY_NONE;
	pOW->phy_inst.inst = NULL;

	return ERROR_INSTANCE;
}

