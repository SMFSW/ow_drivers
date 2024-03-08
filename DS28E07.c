/*!\file DS28E07.c
** \author SMFSW
** \copyright MIT (c) 2021-2024, SMFSW
** \brief DS28E07: 1024-Bit, 1-Wire EEPROM
** \note Alternate PNs: (DS1972), DS2431
**/
/****************************************************************/
#include "sarmfsw.h"

#include "DS28E07.h"

#if defined(OW_DS28E07)
/****************************************************************/
// std libs
#include <string.h>
/****************************************************************/


static OW_slave_t DS28E07_hal[OW_DS28E07_NB] = { 0 };			//!< DS28E07 Slave structure
DS28E07_t DS28E07[OW_DS28E07_NB] = { 0 };						//!< DS28E07 User structure

static const OW_eep_props_t DS28E07_props = { DS28E07_SCRATCHPAD_SIZE, DS28E07_MEMORY_SIZE, DS28E07_PAGE_SIZE, DS28E07_PAGES, DS28E07_MAX_WRITE_ADDR, DS28E07_MAX_READ_ADDR };	//!< DS28E07 eeprom parameters
static const OW_ROM_type FAMILY_CODE = OW_TYPE__EEPROM_1024;	//!< DS28E07 family code

OW_ROM_type DS28E07_Get_FamilyCode(void) {
	return FAMILY_CODE; }


/****************************************************************/


__WEAK FctERR NONNULL__ DS28E07_Init_Sequence(DS28E07_t * const pCpnt)
{
	return DS28E07_Read_AdminData(pCpnt);
}

FctERR NONNULL__ DS28E07_Init(const uint8_t idx, OW_DRV * const pOW, const OW_ROM_ID_t * const pROM)
{
	assert_param(IS_OW_PERIPHERAL(DS28E07, idx));

	if (pROM->familyCode != FAMILY_CODE)	{ return ERROR_COMMON; }	// Family code doesn't match

	OW_SN_SET_DEFAULTS(DS28E07, idx, pROM);
	OW_EEPROM_SET_DEFAULTS(DS28E07, idx);

	FctERR err = OW_slave_init(&DS28E07_hal[idx], pOW, pROM);
	if (!err)	{ err = DS28E07_Init_Sequence(&DS28E07[idx]); }

	if (err)	{ OW_set_enable(&DS28E07_hal[idx], false); }

	return err;
}

FctERR DS28E07_Init_Single(const OW_ROM_ID_t * const pROM) {
	return DS28E07_Init(0, OW_DS28E07, pROM); }



/****************************************************************/


FctERR NONNULL__ DS28E07_Read_AdminData(DS28E07_t * const pCpnt) {
	return DS28E07_Read_Memory(pCpnt, pCpnt->admin.admin_data, DS28E07__ADDR_PROT_0, sizeof(pCpnt->admin.admin_data)); }


static FctERR NONNULL__ DS28E07_Write_AdminData(DS28E07_t * const pCpnt) {
	return DS28E07_Write_Memory(pCpnt, pCpnt->admin.admin_data, DS28E07__ADDR_PROT_0, sizeof(pCpnt->admin.admin_data)); }


FctERR NONNULL__ DS28E07_Get_Protect_Page(DS28E07_t * const pCpnt, DS28E07_prot_page * const pProt, const OW_eep_pages page)
{
	if (page >= DS28E07_PAGES)	{ return ERROR_VALUE; }

	const FctERR err = DS28E07_Read_AdminData(pCpnt);

	if (!err)	{ *pProt = pCpnt->admin.admin_data[page]; }

	return err;
}


FctERR NONNULL__ DS28E07_Get_Protect_Copy(DS28E07_t * const pCpnt, DS28E07_prot_copy * const pProt)
{
	const FctERR err = DS28E07_Read_AdminData(pCpnt);

	if (!err)	{ *pProt = pCpnt->admin.protect_copy; }

	return err;
}


FctERR NONNULL__ DS28E07_Get_Protect_UserBytes(DS28E07_t * const pCpnt, DS28E07_prot_user * const pProt)
{
	const FctERR err = DS28E07_Read_AdminData(pCpnt);

	if (!err)	{ *pProt = pCpnt->admin.protect_user; }

	return err;
}


FctERR NONNULL__ DS28E07_Protect_Page(DS28E07_t * const pCpnt, const DS28E07_prot_page prot, const OW_eep_pages page)
{
	OW_eep_pages	p = 0, p_max = 0;
	FctERR			err = DS28E07_Read_AdminData(pCpnt), err_page = ERROR_OK;

	if (!err)
	{
		if (	(pCpnt->admin.protect_copy == DS28E07__COPY_WRITE_PROTECT_1)									// Administrative data protected
			||	(pCpnt->admin.protect_copy == DS28E07__COPY_WRITE_PROTECT_2))	{ err = ERROR_PROTECT; }		// Administrative data protected
		else if ((page >= DS28E07_PAGES) && (page != OW_EEP__PAGE_ALL))			{ err = ERROR_VALUE; }			// Unknown page(s)
		else if (page == OW_EEP__PAGE_ALL)										{ p_max = DS28E07_PAGES; }
		else																	{ p = page; p_max = page + 1; }

		while (p++ < p_max)
		{
			// Protection bytes are not exactly OTP, they are locked once 0xAA or 0x55 is written in register,
			// Any other value written value will not lock the register and associated page

			if ((pCpnt->admin.admin_data[p] == DS28E07__PAGE_WRITE_PROTECT) || (pCpnt->admin.admin_data[p] == DS28E07__PAGE_EEPROM_MODE))
			{
				err_page |= ERROR_PROTECT;				// Current page already set to protected mode
				continue;
			}
			else if (prot == DS28E07__PAGE_EEPROM_MODE)	// If eeprom mode, write 0xFF on the whole page prior to set as eeprom mode
			{
				const uint8_t data[DS28E07_PAGE_SIZE] = { 0xFF };
				err |= DS28E07_Write_Memory(pCpnt, data, DS28E07_PAGE_SIZE * p, sizeof(data));
				if (err)	{ break; }
			}

			pCpnt->admin.admin_data[p] = prot;
		}

		if (!err)
		{
			err = DS28E07_Write_AdminData(pCpnt);
		}
	}

	return (err | err_page);
}


FctERR NONNULL__ DS28E07_Protect_Copy(DS28E07_t * const pCpnt, const DS28E07_prot_copy prot)
{
	FctERR err = DS28E07_Read_AdminData(pCpnt);

	if (!err)
	{
		if (	(pCpnt->admin.protect_copy == DS28E07__COPY_WRITE_PROTECT_1)								// Administrative data protected
			||	(pCpnt->admin.protect_copy == DS28E07__COPY_WRITE_PROTECT_2))	{ err = ERROR_PROTECT; }	// Administrative data protected

		if (!err)
		{
			pCpnt->admin.protect_copy = prot;
			err = DS28E07_Write_AdminData(pCpnt);
		}
	}

	return err;
}


FctERR NONNULL__ DS28E07_Protect_UserBytes(DS28E07_t * const pCpnt, const DS28E07_prot_user prot)
{
	FctERR err = DS28E07_Read_AdminData(pCpnt);

	if (!err)
	{
		if (	(pCpnt->admin.protect_copy == DS28E07__COPY_WRITE_PROTECT_1)								// Administrative data protected
			||	(pCpnt->admin.protect_copy == DS28E07__COPY_WRITE_PROTECT_2)								// Administrative data protected
			||	(pCpnt->admin.protect_user == DS28E07__USER_WRITE_PROTECT))		{ err = ERROR_PROTECT; }	// User bytes write protected

		if (!err)
		{
			pCpnt->admin.protect_user = prot;
			err = DS28E07_Write_AdminData(pCpnt);
		}
	}

	return err;
}


FctERR NONNULLX__(1) DS28E07_Read_User_WORD(DS28E07_t * const pCpnt, uint16_t * const pWord)
{
	FctERR err = DS28E07_Read_Memory(pCpnt, pCpnt->admin.user.Byte, DS28E07__ADDR_USER_BYTE_1, DS28E07_NB_USER_BYTES);

	if ((!err) && (pWord != NULL))	{ *pWord = pCpnt->admin.user.Word; }

	return err;
}


FctERR NONNULLX__(1) DS28E07_Read_User_BYTES(DS28E07_t * const pCpnt, uint8_t * const pBytes)
{
	FctERR err = DS28E07_Read_Memory(pCpnt, pCpnt->admin.user.Byte, DS28E07__ADDR_USER_BYTE_1, DS28E07_NB_USER_BYTES);

	if ((!err) && (pBytes != NULL))	{ memcpy(pBytes, pCpnt->admin.user.Byte, DS28E07_NB_USER_BYTES); }

	return err;
}


FctERR NONNULL__ DS28E07_Write_UserBytes(DS28E07_t * const pCpnt, const uint8_t user[DS28E07_NB_USER_BYTES])
{
	DS28E07_prot_copy	copy_prot;
	FctERR				err = DS28E07_Get_Protect_Copy(pCpnt, &copy_prot);	// Reads all Administrative area

	if (!err)
	{
		if (	(copy_prot == DS28E07__COPY_WRITE_PROTECT_1)											// Administrative data protected
			||	(copy_prot == DS28E07__COPY_WRITE_PROTECT_2)											// Administrative data protected
			||	(pCpnt->admin.protect_user == DS28E07__USER_WRITE_PROTECT))	{ err = ERROR_PROTECT; }	// User bytes write protected

		if (!err)
		{
			memcpy(&pCpnt->admin.user, user, DS28E07_NB_USER_BYTES);

			err = DS28E07_Write_Memory(pCpnt, pCpnt->admin.admin_data, DS28E07__ADDR_PROT_0, sizeof(pCpnt->admin.admin_data));
		}
	}

	return err;
}


/****************************************************************/
#endif
/****************************************************************/

