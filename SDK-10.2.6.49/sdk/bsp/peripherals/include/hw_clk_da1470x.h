/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup HW_CLK HW Clock Driver
\{
\brief Clock Driver
*/

/**
****************************************************************************************
*
* @file hw_clk_da1470x.h
*
* @brief Clock Driver header file.
*
* Copyright (C) 2020-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef HW_CLK_DA1470x_H_
#define HW_CLK_DA1470x_H_


#if dg_configUSE_HW_CLK

#include "sdk_defs.h"

#define HW_CLK_DELAY_OVERHEAD_CYCLES   (72)
#define HW_CLK_CYCLES_PER_DELAY_REP    (4)

/**
 * \brief Convert settling time (in usec) to 250KHz clock cycles
 *
 * \note The 250KHz clock is derived from RCHS32M_DivN divided by 128.
 */
#define XTAL32M_USEC_TO_250K_CYCLES(x)  ((uint16_t)((x * (dg_configRCHS_32M_FREQ/1000000) + 127) / 128 ))

/**
 * \brief Convert XTAL32M Ready IRQ counter cycles to LP clock cycles
 */
#define XTALRDY_CYCLES_TO_LP_CLK_CYCLES(x, lp_freq) ((((uint32_t)(x)) * lp_freq + dg_configRCHS_FREQ_MIN/(128) - 1) / (dg_configRCHS_FREQ_MIN/128))

/* RCHS_BIAS is common for all the RCHS modes so it should not be part of the mask. */
#define RCHS_REG_TRIM   (REG_MSK(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_DTC)  |       \
                         REG_MSK(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_DTCF) |       \
                         REG_MSK(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_DEL)  |       \
                         REG_MSK(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_RANGE))

/**
 * \addtogroup CLOCK_TYPES
 * \{
 */

/**
 * \brief The type of the system clock
 */
typedef enum sys_clk_is_type {
        SYS_CLK_IS_XTAL32M = 0,
        SYS_CLK_IS_RCHS,
        SYS_CLK_IS_RCLP,
        SYS_CLK_IS_PLL,
        SYS_CLK_IS_INVALID
} sys_clk_is_t;

/**
 * \}
 */

/**
 * \brief The type of clock to be calibrated
 */
typedef enum cal_clk_sel_type {
        CALIBRATE_RCLP = 0,
        CALIBRATE_RCHS = 1,
        CALIBRATE_XTAL32K = 2,
        CALIBRATE_RCX = 3,
        CALIBRATE_DIVN = 5,
} cal_clk_t;

/**
 * \brief The reference clock used for calibration
 */
typedef enum cal_ref_clk_sel_type {
        CALIBRATE_REF_DIVN = 0,
        CALIBRATE_REF_RCLP = 1,
        CALIBRATE_REF_RCHS = 2,
        CALIBRATE_REF_XTAL32K = 3,
        CALIBRATE_REF_EXT = 5,
} cal_ref_clk_t;

/**
 * \brief The system clock type
 *
 * \note Must only be used with functions cm_sys_clk_init(), cm_sys_clk_set(),
 *      cm_sys_clk_request/release()
 */
typedef enum sysclk_type {
        sysclk_RCHS_32 = 0,     //!< RCHS 32MHz
        sysclk_XTAL32M = 2,     //!< 32MHz
        sysclk_RCHS_64 = 4,     //!< RCHS 64MHz
        sysclk_RCHS_96 = 6,     //!< RCHS 96MHz
        sysclk_PLL160  = 10,    //!< 160MHz
        sysclk_BOOTER  = 11,    //!< leave clock decision up to the booter (defined by CS content)
        sysclk_LP      = 255,   //!< not applicable
} sys_clk_t;

/**
 * \brief The RCLP mode
 *
 * \note Must only be used with function hw_clk_set_rclp_mode()
 */
typedef enum rclp_mode_type {
        RCLP_FORCE_FAST = 0,    //!< 512kHz
        RCLP_FORCE_SLOW,        //!< 32kHz
} rclp_mode_t;

/**
 * \brief The RCHS speed output
 *
 * \note Must only be used with functions hw_clk_set/get_rchs_mode()
 */
typedef enum rchs_speed_type {
        RCHS_32 = 0,    //!< 32MHz
        RCHS_96,        //!< 96MHz
        RCHS_64,        //!< 64MHz
} rchs_speed_t;

/**
 * \brief The CPU clock type (speed)
 */
typedef enum cpu_clk_type {
        cpuclk_2M   = 2,    //!< 2 MHz
        cpuclk_4M   = 4,    //!< 4 MHz
        cpuclk_6M   = 6,    //!< 6 MHz
        cpuclk_8M   = 8,    //!< 8 MHz
        cpuclk_10M  = 10,   //!< 10 MHz
        cpuclk_12M  = 12,   //!< 12 MHz
        cpuclk_16M  = 16,   //!< 16 MHz
        cpuclk_20M  = 20,   //!< 20 MHz
        cpuclk_24M  = 24,   //!< 24 MHz
        cpuclk_32M  = 32,   //!< 32 MHz
        cpuclk_40M  = 40,   //!< 40 MHz
        cpuclk_48M  = 48,   //!< 48 MHz
        cpuclk_64M  = 64,   //!< 64 MHz
        cpuclk_80M  = 80,   //!< 80 MHz
        cpuclk_96M  = 96,   //!< 96 MHz
        cpuclk_160M = 160   //!< 160 MHz
} cpu_clk_t;

/**
 * \brief Set the divider of the Slow AMBA Peripheral Bus.
 *
 * \param div The Slow AMBA Peripheral Bus divider
 */
__STATIC_FORCEINLINE void hw_clk_set_pclk_slow_div(apb_div_t div)
{
        ASSERT_WARNING(div <= apb_div16);
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_AMBA_REG, SLOW_PCLK_DIV, div);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Get the divider of the Slow AMBA Peripheral Bus.
 *
 * \return The Slow AMBA Peripheral Bus divider
 */
__STATIC_FORCEINLINE apb_div_t hw_clk_get_pclk_slow_div(void)
{
        return REG_GETF(CRG_TOP, CLK_AMBA_REG, SLOW_PCLK_DIV);
}

/**
 * \brief Check if the RCHS is enabled.
 *
 * \return true if the RCHS is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_check_rchs_status(void)
{
        return REG_GETF(CRG_TOP, CLK_RCHS_REG, RCHS_ENABLE);
}

/**
 * \brief Activate the RCHS.
 */
__STATIC_INLINE void hw_clk_enable_rchs(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_RCHS_REG, RCHS_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Deactivate the RCHS.
 */
__STATIC_FORCEINLINE void hw_clk_disable_rchs(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_RCHS_REG, RCHS_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Store trim value for RCHS 32 and 96 MHz mode
 *
 * \param[in] trim_value trim value for RCHS 32 and 96 MHz mode.
 */
void hw_clk_store_rchs_32_96_mode_trim_value(uint32_t trim_value);

/**
 * \brief Store trim value for RCHS 64 MHz mode
 *
 * \param[in] trim_value trim value for RCHS 64 MHz mode.
 */

void hw_clk_store_rchs_64_mode_trim_value(uint32_t trim_value);

/**
 * \brief Set the speed of RCHS output.
 *
 * \param[in] mode The speed of the RCHS output.
 *
 * \note Switching to/from 64MHz requires the RCHS to settle, which can be > 100us.
 *       Switching 32MHz to/from 96MHz does not require settling.
 */
void hw_clk_set_rchs_mode(rchs_speed_t mode);

/**
 * \brief Get the speed of RCHS output.
 *
 * \return the speed of the RCHS output.
 */
__STATIC_INLINE rchs_speed_t hw_clk_get_rchs_mode(void)
{
        rchs_speed_t rchs_speed = REG_GETF(CRG_TOP, CLK_RCHS_REG, RCHS_SPEED);

        if (rchs_speed & RCHS_64) {
                return RCHS_64;
        } else {
                return rchs_speed;
        }
}

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Set the XTAL32M settling time.
 *
 * \param cycles Number of clock cycles
 * \param high_clock If true use 250kHZ clock, false use 31.25kHz clock
 */
void hw_clk_set_xtalm_settling_time(uint8_t cycles, bool high_clock);
#endif

/**
 * \brief Get the XTAL32M settling time.
 *
 * \return The number of 250KHz clock cycles required for XTAL32M to settle
 */
__STATIC_FORCEINLINE uint16_t hw_clk_get_xtalm_settling_time(void)
{
        uint32_t val = CRG_XTAL->XTAL32M_IRQ_CTRL_REG;
        uint16_t cycles = REG_GET_FIELD(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CNT, val);

        if (REG_GET_FIELD(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_CLK, val) == 1) {
                // 31.25KHz clock cycles. Convert them to 250KHz clock cycles.
                cycles *= 8;
        }
        return cycles;
}

/**
 * \brief Check if the XTAL32M is enabled.
 *
 * \return true if the XTAL32M is enabled, else false.
 */
__STATIC_FORCEINLINE bool hw_clk_check_xtalm_status(void)
{
        return REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_READY) == 1;
}

/**
 * \brief Activate the XTAL32M.
 */
__STATIC_INLINE void hw_clk_enable_xtalm(void)
{
        /* Do nothing if XTAL32M is already up and running. */
        if (hw_clk_check_xtalm_status()) {
                return;
        }
        // Check the power supply
        ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V14_EN));
        /* Enable the XTAL oscillator. */
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_XTAL, XTAL32M_CTRL_REG, XTAL32M_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Deactivate the XTAL32M.
 */
__STATIC_INLINE void hw_clk_disable_xtalm(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_XTAL, XTAL32M_CTRL_REG, XTAL32M_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Check if the XTAL32M has settled.
 *
 * \return true if the XTAL32M has settled, else false.
 */
__STATIC_INLINE bool hw_clk_is_xtalm_started(void)
{
        return (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_READY));
}

/**
 * \brief Return the clock used as the system clock.
 *
 * \return The type of the system clock
 */
__STATIC_FORCEINLINE sys_clk_is_t hw_clk_get_sysclk(void)
{
        static const uint32_t freq_msk = CRG_TOP_CLK_CTRL_REG_RUNNING_AT_RCLP_Msk |
                                   CRG_TOP_CLK_CTRL_REG_RUNNING_AT_RCHS_Msk |
                                   CRG_TOP_CLK_CTRL_REG_RUNNING_AT_XTAL32M_Msk |
                                   CRG_TOP_CLK_CTRL_REG_RUNNING_AT_PLL_Msk;

        static __RETAINED_CONST_INIT const sys_clk_is_t clocks[] = {
                SYS_CLK_IS_RCLP,        // 0b000
                SYS_CLK_IS_RCHS,        // 0b001
                SYS_CLK_IS_XTAL32M,     // 0b010
                SYS_CLK_IS_INVALID,
                SYS_CLK_IS_PLL          // 0b100
        };

        // drop bit0 to reduce the size of clocks[]
        uint32_t index = (CRG_TOP->CLK_CTRL_REG & freq_msk) >> (CRG_TOP_CLK_CTRL_REG_RUNNING_AT_RCLP_Pos + 1);
        ASSERT_WARNING(index <= 4);

        sys_clk_is_t clk = clocks[index];
        ASSERT_WARNING(clk != SYS_CLK_IS_INVALID);
        return clk;
}

/**
 * \brief Get the current system clock.
 *
 * \return The current system clock.
 *
 * \warning This function returns a sys_clk_t enum, whereas the hw_clk_get_sysclk() a sys_clk_is_t.
 *          Consider calling the right function, based on which type of enumerator is needed.
 */
__RETAINED_CODE sys_clk_t hw_clk_get_system_clock(void);

/**
 * \brief Check whether the XTAL32K is the Low Power clock.
 *
 * \return true if XTAL32K is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_xtal32k(void)
{
        return REG_GETF(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE) &&
              (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_XTAL32K);
}

/**
 * \brief Check whether the RCLP is the Low Power clock.
 *
 * \return true if RCLP is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_rclp(void)
{
        return (REG_GETF(CRG_TOP, CLK_RCLP_REG, RCLP_ENABLE)) &&
              (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_RCLP);
}

/**
 * \brief Check whether the RCX is the Low Power clock.
 *
 * \return true if RCX is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_rcx(void)
{
        return REG_GETF(CRG_TOP, CLK_RCX_REG, RCX_ENABLE) &&
              (REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_RCX);
}

/**
 * \brief Check whether the RCX is the Low Power clock.
 *
 * \return true if RCX is the LP clock, else false.
 */
__STATIC_INLINE bool hw_clk_lp_is_external(void)
{
        return REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) == LP_CLK_IS_EXTERNAL;
}

/**
 * \brief Set RCX as the Low Power clock.
 *
 * \warning The RCX must have been enabled before calling this function!
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_rcx(void)
{
#if (MAIN_PROCESSOR_BUILD)
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());
#elif (SNC_PROCESSOR_BUILD)
        ASSERT_WARNING(__get_PRIMASK() == 1);
#endif /* PROCESSOR_BUILD */
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_RCX_REG, RCX_ENABLE));
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_RCX);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set XTAL32K as the Low Power clock.
 *
 * \warning The XTAL32K must have been enabled before calling this function!
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_xtal32k(void)
{
#if (MAIN_PROCESSOR_BUILD)
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());
#elif (SNC_PROCESSOR_BUILD)
        ASSERT_WARNING(__get_PRIMASK() == 1);
#endif /* PROCESSOR_BUILD */
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE));
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_XTAL32K);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set an external digital clock as the Low Power clock.
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_ext32k(void)
{
#if (MAIN_PROCESSOR_BUILD)
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());
#elif (SNC_PROCESSOR_BUILD)
        ASSERT_WARNING(__get_PRIMASK() == 1);
#endif /* PROCESSOR_BUILD */
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_EXTERNAL);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Store trim value for RCLP 512 KHz mode
 *
 * \param[in] trim_value trim value for RCLP 512 KHz mode.
 */
void hw_clk_store_rclp_512_mode_trim_value(uint32_t trim_value);

/**
 * \brief Configure RCLP.
 *
 * \param[in] mode The speed mode of the RCLP.
 */
void hw_clk_set_rclp_mode(rclp_mode_t mode);

/**
 * \brief Get RCLP speed mode
 *
 * \return the speed mode of the RCLP.
 */
__STATIC_INLINE rclp_mode_t hw_clk_get_rclp_mode(void)
{
        return REG_GETF(CRG_TOP, CLK_RCLP_REG, RCLP_LOW_SPEED_FORCE);
}

/**
 * \brief Enable RCLP.
 */
__STATIC_INLINE void hw_clk_enable_rclp(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_RCLP_REG, RCLP_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable RCLP.
 *
 * \warning This bit is gated to '0' automatically when sleep state is entered,
 * and PMU_CTRL_REG.ENABLE_CLKLESS is set to '1'. Do not disable this bit, as
 * deep sleep state is not correctly entered.
 */
__STATIC_INLINE void hw_clk_disable_rclp(void)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) != LP_CLK_IS_RCLP);
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_RCLP_REG, RCLP_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set RCLP as the Low Power clock.
 *
 * \warning The RCLP must have been enabled before calling this function!
 *
 * \note Call with interrupts disabled to ensure that CLK_CTRL_REG
 *       read/modify/write operation is not interrupted
 */
__STATIC_INLINE void hw_clk_lp_set_rclp(void)
{
#if (MAIN_PROCESSOR_BUILD)
        ASSERT_WARNING(__get_PRIMASK() == 1 || __get_BASEPRI());
#elif (SNC_PROCESSOR_BUILD)
        ASSERT_WARNING(__get_PRIMASK() == 1);
#endif /* PROCESSOR_BUILD */
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_RCLP_REG, RCLP_ENABLE));
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL, LP_CLK_IS_RCLP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Configure RCX. This must be done only once since the register is retained.
 */
__STATIC_INLINE void hw_clk_configure_rcx(void)
{
        // Reset values for CLK_RCX_REG register should be used
}

/**
 * \brief Enable RCX but does not set it as the LP clock.
 */
__STATIC_INLINE void hw_clk_enable_rcx(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_RCX_REG, RCX_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable RCX.
 *
 * \warning RCX must not be the LP clock
 */
__STATIC_INLINE void hw_clk_disable_rcx(void)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) != LP_CLK_IS_RCX);
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_RCX_REG, RCX_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Configure XTAL32K. This must be done only once since the register is retained.
 */
__STATIC_INLINE void hw_clk_configure_xtal32k(void)
{
        /*
         * The XTAL32K configuration is applied at system intialization
         * when applying the preferred values.
         */
}

/**
 * \brief Enable XTAL32K but do not set it as the LP clock.
 */
__STATIC_INLINE void hw_clk_enable_xtal32k(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable XTAL32K.
 *
 * \warning XTAL32K must not be the LP clock.
 */
__STATIC_INLINE void hw_clk_disable_xtal32k(void)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, CLK_CTRL_REG, LP_CLK_SEL) != LP_CLK_IS_XTAL32K);
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_XTAL32K_REG, XTAL32K_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Enable the clock calibration interrupt.
 *
 */
__STATIC_INLINE void hw_clk_calibration_enable_irq(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(ANAMISC_BIF, CLK_CAL_IRQ_REG, CLK_CAL_IRQ_EN);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Clear the clock calibration interrupt.
 *
 */
__STATIC_INLINE void hw_clk_calibration_clear_irq(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(ANAMISC_BIF, CLK_CAL_IRQ_REG, CLK_CAL_IRQ_CLR);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Read the status of the clock calibration interrupt.
 *
 * \return the status of the IRQ bit
 */
__STATIC_INLINE bool hw_clk_calibration_status_irq(void)
{
        return REG_GETF(ANAMISC_BIF, CLK_CAL_IRQ_REG, CLK_CAL_IRQ_STATUS) == 0;
}

/**
 * \brief Check the status of a requested calibration.
 *
 * \return true if the calibration has finished (or never run) else false.
 */
__STATIC_INLINE bool hw_clk_calibration_finished(void)
{
        return REG_GETF(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START) == 0;
}

/**
 * \brief Start calibration of a clock.
 *
 * \param[in] clk_type The clock to be calibrated. Must be enabled.
 * \param[in] clk_ref_type The reference clock to USE.
 * \param[in] cycles The number of cycles of the to-be-calibrated clock to be measured using the
 *            reference clock.
 *
 * \warning If clk_ref_type == CALIBRATE_REF_EXT, the clk_type is not used. Instead, the value
 *          returned by hw_clk_get_calibration_data() is the number of clock cycles of DIVN counted
 *          during a single pulse of the EXT clock source used.
 *          The EXT clock source must be applied to a pin with a PID equal to HW_GPIO_FUNC_UART_RX
 */
__RETAINED_HOT_CODE void hw_clk_start_calibration(cal_clk_t clk_type, cal_ref_clk_t clk_ref_type, uint16_t cycles);

/**
 * \brief Return the calibration results.
 *
 * \return The number of cycles of the reference clock corresponding to the programmed
 * (in hw_clk_start_calibration() cycles param) cycles of the clock to be calibrated.
 * In the special case of EXTernal calibration, this function returns the number of cycles of DIVN
 * that correspond to one positive pulse of the EXT source applied.
 */
uint32_t hw_clk_get_calibration_data(void);

/**
 * \brief Set System clock.
 *
 * \param[in] mode The new system clock.
 *
 * \note System clock switch to PLL is only allowed when current system clock is XTAL32M.
 * System clock switch from PLL is only allowed when new system clock is XTAL32M.
 */
__STATIC_FORCEINLINE void hw_clk_set_sysclk(sys_clk_is_t mode)
{
        /* Make sure a valid sys clock is requested */
        ASSERT_WARNING(mode <= SYS_CLK_IS_PLL);

        /* Switch to PLL is only allowed when current system clock is XTAL32M */
        ASSERT_WARNING(mode != SYS_CLK_IS_PLL ||
                REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_XTAL32M)  ||
                REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL))

        /* Switch from PLL is only allowed when new system clock is XTAL32M */
        ASSERT_WARNING(!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL) ||
                mode == SYS_CLK_IS_XTAL32M  ||
                mode == SYS_CLK_IS_PLL);

        if (mode == SYS_CLK_IS_XTAL32M && REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_RCHS)) {
                ASSERT_WARNING(hw_clk_check_xtalm_status());

                GLOBAL_INT_DISABLE();
                REG_SET_BIT(CRG_TOP, CLK_SWITCH2XTAL_REG, SWITCH2XTAL);
                GLOBAL_INT_RESTORE();
        } else {
                GLOBAL_INT_DISABLE();
                REG_SETF(CRG_TOP, CLK_CTRL_REG, SYS_CLK_SEL, mode);
                GLOBAL_INT_RESTORE();
        }

        /* Wait until the switch is done! */
        switch (mode) {
        case SYS_CLK_IS_XTAL32M:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_XTAL32M)) {
                }
                return;

        case SYS_CLK_IS_RCHS:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_RCHS)) {
                }
                return;

        case SYS_CLK_IS_RCLP:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_RCLP)) {
                }
                return;

        case SYS_CLK_IS_PLL:
                while (!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL)) {
                }
                return;
        default:
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Enable the system PLL (160MHz).
 */
__STATIC_FORCEINLINE void hw_clk_pll_sys_on(void)
{
        GLOBAL_INT_DISABLE();

        /* V12 level voltage must be set to 1.2V prior to enabling PLL */
        ASSERT_ERROR(REG_GETF(CRG_TOP, POWER_LVL_REG , V12_LEVEL) == 2);

        /* LDO PLL enable. */
        REG_SET_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, LDO_PLL_ENABLE);

        /* Check the status of the PLL LDO before enabling it! */
        while (!REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, LDO_PLL_OK));

        /* Now turn on PLL. */
        REG_SET_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_EN);

        REG_SET_BIT(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_RST_N);

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable the system PLL (160MHz).
 *
 * \warning The System clock must have been set to RCHS or XTAL32M before calling this function!
 */
__STATIC_FORCEINLINE void hw_clk_pll_sys_off(void)
{
        GLOBAL_INT_DISABLE();

        // The PLL is not the system clk.
        ASSERT_WARNING(!REG_GETF(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_PLL));

        uint32_t val = CRG_XTAL->PLL_SYS_CTRL1_REG;

        REG_CLR_FIELD(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_RST_N, val);

        /* Turn off PLL. */
        REG_CLR_FIELD(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_EN, val);
        /* LDO PLL disable. */
        REG_CLR_FIELD(CRG_XTAL, PLL_SYS_CTRL1_REG, LDO_PLL_ENABLE, val);

        CRG_XTAL->PLL_SYS_CTRL1_REG = val;

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Check if the system PLL (160MHz) is enabled.
 *
 * \return true if the PLL is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_check_pll_status(void)
{
        return REG_GETF(CRG_XTAL, PLL_SYS_CTRL1_REG, PLL_EN);
}

/**
 * \brief Check if the system PLL (160MHz) is on and has locked.
 *
 * \return true if the PLL has locked, else false.
 */
__STATIC_INLINE bool hw_clk_is_pll_locked(void)
{
        return REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, PLL_LOCK_FINE);
}

/**
 * \brief Enable the USB PLL (48MHz).
 */
__STATIC_FORCEINLINE void hw_clk_pll_usb_on(void)
{
        GLOBAL_INT_DISABLE();

        /* V12 level voltage must be set to 1.2V prior to enabling PLL */
        ASSERT_ERROR(REG_GETF(CRG_TOP, POWER_LVL_REG , V12_LEVEL) == 2);

        /* XTAL32M must have been started prior to enabling PLL */
        ASSERT_ERROR(hw_clk_is_xtalm_started());

        /* LDO PLL enable. */
        REG_SET_BIT(CRG_XTAL, PLL_USB_CTRL1_REG, LDO_PLL_ENABLE);

        /* Check the status of the PLL LDO before enabling it! */
        while (!REG_GETF(CRG_XTAL, PLL_USB_STATUS_REG, LDO_PLL_OK));

        /* Now turn on PLL. */
        REG_SET_BIT(CRG_XTAL, PLL_USB_CTRL1_REG, PLL_EN);

        REG_SET_BIT(CRG_XTAL, PLL_USB_CTRL1_REG, PLL_RST_N);

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable the USB PLL (48MHz).
 *
 */
__STATIC_FORCEINLINE void hw_clk_pll_usb_off(void)
{
        GLOBAL_INT_DISABLE();

        uint32_t val = CRG_XTAL->PLL_USB_CTRL1_REG;

        REG_CLR_FIELD(CRG_XTAL, PLL_USB_CTRL1_REG, PLL_RST_N, val);

        /* Turn off PLL. */
        REG_CLR_FIELD(CRG_XTAL, PLL_USB_CTRL1_REG, PLL_EN, val);
        /* LDO PLL disable. */
        REG_CLR_FIELD(CRG_XTAL, PLL_USB_CTRL1_REG, LDO_PLL_ENABLE, val);

        CRG_XTAL->PLL_USB_CTRL1_REG = val;

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Check if the USB PLL (48MHz) is enabled.
 *
 * \return true if the PLL is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_check_pll_usb_status(void)
{
        return REG_GETF(CRG_XTAL, PLL_USB_CTRL1_REG, PLL_EN);
}

/**
 * \brief Check if the USB PLL (48MHz) is on and has locked.
 *
 * \return true if the PLL has locked, else false.
 */
__STATIC_INLINE bool hw_clk_is_pll_usb_locked(void)
{
        return REG_GETF(CRG_XTAL, PLL_USB_STATUS_REG, PLL_LOCK_FINE);
}

/**
 * \brief Activate a System clock.
 *
 * \param[in] clk The clock to activate.
 */
__STATIC_INLINE void hw_clk_enable_sysclk(sys_clk_is_t clk)
{
        switch (clk) {
        case SYS_CLK_IS_XTAL32M:
                hw_clk_enable_xtalm();
                return;
        case SYS_CLK_IS_RCHS:
                hw_clk_enable_rchs();
                return;
        case SYS_CLK_IS_PLL:
                hw_clk_pll_sys_on();
                return;
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Deactivate a System clock.
 *
 * \param[in] clk The clock to deactivate.
 */
__STATIC_INLINE void hw_clk_disable_sysclk(sys_clk_is_t clk)
{
        switch (clk) {
        case SYS_CLK_IS_XTAL32M:
                hw_clk_disable_xtalm();
                return;
        case SYS_CLK_IS_RCHS:
                hw_clk_disable_rchs();
                return;
        case SYS_CLK_IS_PLL:
                hw_clk_pll_sys_off();
                return;
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Check if a System clock is enabled.
 *
 * \return true if the System clock is enabled, else false.
 */
__STATIC_INLINE bool hw_clk_is_enabled_sysclk(sys_clk_is_t clk)
{
        switch (clk) {
        case SYS_CLK_IS_XTAL32M:
                return hw_clk_check_xtalm_status();
        case SYS_CLK_IS_RCHS:
                return hw_clk_check_rchs_status();
        case SYS_CLK_IS_PLL:
                return hw_clk_check_pll_status();
        default:
                /* An invalid clock is requested */
                ASSERT_WARNING(0);
                return false;
        }
}

/**
 * \brief Configure pin to connect an external digital clock.
 */
__STATIC_INLINE void hw_clk_configure_ext32k_pins(void)
{
        GPIO-> P2_09_MODE_REG = 0;
}

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Configure XTAL32M IRQ counter start value.
 *
 * \note At startup, the XTAL32M IRQ counter start value (XTAL32M_IRQ_CNT)
 * must be calculated and stored in the respective register (XTAL32M_IRQ_CTRL_REG).
 * The calculation of the correct start value is based on the IRQ counter captured
 * upon XTAL32M settling (i.e XTAL32M_IRQ_STAT_REG->XTAL32M_IRQ_COUNT_CAP),
 * after trim and preferred settings have been applied.
 * The function stops the XTAL32M, waits 10ms for the oscillation to stop completely
 * and finally re-enables it to achieve a valid IRQ counter calculation.
 * The function leaves the system clock switched to RCHS and XTAL32M settled.
 *
 */

void hw_clk_xtalm_configure_irq(void);

/**
 * \brief Enable XTAL32M interrupt generation.
 *
 * \note When this bit is set the XTAL32M interrupt is generated whenever the
 * oscillator is trimmed and settled, i.e. whenever the oscillator is enabled
 * by the PDC. The interrupt indicates that the oscillator can provide a reliable
 * 32MHz clock.
 */
__STATIC_INLINE void hw_clk_xtalm_irq_enable(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_ENABLE);
        GLOBAL_INT_RESTORE();
}
#endif /* PROCESSOR_BUILD */

#endif /* dg_configUSE_HW_CLK */


#endif /* HW_CLK_DA1470x_H_ */

/**
\}
\}
*/
