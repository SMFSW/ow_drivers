/*!\file OW_component.h
** \author SMFSW
** \copyright MIT (c) 2021-2025, SMFSW
** \brief Base One Wire component
** \note Needed symbols may be defined in globals.h or at project level. If globals.h is not used, symbol NO_GLOBALS_HEADER needs to be defined.
** \hideincludedbygraph
**/
/****************************************************************/
#ifndef OW_COMPONENT_H__
	#define OW_COMPONENT_H__

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


#define IS_OW_PERIPHERAL(name, idx)				((idx) < OW_##name##_NB)							//!< Macro for use with assert_param to check OW peripheral \b idx is valid for \b name peripheral

#define IS_OW_PERIPHERAL_ADDR(name, addr)		((OW_PERIPHERAL_IDX(addr, name) >= 0) &&			\
												(OW_PERIPHERAL_IDX(addr, name) < OW_##name##_NB))	//!< Macro for use with assert_param to check OW peripheral \b addr is valid for \b name peripheral


#define OW_PERIPHERAL_IDX(name, addr)			((int32_t) (((name##_t *) addr) - name))			//!< Macro to get OW peripheral index given \b addr for \b name peripheral

#define OW_PERIPHERAL_DEV_OFFSET(name, type)	OFFSET_OF(name##_t, type)							//!< Macro to get \b type structure offset in \b per peripheral structure

// *****************************************************************************
// Section: Constants
// *****************************************************************************


// *****************************************************************************
// Section: Types
// *****************************************************************************
/*!\enum _OW_ROM_cmd
** \brief Shared commands enum for One Wire
**/
typedef enum PACK__ _OW_ROM_cmd {
	OW__OVERDRIVE_SKIP_ROM = 0x3CU,		//!< Overdrive skip ROM command
	OW__READ_ROM = 0x33U,				//!< Read ROM command
	OW__MATCH_ROM = 0x55U,				//!< Match ROM command
	OW__OVERDRIVE_MATCH_ROM = 0x69U,	//!< Overdrive match ROM command
	OW__RESUME = 0xA5U,					//!< Resume command
	OW__READ_POWER_SUPPLY = 0xB4U,		//!< Signals power supply mode to the master
	OW__SKIP_ROM = 0xCCU,				//!< Skip ROM command
	OW__SEARCH_ROM = 0xF0U,				//!< Search ROM command
} OW_ROM_cmd;


/*! \struct OW_slave_t
**  \brief One Wire slave config and control parameters
**/
typedef struct _OW_slave_t {
	/*! \struct cfg
	**  \brief OW pSlave parameters
	**/
	struct {
		OW_DRV *	bus_inst;			//!< One Wire bus instance
		OW_mutex_t	mutex_id;			//!< Device mutex identifier on OW instance (for mutual exclusion)
		OW_ROM_ID_t	ROM_ID;				//!< One Wire pSlave ROM_ID
		bool		parasite_powered;	//!< Device power type (Parasite power from bus or Vcc)
	} cfg;
	bool			en;					//!< State of pSlave (disabled/enabled)
	bool			busy;				//!< Device busy flag (ongoing operation), useful for devices including multiple functionalities
} OW_slave_t;


// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
/*!\brief OW Slave device initialization
** \hidecallergraph
** \param[in,out] pSlave - pointer to OW slave instance to initialize
** \param[in] pOW - pointer to HAL OW instance
** \param[in] pROM - pointer to ROM Id
**/
void NONNULL__ OW_slave_init(OW_slave_t * const pSlave, OW_DRV * const pOW, const OW_ROM_ID_t * const pROM);


/***************/
/*** SETTERS ***/
/***************/
/*!\brief OW Slave device HAL instance change
** \param[in,out] pSlave - pointer to OW slave instance
** \param[in] pOW - pointer to HAL OW instance
**/
__INLINE void NONNULL_INLINE__ OW_set_slave_instance(OW_slave_t * const pSlave, OW_DRV * const pOW) {
	pSlave->cfg.bus_inst = pOW; }

/*!\brief OW Slave device id change
** \param[in,out] pSlave - pointer to OW slave instance
** \param[in] pROM - pointer to ROM Id
**/
__INLINE void NONNULL_INLINE__ OW_set_slave_id(OW_slave_t * const pSlave, const OW_ROM_ID_t * const pROM) {
	UNUSED_RET memcpy(pSlave->cfg.ROM_ID.romId, pROM->romId, OW_ROM_ID_SIZE); }


/*!\brief Set OW Slave device disabled/enabled state
** \hidecallergraph
** \param[in,out] pSlave - pointer to OW slave instance
** \param[in] en - OW device state (disabled/enabled)
**/
__INLINE void NONNULL_INLINE__ OW_set_enable(OW_slave_t * const pSlave, const bool en) {
	pSlave->en = en; }

/*!\brief Set OW Slave bus/device business
** \param[in,out] pSlave - pointer to OW slave instance
** \param[in] busy - OW bus/device state
**/
__INLINE void NONNULL_INLINE__ OW_set_busy(OW_slave_t * const pSlave, const bool busy) {
	pSlave->busy = busy; }


/***************/
/*** GETTERS ***/
/***************/
/*!\brief Get OW slave device enabled state
** \param[in,out] pSlave - pointer to OW slave instance
** \return true if OW slave is enabled
**/
__INLINE bool NONNULL_INLINE__ OW_is_enabled(const OW_slave_t * const pSlave) {
	return pSlave->en; }

/*!\brief Get OW slave device busy state
** \param[in,out] pSlave - pointer to OW slave instance
** \return true if OW bus/slave is busy
**/
__INLINE bool NONNULL_INLINE__ OW_is_busy(const OW_slave_t * const pSlave) {
	return pSlave->busy; }

/*!\brief Get OW slave device HAL OW instance
** \param[in,out] pSlave - pointer to OW slave instance
** \return OW slave device HAL OW instance
**/
__INLINE OW_DRV * NONNULL_INLINE__ OW_get_pSlave_instance(const OW_slave_t * const pSlave) {
	return pSlave->cfg.bus_inst; }

/*!\brief Get OW slave device id
** \param[in] pSlave - pointer to OW slave instance
** \return OW slave device id
**/
__INLINE OW_ROM_ID_t NONNULL_INLINE__ OW_get_slave_id(const OW_slave_t * const pSlave) {
	return pSlave->cfg.ROM_ID; }

/*!\brief Get OW slave power source
** \param[in] pSlave - pointer to OW slave instance
** \return OW slave power source (true when parasite powered)
**/
__INLINE bool NONNULL_INLINE__ OW_get_power_source(const OW_slave_t * const pSlave) {
	return pSlave->cfg.parasite_powered; }


/*****************/
/*** CALLBACKS ***/
/*****************/
/*!\brief OW Watchdog refresh callback
** \weak Function declared as weak, can be customly implemented in user code is specific actions needs to be taken (IWDG refreshed by default)
**/
void OW_Watchdog_Refresh(void);


/****************************************************************/
#ifdef __cplusplus
	}
#endif

#endif
/****************************************************************/
