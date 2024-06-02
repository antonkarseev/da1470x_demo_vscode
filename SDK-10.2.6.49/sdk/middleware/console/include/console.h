/**
 * \addtogroup UTILITIES
 * \{
 * \addtogroup UTI_CONSOLE Console
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file console.h
 *
 * @brief Definition of the API for the Console utilities service
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#if dg_configUSE_CONSOLE

#include "ad_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

#if !dg_configUSE_CONSOLE_STUBS

/*
 * The console utilities service provides support for serial input and output (even from
 * an interrupt context) by employing the UART adapter for reading and writing, and through
 * one of the UART peripheral HW instances.
 *
 * When attempting to capture serial input or print a string from within an application task
 * or interrupt, the libC standard _read()/_write() are redirected to console_read()/write().
 * To allow these reads/writes, console uses it's own task (console_task_func()) that does
 * the actual UART hardware access through the UART adapter (ad_uart_read/write_async()).
 * Synchronization between the console task and the main execution domain is achieved via an
 * OS-based set of events and notifications.
 *
 * Notes:
 * - Console does not use additional RAM for printing. RAM that is used is allocated during
 *   initialization only.
 * - If data flow is too fast for UART, calls from task will wait, while calls from interrupt
 *   may lose some data.
 * - When the console service and CONFIG_RETARGET are both enabled, the serial input and output
 *   activity will be automatically handled by console instead of standard retarget implementation.
 * - In case of printing, it must be assured that the HW UART CTS line/pin of the USB/UART IC
 *   converter is properly connected to the relevant M33 GPIO pin. Additionally, that an IRQ handler
 *   is defined to be triggered upon a CTS event. See the console_wkup_handler() description for more.
 */

/**
 * \brief Console task priority
 */
#ifndef CONSOLE_TASK_PRIORITY
#define CONSOLE_TASK_PRIORITY       (OS_TASK_PRIORITY_NORMAL)
#endif

/**
 * \brief Initialize console to use with specified serial device
 *
 * This function will allocate all necessary resources for serial console (RAM, task,
 * synchronization primitives).
 *
 * \param [in] ad_uart_ctrl_conf controller configuration
 *
 */
void console_init(const ad_uart_controller_conf_t *ad_uart_ctrl_conf);

/**
 * \brief Write to serial console
 *
 * This function can be called from normal task as well as interrupts.
 * When called from interrupts this function will not block. If buffer can't hold all requested
 * data some data will be dropped and will never leave UART.
 * When called from a task, this function can block if there is no space in buffer to hold data.
 *
 * \param [in] buf pointer to data to send to serial console
 * \param [in] len number of bytes to print
 *
 * \return number of bytes written
 */
int console_write(const char *buf, int len);

/**
 * \brief Read from serial console
 *
 * Call this function to read data from serial console.
 *
 * \param [out] buf pointer to buffer to fill with data from serial console
 * \param [in] len number of bytes to read
 *
 */
int console_read(char *buf, int len);

/**
 * \brief Wake up handler for serial console
 *
 * It shall be called when the UART CTS GPIO pin is asserted to signal this event for the console task.
 * Imperative setup actions in order to use it:
 * - Assure that the HW UART CTS line/pin of the USB/UART IC converter (e.g. FT2232HT in DA1469x ProDK)
 *  is properly connected to the relevant M33 GPIO pin.
 * - As part of the application's HW initialization code, configure waking-up the system when the UART CTS
 *  GPIO pin is low and register a related callback that will be triggered in such a case.
 * - The callback must check the status of the pin and if asserted it shall invoke the
 *  console_wkup_handler() and then clear again the pin before exiting.
 *
 * Note: The UART CTS line/pin is of reverse logic, i.e. it is asserted when low (logic 0).
 */
void console_wkup_handler(void);

#else /* !dg_configUSE_CONSOLE_STUBS */

__STATIC_INLINE void console_init(void)
{

}

__STATIC_INLINE int console_write(const char *buf, int len)
{
        return len;
}

__STATIC_INLINE int console_read(char *buf, int len)
{
        return 0;
}

#endif /* !dg_configUSE_CONSOLE_STUBS */

#ifdef __cplusplus
}
#endif

#endif /* dg_configUSE_CONSOLE */

#endif /* CONSOLE_H_ */

/**
 * \}
 * \}
 */
