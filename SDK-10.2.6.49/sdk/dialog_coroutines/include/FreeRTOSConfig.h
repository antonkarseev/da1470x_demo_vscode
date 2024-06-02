
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
 * 1 tab == 4 spaces!
 */
/* Copyright (c) 2020-2022 Modified by Dialog Semiconductor */

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
 *-----------------------------------------------------------*/

/* Ensure stdint is only used by the compiler, and not the assembler. */
#include <stdint.h>

/* Dialog CoRoutines as default configuration. */
#ifndef configUSE_DIALOG_CO_ROUTINES
# define configUSE_DIALOG_CO_ROUTINES           ( 1 )
#endif

/*-----------------------------------------------------------*/

#include "sdk_defs.h"
#include "../../system/sys_man/sys_timer_internal.h"

/* OS tick definitions. */
extern uint32_t SystemCoreClock;
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )

#ifndef CONFIG_FREERTOS_HEAP_ALGO
#define CONFIG_FREERTOS_HEAP_ALGO       4
#endif


#  define configSYSTICK_CLOCK_HZ                 ( lp_clock_hz )
#  define configTICK_RATE_HZ                     ( ( TickType_t ) lp_tick_rate_hz )
#  define TICK_PERIOD                            ( ( uint32_t ) lp_tick_period )

#  if (dg_configXTAL32K_FREQ == 32000)
#   define LP_configSYSTICK_CLOCK_HZ             ( 32000 )
#   define LP_configTICK_RATE_HZ                 ( ( TickType_t ) 500 )
#   define LP_TICK_PERIOD                        ((LP_configSYSTICK_CLOCK_HZ / LP_configTICK_RATE_HZ))
#  elif (dg_configXTAL32K_FREQ == 32768)
#   define LP_configSYSTICK_CLOCK_HZ             ( 32768 )
#   define LP_configTICK_RATE_HZ                 ( ( TickType_t ) 512 )
#   define LP_TICK_PERIOD                        ((LP_configSYSTICK_CLOCK_HZ / LP_configTICK_RATE_HZ))
#  endif /* dg_configXTAL32K_FREQ */

/* Override the default pdMS_TO_TICKS implementation in projdefs.h */
# define pdMS_TO_TICKS( xTimeInMs )                  ( ( TickType_t ) ( ( ( uint64_t ) ( xTimeInMs ) * ( TickType_t ) ( configTICK_RATE_HZ ) + 1000U / 2 ) / ( TickType_t ) 1000U ) )


#if (configUSE_DIALOG_CO_ROUTINES == 1)
/* Dialog co-routine definitions. */
# define configUSE_PREEMPTION                   ( 1 )
# define configMAX_CO_ROUTINE_PRIORITIES        ( 7 )
# define configMAX_DG_COROUTINE_NAME_LEN        ( 16 )
# ifndef configTOTAL_HEAP_SIZE
#  define configTOTAL_HEAP_SIZE                 ( ( size_t ) ( 3584 ) )
# endif
# if (dg_configOS_ENABLE_THREAD_AWARENESS == 1)
#  undef configUSE_DG_COROUTINE_DEBUG_FACILITY
#  define configUSE_DG_COROUTINE_DEBUG_FACILITY ( 1 )
# endif
#else
/* Task definitions. */
# define configUSE_PREEMPTION                   ( 1 )
# define configMAX_PRIORITIES                   ( 7 )
# define configMINIMAL_STACK_SIZE               ( ( unsigned short ) 100 )
# define configMAX_TASK_NAME_LEN                ( 16 )
# ifndef configTOTAL_HEAP_SIZE
#  define configTOTAL_HEAP_SIZE                 ( ( size_t ) ( 7168 ) )
# endif
#endif /* configUSE_DIALOG_CO_ROUTINES */

#if (dg_configOS_ENABLE_THREAD_AWARENESS == 1)
# define configUSE_TRACE_FACILITY               ( 1 )
# define configRECORD_STACK_HIGH_ADDRESS        ( 1 )
# define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H ( 1 )

# define FREERTOS_TASKS_C_ADDITIONS_INIT()                                             \
        do {                                                                           \
                while (FreeRTOSDebugConfig[0] != FREERTOS_DEBUG_CONFIG_MAJOR_VERSION); \
        } while (0);

# define configGENERATE_RUN_TIME_STATS          ( 1 )
extern unsigned long vGetRunTimeCounterValue(void);
# define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() {}
# define portGET_RUN_TIME_COUNTER_VALUE()       vGetRunTimeCounterValue()
#else
# define configUSE_TRACE_FACILITY               ( 0 )
# define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H ( 0 )
# define configGENERATE_RUN_TIME_STATS          ( 0 )
#endif /* dg_configOS_ENABLE_THREAD_AWARENESS */

#define configUSE_16_BIT_TICKS                  ( 0 )
#define configUSE_TICKLESS_IDLE                 ( 2 )
#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP   ( 2 ) // ticks
#define configIDLE_SHOULD_YIELD                 ( 1 )
#define configUSE_MUTEXES                       ( 1 )
#define configUSE_RECURSIVE_MUTEXES             ( 1 )
#define configUSE_COUNTING_SEMAPHORES           ( 1 )
#if (configUSE_DIALOG_CO_ROUTINES == 1)
# define configQUEUE_REGISTRY_SIZE              ( 4 )
#else
# define configQUEUE_REGISTRY_SIZE              ( 8 )
# define configUSE_APPLICATION_TASK_TAG         ( 0 )
# define configUSE_QUEUE_SETS                   ( 1 )
#endif /* configUSE_DIALOG_CO_ROUTINES */
#define configCHECK_FOR_STACK_OVERFLOW          ( 2 )
#define configUSE_MALLOC_FAILED_HOOK            ( 1 )
#if ((dg_configTRACK_OS_HEAP == 1) || (dg_configUSE_WDOG == 1))
# define configUSE_IDLE_HOOK                    ( 1 )
#else
# define configUSE_IDLE_HOOK                    ( 0 )
#endif /* dg_configTRACK_OS_HEAP || dg_configUSE_WDOG */
#define configUSE_TICK_HOOK                     ( 0 )

#define portSUPPRESS_TICKS_AND_SLEEP( x )       prvSystemSleep( x )
#define configPRE_STOP_PROCESSING()
#define configPRE_SLEEP_PROCESSING( x )
#define configPOST_SLEEP_PROCESSING()
#define configPRE_IDLE_ENTRY( x )               /*cm_lower_all_clocks()*/
#define configPOST_IDLE_ENTRY( x )              /*cm_restore_all_clocks()*/


/* Software timer definitions. */
#define configUSE_TIMERS                        ( 1 )
#if (configUSE_DIALOG_CO_ROUTINES == 1)
# define configTIMER_DG_COROUTINE_PRIORITY      ( configMAX_CO_ROUTINE_PRIORITIES - 1 )
#else
# define configTIMER_TASK_PRIORITY              ( configMAX_PRIORITIES - 1 )
# ifndef configTIMER_TASK_STACK_DEPTH
#  define configTIMER_TASK_STACK_DEPTH          ( configMINIMAL_STACK_SIZE )
# endif
#endif /* configUSE_DIALOG_CO_ROUTINES */
#ifndef configTIMER_QUEUE_LENGTH
# define configTIMER_QUEUE_LENGTH               ( 6 )
#endif

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#if (configUSE_DIALOG_CO_ROUTINES == 1)
# define INCLUDE_uxDgCoRoutinePriorityGet       ( 1 )
# define INCLUDE_vDgCoRoutineDelete             ( 1 )
# define INCLUDE_vDgCoRoutineDelayUntil         ( 1 )
# define INCLUDE_eDgCoRoutineGetState           ( 1 )
# if (dg_configTRACK_OS_HEAP == 1)
#  define INCLUDE_uxDgCoRoutineGetStackHighWaterMark ( 1 )
# endif /* dg_configTRACK_OS_HEAP */
#else
# define INCLUDE_vTaskPrioritySet               ( 1 )
# define INCLUDE_uxTaskPriorityGet              ( 1 )
# define INCLUDE_vTaskDelete                    ( 1 )
# define INCLUDE_vTaskSuspend                   ( 1 )
# define INCLUDE_vTaskDelayUntil                ( 1 )
# define INCLUDE_vTaskDelay                     ( 1 )
# define INCLUDE_eTaskGetState                  ( 1 )
# if (dg_configTRACK_OS_HEAP == 1)
#  define INCLUDE_uxTaskGetStackHighWaterMark   ( 1 )
# endif /* dg_configTRACK_OS_HEAP */
# if (dg_configSYSTEMVIEW == 1)
#  define INCLUDE_xTaskGetCurrentTaskHandle     ( 1 )
# endif /* dg_configSYSTEMVIEW */
#endif /* configUSE_DIALOG_CO_ROUTINES */
#define INCLUDE_xQueueGetMutexHolder            ( 1 )
#define INCLUDE_xTimerPendFunctionCall          ( 1 )

/* Normal assert() semantics without relying on the provision of an assert.h header file. */
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
__RETAINED_CODE void config_assert(void* args);
#define configASSERT( x ) if ( ( x ) == 0 ) { __PUSH_SCRATCH_REGISTERS(); config_assert((void *)__get_SP()); }
#else
#define configASSERT( x ) if ( ( x ) == 0 ) { }
#endif

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names - or at least those used in the unmodified vector table. */
#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                     SysTick_Handler

/* Definitions required for enabling task/co-routine monitoring. */
#if (dg_configENABLE_TASK_MONITORING == 1)
# if (configUSE_DIALOG_CO_ROUTINES == 1)
#  undef INCLUDE_uxDgCoRoutineGetStackHighWaterMark
#  define INCLUDE_uxDgCoRoutineGetStackHighWaterMark ( 1 )
#  undef INCLUDE_eDgCoRoutineGetState
#  define INCLUDE_eDgCoRoutineGetState          ( 1 )
#  undef INCLUDE_pcDgCoRoutineGetName
#  define INCLUDE_pcDgCoRoutineGetName          ( 1 )
#  undef INCLUDE_uxDgCoRoutinePriorityGet
#  define INCLUDE_uxDgCoRoutinePriorityGet      ( 1 )
# else
#  undef INCLUDE_uxTaskGetStackHighWaterMark
#  define INCLUDE_uxTaskGetStackHighWaterMark   ( 1 )
#  undef INCLUDE_eTaskGetState
#  define INCLUDE_eTaskGetState                 ( 1 )
#  undef INCLUDE_uxTaskPriorityGet
#  define INCLUDE_uxTaskPriorityGet             ( 1 )
# endif /* configUSE_DIALOG_CO_ROUTINES */

# if (dg_configOS_ENABLE_THREAD_AWARENESS == 0)
#  undef configUSE_TRACE_FACILITY
#  define configUSE_TRACE_FACILITY              ( 1 )
#  undef configRECORD_STACK_HIGH_ADDRESS
#  define configRECORD_STACK_HIGH_ADDRESS       ( 1 )
# endif
#endif /* dg_configENABLE_TASK_MONITORING */


/*-----------------------------------------------------------*/
/* Cortex-M specific definitions. */
#define configPRIO_BITS                         ( __NVIC_PRIO_BITS )

/* The lowest interrupt priority that can be used in a call to a "set priority" function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY ( (1 << configPRIO_BITS) - 1 )

/* The highest interrupt priority that can be used by any interrupt service
   routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
   INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
   PRIORITY THAN THIS! (higher priorities are lower numeric values on an ARM Cortex-M). */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY ( 1 )

/* Interrupt priorities used by the kernel port layer itself.  These are generic
   to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )


#endif /* FREERTOS_CONFIG_H */
