/**
****************************************************************************************
*
* @file hw_pmu_da1470x.c
*
* @brief Power Manager Unit for DA1470x
*
* Copyright (C) 2020-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#if dg_configUSE_HW_PMU

#include "hw_pmu.h"
#include "sys_tcs.h"
#include <string.h>

/*
 * dependencies -----------
 *                   /
 * rail-------------<
 *                   \
 * dependants -------------
 */


#ifndef HW_PMU_SANITY_CHECKS_ENABLE
#       if dg_configIMAGE_SETUP == DEVELOPMENT_MODE
#               define HW_PMU_SANITY_CHECKS_ENABLE 1
#       else
#               define HW_PMU_SANITY_CHECKS_ENABLE 0
#       endif
#endif

typedef enum {
        HW_PMU_VLED_POWER_AYTO = 0,
        HW_PMU_VLED_POWER_MANUAL = 1,
        HW_PMU_VLED_POWER_MANUAL_NOT_POWERED = 0,
        HW_PMU_VLED_POWER_MANUAL_BY_VSYS = 1,
#if (DEVICE_VARIANT != DA14701)
        HW_PMU_VLED_POWER_MANUAL_BY_BOOST_DCDC = 2
#endif
} HW_PMU_VLED_POWER_SRC;

typedef enum {
        __LP_CLK_RCLP__          = 0,
        __LP_CLK_RCX__           = 1,
        __LP_CLK_XTAL32K__       = 2,
        __LP_CLK_XTAL32K_GEN__   = 3

} HW_PMU_LP_CLK;

/* Power pad mask for all ports. */
typedef enum {
        HW_PMU_PADPWR_P0 = REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_00_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_01_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_02_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_03_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_04_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_05_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_06_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_07_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_08_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_09_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_10_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_11_OUT_CTRL) |

                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_14_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_15_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_16_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_17_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_18_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_19_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_20_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_21_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_22_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_23_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_24_OUT_CTRL) |

                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_27_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_28_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_29_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_30_OUT_CTRL) |
                           REG_MSK(GPIO, P0_PADPWR_CTRL_REG, P0_31_OUT_CTRL),

        HW_PMU_PADPWR_P1 = REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_00_OUT_CTRL) |
                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_01_OUT_CTRL) |

                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_03_OUT_CTRL) |
                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_04_OUT_CTRL) |
                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_05_OUT_CTRL) |
                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_06_OUT_CTRL) |
                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_07_OUT_CTRL) |

                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_11_OUT_CTRL) |
                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_12_OUT_CTRL) |

                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_22_OUT_CTRL) |
                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_23_OUT_CTRL) |

                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_30_OUT_CTRL) |
                           REG_MSK(GPIO, P1_PADPWR_CTRL_REG, P1_31_OUT_CTRL),

        HW_PMU_PADPWR_P2 = REG_MSK(GPIO, P2_PADPWR_CTRL_REG, P2_01_OUT_CTRL) |

                           REG_MSK(GPIO, P2_PADPWR_CTRL_REG, P2_08_OUT_CTRL) |
                           REG_MSK(GPIO, P2_PADPWR_CTRL_REG, P2_09_OUT_CTRL) |
                           REG_MSK(GPIO, P2_PADPWR_CTRL_REG, P2_10_OUT_CTRL) |
                           REG_MSK(GPIO, P2_PADPWR_CTRL_REG, P2_11_OUT_CTRL)
} HW_PMU_PADPWR_MSK;

/* VSYS dependencies check masks */
typedef enum {
        HW_PMU_CHK_VSYS_COMP_OK_MSK                     = (1 << 0)
} HW_PMU_DEPENDENCY_CHK_VSYS_MSK;

/* VLED dependencies check masks */
typedef enum {
        HW_PMU_CHK_VLED_AUTO_MSK                        = (1 << 0),
        HW_PMU_CHK_VLED_MANUAL_BY_VSYS_MSK              = (1 << 1),
#if (DEVICE_VARIANT != DA14701)
        HW_PMU_CHK_VLED_MANUAL_BY_BOOST_DCDC_MSK        = (1 << 2)
#endif
} HW_PMU_DEPENDENCY_CHK_VLED_MSK;

/* VLED dependants check masks */
typedef enum {
        HW_PMU_CHK_VLED_PWMLED_MSK                      = (1 << 0)
} HW_PMU_DEPENDANT_CHK_VLED_MSK;

/* 3V0 dependencies check masks */
typedef enum {
        HW_PMU_CHK_3V0_LDO_MSK                          = (1 << 0),
        HW_PMU_CHK_3V0_LDO_RET_ACTIVE_MSK               = (1 << 1),
        HW_PMU_CHK_3V0_LDO_RET_SLEEP_MSK                = (1 << 2),
        HW_PMU_CHK_3V0_CLAMP_MSK                        = (1 << 3)
} HW_PMU_DEPENDENCY_CHK_3V0_MSK;

/* 3V0 dependants check masks */
typedef enum {
        HW_PMU_CHK_3V0_BANDGAP_MSK                      = (1 << 0),
        HW_PMU_CHK_3V0_GPIO_MSK                         = (1 << 1),
        HW_PMU_CHK_3V0_POR_MSK                          = (1 << 2),
        HW_PMU_CHK_3V0_USB_MSK                          = (1 << 3),
        HW_PMU_CHK_3V0_OTP_MSK                          = (1 << 4),
        HW_PMU_CHK_3V0_VAD_MSK                          = (1 << 5),
        HW_PMU_CHK_3V0_RCHS_MSK                         = (1 << 6),
        HW_PMU_CHK_3V0_RCX_MSK                          = (1 << 7),
        HW_PMU_CHK_3V0_LDO_START_MSK                    = (1 << 8)
} HW_PMU_DEPENDANT_CHK_3V0_MSK;

/* 1V8 dependencies check masks */
typedef enum {
        HW_PMU_CHK_1V8_SIMO_DCDC_ACTIVE_MSK             = (1 << 0),
        HW_PMU_CHK_1V8_SIMO_DCDC_SLEEP_MSK              = (1 << 1)
} HW_PMU_DEPENDENCY_CHK_1V8_MSK;

/* 1V8P dependencies check masks */
typedef enum {
        HW_PMU_CHK_1V8P_SIMO_DCDC_ACTIVE_MSK            = (1 << 0),
        HW_PMU_CHK_1V8P_SIMO_DCDC_SLEEP_MSK             = (1 << 1)
} HW_PMU_DEPENDENCY_CHK_1V8P_MSK;

/* 1V8P dependants check masks */
typedef enum {
        HW_PMU_CHK_1V8P_1V8F_MSK                        = (1 << 0),
        HW_PMU_CHK_1V8P_GPIO_MSK                        = (1 << 1),
        HW_PMU_CHK_1V8P_SDADC_MSK                       = (1 << 3),
        HW_PMU_CHK_1V8P_QSPI_MSK                        = (1 << 4)
} HW_PMU_DEPENDANT_CHK_1V8P_MSK;

/* 1V8F dependencies check masks */
typedef enum {
        HW_PMU_CHK_1V8F_1V8P_ACTIVE_MSK                 = (1 << 0),
        HW_PMU_CHK_1V8F_1V8P_SLEEP_MSK                  = (1 << 1)
} HW_PMU_DEPENDENCY_CHK_1V8F_MSK;

/* 1V8F dependants check masks */
typedef enum {
        HW_PMU_CHK_1V8F_OQSPI_MSK                       = (1 << 0),
} HW_PMU_DEPENDANT_CHK_1V8F_MSK;

/* 1V4 dependencies check masks */
typedef enum {
        HW_PMU_CHK_1V4_SIMO_DCDC_ACTIVE_MSK             = (1 << 0),
        HW_PMU_CHK_1V4_SIMO_DCDC_SLEEP_MSK              = (1 << 1)
} HW_PMU_DEPENDENCY_CHK_1V4_MSK;

/* 1V4 dependants check masks */
typedef enum {
        HW_PMU_CHK_1V4_XTAL32M_MSK                      = (1 << 0),
        HW_PMU_CHK_1V4_PLL_MSK                          = (1 << 1),
        HW_PMU_CHK_1V4_GPADC_MSK                        = (1 << 2),
        HW_PMU_CHK_1V4_1V4RF_MSK                        = (1 << 3)
} HW_PMU_DEPENDANT_CHK_1V4_MSK;

/* 1V2 dependencies check masks */
typedef enum {
        HW_PMU_CHK_1V2_SIMO_DCDC_ACTIVE_MSK             = (1 << 0),
        HW_PMU_CHK_1V2_SIMO_DCDC_SLEEP_MSK              = (1 << 1),
        HW_PMU_CHK_1V2_CLAMP_MSK                        = (1 << 2)
} HW_PMU_DEPENDENCY_CHK_1V2_MSK;

/* 1V2 dependants check masks */
typedef enum {
        HW_PMU_CHK_1V2_RCLP_MSK                         = (1 << 0),
        HW_PMU_CHK_1V2_XTAL32K_MSK                      = (1 << 1),
        HW_PMU_CHK_1V2_USB_MSK                          = (1 << 2),
        HW_PMU_CHK_1V2_OTP_MSK                          = (1 << 3),
        HW_PMU_CHK_1V2_HIGH_SPEED_CLK_MSK               = (1 << 4),
        HW_PMU_CHK_1V2_WAKEUP_UP_MSK                    = (1 << 5),
        HW_PMU_CHK_1V2_UFAST_WAKEUP_UP_0V75_MSK         = (1 << 6),
        HW_PMU_CHK_1V2_UFAST_WAKEUP_UP_0V90_MSK         = (1 << 7)
} HW_PMU_DEPENDANT_CHK_1V2_MSK;


/**************************************** Local Variables *****************************************/

/* Store the buck trim values for the rails. */
static __RETAINED_RW union {
        uint32_t __tcs_buck_trim;
        struct {
                /* Order and size matters, hence it should not change. */
                uint32_t __1v2_trim_1v20  : 4;
                uint32_t __1v2_trim_0v90  : 4;
                uint32_t __1v2_trim_0v75  : 4;
                /* cppcheck-suppress unusedStructMember */
                uint32_t __1v4_trim_1v40  : 4;
                uint32_t __1v8_trim_1v80  : 4;
                uint32_t __1v8_trim_1v20  : 4;
                /* cppcheck-suppress unusedStructMember */
                uint32_t __1v8p_trim_1v80 : 4;
                uint32_t                  : 4;
        };
} hw_pmu_tcs_buck_trim_values = {
        .__1v2_trim_1v20  = 0x8,
        .__1v2_trim_0v90  = 0x8,
        .__1v2_trim_0v75  = 0x8,
        .__1v4_trim_1v40  = 0x8,
        .__1v8_trim_1v80  = 0x8,
        .__1v8_trim_1v20  = 0x8,
        .__1v8p_trim_1v80 = 0x8
};

/************************************** Forward Declarations **************************************/

__STATIC_INLINE bool is_1v4_dcdc_active_enabled(void);
__STATIC_INLINE bool is_1v8_dcdc_active_enabled(void);


__STATIC_INLINE void vled_set_voltage_level(HW_PMU_VLED_VOLTAGE voltage)
{
        REG_SETF(DCDC_BOOST, BOOST_CTRL_REG0, BOOST_VLED_SEL, voltage);
}
__STATIC_INLINE HW_PMU_VLED_VOLTAGE vled_get_voltage_level(void)
{
        return REG_GETF(DCDC_BOOST, BOOST_CTRL_REG0, BOOST_VLED_SEL);
}

__STATIC_INLINE void vled_power_ctrl_enable(void)
{
        REG_SET_BIT(DCDC_BOOST, VLED_PWR_CTRL_REG, VLED_PWR_ENABLE);
}

__STATIC_INLINE bool is_vled_power_ctrl_enabled(void)
{
        return (REG_GETF(DCDC_BOOST, VLED_PWR_CTRL_REG, VLED_PWR_ENABLE));
}

__STATIC_INLINE bool is_vled_power_ctrl_auto(void)
{
        return (REG_GETF(DCDC_BOOST, VLED_PWR_CTRL_REG, VLED_PWR_MANUAL) == HW_PMU_VLED_POWER_AYTO);
}

__STATIC_INLINE bool is_vled_power_ctrl_manual_not_powered(void)
{
        return (REG_GETF(DCDC_BOOST, VLED_PWR_CTRL_REG, VLED_PWR_FORCE) == HW_PMU_VLED_POWER_MANUAL_NOT_POWERED);
}

__STATIC_INLINE bool is_vled_power_ctrl_manual_by_vsys(void)
{
        return (REG_GETF(DCDC_BOOST, VLED_PWR_CTRL_REG, VLED_PWR_FORCE) == HW_PMU_VLED_POWER_MANUAL_BY_VSYS);
}
#if (DEVICE_VARIANT != DA14701)
__STATIC_INLINE bool is_vled_power_ctrl_manual_by_boost_dcdc(void)
{
        return (REG_GETF(DCDC_BOOST, VLED_PWR_CTRL_REG, VLED_PWR_FORCE) == HW_PMU_VLED_POWER_MANUAL_BY_BOOST_DCDC);
}
#endif
__STATIC_INLINE void vled_dcdc_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_VLED_EN);
        /* 1. It is assumed that VLED_PWR_ENABLE is already set.
         * 2. Skip polling if BOOST DCDC operates in bypass mode.
         *    This happens when VSYS voltage level is close or above
         *    that of VLED.
         */
        while ((REG_GETF(CRG_TOP, ANA_STATUS_REG, BOOST_DCDC_VLED_OK) == 0) &&
               (REG_GETF(CRG_TOP, ANA_STATUS_REG, COMP_VSYS_NEAR_VLED) == 0));
}

__STATIC_INLINE void vled_dcdc_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_VLED_EN);
}

__STATIC_INLINE bool is_vled_dcdc_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_VLED_EN);
}

__STATIC_INLINE void vled_dcdc_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_VLED_SLEEP_EN);
}

__STATIC_INLINE void vled_dcdc_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_VLED_SLEEP_EN);
}

__STATIC_INLINE bool is_vled_dcdc_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_VLED_SLEEP_EN);
}

__STATIC_INLINE void vsys_set_voltage_level(HW_PMU_VSYS_VOLTAGE voltage)
{
        REG_SETF(CRG_TOP, POWER_LVL_REG, VSYS_LEVEL, voltage);
}

__STATIC_INLINE HW_PMU_VSYS_VOLTAGE vsys_get_voltage_level(void)
{
        return REG_GETF(CRG_TOP, POWER_LVL_REG, VSYS_LEVEL);
}

__STATIC_INLINE void __3v0_set_active_voltage_level(HW_PMU_3V0_VOLTAGE voltage)
{
        REG_SETF(CRG_TOP, POWER_LVL_REG, V30_LEVEL, voltage);
}

__STATIC_INLINE bool is_vsys_powered(void)
{
        return REG_GETF(CRG_TOP, ANA_STATUS_REG, COMP_VSYS_OK);
}

__STATIC_INLINE HW_PMU_3V0_VOLTAGE __3v0_get_active_voltage_level(void)
{
        return REG_GETF(CRG_TOP, POWER_LVL_REG, V30_LEVEL);
}

__STATIC_INLINE void __3v0_set_sleep_voltage_level(HW_PMU_3V0_VOLTAGE voltage)
{
        REG_SETF(CRG_TOP, POWER_LVL_REG, V30_SLEEP_LEVEL, voltage);
}

__STATIC_INLINE HW_PMU_3V0_VOLTAGE __3v0_get_sleep_voltage_level(void)
{
        return HW_PMU_3V0_VOLTAGE_SLEEP_3V0 +
               REG_GETF(CRG_TOP, POWER_LVL_REG, V30_SLEEP_LEVEL);
}

__STATIC_INLINE void __3v0_ldo_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_V30_EN);
        while (REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_V30_OK) == 0);
}

__STATIC_INLINE void __3v0_ldo_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_V30_EN);
}

__STATIC_INLINE void __3v0_ldo_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_V30_SLEEP_EN);
}

__STATIC_INLINE void __3v0_ldo_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_V30_SLEEP_EN);
}

__STATIC_INLINE void __3v0_ldo_ret_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_RET_V30_EN);
}

__STATIC_INLINE void __3v0_ldo_ret_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_RET_V30_EN);
}

__STATIC_INLINE void __3v0_ldo_ret_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, LDO_RET_V30_SLEEP_EN);
}

__STATIC_INLINE void __3v0_ldo_ret_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, LDO_RET_V30_SLEEP_EN);
}

__STATIC_INLINE void __3v0_clamp_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, CLAMP_V30_EN);
}

__STATIC_INLINE void __3v0_clamp_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, CLAMP_V30_EN);
}

__STATIC_INLINE bool is_3v0_ldo_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_V30_EN);
}

__STATIC_INLINE bool is_3v0_ldo_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_V30_SLEEP_EN);
}

__STATIC_INLINE bool is_3v0_ldo_ret_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_RET_V30_EN);
}

__STATIC_INLINE bool is_3v0_ldo_ret_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_RET_V30_SLEEP_EN);
}

__STATIC_INLINE bool is_3v0_clamp_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, CLAMP_V30_EN);
}

__STATIC_INLINE bool are_all_gpios_powered_by_3v0(void)
{
        /* All the reserved pins of Px_PADPWR_CTRL_REG are powered by 1V8P rail. */
        return false;
}

__STATIC_INLINE void __1v8_set_voltage_level(HW_PMU_1V8_VOLTAGE voltage)
{
        REG_SETF(CRG_TOP, POWER_LVL_REG, V18_LEVEL, voltage);

        /* Don't busy loop for ever in case the rail is not enabled yet.
         * Just apply the level setting.
         */
        if (is_1v8_dcdc_active_enabled()) {
                while (RAW_GETF(0x50000308, 0x4UL) == 0);
        }
}

__STATIC_INLINE HW_PMU_1V8_VOLTAGE __1v8_get_voltage_level(void)
{
        return REG_GETF(CRG_TOP, POWER_LVL_REG, V18_LEVEL);
}

__STATIC_INLINE void __1v8_dcdc_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V18_EN);
        while (REG_GETF(CRG_TOP, ANA_STATUS_REG, BUCK_DCDC_V18_OK) == 0);
}

__STATIC_INLINE void __1v8_dcdc_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V18_EN);
}

__STATIC_INLINE void __1v8_dcdc_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V18_SLEEP_EN);
}

__STATIC_INLINE void __1v8_dcdc_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V18_SLEEP_EN);
}

__STATIC_INLINE bool is_1v8_dcdc_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V18_EN);
}

__STATIC_INLINE bool is_1v8_dcdc_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V18_SLEEP_EN);
}

__STATIC_INLINE bool is_bod_on_1v8_active(void)
{
        return REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V18_EN);
}

__STATIC_INLINE bool is_1v8_simo_dcdc_active_ok(void)
{
        return true;
}

__STATIC_INLINE bool is_1v8_simo_dcdc_sleep_ok(void)
{
        return true;
}

static HW_PMU_ERROR_CODE hw_pmu_1v8_set_trim(HW_PMU_1V8_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (voltage) {
        case HW_PMU_1V8_VOLTAGE_1V2:
                RAW_SETF(0x500000D0, 0x780000UL, hw_pmu_tcs_buck_trim_values.__1v8_trim_1v20);
                break;
        case HW_PMU_1V8_VOLTAGE_1V8:
                RAW_SETF(0x500000D0, 0x780000UL, hw_pmu_tcs_buck_trim_values.__1v8_trim_1v80);
                break;
        default:
                res =  HW_PMU_ERROR_INVALID_ARGS;
        }

        return res;
}

__STATIC_INLINE void __1v8p_dcdc_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V18P_EN);
        while (REG_GETF(CRG_TOP, ANA_STATUS_REG, BUCK_DCDC_V18P_OK) == 0);
}

__STATIC_INLINE void __1v8p_dcdc_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V18P_EN);
}

__STATIC_INLINE void __1v8p_dcdc_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V18P_SLEEP_EN);
}

__STATIC_INLINE void __1v8p_dcdc_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V18P_SLEEP_EN);
}

__STATIC_INLINE bool is_1v8p_dcdc_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V18P_EN);
}

__STATIC_INLINE bool is_1v8p_dcdc_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V18P_SLEEP_EN);
}

__STATIC_INLINE bool are_all_gpios_powered_by_1v8p(void)
{
        return ((GPIO->P0_PADPWR_CTRL_REG & HW_PMU_PADPWR_P0) == HW_PMU_PADPWR_P0) &&
               ((GPIO->P1_PADPWR_CTRL_REG & HW_PMU_PADPWR_P1) == HW_PMU_PADPWR_P1) &&
               ((GPIO->P2_PADPWR_CTRL_REG & HW_PMU_PADPWR_P2) == HW_PMU_PADPWR_P2);
}

__STATIC_INLINE bool is_bod_on_1v8p_active(void)
{
        return REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V18P_EN);
}

__STATIC_FORCEINLINE bool is_1v8p_simo_dcdc_active_ok(void)
{
        return true;
}

__STATIC_FORCEINLINE bool is_1v8p_simo_dcdc_sleep_ok(void)
{
        return true;
}

__STATIC_FORCEINLINE void __1v8f_sw_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, SW_V18F_ON);
        while (REG_GETF(CRG_TOP, ANA_STATUS_REG, SWITCH_V18F_OK) == 0);
}

__STATIC_FORCEINLINE void __1v8f_sw_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, SW_V18F_ON);
}

__STATIC_FORCEINLINE void __1v8f_sw_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, SW_V18F_SLEEP_ON);
}

__STATIC_FORCEINLINE void __1v8f_sw_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, SW_V18F_SLEEP_ON);
}

__STATIC_INLINE bool is_1v8f_sw_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, SW_V18F_ON);
}

__STATIC_INLINE bool is_1v8f_sw_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, SW_V18F_SLEEP_ON);
}

__STATIC_FORCEINLINE bool is_bod_on_1v8f_active(void)
{
        return REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V18F_EN);
}

__STATIC_INLINE void __1v4_set_voltage_level(HW_PMU_1V4_VOLTAGE voltage)
{
        REG_SETF(CRG_TOP, POWER_LVL_REG, V14_LEVEL, voltage);

        /* Don't busy loop for ever in case the rail is not enabled yet.
         * Just apply the level setting.
         */
        if (is_1v4_dcdc_active_enabled()) {
                while (RAW_GETF(0x50000308, 0x2UL) == 0);
        }
}

__STATIC_INLINE HW_PMU_1V4_VOLTAGE __1v4_get_voltage_level(void)
{
        return REG_GETF(CRG_TOP, POWER_LVL_REG, V14_LEVEL);
}

__STATIC_INLINE void __1v4_dcdc_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V14_EN);
        while (REG_GETF(CRG_TOP, ANA_STATUS_REG, BUCK_DCDC_V14_OK) == 0);
}

__STATIC_INLINE void __1v4_dcdc_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V14_EN);
}

__STATIC_INLINE void __1v4_dcdc_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V14_SLEEP_EN);
}

__STATIC_INLINE void __1v4_dcdc_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V14_SLEEP_EN);
}

__STATIC_INLINE bool is_1v4_dcdc_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V14_EN);
}

__STATIC_INLINE bool is_1v4_dcdc_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V14_SLEEP_EN);
}

__STATIC_INLINE bool is_bod_on_1v4_active(void)
{
        return REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V14_EN);
}

__STATIC_INLINE bool is_1v4_simo_dcdc_active_ok(void)
{
        return true;
}

__STATIC_INLINE bool is_1v4_simo_dcdc_sleep_ok(void)
{
        return true;
}

__STATIC_INLINE void __1v2_set_active_voltage_level(HW_PMU_1V2_VOLTAGE voltage)
{
        REG_SETF(CRG_TOP, POWER_LVL_REG, V12_LEVEL, voltage);
        /* No point to check for 1V2 being enabled since it would always be so. */
        while (RAW_GETF(0x50000308, 0x1UL) == 0);
}

__STATIC_INLINE HW_PMU_1V2_VOLTAGE __1v2_get_active_voltage_level(void)
{
        return REG_GETF(CRG_TOP, POWER_LVL_REG, V12_LEVEL);
}

__STATIC_INLINE void __1v2_set_sleep_voltage_level(HW_PMU_1V2_VOLTAGE voltage)
{
        REG_SETF(CRG_TOP, POWER_LVL_REG, V12_SLEEP_LEVEL, voltage);
}

__STATIC_INLINE HW_PMU_1V2_VOLTAGE __1v2_get_sleep_voltage_level(void)
{
        return HW_PMU_1V2_VOLTAGE_SLEEP_0V75 +
               REG_GETF(CRG_TOP, POWER_LVL_REG, V12_SLEEP_LEVEL);
}

__STATIC_INLINE void __1v2_dcdc_active_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V12_EN);
        while (REG_GETF(CRG_TOP, ANA_STATUS_REG, BUCK_DCDC_V12_OK) == 0);
}

__STATIC_INLINE void __1v2_dcdc_active_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V12_EN);
}

__STATIC_INLINE void __1v2_dcdc_sleep_enable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V12_SLEEP_EN);
}

__STATIC_INLINE void __1v2_dcdc_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, DCDC_V12_SLEEP_EN);
}

__STATIC_INLINE void __1v2_clamp_enable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, CLAMP_V12_DIS);
}

__STATIC_INLINE void __1v2_clamp_disable(void)
{
        REG_SET_BIT(CRG_TOP, POWER_CTRL_REG, CLAMP_V12_DIS);
}

__STATIC_INLINE bool is_1v2_dcdc_active_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V12_EN);
}

__STATIC_INLINE bool is_1v2_dcdc_sleep_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V12_SLEEP_EN);
}

__STATIC_INLINE bool is_1v2_clamp_enabled(void)
{
        return !REG_GETF(CRG_TOP, POWER_CTRL_REG, CLAMP_V12_DIS);
}

__STATIC_INLINE bool is_bod_on_1v2_active(void)
{
        return REG_GETF(CRG_TOP, BOD_CTRL_REG, BOD_V12_EN);
}

__STATIC_INLINE bool is_1v2_simo_dcdc_active_ok(void)
{
        return true;
}

__STATIC_INLINE bool is_1v2_ldo_start_enabled(void)
{
        return REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_START_DISABLE) == 0;
}

__STATIC_INLINE bool is_1v2_simo_dcdc_sleep_ok(void)
{
        return true;
}

static __RETAINED_HOT_CODE HW_PMU_ERROR_CODE hw_pmu_1v2_set_trim(HW_PMU_1V2_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (voltage) {
        case HW_PMU_1V2_VOLTAGE_0V75:
        case HW_PMU_1V2_VOLTAGE_SLEEP_0V75:
                RAW_SETF(0x500000D0, 0x7800UL, hw_pmu_tcs_buck_trim_values.__1v2_trim_0v75);
                break;
        case HW_PMU_1V2_VOLTAGE_0V90:
        case HW_PMU_1V2_VOLTAGE_SLEEP_0V90:
                RAW_SETF(0x500000D0, 0x7800UL, hw_pmu_tcs_buck_trim_values.__1v2_trim_0v90);
                break;
        case HW_PMU_1V2_VOLTAGE_1V20:
        case HW_PMU_1V2_VOLTAGE_SLEEP_1V20:
                RAW_SETF(0x500000D0, 0x7800UL, hw_pmu_tcs_buck_trim_values.__1v2_trim_1v20);
                break;

        default:
                res =  HW_PMU_ERROR_INVALID_ARGS;
        }

        return res;
}


/* Generic helper functions not bound to a specific rail. */

__STATIC_INLINE bool is_usb_enabled(void)
{
        return REG_GETF(USB, USB_MCTRL_REG, USBEN);
}

__STATIC_INLINE bool is_por_enabled(void)
{
        enum {HW_PMU_POR_PIN_DISABLED = 0x7F};

        return !(REG_GETF(CRG_TOP, POR_PIN_REG, POR_PIN_SELECT) == HW_PMU_POR_PIN_DISABLED);
}

__STATIC_INLINE bool is_rchs_enabled(void)
{
        return (REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_RCHS) ||
                REG_GETF(CRG_TOP, CLK_RCHS_REG, RCHS_ENABLE));
}

__STATIC_INLINE bool is_rcx_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_RCX_REG, RCX_ENABLE);
}

__STATIC_INLINE bool is_rcx_lp_clk(void)
{
           return (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == __LP_CLK_RCX__);
}

__STATIC_INLINE bool is_pll_enabled(void)
{
        return (REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, PLL_LOCK_FINE)   ||
                REG_GETF(CRG_XTAL, PLL_USB_STATUS_REG, PLL_LOCK_FINE)   ||
                REG_GETF(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_EN)           ||
                REG_GETF(CRG_XTAL, PLL_USB_CTRL1_REG, PLL_EN));
}

__STATIC_INLINE bool is_xtal32m_enabled(void)
{
        return REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_READY);
}

__STATIC_INLINE bool is_gpadc_enabled(void)
{
        if (REG_GETF(CRG_TOP, SYS_STAT_REG, SNC_IS_UP)) {
                return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN);
        } else {
                return false;
        }
}

__STATIC_INLINE bool is_sdadc_enabled(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_LDO_OK);
}

__STATIC_INLINE bool is_rf_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_RADIO_REG, RFCU_ENABLE);

}

__STATIC_INLINE bool is_xtal32k_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE);

}

__STATIC_INLINE bool is_xtal32k_lp_clk(void)
{
        return (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == __LP_CLK_XTAL32K__);
}

__STATIC_INLINE bool is_rclp_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_RCLP_REG, RCLP_ENABLE);

}

__STATIC_INLINE bool is_rclp_lp_clk(void)
{
        return (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == __LP_CLK_RCLP__);
}

__STATIC_INLINE bool is_otp_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_AMBA_REG, OTP_ENABLE);
}

__STATIC_FORCEINLINE bool is_oqspi_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_AMBA_REG, OQSPIF_ENABLE);
}

__STATIC_FORCEINLINE bool is_qspi_enabled(void)
{
        return (REG_GETF(CRG_TOP, CLK_AMBA_REG, QSPIC_ENABLE) ||
                REG_GETF(CRG_TOP, CLK_AMBA_REG, QSPIC2_ENABLE));
}

__STATIC_INLINE bool is_rchs_high_speed_enabled(void)
{
        enum { HW_PMU_RCHS_SPEED_32 =  0,
               HW_PMU_RCHS_SPEED_96 =  1,
               HW_PMU_RCHS_SPEED_64 =  2
        };

        return (REG_GETF(CRG_TOP, CLK_RCHS_REG, RCHS_SPEED) != HW_PMU_RCHS_SPEED_32);
}

__STATIC_INLINE bool are_leds_enabled(void)
{
        return (REG_GETF(PWMLED, LEDS_DRV_CTRL_REG, LED1_EN) ||
                REG_GETF(PWMLED, LEDS_DRV_CTRL_REG, LED2_EN) ||
                REG_GETF(PWMLED, LEDS_DRV_CTRL_REG, LED3_EN));
}

__STATIC_INLINE bool is_vad_wakeup_src(void)
{
        enum {HW_PMU_MAX_PDC_ENTRIES = 0x10, HW_PMU_VAD_TRIGGER_SRC = 0x9};

        for (uint32_t i = 0; i < HW_PMU_MAX_PDC_ENTRIES; i++) {
                if (REG_GETF_INDEXED(PDC, PDC_CTRL0_REG, TRIG_ID, 0x4, i) == HW_PMU_VAD_TRIGGER_SRC) {
                        return true;
                }
        }

        return false;
}

__STATIC_INLINE bool is_ufast_wakeup_mode_enabled(void)
{
        return (REG_GETF(CRG_TOP, PMU_SLEEP_REG, ULTRA_FAST_WAKEUP)) ;
}

/* Dependency / dependant helper functions. */

__STATIC_INLINE HW_PMU_ERROR_CODE check_vsys_dependencies_active(HW_PMU_DEPENDENCY_CHK_VSYS_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_VSYS_COMP_OK_MSK) {
                if (is_vsys_powered()) {
                        return HW_PMU_ERROR_NOERROR;
                }
        }
        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vsys_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_VSYS_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        return check_vsys_dependencies_active(mask);
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vled_dependencies_active(HW_PMU_DEPENDENCY_CHK_VLED_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_VLED_AUTO_MSK) {
                /* The power ctrl can be automatically selected. The power source could
                 * be either VSYS or BOOST DCDC depending on VLED voltage level.
                 * if it is near VSYS level, then VSYS rail is used. If it is higher
                 * then BOOST DCDC becomes its power source.
                 */
                return check_vsys_dependencies_active(HW_PMU_CHK_VSYS_COMP_OK_MSK);
        }
#if (DEVICE_VARIANT != DA14701)
        if (mask & HW_PMU_CHK_VLED_MANUAL_BY_BOOST_DCDC_MSK) {
                /* The power ctrl can be manually selected. The power source
                 * is BOOST DCDC. VLED voltage level could be set above VSYS level.
                 */
                return check_vsys_dependencies_active(HW_PMU_CHK_VSYS_COMP_OK_MSK);
        }
#endif
        if (mask & HW_PMU_CHK_VLED_MANUAL_BY_VSYS_MSK) {
                /* The power ctrl can be manually selected. The power source is
                 * is VSYS. VLED voltage level should be set near VSYS level.
                 */
                return check_vsys_dependencies_active(HW_PMU_CHK_VSYS_COMP_OK_MSK);

        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vled_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_VLED_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        return check_vled_dependencies_active(mask);
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_3v0_dependencies_active(HW_PMU_DEPENDENCY_CHK_3V0_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_3V0_LDO_MSK) {
                /* LDO_V30 can be enabled in active mode. */
                return check_vsys_dependencies_active(HW_PMU_CHK_VSYS_COMP_OK_MSK);
        }

        if (mask & HW_PMU_CHK_3V0_LDO_RET_ACTIVE_MSK) {
                /* LDO_V30_RET can be enabled in active mode. */
                return check_vsys_dependencies_active(HW_PMU_CHK_VSYS_COMP_OK_MSK);
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_3v0_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_3V0_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_3V0_LDO_MSK) {
                /* LDO_V30 can be enabled in sleep mode. */
                return check_vsys_dependencies_sleep(HW_PMU_CHK_VSYS_COMP_OK_MSK);
        }

        if (mask & HW_PMU_CHK_3V0_LDO_RET_SLEEP_MSK) {
                /* LDO_V30_RET can be enabled in sleep mode. */
                return check_vsys_dependencies_sleep(HW_PMU_CHK_VSYS_COMP_OK_MSK);
        }

        if (mask & HW_PMU_CHK_3V0_CLAMP_MSK) {
                /* 3V0 Low Power Clamp can always be enabled. */
                return HW_PMU_ERROR_NOERROR;
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v8_dependencies_active(HW_PMU_DEPENDENCY_CHK_1V8_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8_SIMO_DCDC_ACTIVE_MSK) {
                if (is_1v8_simo_dcdc_active_ok()) {
                        /* SIMO DCDC is sane. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v8_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_1V8_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8_SIMO_DCDC_SLEEP_MSK) {
                if (is_1v8_simo_dcdc_sleep_ok()) {
                        /* SIMO DCDC is sane. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_FORCEINLINE HW_PMU_ERROR_CODE check_1v8p_dependencies_active(HW_PMU_DEPENDENCY_CHK_1V8P_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8P_SIMO_DCDC_ACTIVE_MSK) {
                if (is_1v8p_simo_dcdc_active_ok()) {
                        /* SIMO DCDC is sane. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_FORCEINLINE HW_PMU_ERROR_CODE check_1v8p_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_1V8P_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8P_SIMO_DCDC_SLEEP_MSK) {
                if (is_1v8p_simo_dcdc_sleep_ok()) {
                        /* SIMO DCDC is sane. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_FORCEINLINE HW_PMU_ERROR_CODE check_1v8f_dependencies_active(HW_PMU_DEPENDENCY_CHK_1V8F_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8F_1V8P_ACTIVE_MSK) {
               return check_1v8p_dependencies_active(HW_PMU_CHK_1V8P_SIMO_DCDC_ACTIVE_MSK);
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_FORCEINLINE HW_PMU_ERROR_CODE check_1v8f_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_1V8F_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8F_1V8P_SLEEP_MSK) {
               return check_1v8p_dependencies_sleep(HW_PMU_CHK_1V8P_SIMO_DCDC_SLEEP_MSK);
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v4_dependencies_active(HW_PMU_DEPENDENCY_CHK_1V4_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V4_SIMO_DCDC_ACTIVE_MSK) {
                if (is_1v4_simo_dcdc_active_ok()) {
                        /* SIMO DCDC is sane. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v4_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_1V4_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V4_SIMO_DCDC_SLEEP_MSK) {
                if (is_1v4_simo_dcdc_sleep_ok()) {
                        /* SIMO DCDC is sane. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v2_dependencies_active(HW_PMU_DEPENDENCY_CHK_1V2_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V2_SIMO_DCDC_ACTIVE_MSK) {
                if (is_1v2_simo_dcdc_active_ok()) {
                        /* SIMO DCDC is sane. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v2_dependencies_sleep(HW_PMU_DEPENDENCY_CHK_1V2_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V2_SIMO_DCDC_SLEEP_MSK) {
                if (is_1v2_simo_dcdc_sleep_ok()) {
                        /* SIMO DCDC is sane. */
                        return HW_PMU_ERROR_NOERROR;
                }
        }

        if (mask & HW_PMU_CHK_1V2_CLAMP_MSK) {
                /* 1V2 Low Power Clamp can always be enabled. */
                return HW_PMU_ERROR_NOERROR;
        }

        return HW_PMU_ERROR_NOT_ENOUGH_POWER;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}


__STATIC_INLINE HW_PMU_ERROR_CODE check_vled_dependants_active(HW_PMU_DEPENDANT_CHK_VLED_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_VLED_PWMLED_MSK) {
                       if (are_leds_enabled()) {
                               /* LED is on. */
                               return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                       }
               }
        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_vled_dependants_sleep(HW_PMU_DEPENDANT_CHK_VLED_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        return check_vled_dependants_active(mask);
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_3v0_dependants_active(HW_PMU_DEPENDANT_CHK_3V0_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_3V0_BANDGAP_MSK) {
                /* Rail powers bandgap. */
                return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
        }

        if (mask & HW_PMU_CHK_3V0_GPIO_MSK) {
                if (!are_all_gpios_powered_by_1v8p()) {
                        /* Some GPIOs powered by 3V0. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_3V0_POR_MSK) {
                if (is_por_enabled()) {
                        /* POR block is needed. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_3V0_USB_MSK) {
                if (is_usb_enabled()) {
                        /* USB is on. */
                        return HW_PMU_ERROR_USB_PHY_ON;
                }
        }

        if (mask & HW_PMU_CHK_3V0_RCHS_MSK) {
                if (is_rchs_enabled()) {
                        /* RCHS is on. */
                        return HW_PMU_ERROR_RCHS_ON;
                }
        }

        if (mask & HW_PMU_CHK_3V0_RCX_MSK) {
                if (is_rcx_enabled()) {
                        /* RCX is on. */
                        return HW_PMU_ERROR_RCX_ON;
                }
        }

        if (mask & HW_PMU_CHK_3V0_OTP_MSK) {
                if (is_otp_enabled()) {
                        /* OTP is on. */
                        return HW_PMU_ERROR_OTP_ON;
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_3v0_dependants_sleep(HW_PMU_DEPENDANT_CHK_3V0_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_3V0_BANDGAP_MSK) {

                return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
        }

        if (mask & HW_PMU_CHK_3V0_GPIO_MSK) {
                if (!are_all_gpios_powered_by_1v8p()) {
                        /* Some GPIOs powered by 3V0. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_3V0_POR_MSK) {
                if (is_por_enabled()) {
                        /* POR block is needed. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_3V0_RCX_MSK) {
                if (is_rcx_lp_clk()) {
                        /* RCX is set as LP clock. */
                        return HW_PMU_ERROR_RCX_LP;
                }
        }

        if (mask & HW_PMU_CHK_3V0_VAD_MSK) {
                if (is_vad_wakeup_src()) {
                        /* VAD set as a wake up source. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_3V0_LDO_START_MSK) {
                if (is_1v2_ldo_start_enabled()) {
                        /* LDO_START is enabled. */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v8p_dependants_active(HW_PMU_DEPENDANT_CHK_1V8P_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8P_1V8F_MSK) {
                if (is_1v8f_sw_active_enabled()) {
                        /* 1V8F is connected to 1V8P during active mode. */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & HW_PMU_CHK_1V8P_GPIO_MSK) {
                if (!are_all_gpios_powered_by_3v0()) {
                        /* Some GPIOs powered by 1V8P. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }


        if (mask & HW_PMU_CHK_1V8P_SDADC_MSK) {
                if (is_sdadc_enabled()) {
                        /*  SDADC is on. */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & HW_PMU_CHK_1V8P_QSPI_MSK) {
                if (is_qspi_enabled()) {
                        /* QSPIC is on. */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v8p_dependants_sleep(HW_PMU_DEPENDANT_CHK_1V8P_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8P_1V8F_MSK) {
                if (is_1v8f_sw_sleep_enabled()) {
                        /* 1V8F is connected to 1V8P during sleep mode. */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & HW_PMU_CHK_1V8P_GPIO_MSK) {
                if (!are_all_gpios_powered_by_3v0()) {
                        /* Some GPIOs powered by 1V8P. */
                        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
                }
        }

        if (mask & HW_PMU_CHK_1V8P_QSPI_MSK) {
                if (is_qspi_enabled()) {
                        /* QSPIC is on. */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_FORCEINLINE HW_PMU_ERROR_CODE check_1v8f_dependants_active(HW_PMU_DEPENDANT_CHK_1V8F_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V8F_OQSPI_MSK) {
                if (is_oqspi_enabled()) {
                        /* OQSPIC is on. */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_FORCEINLINE HW_PMU_ERROR_CODE check_1v8f_dependants_sleep(HW_PMU_DEPENDANT_CHK_1V8F_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        return check_1v8f_dependants_active(mask);
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v4_dependants_active(HW_PMU_DEPENDANT_CHK_1V4_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V4_XTAL32M_MSK) {
                if (is_xtal32m_enabled()) {
                        /* XTAL32M is on . */
                        return HW_PMU_ERROR_XTAL32M_ON;
                }
        }

        if (mask & HW_PMU_CHK_1V4_PLL_MSK) {
                if (is_pll_enabled()) {
                        /* PLL is on . */
                        return HW_PMU_ERROR_PLL_ON;
                }
        }

        if (mask & HW_PMU_CHK_1V4_GPADC_MSK) {
                if (is_gpadc_enabled()) {
                        /*  GPADC is on. */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        if (mask & HW_PMU_CHK_1V4_1V4RF_MSK) {
                if (is_rf_enabled()) {
                        /*  RF is on. */
                        return HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY;
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v2_dependants_active(HW_PMU_DEPENDANT_CHK_1V2_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V2_RCLP_MSK) {
                if (is_rclp_enabled()) {
                        /* RCLP is on. */
                        return HW_PMU_ERROR_RCLP_ON;
                }
        }

        if (mask & HW_PMU_CHK_1V2_XTAL32K_MSK) {
                if (is_xtal32k_enabled()) {
                        /* XTAL32K is on . */
                        return HW_PMU_ERROR_XTAL32K_ON;
                }
        }

        if (mask & HW_PMU_CHK_1V2_USB_MSK) {
                if (is_usb_enabled()) {
                        /* USB is on. */
                        return HW_PMU_ERROR_USB_PHY_ON;
                }
        }

        if (mask & HW_PMU_CHK_1V2_OTP_MSK) {
                if (is_otp_enabled()) {
                        /* OTP is on. */
                        return HW_PMU_ERROR_OTP_ON;
                }
        }

        if (mask & HW_PMU_CHK_1V2_HIGH_SPEED_CLK_MSK) {
                if (is_pll_enabled()) {
                        /* PLL is on . */
                        return HW_PMU_ERROR_HIGH_SPEED_CLK_ON;
                }

                if (is_rchs_high_speed_enabled()) {
                        /* RCHS speed greater than 32 MHz. */
                        return HW_PMU_ERROR_HIGH_SPEED_CLK_ON;
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

__STATIC_INLINE HW_PMU_ERROR_CODE check_1v2_dependants_sleep(HW_PMU_DEPENDANT_CHK_1V2_MSK mask)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        if (mask & HW_PMU_CHK_1V2_XTAL32K_MSK) {
                if (is_xtal32k_lp_clk()) {
                        /* XTAL32K is set as LP clock. */
                        return HW_PMU_ERROR_XTAL32K_LP;
                }
        }

        if (mask & HW_PMU_CHK_1V2_RCLP_MSK) {
                if (is_rclp_lp_clk()) {
                        /* RCLP is set as LP clock. */
                        return HW_PMU_ERROR_RCLP_LP;
                }
        }

        if (mask & HW_PMU_CHK_1V2_WAKEUP_UP_MSK) {
                if (is_ufast_wakeup_mode_enabled()) {
                        /* A fast wakeup mode is enabled. */
                        return HW_PMU_ERROR_WAKEUP_SOURCE_ON;
                }

                if (is_vad_wakeup_src()) {
                        /* VAD set as a wake up source. */
                        return HW_PMU_ERROR_WAKEUP_SOURCE_ON;
                }
        }

        if (mask & HW_PMU_CHK_1V2_UFAST_WAKEUP_UP_0V75_MSK) {
                if (is_ufast_wakeup_mode_enabled()) {
                        /* A fast wakeup mode is enabled. */
                        return HW_PMU_ERROR_UFAST_WAKEUP_ON;
                }

        }

        if (mask & HW_PMU_CHK_1V2_UFAST_WAKEUP_UP_0V90_MSK) {
                if (is_ufast_wakeup_mode_enabled()) {
                        if (is_rchs_high_speed_enabled()) {
                                /* RCHS speed greater than 32 MHz. */
                                return HW_PMU_ERROR_HIGH_SPEED_CLK_ON;
                        }
                }
        }

        return HW_PMU_ERROR_NOERROR;
#else
        return HW_PMU_ERROR_NOERROR;
#endif
}

HW_PMU_ERROR_CODE hw_pmu_vled_set_voltage(HW_PMU_VLED_VOLTAGE voltage)
{

#if (DEVICE_VARIANT == DA14701)
        return HW_PMU_ERROR_ACTION_NOT_POSSIBLE;
#else

        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (voltage) {
        case HW_PMU_VLED_VOLTAGE_4V5:
        case HW_PMU_VLED_VOLTAGE_4V75:
        case HW_PMU_VLED_VOLTAGE_5V0:
                vled_set_voltage_level(voltage);
                break;
        default:
                res =  HW_PMU_ERROR_INVALID_ARGS;
        }

        return res;
#endif
}

HW_PMU_ERROR_CODE hw_pmu_vled_onwakeup_enable(HW_PMU_VLED_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_VLED_MAX_LOAD_0_300:
                /* Applicable only in sleep mode. */
                res = HW_PMU_ERROR_INVALID_ARGS;
                break;
        case HW_PMU_VLED_MAX_LOAD_150:
                res = check_vled_dependencies_active(HW_PMU_CHK_VLED_AUTO_MSK                   |
#if (DEVICE_VARIANT != DA14701)
                                                     HW_PMU_CHK_VLED_MANUAL_BY_BOOST_DCDC_MSK   |
#endif
                                                     HW_PMU_CHK_VLED_MANUAL_BY_VSYS_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;

        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_VLED_MAX_LOAD_0_300:
                        /* Applicable only in sleep mode. */
                        res = HW_PMU_ERROR_INVALID_ARGS;
                        break;
                case HW_PMU_VLED_MAX_LOAD_150:
                        /* VLED power ctrl needs to be set whenever the BOOST DCDC converter has to
                         * be functional during active or sleep period.
                         */
                        vled_power_ctrl_enable();
                        vled_dcdc_active_enable();
                        break;
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_vled_onwakeup_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;

        res = check_vled_dependants_active(HW_PMU_CHK_VLED_PWMLED_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif
        vled_dcdc_active_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_vled_onsleep_enable(HW_PMU_VLED_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_VLED_MAX_LOAD_0_300:
                res = check_vled_dependencies_sleep(HW_PMU_CHK_VLED_AUTO_MSK                   |
#if (DEVICE_VARIANT != DA14701)
                                                    HW_PMU_CHK_VLED_MANUAL_BY_BOOST_DCDC_MSK   |
#endif
                                                    HW_PMU_CHK_VLED_MANUAL_BY_VSYS_MSK);
                break;
        case HW_PMU_VLED_MAX_LOAD_150:
                /* Applicable only in active mode. */
                res = HW_PMU_ERROR_INVALID_ARGS;
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;

        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_VLED_MAX_LOAD_0_300:
                        /* VLED power ctrl needs to be set whenever the BOOST DCDC converter has to
                         * be functional during active or sleep period.
                         */
                        vled_power_ctrl_enable();
                        vled_dcdc_sleep_enable();
                        break;
                case HW_PMU_VLED_MAX_LOAD_150:
                        /* Applicable only in active mode. */
                        res = HW_PMU_ERROR_INVALID_ARGS;
                        break;
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_vled_onsleep_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;

        res = check_vled_dependants_sleep(HW_PMU_CHK_VLED_PWMLED_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif
        vled_dcdc_sleep_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vled_active_config(HW_PMU_VLED_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_VLED_RAIL_CONFIG));

        if (is_vled_power_ctrl_enabled()) {
                if (is_vled_power_ctrl_auto() && is_vled_dcdc_active_enabled()) {
                        r_state = POWER_RAIL_ENABLED;
                        rail_config->voltage = vled_get_voltage_level();
                        rail_config->current = HW_PMU_VLED_MAX_LOAD_150;
                        rail_config->src_type = HW_PMU_SRC_TYPE_AUTO;
                } else if (!is_vled_power_ctrl_auto()) {
                        /* manual power selection */
                        if (is_vled_power_ctrl_manual_not_powered()) {
                                r_state = POWER_RAIL_DISABLED;
                        } else if (is_vled_power_ctrl_manual_by_vsys()) {
                                r_state = POWER_RAIL_ENABLED;
                                rail_config->voltage = vled_get_voltage_level();
                                rail_config->current = HW_PMU_VLED_MAX_LOAD_150;
                                rail_config->src_type = HW_PMU_SRC_TYPE_VSYS;
#if (DEVICE_VARIANT != DA14701)
                        } else if (is_vled_power_ctrl_manual_by_boost_dcdc() &&
                                   is_vled_dcdc_active_enabled()) {
                                r_state = POWER_RAIL_ENABLED;
                                rail_config->voltage = vled_get_voltage_level();
                                rail_config->current = HW_PMU_VLED_MAX_LOAD_150;
                                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
#endif
                        } else {
                                /* We should not reach here. */
                                ASSERT_WARNING(0);
                                return POWER_RAIL_DISABLED;
                        }

                } else {
                        return POWER_RAIL_DISABLED;
                }
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vled_onwakeup_config(HW_PMU_VLED_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_vled_active_config(rail_config);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vled_onsleep_config(HW_PMU_VLED_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_VLED_RAIL_CONFIG));

        if (is_vled_power_ctrl_enabled()) {
                if (is_vled_power_ctrl_auto() && is_vled_dcdc_sleep_enabled()) {
                        r_state = POWER_RAIL_ENABLED;
                        rail_config->voltage = vled_get_voltage_level();
                        rail_config->current = HW_PMU_VLED_MAX_LOAD_0_300;
                        rail_config->src_type = HW_PMU_SRC_TYPE_AUTO;
                } else if  (!is_vled_power_ctrl_auto()) {
                        /* manual power selection */
                        if (is_vled_power_ctrl_manual_not_powered()) {
                                r_state = POWER_RAIL_DISABLED;
                        } else if (is_vled_power_ctrl_manual_by_vsys()) {
                                r_state = POWER_RAIL_ENABLED;
                                rail_config->voltage = vled_get_voltage_level();
                                rail_config->current = HW_PMU_VLED_MAX_LOAD_0_300;
                                rail_config->src_type = HW_PMU_SRC_TYPE_VSYS;
#if (DEVICE_VARIANT != DA14701)
                        } else if (is_vled_power_ctrl_manual_by_boost_dcdc() &&
                                   is_vled_dcdc_sleep_enabled()) {
                                r_state = POWER_RAIL_ENABLED;
                                rail_config->voltage = vled_get_voltage_level();
                                rail_config->current = HW_PMU_VLED_MAX_LOAD_0_300;
                                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
#endif
                        } else {
                                /* We should not reach here. */
                                ASSERT_WARNING(0);
                                return POWER_RAIL_DISABLED;
                        }

                } else {
                        return POWER_RAIL_DISABLED;
                }
        }

        return r_state;
}

HW_PMU_ERROR_CODE hw_pmu_vsys_set_voltage(HW_PMU_VSYS_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (voltage) {
        case HW_PMU_VSYS_VOLTAGE_4V2:
        case HW_PMU_VSYS_VOLTAGE_4V4:
        case HW_PMU_VSYS_VOLTAGE_4V6:
        case HW_PMU_VSYS_VOLTAGE_4V8:
                vsys_set_voltage_level(voltage);
                break;
        default:
                res =  HW_PMU_ERROR_INVALID_ARGS;
        }

        return res;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vsys_active_config(HW_PMU_VSYS_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_VSYS_RAIL_CONFIG));

        if (is_vsys_powered()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = vsys_get_voltage_level();
                rail_config->current = HW_PMU_VSYS_MAX_LOAD_1000;
                rail_config->src_type = HW_PMU_SRC_TYPE_AUTO;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vsys_onwakeup_config(HW_PMU_VSYS_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_vsys_active_config(rail_config);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_vsys_onsleep_config(HW_PMU_VSYS_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_vsys_active_config(rail_config);
}

HW_PMU_ERROR_CODE hw_pmu_3v0_set_voltage(HW_PMU_3V0_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (voltage) {
        case HW_PMU_3V0_VOLTAGE_3V0:
        case HW_PMU_3V0_VOLTAGE_3V3:
                __3v0_set_active_voltage_level(voltage);
                break;
        case HW_PMU_3V0_VOLTAGE_SLEEP_3V0:
        case HW_PMU_3V0_VOLTAGE_SLEEP_3V3:
                __3v0_set_sleep_voltage_level(voltage);
                break;
        default:
                res =  HW_PMU_ERROR_INVALID_ARGS;
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_3v0_onwakeup_enable(HW_PMU_3V0_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        /* Applicable only in sleep mode. */
        case HW_PMU_3V0_MAX_LOAD_1:
                res = HW_PMU_ERROR_INVALID_ARGS;
                break;
        case HW_PMU_3V0_MAX_LOAD_10:
                res = check_3v0_dependencies_active(HW_PMU_CHK_3V0_LDO_RET_ACTIVE_MSK);
                break;
        case HW_PMU_3V0_MAX_LOAD_150:
                res = check_3v0_dependencies_active(HW_PMU_CHK_3V0_LDO_MSK);
                break;
        case HW_PMU_3V0_MAX_LOAD_160:
                res = check_3v0_dependencies_active(HW_PMU_CHK_3V0_LDO_MSK |
                                                    HW_PMU_CHK_3V0_LDO_RET_ACTIVE_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                /* Applicable only in sleep mode. */
                case HW_PMU_3V0_MAX_LOAD_1:
                        res = HW_PMU_ERROR_INVALID_ARGS;
                        break;
                case HW_PMU_3V0_MAX_LOAD_10:
                        __3v0_ldo_ret_active_enable();
                        /* Disable other power sources. */
                        __3v0_ldo_active_disable();
                        __3v0_clamp_disable();
                        break;
                case HW_PMU_3V0_MAX_LOAD_150:
                        __3v0_ldo_active_enable();
                        /* Disable other power sources. */
                        __3v0_ldo_ret_active_disable();
                        __3v0_clamp_disable();
                        break;
                case HW_PMU_3V0_MAX_LOAD_160:
                        __3v0_ldo_active_enable();
                        __3v0_ldo_ret_active_enable();
                        /* Disable other power sources. */
                        __3v0_clamp_disable();
                        break;
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_3v0_onwakeup_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;

        res = check_3v0_dependants_active(HW_PMU_CHK_3V0_BANDGAP_MSK |
                                          HW_PMU_CHK_3V0_GPIO_MSK    |
                                          HW_PMU_CHK_3V0_POR_MSK     |
                                          HW_PMU_CHK_3V0_USB_MSK     |
                                          HW_PMU_CHK_3V0_OTP_MSK     |
                                          HW_PMU_CHK_3V0_RCHS_MSK    |
                                          HW_PMU_CHK_3V0_RCX_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        __3v0_ldo_active_disable();
        __3v0_ldo_ret_active_disable();
        __3v0_clamp_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_3v0_onsleep_enable(HW_PMU_3V0_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_3V0_MAX_LOAD_1:
                res = check_3v0_dependencies_sleep(HW_PMU_CHK_3V0_CLAMP_MSK);
                break;
        case HW_PMU_3V0_MAX_LOAD_10:
                res = check_3v0_dependencies_sleep(HW_PMU_CHK_3V0_LDO_RET_SLEEP_MSK);
                break;
        case HW_PMU_3V0_MAX_LOAD_150:
                res = check_3v0_dependencies_sleep(HW_PMU_CHK_3V0_LDO_MSK);
                break;
        case HW_PMU_3V0_MAX_LOAD_160:
                res = check_3v0_dependencies_sleep(HW_PMU_CHK_3V0_LDO_MSK |
                                                   HW_PMU_CHK_3V0_LDO_RET_SLEEP_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_3V0_MAX_LOAD_1:
                        __3v0_clamp_enable();
                        /* Disable other power sources. */
                        __3v0_ldo_ret_sleep_disable();
                        __3v0_ldo_sleep_disable();
                        break;
                case HW_PMU_3V0_MAX_LOAD_10:
                        __3v0_ldo_ret_sleep_enable();
                        /* Disable other power sources. */
                        __3v0_clamp_disable();
                        __3v0_ldo_sleep_disable();
                        break;
                case HW_PMU_3V0_MAX_LOAD_150:
                        __3v0_ldo_sleep_enable();
                        /* Disable other power sources. */
                        __3v0_clamp_disable();
                        __3v0_ldo_ret_sleep_disable();
                        break;
                case HW_PMU_3V0_MAX_LOAD_160:
                        __3v0_ldo_sleep_enable();
                        __3v0_ldo_ret_sleep_enable();
                        /* Disable other power sources. */
                        __3v0_clamp_disable();
                        break;
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_3v0_onsleep_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;

        res = check_3v0_dependants_sleep(HW_PMU_CHK_3V0_BANDGAP_MSK     |
                                         HW_PMU_CHK_3V0_GPIO_MSK        |
                                         HW_PMU_CHK_3V0_POR_MSK         |
                                         HW_PMU_CHK_3V0_VAD_MSK         |
                                         HW_PMU_CHK_3V0_LDO_START_MSK   |
                                         HW_PMU_CHK_3V0_RCX_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        __3v0_clamp_disable();
        __3v0_ldo_ret_sleep_disable();
        __3v0_ldo_sleep_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_3v0_active_config(HW_PMU_3V0_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_3V0_RAIL_CONFIG));

        if (is_3v0_ldo_active_enabled() && is_3v0_ldo_ret_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __3v0_get_active_voltage_level();
                rail_config->current = HW_PMU_3V0_MAX_LOAD_160;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else  if (is_3v0_ldo_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __3v0_get_active_voltage_level();
                rail_config->current = HW_PMU_3V0_MAX_LOAD_150;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else if (is_3v0_ldo_ret_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __3v0_get_active_voltage_level();
                rail_config->current = HW_PMU_3V0_MAX_LOAD_10;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_3v0_onwakeup_config(HW_PMU_3V0_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_3v0_active_config(rail_config);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_3v0_onsleep_config(HW_PMU_3V0_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_3V0_RAIL_CONFIG));

        /* In case all sources are enabled LDO is dominant. */
        if (is_3v0_ldo_sleep_enabled() && is_3v0_ldo_ret_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __3v0_get_sleep_voltage_level();
                rail_config->current = HW_PMU_3V0_MAX_LOAD_160;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else if (is_3v0_ldo_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __3v0_get_sleep_voltage_level();
                rail_config->current = HW_PMU_3V0_MAX_LOAD_150;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else if (is_3v0_ldo_ret_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __3v0_get_sleep_voltage_level();
                rail_config->current = HW_PMU_3V0_MAX_LOAD_10;
                rail_config->src_type = HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE;
        } else if (is_3v0_clamp_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __3v0_get_sleep_voltage_level();
                rail_config->current = HW_PMU_3V0_MAX_LOAD_1;
                rail_config->src_type = HW_PMU_SRC_TYPE_CLAMP;
        }

        return r_state;
}

HW_PMU_ERROR_CODE hw_pmu_1v8_set_voltage(HW_PMU_1V8_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_1V8_VOLTAGE v18_voltage_level = __1v8_get_voltage_level();
#endif

        switch (voltage) {
        case HW_PMU_1V8_VOLTAGE_1V2:
        case HW_PMU_1V8_VOLTAGE_1V8:
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
                if (voltage > v18_voltage_level) {
                        if (is_bod_on_1v8_active()) {
                                return HW_PMU_ERROR_BOD_IS_ACTIVE;
                        }
                }
#endif
                /* The sequence of these steps (applying the voltage level with the corresponding
                 * trim setting) does not matter.
                 */
                __1v8_set_voltage_level(voltage);
                hw_pmu_1v8_set_trim(voltage);
                break;
        default:
                res =  HW_PMU_ERROR_INVALID_ARGS;
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_enable(HW_PMU_1V8_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_1V8_MAX_LOAD_100:
                res = check_1v8_dependencies_active(HW_PMU_CHK_1V8_SIMO_DCDC_ACTIVE_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V8_MAX_LOAD_100:
                        __1v8_dcdc_active_enable();
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1

        if (is_bod_on_1v8_active()) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif

        __1v8_dcdc_active_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onsleep_enable(HW_PMU_1V8_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_1V8_MAX_LOAD_100:
                res = check_1v8_dependencies_sleep(HW_PMU_CHK_1V8_SIMO_DCDC_SLEEP_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V8_MAX_LOAD_100:
                        __1v8_dcdc_sleep_enable();
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v8_onsleep_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1

        if (is_bod_on_1v8_active()) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif

        __1v8_dcdc_sleep_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_active_config(HW_PMU_1V8_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V8_RAIL_CONFIG));

        if (is_1v8_dcdc_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __1v8_get_voltage_level();
                rail_config->current = HW_PMU_1V8_MAX_LOAD_100;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onwakeup_config(HW_PMU_1V8_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_1v8_active_config(rail_config);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onsleep_config(HW_PMU_1V8_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V8_RAIL_CONFIG));

        if (is_1v8_dcdc_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __1v8_get_voltage_level();
                rail_config->current = HW_PMU_1V8_MAX_LOAD_100;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        }

        return r_state;
}

HW_PMU_ERROR_CODE hw_pmu_1v8p_onwakeup_enable(HW_PMU_1V8P_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_1V8P_MAX_LOAD_100:
                res = check_1v8p_dependencies_active(HW_PMU_CHK_1V8P_SIMO_DCDC_ACTIVE_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V8P_MAX_LOAD_100:
                        __1v8p_dcdc_active_enable();
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v8p_onwakeup_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (is_bod_on_1v8p_active()) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_1v8p_dependants_active(HW_PMU_CHK_1V8P_1V8F_MSK     |
                                           HW_PMU_CHK_1V8P_GPIO_MSK     |
                                           HW_PMU_CHK_1V8P_SDADC_MSK    |
                                           HW_PMU_CHK_1V8P_QSPI_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        __1v8p_dcdc_active_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v8p_onsleep_enable(HW_PMU_1V8P_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_1V8P_MAX_LOAD_100:
                res = check_1v8p_dependencies_sleep(HW_PMU_CHK_1V8P_SIMO_DCDC_SLEEP_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V8P_MAX_LOAD_100:
                        __1v8p_dcdc_sleep_enable();
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v8p_onsleep_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (is_bod_on_1v8p_active()) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_1v8p_dependants_sleep(HW_PMU_CHK_1V8P_1V8F_MSK |
                                          HW_PMU_CHK_1V8P_GPIO_MSK |
                                          HW_PMU_CHK_1V8P_QSPI_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        __1v8p_dcdc_sleep_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_active_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V8P_RAIL_CONFIG));

        if (is_1v8p_dcdc_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = HW_PMU_1V8P_VOLTAGE_1V8;
                rail_config->current = HW_PMU_1V8P_MAX_LOAD_100;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_onwakeup_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_1v8p_active_config(rail_config);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_onsleep_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V8P_RAIL_CONFIG));

        if (is_1v8p_dcdc_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = HW_PMU_1V8P_VOLTAGE_1V8;
                rail_config->current = HW_PMU_1V8P_MAX_LOAD_100;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        }

        return r_state;
}

__RETAINED_CODE HW_PMU_ERROR_CODE hw_pmu_1v8f_onwakeup_enable(HW_PMU_1V8F_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_1V8F_MAX_LOAD_100:
                res = check_1v8f_dependencies_active(HW_PMU_CHK_1V8F_1V8P_ACTIVE_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V8F_MAX_LOAD_100:
                        __1v8f_sw_active_enable();
                }
        }

        return res;
}

__RETAINED_CODE HW_PMU_ERROR_CODE hw_pmu_1v8f_onwakeup_disable(void)
{

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (is_bod_on_1v8f_active()) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_1v8f_dependants_active(HW_PMU_CHK_1V8F_OQSPI_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        __1v8f_sw_active_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v8f_onsleep_enable(HW_PMU_1V8F_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_1V8F_MAX_LOAD_100:
                res = check_1v8f_dependencies_sleep(HW_PMU_CHK_1V8F_1V8P_SLEEP_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V8F_MAX_LOAD_100:
                        __1v8f_sw_sleep_enable();
                }
        }

        return res;
}

__RETAINED_CODE HW_PMU_ERROR_CODE hw_pmu_1v8f_onsleep_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (is_bod_on_1v8f_active()) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_1v8f_dependants_sleep(HW_PMU_CHK_1V8F_OQSPI_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif
        __1v8f_sw_sleep_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8f_active_config(HW_PMU_1V8F_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V8F_RAIL_CONFIG));

        if (is_1v8f_sw_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = HW_PMU_1V8F_VOLTAGE_1V8;
                rail_config->current = HW_PMU_1V8F_MAX_LOAD_100;
                rail_config->src_type = HW_PMU_SRC_TYPE_1V8P;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8f_onwakeup_config(HW_PMU_1V8F_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_1v8f_active_config(rail_config);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8f_onsleep_config(HW_PMU_1V8F_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V8F_RAIL_CONFIG));

        if (is_1v8f_sw_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = HW_PMU_1V8F_VOLTAGE_1V8;
                rail_config->current = HW_PMU_1V8F_MAX_LOAD_100;
                rail_config->src_type = HW_PMU_SRC_TYPE_1V8P;
        }

        return r_state;
}

HW_PMU_ERROR_CODE hw_pmu_1v4_set_voltage(HW_PMU_1V4_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_1V4_VOLTAGE v14_voltage_level = __1v4_get_voltage_level();
#endif


        switch (voltage) {
        case HW_PMU_1V4_VOLTAGE_1V2:
        case HW_PMU_1V4_VOLTAGE_1V3:
        case HW_PMU_1V4_VOLTAGE_1V4:
        case HW_PMU_1V4_VOLTAGE_1V5:
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
                if (voltage > v14_voltage_level) {
                        if (is_bod_on_1v4_active()) {
                                return HW_PMU_ERROR_BOD_IS_ACTIVE;
                        }
                }
#endif

                __1v4_set_voltage_level(voltage);
                break;
        default:
                res =  HW_PMU_ERROR_INVALID_ARGS;
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v4_onwakeup_enable(HW_PMU_1V4_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_1V4_MAX_LOAD_20:
                res = check_1v4_dependencies_active(HW_PMU_CHK_1V4_SIMO_DCDC_ACTIVE_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V4_MAX_LOAD_20:
                        __1v4_dcdc_active_enable();
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v4_onwakeup_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (is_bod_on_1v4_active()) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_1v4_dependants_active(HW_PMU_CHK_1V4_XTAL32M_MSK |
                                          HW_PMU_CHK_1V4_PLL_MSK     |
                                          HW_PMU_CHK_1V4_GPADC_MSK   |
                                          HW_PMU_CHK_1V4_1V4RF_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        __1v4_dcdc_active_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v4_onsleep_enable(HW_PMU_1V4_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_1V4_MAX_LOAD_20:
                res = check_1v4_dependencies_sleep(HW_PMU_CHK_1V4_SIMO_DCDC_SLEEP_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V4_MAX_LOAD_20:
                        __1v4_dcdc_sleep_enable();
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v4_onsleep_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1

        if (is_bod_on_1v4_active()) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }
#endif

        __1v4_dcdc_sleep_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v4_active_config(HW_PMU_1V4_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V4_RAIL_CONFIG));

        if (is_1v4_dcdc_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __1v4_get_voltage_level();
                rail_config->current = HW_PMU_1V4_MAX_LOAD_20;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v4_onwakeup_config(HW_PMU_1V4_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_1v4_active_config(rail_config);
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v4_onsleep_config(HW_PMU_1V4_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V4_RAIL_CONFIG));

        if (is_1v4_dcdc_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __1v4_get_voltage_level();
                rail_config->current = HW_PMU_1V4_MAX_LOAD_20;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        }

        return r_state;
}

__RETAINED_HOT_CODE HW_PMU_ERROR_CODE hw_pmu_1v2_set_voltage(HW_PMU_1V2_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;
        HW_PMU_1V2_VOLTAGE v12_active_voltage_level = __1v2_get_active_voltage_level();

        switch (voltage) {
        case HW_PMU_1V2_VOLTAGE_0V75:
        case HW_PMU_1V2_VOLTAGE_0V90:
#if HW_PMU_SANITY_CHECKS_ENABLE == 1

                res = check_1v2_dependants_active(HW_PMU_CHK_1V2_HIGH_SPEED_CLK_MSK);
                if (res != HW_PMU_ERROR_NOERROR) {
                        return res;
                }
#endif
                /* Suppress "No break at the end of case" warning */

        case HW_PMU_1V2_VOLTAGE_1V20:
                if (voltage > v12_active_voltage_level) {
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
                        if (is_bod_on_1v2_active()) {
                                return HW_PMU_ERROR_BOD_IS_ACTIVE;
                        }
#endif
                        /* Increasing 1V2 voltage level: Sequence is important, by first setting the higher level
                         * before applying the trim setting.
                         */
                        __1v2_set_active_voltage_level(voltage);
                        hw_pmu_1v2_set_trim(voltage);
                } else {
                        /* Decreasing 1V2 voltage level: Sequence is important, by first setting the trim level
                         * before switching to the lower level.
                         */
                        hw_pmu_1v2_set_trim(voltage);
                        __1v2_set_active_voltage_level(voltage);
                }

                break;
        case HW_PMU_1V2_VOLTAGE_SLEEP_0V75:
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
                res = check_1v2_dependants_sleep(HW_PMU_CHK_1V2_UFAST_WAKEUP_UP_0V75_MSK);
                if (res != HW_PMU_ERROR_NOERROR) {
                        return res;
                }
#endif
                /* Suppress "No break at the end of case" warning */

        case HW_PMU_1V2_VOLTAGE_SLEEP_0V90:
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
                res = check_1v2_dependants_sleep(HW_PMU_CHK_1V2_UFAST_WAKEUP_UP_0V90_MSK);
                if (res != HW_PMU_ERROR_NOERROR) {
                        return res;
                }
#endif
                /* Suppress "No break at the end of case" warning */

        case HW_PMU_1V2_VOLTAGE_SLEEP_1V20:
                __1v2_set_sleep_voltage_level(voltage);
                break;
        case HW_PMU_1V2_VOLTAGE_HIBERNATION:
                res =  HW_PMU_ERROR_INVALID_ARGS;
                break;
        default:
                res =  HW_PMU_ERROR_INVALID_ARGS;
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v2_onwakeup_enable(HW_PMU_1V2_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_1V2_MAX_LOAD_1:
                /* Applicable only in sleep (hibernation) mode. */
                res = HW_PMU_ERROR_INVALID_ARGS;
                break;
        case HW_PMU_1V2_MAX_LOAD_150:
                res = check_1v2_dependencies_active(HW_PMU_CHK_1V2_SIMO_DCDC_ACTIVE_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V2_MAX_LOAD_1:
                        /* Applicable only in sleep (hibernation) mode. */
                        res = HW_PMU_ERROR_INVALID_ARGS;
                        break;
                case HW_PMU_1V2_MAX_LOAD_150:
                        __1v2_dcdc_active_enable();
                        /* Disable other power sources. */
                        __1v2_clamp_disable();
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v2_onwakeup_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (is_bod_on_1v2_active()) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_1v2_dependants_active(HW_PMU_CHK_1V2_RCLP_MSK    |
                                          HW_PMU_CHK_1V2_XTAL32K_MSK |
                                          HW_PMU_CHK_1V2_USB_MSK     |
                                          HW_PMU_CHK_1V2_OTP_MSK     |
                                          HW_PMU_CHK_1V2_HIGH_SPEED_CLK_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        __1v2_dcdc_active_disable();

        return HW_PMU_ERROR_NOERROR;
}

HW_PMU_ERROR_CODE hw_pmu_1v2_onsleep_enable(HW_PMU_1V2_MAX_LOAD max_load)
{
        HW_PMU_ERROR_CODE res = HW_PMU_ERROR_NOERROR;

        switch (max_load) {
        case HW_PMU_1V2_MAX_LOAD_1:
                res = check_1v2_dependencies_sleep(HW_PMU_CHK_1V2_CLAMP_MSK);
                break;
        case HW_PMU_1V2_MAX_LOAD_150:
                res = check_1v2_dependencies_sleep(HW_PMU_CHK_1V2_SIMO_DCDC_SLEEP_MSK);
                break;
        default:
                res = HW_PMU_ERROR_INVALID_ARGS;
        }

        if (res == HW_PMU_ERROR_NOERROR) {
                switch (max_load) {
                case HW_PMU_1V2_MAX_LOAD_1:
                        __1v2_clamp_enable();
                        /* Disable other power sources. */
                        __1v2_dcdc_sleep_disable();
                        break;
                case HW_PMU_1V2_MAX_LOAD_150:
                        __1v2_dcdc_sleep_enable();
                        /* Disable other power sources. */
                        __1v2_clamp_disable();
                }
        }

        return res;
}

HW_PMU_ERROR_CODE hw_pmu_1v2_onsleep_disable(void)
{
#if HW_PMU_SANITY_CHECKS_ENABLE == 1
        HW_PMU_ERROR_CODE res;
        if (is_bod_on_1v2_active()) {
                return HW_PMU_ERROR_BOD_IS_ACTIVE;
        }

        res = check_1v2_dependants_sleep(HW_PMU_CHK_1V2_RCLP_MSK    |
                                         HW_PMU_CHK_1V2_XTAL32K_MSK |
                                         HW_PMU_CHK_1V2_WAKEUP_UP_MSK);

        if (res != HW_PMU_ERROR_NOERROR) {
                return res;
        }
#endif

        __1v2_dcdc_sleep_disable();

        return HW_PMU_ERROR_NOERROR;
}

__RETAINED_HOT_CODE HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_active_config(HW_PMU_1V2_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V2_RAIL_CONFIG));

        if (is_1v2_dcdc_active_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __1v2_get_active_voltage_level();
                rail_config->current = HW_PMU_1V2_MAX_LOAD_150;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        }

        return r_state;
}

HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_onwakeup_config(HW_PMU_1V2_RAIL_CONFIG *rail_config)
{
        return hw_pmu_get_1v2_active_config(rail_config);
}

__RETAINED_HOT_CODE HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_onsleep_config(HW_PMU_1V2_RAIL_CONFIG *rail_config)
{
        HW_PMU_POWER_RAIL_STATE r_state = POWER_RAIL_DISABLED;

        memset(rail_config, UINT8_MAX, sizeof(HW_PMU_1V2_RAIL_CONFIG));

        /* In case both sources are enabled DCDC is dominant. */
        if (is_1v2_dcdc_sleep_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = __1v2_get_sleep_voltage_level();
                rail_config->current = HW_PMU_1V2_MAX_LOAD_150;
                rail_config->src_type = HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY;
        } else if (is_1v2_clamp_enabled()) {
                r_state = POWER_RAIL_ENABLED;
                rail_config->voltage = HW_PMU_1V2_VOLTAGE_HIBERNATION;
                rail_config->current = HW_PMU_1V2_MAX_LOAD_1;
                rail_config->src_type = HW_PMU_SRC_TYPE_CLAMP;
        }
        return r_state;

}


HW_PMU_ERROR_CODE hw_pmu_store_trim_values(void)
{
        uint32_t *val = NULL;
        uint8_t size = 0;
        HW_PMU_ERROR_CODE ret = HW_PMU_ERROR_NOERROR;

        sys_tcs_get_custom_values(SYS_TCS_GROUP_BUCK_TRIM, &val, &size);
        if (size == 1 && val) { /* TCS entry found, store its value. */
                hw_pmu_tcs_buck_trim_values.__tcs_buck_trim = *val;
        } else { /* TCS entry not found, store the corresponding register fields. */
                uint32_t trim_val;

                trim_val = RAW_GETF(0x500000D0, 0x7800UL);
                hw_pmu_tcs_buck_trim_values.__1v2_trim_0v75 = trim_val;
                hw_pmu_tcs_buck_trim_values.__1v2_trim_0v90 = trim_val;
                hw_pmu_tcs_buck_trim_values.__1v2_trim_1v20 = trim_val;

                trim_val = RAW_GETF(0x500000D0, 0x780000UL);
                hw_pmu_tcs_buck_trim_values.__1v8_trim_1v20 = trim_val;
                hw_pmu_tcs_buck_trim_values.__1v8_trim_1v80 = trim_val;
        }

        return ret;
}

#endif /* dg_configUSE_HW_PMU */

