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

#ifndef PORTMACRO_H
    #define PORTMACRO_H

    #ifdef __cplusplus
        extern "C" {
    #endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

#include "sdk_defs.h"

/* Type definitions. */
    #define portCHAR          char
    #define portFLOAT         float
    #define portDOUBLE        double
    #define portLONG          long
    #define portSHORT         short
    #define portSTACK_TYPE    uint32_t
    #define portBASE_TYPE     long

    typedef portSTACK_TYPE   StackType_t;
    typedef long             BaseType_t;
    typedef unsigned long    UBaseType_t;

    #if ( configUSE_16_BIT_TICKS == 1 )
        typedef uint16_t     TickType_t;
        #define portMAX_DELAY              ( TickType_t ) 0xffff
    #else
        typedef uint32_t     TickType_t;
        #define portMAX_DELAY              ( TickType_t ) 0xffffffffUL

/* 32-bit tick type on a 32-bit architecture, so reads of the tick count do
 * not need to be guarded with a critical section. */
        #define portTICK_TYPE_IS_ATOMIC    1
    #endif
/*-----------------------------------------------------------*/

    /* Code and Data Retention specific constants. */
    #define PRIVILEGED_APP_FUNCTION
    #define PRIVILEGED_DATA                         __attribute__((section("privileged_data_zi")))
    #define INITIALISED_PRIVILEGED_DATA             __attribute__((section("privileged_data_init")))
/*-----------------------------------------------------------*/
/* Architecture specifics. */
    #define portSTACK_GROWTH      ( -1 )
    #define portTICK_PERIOD_MS    ( ( ( TickType_t ) 1000 + ( ( configTICK_RATE_HZ ) / 2) ) / ( configTICK_RATE_HZ ) )
    #define portBYTE_ALIGNMENT    8
    #define portDONT_DISCARD      __attribute__( ( used ) )

    #define portCONVERT_MS_2_TICKS( x )             ( TickType_t ) ( ( ( ( uint64_t ) ( x ) ) * ( configTICK_RATE_HZ ) + ( 1000U / 2 ) ) / 1000 )
    #define portCONVERT_TICKS_2_MS( x )             ( uint32_t ) ( ( ( ( uint64_t ) ( x ) ) * 1000 + ( ( configTICK_RATE_HZ ) / 2 ) ) / ( configTICK_RATE_HZ ) )

    extern void prvSystemSleep( TickType_t xExpectedIdleTime );
    extern void xPortTickAdvance( void );
/*-----------------------------------------------------------*/


/* Scheduler utilities. */
    extern void vPortYield( void );
    #define portNVIC_INT_CTRL_REG     ( *( ( volatile uint32_t * ) 0xe000ed04 ) )
    #define portNVIC_PENDSVSET_BIT    ( 1UL << 28UL )
    #define portYIELD()                                 vPortYield()
    #define portEND_SWITCHING_ISR( xSwitchRequired )    do { if( xSwitchRequired ) portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT; } while( 0 )
    #define portYIELD_FROM_ISR( x )                     portEND_SWITCHING_ISR( x )

    #define portDGCOROUTINE_DEBUG_FACILITY_UPDATE_INFO( pxCRCB ) \
        do { \
            *( StackType_t ** )( ( pxCRCB )->pxTopOfStack + 14 ) = ( pxCRCB )->pxBlockedPC; \
        } while (0);

     #define portDGCOROUTINE_DEBUG_FACILITY_SET_TOPOFSTACK( pxCRCB ) \
        do { \
             ( pxCRCB )->pxTopOfStack = ( ( StackType_t * ) \
                 ( ( ( portPOINTER_SIZE_TYPE ) &( pxCRCB )->pxBlockedPC ) \
                     & ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) ) - 14); \
        } while (0);

    #define portDGCOROUTINE_DEBUG_FACILITY_CRCB_INFO \
        StackType_t *dbg_facility_padding[ 2 ];
/*-----------------------------------------------------------*/


/* Critical section management. */
    extern void vPortEnterCritical( void );
    extern void vPortExitCritical( void );

    #if CMN_TIMING_DEBUG == 0
        extern uint32_t ulSetInterruptMaskFromISR( void ) __attribute__( ( naked ) );
        extern void vClearInterruptMaskFromISR( uint32_t ulMask )  __attribute__( ( naked ) );
    #else
        extern uint32_t ulSetInterruptMaskFromISR( void );
        extern void vClearInterruptMaskFromISR( uint32_t ulMask );
    #endif

    #define portSET_INTERRUPT_MASK_FROM_ISR()         ulSetInterruptMaskFromISR()
    #define portCLEAR_INTERRUPT_MASK_FROM_ISR( x )    vClearInterruptMaskFromISR( x )
    #define portDISABLE_INTERRUPTS() \
        do { \
            __disable_irq(); \
            DBG_CONFIGURE_HIGH(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION); \
        } while (0)
    #define portENABLE_INTERRUPTS() \
        do { \
            DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION); \
            __enable_irq(); \
        } while (0)

    #define portENTER_CRITICAL()                      vPortEnterCritical()
    #define portEXIT_CRITICAL()                       vPortExitCritical()

/*-----------------------------------------------------------*/

/* Debugging. */
    #define portGET_PC()                            ( (uint8_t *)(get_pc()) )

/* Stack tracing. */
    #define portGET_SP()                            ( (uint8_t *)(__get_MSP()) )
    #if defined ( __CC_ARM )
        extern uint32_t Stack_Mem;
        #define portSTACK_LIMIT                     ( (uint8_t *)(&Stack_Mem) )
    #else
        extern uint32_t __StackLimit;
        #define portSTACK_LIMIT                     ( (uint8_t *)(&__StackLimit) )
    #endif

/*-----------------------------------------------------------*/

/* Tickless idle/low power functionality. */
    #ifndef portSUPPRESS_TICKS_AND_SLEEP
        extern void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime );
        #define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime )    vPortSuppressTicksAndSleep( xExpectedIdleTime )
    #endif
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
    #define portDGCOROUTINE_FUNCTION_PROTO( vFunction, uxIndex )    void vFunction( CoRoutineHandle_t xHandle, UBaseType_t uxIndex )
    #define portDGCOROUTINE_FUNCTION( vFunction, uxIndex )          void vFunction( CoRoutineHandle_t xHandle, UBaseType_t uxIndex )

    #define portTASK_FUNCTION_PROTO( vFunction, pvParameters )    void vFunction( void * pvParameters )
    #define portTASK_FUNCTION( vFunction, pvParameters )          void vFunction( void * pvParameters )

    #define portNOP()

    #define portINLINE     __inline

    #ifndef portFORCE_INLINE
        #define portFORCE_INLINE inline __attribute__((always_inline))
    #endif

    #define portMEMORY_BARRIER()    __asm volatile ( "" ::: "memory" )

    portFORCE_INLINE static void *get_pc(void) {
        void *pc;
        __ASM volatile( "mov %0, pc" : "=r" (pc) );
        return pc;
    }
/*-----------------------------------------------------------*/

    #ifdef __cplusplus
        }
    #endif

#endif /* PORTMACRO_H */

#endif /* PROCESSOR_BUILD */
