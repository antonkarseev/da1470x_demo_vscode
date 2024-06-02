/**
 ****************************************************************************************
 *
 * @file dialog_timers.c
 *
 * @brief Extensions to FreeRTOS timers.c
 *
 * Copyright (C) 2020-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"

#if (configUSE_DIALOG_CO_ROUTINES == 1) && (configUSE_TIMERS == 1)

#define _NON_STANDALONE_TIMERS_C
#include "timers.c"

/*
 * The name allocated to the timer daemon/service co-routine. This can be overridden by defining
 * configTIMER_SERVICE_DG_COROUTINE_NAME in FreeRTOSConfig.h.
 */
#ifndef configTIMER_SERVICE_DG_COROUTINE_NAME
        #define configTIMER_SERVICE_DG_COROUTINE_NAME "Tmr Svc"
#endif

/*-----------------------------------------------------------*/

/* File private variables. ----------------------------------*/

/* Handle of the timer service/daemon co-routine. */
PRIVILEGED_DATA static CoRoutineHandle_t xTimerDgCoRoutineHandle = NULL;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
/* Space reserved for storing control block data structure of timer daemon/service co-routine. */
PRIVILEGED_DATA static StaticDgCoRoutine_t xTimerDgCoRoutineBuffer;
#endif /* configSUPPORT_STATIC_ALLOCATION */

/* File private functions -----------------------------------*/

/* The co-routine service/daemon co-routine. */
static void prvTimerDgCoRoutine(CoRoutineHandle_t xHandle, UBaseType_t uxIndex) PRIVILEGED_FUNCTION;

/*
 * If a timer has expired, process it, otherwise, block the timer service/daemon co-routine to
 * wait for either a timer to expire or a command to be received.
 */
static BaseType_t prvProcessTimerOrBlockDgCoRoutine(const TickType_t xNextExpireTime,
        BaseType_t xListWasEmpty) PRIVILEGED_FUNCTION;

/*-----------------------------------------------------------*/

BaseType_t xTimerCreateTimerDgCoRoutine(void)
{
        BaseType_t xReturn = pdFAIL;

        /* Check that the timer service/daemon co-routine's infrastructure has been initialised. */
        prvCheckForValidListAndQueue();

        if (xTimerQueue != NULL) {
#if (configSUPPORT_STATIC_ALLOCATION == 1)
                xTimerDgCoRoutineHandle = xDgCoRoutineCreateStatic(prvTimerDgCoRoutine,
                                                configTIMER_SERVICE_DG_COROUTINE_NAME,
                                                ((UBaseType_t)configTIMER_DG_COROUTINE_PRIORITY),
                                                0, &xTimerDgCoRoutineBuffer);
                if (xTimerDgCoRoutineHandle != NULL) {
                        xReturn = pdPASS;
                }
#else
                xReturn = xDgCoRoutineCreate(prvTimerDgCoRoutine,
                                configTIMER_SERVICE_DG_COROUTINE_NAME,
                                ((UBaseType_t)configTIMER_DG_COROUTINE_PRIORITY),
                                0, &xTimerDgCoRoutineHandle);
#endif /* configSUPPORT_STATIC_ALLOCATION */
        } else {
                mtCOVERAGE_TEST_MARKER();
        }

        configASSERT(xReturn);

        return xReturn;
}
/*-----------------------------------------------------------*/

CoRoutineHandle_t xTimerGetTimerDaemonDgCoRoutineHandle(void)
{
        configASSERT(xTimerDgCoRoutineHandle != NULL);
        return xTimerDgCoRoutineHandle;
}
/*-----------------------------------------------------------*/

static void prvTimerDgCoRoutine(CoRoutineHandle_t xHandle, UBaseType_t uxIndex)
{
        TickType_t xNextExpireTime;
        BaseType_t xListWasEmpty;

        /* Avoid compiler warnings. */
        (void) uxIndex;

        /* CoRoutine start point. */
        crSTART(xHandle);

#if (configUSE_DAEMON_DG_COROUTINE_STARTUP_HOOK == 1)
        {
                extern void vApplicationDaemonDgCoRoutineStartupHook(void);

                /*
                 * Execute application code (if any) in the context of this co-routine at the point
                 * it starts executing.
                 */
                vApplicationDaemonDgCoRoutineStartupHook();
        }
#endif /* configUSE_DAEMON_DG_COROUTINE_STARTUP_HOOK */

        for ( ;; ) {
                BaseType_t xResult;

                /* Get next time at which a timer will expire. */
                xNextExpireTime = prvGetNextExpireTime(&xListWasEmpty);

                /*
                 * Process an expired timer, or block and wait for either a timer to expire, or a
                 * command to be received.
                 */
                xResult = prvProcessTimerOrBlockDgCoRoutine(xNextExpireTime, xListWasEmpty);

                if (xResult == errQUEUE_YIELD) {
                        crSET_STATE0(xHandle);
                }

                /* Empty the command queue. */
                prvProcessReceivedCommands();
        }

        /* CoRoutine end point. */
        crEND();
}
/*-----------------------------------------------------------*/

static BaseType_t prvProcessTimerOrBlockDgCoRoutine(const TickType_t xNextExpireTime,
        BaseType_t xListWasEmpty)
{
        BaseType_t xReturn = pdPASS;
        TickType_t xTimeNow;
        BaseType_t xTimerListsWereSwitched;

        /* Obtain the time now and check whether timer lists were switched. */
        xTimeNow = prvSampleTimeNow(&xTimerListsWereSwitched);
        if (xTimerListsWereSwitched == pdFALSE) {
                /* If a timer has expired, process it. */
                if ((xListWasEmpty == pdFALSE) && (xNextExpireTime <= xTimeNow)) {
                        prvProcessExpiredTimer(xNextExpireTime, xTimeNow);
                } else {
                        /*
                         * The tick count has not overflowed, and the next expire time has not been
                         * reached yet. Therefore the co-routine should block to wait for the next
                         * expire time or a command to be received.
                         */
                        if (xListWasEmpty != pdFALSE) {
                                /* If the current timer list is empty check the overflow list. */
                                xListWasEmpty = listLIST_IS_EMPTY(pxOverflowTimerList);
                        }

                        vQueueDgCRWaitForMessageRestricted(xTimerQueue, (xNextExpireTime - xTimeNow),
                                xListWasEmpty);

                        xReturn = errQUEUE_YIELD;
                }
        } else {
                mtCOVERAGE_TEST_MARKER();
        }

        return xReturn;
}
/*-----------------------------------------------------------*/

#endif /* configUSE_DIALOG_CO_ROUTINES && configUSE_TIMERS */
