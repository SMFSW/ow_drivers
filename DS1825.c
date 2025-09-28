/*!\file DS1825.c
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief DS1825: Programmable Resolution 1-Wire Digital Thermometer With 4-Bit ID
**/
/****************************************************************/
#include "sarmfsw.h"

#include "DS1825.h"

#if defined(OW_DS1825)
/****************************************************************/
// std libs
#include <string.h>
/****************************************************************/


static OW_slave_t DS1825_hal[OW_DS1825_NB] = { 0 };						//!< DS1825 Slave structure
DS1825_t DS1825[OW_DS1825_NB] = { 0 };									//!< DS1825 User structure

static const uint16_t DS1825_convTimes[] = { 94, 188, 375, 750 };		//!< DS1825 conversion times (in ms)

static const OW_temp_props_t DS1825_temp_props = {
	DS1825_convTimes, DS1825__RES_12BIT, DS1825__GRANULARITY, 3 };		//!< DS1825 temperature sensor parameters

static const OW_ROM_type FAMILY_CODE = OW_TYPE__THERMOMETER__4BIT_ID;	//!< DS1825 family code

OW_ROM_type DS1825_Get_FamilyCode(void) {
	return FAMILY_CODE; }


/****************************************************************/


/*!\brief DS1825 read scratchpad
** \param[in,out] pCpnt - Pointer to DS1825 peripheral
** \return FctERR - error code
**/
__STATIC_INLINE float NONNULL_INLINE__ DS1825_Read_Scratchpad(DS1825_t * const pCpnt) {
	return OW_TEMP_Read_Scratchpad(&pCpnt->temp); }

/*!\brief DS1825 write scratchpad
** \param[in,out] pCpnt - Pointer to DS1825 peripheral
** \return FctERR - error code
**/
__STATIC_INLINE float NONNULL_INLINE__ DS1825_Write_Scratchpad(DS1825_t * const pCpnt) {
	return OW_TEMP_Write_Scratchpad(&pCpnt->temp); }


/****************************************************************/


__WEAK FctERR NONNULL__ DS1825_Configuration_Setter_Callback(DS1825_t * const pCpnt)
{
	UNUSED(pCpnt);

	// cppcheck-suppres misra-c2012-3.1
	/**\code
	FctERR err = ERROR_OK;

	// Set to 9bits resolution once and for all
	const DS1825_res res = DS1825__RES_9BIT;

	if (pCpnt->temp.resIdx != res)
	{
		err = DS1825_Set_Resolution(pCpnt, res);
	}
	return err;
	\endcode**/

	return ERROR_OK;
}


__WEAK FctERR NONNULL__ DS1825_Init_Sequence(DS1825_t * const pCpnt)
{
	FctERR err = DS1825_Read_Scratchpad(pCpnt);

	if (!err)
	{
		pCpnt->location = pCpnt->pScratch->configuration.Bits.ADx;
		pCpnt->temp.resIdx = pCpnt->pScratch->configuration.Bits.Rx;

		err = DS1825_Configuration_Setter_Callback(pCpnt);
	}

	return err;
}

FctERR NONNULL__ DS1825_Init(const uint8_t idx, OW_DRV * const pOW, const OW_ROM_ID_t * const pROM)
{
	FctERR err = ERROR_DEVICE_UNKNOWN;

	assert_param(IS_OW_PERIPHERAL(DS1825, idx));

	if (pROM->familyCode == FAMILY_CODE)	// Family code matches
	{
		OW_slave_init(&DS1825_hal[idx], pOW, pROM);
		OW_SN_SET_DEFAULTS(DS1825, idx, pROM);
		OW_TEMP_SET_DEFAULTS(DS1825, idx);

		err = DS1825_Init_Sequence(&DS1825[idx]);
	}

	if (err != ERROR_OK)	{ OW_set_enable(&DS1825_hal[idx], false); }

	return err;
}

FctERR DS1825_Init_Single(const OW_ROM_ID_t * const pROM) {
	return DS1825_Init(0, OW_DS1825, pROM); }


/****************************************************************/


FctERR NONNULL__ DS1825_Set_Resolution(DS1825_t * const pCpnt, const OW_temp_res resolution)
{
	FctERR err = ERROR_VALUE;

	if (resolution <= pCpnt->temp.props.maxResIdx)
	{
		err = DS1825_Read_Scratchpad(pCpnt);

		if (!err)
		{
			pCpnt->pScratch->configuration.Bits.Rx = resolution;

			err = DS1825_Write_Scratchpad(pCpnt);

			if (!err)	{ pCpnt->temp.resIdx = pCpnt->pScratch->configuration.Bits.Rx; }
		}
	}

	return err;
}


/****************************************************************/
#endif
/****************************************************************/

