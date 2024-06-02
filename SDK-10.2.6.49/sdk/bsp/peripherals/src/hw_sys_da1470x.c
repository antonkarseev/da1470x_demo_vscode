/**
****************************************************************************************
*
* @file hw_sys_da1470x.c
*
* @brief System Driver
*
* Copyright (C) 2021-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#include <stdint.h>
#include "hw_clk.h"
#include "hw_gpio.h"
#include "hw_mpu.h"
#include "hw_pd.h"
#include "hw_sys.h"
#include "hw_sys_internal.h"
#include "sys_tcs.h"
#include "bsp_device_definitions_internal.h"

#ifdef CONFIG_USE_SNC
#include "snc.h"
#endif /* CONFIG_USE_SNC */

/**
 * \brief The number of register addresses to check if they are configured in CS
 *
 * \sa hw_sys_apply_default_values
 */
#define NUM_OF_REG_ADDR_IN_CS           1

#if dg_configUSE_HW_MPU
extern uint32_t __Vectors_Size;
#endif

# if MAIN_PROCESSOR_BUILD
#  define POPULATE_DEVICE_INFO
# endif

/*
 * These macros are used to match the values of the CHIP_IDx_REG registers
 * in order to detect the DEVICE_CHIP_ID.
 */
#define ASCII_3107        0x33313037      // '3' '1' '0' '7'
#define ASCII_2798        0x32373938      // '2' '7' '9' '8'

__RETAINED uint32_t hw_sys_pd_com_acquire_cnt;
__RETAINED uint32_t hw_sys_pd_periph_acquire_cnt;
__RETAINED uint32_t hw_sys_pd_audio_acquire_cnt;
__RETAINED uint32_t hw_sys_pd_gpu_acquire_cnt;

__RETAINED_SHARED static uint32_t hw_sys_device_info_data;

/* Function to apply D2798 preferred settings */
static void hw_sys_set_preferred_values_d2798(HW_PD pd)
{
        switch (pd) {
        case HW_PD_SLP:
                REG_SET_MASKED(CRG_TOP, BANDGAP_REG, 0x00001000, 0x00009020);
                *((volatile uint32_t *) &(CRG_TOP->BIAS_VREF_SEL_REG)) = 0x000000CA;
                REG_SET_MASKED(DCDC, BUCK_CTRL_REG, 0x0000C01C, 0x00004CC4);
                RAW_SETF(0x50000304, 0x100, 1);
                REG_SET_MASKED(CRG_TOP, CLK_RCHS_REG, 0x0000001E, 0x001804B2);
                REG_SET_MASKED(CRG_TOP, CLK_RCX_REG, 0x00000F00, 0x00000DFC);
                REG_SET_MASKED(CRG_TOP, CLK_XTAL32K_REG, 0x000000F8, 0x0000009E);
                REG_SET_MASKED(CRG_TOP, PMU_SLEEP_REG, 0x3F800000, 0x3F881E08);
                REG_SET_MASKED(CRG_TOP, POWER_CTRL_REG, 0x00300008, 0x013CC39B);
                REG_SET_MASKED(CRG_VSYS, VSYS_GEN_IRQ_MASK_REG, 0x00000001, 0x00000003);
                break;
        case HW_PD_SYS:
                REG_SET_MASKED(CHARGER, CHARGER_CTRL_REG, 0x00000C00, 0x003F6A78);
                REG_SET_MASKED(CHARGER, CHARGER_PWR_UP_TIMER_REG, 0x0000001F, 0x00000002);
                RAW_SET_MASKED(0x51000604, 0x0001C000, 0x00001E88);
                break;
        case HW_PD_AON:
                break;
        case HW_PD_MEM:
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }
}

/* Function to apply D3107 preferred settings */
static void hw_sys_set_preferred_values_d3107(HW_PD pd)
{
        switch (pd) {
        case HW_PD_SLP:
                REG_SET_MASKED(CRG_TOP, BANDGAP_REG, 0x00001000, 0x00009020);
                *((volatile uint32_t *) &(CRG_TOP->BIAS_VREF_SEL_REG)) = 0x000000CA;
                REG_SET_MASKED(DCDC_BOOST, BOOST_CTRL_REG1, 0x0001F000, 0x0003BBE4);
                REG_SET_MASKED(DCDC, BUCK_CTRL_REG, 0x0000C01C, 0x00008CC4);
                REG_SET_MASKED(CRG_TOP, CLK_RCHS_REG, 0x0000001E, 0x001804B2);
                REG_SET_MASKED(CRG_TOP, CLK_RCX_REG, 0x00000F00, 0x00000DFC);
                REG_SET_MASKED(CRG_TOP, CLK_XTAL32K_REG, 0x000000F8, 0x0000009E);
                REG_SET_MASKED(CRG_TOP, PMU_SLEEP_REG, 0x3F800000, 0x3F881E08);
                REG_SET_MASKED(CRG_TOP, POWER_CTRL_REG, 0x00300008, 0x013CC39B);
                REG_SET_MASKED(CRG_VSYS, VSYS_GEN_IRQ_MASK_REG, 0x00000001, 0x00000003);
                break;
        case HW_PD_SYS:
                REG_SET_MASKED(CHARGER, CHARGER_CTRL_REG, 0x00000C00, 0x003F6A78);
                REG_SET_MASKED(CHARGER, CHARGER_PWR_UP_TIMER_REG, 0x0000001F, 0x00000002);
                RAW_SET_MASKED(0x51000604, 0x0001C000, 0x00001E88);
                break;
        case HW_PD_AON:
                break;
        case HW_PD_MEM:
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }
}

void hw_sys_set_preferred_values(HW_PD pd)
{
        ASSERT_ERROR(pd < HW_PD_MAX);

        if (hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_2798)) {
                /* D2798AB */
                hw_sys_set_preferred_values_d2798(pd);
        } else if (hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_3107)) {
                /* D3107AB */
                hw_sys_set_preferred_values_d3107(pd);
        }
}

void hw_sys_assert_trigger_gpio(void)
{
        if (EXCEPTION_DEBUG == 1) {
                if (dg_configLP_CLK_SOURCE == LP_CLK_IS_DIGITAL) {
                        hw_clk_configure_ext32k_pins();
                }
                hw_gpio_pad_latch_enable_all();

                DBG_SET_HIGH(EXCEPTION_DEBUG, EXCEPTIONDBG);
        }
}

__RETAINED_CODE void hw_sys_pd_com_enable(void)
{
#if (MAIN_PROCESSOR_BUILD)
        GLOBAL_INT_DISABLE();
        ASSERT_ERROR((!hw_sys_pd_com_acquire_cnt) || !REG_GETF(CRG_TOP, PMU_CTRL_REG, SNC_SLEEP));
        ASSERT_ERROR((hw_sys_pd_com_acquire_cnt) || REG_GETF(CRG_TOP, PMU_CTRL_REG, SNC_SLEEP));
        if (++hw_sys_pd_com_acquire_cnt == 1) {
                hw_pd_power_up_com();
        }
        GLOBAL_INT_RESTORE();

        ASSERT_ERROR(REG_GETF(CRG_TOP, SYS_STAT_REG, SNC_IS_UP));
#endif /* MAIN_PROCESSOR_BUILD */
}

__RETAINED_CODE void hw_sys_pd_com_disable(void)
{
#if (MAIN_PROCESSOR_BUILD)
        ASSERT_ERROR(!REG_GETF(CRG_TOP, PMU_CTRL_REG, SNC_SLEEP));

        GLOBAL_INT_DISABLE();
        ASSERT_ERROR(hw_sys_pd_com_acquire_cnt);
        if (--hw_sys_pd_com_acquire_cnt == 0) {
                hw_pd_power_down_com();
        }
        GLOBAL_INT_RESTORE();
#endif /* MAIN_PROCESSOR_BUILD */
}

__RETAINED_CODE void hw_sys_pd_audio_enable(void)
{
        GLOBAL_INT_DISABLE();
        if (++hw_sys_pd_audio_acquire_cnt == 1) {
                hw_pd_power_up_aud();
        }
        GLOBAL_INT_RESTORE();

        ASSERT_ERROR(REG_GETF(CRG_TOP, SYS_STAT_REG, AUD_IS_UP));
}

__RETAINED_CODE void hw_sys_pd_audio_disable(void)
{
        ASSERT_ERROR(!REG_GETF(CRG_TOP, PMU_CTRL_REG, AUD_SLEEP));

        GLOBAL_INT_DISABLE();
        ASSERT_ERROR(hw_sys_pd_audio_acquire_cnt);
        if (--hw_sys_pd_audio_acquire_cnt == 0) {
                hw_pd_power_down_aud();
        }
        GLOBAL_INT_RESTORE();
}

__RETAINED_CODE void hw_sys_pd_gpu_enable(void)
{
        GLOBAL_INT_DISABLE();
        if (++hw_sys_pd_gpu_acquire_cnt == 1) {
                hw_pd_power_up_gpu();
        }
        GLOBAL_INT_RESTORE();

        ASSERT_ERROR(REG_GETF(CRG_TOP, SYS_STAT_REG, GPU_IS_UP));
}

__RETAINED_CODE void hw_sys_pd_gpu_disable(void)
{
        ASSERT_ERROR(!REG_GETF(CRG_TOP, PMU_CTRL_REG, GPU_SLEEP));

        GLOBAL_INT_DISABLE();
        ASSERT_ERROR(hw_sys_pd_gpu_acquire_cnt);
        if (--hw_sys_pd_gpu_acquire_cnt == 0) {
                hw_pd_power_down_gpu();
        }
        GLOBAL_INT_RESTORE();
}

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Configure XTAL32M current setting.
 *
 * \note Function runs once at startup if XTAL32M_TRIM_REG entry does not
 * exist in CS.
 */
static void xtal32m_configure_cur_set(void)
{
        uint8_t cur_set = 8; // start with mid-scale
        // ********************************************************************************
        // ********************************************************************************
        // * Configuration sequence, find optimum value for CUR_SET
        // * depends on crystal loss
        // ********************************************************************************
        // ********************************************************************************
        REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_CUR_SET, cur_set);

        hw_clk_delay_usec(25); // wait a short bit

        hw_clk_enable_xtalm();

        while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_STATE) != 0x8); // wait to reach RUNNIG state
        REG_SETF(CRG_XTAL, XTAL32M_FSM_REG, XTAL32M_FSM_APPLY_CONFIG, 1);
        RAW_SETF(0x5005041C, 0x1000000UL, 0);

        while (1) {
                uint32_t cnt = 0;
                while (!(REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_CMP_OUT) & 0x2)) {// checks high level
                        cnt += 1;
                        if (cnt > 1000) {// break loop on long delay
                                break;
                        }
                }

                if (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_CMP_OUT) & 0x2) {// checks high level
                        if (cur_set > 0) {
                                cur_set -= 1;
                        } else {
                                break;
                        }

                        REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_CUR_SET, 0); // set lowest current (OFF)

                        cnt = 0;
                        while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_CMP_OUT)) {// checks low level
                                cnt += 1;
                                if (cnt > 1000) {// break loop on long delay
                                        break;
                                }
                        }
                } else { // timeout
                        if (cur_set < 15) {
                                REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_CUR_SET, cur_set);
                        }
                        break; // end sequence
                }

                REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_CUR_SET, cur_set);
        }

        // revert setting:
        REG_SETF(CRG_XTAL, XTAL32M_FSM_REG, XTAL32M_FSM_APPLY_CONFIG, 0);
        RAW_SETF(0x5005041C, 0x1000000UL, 1);
        hw_clk_disable_xtalm();
}

void hw_sys_apply_default_values(void)
{
        const uint32_t reg_in_cs[NUM_OF_REG_ADDR_IN_CS] = {
                (uint32_t)&CRG_XTAL->XTAL32M_TRIM_REG
        };

        bool is_reg_trimmed[NUM_OF_REG_ADDR_IN_CS] = {false};

        // Check for plain register entries
        sys_tcs_reg_pairs_in_cs(reg_in_cs, NUM_OF_REG_ADDR_IN_CS, is_reg_trimmed);

        // Apply preferred settings for BOOST_MODE, AMPL_SET - should be applied prior to configure_cur_set
        REG_SETF(CRG_XTAL, XTAL32M_FSM_REG, XTAL32M_BOOST_MODE, 0x1);
        REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_AMPL_SET, 0x1);

        if (!is_reg_trimmed[0]) {
                xtal32m_configure_cur_set();
        }

        // The following default values should always be applied
        REG_SETF(CRG_XTAL, XTAL32M_START_REG, XTAL32M_TRIM, dg_configDEFAULT_XTAL32M_START_REG__XTAL32M_TRIM__VALUE);
        REG_SETF(CRG_XTAL, XTAL32M_START_REG, XTAL32M_CUR_SET, dg_configDEFAULT_XTAL32M_START_REG__XTAL32M_CUR_SET__VALUE);

        /*
         * Store trim value for RCLP 512 KHz mode
         * If there was no trim value in CS, reset value is stored
         */
        hw_clk_store_rclp_512_mode_trim_value(CRG_TOP->CLK_RCLP_REG & REG_MSK(CRG_TOP, CLK_RCLP_REG, RCLP_TRIM));
        /*
         * Store trim value for RCHS 32 and 96 MHz mode
         * If there was no trim value in CS, reset value is stored
         */
        hw_clk_store_rchs_32_96_mode_trim_value(CRG_TOP->CLK_RCHS_REG & RCHS_REG_TRIM);
        /*
         * Store trim value for RCHS 64 MHz mode
         */
        uint32_t *values;
        uint8_t size;
        sys_tcs_get_custom_values(SYS_TCS_GROUP_RCHS_64MHZ, &values, &size);
        if (size == 1 && values) {
                hw_clk_store_rchs_64_mode_trim_value(*values & RCHS_REG_TRIM);
        }

}

void hw_sys_enable_cmac_mem_protection(void)
{
#if defined(CONFIG_USE_BLE)
#if dg_configUSE_HW_MPU

        GLOBAL_INT_DISABLE();

        /*
         * Apply MPU configuration for CMAC code region only if MPU is not enabled,
         * that is, MPU has been already configured before reaching here.
         */
        if ((!hw_mpu_is_enabled()) && (dg_configCMAC_PROTECT_REGION > HW_MPU_REGION_NONE)) {   /*
                                                                                 * Negative value of
                                                                                 * dg_configCMAC_PROTECT_REGION
                                                                                 * leads to unprotected CMAC.
                                                                                 */
                mpu_region_config region_cfg;

                /* Set ro_region to CMAC code section. CMAC code starts at the beginning of RAM cell 10 and stops at the start of data section.
                 * as MPU region that allows
                 * any RO access (privileged & unprivileged).
                 * RAM cells    RAM Size (KB)     Main Use              AHB CPUS (M33) start   AHB CPUS (M33) end
                 * RAM 10           192           BLE stack code        20150000               20180000
                 * */
                region_cfg.start_addr = MEMORY_SYSRAM10_BASE & ~((uint32_t) MPU_END_ADDRESS_MASK);

                region_cfg.end_addr = (region_cfg.start_addr + REG_GETF(MEMCTRL, CMI_DATA_BASE_REG, CMI_DATA_BASE_ADDR) - 1)
                        | MPU_END_ADDRESS_MASK;
                region_cfg.shareability = HW_MPU_SH_NS;
                region_cfg.access_permissions = HW_MPU_AP_RO;
                region_cfg.attributes = HW_MPU_ATTR_NORMAL;
                region_cfg.execute_never = HW_MPU_XN_TRUE;
                hw_mpu_config_region(dg_configCMAC_PROTECT_REGION, &region_cfg);
                hw_mpu_enable(true);
        }

        GLOBAL_INT_RESTORE();
#endif /* dg_configUSE_HW_MPU */
#endif /* CONFIG_USE_BLE */
        return;
}

void hw_sys_enable_ivt_mem_protection(void)
{
#if dg_configUSE_HW_MPU

        GLOBAL_INT_DISABLE();

        /*
         * Apply MPU configuration for IVT region only if MPU is not enabled,
         * that is, MPU has been already configured before reaching here.
         */
        if (!hw_mpu_is_enabled()) {
                mpu_region_config region_cfg;

                /* Set ro_region to IVT section. IVT code starts at the beginning of RAM cell 0 and stops at length
                 * of 0xFF.
                 * as MPU region that allows
                 * any RO access (privileged & unprivileged).
                 * RAM cells    RAM Size (KB)     Main Use      IVT start       IVT end
                 * RAM 0            8           IVT & Others       0x0            0xFF
                 * */
                region_cfg.start_addr = (0x0) & ~((uint32_t) MPU_END_ADDRESS_MASK);

                region_cfg.end_addr = (region_cfg.start_addr + ((uint32_t)&__Vectors_Size) -1) | MPU_END_ADDRESS_MASK;
                region_cfg.shareability = HW_MPU_SH_NS;
                region_cfg.access_permissions = HW_MPU_AP_RO;
                region_cfg.attributes = HW_MPU_ATTR_NORMAL;
                region_cfg.execute_never = HW_MPU_XN_TRUE;
                hw_mpu_config_region(dg_configIVT_PROTECT_REGION, &region_cfg);
                hw_mpu_enable(true);
        }

        GLOBAL_INT_RESTORE();
#endif /* dg_configUSE_HW_MPU */
        return;
}
#endif /* PROCESSOR_BUILD */

#ifdef POPULATE_DEVICE_INFO
/* Get device variant ID as stored in TCS. */
static uint32_t get_device_variant(void)
{
        uint32_t *variant = NULL;
        uint8_t size = 0;

        sys_tcs_get_custom_values(SYS_TCS_GROUP_CHIP_ID, &variant, &size);

        // If the Device Variant entry has been successfully retrieved
        if ((size == 1) && (variant != NULL)) {
                return MAKE_DEVICE_VARIANT_ENCODING(variant[0]);
        }

        return 0;
}
#endif /* POPULATE_DEVICE_INFO */

bool hw_sys_device_info_init(void)
{
#ifdef POPULATE_DEVICE_INFO
        union {
                uint8_t  arr[4];
                uint32_t value;
        } device_chip_id;

        uint32_t revision = MAKE_DEVICE_REVISION_ENCODING((REG_GETF(CHIP_VERSION, CHIP_REVISION_REG, CHIP_REVISION) - 'A'));
        uint32_t swc = MAKE_DEVICE_SWC_ENCODING(REG_GETF(CHIP_VERSION, CHIP_SWC_REG, CHIP_SWC));
        uint32_t step = MAKE_DEVICE_STEP_ENCODING((REG_GETF(CHIP_VERSION, CHIP_TEST1_REG, CHIP_LAYOUT_REVISION) - 'A'));

        device_chip_id.arr[3] = REG_GETF(CHIP_VERSION, CHIP_ID1_REG, CHIP_ID1);
        device_chip_id.arr[2] = REG_GETF(CHIP_VERSION, CHIP_ID2_REG, CHIP_ID2);
        device_chip_id.arr[1] = REG_GETF(CHIP_VERSION, CHIP_ID3_REG, CHIP_ID3);
        device_chip_id.arr[0] = REG_GETF(CHIP_VERSION, CHIP_ID4_REG, CHIP_ID4);

        switch (device_chip_id.value) {
        case ASCII_3107:
                device_chip_id.value = DEVICE_CHIP_ID_3107;
                break;
        case ASCII_2798:
                device_chip_id.value = DEVICE_CHIP_ID_2798;
                break;
        default:
                return false;
        }

        if ((revision < DEVICE_REVISION_MIN) || (revision > DEVICE_REVISION_MAX) ||
            (swc < DEVICE_SWC_MIN) || (swc > DEVICE_SWC_MAX) ||
            (step < DEVICE_STEP_MIN) || (step > DEVICE_STEP_MAX)) {
                return false;
        }

        hw_sys_device_info_data = DA1470X | device_chip_id.value | revision | swc | step;
#endif /* POPULATE_DEVICE_INFO */
        return true;
}

bool hw_sys_device_variant_init(void)
{
#ifdef POPULATE_DEVICE_INFO
        uint32_t variant = get_device_variant();

        if ((variant < DEVICE_VARIANT_MIN) || (variant > DEVICE_VARIANT_MAX)) {
                return false;
        }

        hw_sys_device_info_data |= variant;
#endif /* POPULATE_DEVICE_INFO */
        return true;
}

bool hw_sys_device_info_check(uint32_t mask, uint32_t attribute)
{
        uint32_t attribute_masked = attribute & mask;
        uint32_t hw_sys_device_info_data_masked = hw_sys_device_info_data & mask;
        uint32_t min = DEVICE_INFO_ATTRIBUTE_MIN(mask);
        uint32_t max = DEVICE_INFO_ATTRIBUTE_MAX(mask);

        // Assert against invalid attribute value
        ASSERT_WARNING((attribute_masked >= min) && (attribute_masked <= max));

        return (hw_sys_device_info_data_masked == attribute_masked);
}

uint32_t hw_sys_get_device_info(void)
{
        return hw_sys_device_info_data;
}

bool hw_sys_is_compatible_chip(void)
{
#ifndef POPULATE_DEVICE_INFO
        return true;
#else
        if (hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_2798)) {
                if (hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_A) &&
                    hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_1)) {
                        return true;
                }
        } else if (hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_3107)) {
                if (hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_A) &&
                    hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_0)) {
                        return true;
                }
        }

        return false;
#endif
}

