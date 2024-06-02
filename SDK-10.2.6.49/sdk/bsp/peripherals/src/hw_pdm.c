/**
 ****************************************************************************************
 *
 * @file hw_pdm.c
 *
 * @brief Implementation of the PDM/Audio Low Level Driver.
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_PDM
#include "hw_pdm.h"

uint32_t hw_pdm_clk_init(uint32_t frequency)
{
        uint32_t div;

        /* Translate main clk frequency and requested frequency to proper divider */
        div = (dg_configDIVN_FREQ / frequency);

        /* Calculate the achievable frequency */
        if (dg_configXTAL32M_FREQ % frequency) {
                frequency = (dg_configXTAL32M_FREQ / div);
        }

        /* PDM_CLK frequency according to specification is in the range of 62.5 kHz - 4 MHz */
        ASSERT_WARNING((frequency >= 62500) && (frequency <= 4000000));

        ASSERT_WARNING((div & ~(HW_PDM_CRG_REG_FIELD_MASK(DIV, PDM_DIV) >>
                                HW_PDM_CRG_REG_FIELD_POS(DIV, PDM_DIV))) == 0);

        HW_PDM_CRG_REG_SETF(DIV, PDM_DIV, div);

        return frequency;
}

void hw_pdm_init(HW_SRC_ID id, hw_pdm_config_t *config)
{
        if (config->data_direction == PDM_DIRECTION_INPUT) {
                hw_pdm_set_input_delay(id, config->in_delay);
                hw_pdm_set_in_channel_swap(id, !config->swap_channel);
        } else {
                hw_pdm_set_output_delay(id, config->out_delay);
                hw_pdm_set_out_channel_swap(id, config->swap_channel);
        }

        hw_pdm_set_mode(config->config_mode);
}
#endif /* dg_configUSE_HW_PDM */
