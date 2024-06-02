/**
****************************************************************************************
*
* @file hw_clk_da1470x.c
*
* @brief Clock Driver
*
* Copyright (C) 2020-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#if dg_configUSE_HW_CLK

#include <stdint.h>
#include "hw_clk.h"
#include "sys_tcs.h"

__RETAINED static uint32_t rchs_32_96_mode_trim_value;
__RETAINED static uint32_t rchs_64_mode_trim_value;
__RETAINED static uint32_t rclp_512_mode_trim_value;
/*
 * Function definitions
 */

void hw_clk_store_rchs_32_96_mode_trim_value(uint32_t trim_value)
{
        rchs_32_96_mode_trim_value = trim_value;
}

void hw_clk_store_rchs_64_mode_trim_value(uint32_t trim_value)
{
        rchs_64_mode_trim_value = trim_value;
}

/**
 * \brief Set the speed of RCHS output.
 *
 * \param[in] mode The speed of the RCHS output.
 *
 * \note Switching to/from 64MHz requires the RCHS to settle, which can be > 100us.
 *       Switching 32MHz to/from 96MHz does not require settling.
 */
void hw_clk_set_rchs_mode(rchs_speed_t mode)
{
        /*
         * initialize reg_value with CLK_RCHS_REG fields RCHS_INIT_DTC, RCHS_INIT_DTCF
         * RCHS_INIT_DEL, RCHS_INIT_RANGE reset values
         */
        uint32_t reg_value = ((0x5  << REG_POS(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_DTC))  |
                              (0x2  << REG_POS(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_DTCF)) |
                              (0x80 << REG_POS(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_DEL))  |
                              (0x1  << REG_POS(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_RANGE)));

        if ((mode & RCHS_64) != 0) {
                if (rchs_64_mode_trim_value) {
                        reg_value = rchs_64_mode_trim_value;
                }
        } else {
                reg_value = rchs_32_96_mode_trim_value;
        }

        reg_value |= (mode << REG_POS(CRG_TOP, CLK_RCHS_REG, RCHS_SPEED));

        GLOBAL_INT_DISABLE();

        /* V12 level voltage must be set to 1.2V prior to setting RCHS at 64MHz/96Mhz */
        ASSERT_ERROR((mode == RCHS_32) || (REG_GETF(CRG_TOP, POWER_LVL_REG , V12_LEVEL) == 2));

        REG_SET_MASKED(CRG_TOP, CLK_RCHS_REG, (RCHS_REG_TRIM | REG_MSK(CRG_TOP, CLK_RCHS_REG, RCHS_SPEED)), reg_value);

        GLOBAL_INT_RESTORE();
}


void hw_clk_store_rclp_512_mode_trim_value(uint32_t trim_value)
{
        rclp_512_mode_trim_value = trim_value;
}

void hw_clk_set_rclp_mode(rclp_mode_t mode)
{
        /* initialize reg_value with CLK_RCLP_REG[RCLP_TRIM] reset value */
        uint32_t reg_value = (0x7 << REG_POS(CRG_TOP, CLK_RCLP_REG, RCLP_TRIM));

        if (mode == RCLP_FORCE_SLOW) {
                uint32_t *values;
                uint8_t size;
                sys_tcs_get_custom_values(SYS_TCS_GROUP_RCLP_32KHZ, &values, &size);
                if (size == 1 && values) {
                        reg_value = *values;
                }
                reg_value |= REG_MSK(CRG_TOP, CLK_RCLP_REG, RCLP_LOW_SPEED_FORCE);
        } else {
                reg_value = rclp_512_mode_trim_value;
        }

        GLOBAL_INT_DISABLE();
        REG_SET_MASKED(CRG_TOP, CLK_RCLP_REG, REG_MSK(CRG_TOP, CLK_RCLP_REG, RCLP_LOW_SPEED_FORCE) |
                                              REG_MSK(CRG_TOP, CLK_RCLP_REG, RCLP_TRIM), reg_value);
        GLOBAL_INT_RESTORE();
}

__RETAINED_HOT_CODE void hw_clk_start_calibration(cal_clk_t clk_type, cal_ref_clk_t clk_ref_type, uint16_t cycles)
{
        uint32_t val = 0;

        /* Must be disabled */
        ASSERT_WARNING(!REG_GETF(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START));

        ANAMISC_BIF->CLK_REF_CNT_REG = cycles;                      // # of cal clock cycles

        if (clk_ref_type == CALIBRATE_REF_EXT) {
                REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, EXT_CNT_EN_SEL, val, 1);
                REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, CAL_CLK_SEL, val, 0); /* DivN to be calibrated */
        } else {
                REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, CAL_CLK_SEL, val, clk_ref_type);
        }
        REG_SET_FIELD(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CLK_SEL, val, clk_type);
        ANAMISC_BIF->CLK_REF_SEL_REG = val;

        REG_SET_BIT(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START);
}

uint32_t hw_clk_get_calibration_data(void)
{
        /* Wait until it's finished */
        while (REG_GETF(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START)) {
        }

        return ANAMISC_BIF->CLK_REF_VAL_REG;
}

__RETAINED_CODE uint32_t hw_clk_get_sysclk_freq(void)
{
        switch (hw_clk_get_sysclk()) {
        case SYS_CLK_IS_RCHS:
                switch (hw_clk_get_rchs_mode()) {
                case RCHS_32:
                        return dg_configRCHS_32M_FREQ;
                case RCHS_96:
                        return dg_configRCHS_96M_FREQ;
                case RCHS_64:
                        return dg_configRCHS_64M_FREQ;
                default:
                        ASSERT_WARNING(0);
                        return dg_configRCHS_32M_FREQ;
                }
                break;
        case SYS_CLK_IS_XTAL32M:
                return dg_configXTAL32M_FREQ;

        case SYS_CLK_IS_PLL:
                return dg_configPLL160M_FREQ;
        default:
                ASSERT_WARNING(0);
                return dg_configRCHS_32M_FREQ;
        }
}
#define CLK_DELAY_SANITY_CHECKS
#pragma GCC push_options
#pragma GCC optimize ("O3")
#define DIVIDER 1000000
void hw_clk_delay_usec(uint32_t usec)
{
#ifdef CLK_DELAY_SANITY_CHECKS
        _Static_assert((dg_configXTAL32M_FREQ % DIVIDER) == 0, "dg_configXTAL32M_FREQ % DIVIDER != 0");
        _Static_assert((dg_configPLL160M_FREQ % DIVIDER) == 0, "dg_configPLL160M_FREQ % DIVIDER != 0");
        _Static_assert((HW_CLK_DELAY_OVERHEAD_CYCLES % HW_CLK_CYCLES_PER_DELAY_REP) == 0,
                "HW_CLK_DELAY_OVERHEAD_CYCLES % HW_CLK_CYCLES_PER_DELAY_REP != 0");
#endif

        static const uint8_t OVERHEAD_REPS = HW_CLK_DELAY_OVERHEAD_CYCLES / HW_CLK_CYCLES_PER_DELAY_REP;

#if (MAIN_PROCESSOR_BUILD)
        const uint32_t cycles_per_usec = (hw_clk_get_sysclk_freq() / DIVIDER) >> hw_clk_get_hclk_div();
#elif (SNC_PROCESSOR_BUILD)
        const uint32_t cycles_per_usec = (dg_configDIVN_FREQ / DIVIDER) >> hw_clk_get_hclk_div();
#endif /* PROCESSOR_BUILD */
	uint32_t reps = cycles_per_usec * usec / HW_CLK_CYCLES_PER_DELAY_REP;

#ifdef CLK_DELAY_SANITY_CHECKS
        ASSERT_WARNING(usec <= 0xFFFFFFFF/cycles_per_usec);     // The requested delay is greater than the maximum delay this function can achieve
#endif

        if (reps <= OVERHEAD_REPS) {
                // The requested delay is smaller than the minimum delay this function can achieve.
                // Set the minimum number of reps
                reps = OVERHEAD_REPS + 1;
        }

        asm volatile(
                "       .syntax unified                 \n"     // to prevent CM0/M0+ non-unified syntax
                "       nop                             \n"
                "       nop                             \n"
                "       nop                             \n"
                "       nop                             \n"
                "       nop                             \n"
                "loop:  nop                             \n"
                "       subs %[reps], %[reps], #1       \n"
                "       bne loop                        \n"
                :                                       // outputs
                : [reps] "r" (reps - OVERHEAD_REPS)     // inputs
                :                                       // clobbers
        );
}

#pragma GCC pop_options

#if (MAIN_PROCESSOR_BUILD)
__STATIC_INLINE void finish_xtal32m_config(void)
{
        // Apply preferred settings for SAH fields - should be applied after all other configuration
        REG_SET_MASKED(CRG_XTAL, XTAL32M_CTRL_REG, 0x00000033, 0x00000000);
}

void hw_clk_xtalm_configure_irq(void)
{
        uint8_t irq_cnt_rst = 0xFF;
        uint8_t irq_val;

        hw_clk_disable_xtalm();

        hw_clk_delay_usec(10000); // wait until oscillation is completely stopped (max. 10ms)

        // Use reset values for SAH fields
        REG_SET_MASKED(CRG_XTAL, XTAL32M_CTRL_REG, 0x0000003F, 0x00000015);

        REG_SETF(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CNT, irq_cnt_rst); // program reset value

        hw_clk_enable_xtalm(); // enable xtal32m

        while (!hw_clk_is_xtalm_started()); // wait until READY bit is set

        irq_val = irq_cnt_rst - REG_GETF(CRG_XTAL, XTAL32M_IRQ_STAT_REG, XTAL32M_IRQ_COUNT_CAP);
        irq_val += 5; // added 4 for xtal32m_ready signal and 1 cycle resolution
        irq_val *= 2; // multiply by 2 for temperature margin

        REG_SETF(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CNT, irq_val);

        hw_clk_disable_xtalm();

        finish_xtal32m_config();
}

void hw_clk_set_xtalm_settling_time(uint8_t cycles, bool high_clock)
{
        uint32_t tmp = CRG_XTAL->XTAL32M_IRQ_CTRL_REG;
        REG_SET_FIELD(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CNT, tmp, cycles);
        REG_SET_FIELD(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CLK, tmp, high_clock ? 0 : 1);

        CRG_XTAL->XTAL32M_IRQ_CTRL_REG = tmp;

        finish_xtal32m_config();
}
#endif /* PROCESSOR_BUILD */

__RETAINED_CODE sys_clk_t hw_clk_get_system_clock(void)
{
        switch (hw_clk_get_sysclk()) {
        case SYS_CLK_IS_RCHS:
                switch (hw_clk_get_rchs_mode()) {
                case RCHS_32:
                        return sysclk_RCHS_32;
                case RCHS_96:
                        return sysclk_RCHS_96;
                case RCHS_64:
                        return sysclk_RCHS_64;
                default:
                        ASSERT_WARNING(0);
                        return sysclk_RCHS_32;
                }
                break;
        case SYS_CLK_IS_XTAL32M:
                return sysclk_XTAL32M;

        case SYS_CLK_IS_PLL:
                return sysclk_PLL160;
        default:
                ASSERT_WARNING(0);
                return sysclk_RCHS_32;
        }
}


#endif /* dg_configUSE_HW_CLK */

