/**
 ****************************************************************************************
 *
 * @file custom_config.h
 *
 * @brief Custom configuration file for non-OS applications executing from RAM.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CUSTOM_CONFIG_RAM_H_
#define CUSTOM_CONFIG_RAM_H_

#include "bsp_definitions.h"

#define OS_BAREMETAL
#define SUPPRESS_HelloMsg                       (0)
#define VERIFY_QSPI_WRITE                       (1)
#define __HEAP_SIZE                             0x2000

#define dg_configUSE_LP_CLK                     ( LP_CLK_32768 )
#define dg_configCODE_LOCATION                  NON_VOLATILE_IS_NONE

#define dg_configUSE_WDOG                       (0)
#define dg_configUSE_BOD                        (0)

#define dg_configFLASH_CONNECTED_TO             (FLASH_CONNECTED_TO_1V8)
#define dg_configFLASH_POWER_DOWN               (0)
#define dg_configFLASH_AUTODETECT               (1)
#define dg_configOQSPI_FLASH_AUTODETECT         (1)

#define dg_configPOWER_1V8_ACTIVE               (1)
#define dg_configPOWER_1V8_SLEEP                (1)

#define dg_configUSE_SW_CURSOR                  (1)

#define dg_configCRYPTO_ADAPTER                 (0)

#define dg_configUSE_HW_WKUP                    (0)
#define dg_configUSE_HW_QSPI2                   (1)

#define dg_configFLASH_CONFIG_VERIFY            (1)

#define dg_configFLASH_ADAPTER                  (1)
#define dg_configNVMS_ADAPTER                   (1)
#define dg_configNVMS_VES                       (1)
#define CONFIG_PARTITION_TABLE_CREATE           (0)
#define dg_configUSE_SYS_TCS                    (1)

#define dg_configUSE_SYS_TRNG                   (0)
#define dg_configUSE_SYS_DRBG                   (0)
#define dg_configUNDISCLOSED_UNSUPPORTED_FLASH_DEVICES (1)
/* Include bsp default values */
#include "bsp_defaults.h"
/* Include middleware default values */
#include "middleware_defaults.h"

#endif /* CUSTOM_CONFIG_RAM_H_ */
