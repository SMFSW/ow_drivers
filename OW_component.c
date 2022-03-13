/*!\file OW_component.c
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief Base One Wire component
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_component.h"
/****************************************************************/


FctERR NONNULL__ OW_slave_init(OW_slave_t * pSlave, const OW_DRV * pOW, const OW_ROM_ID_t * const pROM)
{
	/* Check the parameters */
	//assert_param(IS_OW_ALL_INSTANCE(pOW->Instance));

	pSlave->cfg.bus_inst = (OW_DRV *) pOW;
	pSlave->en = true;
	return OW_set_slave_id(pSlave, pROM);
}


FctERR NONNULL__ OW_set_slave_instance(OW_slave_t * pSlave, const OW_DRV * pOW)
{
	/* Check the parameters */
	//assert_param(IS_OW_ALL_INSTANCE(pOW->Instance));

	pSlave->cfg.bus_inst = (OW_DRV *) pOW;
	return ERROR_OK;
}

