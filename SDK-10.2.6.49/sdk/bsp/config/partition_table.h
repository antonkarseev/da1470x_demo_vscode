/**
 ****************************************************************************************
 *
 * @file bsp/config/partition_table.h
 *
 * @brief Partition table selection. Image partition's size definition.
 *
 * Copyright (C) 2016-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/*
 * When partition_table is not overridden by adding a custom partition_table.h file to a project
 * then this file is used to select partition table by macro definition.
 *
 * To use layout other than SDK one, add include path into the project settings that will point
 * to a folder with the custom partition_table file.
 */

/*
 * Helper macro to set the start of a partition right after the end of another partition.
 * It assumes that the following naming pattern is followed:    NVMS_FOO_PART_START, NVMS_FOO_PART_SIZE
 */
#define PLACE_AFTER(__part__)           (NVMS_ ## __part__ ## _PART_START + NVMS_ ## __part__ ## _PART_SIZE)

#if defined(USE_PARTITION_TABLE_8MB_WITH_SUOTA)
#include <8M/suota/partition_table.h>
#elif defined(USE_PARTITION_TABLE_128MB_WITH_SUOTA)
#include <128M/suota/partition_table.h>
#elif defined(USE_CUSTOM_PARTITION_TABLE)
#include <custom/partition_table.h>
#else
/*
 * The default option is the 8M-sized partition scheme.
 * A 128M scheme is also available under the respective directory.
 */
#include <8M/partition_table.h>
#endif

/*
 * Define a maximal size of the image which could be written to QSPI - based on the partition sizes.
 */
#if defined(NVMS_FW_EXEC_PART_SIZE) && defined(NVMS_FW_UPDATE_PART_SIZE)
#if NVMS_FW_EXEC_PART_SIZE < NVMS_FW_UPDATE_PART_SIZE
#define IMAGE_PARTITION_SIZE    NVMS_FW_EXEC_PART_SIZE
#else
#define IMAGE_PARTITION_SIZE    NVMS_FW_UPDATE_PART_SIZE
#endif /* NVMS_FW_EXEC_PART_SIZE < NVMS_FW_UPDATE_PART_SIZE */
#elif defined(NVMS_FIRMWARE_PART_SIZE)
#define IMAGE_PARTITION_SIZE    NVMS_FIRMWARE_PART_SIZE
#else
#error "At least one partition where application could be placed should be defined!"
#endif /* defined(NVMS_FW_EXEC_PART_SIZE) && defined(NVMS_FW_UPDATE_PART_SIZE) */
