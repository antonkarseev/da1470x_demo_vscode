/**
 ****************************************************************************************
 *
 * @file ../config/8M/partition_table.h
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#define NVMS_PRODUCT_HEADER_PART_START  (OQSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00000000)
#define NVMS_PRODUCT_HEADER_PART_SIZE   (0x00002000)    /* Enough to hold primary and backup Product Headers. */

#define NVMS_PARTITION_TABLE_START      (OQSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00002000)
#define NVMS_PARTITION_TABLE_SIZE       (0x00001000)    /* Recommended location, follows the Product Headers. */

#define NVMS_FIRMWARE_PART_START        (OQSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00003000)
#define NVMS_FIRMWARE_PART_SIZE         (0x003FD000)    /* Image firmware max size ~ 4MB */

/* +------------------4MB---------------------+ */

#define NVMS_GENERIC_PART_START         (OQSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00400000)
#define NVMS_GENERIC_PART_SIZE          (0x00080000)

/* +----------------4.5MB---------------------+ */

#define NVMS_LOG_PART_START             (OQSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00480000)
#define NVMS_LOG_PART_SIZE              (0x00280000)

/* +------------------7MB---------------------+ */

#define NVMS_BIN_PART_START             (OQSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00700000)
#define NVMS_BIN_PART_SIZE              (0x000FF000)

#define NVMS_PARAM_PART_START           (OQSPI_MEM1_VIRTUAL_BASE_ADDR + 0x007FF000)
#define NVMS_PARAM_PART_SIZE            (0x00001000)    /* Recommended location, last sector of the flash device. */

/* +------------------8MB---------------------+ */

PARTITION2( NVMS_PRODUCT_HEADER_PART  , 0 )                             /* Mandatory partition - Do not relocate - Do not resize */
PARTITION2( NVMS_PARTITION_TABLE      , PARTITION_FLAG_READ_ONLY )      /* Mandatory partition - Relocate or resize at your own risk! */
PARTITION2( NVMS_FIRMWARE_PART        , 0 )                             /* Mandatory partition - Do not relocate */
PARTITION2( NVMS_GENERIC_PART         , PARTITION_FLAG_VES )            /* Optional - Suggestive position, size and flags */
                                                                        /* NOTE: The size of VES partitions may significantly increase boot-up time */
PARTITION2( NVMS_LOG_PART             , 0 )                             /* Optional - Suggestive position, size and flags */
PARTITION2( NVMS_BIN_PART             , 0 )                             /* Optional - Suggestive position, size and flags */
PARTITION2( NVMS_PARAM_PART           , 0 )                             /* Mandatory partition for NVMS parameter feature - Place at the last flash sector */


