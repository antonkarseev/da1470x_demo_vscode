/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup MID_SYS_SER_FLASH_BACKGROUND_OPS Flash Background Operations
 * \brief System service to execute background flash operations.
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_background_flash_ops.h
 *
 * @brief Background flash operation API
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_BACKGROUND_FLASH_OPS_H_
#define SYS_BACKGROUND_FLASH_OPS_H_

#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS

/**
 * \brief Erase sector using the background flash operations mechanism
 *
 * \param [in] addr     Virtual address of the sector to be erased.
 *
 * \note This function substitutes the oqspi_automode_erase_flash_sector() when the background flash
 *       operations are enabled.
 *
 */
__RETAINED_CODE void sys_background_flash_ops_erase_sector(uint32_t addr);

/**
 * \brief Write flash page using the background operations.
 *
 * \param [in] addr     Virtual address of the flash memory, where the contents pointed by \p src is
 *                      to be written.
 * \param [in] src      Pointer to the source of data to be written.
 * \param [in] size     Number of bytes to be written.
 *
 * return Number of written bytes.
 *
 * \note This function substitutes the oqspi_automode_write_flash_page() when the background flash
 *       operations are enabled.
 */
uint32_t sys_background_flash_ops_write_page(uint32_t addr, const uint8_t *src, uint32_t size);

#endif /* dg_configUSE_SYS_BACKGROUND_FLASH_OPS */

#endif /* SYS_BACKGROUND_FLASH_OPS_H_ */

/**
 \}
 \}
 */
