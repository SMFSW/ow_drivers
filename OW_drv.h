/*!\file OW_drv.h
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief GPIO OneWire driver
**/
/****************************************************************/
#ifndef __OW_DRV_H
	#define __OW_DRV_H

#ifdef __cplusplus
	extern "C" {
#endif

#include "sarmfsw.h"

#include "OW_rom_id.h"
#include "OW_crc.h"

#include "OW_phy.h"


#ifndef OW_BUS_NB
#define OW_BUS_NB				1	//!< Number of One Wire bus
#endif

#ifndef	OW_CUSTOM_BYTE_HANDLERS
#define OW_CUSTOM_BYTE_HANDLERS	0	//!< Custom Byte Transmit/Receive disabled (using common global function instead)
#endif
/****************************************************************/


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\struct OWSearch_State_t
** \brief OneWire Search State struct
**/
typedef struct OWSearch_State_t {
	OW_ROM_ID_t		ROM_ID;						//!< Last found ROM Id
	uint8_t			lastDiscrepancy;			//!< Last found Id
	uint8_t			lastFamilyDiscrepancy;		//!< Last found Family	// TODO: check if needs to be type OW_ROM_type
	bool			lastDeviceFlag;				//!< Last device found flag
} OWSearch_State_t;


typedef struct StructOW_DRV		OW_DRV;			//!< Typedef for OW_DRV used by function pointers included in struct

typedef FctERR (*pfOW_phyWrite_t)(const OW_DRV * const pOW, const uint8_t data);	//!< OneWire Write function typedef
typedef FctERR (*pfOW_phyRead_t)(const OW_DRV * const pOW, uint8_t * const pData);	//!< OneWire Read function typedef
typedef FctERR (*pfOW_phyReset_t)(const OW_DRV * const pOW);						//!< OneWire Reset bus function typedef


/*!\struct StructOW_DRV
** \brief OneWire driver struct
**/
struct StructOW_DRV {
	uint8_t						idx;				//!< OWdrv index
	OWPhy						phy;				//!< OWdrv physical peripheral type
	OW_phy_u					phy_inst;			//!< OWdrv physical instance
	OW_GPIO_HandleTypeDef		GPIO_cfg;			//!< OneWire bus GPIO configuration (if selected phy)
	pfOW_phyReset_t				pfReset;			//!< OneWire bus Reset function pointer
	pfOW_phyWrite_t				pfWriteBit;			//!< OneWire bus Bit Write function pointer
	pfOW_phyRead_t				pfReadBit;			//!< OneWire bus Bit Read function pointer
#if OW_CUSTOM_BYTE_HANDLERS
	// pfWriteByte & pfReadByte might not be needed (common function across physical interfaces)
	// TODO: remove OW_CUSTOM_BYTE_HANDLERS and code when tested across more physical interfaces
	pfOW_phyWrite_t				pfWriteByte;		//!< OneWire bus Byte Write function pointer
	pfOW_phyRead_t				pfReadByte;			//!< OneWire bus Byte Read function pointer
#endif
	OWSearch_State_t			search_state;		//!< OneWire bus search state
};


extern OW_DRV OWdrv[OW_BUS_NB];						//!< OWdrv structure

#define IS_OW_DRV_IDX(IDX)	((IDX) < OW_BUS_NB)		//!< Macro for use with assert_param to check OWdrv index \b IDX is valid


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
/*!\brief OneWire driver instance init
** \param[in,out] pHandle - Pointer to physical handler
** \note OW_Handle_t can be any peripheral instance, or GPIO_HandleTypeDef to init OW on GPIO
** \param[in] idx - Instance index
** \return FctERR - Error code
**/
FctERR NONNULL__ OWInit(OW_Handle_t * const pHandle, const uint8_t idx);


/*!\brief OneWire write bit to bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] bit - bit for transmission
** \return FctERR - Error code
**/
FctERR NONNULL__ OWWrite_bit(const OW_DRV * const pOW, const uint8_t bit);

/*!\brief OneWire write byte to bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] byte - Byte for transmission
** \return FctERR - Error code
**/
FctERR NONNULL__ OWWrite_byte(const OW_DRV * const pOW, const uint8_t byte);

/*!\brief OneWire write to bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] pData - Pointer to data for transmission
** \param[in] len - Number of data bytes to transmit
** \return FctERR - Error code
**/
FctERR NONNULL__ OWWrite(const OW_DRV * const pOW, const uint8_t * const pData, const size_t len);


/*!\brief OneWire read bit from bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pBit - Pointer to bit for reception
** \return FctERR - Error code
**/
FctERR NONNULL__ OWRead_bit(const OW_DRV * const pOW, uint8_t * const pBit);

/*!\brief OneWire read byte from bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] pByte - Pointer to byte for reception
** \return FctERR - Error code
**/
FctERR NONNULL__ OWRead_byte(const OW_DRV * const pOW, uint8_t * const pByte);

/*!\brief OneWire read from bus
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pData - Pointer to data for reception
** \param[in] len - Number of data bytes to receive
** \return FctERR - Error code
**/
FctERR NONNULL__ OWRead(const OW_DRV * const pOW, uint8_t * const pData, const size_t len);


/*!\brief OneWire device select
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] pROM - Pointer to ROM Id structure
**/
void NONNULL__ OWSelect(const OW_DRV * const pOW, const OW_ROM_ID_t * const pROM);

/*!\brief OneWire skip
** \param[in,out] pOW - Pointer to OneWire driver instance
**/
void NONNULL__ OWSkip(const OW_DRV * const pOW);

/*!\brief OneWire resume
** \param[in,out] pOW - Pointer to OneWire driver instance
**/
void NONNULL__ OWResume(const OW_DRV * const pOW);


/*!\brief OneWire search device (first)
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pROM - Pointer to ROM Id structure
** \return FctERR - Error code
**/
FctERR NONNULLX__(1) OWFirst(OW_DRV * const pOW, OW_ROM_ID_t * const pROM);

/*!\brief OneWire search device (any but first)
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] pROM - Pointer to ROM Id structure
** \return FctERR - Error code
**/
FctERR NONNULLX__(1) OWNext(OW_DRV * const pOW, OW_ROM_ID_t * const pROM);

/*!\brief OneWire search all devices
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in,out] ROMId - Pointer to ROM Ids array
** \param[in] max_nb - Maximum number of devices to search for (most likely number of ROMId array elements)
** \return FctERR - Error code
**/
FctERR NONNULL__ OWSearchAll(OW_DRV * const pOW, OW_ROM_ID_t ROMId[], const uint8_t max_nb);


/*!\brief OneWire verify
** \param[in,out] pOW - Pointer to OneWire driver instance
** \return FctERR - Error code
**/
FctERR NONNULL__ OWVerify(OW_DRV * const pOW);

/*!\brief OneWire target setup
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] family_code - device family code
**/
void NONNULL__ OWTargetSetup(OW_DRV * const pOW, const OW_ROM_type family_code);

/*!\brief OneWire family skip
** \note Can be used to skip whole family after first device of this type is discovered
** \param[in,out] pOW - Pointer to OneWire driver instance
**/
void NONNULL__ OWFamilySkipSetup(OW_DRV * const pOW);


/*!\brief OneWire control sequence
** \weak OWROMCmd_Control_Sequence may be user implemented if custom control sequence is required
** \param[in,out] pOW - Pointer to OneWire driver instance
** \param[in] pROM - Pointer to ROM Id structure
** \param[in] pfOriginator - Function pointer of control sequence caller
** \return FctERR - Error code
**/
FctERR NONNULL__ OWROMCmd_Control_Sequence(const OW_DRV * const pOW, const OW_ROM_ID_t * const pROM, const void * const pfOriginator);


/*!\brief OneWire read ROM Id
** \warning Assume a single chip connected (otherwise will fail, CRC check will mismatch as multiple devices will answer)
** \param[in] pOW - Pointer to OneWire driver instance
** \param[in,out] pROM - Pointer to ROM Id structure
** \return FctERR - Error code
**/
FctERR NONNULL__ OWRead_ROM_Id(const OW_DRV * const pOW, OW_ROM_ID_t * const pROM);


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif	/* __OW_DRV_H */
/****************************************************************/
