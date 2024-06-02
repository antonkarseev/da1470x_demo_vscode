/**
 ****************************************************************************************
 *
 * @file dialog_queue.c
 *
 * @brief Extensions to FreeRTOS queue.c
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/* Standard includes. */
#include <stdlib.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"

#if (configUSE_DIALOG_CO_ROUTINES == 1)

#define _NON_STANDALONE_QUEUE_C
#include "queue.c"

/* File private functions -----------------------------------*/

/* Copy data from a queue. */
static void prvCopyDgCRDataFromQueue(Queue_t * const pxQueue, void * const pvBuffer,
        UBaseType_t xPeek) PRIVILEGED_FUNCTION;

/* Copy data to a queue. */
static BaseType_t prvCopyDgCRDataToQueue(Queue_t * const pxQueue, const void *pvItemToQueue,
        UBaseType_t xOverwrite) PRIVILEGED_FUNCTION;

/*-----------------------------------------------------------*/

BaseType_t xQueueDgCRGenericSend(QueueHandle_t xQueue, const void *pvItemToQueue,
        TickType_t xTicksToWait, UBaseType_t xOverwrite)
{
        BaseType_t xReturn, xYieldRequired;
        Queue_t * const pxQueue = (Queue_t *)xQueue;

        if (xOverwrite == pdFALSE) {
                dgcrENTER_CRITICAL();
                {
                        /* Check whether the queue is full and need to block on the queue. */
                        if (pxQueue->uxMessagesWaiting == pxQueue->uxLength) {
                                /* If the queue is full and a timeout is defined, then block. */
                                if (xTicksToWait > (TickType_t) 0) {
                                        vCoRoutineAddToDelayedList(xTicksToWait,
                                                &(pxQueue->xTasksWaitingToSend));
                                        dgcrEXIT_CRITICAL();
                                        return errQUEUE_BLOCKED;
                                } else {
                                        dgcrEXIT_CRITICAL();
                                        return errQUEUE_FULL;
                                }
                        }
                }
                dgcrEXIT_CRITICAL();
        }

        dgcrENTER_CRITICAL();
        {
                /* Check whether the queue is not full in order to copy data into the queue. */
                if ((pxQueue->uxMessagesWaiting < pxQueue->uxLength) || (xOverwrite != pdFALSE)) {
                        /* Copy data into queue if item size is non-zero. */
                        xYieldRequired = prvCopyDgCRDataToQueue(pxQueue, pvItemToQueue, xOverwrite);

                        xReturn = pdPASS;

                        /* Check whether there are co-routines waiting for data to become available. */
                        if (listLIST_IS_EMPTY(&(pxQueue->xTasksWaitingToReceive)) == pdFALSE) {
                                /* Place first blocked co-routine into the ready list. */
                                if (xCoRoutineRemoveFromEventList(&(pxQueue->xTasksWaitingToReceive)) != pdFALSE) {
#if (configUSE_PREEMPTION == 1)
                                        xReturn = errQUEUE_YIELD;
#endif
                                } else {
                                        mtCOVERAGE_TEST_MARKER();
                                }
                        } else if (xYieldRequired != pdFALSE) {
                                /*
                                 * The co-routine was holding multiple mutexes which resulted in
                                 * priority inheritance, and they were given all back. Indicate
                                 * that co-routine should yield its execution in case a higher
                                 * priority co-routine was unblocked.
                                 */
#if (configUSE_PREEMPTION == 1)
                                xReturn = errQUEUE_YIELD;
#endif
                                vDgCoRoutineMissedYield();
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
                } else {
                        xReturn = errQUEUE_FULL;
                }
        }
        dgcrEXIT_CRITICAL();

        return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xQueueDgCRGenericReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait,
        UBaseType_t xPeek)
{
        BaseType_t xReturn;
        Queue_t * const pxQueue = (Queue_t *)xQueue;

        dgcrENTER_CRITICAL();
        {
                /* Check whether the queue is empty and need to block on the queue. */
                if (pxQueue->uxMessagesWaiting == (UBaseType_t) 0) {
                        /* If the queue is empty and a timeout is defined, then block. */
                        if (xTicksToWait > (TickType_t) 0) {
                                vCoRoutineAddToDelayedList(xTicksToWait,
                                        &(pxQueue->xTasksWaitingToReceive));
#if (configUSE_MUTEXES == 1)
                                if (pxQueue->uxQueueType == queueQUEUE_IS_MUTEX) {
                                        xDgCoRoutinePriorityInherit((void *)pxQueue->u.xSemaphore.xMutexHolder);
                                } else {
                                        mtCOVERAGE_TEST_MARKER();
                                }
#endif /* configUSE_MUTEXES */
                                dgcrEXIT_CRITICAL();
                                return errQUEUE_BLOCKED;
                        } else {
                                dgcrEXIT_CRITICAL();
                                return errQUEUE_EMPTY;
                        }
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        }
        dgcrEXIT_CRITICAL();

        dgcrENTER_CRITICAL();
        {
                /* Check whether the queue is not empty in order to retrieve the first item. */
                if (pxQueue->uxMessagesWaiting > (UBaseType_t) 0) {
                        /* Copy data from the queue if item size is non-zero. */
                        prvCopyDgCRDataFromQueue(pxQueue, pvBuffer, xPeek);

#if (configUSE_MUTEXES == 1)
                        if (pxQueue->uxQueueType == queueQUEUE_IS_MUTEX) {
                                /* Record information required for priority inheritance. */
                                pxQueue->u.xSemaphore.xMutexHolder = pvDgCoRoutineIncrementMutexHeldCount();
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
#endif /* configUSE_MUTEXES */

                        xReturn = pdPASS;

                        /*
                         * If an item is to be received and removed from queue, then check
                         * whether there are co-routines waiting for space to become available.
                         */
                        if ((xPeek == pdFALSE)
                                && (listLIST_IS_EMPTY(&(pxQueue->xTasksWaitingToSend)) == pdFALSE)) {
                                /* Place first blocked co-routine into the ready list. */
                                if (xCoRoutineRemoveFromEventList(&(pxQueue->xTasksWaitingToSend)) != pdFALSE) {
#if (configUSE_PREEMPTION == 1)
                                        xReturn = errQUEUE_YIELD;
#endif
                                } else {
                                        mtCOVERAGE_TEST_MARKER();
                                }
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
                } else {
                        xReturn = errQUEUE_EMPTY;
                }
        }
        dgcrEXIT_CRITICAL();

        return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xQueueDgCRGenericSendFromISR(QueueHandle_t xQueue, const void *pvItemToQueue,
        UBaseType_t xOverwrite, BaseType_t *pxCoRoutineWoken)
{
        BaseType_t xReturn;
        Queue_t * const pxQueue = (Queue_t *)xQueue;
        UBaseType_t uxSavedInterruptStatus;

        /* Safety check imposed by FreeRTOS kernel */
        portASSERT_IF_INTERRUPT_PRIORITY_INVALID();

        uxSavedInterruptStatus = dgcrENTER_CRITICAL_FROM_ISR();

        /* Check whether the queue is not full in order to copy data into the queue. */
        if ((pxQueue->uxMessagesWaiting < pxQueue->uxLength) || (xOverwrite != pdFALSE)) {
                /* Copy data into queue if item size is non-zero. */
                (void) prvCopyDgCRDataToQueue(pxQueue, pvItemToQueue, xOverwrite);

                xReturn = pdPASS;

                /* Check that a co-routine has not already been woken. */
                if ((pxCoRoutineWoken == NULL) || ((*pxCoRoutineWoken) == pdFALSE)) {
                        /*
                         * If no co-routine has been woken from the current ISR, check whether
                         * there are co-routines waiting for data to become available.
                         */
                        if (listLIST_IS_EMPTY(&(pxQueue->xTasksWaitingToReceive)) == pdFALSE) {
                                /* Place first blocked co-routine into the ready list. */
                                (void) xCoRoutineRemoveFromEventList(&(pxQueue->xTasksWaitingToReceive));

                                if (pxCoRoutineWoken != NULL) {
                                        *pxCoRoutineWoken = pdTRUE;
                                }
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        } else {
                xReturn = errQUEUE_FULL;
        }

        dgcrEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

        return xReturn;
}
/*-----------------------------------------------------------*/

BaseType_t xQueueDgCRGenericReceiveFromISR(QueueHandle_t xQueue, void *pvBuffer, UBaseType_t xPeek,
        BaseType_t *pxCoRoutineWoken)
{
        BaseType_t xReturn;
        Queue_t * const pxQueue = (Queue_t *)xQueue;
        UBaseType_t uxSavedInterruptStatus;

        /* Safety check imposed by FreeRTOS kernel */

        uxSavedInterruptStatus = dgcrENTER_CRITICAL_FROM_ISR();

        /* Check whether the queue is not empty in order to copy data from the queue. */
        if (pxQueue->uxMessagesWaiting > (UBaseType_t) 0) {
                /* Copy data from the queue if item size is non-zero. */
                prvCopyDgCRDataFromQueue(pxQueue, pvBuffer, xPeek);

                /*
                 * If an item is to be received and removed from queue, check that a co-routine
                 * has not already been woken.
                 */
                if ((xPeek != pdFALSE)
                        && ((pxCoRoutineWoken == NULL) || ((*pxCoRoutineWoken) == pdFALSE))) {
                        /*
                         * If no co-routine has been woken from the current ISR, check whether
                         * there are co-routines waiting for space to become available.
                         */
                        if (listLIST_IS_EMPTY(&(pxQueue->xTasksWaitingToSend)) == pdFALSE) {
                                /* Place first blocked co-routine into the ready list. */
                                (void) xCoRoutineRemoveFromEventList(&(pxQueue->xTasksWaitingToSend));

                                if (pxCoRoutineWoken != NULL) {
                                        *pxCoRoutineWoken = pdTRUE;
                                }
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }

                xReturn = pdPASS;
        } else {
                xReturn = errQUEUE_EMPTY;
        }

        dgcrEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

        return xReturn;
}
/*-----------------------------------------------------------*/

#if (configUSE_TIMERS == 1)

void vQueueDgCRWaitForMessageRestricted(QueueHandle_t xQueue, TickType_t xTicksToWait,
        const BaseType_t xWaitIndefinitely)
{
        Queue_t * const pxQueue = (Queue_t *)xQueue;

        dgcrENTER_CRITICAL();
        {
                /* If there are no messages in the queue, block for the specified period. */
                if (pxQueue->uxMessagesWaiting == (UBaseType_t) 0) {
                        if (xWaitIndefinitely != pdFALSE) {
                                xTicksToWait = portMAX_DELAY;
                        }

                        vDgCoRoutinePlaceOnEventListRestricted(&(pxQueue->xTasksWaitingToReceive),
                                xTicksToWait);
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        }
        dgcrEXIT_CRITICAL();
}
#endif /* configUSE_TIMERS */
/*-----------------------------------------------------------*/

static BaseType_t prvCopyDgCRDataToQueue(Queue_t * const pxQueue, const void *pvItemToQueue,
        UBaseType_t xOverwrite)
{
        BaseType_t xReturn = pdFALSE;
        UBaseType_t uxMessagesWaiting;

        uxMessagesWaiting = pxQueue->uxMessagesWaiting;

        /* If the item size is equal to zero, deal only with mutex inheritance. */
        if (pxQueue->uxItemSize == (UBaseType_t) 0) {
#if (configUSE_MUTEXES == 1)
                if (pxQueue->uxQueueType == queueQUEUE_IS_MUTEX) {
                        xReturn = xDgCoRoutinePriorityDisinherit((void *)pxQueue->u.xSemaphore.xMutexHolder);
                        pxQueue->u.xSemaphore.xMutexHolder = NULL;
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
#endif /* configUSE_MUTEXES */
        } else if (xOverwrite != pdFALSE) {
                /* The item size is greater than zero. Overwrite item to the queue. */
                int8_t *pcWriteTo = pxQueue->pcWriteTo;
                if ((pcWriteTo == pxQueue->pcHead) && (uxMessagesWaiting > 0)) {
                        pcWriteTo = pxQueue->u.xQueue.pcTail - pxQueue->uxItemSize;
                        --uxMessagesWaiting;
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
                memcpy((void *)pcWriteTo, pvItemToQueue, (size_t)pxQueue->uxItemSize);
        } else {
                /*
                 * The item size is greater than zero. Copy item to the queue and update
                 * write position.
                 */
                memcpy((void *)pxQueue->pcWriteTo, pvItemToQueue, (size_t)pxQueue->uxItemSize);
                pxQueue->pcWriteTo += pxQueue->uxItemSize;
                if (pxQueue->pcWriteTo >= pxQueue->u.xQueue.pcTail) {
                        pxQueue->pcWriteTo = pxQueue->pcHead;
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        }
        /* Increment the number of messages waiting in the queue. */
        pxQueue->uxMessagesWaiting = uxMessagesWaiting + (UBaseType_t) 1;

        return xReturn;
}
/*-----------------------------------------------------------*/

static void prvCopyDgCRDataFromQueue(Queue_t * const pxQueue, void * const pvBuffer,
        UBaseType_t xPeek)
{
        int8_t *pcOriginalReadPosition;

        /* If a message is to be peeked, temporarily store its original position. */
        if (xPeek) {
                pcOriginalReadPosition = pxQueue->u.xQueue.pcReadFrom;
        } else {
                mtCOVERAGE_TEST_MARKER();
        }

        /* If the item size is greater than zero, copy it to the buffer and update read position. */
        if (pxQueue->uxItemSize != (UBaseType_t) 0) {
                pxQueue->u.xQueue.pcReadFrom += pxQueue->uxItemSize;
                if (pxQueue->u.xQueue.pcReadFrom >= pxQueue->u.xQueue.pcTail) {
                        pxQueue->u.xQueue.pcReadFrom = pxQueue->pcHead;
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
                memcpy((void *)pvBuffer, (void *)pxQueue->u.xQueue.pcReadFrom, (size_t)pxQueue->uxItemSize);
        } else {
                mtCOVERAGE_TEST_MARKER();
        }

        /*
         * If a message is to be peeked, restore the read position, otherwise
         * decrement the number of messages waiting in the queue.
         */
        if (xPeek) {
                pxQueue->u.xQueue.pcReadFrom = pcOriginalReadPosition;
        } else {
                (pxQueue->uxMessagesWaiting)--;
        }
}
/*-----------------------------------------------------------*/

#endif /* configUSE_DIALOG_CO_ROUTINES */
