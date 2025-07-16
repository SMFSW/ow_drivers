/*!\file OW_dev_sn.c
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief OneWire serial number device type (every OneWire device inheriting from serial number)
**/
/****************************************************************/
#include "sarmfsw.h"

#include "OW_drv.h"

#include "OW_dev_sn.h"
/****************************************************************/


void NONNULL__ OW_SN_Get(const OW_sn_t * const pSN, uint64_t * const pSerialNumber)
{
	*pSerialNumber = pSN->serial_number;
}
