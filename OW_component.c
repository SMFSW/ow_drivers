/*!\file OW_component.c
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief Base One Wire component
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_component.h"
/****************************************************************/


void NONNULL__ OW_slave_init(OW_slave_t * const pSlave, OW_DRV * const pOW, const OW_ROM_ID_t * const pROM)
{
	/* Check the parameters */
	//assert_param(IS_OW_ALL_INSTANCE(pOW->Instance));

	pSlave->cfg.bus_inst = pOW;
	pSlave->en = true;
	OW_set_slave_id(pSlave, pROM);
}


FctERR NONNULL__ OW_slave_get_power_supply(OW_slave_t * const pSlave, bool * const pBusPower)
{
	OW_set_busy(pSlave, true);
	FctERR err = OWROMCmd_Control_Sequence(pSlave->cfg.bus_inst, &pSlave->cfg.ROM_ID, false);

	if (!err)
	{
		uint8_t power;
		OWWrite_byte(pSlave->cfg.bus_inst, OW__READ_POWER_SUPPLY);
		OWRead_byte(pSlave->cfg.bus_inst, &power);
		*pBusPower = nbinEval(power);
	}

	OW_set_busy(pSlave, true);

	return err;
}

