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

#include "FreeRTOS.h"

#if( ( configUSE_DIALOG_CO_ROUTINES == 0 ) || defined ( _NON_STANDALONE_CROUTINE_C ) )

#include "task.h"
#include "croutine.h"

/* Remove the whole file is co-routines are not being used. */
#if ( configUSE_CO_ROUTINES != 0 )

#if( configUSE_DIALOG_CO_ROUTINES == 1 )
    /* Macros referring to FreeRTOS tasks are defined to corresponding ones of Dialog CoRoutines. */
    #define xTaskGetTickCount    xDgCoRoutineGetTickCount

    #if( configUSE_DG_COROUTINE_DEBUG_FACILITY == 1 )
        /* Macros referring to Dialog CoRoutines symbols are defined to corresponding ones of FreeRTOS. */
        #define pxReadyCoRoutineLists             pxReadyTasksLists
        #define xDelayedCoRoutineList1            xDelayedTaskList1
        #define xDelayedCoRoutineList2            xDelayedTaskList2
        #define pxDelayedCoRoutineList            pxDelayedTaskList
        #define pxOverflowDelayedCoRoutineList    pxOverflowDelayedTaskList
        #define xPendingReadyCoRoutineList        xPendingReadyList

        #define pxCurrentCoRoutine                pxCurrentTCB
        #define uxTopCoRoutineReadyPriority       uxTopReadyPriority
    #endif /* configUSE_DG_COROUTINE_DEBUG_FACILITY */
#endif /* configUSE_DIALOG_CO_ROUTINES */

/*
 * Some kernel aware debuggers require data to be viewed to be global, rather
 * than file scope.
 */
    #ifdef portREMOVE_STATIC_QUALIFIER
        #define static
    #endif


/* Lists for ready and blocked co-routines. --------------------*/
    static List_t pxReadyCoRoutineLists[ configMAX_CO_ROUTINE_PRIORITIES ]; /*< Prioritised ready co-routines. */
    static List_t xDelayedCoRoutineList1;                                   /*< Delayed co-routines. */
    static List_t xDelayedCoRoutineList2;                                   /*< Delayed co-routines (two lists are used - one for delays that have overflowed the current tick count. */
    static List_t * pxDelayedCoRoutineList = NULL;                          /*< Points to the delayed co-routine list currently being used. */
    static List_t * pxOverflowDelayedCoRoutineList = NULL;                  /*< Points to the delayed co-routine list currently being used to hold co-routines that have overflowed the current tick count. */
    static List_t xPendingReadyCoRoutineList;                               /*< Holds co-routines that have been readied by an external event.  They cannot be added directly to the ready lists as the ready lists cannot be accessed by interrupts. */

/* Other file private variables. --------------------------------*/
    CRCB_t * pxCurrentCoRoutine = NULL;
    static UBaseType_t uxTopCoRoutineReadyPriority = 0;
    static TickType_t xCoRoutineTickCount = 0, xLastTickCount = 0, xPassedTicks = 0;

/* The initial state of the co-routine when it is created. */
    #define corINITIAL_STATE    ( 0 )

/*
 * Place the co-routine represented by pxCRCB into the appropriate ready queue
 * for the priority.  It is inserted at the end of the list.
 *
 * This macro accesses the co-routine ready lists and therefore must not be
 * used from within an ISR.
 */
    #define prvAddCoRoutineToReadyQueue( pxCRCB )                                                                       \
    {                                                                                                                   \
        if( pxCRCB->uxPriority > uxTopCoRoutineReadyPriority )                                                          \
        {                                                                                                               \
            uxTopCoRoutineReadyPriority = pxCRCB->uxPriority;                                                           \
        }                                                                                                               \
        vListInsertEnd( ( List_t * ) &( pxReadyCoRoutineLists[ pxCRCB->uxPriority ] ), &( pxCRCB->xGenericListItem ) ); \
    }

/*
 * Utility to ready all the lists used by the scheduler.  This is called
 * automatically upon the creation of the first co-routine.
 */
    static void prvInitialiseCoRoutineLists( void );

/*
 * Co-routines that are readied by an interrupt cannot be placed directly into
 * the ready lists (there is no mutual exclusion).  Instead they are placed in
 * in the pending ready list in order that they can later be moved to the ready
 * list by the co-routine scheduler.
 */
    static void prvCheckPendingReadyList( void );

/*
 * Macro that looks at the list of co-routines that are currently delayed to
 * see if any require waking.
 *
 * Co-routines are stored in the queue in the order of their wake time -
 * meaning once one co-routine has been found whose timer has not expired
 * we need not look any further down the list.
 */
    static void prvCheckDelayedList( void );

#if( configUSE_DIALOG_CO_ROUTINES == 1 )
/*
 * Run a co-routine.
 */
static void prvCoRoutineSchedule( void );
#endif /* configUSE_DIALOG_CO_ROUTINES */

/*-----------------------------------------------------------*/

static void prvInitialiseNewCoRoutine(  crCOROUTINE_CODE pxCoRoutineCode,
                                                         UBaseType_t uxIndex,
                                                         UBaseType_t uxPriority,
                                                         CoRoutineHandle_t xCreatedCoRoutine )
    {
    CRCB_t *pxCoRoutine = ( CRCB_t * ) xCreatedCoRoutine;

#if( configUSE_DIALOG_CO_ROUTINES == 0 )
        /* If pxCurrentCoRoutine is NULL then this is the first co-routine to
        * be created and the co-routine data structures need initialising. */
        if( pxCurrentCoRoutine == NULL )
        {
            pxCurrentCoRoutine = pxCoRoutine;
            prvInitialiseCoRoutineLists();
        }
#endif
        /* Check the priority is within limits. */
        if( uxPriority >= configMAX_CO_ROUTINE_PRIORITIES )
        {
            uxPriority = configMAX_CO_ROUTINE_PRIORITIES - 1;
        }

        /* Fill out the co-routine control block from the function parameters. */
        pxCoRoutine->uxState = corINITIAL_STATE;
        pxCoRoutine->uxPriority = uxPriority;
        pxCoRoutine->uxIndex = uxIndex;
        pxCoRoutine->pxCoRoutineFunction = pxCoRoutineCode;

        /* Initialise all the other co-routine control block parameters. */
        vListInitialiseItem( &( pxCoRoutine->xGenericListItem ) );
        vListInitialiseItem( &( pxCoRoutine->xEventListItem ) );

        /* Set the co-routine control block as a link back from the ListItem_t.
         * This is so we can get back to the containing CRCB from a generic item
         * in a list. */
        listSET_LIST_ITEM_OWNER( &( pxCoRoutine->xGenericListItem ), pxCoRoutine );
        listSET_LIST_ITEM_OWNER( &( pxCoRoutine->xEventListItem ), pxCoRoutine );

        /* Event lists are always in priority order. */
        listSET_LIST_ITEM_VALUE( &( pxCoRoutine->xEventListItem ), ( ( TickType_t ) configMAX_CO_ROUTINE_PRIORITIES - ( TickType_t ) uxPriority ) );
    }
/*-----------------------------------------------------------*/

#if( configUSE_DIALOG_CO_ROUTINES == 0 )

    BaseType_t xCoRoutineCreate( crCOROUTINE_CODE pxCoRoutineCode,
                                 UBaseType_t uxPriority,
                                 UBaseType_t uxIndex )
    {
        BaseType_t xReturn;
        CRCB_t * pxCoRoutine;

        /* Allocate the memory that will store the co-routine control block. */
        pxCoRoutine = ( CRCB_t * ) pvPortMalloc( sizeof( CRCB_t ) );

        if( pxCoRoutine )
        {
            /* Initialise co-routine. */
            prvInitialiseNewCoRoutine( pxCoRoutineCode, uxIndex, uxPriority, pxCoRoutine );

            /* Now the co-routine has been initialised it can be added to the ready
             * list at the correct priority. */
            prvAddCoRoutineToReadyQueue( pxCoRoutine );

            xReturn = pdPASS;
        }
        else
        {
            xReturn = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
        }

        return xReturn;
    }
#endif /* configUSE_DIALOG_CO_ROUTINES */
/*-----------------------------------------------------------*/

    void vCoRoutineAddToDelayedList( TickType_t xTicksToDelay,
                                     List_t * pxEventList )
    {
        TickType_t xTimeToWake;
        TickType_t xTicks;

    /* Get tick count. */
#if( configUSE_DIALOG_CO_ROUTINES == 1 )
        xTicks = xDgCoRoutineGetTickCount();
#else
        xTicks = xCoRoutineTickCount;
#endif /* configUSE_DIALOG_CO_ROUTINES */

        /* Calculate the time to wake - this may overflow but this is
         * not a problem. */
        xTimeToWake = xTicks + xTicksToDelay;

        /* We must remove ourselves from the ready list before adding
         * ourselves to the blocked list as the same list item is used for
         * both lists. */
        ( void ) uxListRemove( ( ListItem_t * ) &( pxCurrentCoRoutine->xGenericListItem ) );

        /* The list item will be inserted in wake time order. */
        listSET_LIST_ITEM_VALUE( &( pxCurrentCoRoutine->xGenericListItem ), xTimeToWake );

        if( xTimeToWake < xTicks )
        {
            /* Wake time has overflowed.  Place this item in the
             * overflow list. */
            vListInsert( ( List_t * ) pxOverflowDelayedCoRoutineList, ( ListItem_t * ) &( pxCurrentCoRoutine->xGenericListItem ) );
        }
        else
        {
            /* The wake time has not overflowed, so we can use the
             * current block list. */
            vListInsert( ( List_t * ) pxDelayedCoRoutineList, ( ListItem_t * ) &( pxCurrentCoRoutine->xGenericListItem ) );
        }

        if( pxEventList )
        {
            /* Also add the co-routine to an event list.  If this is done then the
             * function must be called with interrupts disabled. */
            vListInsert( pxEventList, &( pxCurrentCoRoutine->xEventListItem ) );
        }
    }
/*-----------------------------------------------------------*/

    static void prvCheckPendingReadyList( void )
    {
        /* Are there any co-routines waiting to get moved to the ready list?  These
         * are co-routines that have been readied by an ISR.  The ISR cannot access
         * the ready lists itself. */
        while( listLIST_IS_EMPTY( &xPendingReadyCoRoutineList ) == pdFALSE )
        {
            CRCB_t * pxUnblockedCRCB;

            /* The pending ready list can be accessed by an ISR. */
#if( configUSE_DIALOG_CO_ROUTINES == 1 )
            dgcrENTER_CRITICAL();
#else
            portDISABLE_INTERRUPTS();
#endif
            {
                pxUnblockedCRCB = ( CRCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( ( &xPendingReadyCoRoutineList ) );
                ( void ) uxListRemove( &( pxUnblockedCRCB->xEventListItem ) );
            }
#if( configUSE_DIALOG_CO_ROUTINES == 1 )
            dgcrEXIT_CRITICAL();
#else
            portENABLE_INTERRUPTS();
#endif

            ( void ) uxListRemove( &( pxUnblockedCRCB->xGenericListItem ) );
            prvAddCoRoutineToReadyQueue( pxUnblockedCRCB );
        }
    }
/*-----------------------------------------------------------*/

    static void prvCheckDelayedList( void )
    {
        CRCB_t * pxCRCB;

        xPassedTicks = xTaskGetTickCount() - xLastTickCount;

        while( xPassedTicks )
        {
#if( configUSE_DIALOG_CO_ROUTINES == 1 )
            TickType_t xLastWakeTime = 0;
#endif
            xCoRoutineTickCount++;

            /* If the tick count has overflowed we need to swap the ready lists. */
            if( xCoRoutineTickCount == 0 )
            {
                List_t * pxTemp;

                /* Tick count has overflowed so we need to swap the delay lists.  If there are
                 * any items in pxDelayedCoRoutineList here then there is an error! */
                pxTemp = pxDelayedCoRoutineList;
                pxDelayedCoRoutineList = pxOverflowDelayedCoRoutineList;
                pxOverflowDelayedCoRoutineList = pxTemp;
            }

            /* See if this tick has made a timeout expire. */
            while( listLIST_IS_EMPTY( pxDelayedCoRoutineList ) == pdFALSE )
            {
                pxCRCB = ( CRCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxDelayedCoRoutineList );

#if( configUSE_DIALOG_CO_ROUTINES == 1 )
                xLastWakeTime = listGET_LIST_ITEM_VALUE( &( pxCRCB->xGenericListItem ) );
                if( xCoRoutineTickCount < xLastWakeTime )
                {
                    /* Timeout not yet expired. */
                    break;
                }
#else
                if( xCoRoutineTickCount < listGET_LIST_ITEM_VALUE( &( pxCRCB->xGenericListItem ) ) )
                {
                    /* Timeout not yet expired. */
                    break;
                }
#endif /* configUSE_DIALOG_CO_ROUTINES */

#if( configUSE_DIALOG_CO_ROUTINES == 1 )
                dgcrENTER_CRITICAL();
#else
                portDISABLE_INTERRUPTS();
#endif
                {
                    /* The event could have occurred just before this critical
                     *  section.  If this is the case then the generic list item will
                     *  have been moved to the pending ready list and the following
                     *  line is still valid.  Also the pvContainer parameter will have
                     *  been set to NULL so the following lines are also valid. */
                    ( void ) uxListRemove( &( pxCRCB->xGenericListItem ) );

                    /* Is the co-routine waiting on an event also? */
                    if( pxCRCB->xEventListItem.pxContainer )
                    {
                        ( void ) uxListRemove( &( pxCRCB->xEventListItem ) );
                    }
                }
#if( configUSE_DIALOG_CO_ROUTINES == 1 )
                dgcrEXIT_CRITICAL();
#else
                portENABLE_INTERRUPTS();
#endif

                prvAddCoRoutineToReadyQueue( pxCRCB );
            }

#if( configUSE_DIALOG_CO_ROUTINES == 1 )
            if( xPassedTicks > 2 )
            {
                TickType_t xTicksChange = xPassedTicks - 2;
                if( listLIST_IS_EMPTY( pxDelayedCoRoutineList ) == pdFALSE )
                {
                    if( ( xLastWakeTime - xCoRoutineTickCount ) < xTicksChange )
                    {
                        xTicksChange = xLastWakeTime - xCoRoutineTickCount;
                    }
                }

                if( ( (TickType_t)0 - xCoRoutineTickCount ) < xTicksChange )
                {
                    xTicksChange = ( (TickType_t)0 - xCoRoutineTickCount );
                }

                xPassedTicks -= xTicksChange;
                xCoRoutineTickCount += xTicksChange;
            }
#endif /* configUSE_DIALOG_CO_ROUTINES */

            xLastTickCount = xCoRoutineTickCount;
            xPassedTicks--;
        }

#if( configUSE_DIALOG_CO_ROUTINES == 1 ) && ( configUSE_TICKLESS_IDLE != 0 )
        vResetNextDgCoRoutineUnblockTime();
#endif /* configUSE_DIALOG_CO_ROUTINES && configUSE_TICKLESS_IDLE */

    }
/*-----------------------------------------------------------*/

#if( configUSE_DIALOG_CO_ROUTINES == 1 )
    static void prvCoRoutineSchedule( void )
#else
    void vCoRoutineSchedule( void )
#endif /* configUSE_DIALOG_CO_ROUTINES */
    {
        /* Only run a co-routine after prvInitialiseCoRoutineLists() has been
         * called.  prvInitialiseCoRoutineLists() is called automatically when a
         * co-routine is created. */
        if( pxDelayedCoRoutineList != NULL )
        {
            /* See if any co-routines readied by events need moving to the ready lists. */
            prvCheckPendingReadyList();

            /* See if any delayed co-routines have timed out. */
            prvCheckDelayedList();

            /* Find the highest priority queue that contains ready co-routines. */
            while( listLIST_IS_EMPTY( &( pxReadyCoRoutineLists[ uxTopCoRoutineReadyPriority ] ) ) )
            {
                if( uxTopCoRoutineReadyPriority == 0 )
                {
                    /* No more co-routines to check. */
                    return;
                }

                --uxTopCoRoutineReadyPriority;
            }

            /* listGET_OWNER_OF_NEXT_ENTRY walks through the list, so the co-routines
             * of the same priority get an equal share of the processor time. */
            listGET_OWNER_OF_NEXT_ENTRY( pxCurrentCoRoutine, &( pxReadyCoRoutineLists[ uxTopCoRoutineReadyPriority ] ) );

            vDgCoRoutineSchedulerLeaveContextSwitch();
#if( configUSE_DIALOG_CO_ROUTINES == 1 )
#if( ( configUSE_TRACE_FACILITY == 1 ) || ( INCLUDE_uxDgCoRoutineGetStackHighWaterMark == 1 ) || ( INCLUDE_pxDgCoRoutineGetStackStart == 1 ) )
            /* Store the highest address of the co-routine's stack area. */
            dgcrENTER_CRITICAL();
#if( portSTACK_GROWTH < 0 )
            pxCurrentCoRoutine->pxEndOfStack = (StackType_t *)(portGET_SP());
#else
            pxCurrentCoRoutine->pxStack = (StackType_t *)(portGET_SP());
#endif /* portSTACK_GROWTH */
            dgcrEXIT_CRITICAL();
#endif /* configUSE_TRACE_FACILITY || INCLUDE_uxDgCoRoutineGetStackHighWaterMark || INCLUDE_pxDgCoRoutineGetStackStart */
#endif /* configUSE_DIALOG_CO_ROUTINES */

            /* Call the co-routine. */
            ( pxCurrentCoRoutine->pxCoRoutineFunction )( pxCurrentCoRoutine, pxCurrentCoRoutine->uxIndex );
        }
    }
/*-----------------------------------------------------------*/

    static void prvInitialiseCoRoutineLists( void )
    {
        UBaseType_t uxPriority;

        for( uxPriority = 0; uxPriority < configMAX_CO_ROUTINE_PRIORITIES; uxPriority++ )
        {
            vListInitialise( ( List_t * ) &( pxReadyCoRoutineLists[ uxPriority ] ) );
        }

        vListInitialise( ( List_t * ) &xDelayedCoRoutineList1 );
        vListInitialise( ( List_t * ) &xDelayedCoRoutineList2 );
        vListInitialise( ( List_t * ) &xPendingReadyCoRoutineList );

        /* Start with pxDelayedCoRoutineList using list1 and the
         * pxOverflowDelayedCoRoutineList using list2. */
        pxDelayedCoRoutineList = &xDelayedCoRoutineList1;
        pxOverflowDelayedCoRoutineList = &xDelayedCoRoutineList2;
    }
/*-----------------------------------------------------------*/

    BaseType_t xCoRoutineRemoveFromEventList( const List_t * pxEventList )
    {
        CRCB_t * pxUnblockedCRCB;
        BaseType_t xReturn;

        /* This function is called from within an interrupt.  It can only access
         * event lists and the pending ready list.  This function assumes that a
         * check has already been made to ensure pxEventList is not empty. */
        pxUnblockedCRCB = ( CRCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( pxEventList );
        ( void ) uxListRemove( &( pxUnblockedCRCB->xEventListItem ) );
        vListInsertEnd( ( List_t * ) &( xPendingReadyCoRoutineList ), &( pxUnblockedCRCB->xEventListItem ) );

        if( pxUnblockedCRCB->uxPriority >= pxCurrentCoRoutine->uxPriority )
        {
            xReturn = pdTRUE;
#if( configUSE_DIALOG_CO_ROUTINES == 1 )
            vDgCoRoutineMissedYieldForPriority( pxUnblockedCRCB->uxPriority );
#endif
        }
        else
        {
            xReturn = pdFALSE;
        }

        return xReturn;
    }

#endif /* configUSE_CO_ROUTINES == 0 */

#endif /* configUSE_DIALOG_CO_ROUTINES || _NON_STANDALONE_CROUTINE_C */
