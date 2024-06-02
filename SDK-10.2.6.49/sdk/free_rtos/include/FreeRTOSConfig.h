
/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * Copyright (c) 2021 Modified by Dialog Semiconductor
 *
 * 1 tab == 4 spaces!
 *
 * Copyright (c) 2021-2022 Modified by Dialog Semiconductor
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

/* Ensure stdint is only used by the compiler, and not the assembler. */
#include <stdint.h>
#include "hw_watchdog.h"
#include "../../system/sys_man/sys_clock_mgr_internal.h"
#include "../../system/sys_man/sys_timer_internal.h"

/* =========================== System configuration ============================ */

#ifdef FREERTOS_MPU
    #define configENABLE_MPU                            1
#else
    #define configENABLE_MPU                            0
#endif

#define configENABLE_TRUSTZONE                          0

#if (__FPU_USED == 1U)
    #define configENABLE_FPU                            1
#else
    #define configENABLE_FPU                            0
#endif

#ifndef CONFIG_FREERTOS_HEAP_ALGO
    #define CONFIG_FREERTOS_HEAP_ALGO                   4
#endif

extern uint32_t SystemCoreClock;
#define configCPU_CLOCK_HZ                              ( SystemCoreClock )

/* ========================= DEVICE FAMILY definitions ========================== */


#define configSYSTICK_CLOCK_HZ                      ( lp_clock_hz )
#define configTICK_RATE_HZ                          ( ( TickType_t ) lp_tick_rate_hz )
#define TICK_PERIOD                                 ( (uint32_t) lp_tick_period )

#if (dg_configXTAL32K_FREQ == 32000)
    #define LP_configSYSTICK_CLOCK_HZ               ( 32000 )
    #define LP_configTICK_RATE_HZ                   ( ( TickType_t ) 500 )
    #define LP_TICK_PERIOD                          ((LP_configSYSTICK_CLOCK_HZ / LP_configTICK_RATE_HZ))
#elif (dg_configXTAL32K_FREQ == 32768)
    #define LP_configSYSTICK_CLOCK_HZ               ( 32768 )
    #define LP_configTICK_RATE_HZ                   ( ( TickType_t ) 512 )
    #define LP_TICK_PERIOD                          ((LP_configSYSTICK_CLOCK_HZ / LP_configTICK_RATE_HZ))
#endif /* dg_configXTAL32K_FREQ */
    /* Override the default pdMS_TO_TICKS implementation in projdefs.h */
    #define pdMS_TO_TICKS( xTimeInMs )                  ( ( TickType_t ) ( ( ( uint64_t ) ( xTimeInMs ) * ( TickType_t ) ( configTICK_RATE_HZ ) + 1000U / 2 ) / ( TickType_t ) 1000U ) )

    /* Cortex-M specific definitions. */
    #define configPRIO_BITS                             __NVIC_PRIO_BITS

    /* The lowest interrupt priority that can be used in a call to a "set priority" function. */
    #define configLIBRARY_LOWEST_INTERRUPT_PRIORITY     ((1 << configPRIO_BITS) - 1)

/* =============================== TASKS / QUEUES =============================== */

#define configUSE_PREEMPTION                            1
#define configMINIMAL_STACK_SIZE                        ( ( unsigned short ) 100 + ( dg_configSYSTEMVIEW_STACK_OVERHEAD / ( sizeof(StackType_t) ) ) )
                                                        /* dg_configSYSTEMVIEW_STACK_OVERHEAD is in BYTES.
                                                         * the configMINIMAL_STACK_SIZE expresses the size in
                                                         * multiples of sizeof(StackType_t) (in words).
                                                         */
#ifndef configTOTAL_HEAP_SIZE
    #define configTOTAL_HEAP_SIZE                       ( ( size_t ) ( 7168 ) ) //  ( 6500 ) )
#endif
#define configMAX_TASK_NAME_LEN                         ( 16 )
#define configMAX_PRIORITIES                            ( 7 )

#if (dg_configOS_ENABLE_THREAD_AWARENESS == 1)
    #define configUSE_TRACE_FACILITY                    1
    #define configRECORD_STACK_HIGH_ADDRESS             1
    #define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H   1

    #define FREERTOS_TASKS_C_ADDITIONS_INIT()                                      \
        do {                                                                       \
            while (FreeRTOSDebugConfig[0] != FREERTOS_DEBUG_CONFIG_MAJOR_VERSION); \
        } while (0);

    #define configGENERATE_RUN_TIME_STATS               1
    #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()    {}
    extern unsigned long vGetRunTimeCounterValue(void);
    #define portGET_RUN_TIME_COUNTER_VALUE()            vGetRunTimeCounterValue()
#else
    #define configUSE_TRACE_FACILITY                    0
    #define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H   0
    #define configGENERATE_RUN_TIME_STATS               0
#endif /* dg_configOS_ENABLE_THREAD_AWARENESS */

#define configUSE_16_BIT_TICKS                          0
#define configIDLE_SHOULD_YIELD                         1
#define configUSE_MUTEXES                               1
#define configQUEUE_REGISTRY_SIZE                       8
#define configCHECK_FOR_STACK_OVERFLOW                  2
#define configUSE_RECURSIVE_MUTEXES                     1
#define configUSE_APPLICATION_TASK_TAG                  0
#define configUSE_COUNTING_SEMAPHORES                   1
#define configUSE_QUEUE_SETS                            1

/* ================================ IDLE / SLEEP ================================ */

#define configUSE_TICKLESS_IDLE                         2
/* The minimum allowed value for configEXPECTED_IDLE_TIME_BEFORE_SLEEP is 2. */
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP           2
#define portSUPPRESS_TICKS_AND_SLEEP( x )               prvSystemSleep( x )
#define configPRE_STOP_PROCESSING()
#define configPRE_SLEEP_PROCESSING( x )
#define configPOST_SLEEP_PROCESSING()
#define configPRE_IDLE_ENTRY( x )                       /*cm_lower_all_clocks()*/
#define configPOST_IDLE_ENTRY( x )                      /*cm_restore_all_clocks()*/

/* ==================================== Hooks =================================== */
#define configUSE_MALLOC_FAILED_HOOK                    1
#define configUSE_TICK_HOOK                             0
#if ((dg_configTRACK_OS_HEAP == 1) || (dg_configUSE_WDOG == 1))
    #define configUSE_IDLE_HOOK                         1
#else
    #define configUSE_IDLE_HOOK                         0
#endif

/* =========================== Co-routine definitions =========================== */

#define configUSE_CO_ROUTINES                           0
#define configMAX_CO_ROUTINE_PRIORITIES                 ( 2 )

/* ========================= Software timer definitions ========================= */

#define configUSE_TIMERS                                1
#define configTIMER_TASK_PRIORITY                       ( configMAX_PRIORITIES - 1 )
#ifndef configTIMER_QUEUE_LENGTH
    #define configTIMER_QUEUE_LENGTH                    6
#endif
#ifndef configTIMER_TASK_STACK_DEPTH
    #define configTIMER_TASK_STACK_DEPTH                ( configMINIMAL_STACK_SIZE )
#endif

/* ================================ API functions =============================== */

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_xTaskGetIdleTaskHandle                  1
#define INCLUDE_vTaskPrioritySet                        1
#define INCLUDE_uxTaskPriorityGet                       1
#define INCLUDE_vTaskDelete                             1
#define INCLUDE_vTaskCleanUpResources                   1
#define INCLUDE_vTaskSuspend                            1
#define INCLUDE_vTaskDelayUntil                         1
#define INCLUDE_vTaskDelay                              1
#define INCLUDE_eTaskGetState                           1
#define INCLUDE_xEventGroupSetBitFromISR                1
#define INCLUDE_xTimerPendFunctionCall                  1

#if (dg_configTRACK_OS_HEAP == 1)
    #define INCLUDE_uxTaskGetStackHighWaterMark         1
#endif

#if (dg_configSYSTEMVIEW == 1)
    #define INCLUDE_xTaskGetCurrentTaskHandle           1
    #define INCLUDE_pxTaskGetStackStart                 1

    #include "SEGGER_SYSVIEW_FreeRTOS.h"
#endif /* (dg_configSYSTEMVIEW == 1) */

#if (dg_configENABLE_TASK_MONITORING == 1)
    #define INCLUDE_uxTaskGetStackHighWaterMark         1
    #define INCLUDE_eTaskGetState                       1
    #define INCLUDE_uxTaskPriorityGet                   1
    #if (dg_configOS_ENABLE_THREAD_AWARENESS == 0)
        #undef configUSE_TRACE_FACILITY
        #define configUSE_TRACE_FACILITY                1
    #endif
#endif /*dg_configENABLE_TASK_MONITORING */

/* ================================ ASSERT config =============================== */

/* Normal assert() semantics without relying on the provision of an assert.h
 * header file.
 */
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
    __RETAINED_CODE void config_assert(void* args);
    #define configASSERT( x ) if ( ( x ) == 0 ) { __PUSH_SCRATCH_REGISTERS(); config_assert((void *)__get_SP()); }
#else
    #define configASSERT( x ) if ( ( x ) == 0 ) { }
#endif

/* ================================= IRQ Handlers =============================== */

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
 * standard names - or at least those used in the unmodified vector table.
 */
#define vPortSVCHandler                                 SVC_Handler
#define xPortPendSVHandler                              PendSV_Handler
#define xPortSysTickHandler                             SysTick_Handler

/* ======================== Cortex-M specific definitions ======================= */

/* The highest interrupt priority that can be used by any interrupt service
 * routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
 * INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
 * PRIORITY THAN THIS! (higher priorities are lower numeric values on an ARM Cortex-M).
 */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    1

/* Interrupt priorities used by the kernel port layer itself.  These are generic
 * to all Cortex-M ports, and do not rely on any particular library functions.
 */
#define configKERNEL_INTERRUPT_PRIORITY                 (configLIBRARY_LOWEST_INTERRUPT_PRIORITY<<(8-configPRIO_BITS))

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 * See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html.
 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY            (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY<<(8-configPRIO_BITS))

#endif /* FREERTOS_CONFIG_H */
