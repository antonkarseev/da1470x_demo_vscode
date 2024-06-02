/**
 ****************************************************************************************
 *
 * @file hw_usb_charger_v2.c
 *
 * @brief Implementation of the USB Charger Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */



#if (dg_configUSE_HW_USB_CHARGER == 1)

#if (dg_configUSE_HW_PORT_DETECTION == 1)

#include "hw_usb_charger.h"

__RETAINED static hw_usb_charger_chg_det_t  hw_usb_charger_chg_det_cb;

void hw_usb_charger_enable_charge_detection_interrupt(hw_usb_charger_chg_det_t cb)
{
        ASSERT_WARNING(cb);

        hw_usb_charger_chg_det_cb = cb;

        /* Clear the IRQ. */
        CHG_DET->CHG_DET_IRQ_CLEAR_REG = 1;

        NVIC_ClearPendingIRQ(CHARGER_DET_IRQn);
        NVIC_EnableIRQ(CHARGER_DET_IRQn);

        /* Enable the IRQ generation. */
        REG_SET_BIT(CHG_DET, CHG_DET_IRQ_MASK_REG, CHG_DET_IRQ_EN);
}

void hw_usb_charger_disable_detection_interrupt(void)
{
        /* Disable the IRQ generation. */
        REG_CLR_BIT(CHG_DET, CHG_DET_IRQ_MASK_REG, CHG_DET_IRQ_EN);

        /* Clear the IRQ. */
        CHG_DET->CHG_DET_IRQ_CLEAR_REG = 1;

        NVIC_DisableIRQ(CHARGER_DET_IRQn);
        NVIC_ClearPendingIRQ(CHARGER_DET_IRQn);

        hw_usb_charger_chg_det_cb = NULL;
}

void CHARGER_DET_Handler(void)
{
        uint32_t status;

        /* Clear the irq */
        CHG_DET->CHG_DET_IRQ_CLEAR_REG = 1;

        status = CHG_DET->CHG_DET_FSM_STATUS_REG;

        if (hw_usb_charger_chg_det_cb) {
                hw_usb_charger_chg_det_cb(status);
        }
}

#endif /* dg_configUSE_HW_PORT_DETECTION */

#endif /* dg_configUSE_HW_USB_CHARGER */


