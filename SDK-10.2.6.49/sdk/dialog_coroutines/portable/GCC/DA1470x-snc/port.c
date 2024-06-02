/*
 * FreeRTOS Kernel V10.4.4
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
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
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 * Copyright (c) 2021 Modified by Dialog Semiconductor
 *
 */

#if (SNC_PROCESSOR_BUILD)

/*-----------------------------------------------------------
* Implementation of functions defined in portable.h for the ARM CM0 port.
*----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"

#include "sys_power_mgr.h"
#include "sys_timer.h"
#include "hw_clk.h"

#if (dg_configSYSTEMVIEW == 1)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif /* (dg_configSYSTEMVIEW == 1) */

#if (configGENERATE_RUN_TIME_STATS == 1)
unsigned long vGetRunTimeCounterValue(void)
{
        return (in_interrupt() ? sys_timer_get_uptime_ticks_fromISR() : sys_timer_get_uptime_ticks());
}
#endif

/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL_REG             ( *( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG             ( *( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG    ( *( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_INT_CTRL_REG                 ( *( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_SHPR3_REG                    ( *( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_SYSTICK_CLK_BIT              ( 1UL << 2UL )
#define portNVIC_SYSTICK_INT_BIT              ( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT           ( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT       ( 1UL << 16UL )
#define portNVIC_PENDSVSET_BIT                ( 1UL << 28UL )
#define portMIN_INTERRUPT_PRIORITY            ( 255UL )
#define portNVIC_PENDSV_PRI                   ( portMIN_INTERRUPT_PRIORITY << 16UL )
#define portNVIC_SYSTICK_PRI                  ( portMIN_INTERRUPT_PRIORITY << 24UL )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR                      ( 0x01000000 )

/* The systick is a 24-bit counter. */
#define portMAX_24_BIT_NUMBER                 ( 0xffffffUL )

/* A fiddle factor to estimate the number of SysTick counts that would have
 * occurred while the SysTick counter is stopped during tickless idle
 * calculations. */
#ifndef portMISSED_COUNTS_FACTOR
    #define portMISSED_COUNTS_FACTOR    ( 45UL )
#endif

/* Let the user override the pre-loading of the initial LR with the address of
 * prvTaskExitError() in case it messes up unwinding of the stack in the
 * debugger. */
#ifdef configTASK_RETURN_ADDRESS
    #define portTASK_RETURN_ADDRESS    configTASK_RETURN_ADDRESS
#else
    #define portTASK_RETURN_ADDRESS    prvTaskExitError
#endif

/*
 * Setup the timer to generate the tick interrupts.  The implementation in this
 * file is weak to allow application writers to change the timer used to
 * generate the tick interrupt.
 */
void vPortSetupTimerInterrupt( void );

/*
 * Exception handlers.
 */
void xPortPendSVHandler( void ) __attribute__( ( naked ) );
void xPortTickAdvance( void );
void vPortSVCHandler( void );

#if( configUSE_DIALOG_CO_ROUTINES == 0 )
/*
 * Start first task is a separate function so it can be tested in isolation.
 */
static void vPortStartFirstTask( void ) __attribute__( ( naked ) );

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );
#endif /* configUSE_DIALOG_CO_ROUTINES */

/*-----------------------------------------------------------*/

/* Each task maintains its own interrupt status in the critical nesting
 * variable. */
static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

/*-----------------------------------------------------------*/

/*
 * The number of SysTick increments that make up one tick period.
 */
#if ( configUSE_TICKLESS_IDLE == 1 )
    static uint32_t ulTimerCountsForOneTick = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * The maximum number of tick periods that can be suppressed is limited by the
 * 24 bit resolution of the SysTick timer.
 */
#if ( configUSE_TICKLESS_IDLE == 1 )
    static uint32_t xMaximumPossibleSuppressedTicks = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * Compensate for the CPU cycles that pass while the SysTick is stopped (low
 * power functionality only.
 */
#if ( configUSE_TICKLESS_IDLE == 1 )
    static uint32_t ulStoppedTimerCompensation = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*-----------------------------------------------------------*/

#if( configUSE_DIALOG_CO_ROUTINES == 0 )

/*
 * See header file for description.
 */
StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                     TaskFunction_t pxCode,
                                     void * pvParameters )
{
    /* Simulate the stack frame as it would be created by a context switch
     * interrupt. */
    pxTopOfStack--;                                          /* Offset added to account for the way the MCU uses the stack on entry/exit of interrupts. */
    *pxTopOfStack = portINITIAL_XPSR;                        /* xPSR */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) pxCode;                  /* PC */
    pxTopOfStack--;
    *pxTopOfStack = ( StackType_t ) portTASK_RETURN_ADDRESS; /* LR */
    pxTopOfStack -= 5;                                       /* R12, R3, R2 and R1. */
    *pxTopOfStack = ( StackType_t ) pvParameters;            /* R0 */
    pxTopOfStack -= 8;                                       /* R11..R4. */

    return pxTopOfStack;
}
#endif /* configUSE_DIALOG_CO_ROUTINES */
/*-----------------------------------------------------------*/

#if( configUSE_DIALOG_CO_ROUTINES == 0 )

static void prvTaskExitError( void )
{
    volatile uint32_t ulDummy = 0UL;

    /* A function that implements a task must not exit or attempt to return to
     * its caller as there is nothing to return to.  If a task wants to exit it
     * should instead call vTaskDelete( NULL ).
     *
     * Artificially force an assert() to be triggered if configASSERT() is
     * defined, then stop here so application writers can catch the error. */
    configASSERT( uxCriticalNesting == ~0UL );
    portDISABLE_INTERRUPTS();

    while( ulDummy == 0 )
    {
        /* This file calls prvTaskExitError() after the scheduler has been
         * started to remove a compiler warning about the function being defined
         * but never called.  ulDummy is used purely to quieten other warnings
         * about code appearing after this function is called - making ulDummy
         * volatile makes the compiler think the function could return and
         * therefore not output an 'unreachable code' warning for code that appears
         * after it. */
    }
}
#endif /* configUSE_DIALOG_CO_ROUTINES */
/*-----------------------------------------------------------*/

void vPortSVCHandler( void )
{
    /* This function is no longer used, but retained for backward
     * compatibility. */
}
/*-----------------------------------------------------------*/

#if( configUSE_DIALOG_CO_ROUTINES == 0 )

void vPortStartFirstTask( void )
{
    /* The MSP stack is not reset as, unlike on M3/4 parts, there is no vector
     * table offset register that can be used to locate the initial stack value.
     * Not all M0 parts have the application vector table at address 0. */
    __asm volatile (
        "	.syntax unified				\n"
        "	ldr  r2, pxCurrentTCBConst2	\n"/* Obtain location of pxCurrentTCB. */
        "	ldr  r3, [r2]				\n"
        "	ldr  r0, [r3]				\n"/* The first item in pxCurrentTCB is the task top of stack. */
        "	adds r0, #32					\n"/* Discard everything up to r0. */
        "	msr  psp, r0					\n"/* This is now the new top of stack to use in the task. */
        "	movs r0, #2					\n"/* Switch to the psp stack. */
        "	msr  CONTROL, r0				\n"
        "	isb							\n"
        "	pop  {r0-r5}					\n"/* Pop the registers that are saved automatically. */
        "	mov  lr, r5					\n"/* lr is now in r5. */
        "	pop  {r3}					\n"/* Return address is now in r3. */
        "	pop  {r2}					\n"/* Pop and discard XPSR. */
        "	cpsie i						\n"/* The first task has its context and interrupts can be enabled. */
        "	bx   r3						\n"/* Finally, jump to the user defined task code. */
        "								\n"
        "	.align 4					\n"
        "pxCurrentTCBConst2: .word pxCurrentTCB	  "
        );
}
#endif /* configUSE_DIALOG_CO_ROUTINES */
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
BaseType_t xPortStartScheduler( void )
{
    /* Make PendSV, CallSV and SysTick the same priority as the kernel. */
    portNVIC_SHPR3_REG |= portNVIC_PENDSV_PRI;
    portNVIC_SHPR3_REG |= portNVIC_SYSTICK_PRI;

    /* Start the timer that generates the tick ISR.  Interrupts are disabled
     * here already. */
    vPortSetupTimerInterrupt();

    /* Initialise the critical nesting count ready for the first task. */
    uxCriticalNesting = 0;

#if( configUSE_DIALOG_CO_ROUTINES == 1 )
    /* Enable interrupts. */
    portENABLE_INTERRUPTS();

    for( ;; )
    {
            vDgCoRoutineSchedule();
    }
#else
    /* Start the first task. */
    vPortStartFirstTask();

    /* Should never get here as the tasks will now be executing!  Call the task
     * exit error function to prevent compiler warnings about a static function
     * not being called in the case that the application writer overrides this
     * functionality by defining configTASK_RETURN_ADDRESS.  Call
     * vTaskSwitchContext() so link time optimisation does not remove the
     * symbol. */
    vTaskSwitchContext();
    prvTaskExitError();
#endif /* configUSE_DIALOG_CO_ROUTINES */

    /* Should not get here! */
    return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* Not implemented in ports where there is nothing to return to.
     * Artificially force an assert. */
    configASSERT( uxCriticalNesting == 1000UL );
}
/*-----------------------------------------------------------*/

void vPortYield( void )
{
    /* Set a PendSV to request a context switch. */
    portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;

    /* Barriers are normally not required but do ensure the code is completely
     * within the specified behaviour for the architecture. */
    __asm volatile ( "dsb" ::: "memory" );
    __asm volatile ( "isb" );
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
    portDISABLE_INTERRUPTS();
    uxCriticalNesting++;
    __asm volatile ( "dsb" ::: "memory" );
    __asm volatile ( "isb" );
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
    configASSERT( uxCriticalNesting );
    uxCriticalNesting--;

    if( uxCriticalNesting == 0 )
    {
        portENABLE_INTERRUPTS();
    }
}
/*-----------------------------------------------------------*/

#if (CMN_TIMING_DEBUG == 0)
uint32_t ulSetInterruptMaskFromISR( void )
{
    __asm volatile (
        " mrs r0, PRIMASK	\n"
        " cpsid i			\n"
        " bx lr				  "
        ::: "memory"
        );

    /* To avoid compiler warnings.  This line will never be reached. */
    return 0;
}
/*-----------------------------------------------------------*/

void vClearInterruptMaskFromISR( __attribute__( ( unused ) ) uint32_t ulMask )
{
    __asm volatile (
        " msr PRIMASK, r0	\n"
        " bx lr				  "
        ::: "memory"
        );

    /* Just to avoid compiler warning. */
    ( void ) ulMask;
}
#else
uint32_t ulSetInterruptMaskFromISR( void )
{
    uint32 primask = __get_PRIMASK();
    __set_PRIMASK(1);
    DBG_CONFIGURE_HIGH(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);
    return primask;
}
/*-----------------------------------------------------------*/

void vClearInterruptMaskFromISR( uint32_t ulMask )
{
    if (!ulMask) {
        DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);
    }
    __set_PRIMASK(ulMask);
}
#endif
/*-----------------------------------------------------------*/

void xPortPendSVHandler( void )
{
#if( configUSE_DIALOG_CO_ROUTINES == 1 )
    /* This function is retained for backward compatibility. */
#else
    /* This is a naked function. */

    __asm volatile
    (
        "	.syntax unified						\n"
        "	mrs r0, psp							\n"
        "										\n"
        "	ldr	r3, pxCurrentTCBConst			\n"/* Get the location of the current TCB. */
        "	ldr	r2, [r3]						\n"
        "										\n"
        "	subs r0, r0, #32					\n"/* Make space for the remaining low registers. */
        "	str r0, [r2]						\n"/* Save the new top of stack. */
        "	stmia r0!, {r4-r7}					\n"/* Store the low registers that are not saved automatically. */
        " 	mov r4, r8							\n"/* Store the high registers. */
        " 	mov r5, r9							\n"
        " 	mov r6, r10							\n"
        " 	mov r7, r11							\n"
        " 	stmia r0!, {r4-r7}					\n"
        "										\n"
        "	push {r3, r14}						\n"
        "	cpsid i								\n"
        "	bl vTaskSwitchContext				\n"
        "	cpsie i								\n"
        "	pop {r2, r3}						\n"/* lr goes in r3. r2 now holds tcb pointer. */
        "										\n"
        "	ldr r1, [r2]						\n"
        "	ldr r0, [r1]						\n"/* The first item in pxCurrentTCB is the task top of stack. */
        "	adds r0, r0, #16					\n"/* Move to the high registers. */
        "	ldmia r0!, {r4-r7}					\n"/* Pop the high registers. */
        " 	mov r8, r4							\n"
        " 	mov r9, r5							\n"
        " 	mov r10, r6							\n"
        " 	mov r11, r7							\n"
        "										\n"
        "	msr psp, r0							\n"/* Remember the new top of stack for the task. */
        "										\n"
        "	subs r0, r0, #32					\n"/* Go back for the low registers that are not automatically restored. */
        " 	ldmia r0!, {r4-r7}					\n"/* Pop low registers.  */
        "										\n"
        "	bx r3								\n"
        "										\n"
        "	.align 4							\n"
        "pxCurrentTCBConst: .word pxCurrentTCB	  "
    );
#endif /* configUSE_DIALOG_CO_ROUTINES */
}
/*-----------------------------------------------------------*/

void xPortTickAdvance( void )
{
    SEGGER_SYSTEMVIEW_ISR_ENTER();
    uint32_t ulPreviousMask;

    ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
    {
        /* Increment the RTOS tick. */
#if( configUSE_DIALOG_CO_ROUTINES == 1 )
        xDgCoRoutineIncrementTick();
#else
        if( xTaskIncrementTick() != pdFALSE )
        {
            /* Pend a context switch. */
            portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
        }
#endif /* configUSE_DIALOG_CO_ROUTINES */

        SEGGER_SYSTEMVIEW_ISR_EXIT();
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR( ulPreviousMask );
}
/*-----------------------------------------------------------*/

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void vPortSetupTimerInterrupt( void )
{
	sys_timer_start(TICK_PERIOD);
}
/*-----------------------------------------------------------*/

#if( configUSE_DIALOG_CO_ROUTINES == 1 )
#define E_SLEEP_MODE_STATUS             eDgCoRoutineSleepModeStatus
#define eABORT_SLEEP                    eDgCrAbortSleep
#undef eNO_TASKS_WAITING_TIMEOUT
#define eCONFIRM_SLEEP_MODE_STATUS()    eDgCoRoutineConfirmSleepModeStatus()
#else
#define E_SLEEP_MODE_STATUS             eSleepModeStatus
#define eABORT_SLEEP                    eAbortSleep
#define eNO_TASKS_WAITING_TIMEOUT       eNoTasksWaitingTimeout
#define eCONFIRM_SLEEP_MODE_STATUS()    eTaskConfirmSleepModeStatus()
#endif /* configUSE_DIALOG_CO_ROUTINES */

__attribute__((weak)) void prvSystemSleep( TickType_t xExpectedIdleTime )
{
        E_SLEEP_MODE_STATUS eSleepStatus;

        /* A simple WFI() is executed when XTAL32K is not used as the LP clock. System has
         * just booted or woken up after hibernation or deep sleep and the LP clock is not yet settled.
         */
        if( !hw_clk_lp_is_xtal32k() )
        {
                __disable_irq();

                /* Ensure it is still ok to enter the sleep mode. */
                eSleepStatus = eCONFIRM_SLEEP_MODE_STATUS();

                if( eSleepStatus == eABORT_SLEEP )
                {
                        __enable_irq();
                        return;
                }

                pm_execute_wfi();
                // Interrupts are enabled in pm_execute_wfi()

                return;
        }

        /* Enter a critical section that will not effect interrupts bringing the MCU
         * out of sleep mode.
         */
        __disable_irq();

        DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);

        DBG_SET_HIGH(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_ENTER);

        /* Ensure it is still ok to enter the sleep mode. */
        eSleepStatus = eCONFIRM_SLEEP_MODE_STATUS();
        if( eSleepStatus == eABORT_SLEEP )
        {
                DBG_SET_LOW(PWR_MGR_USE_TIMING_DEBUG, PWRDBG_SLEEP_ENTER);

                /* A task has been moved out of the Blocked state since this macro was
                 * executed, or a context switch is being held pending. Do not enter a
                 * sleep state. Restart the tick and exit the critical section.
                 */
                __enable_irq();
        }
        else
        {
#ifdef eNO_TASKS_WAITING_TIMEOUT
                if( eSleepStatus == eNO_TASKS_WAITING_TIMEOUT )
                {
                        /* It is not necessary to configure an interrupt to bring the
                         * microcontroller out of its low power state at a fixed time in the
                         * future.
                         * Enter the low power state.
                         */
                        pm_sleep_enter( 0 );
                }
                else
#endif /* eNO_TASKS_WAITING_TIMEOUT */
                {
                        /* Configure an interrupt to bring the microcontroller out of its low
                         * power state at the time the kernel next needs to execute.
                         * Enter the low power state.
                         */
                        pm_sleep_enter( xExpectedIdleTime * TICK_PERIOD );
                }

                // Interrupts are enabled in pm_sleep_enter()
        }
}

#endif /* PROCESSOR_BUILD */
