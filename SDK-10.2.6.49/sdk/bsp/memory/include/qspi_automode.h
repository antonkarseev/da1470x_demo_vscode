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
 * @file qspi_automode.h
 *
 * @brief Access QSPI device when running in auto mode
 *
 * Copyright (C) 2016-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef QSPI_AUTOMODE_H_
#define QSPI_AUTOMODE_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <sdk_defs.h>
#include "hw_qspi.h"
#include "hw_clk.h"

#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)

/* Macros to put functions that need to be copied to ram in one section (retained) */
typedef struct qspi_ucode_s {
       const uint32_t *code;
       uint8_t size;
} qspi_ucode_t;

#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */


#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)

/**
 * \brief Write flash memory
 *
 * This function allows to write up to page size of data to flash.
 * If size is greater than page size, flash can wrap data and overwrite content of page.
 * It's possible to write less then page size.
 * Memory should be erased before.
 *
 * \note: Do not pass buf pointing to QSPI mapped memory.
 *
 * \param [in] addr offset in flash to write data to
 * \param [in] buf pointer to data to write
 * \param [in] size number of bytes to write
 *
 * return number of bytes written
 *
 */
uint32_t qspi_automode_write_flash_page(uint32_t addr, const uint8_t *buf, uint32_t size);

/**
 * \brief Erase flash sector
 *
 * \param [in] addr starting offset of sector
 */
void qspi_automode_erase_flash_sector(uint32_t addr);

/**
 * \brief Erase whole chip
 */
void qspi_automode_erase_chip(void);

/**
 * \brief Erase specific flash
 *
 * \param [in] id QSPI controller id
 *
 * \return false if the QSPI controller with \p id cannot be used or if a RAM device
 *              is connected to it; true otherwise
 */
bool qspi_automode_erase_chip_by_id(HW_QSPIC_ID id);

/**
 * \brief Read memory
 *
 * \param [in] addr starting offset
 * \param [out] buf buffer to read data to
 * \param [in] len number of bytes to read
 *
 * \returns number of bytes read
 */
uint32_t qspi_automode_read(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * \brief Get address of memory
 *
 * \param [in] addr starting offset
 *
 * \returns address in CPU address space where data is located
 */
const void *qspi_automode_addr(uint32_t addr);

#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */

/**
 * \brief Power up flash
 */
__RETAINED_CODE void qspi_automode_flash_power_up(void);

/**
 * \brief Set QSPI Flash into power down mode
 */
__RETAINED_CODE void qspi_automode_flash_power_down(void);

/**
 * \brief Init QSPI controller
 */
__RETAINED_CODE bool qspi_automode_init(void);

#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)

#if (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
/**
 * \brief Check if a program or sector erase operation is in progress
 *
 * \param[in] id QSPI controller id
 *
 * \return bool True if the BUSY bit is set else false.
 *
 * \warning This function checks the value of the BUSY bit in the Status Register 1 of the Flash. It
 *        is the responsibility of the caller to call the function in the right context. The
 *        function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE bool qspi_check_program_erase_in_progress(HW_QSPIC_ID id);

/**
 * \brief Resume a Flash program or sector erase operation
 *
 * \param[in] id QSPI controller id
 *
 * \warning After the call to this function, the QSPI controller is set to manual mode and the Flash
 *        access to single mode. The function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE void qspi_resume(HW_QSPIC_ID id);

/**
 * \brief Erase a sector of the Flash in manual mode
 *
 * \param[in] addr The address of the sector to be erased.
 *
 * \warning This function does not block until the Flash has processed the command! The QSPI
 *        controller is left to manual mode after the call to this function. The function must be
 *        called with interrupts disabled.
 *
 */
__RETAINED_CODE void flash_erase_sector_manual_mode(uint32_t addr);

/**
 * \brief Program data into a page of the Flash in manual mode
 *
 * \param[in] addr The address of the Flash where the data will be written. It may be anywhere in a
 *        page.
 * \param[in] buf Pointer to the beginning of the buffer that contains the data to be written.
 * \param[in] len The number of bytes to be written.
 *
 * \return The number of bytes written.
 *
 * \warning The boundary of the page where addr belongs to, will not be crossed! The caller should
 *        issue another flash_program_page_manual_mode() call in order to write the remaining data
 *        to the next page. The QSPI controller is left to manual mode after the call to this
 *        function. The function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE uint32_t flash_program_page_manual_mode(uint32_t addr, const uint8_t *buf,
                                                        uint32_t len);
#endif

#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */

/**
 * \brief Configure Flash and QSPI controller for system clock frequency
 *
 * This function is used to change the Flash configuration of the QSPI controller
 * to work with the system clock frequency defined in sys_clk. Dummy clock
 * cycles could be changed here to support higher clock frequencies.
 * QSPI controller clock divider could also be changed if the Flash
 * maximum frequency is smaller than the system clock frequency.
 * This function must be called before changing system clock frequency.
 *
 * \param [in] sys_clk System clock frequency
 *
 */
__RETAINED_CODE void qspi_automode_sys_clock_cfg(sys_clk_t sys_clk);

#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)


/**
 * \brief Verified if passed adder is valid and physically available
 *
 * \param [in] addr starting offset
 */
__RETAINED_CODE bool qspi_is_valid_addr(uint32_t addr);

/**
 * \brief Get maximum available memory size for selected controller id
 *
 * \param[in] id QSPI controller id
 *
 * \return Maximum memory size counted in bytes
 */
uint32_t qspi_get_device_size(const HW_QSPIC_ID id);

/**
 * \brief Get configuration parameters for selected controller id
 *
 * \param [in]          id                    QSPI controller id
 * \param [out]         manufacturer_id       Manufacturer ID
 * \param [out]         device_type           Device type
 * \param [out]         density               Device density
 *
 * \return true if the device is marked as present, false otherwise
 */
bool qspi_get_config(const HW_QSPIC_ID id, uint8_t *manufacturer_id,
                     uint8_t *device_type, uint8_t *density);

/**
 * \brief Read the JEDEC manufacturer ID, device type and device density using command 0x9F
 *
 * \param [in]          id                      QSPI controller id
 * \param [out]         manufacturer_id         Pointer to the variable where the manufacturer ID
 *                                              will be returned
 * \param [out]         device_type             Pointer to the variable where the device type will
 *                                              be returned
 * \param [out]         density                 Pointer to the variable where the device density
 *                                              will be returned
 *
 * \return true if JEDEC Id has been read successfully, false otherwise
 *
 * \warning this function will reset FLASH device connected to the specified QSPI controller
 */
__RETAINED_CODE bool qspi_read_flash_jedec_id(HW_QSPIC_ID id, uint8_t *manufacturer_id,
                                                        uint8_t *device_type, uint8_t *density);

/**
 * \brief Check that connected device is an external RAM
 *
 * \param [in]          id                      QSPI controller id
 *
 * \return true if the connected device is RAM, false otherwise
 *
 */
__RETAINED_CODE bool qspi_is_ram_device(const HW_QSPIC_ID id);
#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */

#endif /* QSPI_AUTOMODE_H_ */
/**
 * \}
 * \}
 */
