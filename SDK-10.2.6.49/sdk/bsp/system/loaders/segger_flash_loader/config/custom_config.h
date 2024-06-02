/**
 ****************************************************************************************
 *
 * @file custom_config.h
 *
 * @brief Custom configuration file for non-FreeRTOS applications executing from RAM.
 *
 * Copyright (C) 2018-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CUSTOM_CONFIG_RAM_H_
#define CUSTOM_CONFIG_RAM_H_

#include "bsp_definitions.h"

#define OS_BAREMETAL

#define __HEAP_SIZE                             0x2000

#define dg_configUSE_LP_CLK                     ( LP_CLK_32768 )
#define dg_configCODE_LOCATION                  NON_VOLATILE_IS_NONE

#define dg_configIMAGE_SETUP                    DEVELOPMENT_MODE

#define dg_configUSE_WDOG                       (0)
#define dg_configUSE_BOD                        (0)

#define dg_configFLASH_CONNECTED_TO             (FLASH_CONNECTED_TO_1V8)
#define dg_configFLASH_POWER_DOWN               (0)
#define dg_configFLASH_AUTODETECT               (1)

#define dg_configPOWER_1V8_ACTIVE               (1)
#define dg_configPOWER_1V8_SLEEP                (1)

#define dg_configUSE_SW_CURSOR                  (1)

#define dg_configCRYPTO_ADAPTER                 (0)

#define dg_configUSE_HW_WKUP                    (0)

#define dg_configFLASH_MAX_WRITE_SIZE           (256)

#define dg_configFLASH_ADAPTER                  (1)
#define dg_configNVMS_ADAPTER                   (1)
#define dg_configNVMS_VES                       (1)
#define CONFIG_PARTITION_TABLE_CREATE           (0)

#define dg_configUSE_SYS_TRNG                   (0)
#define dg_configUSE_SYS_DRBG                   (0)

// segger_flash_loader compilation options
//
// Only compile in functions that make sense to keep RAMCode as small as possible
//
// Non-memory mapped flashes only. Flash cannot be read memory-mapped
// If enabled, the J-Link will make use of the Verify() in order to verify flash contents.
// Otherwise, the built-in memory-mapped readback function will be used.
#define SUPPORT_NATIVE_VERIFY                   (0)
// Non-memory mapped flashes only. Flash cannot be read memory-mapped
// If enabled, the J-Link will make use of the SEGGER_OPEN_Read() in order to read a specified number
// of bytes from flash. Otherwise, the built-in memory-mapped readback function will be used.
#define SUPPORT_NATIVE_READ_FUNCTION            (0)
// To potentially speed up production programming: Erases whole flash bank / chip with special command
#define SUPPORT_ERASE_CHIP                      (1)
// Currently available for Cortex-M only
#define SUPPORT_TURBO_MODE                      (1)
// Flashes with uniform sectors only. Speed up erase because 1 OFL call may erase multiple sectors
#define SUPPORT_SEGGER_OPEN_ERASE               (1)
// If disabled, the J-Link software will assume that erased state of a sector can be determined via
// normal memory-mapped readback of sector.
#define SUPPORT_BLANK_CHECK                     (0)

#define SEGGER_FLASH_LOADER_DEBUG               (0)

#include "bsp_defaults.h"

#endif /* CUSTOM_CONFIG_RAM_H_ */
