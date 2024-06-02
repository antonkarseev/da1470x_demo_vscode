/**
 ****************************************************************************************
 *
 * @file ad_pmu.c
 *
 * @brief PMU adapter API implementation
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifdef CONFIG_USE_BLE
#include "hw_bsr.h"
#endif

#if dg_configPMU_ADAPTER

#include "sdk_defs.h"
#include "ad_pmu.h"
#include "hw_bod.h"
#include "hw_clk.h"
#include "hw_sys_regs.h"
#ifdef OS_PRESENT
#include "sys_power_mgr.h"
#endif

#ifdef CONFIG_USE_BLE
extern __RETAINED_HOT_CODE void cmac_update_power_ctrl_reg_values(uint32_t onsleep_value);
#endif

/*
 * Use initial values that also make sense for baremetal apps,
 * where ad_pmu_init() cannot be called.
 */
__RETAINED_RW static ad_pmu_rail_config_t ad_pmu_1v2_rail_config = {
        .enabled_onwakeup = true,
        .enabled_onsleep = true,
        .rail_1v2 = {
                .current_onwakeup = HW_PMU_1V2_MAX_LOAD_150,
                .current_onsleep = HW_PMU_1V2_MAX_LOAD_150,
                .voltage_onwakeup = HW_PMU_1V2_VOLTAGE_1V20,
                .voltage_onsleep = HW_PMU_1V2_VOLTAGE_SLEEP_0V90
        }
};
__RETAINED static uint8_t ad_pmu_1v2_rail_1v2_acquire_count;

/* Forward declarations */
#ifdef OS_PRESENT

#define AD_PMU_MUTEX_CREATE()   do {                                            \
                                        OS_ASSERT(ad_pmu_mutex == NULL);        \
                                        OS_MUTEX_CREATE(ad_pmu_mutex);          \
                                        OS_ASSERT(ad_pmu_mutex);                \
                                } while (0)

#define AD_PMU_MUTEX_GET()      do {                                                    \
                                        OS_ASSERT(ad_pmu_mutex);                        \
                                        OS_MUTEX_GET(ad_pmu_mutex, OS_MUTEX_FOREVER);   \
                                } while (0)

#define AD_PMU_MUTEX_PUT()      OS_MUTEX_PUT(ad_pmu_mutex)

static void ad_pmu_init(void);
#endif

static void configure_power_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *cfg);
static void ad_pmu_1v2_rail_acquire_1v2_voltage(void);
static void ad_pmu_1v2_rail_release_1v2_voltage(void);
__STATIC_INLINE bool is_rchs_high_speed_enabled(void);
__STATIC_INLINE bool is_ufast_wakeup_mode_enabled(void);
__STATIC_INLINE void __1v8f_sw_sleep_disable(void);
__STATIC_INLINE HW_PMU_1V2_VOLTAGE __1v2_get_active_voltage_level(void);
static void ad_pmu_1v2_rail_set_1v2_voltage_onsleep(void);
static void __ad_pmu_1v2_rail_set_1v2_voltage_onwakeup_bod_safe(HW_PMU_1V2_VOLTAGE voltage);
static void ad_pmu_1v2_rail_set_1v2_voltage_onwakeup(HW_PMU_1V2_VOLTAGE voltage);

#ifdef OS_PRESENT

__RETAINED static OS_MUTEX ad_pmu_mutex;


/**
 * \brief Initialize adapter
 *
 */
static void ad_pmu_init(void)
{
        ad_pmu_rail_config_t ad_pmu_rail_config;

        /* Create Mutex. Call it once! */
        AD_PMU_MUTEX_CREATE();

#if (dg_configUSE_BOD == 1)
        /* Deactivate BOD to configure the rails. */
        hw_bod_deactivate();
#endif

        /*
         * VSYS rail configuration
         */

        ad_pmu_rail_config.enabled_onwakeup = true;
        ad_pmu_rail_config.enabled_onsleep = true;
        ad_pmu_rail_config.rail_vsys.voltage_common = HW_PMU_VSYS_VOLTAGE_4V8;

        configure_power_rail(PMU_RAIL_VSYS, &ad_pmu_rail_config);

        /*
         * VLED rail configuration
         */

        ad_pmu_rail_config.enabled_onwakeup = false;
        ad_pmu_rail_config.enabled_onsleep = false;
        ad_pmu_rail_config.rail_vled.current_onwakeup = HW_PMU_VLED_MAX_LOAD_150;
        ad_pmu_rail_config.rail_vled.current_onsleep = HW_PMU_VLED_MAX_LOAD_0_300;
        ad_pmu_rail_config.rail_vled.voltage_common = HW_PMU_VLED_VOLTAGE_4V5;

        configure_power_rail(PMU_RAIL_VLED, &ad_pmu_rail_config);

        /*
         * 3V0 rail configuration
         */

        ad_pmu_rail_config.enabled_onwakeup = true;
        ad_pmu_rail_config.enabled_onsleep = true;
        ad_pmu_rail_config.rail_3v0.current_onwakeup = HW_PMU_3V0_MAX_LOAD_160;
        ad_pmu_rail_config.rail_3v0.current_onsleep = HW_PMU_3V0_MAX_LOAD_10;
        ad_pmu_rail_config.rail_3v0.voltage_onwakeup = HW_PMU_3V0_VOLTAGE_3V0;
        ad_pmu_rail_config.rail_3v0.voltage_onsleep = HW_PMU_3V0_VOLTAGE_SLEEP_3V0;

        configure_power_rail(PMU_RAIL_3V0, &ad_pmu_rail_config);

        /*
         * 1V8 rail configuration
         */

        ad_pmu_rail_config.enabled_onwakeup = false;
        ad_pmu_rail_config.enabled_onsleep = false;
        ad_pmu_rail_config.rail_1v8.current_onsleep = HW_PMU_1V8_MAX_LOAD_100;
        ad_pmu_rail_config.rail_1v8.current_onwakeup = HW_PMU_1V8_MAX_LOAD_100;
        ad_pmu_rail_config.rail_1v8.voltage_common = HW_PMU_1V8_VOLTAGE_1V8;

        configure_power_rail(PMU_RAIL_1V8, &ad_pmu_rail_config);

        /*
         * 1V8P rail configuration
         */

        ad_pmu_rail_config.enabled_onwakeup = true;
        ad_pmu_rail_config.enabled_onsleep = true;
        ad_pmu_rail_config.rail_1v8p.current_onwakeup = HW_PMU_1V8P_MAX_LOAD_100;
        ad_pmu_rail_config.rail_1v8p.current_onsleep = HW_PMU_1V8P_MAX_LOAD_100;

        configure_power_rail(PMU_RAIL_1V8P, &ad_pmu_rail_config);

        /*
         * 1V8F rail configuration
         */

        ad_pmu_rail_config.enabled_onwakeup = true;
        ad_pmu_rail_config.enabled_onsleep = true;
        ad_pmu_rail_config.rail_1v8f.current_onwakeup = HW_PMU_1V8F_MAX_LOAD_100;
        ad_pmu_rail_config.rail_1v8f.current_onsleep = HW_PMU_1V8F_MAX_LOAD_100;

        configure_power_rail(PMU_RAIL_1V8F, &ad_pmu_rail_config);

        /*
         * 1V4 rail configuration
         */

        ad_pmu_rail_config.enabled_onwakeup = true;
        ad_pmu_rail_config.enabled_onsleep = false;
        ad_pmu_rail_config.rail_1v4.current_onwakeup = HW_PMU_1V4_MAX_LOAD_20;
        ad_pmu_rail_config.rail_1v4.current_onsleep = HW_PMU_1V4_MAX_LOAD_20;
        ad_pmu_rail_config.rail_1v4.voltage_common = HW_PMU_1V4_VOLTAGE_1V4;

        configure_power_rail(PMU_RAIL_1V4, &ad_pmu_rail_config);

        /*
         * 1V2 rail configuration
         */

        ad_pmu_rail_config.enabled_onwakeup = true;
        ad_pmu_rail_config.enabled_onsleep = true;
        ad_pmu_rail_config.rail_1v2.current_onwakeup = HW_PMU_1V2_MAX_LOAD_150;
        ad_pmu_rail_config.rail_1v2.current_onsleep = HW_PMU_1V2_MAX_LOAD_150;
        ad_pmu_rail_config.rail_1v2.voltage_onwakeup = ((hw_clk_get_rchs_mode() > RCHS_32)      ||
                                                        hw_clk_check_pll_status()               ||
                                                        hw_clk_check_pll_usb_status()
                                                        ) ?
                                                        HW_PMU_1V2_VOLTAGE_1V20 : HW_PMU_1V2_VOLTAGE_0V90;
        ad_pmu_rail_config.rail_1v2.voltage_onsleep = HW_PMU_1V2_VOLTAGE_SLEEP_0V90;

        configure_power_rail(PMU_RAIL_1V2, &ad_pmu_rail_config);


#if (dg_configUSE_BOD == 1)
        /* Rails have been configured. Run again BOD  */
        hw_bod_configure();
#endif

}

ADAPTER_INIT(ad_pmu_adapter, ad_pmu_init);

#endif /* OS_PRESENT */

__STATIC_INLINE bool is_rchs_high_speed_enabled(void)
{
        return (hw_clk_get_rchs_mode() != RCHS_32);
}

__STATIC_INLINE bool is_ufast_wakeup_mode_enabled(void)
{
        return (REG_GETF(CRG_TOP, PMU_SLEEP_REG, ULTRA_FAST_WAKEUP)) ;
}

__STATIC_INLINE void __1v8f_sw_sleep_disable(void)
{
        REG_CLR_BIT(CRG_TOP, POWER_CTRL_REG, SW_V18F_SLEEP_ON);
}

__STATIC_INLINE HW_PMU_1V2_VOLTAGE __1v2_get_active_voltage_level(void)
{
        return REG_GETF(CRG_TOP, POWER_LVL_REG, V12_LEVEL);
}

static void configure_power_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *cfg)
{
        HW_PMU_ERROR_CODE error_code =  HW_PMU_ERROR_NOERROR;

        if (cfg->enabled_onwakeup) {
                /* Set rail voltage in active mode. */
                switch (rail) {
                case PMU_RAIL_VSYS:
                        error_code = hw_pmu_vsys_set_voltage(cfg->rail_vsys.voltage_common);
                        break;
                case PMU_RAIL_VLED:
                        error_code = hw_pmu_vled_set_voltage(cfg->rail_vled.voltage_common);
                        break;
                case PMU_RAIL_3V0:
                        error_code = hw_pmu_3v0_set_voltage(cfg->rail_3v0.voltage_onwakeup);
                        break;
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_set_voltage(cfg->rail_1v8.voltage_common);
                        break;
                case PMU_RAIL_1V8P:
                case PMU_RAIL_1V8F:
                        /* Nothing to do. */
                        break;
                case PMU_RAIL_1V4:
                        error_code = hw_pmu_1v4_set_voltage(cfg->rail_1v4.voltage_common);
                        break;
                case PMU_RAIL_1V2:
                        error_code = hw_pmu_1v2_set_voltage(cfg->rail_1v2.voltage_onwakeup);
                        /* Wait 20us for bandgap to ramp up reference */
                        hw_clk_delay_usec(20);
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                /* Power up rail in active mode. */
                switch (rail) {
                case PMU_RAIL_VSYS:
                        /* Nothing to do. */
                        break;
                case PMU_RAIL_VLED:
                        error_code = hw_pmu_vled_onwakeup_enable(cfg->rail_vled.current_onwakeup);
                        break;
                case PMU_RAIL_3V0:
                        error_code = hw_pmu_3v0_onwakeup_enable(cfg->rail_3v0.current_onwakeup);
                        break;
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_onwakeup_enable(cfg->rail_1v8.current_onwakeup);
                        break;
                case PMU_RAIL_1V8P:
                        error_code = hw_pmu_1v8p_onwakeup_enable(cfg->rail_1v8p.current_onwakeup);
                        break;
                case PMU_RAIL_1V8F:
                        error_code = hw_pmu_1v8f_onwakeup_enable(cfg->rail_1v8f.current_onwakeup);
                        break;
                case PMU_RAIL_1V4:
                        error_code = hw_pmu_1v4_onwakeup_enable(cfg->rail_1v4.current_onwakeup);
                        break;
                case PMU_RAIL_1V2:
                        error_code = hw_pmu_1v2_onwakeup_enable(cfg->rail_1v2.current_onwakeup);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

        } else {
                /* Power down rail in active mode. */
                switch (rail) {
                case PMU_RAIL_VSYS:
                        /* Nothing to do. */
                        break;
                case PMU_RAIL_VLED:
                        error_code = hw_pmu_vled_onwakeup_disable();
                        break;
                case PMU_RAIL_3V0:
                        error_code = hw_pmu_3v0_onwakeup_disable();
                        break;
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_onwakeup_disable();
                        break;
                case PMU_RAIL_1V8P:
                        error_code = hw_pmu_1v8p_onwakeup_disable();
                        break;
                case PMU_RAIL_1V8F:
                        error_code = hw_pmu_1v8f_onwakeup_disable();
                        break;
                case PMU_RAIL_1V4:
                        error_code = hw_pmu_1v4_onwakeup_disable();
                        break;
                case PMU_RAIL_1V2:
                        error_code = hw_pmu_1v2_onwakeup_disable();
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }

        if (cfg->enabled_onsleep) {
                /* Power up rail in sleep mode. */
                switch (rail) {
                case PMU_RAIL_VSYS:
                        /* Nothing to do. */
                        break;
                case PMU_RAIL_VLED:
                        error_code = hw_pmu_vled_onsleep_enable(cfg->rail_vled.current_onsleep);
                        break;
                case PMU_RAIL_3V0:
                        error_code = hw_pmu_3v0_onsleep_enable(cfg->rail_3v0.current_onsleep);
                        break;
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_onsleep_enable(cfg->rail_1v8.current_onsleep);
                        break;
                case PMU_RAIL_1V8P:
                        error_code = hw_pmu_1v8p_onsleep_enable(cfg->rail_1v8p.current_onsleep);
                        break;
                case PMU_RAIL_1V8F:
                        error_code = hw_pmu_1v8f_onsleep_enable(cfg->rail_1v8f.current_onsleep);
                        break;
                case PMU_RAIL_1V4:
                        error_code = hw_pmu_1v4_onsleep_enable(cfg->rail_1v4.current_onsleep);
                        break;
                case PMU_RAIL_1V2:
                        error_code = hw_pmu_1v2_onsleep_enable(cfg->rail_1v2.current_onsleep);
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

                /* Set rail voltage in sleep mode. */
                switch (rail) {
                case PMU_RAIL_VSYS:
                        error_code = hw_pmu_vsys_set_voltage(cfg->rail_vsys.voltage_common);
                        break;
                case PMU_RAIL_VLED:
                        error_code = hw_pmu_vled_set_voltage(cfg->rail_vled.voltage_common);
                        break;
                case PMU_RAIL_3V0:
                        error_code = hw_pmu_3v0_set_voltage(cfg->rail_3v0.voltage_onsleep);
                        break;
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_set_voltage(cfg->rail_1v8.voltage_common);
                        break;
                case PMU_RAIL_1V8P:
                case PMU_RAIL_1V8F:
                        /* Nothing to do. */
                        break;
                case PMU_RAIL_1V4:
                        error_code = hw_pmu_1v4_set_voltage(cfg->rail_1v4.voltage_common);
                        break;
                case PMU_RAIL_1V2:
                        error_code = hw_pmu_1v2_set_voltage(cfg->rail_1v2.voltage_onsleep);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);

        } else {
                /* Power down rail in sleep mode. */
                switch (rail) {
                case PMU_RAIL_VSYS:
                        /* Nothing to do. */
                        break;
                case PMU_RAIL_VLED:
                        error_code = hw_pmu_vled_onsleep_disable();
                        break;
                case PMU_RAIL_3V0:
                        error_code = hw_pmu_3v0_onsleep_disable();
                        break;
                case PMU_RAIL_1V8:
                        error_code = hw_pmu_1v8_onsleep_disable();
                        break;
                case PMU_RAIL_1V8P:
                        error_code = hw_pmu_1v8p_onsleep_disable();
                        break;
                case PMU_RAIL_1V8F:
                        error_code = hw_pmu_1v8f_onsleep_disable();
                        break;
                case PMU_RAIL_1V4:
                        error_code = hw_pmu_1v4_onsleep_disable();
                        break;
                case PMU_RAIL_1V2:
                        error_code = hw_pmu_1v2_onsleep_disable();
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
                }

                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }

        /* Update configuration */

        switch (rail) {
        case PMU_RAIL_VSYS:
        case PMU_RAIL_VLED:
        case PMU_RAIL_3V0:
        case PMU_RAIL_1V8:
        case PMU_RAIL_1V8P:
        case PMU_RAIL_1V8F:
        case PMU_RAIL_1V4:
                /* Nothing to do */
                break;
        case PMU_RAIL_1V2:
                ad_pmu_1v2_rail_config = *cfg;
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }
}

static void __ad_pmu_1v2_rail_set_1v2_voltage_onwakeup_bod_safe(HW_PMU_1V2_VOLTAGE voltage)
{
        HW_PMU_ERROR_CODE error_code;
        HW_PMU_1V2_VOLTAGE v12_active_voltage_level;

        v12_active_voltage_level = __1v2_get_active_voltage_level();

        if (voltage > v12_active_voltage_level) {

#if (dg_configUSE_BOD == 1)
                hw_bod_deactivate_channel(BOD_CHANNEL_VDD);
#endif
                error_code = hw_pmu_1v2_set_voltage(voltage);
#if (dg_configUSE_BOD == 1)
                /* Wait 20us for bandgap to ramp up reference */
                hw_clk_delay_usec(20);
                /* Rail has been configured. Enable BOD on VDD.  */
                hw_bod_activate_channel(BOD_CHANNEL_VDD);
#endif
        } else {
                error_code = hw_pmu_1v2_set_voltage(voltage);
        }

        ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
}


static void ad_pmu_1v2_rail_acquire_1v2_voltage(void)
{
        ad_pmu_1v2_rail_1v2_acquire_count++;

        if (ad_pmu_1v2_rail_1v2_acquire_count == 1) {
                /* This is the first time 1V2 rail is forced to 1.2V. */
                __ad_pmu_1v2_rail_set_1v2_voltage_onwakeup_bod_safe(HW_PMU_1V2_VOLTAGE_1V20);
        }
}

static void ad_pmu_1v2_rail_release_1v2_voltage(void)
{
        ad_pmu_1v2_rail_1v2_acquire_count--;

        if (ad_pmu_1v2_rail_1v2_acquire_count == 0) {
                /* This is the last time 1V2 rail is forced to 1.2V.
                 * Restore it's latest configured values set by ad_pmu_init() or
                 * ad_pmu_configure_rail().
                 */
                HW_PMU_ERROR_CODE error_code;
                error_code = hw_pmu_1v2_set_voltage(ad_pmu_1v2_rail_config.rail_1v2.voltage_onwakeup);
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }
}

__RETAINED_HOT_CODE static void ad_pmu_1v2_rail_set_1v2_voltage_onsleep(void)
{
        HW_PMU_ERROR_CODE error_code;

        if (is_ufast_wakeup_mode_enabled() && is_rchs_high_speed_enabled()) {
                error_code = hw_pmu_1v2_set_voltage(HW_PMU_1V2_VOLTAGE_SLEEP_1V20);
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        } else {
                error_code = hw_pmu_1v2_set_voltage(ad_pmu_1v2_rail_config.rail_1v2.voltage_onsleep);
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }
}

static void ad_pmu_1v2_rail_set_1v2_voltage_onwakeup(HW_PMU_1V2_VOLTAGE voltage)
{
        if (is_rchs_high_speed_enabled()) {
                /* Do nothing, the voltage level should have already been taken care
                 * by the caller.
                 */
                ASSERT_WARNING(__1v2_get_active_voltage_level() == HW_PMU_1V2_VOLTAGE_1V20);
        } else {
                __ad_pmu_1v2_rail_set_1v2_voltage_onwakeup_bod_safe(voltage);
        }
}

__RETAINED_HOT_CODE static void ad_pmu_power_off_xip_onsleep(void)
{
        if (dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8F && dg_configOQSPI_FLASH_POWER_OFF == 1) {
                /* Force disable of 1V8F rail, not taking into account any dependencies. */
                __1v8f_sw_sleep_disable();
        }
}

void ad_pmu_1v2_force_max_voltage_request(void)
{
        GLOBAL_INT_DISABLE();
#ifdef CONFIG_USE_BLE
        while (hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

        ad_pmu_1v2_rail_acquire_1v2_voltage();

#ifdef CONFIG_USE_BLE
        /* Communicate power rail levels to CMAC */
        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LVL_REG);

        hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */
        GLOBAL_INT_RESTORE();
}

void ad_pmu_1v2_force_max_voltage_release(void)
{
        GLOBAL_INT_DISABLE();
#ifdef CONFIG_USE_BLE
        while (hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

        ad_pmu_1v2_rail_release_1v2_voltage();

#ifdef CONFIG_USE_BLE
        /* Communicate power rail levels to CMAC */
        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LVL_REG);

        hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */
        GLOBAL_INT_RESTORE();
}

int ad_pmu_configure_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *config)
{
#ifdef OS_PRESENT
        /*  Block forever */
        AD_PMU_MUTEX_GET();
#else
        GLOBAL_INT_DISABLE();
#endif

#ifdef CONFIG_USE_BLE
        while (hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

#if (dg_configUSE_BOD == 1)
        /* Deactivate BOD to configure the rails. */
        hw_bod_deactivate();
#endif

        configure_power_rail(rail, config);

#ifdef CONFIG_USE_BLE
        /* Communicate power rail levels to CMAC */
        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LVL_REG);
#endif /* CONFIG_USE_BLE */

#if (dg_configUSE_BOD == 1)
        /* Rails have been configured. Run again BOD.  */
        hw_bod_configure();
#endif

#ifdef CONFIG_USE_BLE
        hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */


#ifdef OS_PRESENT
        AD_PMU_MUTEX_PUT();
#else
        GLOBAL_INT_RESTORE();
#endif
        return 0;
}

__RETAINED_HOT_CODE void ad_pmu_prepare_for_sleep(void)
{
        GLOBAL_INT_DISABLE();
#ifdef CONFIG_USE_BLE
        while (hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

        ad_pmu_1v2_rail_set_1v2_voltage_onwakeup(HW_PMU_1V2_VOLTAGE_0V90);
        ad_pmu_1v2_rail_set_1v2_voltage_onsleep();
        ad_pmu_power_off_xip_onsleep();

#ifdef CONFIG_USE_BLE
        /* Communicate power rail levels to CMAC */
        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LVL_REG);

        hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */
        GLOBAL_INT_RESTORE();
}

void ad_pmu_restore_for_wake_up(void)
{
        GLOBAL_INT_DISABLE();
#ifdef CONFIG_USE_BLE
        while (hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS) == false);
#endif /* CONFIG_USE_BLE */

        ad_pmu_1v2_rail_set_1v2_voltage_onwakeup(ad_pmu_1v2_rail_config.rail_1v2.voltage_onwakeup);

#ifdef CONFIG_USE_BLE
        /* Communicate power rail levels to CMAC */
        cmac_update_power_ctrl_reg_values(CRG_TOP->POWER_LVL_REG);

        hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_POWER_CTRL_POS);
#endif /* CONFIG_USE_BLE */
        GLOBAL_INT_RESTORE();
}

#endif /* dg_configPMU_ADAPTER */
