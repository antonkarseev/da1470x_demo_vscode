/**
 ****************************************************************************************
 *
 * @file sys_background_flash_ops_internal.h
 *
 * @brief Background flash operation internal API
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_BACKGROUND_FLASH_OPS_INTERNAL_H_
#define SYS_BACKGROUND_FLASH_OPS_INTERNAL_H_

#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS

/**
 * \brief Initialize background flash operations
 */
void sys_background_flash_ops_init(void);

/**
 * \brief Check if there is pending background flash operations
 */
__RETAINED_HOT_CODE bool sys_background_flash_ops_is_pending(void);

/**
 * \brief Process background flash operation
 *
 * \return True, if WFI should be skipped, otherwise false.
 */
__RETAINED_CODE bool sys_background_flash_ops_handle(void);

/**
 * \brief Suspend background flash operation
 */
__RETAINED_CODE void sys_background_flash_ops_suspend(void);

/**
 * \brief Notify whether the pending background flash operation has been completed or not.
 */
__RETAINED_CODE void sys_background_flash_ops_notify(void);

#endif /* dg_configUSE_SYS_BACKGROUND_FLASH_OPS */

#endif /* SYS_BACKGROUND_FLASH_OPS_INTERNAL_H_ */
