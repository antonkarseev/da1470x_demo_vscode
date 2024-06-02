/**
 ****************************************************************************************
 *
 * @file hw_sdadc.c
 *
 * @brief Implementation of the SDADC Low Level Driver.
 *
 * Copyright (C) 2018-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SDADC

#include "hw_sdadc.h"
#if HW_SDADC_DMA_SUPPORT
#include "hw_dma.h"
#define SDADC_DMA_TRIGGER                       HW_DMA_TRIG_GP_ADC_APP_ADC
#endif /* HW_SDADC_DMA_SUPPORT */

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#define SDADC_IRQ                               ADC2_IRQn
#define SDADC_Handler                           APADC_Handler

static hw_sdadc_interrupt_cb intr_cb = NULL;

/*=============================================================================================*/
/* Basic functionality of the SDADC                                                            */
/*=============================================================================================*/

void hw_sdadc_init(const sdadc_config *cfg)
{
        SDADC->SDADC_CTRL_REG = 0x800;
        SDADC->SDADC_PGA_CTRL_REG = 0x20;

        NVIC_DisableIRQ(SDADC_IRQ);
        NVIC_ClearPendingIRQ(SDADC_IRQ);

        hw_sdadc_configure(cfg);
}


void hw_sdadc_deinit(void)
{
        hw_sdadc_unregister_interrupt();
}

void hw_sdadc_reset(void)
{
        // Set control register to default values but keep enable bit set
        SDADC->SDADC_CTRL_REG = 0x800 | REG_MSK(SDADC, SDADC_CTRL_REG, SDADC_EN);
        SDADC->SDADC_PGA_CTRL_REG = 0x20;

        NVIC_DisableIRQ(SDADC_IRQ);
        NVIC_ClearPendingIRQ(SDADC_IRQ);
}

#if HW_SDADC_DMA_SUPPORT
static DMA_setup sdadc_dma_setup;

static void hw_sdadc_dma_configure(sdadc_dma_cfg *cfg)
{
        if (!cfg) {
                return;
        }
        ASSERT_ERROR((cfg->channel & 0x1) == 1);

        hw_sdadc_set_dma_functionality(true);
        /*
         * Volatile user configuration
         */
        sdadc_dma_setup.channel_number = cfg->channel,
        sdadc_dma_setup.dma_prio = cfg->prio,
        sdadc_dma_setup.dest_address = cfg->dest,
        sdadc_dma_setup.length = cfg->len,
        sdadc_dma_setup.callback = cfg->cb,
        sdadc_dma_setup.user_data = cfg->ud,
        /*
         * Fixed configuration applicable to SDADC
         */
        sdadc_dma_setup.bus_width = HW_DMA_BW_HALFWORD,
        sdadc_dma_setup.irq_enable = HW_DMA_IRQ_STATE_ENABLED,
        sdadc_dma_setup.irq_nr_of_trans = 0,
        sdadc_dma_setup.dreq_mode = HW_DMA_DREQ_TRIGGERED,
        sdadc_dma_setup.burst_mode = HW_DMA_BURST_MODE_DISABLED,
        sdadc_dma_setup.a_inc = HW_DMA_AINC_FALSE,
        sdadc_dma_setup.b_inc = HW_DMA_BINC_TRUE,
        sdadc_dma_setup.circular = HW_DMA_MODE_NORMAL,
        sdadc_dma_setup.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE,
        sdadc_dma_setup.dma_init = HW_DMA_INIT_AX_BX_AY_BY,
        sdadc_dma_setup.dma_req_mux = SDADC_DMA_TRIGGER,
        sdadc_dma_setup.src_address = (uint32_t) &SDADC->SDADC_RESULT_REG,
        /*
         * Setup DMA
         */
        hw_dma_channel_initialization(&sdadc_dma_setup);
}
#endif /* HW_SDADC_DMA_SUPPORT */

void hw_sdadc_configure(const sdadc_config *cfg)
{
        if (!cfg) {
                return;
        }
        ASSERT_ERROR(!hw_sdadc_in_progress());
        if (cfg->mask_int) {
                hw_sdadc_enable_interrupt();
        } else {
                hw_sdadc_disable_interrupt();
        }
        hw_sdadc_set_result_mode(cfg->result_mode);

        /* PGA configuration */
        hw_sdadc_pga_set_gain(cfg->pga_gain);
        hw_sdadc_pga_set_bias(cfg->pga_bias);

        ASSERT_WARNING(cfg->pga_en != HW_SDADC_PGA_ENABLE_NONE);
        hw_sdadc_pga_select_enabled_channels(cfg->pga_en);

        if (cfg->pga_en == HW_SDADC_PGA_ENABLE_POSITIVE) {
                ASSERT_WARNING(cfg->pga_mode != HW_SDADC_PGA_MODE_SE_N);
        }
        if (cfg->pga_en == HW_SDADC_PGA_ENABLE_NEGATIVE) {
                ASSERT_WARNING(cfg->pga_mode != HW_SDADC_PGA_MODE_SE_P);
        }
        hw_sdadc_pga_set_mode(cfg->pga_mode);
#if HW_SDADC_DMA_SUPPORT

        hw_sdadc_dma_configure(cfg->dma_setup);
#endif
}

void hw_sdadc_register_interrupt(hw_sdadc_interrupt_cb cb)
{
        intr_cb = cb;

        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MINT, 1);

        NVIC_ClearPendingIRQ(SDADC_IRQ);
        NVIC_EnableIRQ(SDADC_IRQ);
}

void hw_sdadc_unregister_interrupt(void)
{
        NVIC_DisableIRQ(SDADC_IRQ);
        NVIC_ClearPendingIRQ(SDADC_IRQ);
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MINT, 0);

        intr_cb = NULL;
}

void SDADC_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        if (intr_cb) {
                intr_cb();
        } else {
                hw_sdadc_clear_interrupt();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_SDADC */

