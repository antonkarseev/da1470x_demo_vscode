/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup SYS_TIMER
 * \{
 * \addtogroup INTERNAL
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_timer_internal.h
 *
 * @brief System timer internal header file.
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_TIMER_INTERNAL_H_
#define SYS_TIMER_INTERNAL_H_

#include "sdk_defs.h"

#if dg_configUSE_HW_TIMER
# define HAVE_SYS_TIMER
#endif

#ifdef HAVE_SYS_TIMER

#include "hw_timer.h"

/* System Timer - abstraction layer */

#if (MAIN_PROCESSOR_BUILD)

#define LP_CNT_NATIVE_MASK      ( TIMER2_TIMER2_TIMER_VAL_REG_TIM_TIMER_VALUE_Msk >> TIMER2_TIMER2_TIMER_VAL_REG_TIM_TIMER_VALUE_Pos )

#define SYS_HW_TIMER                    HW_TIMER2
#define SYS_HW_TIMER_IRQ                TIMER2_IRQn

#elif (SNC_PROCESSOR_BUILD)

#define LP_CNT_NATIVE_MASK      ( TIMER3_TIMER3_TIMER_VAL_REG_TIM_TIMER_VALUE_Msk >> TIMER3_TIMER3_TIMER_VAL_REG_TIM_TIMER_VALUE_Pos )

#define SYS_HW_TIMER                    HW_TIMER3
#define SYS_HW_TIMER_IRQ                TIMER3_IRQn

#endif /* PROCESSOR_BUILD */


#ifdef OS_PRESENT
extern uint16_t lp_clock_hz; /* The frequency of the current lp clock */
extern uint8_t lp_tick_period; /* The os timer tick period based on the current lp clock */
extern uint16_t lp_tick_rate_hz; /* The os timer tick rate based on the current lp clock */
/**
 * \brief Set lp_clk parameters for OS.
 *
 * \param[in] clk_hz The frequency in Hz of current low power clock.
 * \param[in] tick_rate_hz The tick rate in Hz of the OS.
 * \param[in] tick_period The ratio of LP to OS tick rates.
 */
void sys_timer_set_timer_vars(uint16 clock_hz, uint16_t tick_rate_hz, uint8_t tick_period);

#if (MAIN_PROCESSOR_BUILD)
#ifdef CONFIG_USE_SNC

/**
 * \brief Share lp_clk parameters with SNC.
 *
 */
void sys_timer_share_timer_vars(void);
#endif /* CONFIG_USE_SNC */
#elif SNC_PROCESSOR_BUILD

/**
 * \brief Retrieve lp_clk parameters from M33.
 *
 */
void sys_timer_retrieve_shared_timer_vars(void);
#endif /* MAIN_PROCESSOR_BUILD */
#endif /* OS_PRESENT */


/**
 * \brief Update Real Time Clock value and get current time (in LP cycles).
 *
 * \param[out] timer_value Pointer to the value of the timer
 * \return The current RTC time.
 *
 * \warning This function is used only by the Clock and Power Manager.
 *
 */
__RETAINED_HOT_CODE uint64_t sys_timer_get_timestamp_fromCPM(uint32_t *timer_value);

/**
 * \brief Set an "invalid" trigger value, which refers far away in the future
 *
 */
__STATIC_FORCEINLINE void sys_timer_invalidate_trigger(void)
{
        uint32_t lp_current_time;
        uint32_t trigger;

        lp_current_time = hw_timer_get_count(SYS_HW_TIMER);         // Get current time
        trigger = (lp_current_time - 1) & LP_CNT_NATIVE_MASK;  // Get an "invalid" trigger
        bool sys_timer_irq_en = HW_TIMER_REG_GETF(SYS_HW_TIMER, TIMER_CTRL_REG, TIM_IRQ_EN);
        if (sys_timer_irq_en) {
                HW_TIMER_REG_SETF(SYS_HW_TIMER, TIMER_CTRL_REG, TIM_IRQ_EN, 0);
        }
        hw_timer_set_reload(SYS_HW_TIMER, trigger);
        if (sys_timer_irq_en) {
                HW_TIMER_REG_SETF(SYS_HW_TIMER, TIMER_CTRL_REG, TIM_IRQ_EN, 1);
        }
}

/**
 * \brief Calculate how many ticks have passed since the time the system entered sleep or idle
 *        mode and update OS
 *
 * \return The number of ticks spent while sleeping
 */
__RETAINED_HOT_CODE uint32_t sys_timer_update_slept_time(void);

/**
 * \brief Get the address of the current Real Time Clock time.
 *
 * \return The address of the current RTC time.
 *
 */
uint64_t* sys_timer_get_rtc_time(void);

/**
 * \brief Get the address of the current time.
 *
 * \return The address of the current time.
 *
 */
uint32_t* sys_timer_get_current_time(void);


#endif /* HAVE_SYS_TIMER */

#endif /* SYS_TIMER_INTERNAL_H_ */

/**
\}
\}
\}
\}
*/
