/**
 ****************************************************************************************
 *
 * @file dlg_version.h
 *
 * @brief Bluetooth Controller version definitions specific to Dialog Semiconductor.
 *
 * Copyright (C) 2016-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *
 ****************************************************************************************
 */


#ifndef _DLG_VERSION_H_
#define _DLG_VERSION_H_
/**
 ****************************************************************************************
 * @defgroup DLG_VERSION Version Definitions
 * @ingroup COMMON
 *
 * @brief Bluetooth Controller version definitions specific to Dialog Semiconductor.
 *
 * @{
 ****************************************************************************************
 */

/// Bluetooth Core Specification Versions
/// Bluetooth Core Specification 1.0b (Withdrawn)
#define BLUETOOTH_VERSION_1_0                   0
/// Bluetooth Core Specification 1.1 (Withdrawn)
#define BLUETOOTH_VERSION_1_1                   1
/// Bluetooth Core Specification 1.2 (Withdrawn)
#define BLUETOOTH_VERSION_1_2                   2
/// Bluetooth Core Specification 2.0 + EDR (Withdrawn)
#define BLUETOOTH_VERSION_2_0                   3
/// Bluetooth Core Specification 2.1 + EDR (Withdrawn)
#define BLUETOOTH_VERSION_2_1                   4
/// Bluetooth Core Specification 3.0 + HS (Withdrawn)
#define BLUETOOTH_VERSION_3_0                   5
/// Bluetooth Core Specification 4.0
#define BLUETOOTH_VERSION_4_0                   6
/// Bluetooth Core Specification 4.1
#define BLUETOOTH_VERSION_4_1                   7
/// Bluetooth Core Specification 4.2
#define BLUETOOTH_VERSION_4_2                   8
/// Bluetooth Core Specification 5.0
#define BLUETOOTH_VERSION_5_0                   9
/// Bluetooth Core Specification 5.1
#define BLUETOOTH_VERSION_5_1                   10
/// Bluetooth Core Specification 5.2
#define BLUETOOTH_VERSION_5_2                   11

/// Company Identifier - Fixed to Dialog Semiconductor ID
#define DLG_COMPANY_ID                          (0x00D2)

/// Customer Identifiers (Bit field 13-15)
#define DLG_CUSTOMER_ID_GA                      (0b000)
#define DLG_CUSTOMER_ID_HM                      (0b001)
#define DLG_CUSTOMER_ID_SA                      (0b010)
#define DLG_CUSTOMER_ID_HW                      (0b011)
#define DLG_CUSTOMER_ID_IN                      (0b111)

/// Variant identifiers (Bit field 10-12)
#define DLG_VARIANT_ID_FULL                     (0b000)
#define DLG_VARIANT_ID_EMB_LITE                 (0b001)
#define DLG_VARIANT_ID_EMB_LITE_PLUS            (0b010)

/// MACRO to build the subversion
#define DLG_SUBVERSION_BUILD(customer, variant, build)    (((customer) << 13) | (variant << 10) | (build))

/// @} DLG_VERSION


#endif // _DLG_VERSION_H_

