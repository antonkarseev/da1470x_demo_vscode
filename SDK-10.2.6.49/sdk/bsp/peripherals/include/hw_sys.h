/**
\addtogroup PLA_DRI_PER_OTHER
\{
\addtogroup HW_SYS System Hardware LLD API
\{
\brief System Driver
*/

/**
 ****************************************************************************************
 *
 * @file hw_sys.h
 *
 * @brief System LLD API.
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_SYS_H_
#define HW_SYS_H_

#include "sdk_defs.h"
#include "hw_pd.h"
#include "hw_memctrl.h"

typedef enum {
        HW_SYS_REMAP_ADDRESS_0_TO_ROM = 0,
        HW_SYS_REMAP_ADDRESS_0_TO_OTP = 1,
        HW_SYS_REMAP_ADDRESS_0_TO_OQSPI_FLASH = 2,
        HW_SYS_REMAP_ADDRESS_0_TO_RAM = 3,
        HW_SYS_REMAP_ADDRESS_0_TO_SYSRAM3 = 5,
} HW_SYS_REMAP_ADDRESS_0;

__STATIC_INLINE void hw_sys_set_memory_remapping(HW_SYS_REMAP_ADDRESS_0 value)
{
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0, value);
        GLOBAL_INT_RESTORE();
}

__STATIC_INLINE HW_SYS_REMAP_ADDRESS_0 hw_sys_get_memory_remapping(void)
{
        return (HW_SYS_REMAP_ADDRESS_0)REG_GETF(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0);
}

/**
 * \brief Enable iCache retainability during sleep.
 *
 */
__STATIC_INLINE void hw_sys_set_cache_retained(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, RETAIN_CACHE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable iCache retainability during sleep.
 *
 */
__STATIC_INLINE void hw_sys_disable_cache_retained(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, RETAIN_CACHE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Get iCache retainability setting.
 *
 * \return true, if the iCache is configured to be retained, else false.
 *
 */
__STATIC_INLINE bool hw_sys_is_cache_retained(void)
{
        return REG_GETF(CRG_TOP, PMU_CTRL_REG, RETAIN_CACHE);
}

/**
 * \brief Enable dCache retainability during sleep.
 *
 */
__STATIC_INLINE void hw_sys_enable_dcache_retained(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, RETAIN_DCACHE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable dCache retainability during sleep.
 *
 */
__STATIC_INLINE void hw_sys_disable_dcache_retained(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, RETAIN_DCACHE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Get dCache retainability setting.
 *
 * \return true, if the dCache is configured to be retained, else false.
 *
 */
__STATIC_FORCEINLINE bool hw_sys_is_dcache_retained(void)
{
        return REG_GETF(CRG_TOP, PMU_CTRL_REG, RETAIN_DCACHE);
}


/**
 * \brief Setup the Retention Memory configuration.
 *
 */
__STATIC_INLINE void hw_sys_setup_retmem(void)
{
#if dg_configMEM_RETENTION_MODE
        GLOBAL_INT_DISABLE();

        CRG_TOP->RAM_PWR_CTRL_REG = dg_configMEM_RETENTION_MODE;

        GLOBAL_INT_RESTORE();
#endif /* dg_configMEM_RETENTION_MODE */
}

/**
 * \brief Disable memory retention.
 *
 */
__STATIC_INLINE void hw_sys_no_retmem(void)
{
        GLOBAL_INT_DISABLE();
        CRG_TOP->PMU_CTRL_REG &= ~(REG_MSK(CRG_TOP, PMU_CTRL_REG, RETAIN_CACHE) |
                                   REG_MSK(CRG_TOP, PMU_CTRL_REG, RETAIN_RGP_RAM) |
                                   REG_MSK(CRG_TOP, PMU_CTRL_REG, RETAIN_GPU_CLUT) |
                                   REG_MSK(CRG_TOP, PMU_CTRL_REG, RETAIN_DCACHE));
        CRG_TOP->RAM_PWR_CTRL_REG = 0x5555555;
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Prepare RESET type tracking.
 */
__STATIC_FORCEINLINE void hw_sys_track_reset_type(void)
{
}

/**
 * \brief Enable the clock-less sleep mode.
 *
 */
__STATIC_INLINE void hw_sys_enable_clockless(void)
{
}

/**
 * \brief Disable the clock-less sleep mode.
 *
 */
__STATIC_INLINE void hw_sys_disable_clockless(void)
{
}

/**
 * \brief Activate the "Reset on wake-up" functionality.
 *
 */
__STATIC_INLINE void hw_sys_enable_reset_on_wup(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, RESET_ON_WAKEUP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set the preferred settings of a power domain.
 *
 * \param [in] pd power domain
 */
void hw_sys_set_preferred_values(HW_PD pd);

/**
 * \brief Enable the debugger.
 *
 */
__STATIC_FORCEINLINE void hw_sys_enable_debugger(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, DEBUGGER_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable the debugger.
 *
 */
__STATIC_FORCEINLINE void hw_sys_disable_debugger(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, SYS_CTRL_REG, DEBUGGER_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Check if the debugger is attached.
 *
 * \return true, if the debugger is attached, else false.
 *
 */
__STATIC_FORCEINLINE bool hw_sys_is_debugger_attached(void)
{
        return (REG_GETF(CRG_TOP, SYS_STAT_REG, DBG_IS_ACTIVE) != 0);
}

/**
 * \brief  Trigger a GPIO when ASSERT_WARNING() or ASSERT_ERROR() hits.
 *
 */
__RETAINED_CODE void hw_sys_assert_trigger_gpio(void);

/**
 * \brief Set POR-trigger minimum duration
 *
 * The function configures the Power-On-Reset (POR) timer, which
 * defines how long the nRST or the POR-configured GPIO pin should
 * be asserted in order to trigger a POR.
 *
 * \param [in] time time in 4096 * (RCLP@32KHz period) units. Should be
 *                  less than 128 (~16.2 sec).
 *
 * \note In case of triggering a POR from a GPIO, hw_gpio_configure_por_pin()
 * should also be used.
 *
 * \note Setting \b time to 0 disables POR generation completely (even for nRST).
 *
 * \note Default (reset) value is 0x18 (~3 sec).
 *
 * \note If a POR is successfully triggered, the POR timer is re-set to
 * its default value.
 *
 * \note In hibernation mode, the POR generation is disabled.
 *
 * \sa hw_gpio_configure_por_pin()
 *
 */
__STATIC_FORCEINLINE void hw_sys_set_por_timer(uint8_t time)
{
        ASSERT_WARNING(time <= (CRG_TOP_POR_TIMER_REG_POR_TIME_Msk >> CRG_TOP_POR_TIMER_REG_POR_TIME_Pos));
        REG_SETF(CRG_TOP, POR_TIMER_REG, POR_TIME, time);
}

/**
 * \brief Enables the COM power domain.
 *
 */
__RETAINED_CODE void hw_sys_pd_com_enable(void);

/**
 * \brief Disables the COM power domain. If it has not
 *        been enabled by any other modules, it will be disabled.
 */
__RETAINED_CODE void hw_sys_pd_com_disable(void);

/**
 * \brief Enables the PERIPH power domain.
 *
 */
__RETAINED_CODE void hw_sys_pd_periph_enable(void);

/**
 * \brief Disables the PERIPH power domain. If it has not
 *        been enabled by any other modules, it will be disabled.
 */
__RETAINED_CODE void hw_sys_pd_periph_disable(void);

/**
 * \brief Enables the AUDIO power domain.
 *
 */
__RETAINED_CODE void hw_sys_pd_audio_enable(void);

/**
 * \brief Disables the AUDIO power domain. If it has not
 *        been enabled by any other modules, it will be disabled.
 */
__RETAINED_CODE void hw_sys_pd_audio_disable(void);

/**
 * \brief Enables the GPU power domain.
 *
 */
__RETAINED_CODE void hw_sys_pd_gpu_enable(void);

/**
 * \brief Disables the GPU power domain. If it has not
 *        been enabled by any other modules, it will be disabled.
 */
__RETAINED_CODE void hw_sys_pd_gpu_disable(void);

/**
 * \brief Checks whether there is a register entry in CS for XTAL32M_TRIM_REG
 * If the entry exists, the function applies the default values for XTAL32M_START_REG.CUR_SET
 * and XTAL32M_START_REG.TRIM
 * If not, the function runs xtal32m_configure_cur_set in order to determine the
 * optimum value for XTAL32M_TRIM_REG.CUR_SET (reset values are used for XTAL32M_START_REG.CUR_SET
 * and XTAL32M_START_REG.TRIM)
 */
void hw_sys_apply_default_values(void);

/**
 * \brief enables hibernation sleep mode
 *
 */
__STATIC_FORCEINLINE void hw_sys_enable_hibernation_mode(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, WAKEUP_HIBERN_REG, HIBERNATION_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief enables read only protection in CMAC code and data
 *
 */
void hw_sys_enable_cmac_mem_protection(void);

/**
 * \brief enables "Read-only by any privilege level" and "execute_never" memory protection of IVT
 *
 */
void hw_sys_enable_ivt_mem_protection(void);



#endif /* HW_SYS_H_ */

/**
 * \}
 * \}
 */
