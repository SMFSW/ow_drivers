/*!\file OW_component.c
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief Base One Wire component
**/
/****************************************************************/
#include "sarmfsw.h"

#include "WDG_ex.h"

#include "OW_component.h"
/****************************************************************/


/*!\brief OneWire Salve get power supply source
** \note May be useful to keep bus as busy during a copy scratchpad command or during a conversion (line should be held high, no other transaction allowed on bus)
** \warning Use only if device supports the command (meaning it can be powered by power or bus), otherwise result will be wrong and irrelevant
** \param[in] pSlave - pointer to OW slave instance
** \return FctERR - Error code
**/
__STATIC FctERR NONNULL__ OW_slave_get_power_supply(OW_slave_t * const pSlave)
{
	OW_set_busy(pSlave, true);

	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);

	if (!err)
	{
		uint8_t power;

		UNUSED_RET OWWrite_byte(pSlave->cfg.bus_inst, OW__READ_POWER_SUPPLY);
		UNUSED_RET OWRead_byte(pSlave->cfg.bus_inst, &power);

		pSlave->cfg.parasite_powered = nbinEval(power);
	}

	OW_set_busy(pSlave, false);

	return err;
}


void NONNULL__ OW_slave_init(OW_slave_t * const pSlave, OW_DRV * const pOW, const OW_ROM_ID_t * const pROM)
{
	/* Check the parameters */
	//assert_param(IS_OW_ALL_INSTANCE(pOW->Instance));

	pSlave->cfg.bus_inst = pOW;
	pSlave->cfg.mutex_id = OWInit_Get_Device_Lock_ID(pOW);
	pSlave->en = true;

	OW_set_slave_id(pSlave, pROM);
	UNUSED_RET OW_slave_get_power_supply(pSlave);
}


__WEAK void OW_Watchdog_Refresh(void)
{
	WDG_ex_refresh_IWDG();
}
