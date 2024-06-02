/**
 ****************************************************************************************
 *
 * @file hw_src.c
 *
 * @brief Implementation of the Audio Unit SRC Low Level Driver.
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_SRC
#include "hw_src.h"

#define SRC_CLK   32000  /* SRC_CLK must be 32000 Hz according to design limitation */

/**
 * \brief Clear the SRC over/underflow indications
 *
 * \param [in] id    identifies SRC1, SRC2
 * \param[in]  io    Input/Output selection (IN or OUT)
 */
#define HW_SRC_CLEAR_FLOW_ERROR(id, io) \
        while (HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_##io##_OVFLOW)|| \
                HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_##io##_UNFLOW)) { \
                HW_SRC_REG_SET_BIT(id, SRC1, CTRL_REG, SRC_##io##_FLOWCLR); \
        } \
        HW_SRC_REG_CLR_BIT(id, SRC1, CTRL_REG, SRC_##io##_FLOWCLR);

static uint32_t hw_src_calc_sampling_frequency(uint32_t sample_rate, uint8_t divider, uint8_t *iir_setting)
{
        ASSERT_WARNING(divider > 0);
        ASSERT_WARNING(iir_setting != NULL);

        if (sample_rate > 96000) {
                *iir_setting = 3;
        } else if (sample_rate > 48000) {
                *iir_setting = 1;
        } else {
                *iir_setting = 0;
        }

        sample_rate /= (*iir_setting) + 1;
        uint64_t sampling_frequency = 4096 * (uint64_t)sample_rate * (uint64_t)divider;
        return (sampling_frequency / 100) & 0xFFFFFF;
}

void hw_src_init(HW_SRC_ID id, hw_src_config_t *config)
{
        uint8_t divider;
        uint8_t iir_setting;
        uint32_t val;
        const uint16_t divn_clk = dg_configDIVN_FREQ / 1000;

        ASSERT_WARNING(config && (config->src_clk == SRC_CLK));

        config->id = id;

         if (divn_clk % config->src_clk) {
                 ASSERT_WARNING(0);
                 return;
         }

         divider = divn_clk / config->src_clk;


        ASSERT_WARNING(config->id == HW_SRC1 || config->id == HW_SRC2);

        val = CRG_AUD->SRC_DIV_REG;

        if (config->id == HW_SRC1) {
                REG_SET_FIELD(CRG_AUD, SRC_DIV_REG, SRC_DIV, val, divider);
                REG_SET_FIELD(CRG_AUD, SRC_DIV_REG, CLK_SRC_EN, val, 1);
        } else {
                REG_SET_FIELD(CRG_AUD, SRC_DIV_REG, SRC2_DIV, val, divider);
                REG_SET_FIELD(CRG_AUD, SRC_DIV_REG, CLK_SRC2_EN, val, 1);
        }

        CRG_AUD->SRC_DIV_REG = val;

        if (config->in_sample_rate > 0) {
                uint32_t sampling_frequency = hw_src_calc_sampling_frequency(config->in_sample_rate,
                        divider, &iir_setting);
                HW_SRC_REG_SETF(config->id, SRC1, IN_FS_REG, SRC_IN_FS, sampling_frequency);
                val = SRCBA(config->id)->CTRL_REG;
                HW_SRC_REG_SET_FIELD(SRC1, CTRL_REG, SRC_IN_DS, val, iir_setting);
                SRCBA(config->id)->CTRL_REG = val;
        }

        if (config->out_sample_rate > 0) {
                uint32_t sampling_frequency = hw_src_calc_sampling_frequency(config->out_sample_rate,
                        divider, &iir_setting);
                HW_SRC_REG_SETF(config->id, SRC1, OUT_FS_REG, SRC_OUT_FS, sampling_frequency);
                val = SRCBA(config->id)->SRC1_CTRL_REG;
                HW_SRC_REG_SET_FIELD(SRC1, CTRL_REG, SRC_OUT_US, val, iir_setting);
                SRCBA(config->id)->SRC1_CTRL_REG = val;
        }

        // Clear input data registers
        hw_src_write_input(config->id, 1, 0);
        hw_src_write_input(config->id, 2, 0);

        HW_SRC_CLEAR_FLOW_ERROR(config->id, IN);
        HW_SRC_CLEAR_FLOW_ERROR(config->id, OUT);
}

HW_SRC_FLOW_STATUS hw_src_get_flow_status(HW_SRC_ID id, HW_SRC_DIRECTION direction)
{
        HW_SRC_FLOW_STATUS status = HW_SRC_FLOW_OK;

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        switch (direction) {
        case HW_SRC_IN:
                if (HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_IN_OVFLOW)) {
                        status |= HW_SRC_FLOW_OVER;
                }
                if (HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_IN_UNFLOW)) {
                        status |= HW_SRC_FLOW_UNDER;
                }
                HW_SRC_CLEAR_FLOW_ERROR(id, IN);
                break;
        case HW_SRC_OUT:
                if (HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_OUT_OVFLOW)) {
                        status |= HW_SRC_FLOW_OVER;
                }
                if (HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_OUT_UNFLOW)) {
                        status |= HW_SRC_FLOW_UNDER;
                }
                HW_SRC_CLEAR_FLOW_ERROR(id, OUT);
                break;
        default:
                ASSERT_WARNING(0);
        }
        return status;
}

#endif /* dg_configUSE_HW_SRC */
