/**
 ****************************************************************************************
 *
 * @file oqspi_memory_config_table_internal.h
 *
 * @brief Header file which contains the OQSPI memory configuration table
 *
 * When the OQSPI flash memory autodetection functionality is enabled, the SDK reads the JEDEC ID
 * of the connected devices, and compares them with the JEDEC IDs of the OQSPI flash drivers
 * contained in oqspi_memory_config_table[]. If matched, the OQSPIC is initialized with the settings
 * of the corresponding driver.
 *
 * The SDK provides the option of implementing a custom oqspi_memory_config_table[] in a new header
 * file. In this case, this file can be used as template. In order to build an application using the
 * custom oqspi_memory_config_table[], the dg_configQSPI_MEMORY_CONFIG_TABLE_HEADER must be defined
 * with the name of aforementioned header file.
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef OQSPI_FLASH_CONFIG_TABLE_INTERNAL_H_
#define OQSPI_FLASH_CONFIG_TABLE_INTERNAL_H_


#include "oqspi_mx25u6432.h"
#include "oqspi_w25q64jwim.h"
#include "oqspi_at25sl128.h"
#include "oqspi_at25ql641.h"


const oqspi_flash_config_t* oqspi_memory_config_table[] = {
        &oqspi_mx25u6432_cfg,
        &oqspi_w25q64jwim_cfg,
        &oqspi_at25sl128_cfg,
        &oqspi_at25ql641_cfg,
};

#endif /* OQSPI_FLASH_CONFIG_TABLE_INTERNAL_H_ */
