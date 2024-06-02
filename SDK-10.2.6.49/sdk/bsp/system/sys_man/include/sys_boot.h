/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_BOOT System Boot Service
 *
 * \brief System Boot Service
 *
 * \{
 */

/**
****************************************************************************************
*
* @file sys_boot.h
*
* @brief System Boot Handler header file
*
* Copyright (C) 2021-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef SYS_BOOT_H_
#define SYS_BOOT_H_

#if MAIN_PROCESSOR_BUILD

#include "sdk_defs.h"

/**
 * \brief Booter origin
 */
typedef enum {
        SYS_BOOT_NO_BOOTING     = 0xDEAD,       /**< No booting occurred */
        SYS_BOOT_UART_BOOT      = 0x0000,       /**< UART boot */
        SYS_BOOT_OQSPI_ACTIVE   = 0x1111,       /**< OQSPI active image */
        SYS_BOOT_OQSPI_UPDATE   = 0x2222,       /**< OQSPI update image */
        SYS_BOOT_ORIGIN_INVALID = 0xFFFF,       /**< Invalid */
} SYS_BOOT_ORIGIN;

/**
 * \brief OTP programming status
 */
typedef enum {
        SYS_BOOT_OTP_PASS    = 0xAAAA,          /**< OTP programming passed or no OTP programming */
        SYS_BOOT_OTP_FAIL    = 0xDEAD,          /**< OTP programming failed */
        SYS_BOOT_OTP_INVALID = 0xFFFF,          /**< Invalid */
} SYS_BOOT_OTP_PRORAMMING_STATUS;

/**
 * \brief Makes a secure copy of the result reported by the booter
 *        to avoid potential overwriting of the MTB memory space,
 *        where this result is initially stored.
 */
void sys_boot_secure_copy_boot_result(void);

/**
 * \brief Indicates the origin of the previous boot of the device
 *
 * \return boot origin
 *
 */
SYS_BOOT_ORIGIN sys_boot_get_previous_boot_origin(void);

/**
 * \brief Indicates the OTP programming status of the previous boot of the device
 *
 * \return otp programming status
 *
 */
SYS_BOOT_OTP_PRORAMMING_STATUS sys_boot_get_previous_boot_otp_status(void);

#endif /* MAIN_PROCESSOR_BUILD */
#if dg_configUSE_SYS_BOOT
/**
 * \brief  Check and repair the Primary and the Backup Product Headers
 *
 * Check whether the Primary Product Header is valid or not. If not, copy the Backup Product Header
 * on it and validate the CRC of the repaired Primary Product Header. If yes, check the Backup
 * Product Header. If corrupted, copy the Primary Product Header on it and check the CRC of the
 * repaired Backup Product Header. In case that both product headers are corrupted, the system will
 * never boot, hence it is meaningless to check both of them at every boot.
 *
 */
void sys_boot_restore_product_headers(void);

#endif /* dg_configSYS_BOOT */

#endif /* SYS_BOOT_H_ */
/**
 * \}
 * \}
 */
