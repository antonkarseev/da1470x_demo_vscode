/**
 \addtogroup BSP
 \{
 \addtogroup SYSTEM
 \{
 \addtogroup CLOCK_MANAGER
 \{
 \addtogroup INTERNAL
 \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_clock_mgr_internal.h
 *
 * @brief Clock Manager internal header file.
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_CLOCK_MGR_INTERNAL_H_
#define SYS_CLOCK_MGR_INTERNAL_H_

#include <stdint.h>
#include <stdbool.h>
#include "hw_clk.h"

#define RCX_ACCURACY_LEVEL              8              // Must be a power of 2!


#if dg_configENABLE_RCHS_CALIBRATION
#define RCHS_TEMP_POLL_INT              1000 // in ms
#define RCHS_TEMP_DRIFT                 10 * 82 // 10 Celsius degrees * 82LSB/degree
#define RCHS_UNCOND_TRIGGER             ((24 * 60 * 60 * 1000) / RCHS_TEMP_POLL_INT) /* the number of counts needed to
                                                                                      * trigger the RCHS calibration
                                                                                      * once a day
                                                                                      */
#endif

# if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Basic initialization of the system clocks.
 *
 * \details Enables the IRQs of the system clocks in NVIC, enables XTAL32M interrupt generation and
 * initializes xtal32_pdc_entry.
 *
 * \warning It must be called once from SystemInit()
 */
void cm_sysclk_init_low_level_internal(void);

/**
 * \brief Basic initialization of the low power clock.
 *
 * \details It sets up the low power clock.
 *
 * \warning It must be called once from SystemInit()
 */
void cm_lpclk_init_low_level_internal(void);

# endif

/**
 * \brief Get RCX frequency in hz with accuracy level of RCX_ACCURACY_LEVEL.
 *
 * \return RCX frequency in hz with accuracy level of RCX_ACCURACY_LEVEL.
 */
uint32_t cm_get_rcx_clock_hz_acc(void);

/**
 * \brief Get RCX period in 1024*1024*1000000 with accuracy level of RCX_ACCURACY_LEVEL.
 *
 * \return RCX period in 1024*1024*1000000 with accuracy level of RCX_ACCURACY_LEVEL.
 */
uint32_t cm_get_rcx_clock_period(void);


/**
 * \brief Lower all clocks to the lowest frequency possible (best effort).
 *
 * \details 1. Check which is the lowest system clock that can be used.
 *             The fast RC clock (RCxxM) is the lowest but it does not make sense to use it if the
 *             system clock is the fast XTAL clock (XTALxxM) or the PLL. Thus, the lowest system
 *             clock setting will always be the fast XTAL clock (XTALxxM) if the current system
 *             clock is not the fast RC clock (RCxxM). If the PLL is on then the switch to the fast
 *             XTAL clock (XTALxxM) will be done without disabling the PLL. No block is informed
 *             about the change. If there is an active SPI or I2C transaction, it may fail.
 *
 *          2. Check which is the lowest AHB clock that can be used.
 *             For DA1468x: When a MAC is active, the lowest AHB clock is 16MHz. The frequency
 *             change will destroy any ongoing IR transaction.
 *
 *          Note: if the SysTick runs then it is the dg_configABORT_IF_SYSTICK_CLK_ERR setting that
 *          controls whether the switch will continue or it will be aborted.
 *
 *          3. The APB clock is always set to the lowest setting.
 *
 * \warning It must be called with all interrupts disabled. Cannot be called by Application tasks!
 */
__RETAINED_CODE void cm_lower_all_clocks(void);

/**
 * \brief Restore all clocks to the speed set by the user.
 *
 * \warning It must be called with all interrupts disabled. Cannot be called by Application tasks!
 */
__RETAINED_CODE void cm_restore_all_clocks(void);

#ifdef OS_PRESENT
/**
 * \brief Clear the Event Groups Bit(s), the "settled" flag and the RCX calibration flag.
 *
 * \details It pends the clearing of the Event Groups bit(s) to the OS task daemon. In the case of
 *          waking up from the Tick timer, no other task is ready-to-run anyway. In the case
 *          though of waking up from an external interrupt to WKUPCT then another task of equal
 *          priority to the OS daemon task may also become ready-to-run. But even in this case, the
 *          first task that is made ready-to-run is the OS daemon task and this is the task that the
 *          scheduler will execute first.
 *
 * \warning It must be called from Interrupt Context and with all interrupts disabled.
 *          The priority of the Timers task (OS daemon task) must be the highest!
 *          The function is internal to the CPM and should not be used externally!
 */
__RETAINED_HOT_CODE void cm_sys_clk_wakeup(void);

/**
 * \brief Wait until the fast XTAL clock (XTALxxM) is ready and then switch clocks if necessary.
 *
 * \warning It must be called from Interrupt Context.
 */
void cm_wait_xtalm_ready_fromISR(void);

/**
 * \brief Start the XTAL32M clock only if required
 *
 * If system clock is not RCHS then cm_enable_xtalm() is called to enable XTAL32M
 *
 */
__RETAINED_HOT_CODE void cm_enable_xtalm_if_required(void);
/**
 * \brief Get XTAL32M settling time
 *
 * \return The number of LP clock cycles required for XTAL32M to settle if system clock is not RCHS
 *         0 if system clock is RCHS
 *
 */
__RETAINED_HOT_CODE uint32_t cm_get_xtalm_settling_lpcycles(void);

/**
 * \brief Check if fast XTAL clock (XTALxxM) has settled. If it is switch system clock to fast XTAL
 *        clock (XTALxxM)
 */
__RETAINED_HOT_CODE void cm_switch_to_xtalm_if_settled(void);

/**
 * \brief RCHS clocks calibration callback
 *
 * \details It is called from the sys_adc when temperature delta > RCHS_TEMP_DRIFT.
 *
 */
void cm_rchs_calibration_notify(void);

/**
 * \brief RC clocks calibration mechanism init
 */
void cm_rc_clocks_calibration_task_init(void);
#endif /* OS_PRESENT */

/**
 * \brief Restore the system clock (unprotected).
 *
 * \details It attempts to restore the sys_clk to PLL. Is is assumed that the
 *          system runs at fast XTAL clock (XTALxxM).
 *
 * \param[in] prev_sysclk The sys_clk to use.
 *
 * \warning It must be called from Interrupt Context and/or with all interrupts disabled.
 *          The function is internal to the system and should not be used externally!
 */
void cm_sys_restore_sysclk(sys_clk_t prev_sysclk);


#endif /* SYS_CLOCK_MGR_INTERNAL_H_ */

/**
 \}
 \}
 \}
 \}
 */
