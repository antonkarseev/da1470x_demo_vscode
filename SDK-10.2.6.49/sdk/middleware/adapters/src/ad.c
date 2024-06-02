/**
 ****************************************************************************************
 *
 * @file  ad.c
 *
 * @brief Adapters shared functions
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configI2C_ADAPTER == 1) || (dg_configSPI_ADAPTER == 1) || (dg_configGPADC_ADAPTER == 1) \
                           || (dg_configISO7816_ADAPTER == 1) || (dg_configLCDC_ADAPTER == 1 ) \
                           || (dg_configSDADC_ADAPTER == 1) || (dg_configUART_ADAPTER == 1) \
                           || (dg_configI3C_ADAPTER == 1)

#include <stdint.h>
#include "ad.h"
#include "osal.h"

AD_IO_ERROR ad_io_configure(const ad_io_conf_t *io, uint8_t size, HW_GPIO_POWER voltage_level,
                                                                        AD_IO_CONF_STATE state)
{
        if (!io || (voltage_level >= HW_GPIO_POWER_NONE) || (0 == size)
                        || (size > HW_GPIO_PIN_MAX * HW_GPIO_PORT_MAX)) {
                OS_ASSERT(0);
                return AD_IO_ERROR_INVALID_CFG;
        }

        for (int i = 0; i < size; i++) {
                if (!AD_IO_PIN_PORT_VALID(io[i].port, io[i].pin)) {
                        OS_ASSERT(0);
                        return AD_IO_ERROR_INVALID_PIN;
                }

                switch (state) {
                case AD_IO_CONF_ON:
                        hw_gpio_configure_pin(io[i].port, io[i].pin, io[i].on.mode,
                                                        io[i].on.function, io[i].on.high);
                        break;
                case AD_IO_CONF_OFF:
                        hw_gpio_configure_pin(io[i].port, io[i].pin, io[i].off.mode,
                                                        io[i].off.function, io[i].off.high);
                        break;
                default:
                        OS_ASSERT(0);
                        return AD_IO_ERROR_INVALID_CFG;
                }

                hw_gpio_configure_pin_power(io[i].port, io[i].pin, voltage_level);
        }

        return AD_IO_ERROR_NONE;
}

AD_IO_ERROR ad_io_set_pad_latch(const ad_io_conf_t *io, uint8_t size, AD_IO_PAD_LATCHES_OP operation)
{
        if (!io || (0 == size) || (size > HW_GPIO_PIN_MAX * HW_GPIO_PORT_MAX)) {
                OS_ASSERT(0);
                return AD_IO_ERROR_INVALID_CFG;
        }

        for (int i = 0; i < size; i++) {
                if (!AD_IO_PIN_PORT_VALID(io[i].port, io[i].pin)) {
                        OS_ASSERT(0);
                        return AD_IO_ERROR_INVALID_PIN;
                }

                switch (operation) {
                case AD_IO_PAD_LATCHES_OP_ENABLE:
                        hw_gpio_pad_latch_enable(io[i].port, io[i].pin);
                        break;
                case AD_IO_PAD_LATCHES_OP_DISABLE:
                        hw_gpio_pad_latch_disable(io[i].port, io[i].pin);
                        break;
                case AD_IO_PAD_LATCHES_OP_TOGGLE:
                        hw_gpio_pad_latch_enable(io[i].port, io[i].pin);
                        hw_gpio_pad_latch_disable(io[i].port, io[i].pin);
                        break;
                default:
                        OS_ASSERT(0);
                        return AD_IO_ERROR_INVALID_CFG;
                }
        }

        return AD_IO_ERROR_NONE;
}


#endif /* dg_config*_ADAPTER */
