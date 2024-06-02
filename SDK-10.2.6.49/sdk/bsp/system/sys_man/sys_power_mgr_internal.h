/**
 \addtogroup BSP
 \{
 \addtogroup SYSTEM
 \{
 \addtogroup POWER_MANAGER
 \{
 \addtogroup INTERNAL
 \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_power_mgr_internal.h
 *
 * @brief Power Manager internal header file.
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef SYS_POWER_MGR_INTERNAL_H_
#define SYS_POWER_MGR_INTERNAL_H_

typedef enum system_state_type {
        sys_active = 0,
        sys_idle,
        sys_powered_down,
} system_state_t;

#ifdef OS_PRESENT

#include <stdint.h>
#include <stdbool.h>
#include "osal.h"
#ifdef CONFIG_USE_BLE
#include "ble_stack_config.h"
#endif




/**
 * \brief Get system sleep state
 *
 * \return The system sleep state
 *
 */
__RETAINED_HOT_CODE system_state_t pm_get_system_sleep_state(void);

#endif /* OS_PRESENT */

#endif /* SYS_POWER_MGR_INTERNAL_H_ */

/**
 \}
 \}
 \}
 \}
 */
