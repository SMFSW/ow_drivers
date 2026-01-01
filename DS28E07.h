/*!\file DS28E07.h
** \author SMFSW
** \copyright MIT (c) 2021-2026, SMFSW
** \brief DS28E07: 1024-Bit, 1-Wire EEPROM
** \note Alternate PNs: (DS1972), DS2431
**/
// cppcheck-suppress-begin misra-c2012-19.2
/****************************************************************/
#ifndef OW_DS28E07_H__
	#define OW_DS28E07_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#include "OW_component.h"
#include "OW_dev_sn.h"
#include "OW_dev_eeprom.h"
/****************************************************************/


#ifndef OW_DS28E07_NB
//! \note Define OW_DS28E07_NB_NB to enable multiple peripherals of this type
#define OW_DS28E07_NB	1U	//!< Number of DS28E07 peripherals
#endif


// *****************************************************************************
// Section: Constants
// *****************************************************************************
#define DS28E07_PAGES			OW_EEP__PAGE4						//!< DS28E07 Number of Pages (4 pages)
#define DS28E07_PAGE_SIZE		0x20U								//!< DS28E07 Page Size
#define DS28E07_SCRATCHPAD_SIZE	0x08U								//!< DS28E07 Scratchpad Size
#define DS28E07_MEMORY_SIZE		(DS28E07_PAGE_SIZE * DS28E07_PAGES)	//!< DS28E07 Maximum size
#define DS28E07_MAX_WRITE_ADDR	DS28E07__ADDR_USER_BYTE_2			//!< DS28E07 Maximum write address
#define DS28E07_MAX_READ_ADDR	(DS28E07__ADDR_USER_BYTE_2 + 8U)	//!< DS28E07 Maximum read address
#define DS28E07_NB_USER_BYTES	2U									//!< DS28E07 Number of User Bytes
#define DS28E07_COPY_TIME		12U									//!< DS28E07 Time to copy scratchpad to EEPROM

// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\enum DS28E07_reg_map
** \brief Register map enum of DS28E07
**/
typedef enum PACK__ {
	DS28E07__ADDR_PAGE_0 = 0U,				//!< User memory Page 0
	DS28E07__ADDR_PAGE_1 = 0x20U,			//!< User memory Page 1
	DS28E07__ADDR_PAGE_2 = 0x40U,			//!< User memory Page 2
	DS28E07__ADDR_PAGE_3 = 0x60U,			//!< User memory Page 3
	DS28E07__ADDR_PROT_0 = 0x80U,			//!< Protection Control Byte Page 0 (55h: Write Protect P0; AAh: EPROM mode P0; 55h or AAh: Write Protect 80h)
	DS28E07__ADDR_PROT_1 = 0x81U,			//!< Protection Control Byte Page 1 (55h: Write Protect P1; AAh: EPROM mode P1; 55h or AAh: Write Protect 80h)
	DS28E07__ADDR_PROT_2 = 0x82U,			//!< Protection Control Byte Page 2 (55h: Write Protect P2; AAh: EPROM mode P2; 55h or AAh: Write Protect 80h)
	DS28E07__ADDR_PROT_3 = 0x83U,			//!< Protection Control Byte Page 3 (55h: Write Protect P3; AAh: EPROM mode P3; 55h or AAh: Write Protect 80h)
	DS28E07__ADDR_PROT_COPY = 0x84U,		//!< Copy Protection Byte (55h or AAh: Copy Protect 0080:008Fh (? probably 0087h), and any write-protected Pages)
	DS28E07__ADDR_PROT_FACTORY = 0x85U,		//!< Factory byte. Set at Factory. (AAh:Write Protect 85h, 86h, 87h; 55h: Write Protect 85h, un-protect 86h, 87h)
	DS28E07__ADDR_USER_BYTE_1 = 0x86U,		//!< User Byte / Manufacturer ID
	DS28E07__ADDR_USER_BYTE_2 = 0x87U,		//!< User Byte / Manufacturer ID
	DS28E07__ADDR_RESERVED_START = 0x88U,	//!< Reserved
	DS28E07__ADDR_RESERVED_END = 0xFEU,		//!< Reserved
	DS28E07__ADDR_REVISION_CODE = 0xFFU		//!< Chip Revision Code
} DS28E07_reg;


/*!\enum DS28E07_cmd
** \brief Commands enum for DS28E07
** \note Unused
**/
typedef enum PACK__ {
	DS28E07__WRITE_SCRATCHPAD = OW_EEP__WRITE_SCRATCHPAD,	//!< Write scratchpad command
	DS28E07__COPY_SCRATCHPAD = OW_EEP__COPY_SCRATCHPAD,		//!< Copy scratchpad command
	DS28E07__READ_SCRATCHPAD = OW_EEP__READ_SCRATCHPAD,		//!< Read scratchpad command
	DS28E07__READ_MEMORY = OW_EEP__READ_MEMORY				//!< Read memory command
} DS28E07_cmd;


/*!\enum DS28E07_prot_page
** \brief Pages write protection values for DS28E07
** \note Unknown values corresponds to DS28E07__PAGE_WRITE_NOT_SET
** \warning Write protect registers are locked once written to any known protection value
**/
typedef enum PACK__ {
	DS28E07__PAGE_WRITE_NOT_SET = 0U,		//!< Write protect not-set value
	DS28E07__PAGE_WRITE_PROTECT = 0x55U,	//!< Write protect value
	DS28E07__PAGE_EEPROM_MODE = 0xAAU		//!< EEPROM mode value (write allowed)
} DS28E07_prot_page;


/*!\enum DS28E07_prot_copy
** \brief Copy write protection values for DS28E07
** \note Unknown values corresponds to DS28E07__COPY_WRITE_NOT_SET
** \warning Write protect registers are locked once written to any known protection value
**/
typedef enum PACK__ {
	DS28E07__COPY_WRITE_NOT_SET = 0U,		//!< Copy protect not set value
	DS28E07__COPY_WRITE_PROTECT_1 = 0x55U,	//!< Copy protect value
	DS28E07__COPY_WRITE_PROTECT_2 = 0xAAU	//!< Copy protect value
} DS28E07_prot_copy;


/*!\enum DS28E07_prot_user
** \brief User bytes write protection values for DS28E07
** \note Unknown values corresponds to DS28E07__USER_WRITE_NOT_SET
** \warning Write protect registers are locked once written to any known protection value
**/
typedef enum PACK__ {
	DS28E07__USER_WRITE_NOT_SET = 0U,		//!< User protect not set value
	DS28E07__USER_WRITE_UNPROTECT = 0x55U,	//!< User un-protect value
	DS28E07__USER_WRITE_PROTECT = 0xAAU		//!< User protect value
} DS28E07_prot_user;


/*!\union uDS28E07_REG__ES
** \brief Union for E/S register of DS28E07
**/
typedef union PACK__ {
	uint8_t Byte;
	struct PACK__ {
		uint8_t E		:3;	/*!<
							** (loaded with the incoming T[2:0] on a Write Scratchpad command and increment on each subsequent
							** data byte. This is, in effect, a byte-ending off-set counter within the 8-byte scratchpad.)*/
		uint8_t			:2;
		uint8_t	PF		:1;	/*!< Power Failure
							** (a logic 1 if the data in the scratchpad is not valid due to a loss of power or if the master
							** sends fewer bytes than needed to reach the end of the scratch-pad.)*/
		uint8_t			:1;
		uint8_t	AA		:1;	/*!< Authorization Accepted
							** (acts as a flag to indicate that the data stored in the scratchpad has already been copied
							** to the target memory address. Writing data to the scratchpad clears this flag.)*/
	} Bits;
} uDS28E07_REG__ES;


/*!\struct DS28E07_t
** \brief DS28E07 user interface struct
**/
typedef struct {
	/*** device generic peripheral types structures ***/
	OW_sn_t						sn;				//!< Serial Number device type structure
	OW_eep_t					eep;			//!< EEPROM device type structure
	/*** device specific variables ***/
	OW_eep_scratch_t			scratch;								//!< Scratchpad structure
	uint8_t						scratch_data[DS28E07_SCRATCHPAD_SIZE];	//!< Scratchpad data array
	union PACK__ {
		uint8_t					admin_data[8];	//!< Administrative data array
		struct PACK__ {
			DS28E07_prot_page	protect_page0;	//!< Page 0 protection byte
			DS28E07_prot_page	protect_page1;	//!< Page 1 protection byte
			DS28E07_prot_page	protect_page2;	//!< Page 2 protection byte
			DS28E07_prot_page	protect_page3;	//!< Page 3 protection byte
			DS28E07_prot_copy	protect_copy;	//!< Copy protection byte
			DS28E07_prot_user	protect_user;	//!< User bytes protection byte
			uWord PACK__		user;			//!< User bytes
		};
	} admin;
} DS28E07_t;

extern DS28E07_t DS28E07[OW_DS28E07_NB];	//!< DS28E07 User structure


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
/*!\brief DS28E07 peripheral family code getter
** \return DS28E07 family code
**/
OW_ROM_type DS28E07_Get_FamilyCode(void);


/******************/
/*** Slave init ***/
/******************/
/*!\brief Initialization Sequence for DS28E07 peripheral
** \weak DS28E07 Init sequence may be user implemented if custom initialization sequence needed
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \return FctERR - error code
**/
FctERR NONNULL__ DS28E07_Init_Sequence(DS28E07_t * const pCpnt);

/*!\brief Initialization for DS28E07 peripheral
** \param[in] idx - DS28E07 index
** \param[in] pOW - pointer to OneWire driver instance
** \param[in] pROM - Pointer to ROM Id structure
** \return FctERR - error code
**/
FctERR NONNULL__ DS28E07_Init(const uint8_t idx, OW_DRV * const pOW, const OW_ROM_ID_t * const pROM);

/*!\brief Initialization for DS28E07 peripheral
** \warning In case multiple devices (defined by OW_DS28E07_NB > 1), you shall use DS28E07_Init instead
** \param[in] pROM - Pointer to ROM Id structure
** \return FctERR - error code
**/
FctERR DS28E07_Init_Single(const OW_ROM_ID_t * const pROM);


/*************************************/
/*** Low level access / Procedures ***/
/*************************************/

#ifndef DOXY
/*!\brief DS28E07 Serial Number getter
** \param[in] pCpnt - Pointer to DS28E07 peripheral
** \return DS28E07 peripheral serial number
**/
#endif
OW_SN_GETTER(DS28E07);


/*!\brief DS28E07 EEPROM device write cycle time handler
** \note Non blocking mode: start copy, test copy time, release bus
** \note Handler shall be called periodically in a main like loop
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ DS28E07_WriteCycle_Handler(DS28E07_t * const pCpnt) {
	return OW_EEP_WriteCycle_Handler(&pCpnt->eep); }


/*!\brief DS28E07 read from memory
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in,out] pData - Pointer to data for reception
** \param[in] addr - Target memory cell start address
** \param[in] len - Number of data bytes to receive
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ DS28E07_Read_Memory(DS28E07_t * const pCpnt, uint8_t * pData, const uint32_t addr, const size_t len) {
	return OW_EEP_Read_Memory(&pCpnt->eep, pData, addr, len); }

/*!\brief DS28E07 write to memory
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] pData - Pointer to data for transmission
** \param[in] addr - Target memory cell start address
** \param[in] len - Number of data bytes to transmit
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ DS28E07_Write_Memory(DS28E07_t * const pCpnt, const uint8_t * pData, const uint32_t addr, const size_t len) {
	return OW_EEP_Write_Memory(&pCpnt->eep, pData, addr, len); }


/*!\brief DS28E07 read scratchpad
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ DS28E07_Read_Scratchpad(DS28E07_t * const pCpnt) {
	return OW_EEP_Read_Scratchpad(&pCpnt->eep); }

/*!\brief DS28E07 write scratchpad
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] pData - Pointer to data for transmission
** \param[in] addr - Target memory cell start address
** \param[in] len - Number of data bytes to transmit
** \return FctERR - error code
**/
__INLINE FctERR NONNULL_INLINE__ DS28E07_Write_Scratchpad(DS28E07_t * const pCpnt, const uint8_t * pData, const uint32_t addr, const size_t len) {
	return OW_EEP_Write_Scratchpad(&pCpnt->eep, pData, addr, len); }


/*!\brief DS28E07 read administrative data
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \return FctERR - error code
**/
FctERR NONNULL__ DS28E07_Read_AdminData(DS28E07_t * const pCpnt);


/*!\brief DS28E07 get page protection value
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] pProt - Pointer to protection value output
** \param[in] page - Page number
** \return FctERR - error code
**/
FctERR NONNULL__ DS28E07_Get_Protect_Page(DS28E07_t * const pCpnt, DS28E07_prot_page * const pProt, const OW_eep_pages page);

/*!\brief DS28E07 get copy protection value
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] pProt - Pointer to protection value output
** \return FctERR - error code
**/
FctERR NONNULL__ DS28E07_Get_Protect_Copy(DS28E07_t * const pCpnt, DS28E07_prot_copy * const pProt);

/*!\brief DS28E07 get user bytes protection value
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] pProt - Pointer to protection value output
** \return FctERR - error code
**/
FctERR NONNULL__ DS28E07_Get_Protect_UserBytes(DS28E07_t * const pCpnt, DS28E07_prot_user * const pProt);

/*!\brief DS28E07 set page protection
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] prot - Protection value
** \param[in] page - Page number
** \return FctERR - error code
**/
FctERR NONNULL__ DS28E07_Protect_Page(DS28E07_t * const pCpnt, const DS28E07_prot_page prot, const OW_eep_pages page);

/*!\brief DS28E07 set copy protection
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] prot - Protection value
** \return FctERR - error code
**/
FctERR NONNULL__ DS28E07_Protect_Copy(DS28E07_t * const pCpnt, const DS28E07_prot_copy prot);

/*!\brief DS28E07 set user bytes protection
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] prot - Protection value
** \return FctERR - error code
**/
FctERR NONNULL__ DS28E07_Protect_UserBytes(DS28E07_t * const pCpnt, const DS28E07_prot_user prot);


/*!\brief DS28E07 get user bytes (as a WORD)
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] pWord - Pointer to word output
** \return FctERR - error code
**/
FctERR NONNULLX__(1) DS28E07_Read_User_WORD(DS28E07_t * const pCpnt, uint16_t * const pWord);

/*!\brief DS28E07 get user bytes (as BYTE array)
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] pBytes - Pointer to bytes output
** \return FctERR - error code
**/
FctERR NONNULLX__(1) DS28E07_Read_User_BYTES(DS28E07_t * const pCpnt, uint8_t * const pBytes);

/*!\brief DS28E07 set user bytes (as BYTE array)
** \param[in,out] pCpnt - Pointer to DS28E07 component
** \param[in] user - User bytes array
** \return FctERR - error code
**/
FctERR NONNULL__ DS28E07_Write_UserBytes(DS28E07_t * const pCpnt, const uint8_t user[DS28E07_NB_USER_BYTES]);


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif
// cppcheck-suppress-end misra-c2012-19.2
/****************************************************************/

