/**
 ****************************************************************************************
 *
 * @file hw_wkup_v2.c
 *
 * @brief Implementation of the Wakeup Controller Low Level Driver.
 *
 * Copyright (C) 2015-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_WKUP


#include <stdio.h>
#include <string.h>
#include "hw_wkup.h"

#if (dg_configSYSTEMVIEW == 1)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif /* (dg_configSYSTEMVIEW == 1) */


__RETAINED static hw_wkup_interrupt_cb intr_cb_key;
__RETAINED static hw_wkup_interrupt_cb intr_cb_p0;
__RETAINED static hw_wkup_interrupt_cb intr_cb_p1;
__RETAINED static hw_wkup_interrupt_cb intr_cb_p2;

void hw_wkup_init(const wkup_config *cfg)
{
        unsigned int i;

        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_TMR_REG, WAKEUPCT_ENABLE);
        GLOBAL_INT_RESTORE();

        /* reset configuration */
        WAKEUP->WKUP_CTRL_REG = 0;

        /* reset all pin settings to default */
        for (i = 0; i < HW_GPIO_PORT_MAX; i++) {
                *(HW_WKUP_SELECT_KEY_P0_BASE_REG + i) = 0;
                *(HW_WKUP_SELECT_GPIO_P0_BASE_REG + i) = 0;
                *(HW_WKUP_POL_P0_BASE_REG + i) = 0;
                *((volatile uint32_t *)(&WAKEUP-> WKUP_CLEAR_P0_REG) + i) = 0xFFFFFFFF;
                *(HW_WKUP_SELECT1_GPIO_P0_BASE_REG + i) = 0;
        }

        /* Disable interrupts */
        NVIC_DisableIRQ(KEY_WKUP_GPIO_IRQn);
        NVIC_DisableIRQ(GPIO_P0_IRQn);
        NVIC_DisableIRQ(GPIO_P1_IRQn);
        NVIC_DisableIRQ(GPIO_P2_IRQn);
        hw_wkup_configure(cfg);
}

void hw_wkup_configure(const wkup_config *cfg)
{
        int i;

        if (!cfg) {
                return;
        }

        hw_wkup_set_key_debounce_time(cfg->debounce);

        for (i = 0; i < HW_GPIO_PORT_MAX; i++) {
                /* register has inverted logic than pin_trigger state bitmask */
                *(HW_WKUP_POL_P0_BASE_REG + i) = ~cfg->pin_trigger[i];
                *(HW_WKUP_SELECT1_GPIO_P0_BASE_REG + i) = cfg->gpio_sense[i];
                *(HW_WKUP_SELECT_KEY_P0_BASE_REG + i) = cfg->pin_wkup_state[i];
                *(HW_WKUP_SELECT_GPIO_P0_BASE_REG + i) = cfg->pin_gpio_state[i];
        }
}

static void set_polarity(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_WKUP_TRIGGER trigger)
{
        volatile uint32_t *pol_set_ptr  = HW_WKUP_POL_P0_BASE_REG + port;

        switch (trigger) {
         case HW_WKUP_TRIG_EDGE_HI:
         case HW_WKUP_TRIG_LEVEL_HI_DEB:
         case HW_WKUP_TRIG_LEVEL_HI:
                 /* set polarity to high */
                 *pol_set_ptr &= ~(0x1 << pin);
                 break;
         case HW_WKUP_TRIG_EDGE_LO:
         case HW_WKUP_TRIG_LEVEL_LO_DEB:
         case HW_WKUP_TRIG_LEVEL_LO:
                 /* set polarity to low */
                 *pol_set_ptr |= 0x1 << pin;
                 break;
         default:
                 /* wrong trigger value*/
                 ASSERT_ERROR(0);
         }

}

void hw_wkup_set_trigger(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_WKUP_TRIGGER trigger)
{
        volatile uint32_t *key_enable_ptr = HW_WKUP_SELECT_KEY_P0_BASE_REG + port;
        volatile uint32_t *gpio_enable_ptr = HW_WKUP_SELECT_GPIO_P0_BASE_REG + port;
        volatile uint32_t *gpio_sensitivity_ptr = HW_WKUP_SELECT1_GPIO_P0_BASE_REG + port;

        /* first disable key and gpio triggers of specific pin*/
        *key_enable_ptr &= ~(0x1 << pin);
        *gpio_enable_ptr &= ~(0x1 << pin);
        *gpio_sensitivity_ptr &= ~(0x1 << pin);
        if (trigger == HW_WKUP_TRIG_DISABLED) {
                return;
        }

        set_polarity(port, pin, trigger);

        switch (trigger) {
        case HW_WKUP_TRIG_LEVEL_LO_DEB:
        case HW_WKUP_TRIG_LEVEL_HI_DEB:
                *key_enable_ptr |= 0x1 << pin;
                break;
        case HW_WKUP_TRIG_LEVEL_LO:
        case HW_WKUP_TRIG_LEVEL_HI:
                *gpio_enable_ptr |= 0x1 << pin;
                break;
        case HW_WKUP_TRIG_EDGE_LO:
        case HW_WKUP_TRIG_EDGE_HI:
                *gpio_sensitivity_ptr |= 0x1 << pin;
                *gpio_enable_ptr |= 0x1 << pin;
                break;
        default:
                /* wrong trigger value*/
                ASSERT_ERROR(0);
        }

}

void hw_wkup_configure_hibernation(HW_WKUP_HIBERN_PIN pin, HW_WKUP_HIBERN_PD_EN_PIN enable_pd)
{
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, WAKEUP_HIBERN_REG, WAKEUP_EN, pin);
        REG_SETF(CRG_TOP, WAKEUP_HIBERN_REG, WAKEUP_PD_EN, enable_pd);
        GLOBAL_INT_RESTORE();
}

void hw_wkup_register_key_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio)
{
        intr_cb_key = cb;

        NVIC_ClearPendingIRQ(KEY_WKUP_GPIO_IRQn);
        NVIC_SetPriority(KEY_WKUP_GPIO_IRQn, prio);
        NVIC_EnableIRQ(KEY_WKUP_GPIO_IRQn);
}

void hw_wkup_register_gpio_p0_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio)
{
        intr_cb_p0 = cb;

        NVIC_ClearPendingIRQ(GPIO_P0_IRQn);
        NVIC_SetPriority(GPIO_P0_IRQn, prio);
        NVIC_EnableIRQ(GPIO_P0_IRQn);
}

void hw_wkup_register_gpio_p1_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio)
{
        intr_cb_p1 = cb;

        NVIC_ClearPendingIRQ(GPIO_P1_IRQn);
        NVIC_SetPriority(GPIO_P1_IRQn, prio);
        NVIC_EnableIRQ(GPIO_P1_IRQn);
}

void hw_wkup_register_gpio_p2_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio)
{
        intr_cb_p2 = cb;

        NVIC_ClearPendingIRQ(GPIO_P2_IRQn);
        NVIC_SetPriority(GPIO_P2_IRQn, prio);
        NVIC_EnableIRQ(GPIO_P2_IRQn);
}

void hw_wkup_unregister_interrupts(void)
{
        intr_cb_key = NULL;
        intr_cb_p0  = NULL;
        intr_cb_p1  = NULL;
        intr_cb_p2  = NULL;

        NVIC_DisableIRQ(KEY_WKUP_GPIO_IRQn);
        NVIC_DisableIRQ(GPIO_P0_IRQn);
        NVIC_DisableIRQ(GPIO_P1_IRQn);
        NVIC_DisableIRQ(GPIO_P2_IRQn);
}

void hw_wkup_key_handler(void)
{
        if (intr_cb_key) {
                intr_cb_key();
        }
}

void hw_wkup_p0_handler(void)
{
        if (intr_cb_p0) {
                intr_cb_p0();
        } else {
                hw_wkup_clear_gpio_status(HW_GPIO_PORT_0, WAKEUP_WKUP_CLEAR_P0_REG_WKUP_CLEAR_P0_Msk);
        }
}

void hw_wkup_p1_handler(void)
{
        if (intr_cb_p1) {
                intr_cb_p1();
        } else {
                hw_wkup_clear_gpio_status(HW_GPIO_PORT_1, WAKEUP_WKUP_CLEAR_P1_REG_WKUP_CLEAR_P1_Msk);
        }

}

void hw_wkup_p2_handler(void)
{
        if (intr_cb_p2) {
                intr_cb_p2();
        } else {
                hw_wkup_clear_gpio_status(HW_GPIO_PORT_2, WAKEUP_WKUP_CLEAR_P2_REG_WKUP_CLEAR_P2_Msk);
        }

}

void Key_Wkup_GPIO_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        hw_wkup_reset_key_interrupt();
        NVIC_ClearPendingIRQ(KEY_WKUP_GPIO_IRQn);
        hw_wkup_key_handler();

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void GPIO_P0_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        NVIC_ClearPendingIRQ(GPIO_P0_IRQn);
        hw_wkup_p0_handler();

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void GPIO_P1_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        NVIC_ClearPendingIRQ(GPIO_P1_IRQn);
        hw_wkup_p1_handler();

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void GPIO_P2_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        NVIC_ClearPendingIRQ(GPIO_P2_IRQn);
        hw_wkup_p2_handler();

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
#endif /* dg_configUSE_HW_WKUP */
