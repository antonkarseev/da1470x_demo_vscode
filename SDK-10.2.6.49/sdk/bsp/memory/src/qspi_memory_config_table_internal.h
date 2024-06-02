/**
 ****************************************************************************************
 *
 * @file qspi_memory_config_table_internal.h
 *
 * @brief Header file which contains the QSPI memory configuration table
 *
 * When the memory autodetection functionality is enabled, the SDK reads the JEDEC ID of the
 * connected devices, and compares them with the JEDEC IDs of the QSPI flash/PSRAM drivers
 * contained in qspi_memory_config_table[]. If matched, the QSPIC/QSPIC2 are initialized with
 * the settings of the corresponding drivers(s).
 *
 * The SDK provides the option of implementing a custom qspi_memory_config_table[] in a new header
 * file. In this case, this file can be used as template. In order to build an application using the
 * custom qspi_memory_config_table[], the dg_configQSPI_MEMORY_CONFIG_TABLE_HEADER must be defined
 * with the name of aforementioned header file.
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef QSPI_FLASH_CONFIG_TABLE_INTERNAL_H_
#define QSPI_FLASH_CONFIG_TABLE_INTERNAL_H_

#include "qspi_at25sl128.h"
#include "qspi_w25q256jw.h"
#include "psram_aps6404jsq.h"



const qspi_flash_config_t* qspi_memory_config_table[] = {
        &flash_at25sl128_config,
        &flash_w25q256jw_config,
        &psram_aps6404jsq_config,


};

#endif /* QSPI_FLASH_CONFIG_TABLE_INTERNAL_H_ */
