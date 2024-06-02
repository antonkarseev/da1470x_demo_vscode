/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup CLOCK_MANAGER Clock Manager Service
 *
 * \brief Clock Manager
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_clock_mgr.h
 *
 * @brief Clock Manager header file.
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_CLOCK_MGR_H_
#define SYS_CLOCK_MGR_H_

#include <stdint.h>
#include <stdbool.h>
#include "hw_clk.h"

/**
 * \brief SYS_CLOCK_MGR error codes
 */
typedef enum {
        cm_sysclk_div1_clk_in_use,
        cm_sysclk_pll_used_by_task,
        cm_sysclk_higher_prio_used,
        cm_sysclk_success
} cm_sys_clk_set_status_t;

/**
 * \brief Initialize system clock.
 *
 * \details It initializes the Clock Manager and sets the system clock (sys_clk) to the (initially)
 *          preferred clock source.
 *
 * \param[in] type: The clock source to use as sys_clk.
 *
 * \note The sys_clk can be changed later by calling cm_sys_clk_set().
 *
 * \warning It has to be called only once, after power-up and before calling most other functions of
 *          the Clock Manager (since it initializes the internal data used by them).
 *
 * \warning It must be called with interrupts enabled!
 *
 * \warning If case of selecting PLL as sys_clk, then any attempt to switch to another sys_clk later
 *          using cm_sys_clk_set() will fail (i.e. will return cm_sysclk_pll_used_by_task)!
 *          Therefore, in case of intending to use PLL only temporarily, a different sys_clk should
 *          first be chosen with cm_sys_clk_init() and the switch to PLL must be done at a later
 *          step, when needed, using the cm_sys_clk_set() function.
 *
 * \sa cm_sys_clk_set()
 */
void cm_sys_clk_init(sys_clk_t type);

/**
 * \brief Calibrate RCX
 */
void cm_rcx_calibrate(void);

/**
 * \brief Set the system clock.
 *
 * \details This function requests from the Clock Manager to use a specific clock source as system
 *          clock (sys_clk). If this is possible, the request is accepted and the function switches
 *          the sys_clk setting to the selected clock source and returns cm_sysclk_success.
 *          Otherwise, the request is rejected and the function returns a value that denotes the
 *          reason why the switch was not possible.
 *
 *          If the request involves enabling the fast Xtal clock (XTAL32M), then, apart from
 *          powering on the XTAL32M, the function will wait for it to settle before setting it as
 *          sys_clk. If the PLL is requested, the function will also wait for the PLL to lock.
 *
 *          The Clock Manager will also take care of automatically restoring the sys_clk to the
 *          requested setting after each wake-up (as soon as the XTAL32M settles (and the PLL
 *          locks), if used).
 *
 *          If a switch to PLL is performed, the Clock Manager retains the PLL as sys_clk until it
 *          is not needed anymore. This is more specifically done on the following scheme:
 *          A successful PLL request is noted down by the Clock Manager and is considered "active"
 *          until it is "invalidated". A request for a different clock source - regardless if it is
 *          eventually accepted or rejected - invalidates the latest successful PLL request* (if
 *          any). If after this (i.e. the invalidation), there still is at least one active PLL
 *          request, then the new request (i.e. the non-PLL one) is rejected (the function returns
 *          cm_sysclk_pll_used_by_task) and the PLL is retained as sys_clk. Otherwise, the new
 *          request is accepted and the function changes the sys_clk and returns cm_sys_clk_success.
 *
 *          *If OS_PRESENT, it invalidates only the latest successful PLL request that was made by
 *          the same task.
 *
 *          On top of the above, if there is at least one peripheral that currently uses the Div1
 *          clock, the function rejects any request for a sys_clk setting other than the current
 *          one. If the dg_configABORT_IF_SYSTICK_CLK_ERR setting is enabled, the same also applies
 *          for the case the SysTick timer is running. In such cases, the function returns
 *          cm_sysclk_div1_clk_in_use.
 *
 * \param[in] type: The clock source to use as sys_clk.
 *
 * \return cm_sysclk_success:            if the requested sys_clk switch was applied
 * \return cm_sysclk_div1_clk_in_use:    if the sys_clk cannot be switched because a peripheral is
 *                                       clocked by the Div1 clock
 * \return cm_sysclk_pll_used_by_task:   if the PLL is still needed
 *
 * \warning This function provides protection only for the PLL clock. In order to have protection also
 *          for the other available sys_clk options based on a certain desired sys_clk priority scheme,
 *          it is recommended to use the cm_sys_clk_set_priority(), cm_sys_clk_request() and
 *          cm_sys_clk_release() functions instead.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
cm_sys_clk_set_status_t cm_sys_clk_set(sys_clk_t type);

/**
 * \brief Set the system clock priority.
 *
 * \details The system clock priority indicates which sys_clk prevails against another one
 *          when requesting a new sys_clk using cm_sys_clk_request().
 *
 * \param[in] sys_clk_prio_array: pointer to array containing the system clock priority (in decreasing order)
 *
 * \note If called, the Clock Manager considers cm_sys_clk_request() to be used for any possible
 *       future system clock switching (instead of cm_sys_clk_set()).
 *
 * \note The provided array is expected to hold the first five members of the sys_clk_t enum in the order that
 *       denotes the desired priority scheme, with each type appearing only once in the array and the first
 *       element denoting the highest priority sys_clk. In case the array contains more elements, only the first
 *       five ones are taken into account.
 *
 * \warning If dg_configENABLE_RCHS_CALIBRATION is set to 1, the priorities of all RCHS clock types must be
 *          lower than the one of XTAL32M.
 *
 * \warning It has to be called only once, after power-up and before calling cm_sys_clk_init().
 *
 * \sa cm_sys_clk_request()
 *
 */
void cm_sys_clk_set_priority(sys_clk_t *sys_clk_prio_array);

/**
 * \brief Request for system clock switch.
 *
 * \details This function can be used for system clock switching instead of cm_sys_clk_set().
 *          It requests from the Clock Manager to use a specific clock source as system
 *          clock (sys_clk). If this is possible, the request is accepted and the function switches
 *          the sys_clk setting to the selected clock source and returns cm_sysclk_success.
 *          Otherwise, the request is rejected and the function returns a value that denotes the
 *          reason why the switch was not possible.
 *
 *          The request can be cancelled/released later by calling cm_sys_clk_release().
 *          Until then, the requested sys_clk is protected from any lower priority sys_clk requests
 *          (with the priority as defined by cm_sys_clk_set_priority()).
 *
 *          There are two reasons that make switching the system clock not possible.
 *          Either if there is at least one peripheral that currently uses the Div1 clock,
 *          or if the current system clock has higher priority and is currently in use.
 *          In the latter case, in order for the switch to be successful a release operation
 *          should execute first.
 *
 *          If the request involves enabling the fast Xtal clock (XTAL32M), then, apart from
 *          powering on the XTAL32M, the function will wait for it to settle before setting it as
 *          sys_clk. If the PLL is requested, the function will also wait for the PLL to lock.
 *
 *          The Clock Manager will also take care of automatically restoring the sys_clk to the
 *          requested setting after each wake-up (as soon as the XTAL32M settles (and the PLL
 *          locks), if used).
 *
 * \param[in] type: The clock source to use as sys_clk.
 *
 * \return cm_sysclk_success:            if the requested sys_clk switch was applied
 * \return cm_sysclk_div1_clk_in_use:    if the sys_clk cannot be switched because a peripheral is
 *                                       clocked by the Div1 clock
 * \return cm_sysclk_higher_prio_used:   if a higher priority system clock is still in use
 *
 * \note Requires that cm_sys_clk_set_priority() has been called in advance.
 *
 * \note Even if the request is rejected, it is still taken into account (until cancelled by a
 *       corresponding cm_sys_clk_release() call), and the switch to the requested clock may take
 *       place later (e.g. as soon as the existing higher priority clock requests are released).
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 *
 * \sa cm_sys_clk_set_priority()
 * \sa cm_sys_clk_release()
 */
cm_sys_clk_set_status_t cm_sys_clk_request(sys_clk_t type);

/**
 * \brief Restores the system clock. It terminates a matching request.
 *
 * \details If there are other sys_clk requests that are still active (i.e. not released),
 *          the system will switch to the one that has the highest priority. Otherwise, it will
 *          switch to the initial sys_clk that was specified when cm_sys_clk_init() was called.
 *          No change however will be made if Div1 is in use.
 *
 * \param[in] type: The clock source the system was requested to use, when the matching
 *                  cm_sys_clk_request() was called.
 *
 * \return cm_sysclk_success:           the release operation was completed successfully and either
 *                                      triggered a switch to a lower_priority sys_clk or did not
 *                                      trigger any sys_clk_change (either because there are still
 *                                      other active requests for the same sys_clk that have not been
 *                                      released yet or because there are no other active requests and
 *                                      the released sys_clk is also the default one
 *                                      (as specified in cm_sys_clk_init()))
 * \return cm_sysclk_div1_clk_in_use:   the release operation was completed successfully and it should
 *                                      normally trigger a new sys_clk change (based on other active requests)
 *                                      but it didn't because a peripheral is clocked by the Div1 clock
 *
 * \warning This function must be called ONLY for terminating a matching cm_sys_clk_request.
 *          If called alone the system will reach an error-state!
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 *
 * \sa cm_sys_clk_request
 */
cm_sys_clk_set_status_t cm_sys_clk_release(sys_clk_t type);


/**
 * \brief Set the CPU clock.
 *
 * \details It attempts to achieve a specific CPU clock frequency by applying the proper settings
 *          for the system clock (sys_clk) and the AMBA High speed bus (AHB) divider. (The ARM CPU
 *          runs using the AHB clock.) If the requested frequency is not achievable with any
 *          sys_clk, it returns false. It will also return false in case PLL160 is in use and the
 *          requested frequency is only achievable with a different sys_clk setting.
 *
 *          If the requested frequency requires enabling the fast Xtal clock (XTAL32M), then, apart
 *          from powering on XTAL32M, the function will wait for it to settle before setting it as
 *          sys_clk.  If it also requires using the PLL as sys_clk, then the function, will also
 *          wait for it to lock.
 *
 *          The Clock Manager will also take care of restoring the last-requested CPU clock
 *          frequency (unless implicitly changed using cm_sys_clk_set()) to the type set by this
 *          function after each wake-up, automatically, as soon as XTAL32M (and PLL) settles.
 *
 * \param[in] clk: The CPU clock frequency (in MHz).
 *
 * \return True if the requested clock switch was applied, else false.
 *
 * \note In case there are more than one possible ways to achieve the specified CPU frequency, the
 *       function tries to preserve the current sys_clk setting if possible, otherwise:
 *       - If XTAL32M is currently being used (i.e. currently selected clock is XTAL32M or PLL160M)
 *       it tries to make use of it.
 *       - If it is not currently being used (i.e. the currently selected clock is the high speed RC
 *         (RCHS)), it tries to avoid using it (and continue using RCHS).
 *
 * \warning If the requested frequency implies switching to PLL160, the function will do so but
 *          (unlike the cm_sys_clk_set() function) this will not affect (i.e. it will not protect
 *          PLL160 from) any future cm_sys_clk_set() or cm_cpu_clk_set() calls requesting sys_clk
 *          settings other than PLL160 (by the same or other task)!
 *
 * \warning Similarly, in case of making use of the cm_sys_clk_request/release() mechanism, this
 *          function takes into account any existing active sys_clk requests before switching to
 *          the appropriate sys_clk but it does not also affect the existing requests (it does not
 *          constitute a request by itself).
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
bool cm_cpu_clk_set(cpu_clk_t clk);

/**
 * \brief Set the CPU clock (interrupt safe version).
 *
 * \details It sets the system clock (sys_clk) and the AMBA High speed bus (AHB) divider to the
 *          requested values. So, in fact, since The ARM CPU runs using the AHB clock, it
 *          implicitly sets the CPU clock.
 *
 * \param[in] clk: The clock source to use as sys_clk.
 * \param[in] hdiv: The divider of the AHB clock.
 *
 * \warning It must only be called from Interrupt Context and/or with all interrupts disabled.
 *
 * \warning The caller must have checked that the current sys_clk is not the desired one before
 *          calling this function.
 *
 */
void cm_cpu_clk_set_fromISR(sys_clk_t clk, ahb_div_t hdiv);
/**
 * \brief Set slow-pclk divider
 *
 * \details Changes the setting of the AMBA Slow Peripheral Bus clock (slow-pclk) divider.
 *          The frequency of the slow-pclk is (DivN / (1 << div)).
 *
 * \param[in] div: The new slow-pclk divider setting.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
void cm_apb_slow_set_clock_divider(apb_div_t div);

/**
 * \brief Set pclk divider
 *
 * \details Changes the setting of the AMBA Fast Peripheral Bus clock (pclk) divider.
 *          The frequency of the pclk is (system_clock / (1 << hclk_div)) / (1 << div).
 *
 * \param[in] div: The new pclk divider setting.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
void cm_apb_set_clock_divider(apb_div_t div);

/**
 * \brief Change the divider of the AMBA High speed Bus (AHB) clock.
 *
 * \details The frequency of the AHB clock is (system_clock / (1 << div)).
 *          Note: if the SysTick runs then it is the dg_configABORT_IF_SYSTICK_CLK_ERR setting that
 *          controls whether the switch will be aborted or not.
 *
 * \param[in] div: The new value of the AHB divider.
 *
 * \return True if the divider was changed to the requested value, else false.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */

bool cm_ahb_set_clock_divider(ahb_div_t div);

/**
 * \brief Get sys_clk
 *
 * \details Returns that system clock (sys_clk) that the system uses at that moment.
 *
 * \return: The real sys_clk used by the system.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
sys_clk_t cm_sys_clk_get(void);

/**
 * \brief Get sys_clk (interrupt safe)
 *
 * \details Returns the system clock (sys_clk) that the system uses at that moment (interrupt safe version).
 *
 * \return The real sys_clk used by the system.
 */
__RETAINED_HOT_CODE sys_clk_t cm_sys_clk_get_fromISR(void);
/**
 * \brief Get slow-pclk divider
 *
 * \details Returns the current setting of the AMBA Slow Peripheral Bus clock (slow-pclk) divider.
 *
 * \return The slow-pclk divider being used.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
apb_div_t cm_apb_slow_get_clock_divider(void);

/**
 * \brief Get pclk divider
 *
 * \details Returns the current setting of the AMBA Fast Peripheral Bus clock (pclk) divider.
 *
 * \return The (fast) pclk divider being used.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
apb_div_t cm_apb_get_clock_divider(void);
/**
 * \brief Get hclk divider
 *
 * \details Returns the current setting of the AMBA High speed Bus clock (hclk) divider.
 *
 * \return The hclk divider being used.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
ahb_div_t cm_ahb_get_clock_divider(void);

/**
 * \brief Get the CPU clock.
 *
 * \return The CPU clock being used.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
cpu_clk_t cm_cpu_clk_get(void);

/**
 * \brief Get the CPU clock (interrupt safe version).
 *
 * \return The CPU clock being used.
 *
 * \warning It can be called from Interrupt Context.
 */
__RETAINED_HOT_CODE cpu_clk_t cm_cpu_clk_get_fromISR(void);

/**
 * \brief Calibrate RC32K.
 */
void cm_calibrate_rc32k(void);

#ifdef OS_PRESENT
/**
 * \brief Converts usec to RCX cycles.
 *
 * \return The number of RCX cycles for the given time period.
 *
 * \warning Maximum time period is 4.095msec.
 */
__RETAINED_CODE uint32_t cm_rcx_us_2_lpcycles(uint32_t usec);

/**
 * \brief Converts time to RCX cycles.
 *
 * \return The number of RCX cycles for the given time period.
 *
 * \warning This is a low accuracy function. To have good accuracy, the minimum time period should
 *          be 1msec and the maximum 200msec. Above 200msec, the function calculates more RCX cycles
 *          than necessary.
 */
uint32_t cm_rcx_us_2_lpcycles_low_acc(uint32_t usec);

#endif /* OS_PRESENT */

/**
 * \brief Wait until the fast Xtal clock has settled.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning If may block.
 */
void cm_wait_xtalm_ready(void);

#ifdef OS_PRESENT
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
/**
 * \brief Initialize the RCX calibration task.
 */
void cm_rcx_calibration_task_init(void);

/**
 * \brief Trigger RCX calibration.
 */
void cm_rcx_trigger_calibration(void);
#endif /* dg_configUSE_LP_CLK */
/**
 * \brief Initialize the Low Power clock.
 *
 * \note Since the XTAL32K settling takes a long time, the function does not block waiting for
 *       XTAL32K to settle. However, the system is kept in active mode until this completes.
 *
 * \warning As for most other functions of the Clock Manager, it should not be called before cm_sys_clk_init().
 *
 * \warning It cannot be called from Interrupt Context.
 */
void cm_lp_clk_init(void);

/**
 * \brief Check if the Low Power clock is available.
 *
 * \return true if the LP clock is available, else false.
 *
 * \warning It cannot be called from Interrupt Context.
 */
__RETAINED_HOT_CODE bool cm_lp_clk_is_avail(void);

/**
 * \brief Check if the Low Power clock is available (interrupt safe version).
 *
 * \return true if the LP clock is available, else false.
 *
 * \warning It can be called from Interrupt Context.
 */
__RETAINED_CODE bool cm_lp_clk_is_avail_fromISR(void);

/**
 * \brief Wait until the Low Power clock is available.
 *
 * \warning It may block. It cannot be called from Interrupt Context.
 */
void cm_wait_lp_clk_ready(void);

/**
 * \brief   Clear the flag that indicates that the Low Power clock is available.
 *
 * \details It is called when the system wakes up from a "forced" deep sleep state and the low power
 *          XTAL (XTAL32K) is used as the Low Power clock so that the system won't enter sleep until
 *          XTAL32K has settled.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
__RETAINED_HOT_CODE void cm_lp_clk_wakeup(void);

#endif /* OS_PRESENT */

/**
 * \brief Wait until the PLL is locked. If the PLL is locked then the function exits immediately.
 *
 * \warning It cannot be called from Interrupt Context.
 *
 * \warning It may block.
 */
void cm_wait_pll_lock(void);


/**
 * \brief Check if the fast Xtal clock is ready.
 *
 * \return True if the fast Xtal clock has settled, else false.
 */
__RETAINED_HOT_CODE bool cm_poll_xtalm_ready(void);

/**
 * \brief Start the fast Xtal clock
 *
 * \details Checks if the fast Xtal clock (XTAL32M) is started. If not, it checks if there is a PDC
 *          entry for starting XTAL32M and if there is, it uses PDC to start it. Otherwise, it
 *          enables it using hw_clk_enable_sysclk().
 */
__RETAINED_HOT_CODE void cm_enable_xtalm(void);


/* ---------------------------------------------------------------------------------------------- */

/*
 * Functions intended to be used only by the Clock and Power Manager.
 */

/**
 * \brief Set the system clock (unprotected).
 *
 * \details It attempts to:
 *              - Prepare the system clock for sleep: called when the system is entering power-down mode.
 *                        The system clock settings of the application are kept in order to be able to
 *                        restore them. If the PLL is active it will be turned off.
 *                        (It is called with the scheduler stopped and all interrupts disabled in
 *                        this case.)
 *              - Restore the previous setting: called when the fast Xtal clock settles.
 *                        (It is called from ISR context with all interrupts disabled in this case.)
 *
 * \param[in] entering_sleep true if the system is going to sleep, else false.
 *
 * \warning It must be called from Interrupt Context and/or with all interrupts disabled.
 *
 * \warning The function is internal to the clock and power managers and should not be used externally!
 */
__RETAINED_HOT_CODE void cm_sys_clk_sleep(bool entering_sleep);

/**
 * \brief Halt until the fast Xtal clock has settled.
 *
 * \details It executes a WFI() call waiting for the fast Xtal clock Ready interrupt.
 *          Any other interrupts that hit are served.
 */
__RETAINED_HOT_CODE void cm_halt_until_xtalm_ready(void);

/**
 * \brief Register a callback function to be called when the fast Xtal clock settles.
 *
 * \details cb pointer to the callback function
 */
void cm_register_xtal_ready_callback(void (*cb)(void));

/**
 * \brief Halt until PLL is locked
 *
 * \details It executes a WFI() call waiting for the PLL Lοck interrupt.
 *          Any other interrupts that hit are served.
 */
__RETAINED_HOT_CODE void cm_halt_until_pll_locked(void);

/**
 * \brief Halt until system clock (either PLL or XTAL32M) is ready.
 *
 * \details It executes a WFI() call waiting for the XTAL32M Ready interrupt and PLL LOCK interrupt
 *          if needed. Any other interrupts that hit are served.
 */
__RETAINED_HOT_CODE void cm_halt_until_sysclk_ready(void);


/**
 * \brief Halt until USB PLL is locked
 *
 * \details It executes a WFI() call waiting for the PLL48_LOCK_IRQn.
 */
__RETAINED_CODE void cm_halt_until_pll_usb_locked(void);

/**
 * \brief Enable USB PLL
 */
void cm_sys_enable_pll_usb(void);

/**
 * \brief Disable USB PLL
 */
void cm_sys_disable_pll_usb(void);

/**
 * \brief Calibrate RCHS
 */
void cm_rchs_calibrate(void);

#endif /* SYS_CLOCK_MGR_H_ */

/**
 \}
 \}
 */
