/**
 ****************************************************************************************
 *
 * @file uartboot_types.h
 *
 * @brief Common type definitions
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef UARTBOOT_TYPES_H
#define UARTBOOT_TYPES_H

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>


/*                 Memory layout
 * +=====================+==========================+
 * +        len          +          table           +
 * +=====================+==========================+
 * +    start_address    +          entry 1         +
 * +    size             +                          +
 * +    sector_size      +                          +
 * +    type             +                          +
 * +    name             +                          +
 * +=====================+==========================+
 * +        len          +          name            +
 * +                     +                          +
 * +                     +                          +
 * +                     +                          +
 * +        \0           +                          +
 * +---------------------+--------------------------+
 * +           potential   padding                  +
 * +=====================+==========================+
 * +    start_address    +          entry N         +
 * +    size             +                          +
 * +    sector_size      +                          +
 * +    type             +                          +
 * +    name             +                          +
 * +=====================+==========================+
 * +        len          +          name            +
 * +                     +                          +
 * +        \0           +                          +
 * +---------------------+--------------------------+
 * +                                                +
 * +           potential   padding                  +
 * +                                                +
 * +=====================+==========================+
 */

 /**
  * \brief Partition name buffer structure
  *
  */
typedef struct {
        uint16_t len;                   /**< Name length in bytes (including '\0') */
        uint8_t str;                    /**< Partition name (characters array)  */
} cmd_partition_name_t;

/**
 * \brief Partition entry structure
 *
 */
typedef struct {
        uint32_t start_address;         /**< Start address */
        uint32_t size;                  /**< Size */
        uint16_t sector_size;           /**< Sector size - can be aligned with FLASH sector e.g. 4KB */
        uint8_t  type;                  /**< Partition ID \sa nvms_partition_id_t */
        cmd_partition_name_t name;      /**< Partition name buffer */
} cmd_partition_entry_t;

/**
 * \brief Partition table structure
 *
 */
typedef struct {
        uint16_t len;                   /**< Size of the whole structure in bytes */
        cmd_partition_entry_t entry;    /**< Flexible array with all partition entries */
} cmd_partition_table_t;


/**
 * \brief Product information structure
 *
 */
typedef struct {
        uint16_t len;   /**< Size of product information in bytes */
        char     str;   /**< Product information (character array) */
} cmd_product_info_t;

#ifdef __cplusplus
}
#endif

#endif /* UARTBOOT_TYPES_H */
