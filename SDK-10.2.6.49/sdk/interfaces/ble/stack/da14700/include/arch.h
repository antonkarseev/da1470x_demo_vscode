/**
 ****************************************************************************************
 *
 * @file arch.h
 *
 * @brief This file contains the definitions of the macros and functions that are
 * architecture dependent.  The implementation of those is implemented in the
 * appropriate architecture directory.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 * Copyright (c) 2015-2018 Modified by Dialog Semiconductor
 *
 ****************************************************************************************
 */


#ifndef _ARCH_H_
#define _ARCH_H_

/**
 ****************************************************************************************
 * @defgroup REFIP REFIP
 * @brief Reference IP Platform
 *
 * This module contains reference platform components - REFIP.
 *
 *
 * @{
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DRIVERS DRIVERS
 * @ingroup REFIP
 * Reference IP Platform Drivers
 *
 * This module contains the necessary drivers to run the platform with the
 * RW BT SW protocol stack.
 *
 * This has the declaration of the platform architecture API.
 *
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>     // standard integer definition
#include "compiler.h"   // inline functions
#include "cmac_config_tables.h"
#if (CMAC_CPU) || defined(HOST_LIB)
#include "global_dbg.h"
#include "ll.h"         // define GLOBAL_INT_** macros as inline assembly
#endif


/*
 * CPU WORD SIZE
 ****************************************************************************************
 */
/// ARM is a 32-bit CPU
#define CPU_WORD_SIZE                           (4)

/*
 * CPU Endianness
 ****************************************************************************************
 */
/// ARM is little endian
#define CPU_LE                                  (1)

/*
 * DEBUG configuration
 ****************************************************************************************
 */
#if defined(CFG_DBG)
#define PLF_DEBUG                               (1)
#else //CFG_DBG
#define PLF_DEBUG                               (0)
#endif //CFG_DBG

/*
 * NVDS
 ****************************************************************************************
 */

/// NVDS
#ifdef CFG_NVDS
#define PLF_NVDS                                (1)
#else // CFG_NVDS
#define PLF_NVDS                                (0)
#endif // CFG_NVDS

/*
 * LLD ROM defines
 ****************************************************************************************
 */
struct lld_sleep_env_tag
{
        uint32_t irq_mask;
};

/*
 * UART
 ****************************************************************************************
 */

/// UART
#define PLF_UART                                (1)


/*
 * DEFINES
 ****************************************************************************************
 */

/// Possible errors detected by FW
#define RESET_NO_ERROR                          (0x00000000)
#define RESET_MEM_ALLOC_FAIL                    (0xF2F2F2F2)

/// Reset platform and stay in ROM
#define RESET_TO_ROM                            (0xA5A5A5A5)

/// Reset platform and reload FW
#define RESET_AND_LOAD_FW                       (0xC3C3C3C3)


/*
 * EXPORTED FUNCTION DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Fast division by 625.
 *
 * This function computes the quotient and the remainder of a division by 625.
 *
 * @param [in] q The number to be divided by 625
 * @param [out] rem The remainder of the division (q / 625)
 *
 * @return The quotient of the division (q / 625)
 ****************************************************************************************
 */
uint32_t fast_div_by_625(uint32_t q, uint32_t *rem);

/**
 ****************************************************************************************
 * @brief Fast division by 100.
 *
 * This function computes the quotient and the remainder of a division by 100.
 *
 * @param [in] q The number to be divided by 100
 * @param [out] rem The remainder of the division (q / 100)
 *
 * @return The quotient of the division (q / 100)
 *
 * @warning The function is accurate up to: MAX_SLEEP_TIME_IN_SLOTS * 48.
 *
 ****************************************************************************************
 */
uint32_t fast_div_by_100(uint32_t q, uint32_t *rem);

/**
 ****************************************************************************************
 * @brief Compute size of SW stack used.
 *
 * This function is compute the maximum size stack used by SW.
 *
 * @return Size of stack used (in bytes)
 ****************************************************************************************
 */
uint16_t get_stack_usage(void);

/**
 ****************************************************************************************
 * @brief Re-boot FW.
 *
 * This function is used to re-boot the FW when error has been detected, it is the end of
 * the current FW execution.
 * After waiting transfers on UART to be finished, and storing the information that
 * FW has re-booted by itself in a non-loaded area, the FW restart by branching at FW
 * entry point.
 *
 * Note: when calling this function, the code after it will not be executed.
 *
 * @param[in] error      Error detected by FW
 ****************************************************************************************
 */
void platform_reset(uint32_t error);



/*
 * WEAK Library functions that can be exported by the SDK
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief CMAC Platform reset hook.
 *
 * This function is provided to the SDK to execute all the required operations when the
 * Host stack reports an error that cannot be handled (i.e. memory overflow).
 *
 * Note that when the function is called, the interrupts are already disabled.
 ****************************************************************************************
 */
void platform_reset_sdk(uint32_t error);

/**
 ****************************************************************************************
 * @brief CMAC to System event hook.
 *
 * This function is provided to the SDK for any additional actions required in the 
 * CMAC2SYS_Handler().
 ****************************************************************************************
 */
void cmac2sys_notify(void);

/**
 ****************************************************************************************
 * @brief CMAC to System interrupt entry hook.
 *
 * This function is provided to the SDK for any additional actions required in the 
 * CMAC2SYS_Handler().
 ****************************************************************************************
 */
void cmac2sys_isr_enter(void);

/**
 ****************************************************************************************
 * @brief CMAC to System interrupt exit hook.
 *
 * This function is provided to the SDK for any additional actions required in the 
 * CMAC2SYS_Handler().
 ****************************************************************************************
 */
void cmac2sys_isr_exit(void);

/**
 ****************************************************************************************
 * @brief CMAC On Error critical event hook.
 *
 * This function is provided to the SDK to handle blocking errors reported by CMAC 
 * (e.g. Hardfault or Watchdog or hardware errors). In its default implementation that is
 * included in the library, the function will issue a BKPT, which will lead to a halt, if
 * a debugger is attached to the Host CPU, or a hardfault.
 ****************************************************************************************
 */
void sys_cmac_on_error_handler(void);

/**
 ****************************************************************************************
 * @brief CMAC internal Debug Event handler hook.
 *
 * This function is provided to the SDK in case it needs to override the default debug 
 * event handling that is provided by the library. It is not clear yet under which 
 * conditions this feature could be useful.
 *
 ****************************************************************************************
 */
bool internal_dbg_evt_handling(uint32_t code, uint32_t subcode);

/**
 ****************************************************************************************
 * @brief CMAC Direct Test Report Event handler hook.
 *
 * This function is provided to the SDK to process the Direct Test report that is sent by
 * CMAC in certain cases and if it has been requested to do so explicitly.
 ****************************************************************************************
 */
void hci_dbg_report_evt_process(uint8_t *payload);

/**
 ****************************************************************************************
 * @brief CMAC Wakeup-time fix event hook.
 *
 * This function is provided to the SDK to execute the required operations when CMAC 
 * determines that an update of the wakeup_time is needed.
 ****************************************************************************************
 */
void sys_proc_handler(void);

/*
 * ASSERTION CHECK
 ****************************************************************************************
 */

#if (CMAC_CPU) || defined(HOST_LIB)
/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond)                        ASSERT_ERROR(cond)

/// Assertions showing a critical error that could require a full system reset
#define ASSERT_INFO(cond, param0, param1)       ASSERT(cond)

/// Assertions showing a non-critical problem that has to be fixed by the SW
#define ASSERT_WARN(cond)                       ASSERT(cond)
#endif

#define CHECK_AND_CALL(func_ptr)                do { } while(0)

/// @} DRIVERS
/// @} REFIP
#endif // _ARCH_H_
