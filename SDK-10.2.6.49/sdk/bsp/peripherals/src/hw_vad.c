/**
 ****************************************************************************************
 *
 * @file hw_vad.c
 *
 * @brief Implementation of VAD Low Level Driver.
 *
 * Copyright (C) 2020-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_VAD

#include "hw_vad.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

__RETAINED static hw_vad_interrupt_cb intr_cb;

void hw_vad_set_mode(HW_VAD_MODE mode)
{
        uint32_t tmp = VAD->VAD_CTRL3_REG;
        switch (mode) {
        case HW_VAD_MODE_STANDBY:
                REG_SET_BIT(VAD, VAD_CTRL3_REG, VAD_SB);
                break;
        case HW_VAD_MODE_SLEEP:
                REG_SET_FIELD(VAD, VAD_CTRL3_REG, VAD_SB, tmp, 0);
                REG_SET_FIELD(VAD, VAD_CTRL3_REG, VAD_SLEEP, tmp, 1);
                VAD->VAD_CTRL3_REG = tmp;
                break;
        case HW_VAD_MODE_ALWAYS_LISTENING:
                REG_SET_FIELD(VAD, VAD_CTRL3_REG, VAD_SB, tmp, 0);
                REG_SET_FIELD(VAD, VAD_CTRL3_REG, VAD_SLEEP, tmp, 0);
                VAD->VAD_CTRL3_REG = tmp;
                break;
        default:
                //Wrong mode
                ASSERT_ERROR(0);
        }
}

HW_VAD_MODE hw_vad_get_mode(void)
{
        if (REG_GETF(VAD, VAD_CTRL3_REG, VAD_SB)) {
                return HW_VAD_MODE_STANDBY;
        }

        if (REG_GETF(VAD, VAD_CTRL3_REG, VAD_SLEEP)) {
                return HW_VAD_MODE_SLEEP;
        } else {
                return HW_VAD_MODE_ALWAYS_LISTENING;
        }
}

void hw_vad_configure(const hw_vad_config_t *cfg)
{
        if (!cfg) {
                return;
        }

        REG_SETF(CRG_TOP, CLK_CTRL_REG, VAD_CLK_SEL, cfg->mclk);

        hw_vad_set_clock_div(cfg->mclk_div);

        hw_vad_set_irq_mode(cfg->irq_mode);

        hw_vad_set_voice_track_sens(cfg->voice_sens);
        hw_vad_set_bg_noise_sens(cfg->noise_sens);
        hw_vad_set_pwr_lvl_sens(cfg->power_sens);

        hw_vad_set_min_delay(cfg->min_delay);
        hw_vad_set_min_evt_duration(cfg->min_event);
        hw_vad_set_nfi_threshold(cfg->nfi_threshold);
}

void hw_vad_get_config(hw_vad_config_t* cfg)
{
        if (!cfg) {
                return;
        }

        cfg->mclk = REG_GETF(CRG_TOP, CLK_CTRL_REG, VAD_CLK_SEL);
        cfg->mclk_div = REG_GETF(VAD, VAD_CTRL3_REG, VAD_MCLK_DIV);
        cfg->irq_mode =  REG_GETF(VAD, VAD_CTRL4_REG, VAD_IRQ_MODE);

        uint32_t tmp = VAD->VAD_CTRL0_REG;
        cfg->voice_sens = REG_GET_FIELD(VAD, VAD_CTRL0_REG, VAD_VTRACK, tmp);
        cfg->noise_sens = REG_GET_FIELD(VAD, VAD_CTRL0_REG, VAD_NTRACK, tmp);
        cfg->power_sens = REG_GET_FIELD(VAD, VAD_CTRL0_REG, VAD_PWR_LVL_SNSTVTY, tmp);

        tmp = VAD->VAD_CTRL1_REG;
        cfg->min_delay = REG_GET_FIELD(VAD, VAD_CTRL1_REG, VAD_MINDELAY, tmp);
        cfg->min_event = REG_GET_FIELD(VAD, VAD_CTRL1_REG, VAD_MINEVENT, tmp);

        cfg->nfi_threshold = hw_vad_get_nfi_threshold();
}

void hw_vad_reset(void)
{
        hw_vad_set_mode(HW_VAD_MODE_STANDBY);

        const hw_vad_config_t cfg = {
                .mclk = HW_VAD_MCLK_RCLP32K,
                .mclk_div = HW_VAD_MCLK_DIV_1,
                .irq_mode = HW_VAD_IRQ_MODE_HIGH,
                .voice_sens = HW_VAD_VOICE_SENS_DEFAULT,
                .noise_sens = HW_VAD_NOISE_SENS_DEFAULT,
                .power_sens = HW_VAD_PWR_LVL_SENS_6_DB,
                .min_delay = HW_VAD_MIN_DELAY_1536_CYCLES,
                .min_event = HW_VAD_MIN_EVENT_32_CYCLES,
                .nfi_threshold = 0x27
        };
        hw_vad_configure(&cfg);
}

void hw_vad_register_interrupt(hw_vad_interrupt_cb cb)
{
        NVIC_ClearPendingIRQ(VAD_IRQn);
        NVIC_EnableIRQ(VAD_IRQn);

        intr_cb = cb;
}

void hw_vad_unregister_interrupt(void)
{
        intr_cb = NULL;

        NVIC_DisableIRQ(VAD_IRQn);
        NVIC_ClearPendingIRQ(VAD_IRQn);
}

void VAD_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();
        /* Set VAD mode to sleep */
        hw_vad_set_mode(HW_VAD_MODE_SLEEP);

        if (intr_cb) {
                intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
#endif /* dg_configUSE_HW_VAD */
