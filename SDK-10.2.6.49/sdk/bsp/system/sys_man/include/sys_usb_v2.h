/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup USB_SERVICE USB System Service
 *
 * \brief System USB
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_usb_v2.h
 *
 * @brief System USB header file.
 *
 * Copyright (C) 2018-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef SYS_USB_V2_H_
#define SYS_USB_V2_H_

#include <sdk_defs.h>
#include <hw_usb.h>

#if dg_configUSE_USB_ENUMERATION
/**
 * \brief USB driver configuration
 *
 * Configuration of  USB low level driver(s)
 *
 * \note Only DMA-specific configuration is currently applicable
 *
 */
typedef struct {
#if HW_USB_DMA_SUPPORT
        usb_config lld;                 /**< Low level driver configuration */
        bool       acquired_dma;        /**< Flag indicating the TX & RX DMA channel acquisition.
                                             Read-only: This flag is updated automatically by the USB service. */
#endif
} sys_usb_conf_t;

/**
 * \brief USB driver configuration
 *
 * \deprecated  This struct is deprecated, consider using sys_usb_conf_t instead,
 *              which contains a USB driver configuration.
 */
DEPRECATED_MSG("This struct is deprecated, consider using sys_usb_conf_t instead.")
typedef sys_usb_conf_t sys_usb_driver_conf_t;

/**
 * \brief Initialize sys_usb service.
 *
 * \param[in] cfg the configuration of the USB service for data transfer
 */
void sys_usb_cfg(const sys_usb_conf_t *cfg);

/**
 * \brief Finalize the attach procedure.
 *        Called from sys_usb or sys_charger, depending on configuration
 *
 */
void sys_usb_finalize_attach(void);
#endif /* dg_configUSE_USB_ENUMERATION */

/**
 * \brief Initialize USB and VBUS events handling subsystem.
 * The Function creates the sys_usb task which is the recipient
 * of the USB events and VBUS events.
 * The VBUS events are the starting point for both Charger and USB-Data
 * functionality.
 * When the USB-Data functionality is enabled the sys_usb_init()
 * initialize also the call-backs between the emUSB and the Lower Level
 * USB/VBUS handling (LLD, sys_usb).
 * If only Charger functionality is enabled then only the VBUS events are
 * handled by the Lower level USB/VBUS level
 *
 * The USB/Charger interrupt is enabled as last action in the sys_usb_init
 */
void sys_usb_init(void);

/******************** Weak functions to be implemented, if needed, by the application code ********/

/**
 * \brief Notification hook called when VBUS is attached.
 *        This optional callback can be implemented by the application
 *        and used to get the VBUS plug-in event notification at
 *        application level.
 *
 * \note  In the case of data enumeration, the application code
 *        should trigger the data enumeration actions from
 *        \ref sys_usb_ext_hook_begin_enumeration() (and not from
 *        \ref sys_usb_ext_hook_attach()).
 *        The application code must not block or execute
 *        for long time in \ref sys_usb_ext_hook_attach(). */
__WEAK void sys_usb_ext_hook_attach(void);

/**
 * \brief Notification hook called when VBUS is detach.
 *        This optional callback can be implemented by the application
 *        and used to get the VBUS plug-out event notification at
 *        application level.
 *
 * \note  In the case of data enumeration, the application code
 *        should trigger from \ref sys_usb_ext_hook_detach() any actions
 *        required to stop the USB data functionality at application level.
 *        The application code must not block or execute
 *        for long time in \ref sys_usb_ext_hook_detach().
 */
__WEAK void sys_usb_ext_hook_detach(void);

/**
 * \brief Notification hook to be called when charger detection,
 * if any, is completed, and the device can begin enumeration.
 *
 */
__WEAK void sys_usb_ext_hook_begin_enumeration(void);

#endif /* SYS_USB_V2_H_ */


/**
\}
\}
*/
