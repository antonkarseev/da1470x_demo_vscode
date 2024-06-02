/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup BSP_DEBUG Debug Configuration
 * \brief Board debug support configuration definitions
 * \{
*/

/**
 ****************************************************************************************
 *
 * @file bsp_debug.h
 *
 * @brief Board Support Package. Debug Configuration.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BSP_DEBUG_H_
#define BSP_DEBUG_H_

/**
 * \addtogroup DEBUG_SETTINGS
 *
 * \brief Debugging settings
 *
 * \{
 */
/* -------------------------------------- Debug settings ---------------------------------------- */

/**
 * \brief Enable debugger
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configENABLE_DEBUGGER
#define dg_configENABLE_DEBUGGER                (1)
#endif

/**
 * \brief Enable OS thread aware debugging
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configOS_ENABLE_THREAD_AWARENESS
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
#define dg_configOS_ENABLE_THREAD_AWARENESS     (1)
#else
#define dg_configOS_ENABLE_THREAD_AWARENESS     (0)
#endif
#endif

/**
 * \brief Enable FreeRTOS thread aware debugging
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \deprecated This macro is deprecated. Use dg_configOS_ENABLE_THREAD_AWARENESS instead
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configFREERTOS_ENABLE_THREAD_AWARENESS
#define dg_configFREERTOS_ENABLE_THREAD_AWARENESS dg_configOS_ENABLE_THREAD_AWARENESS
#else
#undef dg_configOS_ENABLE_THREAD_AWARENESS
#define dg_configOS_ENABLE_THREAD_AWARENESS dg_configFREERTOS_ENABLE_THREAD_AWARENESS
#pragma message "dg_configFREERTOS_ENABLE_THREAD_AWARENESS is obsolete! Use dg_configOS_ENABLE_THREAD_AWARENESS instead"
#endif

/**
 * \brief Enable CMAC debugger
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configENABLE_CMAC_DEBUGGER
#define dg_configENABLE_CMAC_DEBUGGER           (1)
#endif

/**
 * \brief Enable SNC debugger
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configENABLE_SNC_DEBUGGER
#define dg_configENABLE_SNC_DEBUGGER            (1)
#endif

/**
 * \brief Enable SNC sleep status
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configENABLE_SNC_SLEEP_STATUS
#define dg_configENABLE_SNC_SLEEP_STATUS        (0)
#endif

#ifdef CONFIG_USE_SNC
#define SNC_NEVER_STARTED                   (0)            /* SNC is disabled or SNC debug status is not enabled */
#define SNC_ACTIVE_RESET_HANDLER            (1 << 0)       /* SNC is still active after calling Reset_Handler() */
#define SNC_ACTIVE_AFTER_DEEPSLEEP          (1 << 1)       /* SNC is unable to go to sleep after calling goto_deepsleep() */
#define SNC_ACTIVE_WAKUP_FROM_DEEPSLEEP     (1 << 2)       /* SNC woke up from wakeup_from_deepsleep() */
#define SNC_SLEPT_GOTO_DEEPSLEEP            (1 << 3)       /* SNC is slept by goto_deepsleep() */
#define SNC_SLEPT_UNINTENDED_WKUP           (1 << 4)       /* SNC is slept by unintended_wakeup() */
#endif /*CONFIG_USE_SNC */

/**
 * \brief Use SW cursor
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SW_CURSOR
#define dg_configUSE_SW_CURSOR                  (0)
#define SW_CURSOR_PORT                          (0)
#define SW_CURSOR_PIN                           (0)
#else
#               if !defined SW_CURSOR_PORT  &&  !defined SW_CURSOR_PIN
#                       define SW_CURSOR_PORT   (0)
#                       define SW_CURSOR_PIN    (6)
#               endif
#endif

/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */

/**
 * \addtogroup DEBUG_SETTINGS
 * \{
 * \addtogroup SYSTEM_VIEW
 *
 * \brief System View configuration settings
 * \{
 */

/* ----------------------------- Segger System View configuration ------------------------------- */

/**
 * \brief Segger's System View
 *
 * When enabled the application should also call SEGGER_SYSVIEW_Conf() to enable system monitoring.
 * OS_TOTAL_HEAP_SIZE should be increased by dg_configSYSTEMVIEW_STACK_OVERHEAD bytes for each system task.
 * For example, if there are 8 system tasks OS_TOTAL_HEAP_SIZE should be increased by
 * (8 * dg_configSYSTEMVIEW_STACK_OVERHEAD) bytes.
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app,}
 *
 */
#ifndef dg_configSYSTEMVIEW
#define dg_configSYSTEMVIEW                     (0)
#endif

/**
 * \brief Stack size overhead when System View API is used
 *
 * All thread stack sizes plus the the stack of IRQ handlers will be increased by
 * dg_configSYSTEMVIEW_STACK_OVERHEAD Bytes to avoid stack overflow when
 * System View is monitoring the system.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#if dg_configSYSTEMVIEW
# ifndef dg_configSYSTEMVIEW_STACK_OVERHEAD
#  define dg_configSYSTEMVIEW_STACK_OVERHEAD    (256)         /* in Bytes */
# endif
#else
# undef dg_configSYSTEMVIEW_STACK_OVERHEAD
# define dg_configSYSTEMVIEW_STACK_OVERHEAD     (0)
#endif /* dg_configSYSTEMVIEW */

/*
 * Enable/Disable System View monitoring time critical interrupt handlers (BLE, CPM, USB).
 * Disabling ISR monitoring could help reducing assertions triggered by System View monitoring overhead.
 *
 */

/**
 * \brief Let System View monitor BLE related ISRs (BLE_GEN_Handler / BLE_WAKEUP_LP_Handler).
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYSTEMVIEW_MONITOR_BLE_ISR
#define dg_configSYSTEMVIEW_MONITOR_BLE_ISR     (1)
#endif

/**
 * \brief Let System View monitor CPM related ISRs (SWTIM1_Handler / WKUP_GPIO_Handler).
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYSTEMVIEW_MONITOR_CPM_ISR
#define dg_configSYSTEMVIEW_MONITOR_CPM_ISR     (1)
#endif

/**
 * \brief Let System View monitor USB related ISRs (USB_Handler / VBUS_Handler).
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYSTEMVIEW_MONITOR_USB_ISR
#define dg_configSYSTEMVIEW_MONITOR_USB_ISR     (1)
#endif


/**
 * \brief Set the BASEPRI mask to be used for the SystemView protection from interrupts
 *        and task switching.
 *        The dg_configSEGGER_RTT_MAX_INTERRUPT_PRIORITY must be
 *        less or equal to configMAX_SYSCALL_INTERRUPT_PRIORITY
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#if dg_configSYSTEMVIEW == 1
  /* The SEGGER_RTT_MAX_INTERRUPT_PRIORITY must be less or equal to configMAX_SYSCALL_INTERRUPT_PRIORITY */
  #ifndef dg_configSEGGER_RTT_MAX_INTERRUPT_PRIORITY
    #define SEGGER_RTT_MAX_INTERRUPT_PRIORITY   (0x10)
  #else
    #define SEGGER_RTT_MAX_INTERRUPT_PRIORITY   (dg_configSEGGER_RTT_MAX_INTERRUPT_PRIORITY)
  #endif
#endif
/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 * \}
 */

/* --------------------------------- DEBUG GPIO handling macros --------------------------------- */

#define REG32P(reg)                     (volatile uint32_t *)(&(reg))

#define DBG_GET_SET_PAD_LATCH_REG(name)                                         \
        *((REG32P(name##_SET_REG) == REG32P(GPIO->P0_SET_DATA_REG))             \
                ? &CRG_TOP->P0_SET_PAD_LATCH_REG                                \
                : (REG32P(name##_SET_REG) == REG32P(GPIO->P1_SET_DATA_REG))     \
                        ? &CRG_TOP->P1_SET_PAD_LATCH_REG                        \
                        : &CRG_TOP->P2_SET_PAD_LATCH_REG)

#define DBG_GET_RESET_PAD_LATCH_REG(name)                                       \
        *((REG32P(name##_RESET_REG) == REG32P(GPIO->P0_RESET_DATA_REG))         \
                ? &CRG_TOP->P0_RESET_PAD_LATCH_REG                              \
                : (REG32P(name##_RESET_REG) == REG32P(GPIO->P1_RESET_DATA_REG)) \
                        ? &CRG_TOP->P1_RESET_PAD_LATCH_REG                      \
                        : &CRG_TOP->P2_RESET_PAD_LATCH_REG)

#define DBG_TOGGLE_PIN_PAD_LATCH(name)                                           \
do {                                                                             \
        DBG_GET_SET_PAD_LATCH_REG(name) = name##_PIN;                            \
        DBG_GET_RESET_PAD_LATCH_REG(name) = name##_PIN;                          \
} while (0)

#define DBG_SET_PIN_REG(name)             name##_SET_REG = name##_PIN
#define DBG_RESET_PIN_REG(name)           name##_RESET_REG = name##_PIN

#define DBG_CONFIGURE(flag, name, func)                                          \
do {                                                                             \
        if (flag == 1) {                                                         \
                name##_MODE_REG = 0x300 + func;                                  \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
} while (0)

#define DBG_CONFIGURE_HIGH(flag, name)                                           \
do {                                                                             \
        if (flag == 1) {                                                         \
                name##_MODE_REG = 0x300;                                         \
                DBG_SET_PIN_REG(name);                                           \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
} while (0)

#define DBG_CONFIGURE_LOW(flag, name)                                            \
do {                                                                             \
        if (flag == 1) {                                                         \
                name##_MODE_REG = 0x300;                                         \
                DBG_RESET_PIN_REG(name);                                         \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
} while (0)

#define DBG_SET_HIGH(flag, name)                                                 \
do {                                                                             \
        if (flag == 1) {                                                         \
                name##_MODE_REG = 0x300;                                         \
                DBG_SET_PIN_REG(name);                                           \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
} while (0)

#define DBG_SET_LOW(flag, name)                                                  \
do {                                                                             \
        if (flag == 1) {                                                         \
                name##_MODE_REG = 0x300;                                         \
                DBG_RESET_PIN_REG(name);                                         \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
} while (0)


/* ---------------------------------------------------------------------------------------------- */


/* ---------------------------------- HardFault or NMI event ------------------------------------ */

#ifndef EXCEPTION_DEBUG
#define EXCEPTION_DEBUG                         (0)     // Requires GPIO config.
#endif

/* ---------------------------------------------------------------------------------------------- */


/* --------------------------------- Clock and Power Manager ------------------------------------ */


#ifndef PWR_MGR_DEBUG
#define PWR_MGR_DEBUG                           (0)
#endif

#ifndef SYS_TIM_DEBUG
#define SYS_TIM_DEBUG                           (0)
#endif

#ifndef PWR_MGR_FUNCTIONAL_DEBUG
#define PWR_MGR_FUNCTIONAL_DEBUG                (0)     // Requires GPIO config.
#endif

#ifndef PWR_MGR_USE_TIMING_DEBUG
#define PWR_MGR_USE_TIMING_DEBUG                (0)     // Requires GPIO config.
#endif

#ifndef CLK_MGR_USE_TIMING_DEBUG
#define CLK_MGR_USE_TIMING_DEBUG                (0)     // Requires GPIO config.
#endif

#ifndef RC_CLK_CALIBRATION_DEBUG
#define RC_CLK_CALIBRATION_DEBUG                (0)     // Requires GPIO config.
#endif


/* Controls which RAM blocks will be retained when the MEASURE_SLEEP_CURRENT test mode is used
 * (optional). */
#ifndef dg_configTESTMODE_RETAIN_RAM
#define dg_configTESTMODE_RETAIN_RAM            (0x1F)
#endif

/* Controls whether the Cache will be retained when the MEASURE_SLEEP_CURRENT test mode is used
 * (optional). */
#ifndef dg_configTESTMODE_RETAIN_CACHE
#define dg_configTESTMODE_RETAIN_CACHE          (0)
#endif

/* Controls whether the ECC RAM will be retained when the MEASURE_SLEEP_CURRENT test mode is used
 * (optional). */
#ifdef dg_config_TESTMODE_RETAIN_ECCRAM
#error "dg_config_TESTMODE_RETAIN_ECCRAM is no longer supported. "
       "Use dg_configTESTMODE_RETAIN_ECCRAM instead (no underscore)!"
#endif

#ifndef dg_configTESTMODE_RETAIN_ECCRAM
#define dg_configTESTMODE_RETAIN_ECCRAM        (0)
#endif

/* ---------------------------------------------------------------------------------------------- */


/* --------------------------------------- USB Charger ------------------------------------------ */


#ifndef SYS_CHARGER_TIMING_DEBUG
#define SYS_CHARGER_TIMING_DEBUG                (0)
#endif


/* ---------------------------------------------------------------------------------------------- */


/* ------------------------------------------- BLE ---------------------------------------------- */


#ifndef BLE_ADAPTER_DEBUG
#define BLE_ADAPTER_DEBUG                       (0)     // Requires GPIO config.
#endif

#define BLE_RX_EN_FUNC                          (57)

#ifndef BLE_WINDOW_STATISTICS
#define BLE_WINDOW_STATISTICS                   (0)
#endif

#ifndef BLE_SLEEP_PERIOD_DEBUG
#define BLE_SLEEP_PERIOD_DEBUG                  (0)     // Requires logging and window statistics.
#endif

#ifndef BLE_WAKEUP_MONITOR_PERIOD
#define BLE_WAKEUP_MONITOR_PERIOD               (1024)
#endif

#ifndef BLE_MAX_MISSES_ALLOWED
#define BLE_MAX_MISSES_ALLOWED                  (0)
#endif

#ifndef BLE_MAX_DELAYS_ALLOWED
#define BLE_MAX_DELAYS_ALLOWED                  (0)
#endif

#ifndef BLE_SSP_DEBUG
#define BLE_SSP_DEBUG                           (0)
#endif

/* ---------------------------------------------------------------------------------------------- */


/* ------------------------------------------ Flash --------------------------------------------- */
#ifndef FLASH_DEBUG
#define FLASH_DEBUG                             (0)     // Requires GPIO config.
#endif

#ifndef __DBG_OQSPI_ENABLED
#define __DBG_OQSPI_ENABLED                     (0)
#endif

#ifndef __DBG_QSPI_ENABLED
#define __DBG_QSPI_ENABLED                      (0)
#endif

/* ---------------------------------------------------------------------------------------------- */


/* ------------------------------------------ Common -------------------------------------------- */
#ifndef CMN_TIMING_DEBUG
#define CMN_TIMING_DEBUG                        (0)     // Requires GPIO config.
#endif
/* ---------------------------------------------------------------------------------------------- */

/* ------------------------------------ GPIO configuration -------------------------------------- */

/* Enable/Disable GPIO pin assignment conflict detection
 */
#define DEBUG_GPIO_ALLOC_MONITOR_ENABLED        (0)


/* Exception handling debug configuration
 *
 */
#if (EXCEPTION_DEBUG == 0)
// Dummy values to suppress compiler errors
#define EXCEPTIONDBG_MODE_REG                   *(volatile int *)0x20000000
#define EXCEPTIONDBG_SET_REG                    *(volatile int *)0x20000000
#define EXCEPTIONDBG_RESET_REG                  *(volatile int *)0x20000000
#define EXCEPTIONDBG_PIN                        (0)

#else


// Tick
#error "EXCEPTIONDBG_XXX not defined yet!"
/* #define EXCEPTIONDBG_MODE_REG                   GPIO->P0_30_MODE_REG */
/* #define EXCEPTIONDBG_SET_REG                    GPIO->P0_SET_DATA_REG */
/* #define EXCEPTIONDBG_RESET_REG                  GPIO->P0_RESET_DATA_REG */
/* #define EXCEPTIONDBG_PIN                        (1 << 30) */


#endif /* EXCEPTION_DEBUG */

/* Functional debug configuration
 *
 * Note that GPIO overlapping is allowed if the tracked events are discrete and the initial GPIO
 * configuration is the same! No checking is performed for erroneous configuration though!
 *
 */

#if (PWR_MGR_FUNCTIONAL_DEBUG == 0)
// Dummy values to suppress compiler errors
#define PWRDBG_TICK_MODE_REG                    *(volatile int *)0x20000000
#define PWRDBG_TICK_SET_REG                     *(volatile int *)0x20000000
#define PWRDBG_TICK_RESET_REG                   *(volatile int *)0x20000000
#define PWRDBG_TICK_PIN                         (0)

#define PWRDBG_POWERUP_MODE_REG                 *(volatile int *)0x20000000
#define PWRDBG_POWERUP_SET_REG                  *(volatile int *)0x20000000
#define PWRDBG_POWERUP_RESET_REG                *(volatile int *)0x20000000
#define PWRDBG_POWERUP_PIN                      (0)

#else

// Tick
#define PWRDBG_TICK_MODE_REG                    GPIO->P0_19_MODE_REG
#define PWRDBG_TICK_SET_REG                     GPIO->P0_SET_DATA_REG
#define PWRDBG_TICK_RESET_REG                   GPIO->P0_RESET_DATA_REG
#define PWRDBG_TICK_PIN                         (1 << 19)

// Active / Sleep
#define PWRDBG_POWERUP_MODE_REG                 GPIO->P0_21_MODE_REG
#define PWRDBG_POWERUP_SET_REG                  GPIO->P0_SET_DATA_REG
#define PWRDBG_POWERUP_RESET_REG                GPIO->P0_RESET_DATA_REG
#define PWRDBG_POWERUP_PIN                      (1 << 21)

#endif /* PWR_MGR_FUNCTIONAL_DEBUG */


/* Timing debug configuration
 *
 * Note that in this mode the pad latches are removed immediately after the execution resumes from
 * the __WFI(). Because of this, it is not advised to use this feature in projects that use GPIOS.
 * Nevertheless, in case it is used, make sure that the "peripheral initialization" is also done
 * at that point, modifying sys_power_mgr.c accordingly.
 *
 * Note also that GPIO overlapping is allowed if the tracked events are discrete and the initial
 * GPIO configuration is the same! No checking is performed for erroneous configuration though!
 *
 */

#if (PWR_MGR_USE_TIMING_DEBUG == 0)
// Dummy values to suppress compiler errors
#define PWRDBG_SLEEP_ENTER_MODE_REG             *(volatile int *)0x20000000
#define PWRDBG_SLEEP_ENTER_SET_REG              *(volatile int *)0x20000000
#define PWRDBG_SLEEP_ENTER_RESET_REG            *(volatile int *)0x20000000
#define PWRDBG_SLEEP_ENTER_PIN                  (0)

#define PWRDBG_SLEEP_EXIT_MODE_REG              *(volatile int *)0x20000000
#define PWRDBG_SLEEP_EXIT_SET_REG               *(volatile int *)0x20000000
#define PWRDBG_SLEEP_EXIT_RESET_REG             *(volatile int *)0x20000000
#define PWRDBG_SLEEP_EXIT_PIN                   (0)

#else

// Power manager: sleep or idle entry (until __WFI() is called)
#define PWRDBG_SLEEP_ENTER_MODE_REG             GPIO->P0_17_MODE_REG
#define PWRDBG_SLEEP_ENTER_SET_REG              GPIO->P0_SET_DATA_REG
#define PWRDBG_SLEEP_ENTER_RESET_REG            GPIO->P0_RESET_DATA_REG
#define PWRDBG_SLEEP_ENTER_PIN                  (1 << 17)

// Power manager: sleep or idle exit
#define PWRDBG_SLEEP_EXIT_MODE_REG              GPIO->P0_18_MODE_REG
#define PWRDBG_SLEEP_EXIT_SET_REG               GPIO->P0_SET_DATA_REG
#define PWRDBG_SLEEP_EXIT_RESET_REG             GPIO->P0_RESET_DATA_REG
#define PWRDBG_SLEEP_EXIT_PIN                   (1 << 18)

#endif /* CLK_MGR_USE_TIMING_DEBUG */

#if (CLK_MGR_USE_TIMING_DEBUG == 0)
// Dummy values to suppress compiler errors
#define CLKDBG_LOWER_CLOCKS_MODE_REG            *(volatile int *)0x20000000
#define CLKDBG_LOWER_CLOCKS_SET_REG             *(volatile int *)0x20000000
#define CLKDBG_LOWER_CLOCKS_RESET_REG           *(volatile int *)0x20000000
#define CLKDBG_LOWER_CLOCKS_PIN                 (0)

#define CLKDBG_XTAL32M_SETTLED_MODE_REG         *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_SETTLED_SET_REG          *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_SETTLED_RESET_REG        *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_SETTLED_PIN              (0)

#define CLKDBG_XTAL32M_ISR_MODE_REG             *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_ISR_SET_REG              *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_ISR_RESET_REG            *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_ISR_PIN                  (0)

#define CLKDBG_XTAL32M_READY_MODE_REG           *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_READY_SET_REG            *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_READY_RESET_REG          *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_READY_PIN                (0)

#define CLKDBG_PLL_ON_MODE_REG                  *(volatile int *)0x20000000
#define CLKDBG_PLL_ON_SET_REG                   *(volatile int *)0x20000000
#define CLKDBG_PLL_ON_RESET_REG                 *(volatile int *)0x20000000
#define CLKDBG_PLL_ON_PIN                       (0)

#else

// Low clocks
#define CLKDBG_LOWER_CLOCKS_MODE_REG            GPIO->P0_15_MODE_REG
#define CLKDBG_LOWER_CLOCKS_SET_REG             GPIO->P0_SET_DATA_REG
#define CLKDBG_LOWER_CLOCKS_RESET_REG           GPIO->P0_RESET_DATA_REG
#define CLKDBG_LOWER_CLOCKS_PIN                 (1 << 15)

// XTAL32M settling
#define CLKDBG_XTAL32M_SETTLED_MODE_REG         GPIO->P0_21_MODE_REG
#define CLKDBG_XTAL32M_SETTLED_SET_REG          GPIO->P0_SET_DATA_REG
#define CLKDBG_XTAL32M_SETTLED_RESET_REG        GPIO->P0_RESET_DATA_REG
#define CLKDBG_XTAL32M_SETTLED_PIN              (1 << 21)

// XTAL32M ISR
#define CLKDBG_XTAL32M_ISR_MODE_REG             GPIO->P0_22_MODE_REG
#define CLKDBG_XTAL32M_ISR_SET_REG              GPIO->P0_SET_DATA_REG
#define CLKDBG_XTAL32M_ISR_RESET_REG            GPIO->P0_RESET_DATA_REG
#define CLKDBG_XTAL32M_ISR_PIN                  (1 << 22)

// XTAL32M Ready
#define CLKDBG_XTAL32M_READY_MODE_REG           GPIO->P0_23_MODE_REG
#define CLKDBG_XTAL32M_READY_SET_REG            GPIO->P0_SET_DATA_REG
#define CLKDBG_XTAL32M_READY_RESET_REG          GPIO->P0_RESET_DATA_REG
#define CLKDBG_XTAL32M_READY_PIN                (1 << 23)

// PLL is on
#define CLKDBG_PLL_ON_MODE_REG                  GPIO->P0_24_MODE_REG
#define CLKDBG_PLL_ON_SET_REG                   GPIO->P0_SET_DATA_REG
#define CLKDBG_PLL_ON_RESET_REG                 GPIO->P0_RESET_DATA_REG
#define CLKDBG_PLL_ON_PIN                       (1 << 24)

#endif /* CLK_MGR_USE_TIMING_DEBUG */



#if (BLE_ADAPTER_DEBUG == 0)
#define BLEBDG_ADAPTER_MODE_REG                 *(volatile int *)0x20000000
#define BLEBDG_ADAPTER_SET_REG                  *(volatile int *)0x20000000
#define BLEBDG_ADAPTER_RESET_REG                *(volatile int *)0x20000000
#define BLEBDG_ADAPTER_PIN                      (0)

#else


#endif /* BLE_ADAPTER_DEBUG */


#if (SYS_CHARGER_TIMING_DEBUG == 0)

#define SYS_CHARGER_DBG_VBUS_MODE_REG           *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_VBUS_SET_REG            *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_VBUS_RESET_REG          *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_VBUS_PIN                (0)

#define SYS_CHARGER_DBG_CH_EVT_MODE_REG         *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_EVT_SET_REG          *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_EVT_RESET_REG        *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_EVT_PIN              (0)

#define SYS_CHARGER_DBG_PRE_CH_MODE_REG         *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_PRE_CH_SET_REG          *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_PRE_CH_RESET_REG        *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_PRE_CH_PIN              (0)

#define SYS_CHARGER_DBG_CH_MODE_REG             *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_SET_REG              *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_RESET_REG            *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_PIN                  (0)

#define SYS_CHARGER_DBG_EOC_MODE_REG            *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_EOC_SET_REG             *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_EOC_RESET_REG           *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_EOC_PIN                 (0)

#define SYS_CHARGER_DBG_ENUM_DONE_MODE_REG      *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_ENUM_DONE_SET_REG       *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_ENUM_DONE_RESET_REG     *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_ENUM_DONE_PIN           (0)

#define SYS_CHARGER_DBG_SUS_MODE_REG             *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_SUS_SET_REG              *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_SUS_RESET_REG            *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_SUS_PIN                  (0)

#else

#define SYS_CHARGER_DBG_VBUS_MODE_REG           GPIO->P0_30_MODE_REG
#define SYS_CHARGER_DBG_VBUS_SET_REG            GPIO->P0_SET_DATA_REG
#define SYS_CHARGER_DBG_VBUS_RESET_REG          GPIO->P0_RESET_DATA_REG
#define SYS_CHARGER_DBG_VBUS_PIN                (1 << 30)

#define SYS_CHARGER_DBG_CH_EVT_MODE_REG         GPIO->P0_31_MODE_REG
#define SYS_CHARGER_DBG_CH_EVT_SET_REG          GPIO->P0_SET_DATA_REG
#define SYS_CHARGER_DBG_CH_EVT_RESET_REG        GPIO->P0_RESET_DATA_REG
#define SYS_CHARGER_DBG_CH_EVT_PIN              (1 << 31)

#define SYS_CHARGER_DBG_PRE_CH_MODE_REG         GPIO->P2_12_MODE_REG
#define SYS_CHARGER_DBG_PRE_CH_SET_REG          GPIO->P2_SET_DATA_REG
#define SYS_CHARGER_DBG_PRE_CH_RESET_REG        GPIO->P2_RESET_DATA_REG
#define SYS_CHARGER_DBG_PRE_CH_PIN              (1 << 12)

#define SYS_CHARGER_DBG_CH_MODE_REG             GPIO->P2_00_MODE_REG
#define SYS_CHARGER_DBG_CH_SET_REG              GPIO->P2_SET_DATA_REG
#define SYS_CHARGER_DBG_CH_RESET_REG            GPIO->P2_RESET_DATA_REG
#define SYS_CHARGER_DBG_CH_PIN                  (1 << 0)

#define SYS_CHARGER_DBG_EOC_MODE_REG            GPIO->P2_02_MODE_REG
#define SYS_CHARGER_DBG_EOC_SET_REG             GPIO->P2_SET_DATA_REG
#define SYS_CHARGER_DBG_EOC_RESET_REG           GPIO->P2_RESET_DATA_REG
#define SYS_CHARGER_DBG_EOC_PIN                 (1 << 2)

#define SYS_CHARGER_DBG_ENUM_DONE_MODE_REG      GPIO->P1_30_MODE_REG
#define SYS_CHARGER_DBG_ENUM_DONE_SET_REG       GPIO->P1_SET_DATA_REG
#define SYS_CHARGER_DBG_ENUM_DONE_RESET_REG     GPIO->P1_RESET_DATA_REG
#define SYS_CHARGER_DBG_ENUM_DONE_PIN           (1 << 30)

#define SYS_CHARGER_DBG_SUS_MODE_REG            GPIO->P1_31_MODE_REG
#define SYS_CHARGER_DBG_SUS_SET_REG             GPIO->P1_SET_DATA_REG
#define SYS_CHARGER_DBG_SUS_RESET_REG           GPIO->P1_RESET_DATA_REG
#define SYS_CHARGER_DBG_SUS_PIN                 (1 << 31)

#endif /* SYS_CHARGER_TIMING_DEBUG */


#if (CMN_TIMING_DEBUG == 0)
// Common: Inside critical section (initial configuration: low)
#define CMNDBG_CRITICAL_SECTION_MODE_REG       *(volatile int *)0x20000000
#define CMNDBG_CRITICAL_SECTION_SET_REG        *(volatile int *)0x20000000
#define CMNDBG_CRITICAL_SECTION_RESET_REG      *(volatile int *)0x20000000
#define CMNDBG_CRITICAL_SECTION_PIN            (0)

#else


// Common: Inside critical section
#error "CMNDBG_CRITICAL_SECTION_XXX not defined yet!"
/* #define CMNDBG_CRITICAL_SECTION_MODE_REG        GPIO->P0_24_MODE_REG */
/* #define CMNDBG_CRITICAL_SECTION_SET_REG         GPIO->P0_SET_DATA_REG */
/* #define CMNDBG_CRITICAL_SECTION_RESET_REG       GPIO->P0_RESET_DATA_REG */
/* #define CMNDBG_CRITICAL_SECTION_PIN             (1 << 24) */


#endif /* CMN_TIMING_DEBUG */


/* Flash debug configuration
 *
 */
#if (FLASH_DEBUG == 0)
// Write page (initial configuration: low)
#define FLASHDBG_PAGE_PROG_MODE_REG             *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_SET_REG              *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_RESET_REG            *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_PIN                  (0)

// Program page wait loop (initial configuration: low)
#define FLASHDBG_PAGE_PROG_WL_MODE_REG          *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_SET_REG           *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_RESET_REG         *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_PIN               (0)

// Program page wait loop - pending irq check (initial configuration: low)
#define FLASHDBG_PAGE_PROG_WL_IRQ_MODE_REG      *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_IRQ_SET_REG       *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_IRQ_RESET_REG     *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_IRQ_PIN           (0)

// Suspend op (initial configuration: low)
#define FLASHDBG_SUSPEND_MODE_REG               *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_SET_REG                *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_RESET_REG              *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_PIN                    (0)

// Erase sector cmd (initial configuration: low)
#define FLASHDBG_SECTOR_ERASE_MODE_REG          *(volatile int *)0x20000000
#define FLASHDBG_SECTOR_ERASE_SET_REG           *(volatile int *)0x20000000
#define FLASHDBG_SECTOR_ERASE_RESET_REG         *(volatile int *)0x20000000
#define FLASHDBG_SECTOR_ERASE_PIN               (0)

// Notify task (initial configuration: low)
#define FLASHDBG_TASK_NOTIFY_MODE_REG           *(volatile int *)0x20000000
#define FLASHDBG_TASK_NOTIFY_SET_REG            *(volatile int *)0x20000000
#define FLASHDBG_TASK_NOTIFY_RESET_REG          *(volatile int *)0x20000000
#define FLASHDBG_TASK_NOTIFY_PIN                (0)

// Suspend action (low level) (initial configuration: low)
#define FLASHDBG_SUSPEND_ACTION_MODE_REG        *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_ACTION_SET_REG         *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_ACTION_RESET_REG       *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_ACTION_PIN             (0)

// Resume op (initial configuration: low)
#define FLASHDBG_RESUME_MODE_REG                *(volatile int *)0x20000000
#define FLASHDBG_RESUME_SET_REG                 *(volatile int *)0x20000000
#define FLASHDBG_RESUME_RESET_REG               *(volatile int *)0x20000000
#define FLASHDBG_RESUME_PIN                     (0)

#else



# if dg_configUSE_HW_EMMC
# error "Flash debug pins are also used by EMMC"
# endif

# if dg_configUSE_HW_LCDC
# error "Flash debug pins are also used by LCD Controller"
# endif

# pragma message "To use the FLASH_DEBUG option in DA1470X DK consider: "       \
                 "(a) switching OFF the signals P103 - P106 of the SW2 and "    \
                 "(b) NOT connecting a board to J37 (LCDC) and J18 (MicroBUS2)"

// Write page
#define FLASHDBG_PAGE_PROG_MODE_REG             GPIO->P1_01_MODE_REG
#define FLASHDBG_PAGE_PROG_SET_REG              GPIO->P1_SET_DATA_REG
#define FLASHDBG_PAGE_PROG_RESET_REG            GPIO->P1_RESET_DATA_REG
#define FLASHDBG_PAGE_PROG_PIN                  (1 << 1)

// Program page wait loop
#define FLASHDBG_PAGE_PROG_WL_MODE_REG          GPIO->P1_02_MODE_REG
#define FLASHDBG_PAGE_PROG_WL_SET_REG           GPIO->P1_SET_DATA_REG
#define FLASHDBG_PAGE_PROG_WL_RESET_REG         GPIO->P1_RESET_DATA_REG
#define FLASHDBG_PAGE_PROG_WL_PIN               (1 << 2)

// Program Page wait loop - pending IRQ check
#define FLASHDBG_PAGE_PROG_WL_IRQ_MODE_REG      GPIO->P1_03_MODE_REG
#define FLASHDBG_PAGE_PROG_WL_IRQ_SET_REG       GPIO->P1_SET_DATA_REG
#define FLASHDBG_PAGE_PROG_WL_IRQ_RESET_REG     GPIO->P1_RESET_DATA_REG
#define FLASHDBG_PAGE_PROG_WL_IRQ_PIN           (1 << 3)

// Erase/Program Suspend operation
#define FLASHDBG_SUSPEND_MODE_REG               GPIO->P1_04_MODE_REG
#define FLASHDBG_SUSPEND_SET_REG                GPIO->P1_SET_DATA_REG
#define FLASHDBG_SUSPEND_RESET_REG              GPIO->P1_RESET_DATA_REG
#define FLASHDBG_SUSPEND_PIN                    (1 << 4)

// Erase Sector command
#define FLASHDBG_SECTOR_ERASE_MODE_REG          GPIO->P1_05_MODE_REG
#define FLASHDBG_SECTOR_ERASE_SET_REG           GPIO->P1_SET_DATA_REG
#define FLASHDBG_SECTOR_ERASE_RESET_REG         GPIO->P1_RESET_DATA_REG
#define FLASHDBG_SECTOR_ERASE_PIN               (1 << 5)

// Notify task
#define FLASHDBG_TASK_NOTIFY_MODE_REG           GPIO->P1_06_MODE_REG
#define FLASHDBG_TASK_NOTIFY_SET_REG            GPIO->P1_SET_DATA_REG
#define FLASHDBG_TASK_NOTIFY_RESET_REG          GPIO->P1_RESET_DATA_REG
#define FLASHDBG_TASK_NOTIFY_PIN                (1 << 6)

// Erase/Program Suspend operation (low level)
#define FLASHDBG_SUSPEND_ACTION_MODE_REG        GPIO->P1_07_MODE_REG
#define FLASHDBG_SUSPEND_ACTION_SET_REG         GPIO->P1_SET_DATA_REG
#define FLASHDBG_SUSPEND_ACTION_RESET_REG       GPIO->P1_RESET_DATA_REG
#define FLASHDBG_SUSPEND_ACTION_PIN             (1 << 7)

// Erase/Program Resume operation
#define FLASHDBG_RESUME_MODE_REG                GPIO->P1_08_MODE_REG
#define FLASHDBG_RESUME_SET_REG                 GPIO->P1_SET_DATA_REG
#define FLASHDBG_RESUME_RESET_REG               GPIO->P1_RESET_DATA_REG
#define FLASHDBG_RESUME_PIN                     (1 << 8)



#endif /* FLASH_DEBUG */

#if (RC_CLK_CALIBRATION_DEBUG == 0)
// RCX calibration in uCode
#define RCCLKDBG_UCODE_RCX_CAL_START_MODE_REG   *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_RCX_CAL_START_SET_REG    *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_RCX_CAL_START_RESET_REG  *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_RCX_CAL_START_PIN        (0)

// RC32K trigger set the flag from uCode
#define RCCLKDBG_UCODE_RC32K_TRIGGER_MODE_REG   *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_RC32K_TRIGGER_SET_REG    *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_RC32K_TRIGGER_RESET_REG  *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_RC32K_TRIGGER_PIN        (0)

// RCX trigger set the flag from uCode
#define RCCLKDBG_UCODE_RCX_TRIGGER_MODE_REG     *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_RCX_TRIGGER_SET_REG      *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_RCX_TRIGGER_RESET_REG    *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_RCX_TRIGGER_PIN          (0)

// XTAL32M settle from uCode
#define RCCLKDBG_UCODE_XTAL32M_SETTLE_MODE_REG  *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_XTAL32M_SETTLE_SET_REG   *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_XTAL32M_SETTLE_RESET_REG *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_XTAL32M_SETTLE_PIN       (0)

// M33 notify from uCode
#define RCCLKDBG_UCODE_M33_NOTIFY_MODE_REG      *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_M33_NOTIFY_SET_REG       *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_M33_NOTIFY_RESET_REG     *(volatile int *)0x20000000
#define RCCLKDBG_UCODE_M33_NOTIFY_PIN           (0)

// RC32K Calibration Done in M33
#define RCCLKDBG_M33_RC32K_CAL_DONE_MODE_REG    *(volatile int *)0x20000000
#define RCCLKDBG_M33_RC32K_CAL_DONE_SET_REG     *(volatile int *)0x20000000
#define RCCLKDBG_M33_RC32K_CAL_DONE_RESET_REG   *(volatile int *)0x20000000
#define RCCLKDBG_M33_RC32K_CAL_DONE_PIN         (0)

// RCX Calibration Done in M33
#define RCCLKDBG_M33_RCX_CAL_DONE_MODE_REG      *(volatile int *)0x20000000
#define RCCLKDBG_M33_RCX_CAL_DONE_SET_REG       *(volatile int *)0x20000000
#define RCCLKDBG_M33_RCX_CAL_DONE_RESET_REG     *(volatile int *)0x20000000
#define RCCLKDBG_M33_RCX_CAL_DONE_PIN           (0)

#else

// RCX calibration in uCode
#define RCCLKDBG_UCODE_RCX_CAL_START_MODE_REG   GPIO->P1_02_MODE_REG
#define RCCLKDBG_UCODE_RCX_CAL_START_SET_REG    GPIO->P1_SET_DATA_REG
#define RCCLKDBG_UCODE_RCX_CAL_START_RESET_REG  GPIO->P1_RESET_DATA_REG
#define RCCLKDBG_UCODE_RCX_CAL_START_PIN        (1 << 2)

// RC32K trigger set the flag from uCode
#define RCCLKDBG_UCODE_RC32K_TRIGGER_MODE_REG   GPIO->P1_03_MODE_REG
#define RCCLKDBG_UCODE_RC32K_TRIGGER_SET_REG    GPIO->P1_SET_DATA_REG
#define RCCLKDBG_UCODE_RC32K_TRIGGER_RESET_REG  GPIO->P1_RESET_DATA_REG
#define RCCLKDBG_UCODE_RC32K_TRIGGER_PIN        (1 << 3)

// RCX trigger set the flag from uCode
#define RCCLKDBG_UCODE_RCX_TRIGGER_MODE_REG     GPIO->P1_04_MODE_REG
#define RCCLKDBG_UCODE_RCX_TRIGGER_SET_REG      GPIO->P1_SET_DATA_REG
#define RCCLKDBG_UCODE_RCX_TRIGGER_RESET_REG    GPIO->P1_RESET_DATA_REG
#define RCCLKDBG_UCODE_RCX_TRIGGER_PIN          (1 << 4)

// XTAL32M settle from uCode
#define RCCLKDBG_UCODE_XTAL32M_SETTLE_MODE_REG  GPIO->P1_05_MODE_REG
#define RCCLKDBG_UCODE_XTAL32M_SETTLE_SET_REG   GPIO->P1_SET_DATA_REG
#define RCCLKDBG_UCODE_XTAL32M_SETTLE_RESET_REG GPIO->P1_RESET_DATA_REG
#define RCCLKDBG_UCODE_XTAL32M_SETTLE_PIN       (1 << 5)

// M33 notify from uCode
#define RCCLKDBG_UCODE_M33_NOTIFY_MODE_REG      GPIO->P1_06_MODE_REG
#define RCCLKDBG_UCODE_M33_NOTIFY_SET_REG       GPIO->P1_SET_DATA_REG
#define RCCLKDBG_UCODE_M33_NOTIFY_RESET_REG     GPIO->P1_RESET_DATA_REG
#define RCCLKDBG_UCODE_M33_NOTIFY_PIN           (1 << 6)

// RC32K Calibration Done in M33
#define RCCLKDBG_M33_RC32K_CAL_DONE_MODE_REG    GPIO->P1_07_MODE_REG
#define RCCLKDBG_M33_RC32K_CAL_DONE_SET_REG     GPIO->P1_SET_DATA_REG
#define RCCLKDBG_M33_RC32K_CAL_DONE_RESET_REG   GPIO->P1_RESET_DATA_REG
#define RCCLKDBG_M33_RC32K_CAL_DONE_PIN         (1 << 7)

// RCX Calibration Done in M33
#define RCCLKDBG_M33_RCX_CAL_DONE_MODE_REG      GPIO->P1_08_MODE_REG
#define RCCLKDBG_M33_RCX_CAL_DONE_SET_REG       GPIO->P1_SET_DATA_REG
#define RCCLKDBG_M33_RCX_CAL_DONE_RESET_REG     GPIO->P1_RESET_DATA_REG
#define RCCLKDBG_M33_RCX_CAL_DONE_PIN           (1 << 8)

#endif /* RC_CLK_CALIBRATION_DEBUG */

/* Enables the logging of stack (RW) heap memories usage.
 *
 * The feature shall only be enabled in development/debug mode
 */
#ifndef dg_configLOG_BLE_STACK_MEM_USAGE
#define dg_configLOG_BLE_STACK_MEM_USAGE                (0)
#endif


#endif /* BSP_DEBUG_H_ */

/**
\}
\}
*/
