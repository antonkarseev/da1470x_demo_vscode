/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
  * \addtogroup PLA_MEMORY
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file oqspi_automode.h
 *
 * @brief OQSPI flash memory automode API header file.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef OQSPI_AUTOMODE_H_
#define OQSPI_AUTOMODE_H_

#if dg_configUSE_HW_OQSPI

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sdk_defs.h>
#include "hw_oqspi.h"
#include "hw_clk.h"
#include "oqspi_common.h"

/*
 * Defines (generic)
 */
#define OQSPI_FLASH_PAGE_SIZE                   (0x100)
#define OQSPI_FLASH_SECTOR_SIZE                 (FLASH_SECTOR_SIZE)

/**
 * \brief Write flash memory
 *
 * This function allows to write up to page size of data to flash.
 * If size is greater than page size, flash can wrap data and overwrite content of page.
 * It's possible to write less than page size.
 * Memory should be erased before.
 *
 * \warning Do not pass buf pointing to OQSPI mapped memory.
 *
 * \param [in] addr offset in flash to write data to
 * \param [in] buf pointer to data to write
 * \param [in] size number of bytes to write
 *
 * return number of bytes written
 *
 * \warning This function must not be used when the flash background operations are enabled.
 *          Consider using sys_background_flash_ops_write_page() instead.
 *
 */
__RETAINED_CODE uint32_t oqspi_automode_write_flash_page(uint32_t addr, const uint8_t *buf, uint32_t size);

/**
 * \brief Erase Flash Sector
 *
 * The sector will be erased either in auto or in manual access mode depending
 * on dgconfigOQSPI_ERASE_IN_AUTOMODE.
 *
 * \param [in] addr The address of the sector to be erased.
 *
 * \warning This function must not be used when the flash background operations are enabled.
 *          Consider using sys_background_flash_ops_erase_sector() instead.
 *
 * \sa dgconfigOQSPI_ERASE_IN_AUTOMODE
 */
__RETAINED_CODE void oqspi_automode_erase_flash_sector(uint32_t addr);

/**
 * \brief Erase whole chip
 */
void oqspi_automode_erase_chip(void);

/**
 * \brief Read memory
 *
 * \param [in] addr starting offset
 * \param [out] buf buffer to read data to
 * \param [in] len number of bytes to read
 *
 * \returns number of bytes read
 */
uint32_t oqspi_automode_read(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * \brief Get the OQSPI flash memory physical address of the corresponding virtual address.
 *
 * \param [in] virtual_addr The OQSPI flash memory virtual address for which the physical address
 *                          will be returned.
 *
 * \returns The OQSPI flash memory physical address of the corresponding virtual address.
 */
const void *oqspi_automode_get_physical_addr(uint32_t virtual_addr);

/**
 * \brief Power up flash
 */
__RETAINED_CODE void oqspi_automode_flash_power_up(void);

/**
 * \brief Set OQSPI Flash into power down mode
 */
__RETAINED_CODE void oqspi_automode_flash_power_down(void);

/**
 * \brief Init OQSPI controller
 *
 * \return true if the OQSPIC initialization succeeds, otherwise false.
 */
__RETAINED_CODE bool oqspi_automode_init(void);

/**
 * \brief Configure Flash and OQSPI controller for system clock frequency
 *
 * This function is used to change the Flash configuration of the OQSPI controller
 * to work with the system clock frequency defined in sys_clk. Dummy clock
 * cycles could be changed here to support higher clock frequencies.
 * OQSPI controller clock divider could also be changed if the Flash
 * maximum frequency is smaller than the system clock frequency.
 * This function must be called before changing system clock frequency.
 *
 * \param [in] sys_clk System clock frequency
 *
 */
__RETAINED_CODE void oqspi_automode_sys_clock_cfg(sys_clk_t sys_clk);

/**
 * \brief Verified that passed address is valid and physically available
 *
 * \param [in] addr starting offset
 */
__RETAINED_CODE bool oqspi_is_valid_addr(uint32_t addr);

/**
 * \brief Get maximum available memory size for selected controller id
 *
 * \return Maximum memory size counted in bytes
 */
uint32_t oqspi_get_device_size(void);

/**
 * \brief Get the JEDEC ID parameters of the OQSPI flash driver.
 *
 * \param [out] jedec Pointer to a JEDEC ID structure where the JEDEC parameters will be returned
 *                    (manufacturer ID, type, density)
 *
 * \return true if the device is marked as present, false otherwise
 */
bool oqspi_get_config(jedec_id_t *jedec);

/**
 * \brief Read the JEDEC ID parameters (manufacturer ID, type, density).
 *
 * \param [out] jedec Pointer to a JEDEC ID structure where the JEDEC parameters will be returned
 *                    (manufacturer ID, type, density)
 *
 * \return True if JEDEC ID has been successfully read, otherwise false.
 *
 * \warning The criteria for successful JEDEC ID reading depend on whether one of the following
 *          compilation options are enabled or not.
 *
 *          - dg_configOQSPI_FLASH_AUTODETECT = 1: The read JEDEC ID is compared to the JEDEC IDs
 *          of all supported memories, and if one of them matches, the function returns true. On top
 *          of that, the oqspi_flash_config is equalized with the flash driver configuration structure
 *          of the detected memory. On the contrary, if none of the JEDEC IDs matches, the function
 *          returns false.
 *
 *          - dg_configOQSPI_FLASH_CONFIG_VERIFY = 1: The read JEDEC ID is compared to the JEDEC ID
 *          of the memory, for which the application is built for, and if it matches, the function
 *          returns true. Otherwise, returns false.
 *
 *          - dg_configOQSPI_FLASH_AUTODETECT = dg_configOQSPI_FLASH_CONFIG_VERIFY = 0 (default):
 *          Is checked that the first byte (manufacturer_id) of read JEDEC ID sequence does not
 *          equal to 0xFF and 0x00. If this condition is fulfilled, the JEDEC ID is considered valid
 *          and the function returns true. Otherwise, it returns false. Since the aforementioned
 *          condition is relatively weak, it is highly recommended to apply a thorougher check
 *          afterwards.
 *
 *          Should also be noted that the aforementioned options are mutually exclusive, thus cannot
 *          be enabled both at the same time.
 *
 * \warning This function will reset the connected OQSPI flash memory to make sure that the memory
 *          is in single bus mode.
 *
 * \warning The function leaves the OQSPIC in auto access mode.
 *
 * \warning The Read JEDEC ID command is limited at a specific OQSPIC clock frequency, thus if the
 *          first attempt fails, the function switches the OQSPIC clock divider to max (8) and tries
 *          again. However, it is responsibility of the caller to make sure that the system clock is
 *          set properly before calling this function. For instance, if the system clock is PLL160 the
 *          second attempt to read the jedec id will use OQSPI clock frequency = 160MHz / 8 = 20MHz.
 *          If the maximum allowed clock frequency is 8MHz, both attempts will fail.
 */
__RETAINED_CODE bool oqspi_read_flash_jedec_id(jedec_id_t *jedec);

#endif /* dg_configUSE_HW_OQSPI */
#endif /* OQSPI_AUTOMODE_H_ */
/**
 * \}
 * \}
 */
