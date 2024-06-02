/**
 * \addtogroup PLA_DRI_PER_COMM
 * \{
 * \addtogroup HW_USB_CHARGER
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_usb_charger_v2.h
 *
 * @brief Implementation of the USB Charger Low Level Driver.
 *
 * Copyright (C) 2018-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_USB_CHARGER_V2_H_
#define HW_USB_CHARGER_V2_H_


#if dg_configUSE_HW_USB_CHARGER

#include <sdk_defs.h>

/**
 * \enum HW_USB_CHARGER_PRIMARY_CONN_TYPE
 * \brief Primary charger detection result.
 *
 */
typedef enum {
        HW_USB_CHARGER_PRIMARY_CONN_TYPE_NONE   = 0,    /**< Nothing connected. */
        HW_USB_CHARGER_PRIMARY_CONN_TYPE_SDP    = 0,    /**< SDP port */
        HW_USB_CHARGER_PRIMARY_CONN_TYPE_CDP    = 1,    /**< CDP port */
        HW_USB_CHARGER_PRIMARY_CONN_TYPE_DCP    = 1     /**< DCP port */
} HW_USB_CHARGER_PRIMARY_CONN_TYPE;

/**
 * \enum HW_USB_CHARGER_SECONDARY_CONN_TYPE
 * \brief Secondary charger detection result.
 *
 */
typedef enum {
        HW_USB_CHARGER_SECONDARY_CONN_TYPE_CDP  = 0,    /**< CDP port */
        HW_USB_CHARGER_SECONDARY_CONN_TYPE_DCP  = 1     /**< CDP port */
} HW_USB_CHARGER_SECONDARY_CONN_TYPE;

/*********************************  Charger detection related services **********************************************/

/**
 * \brief Enable USB Charger detection circuit and start contact detection.
 *
 */
__STATIC_INLINE void hw_usb_charger_start_contact_detection(void)
{
        CHG_DET->CHG_DET_SW_CTRL_REG = REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, USB_CHARGE_ON)     |
                                       REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, IDP_SRC_ON);
}

/**
 * \brief Enable USB Charger detection circuit and start primary detection.
 *
 */
__STATIC_INLINE void hw_usb_charger_start_primary_detection(void)
{
        CHG_DET->CHG_DET_SW_CTRL_REG = REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, USB_CHARGE_ON)     |
                                       REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, VDP_SRC_ON)        |
                                       REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, IDM_SINK_ON);
}

/**
 * \brief Enable USB Charger detection circuit and start secondary detection.
 *
 */
__STATIC_INLINE void hw_usb_charger_start_secondary_detection(void)
{
        CHG_DET->CHG_DET_SW_CTRL_REG = REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, USB_CHARGE_ON)     |
                                       REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, VDM_SRC_ON)        |
                                       REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, IDP_SINK_ON);
}

/**
 * \brief Enable USB Charger detection circuit and pull D+ high.
 *
 */
__STATIC_INLINE void hw_usb_charger_set_dp_high(void)
{
        CHG_DET->CHG_DET_SW_CTRL_REG = REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, USB_CHARGE_ON)   |
                                       REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, VDP_SRC_ON);
}

/**
 * \brief Get USB Charger primary detection result.
 *
 *  Detect > 500mA-capable ports (CDP and DCP) from < 500mA ports (SDP).
 *
 * \return Primary charger detection result.
 *
 */
__STATIC_INLINE HW_USB_CHARGER_PRIMARY_CONN_TYPE hw_usb_charger_get_primary_detection_result(void)
{
        return REG_GETF(CHG_DET, CHG_DET_STATUS_REG, USB_CHG_DET);
}

/**
 * \brief Get USB Charger secondary detection result.
 *
 *  Detect CDP from  DCP ports.
 *
 * \return Secondary charger detection result.
 *
 */
__STATIC_INLINE HW_USB_CHARGER_SECONDARY_CONN_TYPE hw_usb_charger_get_secondary_detection_result(void)
{
        return REG_GETF(CHG_DET, CHG_DET_STATUS_REG, USB_DCP_DET);
}

/**
 * \brief USB Charger detection circuit is enabled. Any other kind of detection
 *        (contact, primary or secondary) is disabled.
 *
 */
__STATIC_INLINE void hw_usb_charger_stop_any_detection(void)
{
        CHG_DET->CHG_DET_SW_CTRL_REG = REG_MSK(CHG_DET, CHG_DET_SW_CTRL_REG, USB_CHARGE_ON);
}

/**
 * \brief Disable USB Charger detection circuit.
 *
 */
__STATIC_INLINE void hw_usb_charger_disable_detection(void)
{
        CHG_DET->CHG_DET_SW_CTRL_REG = 0;
}

/*********************************  USB IRQ related services **********************************************/

/**
 * \brief Get USB Charger status and clear the USB_IRQn interrupt.
 *
 * \note A ~20ms delay is needed for safely reading the charger status.
 *
 */
__STATIC_INLINE uint32_t hw_usb_charger_get_charger_status(void)
{
        return USB->USB_CHARGER_STAT_REG;
}

/**
 * \brief Check USB contact.
 *
 * \param[in] usb_charger_status: charger status
 *
 * \retval true:  Data pins make a contact.
 * \retval false: Data pins do not make a contact.
 *
 */
__STATIC_INLINE bool hw_usb_charger_has_data_pin_contact_detected(uint32_t usb_charger_status)
{
        return !(usb_charger_status & REG_MSK(USB, USB_CHARGER_STAT_REG, USB_DP_VAL));
}


#if (dg_configUSE_HW_PORT_DETECTION == 1)


/**
 * \enum HW_USB_CHARGER_DET_STAT
 * \brief IRQ status bits.
 *        The IRQ is triggered when the HW FSM reaches its terminal state.
 */
typedef enum {
        HW_USB_CHARGER_DET_STAT_NO_DCD                                          /**< No contact detection */
        = REG_MSK(CHG_DET, CHG_DET_FSM_STATUS_REG, NO_CONTACT_DETECTED),
        HW_USB_CHARGER_DET_STAT_2P4_AMP_PORT                                    /**< 2.4A port detected. */
        = REG_MSK(CHG_DET, CHG_DET_FSM_STATUS_REG, PORT_2P4AMP_DETECTED),
        HW_USB_CHARGER_DET_STAT_2_AMP_PORT                                      /**< 2A port detected. */
        = REG_MSK(CHG_DET, CHG_DET_FSM_STATUS_REG, PORT_2AMP_DETECTED),
        HW_USB_CHARGER_DET_STAT_1_AMP_PORT                                      /**< 1A port detected. */
        = REG_MSK(CHG_DET, CHG_DET_FSM_STATUS_REG, PORT_1AMP_DETECTED),
        HW_USB_CHARGER_DET_STAT_PS2_PROP_PORT                                   /**< PS2 or proprietary port detected. */
        = REG_MSK(CHG_DET, CHG_DET_FSM_STATUS_REG, PS2_PROP_PORT_DETECTED),
        HW_USB_CHARGER_DET_STAT_DCP_PORT                                        /**< DCP detected. */
        = REG_MSK(CHG_DET, CHG_DET_FSM_STATUS_REG, DCP_PORT_DETECTED),
        HW_USB_CHARGER_DET_STAT_CDP_PORT                                        /**< CDP detected. */
        = REG_MSK(CHG_DET, CHG_DET_FSM_STATUS_REG, CDP_PORT_DETECTED),
        HW_USB_CHARGER_DET_STAT_SDP_PORT                                        /**< SDP detected. */
        = REG_MSK(CHG_DET, CHG_DET_FSM_STATUS_REG, SDP_PORT_DETECTED),
        HW_USB_CHARGER_DET_STAT_COMPLETED                                       /**< HW port detection completed. */
        = REG_MSK(CHG_DET, CHG_DET_FSM_STATUS_REG, DETECTION_COMPLETED)
} HW_USB_CHARGER_DET_STAT;

/**
 * \brief Charger's HW detection callback.
 *
 * \param [in] status: IRQ status.
 *
 */
typedef void (*hw_usb_charger_chg_det_t)(uint32_t status);

/**
 * \brief Set charger detection HW FSM operating mode.
 *
 * When set, charger's FSM is enabled.
 *
 * \param [in] mode: True to enable HW FSM, false to disable it.
 *
 */
__STATIC_INLINE void hw_usb_charger_set_charge_detection_fsm_operating_mode(bool mode)
{
        REG_SETF(CHG_DET, CHG_DET_FSM_CTRL_REG, CHG_DET_EN, mode);
}

/**
 * \brief Enable interrupt on NVIC and charge detection block.
 *
 * \param [in] cb: Callback function that will be called when the interrupt line is asserted.
 *
 */
void hw_usb_charger_enable_charge_detection_interrupt(hw_usb_charger_chg_det_t cb);

/**
 * \brief Disable interrupt on NVIC and charge detection block.
 *
 */
void hw_usb_charger_disable_detection_interrupt(void);

#endif /* dg_configUSE_HW_PORT_DETECTION */


#endif /* dg_configUSE_HW_USB_CHARGER */


#endif /* HW_USB_CHARGER_V2_H_ */

/**
\}
\}
*/
