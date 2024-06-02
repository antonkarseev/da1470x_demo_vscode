/**
 ****************************************************************************************
 *
 * @file sys_sw_cursor.c
 *
 * @brief System service providing software cursors
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_SW_CURSOR

#include "hw_clk.h"
#include "hw_gpio.h"
#include "hw_pd.h"
#include "hw_sys.h"

#define SW_CURSOR_GPIO                  *(SW_CURSOR_PORT == 0 ? \
                                                (SW_CURSOR_PIN == 0 ? &(GPIO->P0_00_MODE_REG) : \
                                                (SW_CURSOR_PIN == 1 ? &(GPIO->P0_01_MODE_REG) : \
                                                (SW_CURSOR_PIN == 2 ? &(GPIO->P0_02_MODE_REG) : \
                                                (SW_CURSOR_PIN == 3 ? &(GPIO->P0_03_MODE_REG) : \
                                                (SW_CURSOR_PIN == 4 ? &(GPIO->P0_04_MODE_REG) : \
                                                (SW_CURSOR_PIN == 5 ? &(GPIO->P0_05_MODE_REG) : \
                                                (SW_CURSOR_PIN == 6 ? &(GPIO->P0_06_MODE_REG) : \
                                                (SW_CURSOR_PIN == 7 ? &(GPIO->P0_07_MODE_REG) : \
                                                (SW_CURSOR_PIN == 8 ? &(GPIO->P0_08_MODE_REG) : \
                                                (SW_CURSOR_PIN == 9 ? &(GPIO->P0_09_MODE_REG) : \
                                                (SW_CURSOR_PIN == 10 ? &(GPIO->P0_10_MODE_REG) : \
                                                (SW_CURSOR_PIN == 11 ? &(GPIO->P0_11_MODE_REG) : \
                                                (SW_CURSOR_PIN == 12 ? &(GPIO->P0_12_MODE_REG) : \
                                                (SW_CURSOR_PIN == 13 ? &(GPIO->P0_13_MODE_REG) : \
                                                (SW_CURSOR_PIN == 14 ? &(GPIO->P0_14_MODE_REG) : \
                                                (SW_CURSOR_PIN == 15 ? &(GPIO->P0_15_MODE_REG) : \
                                                (SW_CURSOR_PIN == 16 ? &(GPIO->P0_16_MODE_REG) : \
                                                (SW_CURSOR_PIN == 17 ? &(GPIO->P0_17_MODE_REG) : \
                                                (SW_CURSOR_PIN == 18 ? &(GPIO->P0_18_MODE_REG) : \
                                                (SW_CURSOR_PIN == 19 ? &(GPIO->P0_19_MODE_REG) : \
                                                (SW_CURSOR_PIN == 20 ? &(GPIO->P0_20_MODE_REG) : \
                                                (SW_CURSOR_PIN == 21 ? &(GPIO->P0_21_MODE_REG) : \
                                                (SW_CURSOR_PIN == 22 ? &(GPIO->P0_22_MODE_REG) : \
                                                (SW_CURSOR_PIN == 23 ? &(GPIO->P0_23_MODE_REG) : \
                                                (SW_CURSOR_PIN == 24 ? &(GPIO->P0_24_MODE_REG) : \
                                                (SW_CURSOR_PIN == 25 ? &(GPIO->P0_25_MODE_REG) : \
                                                (SW_CURSOR_PIN == 26 ? &(GPIO->P0_26_MODE_REG) : \
                                                (SW_CURSOR_PIN == 27 ? &(GPIO->P0_27_MODE_REG) : \
                                                (SW_CURSOR_PIN == 28 ? &(GPIO->P0_28_MODE_REG) : \
                                                (SW_CURSOR_PIN == 29 ? &(GPIO->P0_29_MODE_REG) : \
                                                (SW_CURSOR_PIN == 30 ? &(GPIO->P0_30_MODE_REG) : \
                                                                       &(GPIO->P0_31_MODE_REG)))))))))))))))))))))))))))))))) \
                                                : \
                                                (SW_CURSOR_PIN == 0 ? &(GPIO->P1_00_MODE_REG) : \
                                                (SW_CURSOR_PIN == 1 ? &(GPIO->P1_01_MODE_REG) : \
                                                (SW_CURSOR_PIN == 2 ? &(GPIO->P1_02_MODE_REG) : \
                                                (SW_CURSOR_PIN == 3 ? &(GPIO->P1_03_MODE_REG) : \
                                                (SW_CURSOR_PIN == 4 ? &(GPIO->P1_04_MODE_REG) : \
                                                (SW_CURSOR_PIN == 5 ? &(GPIO->P1_05_MODE_REG) : \
                                                (SW_CURSOR_PIN == 6 ? &(GPIO->P1_06_MODE_REG) : \
                                                (SW_CURSOR_PIN == 7 ? &(GPIO->P1_07_MODE_REG) : \
                                                (SW_CURSOR_PIN == 8 ? &(GPIO->P1_08_MODE_REG) : \
                                                (SW_CURSOR_PIN == 9 ? &(GPIO->P1_09_MODE_REG) : \
                                                (SW_CURSOR_PIN == 10 ? &(GPIO->P1_10_MODE_REG) : \
                                                (SW_CURSOR_PIN == 11 ? &(GPIO->P1_11_MODE_REG) : \
                                                (SW_CURSOR_PIN == 12 ? &(GPIO->P1_12_MODE_REG) : \
                                                (SW_CURSOR_PIN == 13 ? &(GPIO->P1_13_MODE_REG) : \
                                                (SW_CURSOR_PIN == 14 ? &(GPIO->P1_14_MODE_REG) : \
                                                (SW_CURSOR_PIN == 15 ? &(GPIO->P1_15_MODE_REG) : \
                                                (SW_CURSOR_PIN == 16 ? &(GPIO->P1_16_MODE_REG) : \
                                                (SW_CURSOR_PIN == 17 ? &(GPIO->P1_17_MODE_REG) : \
                                                (SW_CURSOR_PIN == 18 ? &(GPIO->P1_18_MODE_REG) : \
                                                (SW_CURSOR_PIN == 19 ? &(GPIO->P1_19_MODE_REG) : \
                                                (SW_CURSOR_PIN == 20 ? &(GPIO->P1_20_MODE_REG) : \
                                                (SW_CURSOR_PIN == 21 ? &(GPIO->P1_21_MODE_REG) : \
                                                                       &(GPIO->P1_22_MODE_REG))))))))))))))))))))))))

#define SW_CURSOR_SET                   *(SW_CURSOR_PORT == 0 ? &(GPIO->P0_SET_DATA_REG) :    \
                                                                &(GPIO->P1_SET_DATA_REG))

#define SW_CURSOR_RESET                 *(SW_CURSOR_PORT == 0 ? &(GPIO->P0_RESET_DATA_REG) :  \
                                                                &(GPIO->P1_RESET_DATA_REG))

#define SW_CURSOR_SET_PAD_LATCH         *(SW_CURSOR_PORT == 0 ? &(CRG_TOP->P0_SET_PAD_LATCH_REG) :  \
                                                                &(CRG_TOP->P1_SET_PAD_LATCH_REG))

#define SW_CURSOR_RESET_PAD_LATCH       *(SW_CURSOR_PORT == 0 ? &(CRG_TOP->P0_RESET_PAD_LATCH_REG) :  \
                                                                &(CRG_TOP->P1_RESET_PAD_LATCH_REG))
void sys_sw_cursor_setup(void)
{
        hw_gpio_set_pin_function(SW_CURSOR_PORT, SW_CURSOR_PIN, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_GPIO);
        hw_gpio_pad_latch_enable(SW_CURSOR_PORT, SW_CURSOR_PIN);
        hw_gpio_pad_latch_disable(SW_CURSOR_PORT, SW_CURSOR_PIN);
}

void sys_sw_cursor_trigger(void)
{
        hw_gpio_configure_pin(SW_CURSOR_PORT, SW_CURSOR_PIN, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO, true);
        hw_gpio_pad_latch_enable(SW_CURSOR_PORT, SW_CURSOR_PIN);
        hw_clk_delay_usec(50);
        hw_gpio_set_pin_function(SW_CURSOR_PORT, SW_CURSOR_PIN, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_GPIO);
        hw_gpio_pad_latch_disable(SW_CURSOR_PORT, SW_CURSOR_PIN);
}

#endif /* dg_configUSE_SW_CURSOR */

