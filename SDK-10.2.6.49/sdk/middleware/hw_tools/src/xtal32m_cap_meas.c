/**
 ****************************************************************************************
 *
 * @file xtal32m_configure_startup.c
 *
 * @brief xtal32m_configure_startup source file.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#include <stdio.h>
#include "osal.h"
#include "hw_gpio.h"
#include "hw_pd.h"
#include "sys_power_mgr.h"
#include "sys_clock_mgr.h"
#include "Xtal_TRIM.h"
#ifndef OS_PRESENT
#include "hw_timer.h"
#endif /* OS_PRESENT */

/*
 *  State of XTAL startup
 */
#define XTAL32M_IDLE            0x0
#define XTAL32M_WAIT_LDO        0x1
#define XTAL32M_WAIT_BIAS       0x2
#define XTAL32M_XTAL_DRIVE      0x3
#define XTAL32M_START_BLANK     0x4
#define XTAL32M_START           0x5
#define XTAL32M_SETTLE_BLANK    0x6
#define XTAL32M_SETTLE          0x7
#define XTAL32M_RUN             0x8
#define XTAL32M_CAP_TEST_IDLE   0x9
#define XTAL32M_CAP_TEST_MEAS   0xA
#define XTAL32M_CAP_TEST_END    0xB

typedef struct __xtal32m_caps
{
        double Cs;
        double vlow;
        double vhold;
        double Chold;
        double vp;
        double vn;
        double vsum;
        double CLN;
        double CLP;
}_xtal32m_caps;

uint16_t xtal32m_cap_meas(void);
static _xtal32m_caps xtal32m_caps;
static uint16_t xtal32m_cap_meas_run(void);
static double xtal32m_eq_meas_cap(uint32_t vLow);
static uint32_t xtal32m_meas_cap(void);

#ifdef OS_PRESENT
__RETAINED static OS_TIMER cap_meas_sleep_timer = NULL;
__RETAINED static sleep_mode_t prev_sleep_mode;
static void timer_sleep_cb()
{

}
#else
#include "hw_pdc.h"
#include "hw_watchdog.h"
#define NO_OS_SLEEP_HW_TIMER                    (HW_TIMER2)
#define NO_OS_SLEEP_PDC_ENTRY                   (HW_PDC_PERIPH_TRIG_ID_TIMER2)
#define NO_OS_SLEEP_MS_DURATION                 (100)
static void prepare_no_os_sleep_wakeup(void)
{
        static uint32_t pdc_entry_index;

        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                NO_OS_SLEEP_PDC_ENTRY,
                                                HW_PDC_MASTER_CM33,
                                                0));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);

        pm_set_sys_wakeup_mode(pm_sys_wakeup_mode_fast);

        hw_watchdog_set_pos_val(dg_configWDOG_IDLE_RESET_VALUE);

        timer_config timer_cfg = {
                .clk_src = HW_TIMER_CLK_SRC_INT,
                .prescaler = 15-1,
                .mode = HW_TIMER_MODE_TIMER,
                .timer = { .direction = HW_TIMER_DIR_UP,
                           .reload_val = NO_OS_SLEEP_MS_DURATION - 1,
                           .free_run = false },
                .pwm = {
                        .frequency = 0,
                        .duty_cycle = 0
                },
        };

        hw_timer_init(NO_OS_SLEEP_HW_TIMER, &timer_cfg);
        hw_timer_register_int(NO_OS_SLEEP_HW_TIMER, NULL);
        hw_timer_enable(NO_OS_SLEEP_HW_TIMER);
}

static void stop_no_os_sleep_wakeup(void)
{
        hw_watchdog_freeze();                   // Stop watchdog
        hw_timer_disable(NO_OS_SLEEP_HW_TIMER);
}

#endif /* OS_PRESENT */
uint16_t run_xtal32m_cap_meas(void)
{
        uint16_t status = -XTAL_CALIBRATION_ERROR;

        /*
         *  xtal32m_cap_meas expects to be able to close the XTAL in order to measure it.
         *  Since the XTAL is enabled by the PDC,
         *  before performing the test, the system will be forced to sleep in order to close the XTAL.
         *  All peripherals will be closed and CMAC will remain in sleep.
         *  This way the XTAL will not be needed and will not be enabled on wake-up.
         */

#ifdef OS_PRESENT
        /*
         *  Wait until the system is able to go to sleep.
         */
        cm_wait_lp_clk_ready();

        /*
         *  Wait for the other masters to able to enter sleep.
         */
        hw_pd_wait_power_down_rad();
#endif /* OS_PRESENT */

        /*
         * Change system clock to RCHS_32 before entering sleep.
         * System will not need the XTAL on wake-up,
         * so xtal32m_cap_meas will be able to perform the measurement.
         */
        sys_clk_t prev_clk = cm_sys_clk_get();
        if (prev_clk != sysclk_RCHS_32) {
                cm_sys_clk_set(sysclk_RCHS_32);
        }
        if (cm_sys_clk_get() != sysclk_RCHS_32) {
                status = -XTAL_CALIBRATION_ERROR;
                return status;
        }
#ifdef OS_PRESENT
        /* Set sleep mode */
        prev_sleep_mode = pm_sleep_mode_set(pm_mode_extended_sleep);

        /* Start timer to release M33 from sleep. */
        if (!cap_meas_sleep_timer) {
                cap_meas_sleep_timer = OS_TIMER_CREATE("cap_meas_sleep", 1, OS_TIMER_FAIL, NULL, timer_sleep_cb);
        }
        OS_TIMER_CHANGE_PERIOD(cap_meas_sleep_timer, OS_MS_2_TICKS(100), OS_TIMER_FOREVER);
        OS_TIMER_START(cap_meas_sleep_timer, OS_TIMER_FOREVER);

        /*
         * M33 should wait here longer than the sleep time.
         */
        OS_DELAY(OS_MS_2_TICKS(200));

        /* Set to the previous sleep mode */
        pm_sleep_mode_set(prev_sleep_mode);
#else
        /*
         * Prepare a wake-up before entering sleep.
         */
        prepare_no_os_sleep_wakeup();

        /*
         * Enter sleep state.
         */
        pm_sleep_enter_no_os(pm_mode_extended_sleep);

        hw_clk_delay_usec(10000);

        /*
         * Clear any sleep and wake-up related operation.
         */
        stop_no_os_sleep_wakeup();
#endif /* OS_PRESENT */


        /*
         * Perform cap_meas
         */
        status = xtal32m_cap_meas();

        /*
         * Wait for XTAL block to settle.
         */
        hw_clk_delay_usec(50000);

        /*
         * Restore system clock.
         */
        cm_sys_clk_set_status_t cm_sys_clk_set_status = cm_sys_clk_set(prev_clk);

        /*
         * Based on the operation status, and the clock switch status,
         * enumerate the overall operation status for the host application.
         */
        if (status == -XTAL_OPERATION_SUCCESS) {
                if (cm_sys_clk_set_status == cm_sysclk_success) {
                        status = -XTAL_OPERATION_SUCCESS;
                } else {
                        status = -XTAL_CALIBRATION_ERROR;
                }
        }

        return status;
}

// ********************************************************************************
// ********************************************************************************
// * Configuration startup
// * Measures xtal parameters and obtains suitable values for:
// * XTAL32M_TRIM_REG.XTAL32M_BOOST_TRIM
// ********************************************************************************
// ********************************************************************************
uint16_t xtal32m_cap_meas(void)
{
        double C_sns;
        uint32_t boost_sns, boost_trim;

        /*
         * System clock should not be based on XTAL.
         */
        sys_clk_is_t sys_clk_is = hw_clk_get_sysclk();
        if ((sys_clk_is == SYS_CLK_IS_XTAL32M) || (sys_clk_is == SYS_CLK_IS_PLL)) {
                return (uint16_t) -XTAL_CALIBRATION_ERROR;
        }

        REG_SETF(CRG_XTAL, XTAL32M_CTRL_REG, XTAL32M_ENABLE, 0);
        for (int i= 0; i < 500; i++) {
                hw_clk_delay_usec(100);
        }

        // Preferred settings
        REG_SETF(CRG_XTAL, XTAL32M_FSM_REG, XTAL32M_BOOST_MODE, 1);
        REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_AMPL_SET, 0x1);

        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_ENABLE);

        xtal32m_meas_cap();

        REG_SET_BIT(CRG_XTAL, XTAL32M_IRQ_CTRL_REG, XTAL32M_IRQ_ENABLE);
        GLOBAL_INT_RESTORE();

        boost_sns = RAW_GETF(0x5005042C, 0xFC000000UL);
        if (boost_sns < 32) {
                C_sns = 500e-15 + (boost_sns / 168.0) * 550e-15;
        } else {
                C_sns = 500e-15 - ((64-boost_sns) / 144.0) * 550e-15;
        }

        boost_trim = (4 * xtal32m_caps.Cs/C_sns-1);
        if (boost_trim < 0x04) {
                boost_trim = 0x00;
        } else if (boost_trim == 0x04) {
                boost_trim = 0x05;
        }
        REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_BOOST_TRIM, boost_trim);

        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN, 0);

        return (uint16_t) -XTAL_OPERATION_SUCCESS;
}

static double v_adc;

static uint16_t xtal32m_cap_meas_run(void)
{
        hw_clk_delay_usec(50);

        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_MEAS_START, 1);
        while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_STATE) != XTAL32M_CAP_TEST_END);

        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_START, 1);
        while (REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_START));

        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_MEAS_START, 0);
        while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_STATE) != XTAL32M_CAP_TEST_IDLE);

        return REG_GETF(GPADC, GP_ADC_RESULT_REG, GP_ADC_VAL);
}

static double xtal32m_eq_meas_cap(uint32_t vLow)
{
        const double T_rcosc = 1/32e6;
        double C_eq;
        uint32_t N_meas;
        uint16_t d_adc;
        uint16_t d_adc_prev;
        uint16_t cap_meas_time = 0;

        // Loop finds maximum setting for MEAS_TIME, for which the adc does not overflow
        // Start with lowest sensitivity, keep increasing until overflow, and use the previous (non-overflowed) result
        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_MEAS_TIME, 0);
        d_adc = xtal32m_cap_meas_run();

        while (cap_meas_time < 14) {
                d_adc_prev = d_adc;
                REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_MEAS_TIME, cap_meas_time + 1);
                d_adc = xtal32m_cap_meas_run();
                if (d_adc > ((1<<16) - (1<<12))) {
                        break;
                }
                cap_meas_time += 1;
        }

        v_adc = 0.9 * (double) (d_adc_prev-vLow) / (double) (1<<16);

        N_meas = (32 << cap_meas_time);
        C_eq = (double) N_meas * T_rcosc / v_adc;

        switch (REG_GETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_MEAS_CUR)) {
        case 0:
                C_eq *= 166e-9;
                break;

        case 1:
                C_eq *= 500e-9;
                break;

        case 2:
                C_eq *= 1e-6;
                break;

        case 3:
                C_eq *= 5e-6;
                break;
        }

        return C_eq;
}

// ********************************************************************************
// CAP-MEAS
// measures capacitances around xtal, used to determine the correct startup settings
// ********************************************************************************
static uint32_t xtal32m_meas_cap(void)
{
        uint32_t vLow;
        double Chold;
        double CapP;
        double CapN;
        double xtal32m_capsum;

        // Configure ADC
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SE, 1);
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN, 1);

        RAW_SETF(0x5002080C, 0x80UL, 0x1);

        REG_SETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_P, 1); // xtal cap-test mode
        REG_SETF(GPADC, GP_ADC_OFFP_REG, GP_ADC_OFFP, 0x2A0); // provide some OFFSET such that the low voltage can be measured.

        RAW_SETF(0x5005041C, 0x20000UL, 0x0);

        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_CAP_SELECT, 5); // measure LOW reference on XTAL_P
        while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_STATE) != XTAL32M_CAP_TEST_IDLE);

        RAW_SETF(0x5005041C, 0x200000UL, 0x1);

        // ***************** Get VLOW
        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_MEAS_START, 1);
        while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_STATE) != XTAL32M_CAP_TEST_END);

        hw_clk_delay_usec(50);

        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_START, 1);
        while (REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_START));

        vLow = REG_GETF(GPADC, GP_ADC_RESULT_REG, GP_ADC_VAL);
        xtal32m_caps.vlow = 0.9 * (double) REG_GETF(GPADC, GP_ADC_RESULT_REG, GP_ADC_VAL) / (1<<16);

        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_MEAS_START, 0);
        while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_STATE) != XTAL32M_CAP_TEST_IDLE);

        // ***************** Get Chold
        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_CAP_SELECT, 1); // measure HOLD capacitance.
        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_MEAS_CUR, 0); // 100nA current - smaller cap /w buffer enabled.

        Chold = xtal32m_eq_meas_cap(vLow);
        xtal32m_caps.vhold = v_adc;
        xtal32m_caps.Chold = Chold;

        // ***************** Get CapP = CL0 + Cs
        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_CAP_SELECT, 2); // measure XTAL_P capacitance.
        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_MEAS_CUR, 2); // 1uA current
        CapP = xtal32m_eq_meas_cap(vLow) - Chold;
        xtal32m_caps.vp = v_adc;

        // ***************** Get CapN = CL1 + Cs
        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_CAP_SELECT, 3); // measure XTAL_N capacitance.
        CapN = xtal32m_eq_meas_cap(vLow) - Chold;
        xtal32m_caps.vn = v_adc;

        // ***************** Get Csum = CL0 + CL1
        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_CAP_SELECT, 4); // measure XTAL_N capacitance.
        xtal32m_capsum = xtal32m_eq_meas_cap(vLow) - Chold;
        xtal32m_caps.vsum = v_adc;

        REG_SETF(CRG_XTAL, XTAL32M_CAP_MEAS_REG, XTAL32M_CAP_SELECT, 0); // goto idle
        while (REG_GETF(CRG_XTAL, XTAL32M_STAT0_REG, XTAL32M_STATE) != XTAL32M_IDLE);

        xtal32m_caps.Cs = (CapP + CapN - xtal32m_capsum) / 2.0;
        xtal32m_caps.CLN = CapN - xtal32m_caps.Cs;
        xtal32m_caps.CLP = CapP - xtal32m_caps.Cs;

        RAW_SETF(0x5005041C, 0x20000UL, 0x1);

        return 0;
}
