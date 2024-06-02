/**
 \addtogroup PLA_BSP_SYSTEM
 \{
 \addtogroup PLA_MEMORY
 \{
 \addtogroup INTERNAL
 \{
 */

/**
 ****************************************************************************************
 *
 * @file oqspi_automode_internal.h
 *
 * @brief OQSPI flash memory automode internal header file.
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef OQSPI_AUTOMODE_INTERNAL_H_
#define OQSPI_AUTOMODE_INTERNAL_H_

/**
 * \brief Switch the OQSPIC to auto access mode.
 */
__RETAINED_CODE void oqspi_automode_int_enter_auto_access_mode(void);

/**
 * \brief Write data to flash memory
 *
 * \param[in] addr      Address of the flash memory, where the content of the buf is to be written.
 * \param[in] buf       Pointer to the source of data to be written.
 * \param[in] size      Number of bytes to write.
 *
 * \return Number of written bytes by the function call.
 *
 * \warning This function switches and leaves the OQSPIC to manual access mode. Therefore, it must
 *          be called with disabled interrupts. It's up to the caller to switch the OQSPIC back to
 *          auto access mode, in order to re-enable XiP.
 *
 * \warning The write operation will not exceed the page boundary. Thus, It's up to the caller to
 *          issue another call of the function, in order to write the remaining data to the next page.
 */
__RETAINED_CODE uint32_t oqspi_automode_int_flash_write_page(uint32_t addr, const uint8_t *buf, uint32_t size);

#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS

/**
 * \brief Resume erase/write flash operation.
 *
 * \warning This function switches and leaves the OQSPIC to manual access mode. Therefore, it must
 *          be called with disabled interrupts. It's up to the caller to switch the OQSPIC back to
 *          auto access mode, in order to re-enable XiP.
 */
__RETAINED_CODE void oqspi_automode_int_resume(void);

/**
 * \brief suspend erase/write flash operation.
 *
 * \warning This function switches and leaves the OQSPIC to manual access mode. Therefore, it must
 *          be called with disabled interrupts. It's up to the caller to switch the OQSPIC back to
 *          auto access mode, in order to re-enable XiP.
 */
__RETAINED_CODE void oqspi_automode_int_suspend(void);

/**
 * \brief Check whether the flash memory is suspended.
 *
 * \warning Before calling this function, the caller has to disable the interrupts and switch the
 *          OQSPIC to manual access mode.
 *
 * \return True, if the flash memory is suspended, otherwise false.
 */
__RETAINED_CODE bool oqspi_automode_int_is_suspended(void);

/**
 * \brief Check whether the flash memory is busy.
 *
 * \warning This function switches and leaves the OQSPIC to manual access mode. Therefore, it must
 *          be called with disabled interrupts. It's up to the caller to switch the OQSPIC back to
 *          auto access mode, in order to re-enable XiP.
 *
 * \return True, if the flash memory is busy, otherwise false.
 */
__RETAINED_CODE bool oqspi_automode_int_is_busy(void);

#endif /* dg_configUSE_SYS_BACKGROUND_FLASH_OPS */

#endif /* OQSPI_AUTOMODE_INTERNAL_H_ */

/**
 \}
 \}
 \}
 */
