/**
 ****************************************************************************************
 *
 * @file sys_clock_mgr_da1470x.c
 *
 * @brief Clock Manager
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_CLOCK_MGR

#include "sdk_defs.h"
#include "hw_otpc.h"
#include "sys_adc.h"
#include "sys_clock_mgr.h"
#include "sys_clock_mgr_internal.h"
#include "sys_power_mgr.h"
#include "sys_power_mgr_internal.h"
#include "qspi_automode.h"
#include "oqspi_automode.h"
#include "hw_lcdc.h"
#include "hw_pdc.h"
#include "hw_pd.h"
#include "hw_pmu.h"
#include "hw_usb.h"
#include "hw_gpio.h"
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
#include "../sys_man/sys_rcx_calibrate_internal.h"
#endif
#if (dg_configRTC_CORRECTION == 1)
#include "hw_rtc.h"
#endif
#if dg_configUSE_HW_RTC
#include "hw_rtc.h"
#endif
#if (dg_configPMU_ADAPTER == 1)
#include "../adapters/src/ad_pmu_internal.h"
#endif
#ifdef OS_PRESENT
#include "../sys_man/sys_timer_internal.h"
#endif

#if (dg_configSYSTEMVIEW)
#include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#define SEGGER_SYSTEMVIEW_ISR_ENTER()
#define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#ifdef OS_PRESENT
#include "osal.h"
#include "sdk_list.h"

#include "hw_qspi.h"

#if (dg_configUSE_BOD == 1)
#include "hw_bod.h"
#endif
//#include "sys_tcs.h"

#if (CLK_MGR_USE_TIMING_DEBUG == 1)
#pragma message "Clock manager: GPIO Debugging is on!"
#endif

#ifdef CONFIG_USE_BLE
#include "ad_ble.h"
#endif

#define XTAL32_AVAILABLE                1       // XTAL32M availability
#define LP_CLK_AVAILABLE                2       // LP clock availability
#define PLL_AVAILABLE                   4       // PLL locked
#define PLL_USB_AVAILABLE               8       // USB PLL48 locked
#endif /* OS_PRESENT */

#define RCX_MIN_HZ                      450
#define RCX_MAX_HZ                      550
/* RCX frequency range varies between 13kHz and 17kHz. RCX_MIN/MAX_TICK_CYCLES correspond to the number of
 * min and max RCX cycles respectively in a 2msec duration, which is the optimum OS tick. */
#define RCX_MIN_TICK_CYCLES             26
#define RCX_MAX_TICK_CYCLES             34

/* ~3 msec for the 1st calibration. This is the maximum allowed value when the 96MHz clock is
 * used. It can be increased when the sys_clk has lower frequency (i.e. multiplied by 2 for 48MHz,
 * 3 for 32MHz). The bigger it is, the longer it takes to complete the power-up
 * sequence. */
#define RCX_CALIBRATION_CYCLES_PUP      44

/* Total calibration time = N*3 msec. Increase N to get a better estimation of the frequency of
 * RCX. */
#define RCX_REPEAT_CALIBRATION_PUP      10

/* Bit field to trigger the RCX Calibration task to start calibration. */
#define RCX_DO_CALIBRATION              (1 << 1)

/* Bit field to trigger the RCHS Calibration task to start calibration. */
#define RCHS_DO_CALIBRATION             (1 << 0)

#define RCHS_CALIBRATION_CYCLES         384     // number of RCHS measurement cycles
#define BAND_TRIM_LOW_LIMIT             50      // RCHS_INIT_DEL low limit
#define BAND_TRIM_HIGH_LIMIT            200     // RCHS_INIT_DEL high limit

/*
 * Global and / or retained variables
 */

__RETAINED static uint16_t rcx_clock_hz;
__RETAINED static uint8_t rcx_tick_period;                        // # of cycles in 1 tick
__RETAINED static uint16_t rcx_tick_rate_hz;

__RETAINED static uint32_t rcx_clock_hz_acc;               // Accurate RCX freq (1/RCX_ACCURACY_LEVEL accuracy)
__RETAINED static uint32_t rcx_clock_period;               // usec multiplied by 1024 * 1024

static const uint64_t rcx_period_dividend = 1048576000000;             // 1024 * 1024 * 1000000;

#if (dg_configRTC_CORRECTION == 1)

/*
 * RTC compensation variables
 */
#define DAY_IN_USEC                     (24 * 60 * 60 * 1000 * 1000LL)
#define HDAY_IN_USEC                    (12 * 60 * 60 * 1000 * 1000LL)
#define HUNDREDTHS_OF_SEC_us            10000

__RETAINED static uint32_t rcx_freq_prev;
__RETAINED static uint64_t rtc_usec_prev;
__RETAINED static int32_t rtc_usec_correction;
__RETAINED static uint32_t initial_rcx_clock_hz_acc;
#endif

__RETAINED_RW static sys_clk_t sysclk = sysclk_LP;      // Invalidate system clock
__RETAINED_RW static sys_clk_t sysclk_booter = sysclk_LP;
__RETAINED static ahb_div_t ahbclk;
__RETAINED static apb_div_t apbclk;
__RETAINED static apb_div_t apb_slowclk;

#define CM_SYS_CLK_REQUEST_MAX          (UINT8_MAX)
#define CM_SYS_CLK_NUM                  (5)
__RETAINED static sys_clk_t sys_clk_prio[CM_SYS_CLK_NUM];
__RETAINED static uint8_t sys_clk_cnt[CM_SYS_CLK_NUM];
__RETAINED static bool sys_clk_cnt_ind; // if true, cm_sys_clk_request() should be used (instead of cm_sys_clk_set())
__RETAINED static uint8_t default_sys_clk_index;
#if (dg_configPMU_ADAPTER == 0)
__RETAINED static HW_PMU_1V2_VOLTAGE vdd_voltage;
#endif /* dg_configPMU_ADAPTER */
__RETAINED static uint8_t pll_count;
#if dg_configUSE_HW_PDC
__RETAINED static uint32_t xtal32_pdc_entry;
#endif /* dg_configUSE_HW_PDC */

__RETAINED static void (*xtal_ready_callback)(void);

static sys_clk_t sys_clk_next;
static ahb_div_t ahb_clk_next;

#ifdef OS_PRESENT
static volatile bool xtal32m_settled_notification = false;
#endif
static volatile bool xtal32m_settled = false;
static volatile bool pll_locked = false;
static volatile bool pll_usb_locked = false;

#if dg_configENABLE_RCHS_CALIBRATION
__RETAINED static OS_TASK xRCClocksCalibTaskHandle;
#endif

#ifdef OS_PRESENT
__RETAINED static OS_MUTEX cm_mutex;
__RETAINED static OS_EVENT_GROUP xEventGroupCM_xtal;
__RETAINED static OS_TIMER xLPSettleTimer;

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
__RETAINED static OS_TASK xRCXCalibTaskHandle;
#endif

typedef struct clk_mgr_task_list_elem_t clk_mgr_task_list_elem_t;
struct clk_mgr_task_list_elem_t {
        clk_mgr_task_list_elem_t *next;
        OS_TASK task;
        uint8_t task_pll_count;
};

__RETAINED static void* clk_mgr_task_list;

#endif /* OS_PRESENT */

/*
 * Forward declarations
 */
static cm_sys_clk_set_status_t sys_clk_set(sys_clk_t type);
static void apb_set_clock_divider(apb_div_t div);
static void apb_slow_set_clock_divider(apb_div_t div);
static bool ahb_set_clock_divider(ahb_div_t div);
static void cm_wait_pll_usb_lock(void);
static uint8_t index_find_in_prio_list(sys_clk_t type);

#ifdef OS_PRESENT
#define CM_ENTER_CRITICAL_SECTION() OS_ENTER_CRITICAL_SECTION()
#define CM_LEAVE_CRITICAL_SECTION() OS_LEAVE_CRITICAL_SECTION()

#define CM_MUTEX_CREATE()       OS_ASSERT(cm_mutex == NULL); \
                                OS_MUTEX_CREATE(cm_mutex); \
                                OS_ASSERT(cm_mutex)
#define CM_MUTEX_GET()          OS_ASSERT(cm_mutex); \
                                OS_MUTEX_GET(cm_mutex, OS_MUTEX_FOREVER)
#define CM_MUTEX_PUT()          OS_MUTEX_PUT(cm_mutex)

#else
#define CM_ENTER_CRITICAL_SECTION() GLOBAL_INT_DISABLE()
#define CM_LEAVE_CRITICAL_SECTION() GLOBAL_INT_RESTORE()

#define CM_MUTEX_CREATE()
#define CM_MUTEX_GET()
#define CM_MUTEX_PUT()

#endif /* OS_PRESENT */

/*
 * Function definitions
 */

/**
 * \brief Get the CPU clock frequency in MHz
 *
 * \param[in] clk The system clock
 * \param[in] div The HCLK divider
 *
 * \return The clock frequency
 */
__RETAINED_CODE static uint32_t get_clk_freq(sys_clk_t clk, ahb_div_t div)
{
        sys_clk_t clock = clk;

        if (clock == sysclk_RCHS_32) {
                clock = sysclk_XTAL32M;
        }

        return ( 16 >> div ) * clock;
}

/**
 * \brief Adjust OTP access timings according to the AHB clock frequency.
 *
 * \warning In mirrored mode, the OTP access timings are left unchanged since the system is put to
 *          sleep using the RC32M clock and the AHB divider set to 1, which are the same settings
 *          that the system runs after a power-up or wake-up!
 */
static __RETAINED_CODE void adjust_otp_access_timings(void)
{
#if (dg_configUSE_HW_OTPC == 1)
        if (hw_otpc_is_active()) {
                uint32_t clk_freq = get_clk_freq(sys_clk_next, ahb_clk_next);
                HW_OTPC_SYS_CLK_FREQ freq = hw_otpc_convert_sys_clk_mhz(clk_freq);
                ASSERT_ERROR(freq != HW_OTPC_SYS_CLK_FREQ_INVALID_VALUE);
                hw_otpc_set_speed(freq);
        }
#endif
}

/**
 * \brief Lower AHB and APB clocks to the minimum frequency.
 *
 * \warning It can be called only at wake-up.
 */
__STATIC_INLINE void lower_amba_clocks(void)
{
        // Lower the AHB clock (fast --> slow clock switch)
        hw_clk_set_hclk_div((uint32_t)ahb_div16);
        adjust_otp_access_timings();
}

/**
 * \brief Restore AHB and APB clocks to the maximum (default) frequency.
 *
 * \warning It can be called only at wake-up.
 */
__STATIC_INLINE void restore_amba_clocks(void)
{
        // Restore the AHB clock (slow --> fast clock switch)
        adjust_otp_access_timings();
        hw_clk_set_hclk_div(ahbclk);
}

static void memories_sys_clock_cfg(sys_clk_t clk)
{
        adjust_otp_access_timings();

#if (dg_configUSE_HW_OQSPI == 1)
        oqspi_automode_sys_clock_cfg(clk);
#endif

#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)
        qspi_automode_sys_clock_cfg(clk);
#endif
}

/**
 * \brief Switch to RCHS.
 *
 * \details Set RCHS as the system clock.
 */
static void switch_to_rchs(rchs_speed_t mode)
{
        hw_clk_enable_sysclk(SYS_CLK_IS_RCHS);

        sys_clk_t clk = (mode == RCHS_96) ? sysclk_RCHS_96 :
                        (mode == RCHS_32) ? sysclk_RCHS_32 : sysclk_RCHS_64;

        /* When switching from lower to higher system clock frequency, the memories and their
           controllers must be reconfigured before clock switching  */
        if (sysclk <= clk) {
                memories_sys_clock_cfg(clk);
        }

        hw_clk_set_rchs_mode(mode);
        hw_clk_set_sysclk(SYS_CLK_IS_RCHS);     // Set RCHS as sys_clk

        /* When switching from higher to lower system clock frequency, the memories and their
           controllers must be reconfigured after clock switching  */
        if (sysclk > clk) {
                memories_sys_clock_cfg(clk);
        }

        /*
         * Disable RCHS. RCHS will remain enabled by the hardware as long as it is used
         * as system clock.
         */
        hw_clk_disable_sysclk(SYS_CLK_IS_RCHS);
}

/**
 * \brief Switch to XTAL32M.
 *
 * \details Sets the XTAL32M as the system clock.
 *
 * \warning It does not block. It assumes that the caller has made sure that the XTAL32M has
 *          settled.
 */
static void switch_to_xtal32m(void)
{
        if (hw_clk_get_sysclk() != SYS_CLK_IS_XTAL32M) {
                ASSERT_WARNING(hw_clk_is_xtalm_started());

                hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);          // Set XTAL32 as sys_clk
                if (sysclk > sysclk_XTAL32M) {                 // fast --> slow clock switch
                        memories_sys_clock_cfg(sysclk_XTAL32M);
                }
        }
}
/**
 * \brief Set 1V2 max voltage level
 *
 */
static void pmu_1v2_set_max_voltage(void)
{
#if (dg_configPMU_ADAPTER == 1)
        ad_pmu_1v2_force_max_voltage_request();
#else
        HW_PMU_1V2_RAIL_CONFIG rail_config;
        hw_pmu_get_1v2_active_config(&rail_config);

        vdd_voltage = rail_config.voltage;
        if (vdd_voltage != HW_PMU_1V2_VOLTAGE_1V20) {
                // VDD voltage must be set to 1.2V prior to switching clock to PLL
                HW_PMU_ERROR_CODE error_code;
#if (dg_configUSE_BOD == 1)
                hw_bod_deactivate_channel(BOD_CHANNEL_VDD);
#endif
                error_code = hw_pmu_1v2_set_voltage(HW_PMU_1V2_VOLTAGE_1V20);
#if (dg_configUSE_BOD == 1)
                /* Wait 20us for bandgap to ramp up reference */
                hw_clk_delay_usec(20);
                /* Rail has been configured. Enable BOD on VDD.  */
                hw_bod_activate_channel(BOD_CHANNEL_VDD);
#endif
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }
#endif /* dg_configPMU_ADAPTER */
}

/**
 * \brief Restore 1V2 voltage level
 *
 */
static void pmu_1v2_restore_voltage(void)
{
#if (dg_configPMU_ADAPTER == 1)
        ad_pmu_1v2_force_max_voltage_release();
#else
        if (vdd_voltage != HW_PMU_1V2_VOLTAGE_1V20) {
                HW_PMU_ERROR_CODE error_code;
                error_code = hw_pmu_1v2_set_voltage(vdd_voltage);
                ASSERT_WARNING(error_code == HW_PMU_ERROR_NOERROR);
        }
#endif /* dg_configPMU_ADAPTER */
}
/**
 * \brief Disable PLL
 *
 * \details Restore VDD voltage to 0.9V if required.
 */
static void disable_pll(void)
{
        if (hw_clk_is_enabled_sysclk(SYS_CLK_IS_PLL)) {
                hw_clk_disable_sysclk(SYS_CLK_IS_PLL);

                // VDD voltage can be lowered since PLL is not the system clock anymore
                pmu_1v2_restore_voltage();
                pll_locked = false;
                DBG_SET_LOW(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_PLL_ON);
        }
}

/**
 * \brief Enable PLL
 *
 * \details Changes the VDD voltage to 1.2V if required.
 */
static void enable_pll(void)
{
        if (hw_clk_is_pll_locked()) {
                pll_locked = true;
        }
        else if (hw_clk_is_enabled_sysclk(SYS_CLK_IS_PLL) == false) {
                ASSERT_WARNING(!pll_locked);

                pmu_1v2_set_max_voltage();
                hw_clk_enable_sysclk(SYS_CLK_IS_PLL);           // Turn on PLL
                DBG_SET_HIGH(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_PLL_ON);
        }
}

/**
 * \brief Switch to PLL.
 *
 * \details XTAl32M needs to be the running system clock in order to switch to PLL.
 */
static void switch_to_pll(void)
{
#if dg_configUSE_HW_USB && dg_configUSE_USB_ENUMERATION
        /* USB device should not be in use*/
        ASSERT_WARNING(!hw_usb_active());
#endif /* dg_configUSE_HW_USB */

        if (hw_clk_get_sysclk() == SYS_CLK_IS_XTAL32M) {
                memories_sys_clock_cfg(sysclk_PLL160);
                /*
                 * If ultra-fast wake-up mode is used, make sure that the startup state
                 * machine is finished and all power regulation is in order.
                 */
                while (REG_GETF(CRG_TOP, SYS_STAT_REG, POWER_IS_UP) == 0);

                /*
                 * Core voltage may have been changed from 0.9V to 1.2V.
                 * Wait for VDD to settle in order to switch system clock to PLL.
                 */
                while ((REG_GETF(CRG_TOP, ANA_STATUS_REG, BUCK_DCDC_V12_OK) == 0));
                hw_clk_set_sysclk(SYS_CLK_IS_PLL);                   // Set PLL as sys_clk
        }
}

#ifdef OS_PRESENT
#if dg_configUSE_HW_RTC
/* this function configures the RTC clock and RTC_KEEP_RTC_REG*/
static void reconfigure_rtc(void)
{
        uint16_t div_int;
        uint16_t div_frac;

        div_int = lp_clock_hz / 100;
        div_frac = 10 * (lp_clock_hz - (div_int * 100));

        hw_rtc_clk_config(RTC_DIV_DENOM_1000, div_int, div_frac);
}
#endif
/**
 * \brief The handler of the XTAL32K LP settling timer.
 */
static void vLPTimerCallback(OS_TIMER pxTimer)
{
        OS_ENTER_CRITICAL_SECTION();                            // Critical section
        if ((dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG) &&
                ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768))) {
                hw_clk_set_lpclk(LP_CLK_IS_XTAL32K);            // Set XTAL32K as the LP clock
                sys_timer_set_timer_vars(LP_configSYSTICK_CLOCK_HZ, LP_configTICK_RATE_HZ, LP_TICK_PERIOD);
                hw_clk_disable_lpclk(LP_CLK_IS_RCX);            // Disable RCX
#if dg_configUSE_HW_RTC
                reconfigure_rtc();
#endif
        }

#ifdef CONFIG_USE_BLE
        // Inform ble adapter about the availability of the LP clock.
        ad_ble_lpclock_available();
#endif

        OS_LEAVE_CRITICAL_SECTION();                            // Exit critical section

        // Inform (blocked) Tasks about the availability of the LP clock.
        OS_EVENT_GROUP_SET_BITS(xEventGroupCM_xtal, LP_CLK_AVAILABLE);

        // Stop the Timer.
        OS_TIMER_STOP(xLPSettleTimer, OS_TIMER_FOREVER);
}

/**
 * \brief Handle the indication that the XTAL32M has settled.
 *
 */
__RETAINED_HOT_CODE static OS_BASE_TYPE xtal32m_is_ready(OS_BASE_TYPE *xHigherPriorityTaskWoken)
{
        OS_BASE_TYPE xResult = OS_FAIL;

        if (xtal32m_settled_notification == false) {
                // Do not send the notification twice
                xtal32m_settled_notification = true;

                DBG_SET_HIGH(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_XTAL32M_SETTLED);
                if (xtal_ready_callback) {
                        xtal_ready_callback();
                }

                if (xEventGroupCM_xtal != NULL) {
                        // Inform blocked Tasks
                        *xHigherPriorityTaskWoken = OS_FALSE;           // Must be initialized to OS_FALSE

                        xResult = OS_EVENT_GROUP_SET_BITS_FROM_ISR_NO_YIELD(
                                        xEventGroupCM_xtal, XTAL32_AVAILABLE,
                                        xHigherPriorityTaskWoken);
                }

                DBG_SET_LOW(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_XTAL32M_SETTLED);
        }
        return xResult;
}

/**
 * \brief Handle the indication that the PLL is locked and therefore available.
 */
static OS_BASE_TYPE pll_is_locked(OS_BASE_TYPE *xHigherPriorityTaskWoken)
{
        OS_BASE_TYPE xResult = OS_FAIL;

        if (xEventGroupCM_xtal != NULL) {
                *xHigherPriorityTaskWoken = OS_FALSE;           // Must be initialized to OS_FALSE

                xResult = OS_EVENT_GROUP_SET_BITS_FROM_ISR_NO_YIELD(
                                xEventGroupCM_xtal, PLL_AVAILABLE,
                                xHigherPriorityTaskWoken);
        }

        return xResult;
}

/**
 * \brief Handle the indication that the USB PLL48 is locked and therefore available.
 */
static OS_BASE_TYPE pll_usb_is_locked(OS_BASE_TYPE *xHigherPriorityTaskWoken)
{
        OS_BASE_TYPE xResult = OS_FAIL;

        if (xEventGroupCM_xtal != NULL) {
                *xHigherPriorityTaskWoken = OS_FALSE; // Must be initialized to OS_FALSE

                xResult = OS_EVENT_GROUP_SET_BITS_FROM_ISR_NO_YIELD(
                                xEventGroupCM_xtal, PLL_USB_AVAILABLE,
                                xHigherPriorityTaskWoken);
        }

        return xResult;
}

#endif /* OS_PRESENT */

/**
 * \brief Calculates the optimum tick rate and the number of LP cycles (RCX) per tick.
 *
 * \param[in] freq The RCX clock frequency (in Hz).
 * \param[out] tick_period The number of LP cycles per tick.
 *
 * \return uint32_t The optimum tick rate.
 */
static uint32_t get_optimum_tick_rate(uint16_t freq, uint8_t *tick_period)
{
        uint32_t optimum_rate = 0;
        int err = 65536;

        for (int tick = RCX_MIN_TICK_CYCLES; tick <= RCX_MAX_TICK_CYCLES; tick++) {
                uint32_t hz = 2 * freq / tick;
                hz = (hz & 1) ? hz / 2 + 1 : hz / 2;

                if ((hz >= RCX_MIN_HZ) && (hz <= RCX_MAX_HZ)) {
                        int res = hz * tick * 65536 / freq;
                        res -= 65536;
                        if (res < 0) {
                                res *= -1;
                        }
                        if (res < err) {
                                err = res;
                                optimum_rate = hz;
                                *tick_period = tick;
                        }
                }
        }

        return optimum_rate;
}

__RETAINED_HOT_CODE void cm_enable_xtalm_if_required(void)
{
        if ((sysclk == sysclk_XTAL32M) || (sysclk == sysclk_PLL160)) {
                cm_enable_xtalm();
        }
}

__RETAINED_HOT_CODE uint32_t cm_get_xtalm_settling_lpcycles(void)
{
        if ((sysclk == sysclk_RCHS_32) || (sysclk == sysclk_RCHS_64) || (sysclk == sysclk_RCHS_96)) {
                return 0;
        }

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        return XTALRDY_CYCLES_TO_LP_CLK_CYCLES(hw_clk_get_xtalm_settling_time(), rcx_clock_hz);
#else
        return XTALRDY_CYCLES_TO_LP_CLK_CYCLES(hw_clk_get_xtalm_settling_time(), dg_configXTAL32K_FREQ);
#endif
}

#if dg_configUSE_HW_PDC

static uint32_t get_pdc_xtal32m_entry(void)
{

        uint32_t entry = HW_PDC_INVALID_LUT_INDEX;

#ifdef OS_PRESENT
        // Search for the RTOS timer entry
        entry = hw_pdc_find_entry(HW_PDC_TRIG_SELECT_PERIPHERAL, HW_PDC_PERIPH_TRIG_ID_TIMER2,
                                        HW_PDC_MASTER_CM33, HW_PDC_LUT_ENTRY_EN_XTAL, 0);
        if (entry != HW_PDC_INVALID_LUT_INDEX) {
                return entry;
        }

#endif
        // Search for any entry that will wake-up M33 and start the XTAL32M
        entry = hw_pdc_find_entry(HW_PDC_FILTER_DONT_CARE, HW_PDC_FILTER_DONT_CARE,
                                        HW_PDC_MASTER_CM33, HW_PDC_LUT_ENTRY_EN_XTAL, 0);
        return entry;
}
#endif

__RETAINED_HOT_CODE void cm_enable_xtalm(void)
{
        GLOBAL_INT_DISABLE();

#if dg_configUSE_HW_PDC
        if (xtal32_pdc_entry == HW_PDC_INVALID_LUT_INDEX) {
                // Find a PDC entry for enabling XTAL32M
                xtal32_pdc_entry = get_pdc_xtal32m_entry();

                if (xtal32_pdc_entry == HW_PDC_INVALID_LUT_INDEX) {
                        // If no PDC entry exists, add a new entry for enabling the XTAL32M
                        xtal32_pdc_entry = hw_pdc_add_entry( HW_PDC_TRIGGER_FROM_MASTER( HW_PDC_MASTER_CM33,
                                                                                  HW_PDC_LUT_ENTRY_EN_XTAL ) );
                }

                ASSERT_WARNING(xtal32_pdc_entry != HW_PDC_INVALID_LUT_INDEX);

                // XTAL32M may not been started. Use PDC to start it.
                hw_pdc_set_pending(xtal32_pdc_entry);
                hw_pdc_acknowledge(xtal32_pdc_entry);

                // Clear the XTAL32M_XTAL_ENABLE bit to allow PDC to disable XTAL32M when going to sleep.
                hw_clk_disable_sysclk(SYS_CLK_IS_XTAL32M);
        }
#endif

        xtal32m_settled = hw_clk_is_xtalm_started();

        if (xtal32m_settled == false) {
                if (hw_clk_is_enabled_sysclk(SYS_CLK_IS_XTAL32M) == false) {
#if dg_configUSE_HW_PDC
                        // XTAL32M has not been started. Use PDC to start it.
                        hw_pdc_set_pending(xtal32_pdc_entry);
                        hw_pdc_acknowledge(xtal32_pdc_entry);

#else
                        // PDC is not used. Enable XTAL32M by setting XTAL32M_XTAL_ENABLE bit
                        // in XTAL32M_CTRL1_REG
                        hw_clk_enable_sysclk(SYS_CLK_IS_XTAL32M);
#endif
                }
        }

        GLOBAL_INT_RESTORE();
}

#if (MAIN_PROCESSOR_BUILD)
void cm_sysclk_init_low_level_internal(void)
{
        NVIC_ClearPendingIRQ(XTAL32M_RDY_IRQn);
        NVIC_EnableIRQ(XTAL32M_RDY_IRQn);                      // Activate XTAL32 Ready IRQ

        NVIC_ClearPendingIRQ(PLL_LOCK_IRQn);
        NVIC_EnableIRQ(PLL_LOCK_IRQn);                         // Activate PLL160M Lock IRQ

        NVIC_ClearPendingIRQ(PLL48_LOCK_IRQn);
        NVIC_EnableIRQ(PLL48_LOCK_IRQn);                       // Activate PLL48M Lock IRQ

        hw_clk_xtalm_irq_enable();

        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC != 0) {
                uint16_t rdy_cnt = XTAL32M_USEC_TO_250K_CYCLES(dg_configXTAL32M_SETTLE_TIME_IN_USEC);
                hw_clk_set_xtalm_settling_time(rdy_cnt, true);
        }

#if dg_configUSE_HW_PDC
        xtal32_pdc_entry = HW_PDC_INVALID_LUT_INDEX;
#endif
}

void cm_lpclk_init_low_level_internal(void)
{
        if (dg_configLP_CLK_SOURCE == LP_CLK_IS_DIGITAL) {
                hw_clk_configure_ext32k_pins();                 // Configure Ext32K pins
                hw_gpio_pad_latch_enable(HW_GPIO_PORT_2,HW_GPIO_PIN_9);
                hw_gpio_pad_latch_disable(HW_GPIO_PORT_2,HW_GPIO_PIN_9);
                hw_clk_disable_lpclk(LP_CLK_IS_XTAL32K);        // Disable XTAL32K
                hw_clk_disable_lpclk(LP_CLK_IS_RCX);            // Disable RCX
#ifdef OS_PRESENT
                sys_timer_set_timer_vars(LP_configSYSTICK_CLOCK_HZ, LP_configTICK_RATE_HZ, LP_TICK_PERIOD);
#endif
                hw_clk_set_lpclk(LP_CLK_IS_EXTERNAL);           // Set EXTERNAL as the LP clock
        } else { // LP_CLK_IS_ANALOG
                hw_clk_enable_lpclk(LP_CLK_IS_RCX);     // Enable RCX
                cm_rcx_calibrate();
                hw_clk_set_lpclk(LP_CLK_IS_RCX);        // Set RCX as the LP clock
                if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                        hw_clk_disable_lpclk(LP_CLK_IS_XTAL32K);        // Disable XTAL32K
                } else if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                        // No need to configure XTAL32K pins. Pins are automatically configured
                        // when LP_CLK_IS_XTAL32K is enabled.
                        hw_clk_configure_lpclk(LP_CLK_IS_XTAL32K);      // Configure XTAL32K
                        hw_clk_enable_lpclk(LP_CLK_IS_XTAL32K);         // Enable XTAL32K
                        // LP clock cannot be set to XTAL32K here. XTAL32K needs a few seconds to settle after power up.
                } else {
                        ASSERT_WARNING(0);                              // Should not be here!
                }
        }
}
#endif /* MAIN_PROCESSOR_BUILD */

void cm_rcx_calibrate(void)
{
        // Run a dummy calibration to make sure the clock has settled
        hw_clk_start_calibration(CALIBRATE_RCX, CALIBRATE_REF_DIVN, 25);
        hw_clk_get_calibration_data();

        // Run actual calibration
        uint32_t hz_value = 0;
        uint32_t cal_value;
        uint64_t max_clk_count;

        for (int i = 0; i < RCX_REPEAT_CALIBRATION_PUP; i++) {
                hw_clk_start_calibration(CALIBRATE_RCX, CALIBRATE_REF_DIVN, RCX_CALIBRATION_CYCLES_PUP);
                cal_value = hw_clk_get_calibration_data();

                // Process calibration results
                max_clk_count = (uint64_t)dg_configXTAL32M_FREQ * RCX_CALIBRATION_CYCLES_PUP * RCX_ACCURACY_LEVEL;
                hz_value += (uint32_t)(max_clk_count / cal_value);
        }

        rcx_clock_hz_acc = (hz_value + (RCX_REPEAT_CALIBRATION_PUP / 2)) / RCX_REPEAT_CALIBRATION_PUP;
        rcx_clock_hz = rcx_clock_hz_acc / RCX_ACCURACY_LEVEL;
        rcx_clock_period = (uint32_t)((rcx_period_dividend * RCX_ACCURACY_LEVEL) / rcx_clock_hz_acc);
        rcx_tick_rate_hz = get_optimum_tick_rate(rcx_clock_hz, &rcx_tick_period);
#if (dg_configRTC_CORRECTION == 1)
        rcx_freq_prev = rcx_clock_hz_acc;
        initial_rcx_clock_hz_acc = rcx_clock_hz_acc;
#endif

#ifdef OS_PRESENT
#ifdef CONFIG_USE_SNC
        /* publish LP clock variables to SNC */
        sys_timer_share_timer_vars();
#endif
        sys_timer_set_timer_vars(rcx_clock_hz, rcx_tick_rate_hz, rcx_tick_period);
#endif
}

uint32_t cm_get_rcx_clock_hz_acc(void)
{
        return rcx_clock_hz_acc;
}

uint32_t cm_get_rcx_clock_period(void)
{
        return rcx_clock_period;
}

static uint32_t calibrate_rchs(void)
{
        /* Run calibration process */
        hw_clk_start_calibration(CALIBRATE_RCHS, CALIBRATE_REF_DIVN, RCHS_CALIBRATION_CYCLES);
        uint32_t cal_value = hw_clk_get_calibration_data();
        uint64_t max_clk_count = (uint64_t)dg_configXTAL32M_FREQ * RCHS_CALIBRATION_CYCLES;
        return (uint32_t)(max_clk_count / cal_value);
}

static uint32_t get_freq_sns(void)
{
        uint32_t m_range_1_del_low, m_range_1_del_high;

        REG_SETF(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_DEL, BAND_TRIM_LOW_LIMIT);
        /* Run calibration process */
        m_range_1_del_low = calibrate_rchs();

        REG_SETF(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_DEL, BAND_TRIM_HIGH_LIMIT);

        m_range_1_del_high = calibrate_rchs();

        return ((m_range_1_del_low - m_range_1_del_high) / (BAND_TRIM_HIGH_LIMIT - BAND_TRIM_LOW_LIMIT));
}

static void trim_rchs(rchs_speed_t mode, uint32_t freq_target)
{
        uint8_t rchs_init_range;
        int32_t trim_step = 127;
        int32_t trim;

        hw_clk_set_rchs_mode(mode);

        rchs_init_range = 2;
        REG_SETF(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_RANGE, rchs_init_range);
        uint32_t freq_sns = get_freq_sns();
        trim = 127;
        REG_SETF(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_DEL, trim);

        /* Run calibration process */
        uint32_t freq = calibrate_rchs();

        trim_step = ((int32_t)freq - (int32_t)freq_target) / (int32_t)freq_sns;

        while (trim_step != 0) {

                trim += trim_step;

                if (trim > 255) {
                        rchs_init_range = REG_GETF(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_RANGE) + 1;
                        REG_SETF(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_RANGE, rchs_init_range);
                        trim = 127;
                        freq_sns = get_freq_sns();
                }

                if (trim < 0) {
                        if (rchs_init_range > 0) {
                                rchs_init_range = REG_GETF(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_RANGE) - 1;
                        }
                        REG_SETF(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_RANGE, rchs_init_range);
                        trim = 127;
                        freq_sns = get_freq_sns();
                }

                REG_SETF(CRG_TOP, CLK_RCHS_REG, RCHS_INIT_DEL, trim);

                freq = calibrate_rchs();

                trim_step = ((int32_t)freq - (int32_t)freq_target) / (int32_t)freq_sns;
        }
}

void cm_rchs_calibrate(void)
{
        rchs_speed_t rchs_mode = hw_clk_get_rchs_mode();

        trim_rchs(RCHS_96, dg_configRCHS_96M_FREQ);
        hw_clk_store_rchs_32_96_mode_trim_value(CRG_TOP->CLK_RCHS_REG & RCHS_REG_TRIM);
        trim_rchs(RCHS_64, dg_configRCHS_64M_FREQ);
        hw_clk_store_rchs_64_mode_trim_value(CRG_TOP->CLK_RCHS_REG & RCHS_REG_TRIM);

        hw_clk_set_rchs_mode(rchs_mode);

}

#if dg_configENABLE_RCHS_CALIBRATION
void cm_rchs_calibration_notify(void)
{
        OS_TASK_NOTIFY(xRCClocksCalibTaskHandle, RCHS_DO_CALIBRATION, OS_NOTIFY_SET_BITS);
}
/**
 * \brief RC clocks calibration Task function.
 *
 * \param [in] pvParameters ignored.
 */
static OS_TASK_FUNCTION(rc_clocks_calibration_task, pvParameters)
{
        uint32_t ulNotifiedValue;

        while (1) {
                // Wait for the internal notifications.
                OS_BASE_TYPE xResult = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &ulNotifiedValue,
                                                                        OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(xResult == OS_OK);

                if (ulNotifiedValue & RCHS_DO_CALIBRATION) {

                        CM_MUTEX_GET();

                        sys_clk_t sysclk_cur = cm_sys_clk_get();
                        bool rchs_is_sys_clk = (sysclk_cur == sysclk_RCHS_32) ||
                                               (sysclk_cur == sysclk_RCHS_64) ||
                                               (sysclk_cur == sysclk_RCHS_96);
                        bool cal_is_allowed = true;

                        if (rchs_is_sys_clk) {
                                if (sys_clk_cnt_ind) {
                                        cal_is_allowed = (cm_sys_clk_request(sysclk_XTAL32M) != cm_sysclk_div1_clk_in_use);
                                } else {
                                        cal_is_allowed = (cm_sys_clk_set(sysclk_XTAL32M) != cm_sysclk_div1_clk_in_use);
                                }
                        }

                        if (cal_is_allowed) {
                                pmu_1v2_set_max_voltage();
                                cm_rchs_calibrate();
                                pmu_1v2_restore_voltage();
                        }

                        if (rchs_is_sys_clk) {
                                if (sys_clk_cnt_ind) {
                                        cm_sys_clk_release(sysclk_XTAL32M);
                                } else {
                                        if (cal_is_allowed) {
                                                cm_sys_clk_set(sysclk_cur);
                                        }
                                }
                        }
                        CM_MUTEX_PUT();
                }
        }
}

void cm_rc_clocks_calibration_task_init(void)
{
        OS_BASE_TYPE status;

#if (dg_configRF_ENABLE_RECALIBRATION == 0)
        // if radio calibration is disabled, enable sys_adc
        // else sys_adc will be enabled by the ble adapter
        sys_adc_init();
        sys_adc_enable();
#endif
        // Create the RC clocks calibration task
        status = OS_TASK_CREATE("RC_clocks_cal",
                                rc_clocks_calibration_task,
                                ( void * ) NULL,
                                OS_MINIMAL_TASK_STACK_SIZE,
                                OS_TASK_PRIORITY_LOWEST,
                                xRCClocksCalibTaskHandle);
        OS_ASSERT(status == OS_OK);
}
#endif /* dg_configENABLE_RCHS_CALIBRATION */

void cm_sys_clk_init(sys_clk_t type)
{
        CM_MUTEX_CREATE();                                  // Create Mutex. Called only once!
#ifdef OS_PRESENT
        xEventGroupCM_xtal = OS_EVENT_GROUP_CREATE();       // Create Event Group
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);
#endif /* OS_PRESENT */
        ahbclk = cm_ahb_get_clock_divider();
        apbclk = cm_apb_get_clock_divider();
        apb_slowclk = cm_apb_slow_get_clock_divider();

        ahb_clk_next = ahbclk;

        ASSERT_WARNING(type != sysclk_LP);                  // Not Applicable!

        HW_PMU_1V2_RAIL_CONFIG rail_config;
        hw_pmu_get_1v2_active_config(&rail_config);
#if (dg_configPMU_ADAPTER == 0)
        vdd_voltage = rail_config.voltage;
#endif /* dg_configPMU_ADAPTER */

        /*
         * Disable RCHS. RCHS will remain enabled by the hardware as long as it is used
         * as system clock.
         */
        hw_clk_disable_sysclk(SYS_CLK_IS_RCHS);

        CM_ENTER_CRITICAL_SECTION();

        sysclk = sysclk_booter = cm_sys_clk_get();

        if (type == sysclk_BOOTER) {
                sys_clk_next = sysclk_booter;
        } else {
                sys_clk_next = type;
                if ((sys_clk_next == sysclk_RCHS_32) || (sys_clk_next == sysclk_RCHS_64) || (sys_clk_next == sysclk_RCHS_96)) {
                        rchs_speed_t rchs_mode;
                        switch (sys_clk_next) {
                        case sysclk_RCHS_32:
                                rchs_mode = RCHS_32;
                                break;
                        case sysclk_RCHS_64:
                                pmu_1v2_set_max_voltage();
                                rchs_mode = RCHS_64;
                                break;
                        case sysclk_RCHS_96:
                                pmu_1v2_set_max_voltage();
                                rchs_mode = RCHS_96;
                                break;
                        default:
                                rchs_mode = RCHS_32;
                                ASSERT_WARNING(0);
                        }
                        switch_to_rchs(rchs_mode);
                } else {
                        cm_enable_xtalm();

                        /*
                         * Note: In case the XTAL32M (or PLL) has not settled (or locked) yet, then we
                         *       simply set the cm_sysclk to the user setting and skip waiting for the
                         *       XTAL32M to settle. In this case, the system clock will be set to the
                         *       XTAL32M (or the PLL) when the XTAL32M_RDY_IRQn hits. Every task or Adapter
                         *       must block until the requested system clock is available. Sleep may have to
                         *       be blocked as well.
                         */
                        if (cm_poll_xtalm_ready()) {
                                switch_to_xtal32m();

                                hw_clk_disable_sysclk(SYS_CLK_IS_RCHS);

                                if (sys_clk_next == sysclk_PLL160) {
                                        if (hw_clk_is_pll_locked()) {
                                                switch_to_pll();
                                        }
                                        else {
                                                // System clock will be switched to PLL when PLL is locked
                                                enable_pll();
                                        }
                                }
                                else {
                                        disable_pll();
#ifdef OS_PRESENT
                                        OS_EVENT_GROUP_CLEAR_BITS(xEventGroupCM_xtal, PLL_AVAILABLE);
#endif
                                }
                        }
                }
        }
        sysclk = sys_clk_next;

        CM_MUTEX_GET();
        if (sys_clk_cnt_ind) {
                default_sys_clk_index = index_find_in_prio_list(sysclk);
        } else {
                pll_count = (sys_clk_next == sysclk_PLL160) ? 1 : 0;
        }
        CM_MUTEX_PUT();

        CM_LEAVE_CRITICAL_SECTION();
}

static void cm_sys_enable_xtalm(sys_clk_t type)
{
        if ((type == sysclk_XTAL32M) || (type == sysclk_PLL160)) {
                cm_enable_xtalm();

                // Make sure the XTAL32M has settled
                cm_wait_xtalm_ready();
        }
}

static void sys_enable_pll(void)
{
        enable_pll();
        cm_wait_pll_lock();
}

void cm_sys_enable_pll_usb(void)
{
        CM_MUTEX_GET();

        cm_enable_xtalm();
        // Make sure the XTAL32M has settled
        cm_wait_xtalm_ready();

        pmu_1v2_set_max_voltage();
        while (REG_GETF(CRG_TOP, POWER_LVL_REG , V12_LEVEL) != 2);

        hw_clk_pll_usb_on();
        cm_wait_pll_usb_lock();

        CM_MUTEX_PUT();
}

void cm_sys_disable_pll_usb(void)
{
        CM_MUTEX_GET();

#ifdef OS_PRESENT
        OS_EVENT_GROUP_CLEAR_BITS(xEventGroupCM_xtal, PLL_USB_AVAILABLE);
#endif
        hw_clk_pll_usb_off();
        pll_usb_locked = false;

        pmu_1v2_restore_voltage();

        CM_MUTEX_PUT();
}

#ifdef OS_PRESENT
bool sys_clk_mgr_match_task(const void *elem, const void *ud)
{
        return ((clk_mgr_task_list_elem_t*)elem)->task == ud;
}
#endif /* OS_PRESENT */

static uint8_t index_find_in_prio_list(sys_clk_t type)
{
        uint8_t index;
        for (index = 0; index < CM_SYS_CLK_NUM; index++) {
                if (type == sys_clk_prio[index]) {
                        return index;
                }
        }
        ASSERT_WARNING(0);
        return 0;
}

static cm_sys_clk_set_status_t cm_sys_clk_update(void)
{
        uint8_t clk_next_index = 0;

        for (clk_next_index = 0; clk_next_index < CM_SYS_CLK_NUM; clk_next_index++) {
                if (sys_clk_cnt[clk_next_index] > 0) {
                        break;
                }
        }

        if (clk_next_index == CM_SYS_CLK_NUM) {
                clk_next_index = default_sys_clk_index;
        }

        if ((sys_clk_prio[clk_next_index] == sysclk_XTAL32M) || (sys_clk_prio[clk_next_index] == sysclk_PLL160)) {
                cm_sys_enable_xtalm(sys_clk_prio[clk_next_index]);
        }
        if (sys_clk_prio[clk_next_index] == sysclk_PLL160) {
                sys_enable_pll();
        }

        cm_sys_clk_set_status_t ret = sys_clk_set(sys_clk_prio[clk_next_index]);

        if (sysclk != sysclk_PLL160) {
                disable_pll();
#ifdef OS_PRESENT
                OS_EVENT_GROUP_CLEAR_BITS(xEventGroupCM_xtal, PLL_AVAILABLE);
#endif
        }

        return ret;

}
void cm_sys_clk_set_priority(sys_clk_t *sys_clk_prio_array)
{
        ASSERT_WARNING(sys_clk_cnt_ind == false);
#if dg_configENABLE_RCHS_CALIBRATION
        ASSERT_WARNING((sys_clk_prio_array[0] == sysclk_XTAL32M) ||
                ((sys_clk_prio_array[0] == sysclk_PLL160) && (sys_clk_prio_array[1] == sysclk_XTAL32M)));
#endif
        memcpy(sys_clk_prio, sys_clk_prio_array, sizeof(sys_clk_prio));
        sys_clk_cnt_ind = true;
}

cm_sys_clk_set_status_t cm_sys_clk_request(sys_clk_t type)
{
        ASSERT_WARNING(sys_clk_cnt_ind == true);

        ASSERT_WARNING(type < sysclk_LP);      // Not Applicable!
        if (type == sysclk_BOOTER) {
                type = sysclk_booter;
        }

        uint8_t clk = index_find_in_prio_list(type);

        ASSERT_ERROR(sys_clk_cnt[clk] < CM_SYS_CLK_REQUEST_MAX);

        CM_MUTEX_GET();

        sys_clk_cnt[clk]++;
        cm_sys_clk_set_status_t ret = cm_sys_clk_update();

        if ((ret == cm_sysclk_success) && sysclk != type) {
                ret = cm_sysclk_higher_prio_used;
        }
        CM_MUTEX_PUT();

        return ret;
}

cm_sys_clk_set_status_t cm_sys_clk_release(sys_clk_t type)
{
        ASSERT_WARNING(sys_clk_cnt_ind == true);

        ASSERT_WARNING(type < sysclk_LP);      // Not Applicable!
        if (type == sysclk_BOOTER) {
                type = sysclk_booter;
        }

        uint8_t clk = index_find_in_prio_list(type);
        ASSERT_ERROR(sys_clk_cnt[clk] != 0);

        CM_MUTEX_GET();

        sys_clk_cnt[clk]--;
        cm_sys_clk_set_status_t ret = cm_sys_clk_update();

        CM_MUTEX_PUT();

        return ret;
}

cm_sys_clk_set_status_t cm_sys_clk_set(sys_clk_t type)
{
        cm_sys_clk_set_status_t ret;

        /* cm_sys_clk_request/release are used
         * cm_sys_clk_set can not be used in parallel*/
        ASSERT_WARNING(sys_clk_cnt_ind == false);

        ASSERT_WARNING(type != sysclk_LP);                      // Not Applicable!

        if (type == sysclk_BOOTER) {
                type = sysclk_booter;
        }
#ifdef OS_PRESENT
        clk_mgr_task_list_elem_t *elem;
        OS_TASK task = OS_GET_CURRENT_TASK();
#endif /* OS_PRESENT */

        CM_MUTEX_GET();

        if (type != sysclk_PLL160) {
                if (pll_count > 1) {
#ifdef OS_PRESENT
                        /* Check if the current task is in the list */
                        elem = list_find(&clk_mgr_task_list, sys_clk_mgr_match_task, task);
                        if (elem) {
                                elem->task_pll_count--;
                                if (elem->task_pll_count < 1) {
                                        /* Remove the task and decrease global pll_count */
                                        list_unlink(&clk_mgr_task_list, sys_clk_mgr_match_task,
                                                    task);
                                        OS_FREE(elem);
                                        pll_count--;
                                }
                        }
#else
                        pll_count--;
#endif /* OS_PRESENT */
                        CM_MUTEX_PUT();
                        return cm_sysclk_pll_used_by_task;
                }
#ifdef OS_PRESENT
                if (pll_count == 1) {
                        elem = list_find(&clk_mgr_task_list, sys_clk_mgr_match_task, task);
                        if (elem == NULL) {
                                // If this is not the task that has requested PLL
                                CM_MUTEX_PUT();
                                return cm_sysclk_pll_used_by_task;
                        } else if (elem->task_pll_count > 1) {
                                elem->task_pll_count--;
                                CM_MUTEX_PUT();
                                return cm_sysclk_pll_used_by_task;
                        }

                }
#endif /* OS_PRESENT */
        }

        cm_sys_enable_xtalm(type);

        if (type == sysclk_PLL160) {
                sys_enable_pll();
        }

        ret = sys_clk_set(type);

        if (ret == cm_sysclk_success) {
                if (type == sysclk_PLL160) {
#ifdef OS_PRESENT
                        elem = list_find(&clk_mgr_task_list, sys_clk_mgr_match_task, task);
                        if (elem == NULL) {
                                // Add the current task in the list
                                elem = OS_MALLOC(sizeof(clk_mgr_task_list_elem_t));
                                OS_ASSERT(elem);
                                elem->task = task;
                                elem->task_pll_count = 1;
                                list_add(&clk_mgr_task_list, elem);
                                pll_count++;
                        } else {
                                elem->task_pll_count++;
                        }
#else
                        pll_count++;
#endif /* OS_PRESENT */
                } else if (pll_count > 0) {
                        ASSERT_WARNING(pll_count == 1);
#ifdef OS_PRESENT
                        /* The current task must be is in the list. */
                        elem = list_find(&clk_mgr_task_list, sys_clk_mgr_match_task, task);
                        OS_ASSERT(elem);
                        ASSERT_WARNING(elem->task_pll_count == 1);
                        /* Remove the task element and decrease global pll counter */
                        elem = list_unlink(&clk_mgr_task_list, sys_clk_mgr_match_task,
                                                                         task);
                        OS_FREE(elem);
                        pll_count--;
#else
                        pll_count--;
#endif /* OS_PRESENT */
                }

        }

        if (sysclk != sysclk_PLL160) {
                disable_pll();
#ifdef OS_PRESENT
                OS_EVENT_GROUP_CLEAR_BITS(xEventGroupCM_xtal, PLL_AVAILABLE);
#endif
        }
        CM_MUTEX_PUT();

        return ret;
}

#define CHECK_PER_DIV1_CLK(val, per) ((val & REG_MSK(CRG_SNC, CLK_SNC_REG, per ## _ENABLE)) && \
                                      (val & REG_MSK(CRG_SNC, CLK_SNC_REG, per ## _CLK_SEL)))

/**
 * \brief Check if div1 clock is used by a peripheral
 *
 * \return true if div1 is used by a peripheral
 */
static bool sys_clk_check_div1(void)
{
        uint32_t tmp;

        // Check if SysTick is ON and if it is affected
        if (dg_configABORT_IF_SYSTICK_CLK_ERR) {
                if (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) {
                        return true;
                }
        }

        // Check if peripherals are clocked by DIV1 clock

        // Check SPI3 clock
        tmp = CRG_SYS->CLK_SYS_REG;
        if ((tmp & REG_MSK(CRG_SYS, CLK_SYS_REG, SPI3_ENABLE)) &&
                        (tmp & REG_MSK(CRG_SYS, CLK_SYS_REG, SPI3_CLK_SEL))) {
                return true;
        }

        // Check if eMMC is enabled
        if (tmp & REG_MSK(CRG_CTRL, CLK_PDCTRL_REG, EMMC_ENABLE)) {
                return true;
        }

        if (hw_pd_check_snc_status()) {
                tmp = CRG_SNC->CLK_SNC_REG;

                // Check SPI clock
                if (CHECK_PER_DIV1_CLK(tmp, SPI)) {
                        return true;
                }

                // Check SPI2 clock
                if (CHECK_PER_DIV1_CLK(tmp, SPI2)) {
                        return true;
                }

                // Check I2C clock
                if (CHECK_PER_DIV1_CLK(tmp, I2C)) {
                        return true;
                }

                // Check I2C2 clock
                if (CHECK_PER_DIV1_CLK(tmp, I2C2)) {
                        return true;
                }

                // Check I2C3 clock
                if (CHECK_PER_DIV1_CLK(tmp, I2C3)) {
                        return true;
                }

                // Check I3C clock
                if (CHECK_PER_DIV1_CLK(tmp, I3C)) {
                        return true;
                }

                // Check UART clock
                if (CHECK_PER_DIV1_CLK(tmp, UART)) {
                        return true;
                }

                // Check UART2 clock
                if (CHECK_PER_DIV1_CLK(tmp, UART2)) {
                        return true;
                }

                // Check UART3 clock
                if (CHECK_PER_DIV1_CLK(tmp, UART3)) {
                        return true;
                }
        }

        if (hw_pd_check_aud_status()) {
                // Check PCM clock
                tmp = CRG_AUD->PCM_DIV_REG;
                if ((tmp & REG_MSK(CRG_AUD, PCM_DIV_REG, CLK_PCM_EN)) &&
                                (tmp & REG_MSK(CRG_AUD, PCM_DIV_REG, PCM_SRC_SEL))) {
                        return true;
                }
        }

#if (dg_configUSE_HW_LCDC == 1)
        // Check LCD controller
        if (hw_lcdc_clk_is_div1()) {
                return true;
        }
#endif
        return false;
}

static cm_sys_clk_set_status_t sys_clk_set(sys_clk_t type)
{
        cm_sys_clk_set_status_t ret;

        CM_ENTER_CRITICAL_SECTION();

        if (type != sysclk && sys_clk_check_div1()) {
                ret = cm_sysclk_div1_clk_in_use;
        } else {
                ret = cm_sysclk_success;

                if (type != sysclk) {
                        sys_clk_next = type;
                        ahb_clk_next = ahbclk;

                        switch (sys_clk_next) {
                        case sysclk_PLL160:
                                if (sysclk != sysclk_XTAL32M) {
                                        // Transition from RCHS to PLL is not allowed.
                                        // Switch to XTAL32M first.
                                        switch_to_xtal32m();
                                        if (sysclk >= sysclk_RCHS_64) {
                                                // Restore RCHS frequency to 32MHz
                                                hw_clk_set_rchs_mode(RCHS_32);

                                                // Restore V12 voltage here. It will be set to 1.2V again when PLL160 is enabled.
                                                pmu_1v2_restore_voltage();
                                        }
                                }
                                switch_to_pll();
                                break;
                        case sysclk_RCHS_32:
                                if (sysclk == sysclk_PLL160) {
                                        // Transition from PLL to RCHS is not allowed.
                                        // Switch to XTAL32M first.
                                        switch_to_xtal32m();
                                }
                                switch_to_rchs(RCHS_32);
                                if ((sysclk > sysclk_XTAL32M) && (sysclk < sysclk_PLL160)) {
                                        pmu_1v2_restore_voltage();
                                }
                                break;
                        case sysclk_RCHS_64:
                        case sysclk_RCHS_96:
                                if (sysclk == sysclk_PLL160) {
                                        // Transition from PLL to RCHS is not allowed.
                                        // Switch to XTAL32M first.
                                        switch_to_xtal32m();
                                }
                                if (sysclk <= sysclk_XTAL32M || sysclk == sysclk_PLL160) {
                                        // When sysclk is PLL160 call pmu_1v2_set_max_voltage() to increase the counter.
                                        // pmu_1v2_restore_voltage() will be called when PLL160 is disabled.
                                        pmu_1v2_set_max_voltage();
                                }
                                switch_to_rchs(sys_clk_next == sysclk_RCHS_64 ? RCHS_64 : RCHS_96);
                                break;
                        case sysclk_XTAL32M:
                                switch_to_xtal32m();
                                if ((sysclk > sysclk_XTAL32M) && (sysclk < sysclk_PLL160)) {
                                        // Restore RCHS frequency to 32MHz.
                                        // Do this first to allow pmu_1v2_restore_voltage() to operate properly.
                                        hw_clk_set_rchs_mode(RCHS_32);
                                        pmu_1v2_restore_voltage();
                                }
                                break;
                        default:
                                ASSERT_WARNING(0);
                                break;
                        }

                        sysclk = sys_clk_next;
                }
        }

        CM_LEAVE_CRITICAL_SECTION();

        return ret;
}

void cm_apb_slow_set_clock_divider(apb_div_t div)
{
        CM_MUTEX_GET();
        apb_slow_set_clock_divider(div);
        CM_MUTEX_PUT();
}

static void apb_slow_set_clock_divider(apb_div_t div)
{
        hw_clk_set_pclk_slow_div(div);
        apb_slowclk = div;
}

void cm_apb_set_clock_divider(apb_div_t div)
{
        CM_MUTEX_GET();
        apb_set_clock_divider(div);
        CM_MUTEX_PUT();
}

static void apb_set_clock_divider(apb_div_t div)
{
        hw_clk_set_pclk_div(div);
        apbclk = div;
}

bool cm_ahb_set_clock_divider(ahb_div_t div)
{
        CM_MUTEX_GET();
        bool ret = ahb_set_clock_divider(div);
        CM_MUTEX_PUT();

        return ret;
}

static bool ahb_set_clock_divider(ahb_div_t div)
{
        bool ret = true;

        CM_ENTER_CRITICAL_SECTION();

        do {
                if (ahbclk == div) {
                        break;
                }

                // Check if SysTick is ON and if it is affected
                if (dg_configABORT_IF_SYSTICK_CLK_ERR) {
                        if (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk) {
                                ret = false;
                                break;
                        }
                }

                ahb_clk_next = div;

                if (ahbclk < div) {
                        // fast --> slow clock switch
                        hw_clk_set_hclk_div(div);
                        adjust_otp_access_timings();         // Adjust OTP timings
                } else {
                        // slow --> fast clock switch
                        adjust_otp_access_timings();         // Adjust OTP timings
                        hw_clk_set_hclk_div(div);
                }

                ahbclk = div;

        } while (0);

        CM_LEAVE_CRITICAL_SECTION();

        return ret;
}

/**
 * \brief Checks whether a system clock switch is possible based on
 *        clock requests (sys_clk_cnt) and priority (sys_clk_prio)
 *
 * \return true if the switch is possible, false otherwise
 */
static bool sys_clk_switch(sys_clk_t clk, sys_clk_t clk_next)
{
        uint8_t clk_index = index_find_in_prio_list(clk);
        uint8_t clk_next_index = index_find_in_prio_list(clk_next);

        if (clk_next_index > clk_index) {
                if (sys_clk_cnt[clk_index] > 0) {
                        return false;
                }
        }
        return true;
}

bool cm_cpu_clk_set(cpu_clk_t clk)
{
        sys_clk_t new_sysclk;
        sys_clk_t old_sysclk;
        ahb_div_t new_ahbclk;
        uint8_t ahb_z;
        bool ret = false;

        CM_MUTEX_GET();

        old_sysclk = sysclk;
        new_ahbclk = ahb_div1;

        switch (clk) {
        case cpuclk_10M:
        case cpuclk_20M:
        case cpuclk_40M:
        case cpuclk_80M:
        case cpuclk_160M:
                new_sysclk = sysclk_PLL160;
                ahb_z = 24;
                break;
        case cpuclk_6M:
        case cpuclk_12M:
        case cpuclk_24M:
        case cpuclk_48M:
        case cpuclk_96M:
                if (pll_count > 0) {
                        CM_MUTEX_PUT();
                        return false;
                }
                new_sysclk = sysclk_RCHS_96;
                ahb_z = 25;
                break;
        case cpuclk_64M:
                if (pll_count > 0) {
                        CM_MUTEX_PUT();
                        return false;
                }
                new_sysclk = sysclk_RCHS_64;
                ahb_z = 25;
                break;
        case cpuclk_4M:
        case cpuclk_8M:
        case cpuclk_16M:
        case cpuclk_32M:
                if (pll_count > 0) {
                        CM_MUTEX_PUT();
                        return false;
                }
                new_sysclk = ((sysclk == sysclk_RCHS_32) || (sysclk == sysclk_RCHS_96)) ? sysclk_RCHS_32 :
                        (sysclk == sysclk_RCHS_64) ? sysclk_RCHS_64 : sysclk_XTAL32M;
                ahb_z = ((new_sysclk == sysclk_RCHS_32) || (new_sysclk == sysclk_XTAL32M)) ? 26 : 25;
                break;
        case cpuclk_2M:
                if (pll_count > 0) {
                        CM_MUTEX_PUT();
                        return false;
                }
                new_sysclk = ((sysclk == sysclk_RCHS_32) || (sysclk == sysclk_RCHS_64) || (sysclk == sysclk_RCHS_96))
                        ? sysclk_RCHS_32 : sysclk_XTAL32M;
                ahb_z = 26;
                break;
        default:
                CM_MUTEX_PUT();
                return false;
        }

        if (sys_clk_cnt_ind && !sys_clk_switch(old_sysclk, new_sysclk)) {
                CM_MUTEX_PUT();
                return false;
        }

        new_ahbclk = (ahb_div_t)(__CLZ((uint32_t)clk) - ahb_z);

        cm_sys_enable_xtalm(new_sysclk);

        if (new_sysclk == sysclk_PLL160) {
                sys_enable_pll();
        }


        if (sys_clk_set(new_sysclk) == cm_sysclk_success) {
                ret = ahb_set_clock_divider(new_ahbclk);

                if (ret == false) {
                        ASSERT_WARNING(old_sysclk != sysclk_LP);   // Not Applicable!
                        cm_sys_enable_xtalm(old_sysclk);
                        sys_clk_set(old_sysclk);                   // Restore previous setting
                }
        }

        if (sysclk != sysclk_PLL160) {
                disable_pll();
#ifdef OS_PRESENT
                OS_EVENT_GROUP_CLEAR_BITS(xEventGroupCM_xtal, PLL_AVAILABLE);
#endif
        }
        CM_MUTEX_PUT();

        return ret;
}

void cm_cpu_clk_set_fromISR(sys_clk_t clk, ahb_div_t hdiv)
{
        ASSERT_WARNING(clk != sysclk_LP);               // Not Applicable!

        sysclk = clk;
        ahbclk = hdiv;
        cm_sys_clk_sleep(false);                        // Pretend an XTAL32M settled event
}

sys_clk_t cm_sys_clk_get(void)
{
        sys_clk_t clk;

        CM_MUTEX_GET();
        CM_ENTER_CRITICAL_SECTION();

        clk = cm_sys_clk_get_fromISR();

        CM_LEAVE_CRITICAL_SECTION();
        CM_MUTEX_PUT();

        return clk;
}

__RETAINED_HOT_CODE sys_clk_t cm_sys_clk_get_fromISR(void)
{
        return hw_clk_get_system_clock();
}

apb_div_t cm_apb_slow_get_clock_divider(void)
{
        CM_MUTEX_GET();
        apb_div_t clk = (apb_div_t)hw_clk_get_pclk_slow_div();
        CM_MUTEX_PUT();

        return clk;
}

apb_div_t cm_apb_get_clock_divider(void)
{
        CM_MUTEX_GET();
        apb_div_t clk = (apb_div_t)hw_clk_get_pclk_div();
        CM_MUTEX_PUT();

        return clk;
}

ahb_div_t cm_ahb_get_clock_divider(void)
{
        ahb_div_t clk;

        CM_MUTEX_GET();
        CM_ENTER_CRITICAL_SECTION();                            // Critical section

        clk = (ahb_div_t)hw_clk_get_hclk_div();

        CM_LEAVE_CRITICAL_SECTION();                            // Exit critical section
        CM_MUTEX_PUT();
        return clk;
}

cpu_clk_t cm_cpu_clk_get(void)
{
        sys_clk_t curr_sysclk = cm_sys_clk_get();
        ahb_div_t curr_ahbclk = cm_ahb_get_clock_divider();

        return (cpu_clk_t)get_clk_freq(curr_sysclk, curr_ahbclk);
}

#ifdef OS_PRESENT

__RETAINED_HOT_CODE  cpu_clk_t cm_cpu_clk_get_fromISR(void)
{
        sys_clk_t curr_sysclk = cm_sys_clk_get_fromISR();
        ahb_div_t curr_ahbclk = hw_clk_get_hclk_div();

        return (cpu_clk_t)get_clk_freq(curr_sysclk, curr_ahbclk);
}
#endif /* OS_PRESENT */

/**
 * \brief Interrupt handler of the XTAL32M_RDY_IRQn.
 *
 */
void XTAL32M_Ready_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        DBG_SET_HIGH(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_XTAL32M_ISR);

        DBG_SET_HIGH(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_XTAL32M_READY);
        while (!hw_clk_is_xtalm_started());
        DBG_SET_LOW(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_XTAL32M_READY);
        xtal32m_settled = true;

#ifdef OS_PRESENT
        if (xEventGroupCM_xtal != NULL) {
                OS_BASE_TYPE xHigherPriorityTaskWoken, xResult;

                xResult = xtal32m_is_ready(&xHigherPriorityTaskWoken);

                if (xResult != OS_FAIL) {
                        /*
                         * If xHigherPriorityTaskWoken is now set to OS_TRUE then a context
                         * switch should be requested.
                         */
                        if (xHigherPriorityTaskWoken != OS_FALSE) {
                                OS_TASK_YIELD_FROM_ISR();
                        }
                }
        }
#endif /* OS_PRESENT */

        if (sysclk == sysclk_XTAL32M || sysclk == sysclk_PLL160) {
                // Restore system clocks.
                cm_sys_clk_sleep(false);
        }

        DBG_SET_LOW(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_XTAL32M_ISR);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

/**
 * \brief Interrupt handler of the PLL_LOCK_IRQn.
 *
 */
void PLL_Lock_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        if (!hw_clk_is_pll_locked()) {
                SEGGER_SYSTEMVIEW_ISR_EXIT();
                return;
        }

        pll_locked = true;

        if (sys_clk_next == sysclk_PLL160) {
                switch_to_pll();
        }

#ifdef OS_PRESENT
        if (xEventGroupCM_xtal != NULL) {
                OS_BASE_TYPE xHigherPriorityTaskWoken, xResult;

                xResult = pll_is_locked(&xHigherPriorityTaskWoken);

                if (xResult != OS_FAIL) {
                        /* If xHigherPriorityTaskWoken is now set to OS_TRUE then a context
                         * switch should be requested. */
                        if (xHigherPriorityTaskWoken != OS_FALSE) {
                                OS_TASK_YIELD_FROM_ISR();
                        }
                }
        }
#endif /* OS_PRESENT */
        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

/**
 * \brief Interrupt handler of the PLL_LOCK_IRQn.
 *
 */
void PLL48_Lock_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        if (!hw_clk_is_pll_usb_locked()) {
                SEGGER_SYSTEMVIEW_ISR_EXIT();
                return;
        }

        pll_usb_locked = true;

#ifdef OS_PRESENT
        if (xEventGroupCM_xtal != NULL) {
                OS_BASE_TYPE xHigherPriorityTaskWoken, xResult;

                xResult = pll_usb_is_locked(&xHigherPriorityTaskWoken);

                if (xResult != OS_FAIL) {
                        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
                         * switch should be requested. */
                        OS_TASK_YIELD_FROM_ISR();
                }
        }
#endif
        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void cm_wait_xtalm_ready(void)
{
#ifdef OS_PRESENT
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        if (!xtal32m_settled) {
                // Do not go to sleep while waiting for XTAL32M to settle.
                pm_sleep_mode_request(pm_mode_idle);
                OS_EVENT_GROUP_WAIT_BITS(xEventGroupCM_xtal,
                                XTAL32_AVAILABLE,
                                OS_EVENT_GROUP_FAIL,            // Don't clear bit after ret
                                OS_EVENT_GROUP_OK,              // Wait for all bits
                                OS_EVENT_GROUP_FOREVER);        // Block forever

                /* If we get here, XTAL32 must have settled */
                ASSERT_WARNING(xtal32m_settled == true);
                pm_sleep_mode_release(pm_mode_idle);
        }
#else
        cm_halt_until_xtalm_ready();
#endif /* OS_PRESENT */
}

void cm_wait_pll_lock(void)
{
#ifdef OS_PRESENT
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        if (!pll_locked) {
                // Do not go to sleep while waiting for PLL to lock.
                pm_sleep_mode_request(pm_mode_idle);
                OS_EVENT_GROUP_WAIT_BITS(xEventGroupCM_xtal,
                                PLL_AVAILABLE,
                                OS_EVENT_GROUP_FAIL,            // Don't clear bit after ret
                                OS_EVENT_GROUP_OK,              // Wait for all bits
                                OS_EVENT_GROUP_FOREVER);        // Block forever

                /* If we get here, PLL must be locked */
                ASSERT_WARNING(pll_locked == true);
                pm_sleep_mode_release(pm_mode_idle);
        }
#else
        cm_halt_until_pll_locked();
#endif /* OS_PRESENT */
}

static void cm_wait_pll_usb_lock(void)
{
#ifdef OS_PRESENT
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        if (!pll_usb_locked) {
                // Do not go to sleep while waiting for PLL to lock.
                pm_sleep_mode_request(pm_mode_idle);
                OS_EVENT_GROUP_WAIT_BITS(xEventGroupCM_xtal,
                                PLL_USB_AVAILABLE,
                                OS_EVENT_GROUP_FAIL,            // Don't clear bit after ret
                                OS_EVENT_GROUP_OK,              // Wait for all bits
                                OS_EVENT_GROUP_FOREVER);        // Block forever

                /* If we get here, PLL must be locked */
                ASSERT_WARNING(pll_usb_locked == true);
                pm_sleep_mode_release(pm_mode_idle);
        }
#else
        cm_halt_until_pll_usb_locked();
#endif
}

__RETAINED_HOT_CODE void cm_halt_until_sysclk_ready(void)
{
        if ((sysclk == sysclk_XTAL32M) || (sysclk == sysclk_PLL160)) {
                cm_halt_until_xtalm_ready();
        }

        if (sysclk == sysclk_PLL160) {
                cm_halt_until_pll_locked();
        }
}

#ifdef OS_PRESENT

void cm_calibrate_rc32k(void)
{
}

uint32_t cm_rcx_us_2_lpcycles(uint32_t usec)
{
        /* Can only convert up to 4095 usec */
        ASSERT_WARNING(usec < 4096);

        return ((usec << 20) / rcx_clock_period) + 1;
}

uint32_t cm_rcx_us_2_lpcycles_low_acc(uint32_t usec)
{
        return ((1 << 20) / (rcx_clock_period / usec)) + 1;
}

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
#if (dg_configRTC_CORRECTION == 1)

extern void hw_rtc_register_cb(void (*cb)(const hw_rtc_time_t *time));

static void cm_rtc_callback(const hw_rtc_time_t *time)
{
        rtc_usec_prev = ((((time->hour * 60 + time->minute) * 60) + time->sec) * 1000 + time->hsec *10)*1000LL;
        if (time->hour_mode && time->pm_flag) {
                rtc_usec_prev += HDAY_IN_USEC;
        }
        rtc_usec_correction = 0;
        rcx_freq_prev = initial_rcx_clock_hz_acc;
}
/**
 * \brief Apply compensation value to RTC.
 *
 * \p This function takes as input the new hundredths-of-seconds value.
 *
 * \param [in] new_hos value for the field hundredths-of-seconds of RTC.
 *
 * \note This function deals only with hundredths of seconds, nothing bigger than that.
 *
 */
static void cm_apply_rtc_compensation_hos(uint8_t new_hos)
{
        hw_rtc_time_stop();
        uint32_t reg = RTC->RTC_TIME_REG;
        REG_SET_FIELD(RTC, RTC_TIME_REG, RTC_TIME_H_U,reg, (new_hos % 10) );
        REG_SET_FIELD(RTC, RTC_TIME_REG, RTC_TIME_H_T,reg, (new_hos / 10) );
        RTC->RTC_TIME_REG = reg;
        hw_rtc_time_start();
}

/**
 * \brief Calculate RTC's compensation value and apply it, if desired.
 *
 * \p This function needs as input the latest calculated freq rcx_clock_hz and the initial one initial_rcx_clock_hz_acc.
 *
 * \note This function compensates up to hundredths of seconds.
 * \warning Must be called with interrupts disabled.
 *
 */
static void cm_calculate_rtc_compensation_value(void)
{
        hw_rtc_time_t current_time;
        uint32_t usec_delta_i, usec_delta_r, mean_rcx_clock_hz_acc;
        uint64_t usec;
        int32_t delta_slp_time;

        uint8_t num_of_hundredths, rtc_time_hundredths, new_rtc_time_hundredths;
        bool negative_offset = 0;
        bool mod_rtc_val;

        // Synchronize compensation process with RCX's rising edge.
        // Wait until Timer2 val changes. This happens in every RCX's rising edge.
        uint32_t val = REG_GETF(TIMER2, TIMER2_TIMER_VAL_REG , TIM_TIMER_VALUE);
        while ( REG_GETF(TIMER2, TIMER2_TIMER_VAL_REG , TIM_TIMER_VALUE) == val );

        // Read actual time from RTC
        hw_rtc_get_time_clndr(&current_time, NULL);

        usec = ((((current_time.hour * 60 + current_time.minute) * 60) + current_time.sec) * 1000 + current_time.hsec *10)*1000LL;
        if (current_time.hour_mode && current_time.pm_flag) {
                usec += HDAY_IN_USEC;
        }

        if (usec >= rtc_usec_prev) {
                usec_delta_i = usec - rtc_usec_prev;
        } else {
                usec_delta_i = (DAY_IN_USEC + usec) - rtc_usec_prev;
        }
        // Calculate the mean frequency from the last measurement
        mean_rcx_clock_hz_acc = (rcx_freq_prev + rcx_clock_hz_acc) / 2;

        // Calculate the theoretical time
        usec_delta_r = (uint32_t)(((uint64_t)usec_delta_i * (uint64_t)mean_rcx_clock_hz_acc) / (uint64_t)initial_rcx_clock_hz_acc);

        delta_slp_time = (int32_t)usec_delta_r - (int32_t)usec_delta_i;         // theoretical time - actual time
        rtc_usec_correction += delta_slp_time;                                  // correction factor

        if (rtc_usec_correction / HUNDREDTHS_OF_SEC_us > 0 ) {
                // rcx is rushing, rtc_usec_correction > 0, frequency is greater than initial
                negative_offset = true;
                mod_rtc_val = true;
        } else if (rtc_usec_correction / HUNDREDTHS_OF_SEC_us < 0) {
                // rcx is delayed, rtc_usec_correction < 0, frequency is smaller than initial
                negative_offset = false;
                mod_rtc_val = true;
        } else {
                mod_rtc_val = false;
        }

        rtc_usec_prev = usec;
        rcx_freq_prev = rcx_clock_hz_acc;

        if (mod_rtc_val) {
                // num_of_hundredths must be a positive number
                if (rtc_usec_correction < 0) {
                        num_of_hundredths = (rtc_usec_correction * (-1)) / HUNDREDTHS_OF_SEC_us;
                } else {
                        num_of_hundredths = rtc_usec_correction / HUNDREDTHS_OF_SEC_us;
                }

                rtc_time_hundredths = current_time.hsec;          // RTC's hos should not have changed yet.
                if (!negative_offset) {                         // if rcx is delayed, RTC is delayed too
                        if (rtc_time_hundredths + num_of_hundredths > 99) {
                                num_of_hundredths = 99 - rtc_time_hundredths;
                        }
                        rtc_usec_correction += (HUNDREDTHS_OF_SEC_us * num_of_hundredths);
                        new_rtc_time_hundredths = rtc_time_hundredths + num_of_hundredths;
                        rtc_usec_prev += (HUNDREDTHS_OF_SEC_us * num_of_hundredths);
                } else {                                        // if rcx is rushing, RTC is rushing too
                        if (rtc_time_hundredths < num_of_hundredths) {
                                num_of_hundredths = rtc_time_hundredths;
                        }
                        rtc_usec_correction -= (HUNDREDTHS_OF_SEC_us * num_of_hundredths);
                        new_rtc_time_hundredths = rtc_time_hundredths - num_of_hundredths;
                        rtc_usec_prev -= (HUNDREDTHS_OF_SEC_us * num_of_hundredths);
                }
                if (new_rtc_time_hundredths > 99) {
                        return;
                }
                cm_apply_rtc_compensation_hos(new_rtc_time_hundredths);
        }
}

#endif /* dg_configRTC_CORRECTION == 1 */

/**
 * \brief RCX Calibration Task function.
 *
 * \param [in] pvParameters ignored.
 */
static OS_TASK_FUNCTION(rcx_calibration_task, pvParameters )
{
        uint32_t ulNotifiedValue;
        OS_BASE_TYPE xResult __UNUSED;
        uint32_t cal_value;


#if (dg_configRTC_CORRECTION == 1)
        hw_rtc_register_cb(cm_rtc_callback);
#endif

        while (1) {
                // Wait for the internal notifications.
                xResult = OS_TASK_NOTIFY_WAIT(OS_TASK_NOTIFY_NONE, OS_TASK_NOTIFY_ALL_BITS, &ulNotifiedValue,
                                                                        OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(xResult == OS_OK);

                if (ulNotifiedValue & RCX_DO_CALIBRATION) {
                        uint64_t max_clk_count;

                        OS_ENTER_CRITICAL_SECTION();

                        cal_value        = hw_clk_get_calibration_data();
                        max_clk_count    = (uint64_t)dg_configXTAL32M_FREQ * RCX_CALIBRATION_CYCLES_PUP * RCX_ACCURACY_LEVEL;
                        rcx_clock_hz_acc = (max_clk_count + (cal_value >> 1)) / cal_value;
                        rcx_clock_hz     = rcx_clock_hz_acc / RCX_ACCURACY_LEVEL;
                        rcx_tick_rate_hz = get_optimum_tick_rate(rcx_clock_hz, &rcx_tick_period);
                        rcx_clock_period = (uint32_t)((rcx_period_dividend * RCX_ACCURACY_LEVEL) / rcx_clock_hz_acc);

#ifdef CONFIG_USE_BLE
#if (USE_BLE_SLEEP == 1)
                        /*
                         * Notify CMAC about the new values of:
                         *   rcx_clock_period
                         *   rcx_clock_hz_acc
                         */
                        ad_ble_update_rcx();
#endif /* (USE_BLE_SLEEP == 1) */
#endif /* CONFIG_USE_BLE */

#if (dg_configRTC_CORRECTION == 1)
                        // Run RTC compensation only if RTC time is running.
                        if (!HW_RTC_REG_GETF(RTC_CONTROL_REG, RTC_TIME_DISABLE)) {
                                cm_calculate_rtc_compensation_value();
                        }
#endif
                        OS_LEAVE_CRITICAL_SECTION();

#if (CPM_USE_RCX_DEBUG == 1)
                        log_printf(LOG_NOTICE, 1,
                                "clock_hz=%5d, tick_period=%3d, tick_rate_hz=%5d, clock_period=%10d\r\n",
                                rcx_clock_hz, rcx_tick_period, rcx_tick_rate_hz,
                                rcx_clock_period);
#endif
                }
        }
}
#endif /* (dg_configUSE_LP_CLK == LP_CLK_RCX) */
/**
 * \brief Start the timer and block sleep while the low power clock is settling.
 *
 * \details It starts the timer that blocks system from sleeping for
 *          dg_configXTAL32K_SETTLE_TIME. This is needed when the XTAL32K is used to make sure
 *          that the clock has settled properly before going back to sleep again.
 */
static void lp_clk_timer_start(void)
{
        /* Start the timer.  No block time is specified, and even if one was
         it would be ignored because the RTOS scheduler has not yet been
         started. */
        if (OS_TIMER_START(xLPSettleTimer, 0) != OS_TIMER_SUCCESS) {
                // The timer could not be set into the Active state.
                OS_ASSERT(0);
        }
}

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
void cm_rcx_calibration_task_init(void) {

        /* Start the task that will handle the calibration calculations,
         * which require ~80usec@32MHz to complete. */

        OS_BASE_TYPE status;

        // Create the RCX calibration task
        status = OS_TASK_CREATE("RCXcal",                       // The text name of the task.
                                rcx_calibration_task,           // The function that implements the task.
                                NULL,                           // No parameter is passed to the task.
                                OS_MINIMAL_TASK_STACK_SIZE,     // The size of the stack to allocate.
                                OS_TASK_PRIORITY_LOWEST,        // The priority assigned to the task.
                                xRCXCalibTaskHandle);           // The task handle is required.
        OS_ASSERT(status == OS_OK);

        (void) status;                                          // To satisfy the compiler
}
#endif
void cm_lp_clk_init(void)
{
        CM_MUTEX_GET();

        xLPSettleTimer = OS_TIMER_CREATE("LPSet",
                                OS_MS_2_TICKS(dg_configXTAL32K_SETTLE_TIME),
                                OS_TIMER_FAIL,          // Run once
                                (void *) 0,             // Timer id == none
                                vLPTimerCallback);      // Call-back
        OS_ASSERT(xLPSettleTimer != NULL);

        if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                lp_clk_timer_start();
        } else {
                // No need to wait for LP clock
                OS_EVENT_GROUP_SET_BITS(xEventGroupCM_xtal, LP_CLK_AVAILABLE);
        }

        CM_MUTEX_PUT();
}

__RETAINED_HOT_CODE bool cm_lp_clk_is_avail(void)
{
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        return (OS_EVENT_GROUP_GET_BITS(xEventGroupCM_xtal) & LP_CLK_AVAILABLE);
}

__RETAINED_CODE bool cm_lp_clk_is_avail_fromISR(void)
{
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        return (OS_EVENT_GROUP_GET_BITS_FROM_ISR(xEventGroupCM_xtal) & LP_CLK_AVAILABLE);
}

void cm_wait_lp_clk_ready(void)
{
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        OS_EVENT_GROUP_WAIT_BITS(xEventGroupCM_xtal,
                LP_CLK_AVAILABLE,
                OS_EVENT_GROUP_FAIL,                            // Don't clear bit after ret
                OS_EVENT_GROUP_OK,                              // Wait for all bits
                OS_EVENT_GROUP_FOREVER);                        // Block forever
}

__RETAINED_HOT_CODE void cm_lp_clk_wakeup(void)
{
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR(xEventGroupCM_xtal, LP_CLK_AVAILABLE);
}
#endif /* OS_PRESENT */

/*
 * Functions intended to be used only by the Clock and Power Manager or in hooks.
 */
__RETAINED_CODE static void apply_lowered_clocks(sys_clk_t new_sysclk, ahb_div_t new_ahbclk)
{

        // First the system clock
        if (new_sysclk != sysclk) {
                sys_clk_next = new_sysclk;

                // fast --> slow clock switch
                hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);                  // Set XTAL32 as sys_clk
                adjust_otp_access_timings();                         // Adjust OTP timings
        }
        // else cm_sysclk is RC32 as in all other cases it is set to XTAL32M.

        // Then the AHB clock
        if (new_ahbclk != ahbclk) {
                ahb_clk_next = new_ahbclk;

                if (ahbclk < new_ahbclk) {
                        // fast --> slow clock switch
                        hw_clk_set_hclk_div(new_ahbclk);
                        adjust_otp_access_timings();                 // Adjust OTP timings
                } else {
                        // slow --> fast clock switch
                        adjust_otp_access_timings();                 // Adjust OTP timings
                        hw_clk_set_hclk_div(new_ahbclk);
                }
        }
}

__RETAINED_CODE void cm_lower_all_clocks(void)
{
        DBG_SET_HIGH(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_LOWER_CLOCKS);

        sys_clk_t new_sysclk;
        ahb_div_t new_ahbclk = ahb_div1;

#ifdef OS_PRESENT
        // Cannot lower clocks if the first calibration has not been completed.
        if ((dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG) && (dg_configUSE_LP_CLK == LP_CLK_RCX) && !cm_lp_clk_is_avail_fromISR()) {
                return;
        }
#endif /* OS_PRESENT */

        // Check which is the lowest system clock that can be used.
        do {
                new_sysclk = sysclk;

                switch (sysclk) {
                case sysclk_RCHS_32:
                case sysclk_RCHS_64:
                case sysclk_RCHS_96:
                        // fall-through
                case sysclk_XTAL32M:
                        // unchanged: new_sysclk = cm_sysclk
                        break;
                case sysclk_PLL160:
                        // Check XTAL32 has settled.
                        if (!xtal32m_settled) {
                                break;
                        }
                        new_sysclk = sysclk_XTAL32M;
                        break;

                case sysclk_LP:
                        // fall-through
                default:
                        // should never reach this point
                        ASSERT_WARNING(0);
                }
        } while (0);

        if (!xtal32m_settled) {
                new_ahbclk = ahb_div16;                               // Use 2MHz AHB clock.
        } else {
                new_ahbclk = ahb_div8;                                // Use 4Mhz AHB clock.
        }

        // Check if the SysTick is ON and if it is affected
        if ((dg_configABORT_IF_SYSTICK_CLK_ERR) && (SysTick->CTRL & SysTick_CTRL_ENABLE_Msk)) {
                if ((new_sysclk != sysclk) || (new_ahbclk != ahbclk)) {
                        /*
                         * This is an application error! The SysTick should not run with any of the
                         * sleep modes active! This is because the OS may decide to go to sleep
                         * because all tasks are blocked and nothing is pending, although the
                         * SysTick is running.
                         */
                        new_sysclk = sysclk;
                        new_ahbclk = ahbclk;
                }
        }

        apply_lowered_clocks(new_sysclk, new_ahbclk);
}

__RETAINED_CODE void cm_restore_all_clocks(void)
{
#ifdef OS_PRESENT
        if ((dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG) && (dg_configUSE_LP_CLK == LP_CLK_RCX) && !cm_lp_clk_is_avail_fromISR()) {
                return;
        }
#endif /* OS_PRESENT */

        // Set the AMBA High speed Bus clock
        if (ahbclk != (ahb_div_t)hw_clk_get_hclk_div()) {
                ahb_clk_next = ahbclk;

                adjust_otp_access_timings();                         // Adjust OTP timings
                hw_clk_set_hclk_div(ahbclk);
        }

        // Set the system clock
        if (xtal32m_settled && ((sysclk == sysclk_XTAL32M) || (sysclk == sysclk_PLL160))) {
                sys_clk_next = sysclk;

                adjust_otp_access_timings();                         // Adjust OTP timings
                if (sysclk >= sysclk_PLL160) {
                        hw_clk_set_sysclk(SYS_CLK_IS_PLL);           // Set PLL as sys_clk
                } else {
                        hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);       // Set XTAL32 as sys_clk
                }
        }
        DBG_SET_LOW(CLK_MGR_USE_TIMING_DEBUG, CLKDBG_LOWER_CLOCKS);
}

#ifdef OS_PRESENT
void cm_wait_xtalm_ready_fromISR(void)
{
        if (!xtal32m_settled) {
                while (NVIC_GetPendingIRQ(XTAL32M_RDY_IRQn) == 0);
                xtal32m_settled = true;
                cm_switch_to_xtalm_if_settled();
                NVIC_ClearPendingIRQ(XTAL32M_RDY_IRQn);
        }
}

#endif /* OS_PRESENT */

__RETAINED_HOT_CODE bool cm_poll_xtalm_ready(void)
{
        return xtal32m_settled;
}

__RETAINED_HOT_CODE void cm_halt_until_xtalm_ready(void)
{
#ifdef OS_PRESENT
        while (!xtal32m_settled) {
                GLOBAL_INT_DISABLE();
                /* System waking up. We ignore this PRIMASK set. */
                DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);
                if (!xtal32m_settled) {
                        lower_amba_clocks();
                        __WFI();
                        restore_amba_clocks();
                }
                GLOBAL_INT_RESTORE();
        }
#else
        while (!xtal32m_settled) {
                GLOBAL_INT_DISABLE();
                lower_amba_clocks();
                __WFI();
                restore_amba_clocks();
                GLOBAL_INT_RESTORE();
        }
#endif /* OS_PRESENT */
}

void cm_register_xtal_ready_callback(void (*cb)(void))
{
        xtal_ready_callback = cb;
}

__RETAINED_HOT_CODE void cm_halt_until_pll_locked(void)
{
#ifdef OS_PRESENT
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        while (!pll_locked) {
                GLOBAL_INT_DISABLE();
                /* System waking up. We ignore this PRIMASK set. */
                if (!pll_locked) {
                        lower_amba_clocks();
                        __WFI();
                        restore_amba_clocks();
                }
                GLOBAL_INT_RESTORE();
        }
#else
        while (!pll_locked) {
                GLOBAL_INT_DISABLE();
                lower_amba_clocks();
                __WFI();
                restore_amba_clocks();
                GLOBAL_INT_RESTORE();
        }
#endif /* OS_PRESENT */
}

void cm_halt_until_pll_usb_locked(void)
{
#ifdef OS_PRESENT
        ASSERT_WARNING(xEventGroupCM_xtal != NULL);

        while (!pll_usb_locked) {
                GLOBAL_INT_DISABLE();
                /* System waking up. We ignore this PRIMASK set. */
                if (!pll_usb_locked) {
                        lower_amba_clocks();
                        __WFI();
                        restore_amba_clocks();
                }
                GLOBAL_INT_RESTORE();
        }
#else
        while (!pll_usb_locked) {
                GLOBAL_INT_DISABLE();
                lower_amba_clocks();
                __WFI();
                restore_amba_clocks();
                GLOBAL_INT_RESTORE();
        }
#endif /* OS_PRESENT */
}

/**
 * \brief Switch to XTAL32M - Interrupt Safe version.
 *
 * \details Waits until the XTAL32M has settled and sets it as the system clock.
 *
 * \warning It is called from Interrupt Context.
 */
__RETAINED_HOT_CODE  static void switch_to_xtal_safe(void)
{
        cm_halt_until_xtalm_ready();

        if (sys_clk_next > sysclk) {
                adjust_otp_access_timings();         // Adjust OTP timings
                hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);  // Set XTAL32 as sys_clk
        } else {
                hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);  // Set XTAL32 as sys_clk
                adjust_otp_access_timings();         // Adjust OTP timings
        }
}

__RETAINED_HOT_CODE void cm_sys_clk_sleep(bool entering_sleep)
{
        ahb_clk_next = ahb_div1;

        if (entering_sleep) {
                // Sleep entry : No need to switch to RCHS. PDC will do it.
                if ((REG_GETF(CRG_TOP, PMU_SLEEP_REG, ULTRA_FAST_WAKEUP) != 1) &&
                        ((sysclk == sysclk_RCHS_96) || (sysclk == sysclk_RCHS_64))) {
                        switch_to_rchs(RCHS_32);
                        // VDD voltage can be lowered
                        pmu_1v2_restore_voltage();
                }
                if (sysclk == sysclk_PLL160) {
                        if (hw_clk_get_sysclk() == SYS_CLK_IS_PLL) {
                                // Transition from PLL to RCHS is not allowed.
                                // Switch to XTAL32M first.
                                switch_to_xtal32m();
                        }
                        // No need to disable RCHS. It is already disabled.
                        disable_pll();
                }

                if ((sysclk == sysclk_PLL160) || (sysclk == sysclk_XTAL32M)) {
#if dg_configUSE_HW_PDC
                        if (xtal32_pdc_entry == HW_PDC_INVALID_LUT_INDEX) {
                                switch_to_rchs(hw_clk_get_rchs_mode());
                                hw_clk_disable_sysclk(SYS_CLK_IS_XTAL32M);
                        }
#else
                        switch_to_rchs(hw_clk_get_rchs_mode());
                        hw_clk_disable_sysclk(SYS_CLK_IS_XTAL32M);
#endif
                }

                // Make sure that the AHB and APB busses are clocked at 32MHz.
                if (ahbclk != ahb_div1) {
                        // slow --> fast clock switch
                        adjust_otp_access_timings();                 // Adjust OTP timings
                        hw_clk_set_hclk_div(ahb_div1);                  // cm_ahbclk is not altered!
                }
                hw_clk_set_pclk_div(apb_div1);                          // cm_apbclk is not altered!
        } else {
                /*
                 * XTAL32M ready: transition to the cm_sysclk, cm_ahbclk and cm_apbclk that were set
                 * by the user.
                 *
                 * Note that when the system wakes up the system clock is RCHS and the AHB / APB are
                 * clocked at highest frequency (because this is what the setting was just before
                 * sleep entry).
                 */

                if (((sysclk == sysclk_XTAL32M) || (sysclk == sysclk_PLL160)) && xtal32m_settled) {
                        sys_clk_t tmp_sys_clk = sysclk;

                        if (hw_clk_get_sysclk() == SYS_CLK_IS_RCHS) {
                                sys_clk_next = sysclk_XTAL32M;
                                sysclk = cm_sys_clk_get_fromISR();
                                switch_to_xtal_safe();
                                sysclk = sys_clk_next;

                                sys_clk_next = tmp_sys_clk;
                        }

                        if (sys_clk_next == sysclk_PLL160) {
                                if (hw_clk_is_pll_locked()) {
                                        switch_to_pll();
                                }
                                else {
                                        // System clock will be switched to PLL when PLL is locked
                                        enable_pll();
                                }
                        }
                        sysclk = sys_clk_next;
                } else if ((REG_GETF(CRG_TOP, PMU_SLEEP_REG, ULTRA_FAST_WAKEUP) != 1) &&
                        ((sysclk == sysclk_RCHS_96) || (sysclk == sysclk_RCHS_64))) {
                        pmu_1v2_set_max_voltage();
                        switch_to_rchs((sysclk == sysclk_RCHS_96) ? RCHS_96 : RCHS_64);
                }       // else
                        // If the user uses RCHS@32 as the system clock or
                        // RCHS@96/64 and fast wakeup then there's nothing to be done!

                if (ahbclk != ahb_div1) {
                        ahb_clk_next = ahbclk;

                        // fast --> slow clock switch
                        hw_clk_set_hclk_div(ahbclk);                 // cm_ahbclk is not altered!
                        adjust_otp_access_timings();                 // Adjust OTP timings
                }
                // else cm_ahbclk == ahb_div1 and nothing has to be done!

                if (apbclk != apb_div1) {
                        hw_clk_set_pclk_div(apbclk);
                }
                // else cm_apbclk == apb_div1 and nothing has to be done!
        }
}

void cm_sys_restore_sysclk(sys_clk_t prev_sysclk)
{
        ASSERT_ERROR(prev_sysclk == sysclk_PLL160);

        sys_enable_pll();
        sys_clk_next = prev_sysclk;
        switch_to_pll();
}

#ifdef OS_PRESENT
__RETAINED_HOT_CODE void cm_sys_clk_wakeup(void)
{
        /*
         * Clear the "XTAL32_AVAILABLE" flag in the Event Group of the Clock Manager. It will be
         * set again to 1 when the XTAL32M has settled.
         * Note: this translates to a message in a queue that unblocks the Timer task in order to
         * service it. This will be done when the OS scheduler is resumed. Even if the
         * XTAL32M_RDY_IRQn hits while still in this function (pm_sys_wakeup_mode_is_XTAL32 is true), this
         * will result to a second message being added to the same queue. When the OS scheduler is
         * resumed, the first task that will be executed is the Timer task. This will first process
         * the first message of the queue (clear Event Group bits) and then the second (set Event
         * Group bits), which is the desired operation.
         */

        /* Timer task must have the highest priority so that it runs first
         * as soon as the OS scheduler is unblocked.
         * See caller (system_wake_up()) */
        ASSERT_WARNING(OS_DAEMON_TASK_PRIORITY == OS_TASK_PRIORITY_HIGHEST);

        xtal32m_settled_notification = false;
        xtal32m_settled = hw_clk_is_xtalm_started();
        if (!xtal32m_settled) {
                OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR(xEventGroupCM_xtal, XTAL32_AVAILABLE);
        }

        pll_locked = hw_clk_is_pll_locked();
        pll_usb_locked = false;
        if (!pll_locked) {
                OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR(xEventGroupCM_xtal, PLL_AVAILABLE);
        }
}

__RETAINED_HOT_CODE void cm_switch_to_xtalm_if_settled(void)
{
        if ((sysclk == sysclk_XTAL32M || sysclk == sysclk_PLL160) && xtal32m_settled) {
                GLOBAL_INT_DISABLE();

                // Restore system clocks
                cm_sys_clk_sleep(false);

                GLOBAL_INT_RESTORE();

                OS_BASE_TYPE xHigherPriorityTaskWoken;

                xtal32m_is_ready(&xHigherPriorityTaskWoken);
        }
}

#endif /* OS_PRESENT */

#endif /* dg_configUSE_CLOCK_MGR */


