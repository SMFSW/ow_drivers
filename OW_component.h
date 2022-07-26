/*!\file OW_component.h
** \author SMFSW
** \copyright MIT (c) 2021-2022, SMFSW
** \brief Base One Wire component
** \note Needed symbols may be defined in globals.h or at project level. If globals.h is not used, symbol NO_GLOBALS_HEADER needs to be defined.
**/
/****************************************************************/
#ifndef __OW_COMPONENT_H
	#define __OW_COMPONENT_H

#ifdef __cplusplus
	extern "C" {
#endif

#include <string.h>
#include "sarmfsw.h"

#include "OW_drv.h"

#ifndef NO_GLOBALS_HEADER
#include "globals.h"
#endif
/****************************************************************/


#define IS_OW_PERIPHERAL(name, idx)	((idx) < OW_##name##_NB)	//!< Macro for use with assert_param to check OW peripheral \b idx is valid for \b name peripheral

#define IS_OW_PERIPHERAL_ADDR(name, addr)	((OW_PERIPHERAL_IDX(addr, name) >= 0) &&			\
											(OW_PERIPHERAL_IDX(addr, name) < OW_##name##_NB))	//!< Macro for use with assert_param to check OW peripheral \b addr is valid for \b name peripheral


#define OW_PERIPHERAL_IDX(name, addr)		((int32_t) (((name##_t *) addr) - name))			//!< Macro to get OW peripheral index given \b addr for \b name peripheral


// *****************************************************************************
// Section: Constants
// *****************************************************************************


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\enum OW_ROM_cmd
** \brief Shared commands enum for One Wire
**/
typedef enum PACK__ OW_ROM_cmd {
	OW__OVERDRIVE_SKIP_ROM = 0x3C,	//!< Overdrive skip ROM command
	OW__READ_ROM = 0x33,			//!< Read ROM command
	OW__MATCH_ROM = 0x55,			//!< Match ROM command
	OW__OVERDRIVE_MATCH_ROM = 0x69,	//!< Overdrive match ROM command
	OW__RESUME = 0xA5,				//!< Resume command
	OW__READ_POWER_SUPPLY = 0xB4,	//!< Signals power supply mode to the master
	OW__SKIP_ROM = 0xCC,			//!< Skip ROM command
	OW__SEARCH_ROM = 0xF0,			//!< Search ROM command
} OW_ROM_cmd;


/*! \struct OW_slave_t
**  \brief One Wire slave config and control parameters
**/
typedef struct OW_slave_t {
	/*! \struct cfg
	**  \brief OW pSlave parameters
	**/
	struct {
		OW_DRV *	bus_inst;	//!< One Wire bus instance
		OW_ROM_ID_t	ROM_ID;		//!< One Wire pSlave ROM_ID
	} cfg;
	bool			en;			//!< State of pSlave (disabled/enabled)
	bool			busy;		//!< TODO: implement to be checked if transaction pending (it?)
} OW_slave_t;


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************


/*!\brief OW Slave device initialization
** \param[in,out] pSlave - pointer to OW slave instance to initialize
** \param[in] pOW - pointer to HAL OW instance
** \param[in] pROM - pointer to ROM Id
** \return Error code
**/
FctERR NONNULL__ OW_slave_init(OW_slave_t * pSlave, const OW_DRV * pOW, const OW_ROM_ID_t * const pROM);


/*!\brief OneWire Salve get power supply source
** \note May be useful to keep bus as busy during a copy scratchpad command or during a conversion (line should be held high, no other transaction allowed on bus)
** \warning Use only if device supports the command (meaning it can be powered by power or bus), otherwise result will be wrong and irrelevant
** \param[in] pSlave - pointer to OW slave instance
** \param[in,out] pBusPower - pointer to bus power variable result
** \return FctERR - Error code
**/
FctERR NONNULL__ OW_slave_get_power_supply(OW_slave_t * pSlave, bool * const pBusPower);

/***************/
/*** SETTERS ***/
/***************/
/*!\brief OW Slave device HAL instance change
** \param[in,out] pSlave - pointer to OW slave instance
** \param[in] pOW - pointer to HAL OW instance
** \return Error code
**/
__INLINE FctERR NONNULL_INLINE__ OW_set_slave_instance(OW_slave_t * pSlave, const OW_DRV * pOW) {
	pSlave->cfg.bus_inst = (OW_DRV *) pOW;
	return ERROR_OK; }

/*!\brief OW Slave device id change
** \param[in,out] pSlave - pointer to OW slave instance
** \param[in] pROM - pointer to ROM Id
** \return Error code
**/
__INLINE FctERR NONNULL_INLINE__ OW_set_slave_id(OW_slave_t * pSlave, const OW_ROM_ID_t * const pROM) {
	memcpy(pSlave->cfg.ROM_ID.romId, pROM->romId, sizeof(OW_ROM_ID_t));
	return ERROR_OK; }


/*!\brief Set OW Slave device disabled/enabled state
** \param[in,out] pSlave - pointer to OW slave instance
** \param[in] en - OW device state (disabled/enabled)
** \return Nothing
**/
__INLINE FctERR NONNULL_INLINE__ OW_set_enable(OW_slave_t * pSlave, const bool en) {
	pSlave->en = en;
	return ERROR_OK; }

/*!\brief Set OW Slave bus/device business
** \param[in,out] pSlave - pointer to OW slave instance
** \param[in] busy - OW bus/device state
** \return Nothing
**/
__INLINE FctERR NONNULL_INLINE__ OW_set_busy(OW_slave_t * pSlave, const bool busy) {
	pSlave->busy = busy;
	return ERROR_OK; }


/***************/
/*** GETTERS ***/
/***************/
/*!\brief Get OW slave device enabled state
** \param[in,out] pSlave - pointer to OW slave instance
** \return true if OW slave is enabled
**/
__INLINE bool NONNULL_INLINE__ OW_is_enabled(const OW_slave_t * pSlave) {
	return pSlave->en; }

/*!\brief Get OW slave device busy state
** \param[in,out] pSlave - pointer to OW slave instance
** \return true if OW bus/slave is busy
**/
__INLINE bool NONNULL_INLINE__ OW_is_busy(const OW_slave_t * pSlave) {
	return pSlave->busy; }

/*!\brief Get OW slave device HAL OW instance
** \param[in,out] pSlave - pointer to OW slave instance
** \return OW slave device HAL OW instance
**/
__INLINE OW_DRV * NONNULL_INLINE__ OW_get_pSlave_instance(const OW_slave_t * pSlave) {
	return pSlave->cfg.bus_inst; }

/*!\brief Get OW slave device id
** \param[in] pSlave - pointer to OW slave instance
** \return OW slave device id
**/
__INLINE OW_ROM_ID_t NONNULL_INLINE__ OW_get_slave_id(const OW_slave_t * pSlave) {
	return pSlave->cfg.ROM_ID; }


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif	/* __OW_COMPONENT_H */
/****************************************************************/
