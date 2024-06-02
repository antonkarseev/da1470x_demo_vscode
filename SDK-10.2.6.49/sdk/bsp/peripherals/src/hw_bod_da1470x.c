/**
****************************************************************************************
*
* @file hw_bod_da1470x.c
*
* @brief BOD LLD
*
* Copyright (C) 2020-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#if dg_configUSE_BOD

#include "hw_bod.h"
#include "hw_pmu.h"

static void hw_bod_activate_on_wakeup(void)
{
        hw_bod_deactivate_channel(BOD_CHANNEL_1V8);
        hw_bod_deactivate_channel(BOD_CHANNEL_1V8P);
        hw_bod_deactivate_channel(BOD_CHANNEL_1V8F);
        hw_bod_deactivate_channel(BOD_CHANNEL_1V4);
        hw_bod_deactivate_channel(BOD_CHANNEL_VDD);


        HW_PMU_1V8_RAIL_CONFIG _1v8_rail_config;
        if (hw_pmu_get_1v8_onwakeup_config(&_1v8_rail_config) == POWER_RAIL_ENABLED) {
                hw_bod_activate_channel(BOD_CHANNEL_1V8);
        }

        HW_PMU_1V8P_RAIL_CONFIG _1v8p_rail_config;
        if (hw_pmu_get_1v8p_onwakeup_config(&_1v8p_rail_config) == POWER_RAIL_ENABLED) {
                hw_bod_activate_channel(BOD_CHANNEL_1V8P);
        }

        HW_PMU_1V8F_RAIL_CONFIG _1v8f_rail_config;
        if (hw_pmu_get_1v8f_onwakeup_config(&_1v8f_rail_config) == POWER_RAIL_ENABLED) {
                hw_bod_activate_channel(BOD_CHANNEL_1V8F);
        }

        HW_PMU_1V4_RAIL_CONFIG _1v4_rail_config;
        if (hw_pmu_get_1v4_onwakeup_config(&_1v4_rail_config) == POWER_RAIL_ENABLED) {
                hw_bod_activate_channel(BOD_CHANNEL_1V4);
        }

        HW_PMU_1V2_RAIL_CONFIG _1v2_rail_config;
        if (hw_pmu_get_1v2_onwakeup_config(&_1v2_rail_config) == POWER_RAIL_ENABLED) {
                hw_bod_activate_channel(BOD_CHANNEL_VDD);
        }

}

void hw_bod_configure(void)
{
        hw_bod_activate_on_wakeup();

        /* Generate Reset on a BOD event */

        uint32_t mask = REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VBUS_RST_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VBAT_RST_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_VSYS_RST_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18_RST_EN)  |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18P_RST_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V18F_RST_EN) |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V14_RST_EN)  |
                        REG_MSK(CRG_TOP, BOD_CTRL_REG, BOD_V12_RST_EN);


        REG_SET_MASKED(CRG_TOP, BOD_CTRL_REG, mask, UINT32_MAX);
}
#endif /* dg_configUSE_BOD */

