/**
 ****************************************************************************************
 *
 * @file co_version.h
 *
 * @brief Version definitions for BT4.0
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 * Copyright (c) 2015-2021 Modified by Dialog Semiconductor
 *
 *
 ****************************************************************************************
 */


#ifndef _DA14690_CO_VERSION_H_
#define _DA14690_CO_VERSION_H_
/**
 ****************************************************************************************
 * @defgroup CO_VERSION Version Defines
 * @ingroup COMMON
 *
 * @brief Bluetooth Controller Version definitions.
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "dlg_version.h"    // Version definitions specific to Dialog Semiconductor

/*
 * New Versioning Scheme according to [SDKDEVA-2860]
 * Used in:
 * - HCI Subversion (hci_subverision field of the HCI_Read_Local_Version_Information command response)
 * - LMP Subversion (LMP_PAL_Subversion field of the HCI_Read_Local_Version_Information command response)
 * - Link Layer Subversion (SubVersNr field of the LL_VERSION_IND packet)
 */

/// Configure Bluetooth Version
#define BLUETOOTH_VERSION                       BLUETOOTH_VERSION_5_2

/// Configure Customer Identifier
#define CUSTOMER_ID                             DLG_CUSTOMER_ID_GA

/// Configure Variant identifier
#define VARIANT_ID                              DLG_VARIANT_ID_FULL

/// Configure Build Identifier
/// Build Identifiers used for Bluetooth Core Specification 5.2 version
#define BUILD_ID                                (32)


/// Company Identifier (Fixed to Dialog Semiconductor ID, should not be changed)
#define COMPANY_ID                              DLG_COMPANY_ID

/// Calculate HCI, LMP and LL versions
#define HCI_LMP_LL_VERSION                      BLUETOOTH_VERSION
/// Calculate HCI, LMP and LL subversions
#define HCI_LMP_LL_SUBVERSION                   DLG_SUBVERSION_BUILD(CUSTOMER_ID, VARIANT_ID, BUILD_ID)

/*
 * Old Versioning Scheme for internal use only
 */
/// RWBT SW Major Version
#define RWBLE_SW_VERSION_MAJOR                  BLUETOOTH_VERSION
/// RWBT SW Minor Version
#define RWBLE_SW_VERSION_MINOR                  0
/// RWBT SW Build Version
#define RWBLE_SW_VERSION_BUILD                  1
/// RWBT SW Major Version
#define RWBLE_SW_VERSION_SUB_BUILD              0

/// Combine major and minor version numbers into a single value
/// Assumes that minor numbers never exceed 100
#define RWBLE_SW_VERSION (RWBLE_SW_VERSION_MAJOR * 100 + RWBLE_SW_VERSION_MINOR)
/// Macro that produces version literals as (major, minor) pairs
#define MK_VERSION(maj, min) ((maj) * 100 + (min))
#define VERSION_8_0                             MK_VERSION(8, 0)
#define VERSION_8_1                             MK_VERSION(8, 1)
#define VERSION_9_0                             MK_VERSION(9, 0)
#define VERSION_10_0                            MK_VERSION(10, 0)


/// @} CO_VERSION


#endif // _DA14690_CO_VERSION_H_

