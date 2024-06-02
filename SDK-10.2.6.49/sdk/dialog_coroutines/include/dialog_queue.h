/**
 ****************************************************************************************
 *
 * @file dialog_queue.h
 *
 * @brief Extensions to FreeRTOS queues
 *
 * Copyright (C) 2020-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DIALOG_QUEUE_H_
#define DIALOG_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Post item on queue
 *
 * This function is used for the calling co-routine to post an item on a queue (i.e. the back of
 * the queue) by copy, not by reference. If there is no space available on the queue, the
 * co-routine blocks and waits for space to become available. The co-routine does not consume any
 * CPU time while it is in the Blocked state.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine (e.g. with higher
 * priority).
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xQueue handle of the queue on which the item will be posted
 * \param [in] pvItemToQueue pointer to the item that will be placed on the queue
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for space to become available on the queue
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if the item
 * was successfully posted on the queue, otherwise errQUEUE_FULL
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrQUEUE_SEND(xHandle, xQueue, pvItemToQueue, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xQueueDgCRSend((xQueue), (pvItemToQueue), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xQueueDgCRSend((xQueue), (pvItemToQueue), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrQUEUE_SEND(xHandle, xQueue, pvItemToQueue, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xQueueDgCRSend((xQueue), (pvItemToQueue), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xQueueDgCRSend((xQueue), (pvItemToQueue), 0); \
                } \
        } while (0)
#endif

/**
 * \brief Overwrite item on queue
 *
 * This function is used for the calling co-routine either to post an item on a queue if the queue
 * is empty, or overwrite the last item on the queue, by copy, not by reference. The calling
 * co-routine does not block and wait for space to become available if the queue is full, because it
 * overwrites the last item.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine with higher
 * priority.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xQueue handle of the queue on which the item will be posted
 * \param [in] pvItemToQueue pointer to the item that will be placed on the queue
 * \param [out] pxResult pointer to store the result of this function, that is, always pdPASS
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrQUEUE_OVERWRITE(xHandle, xQueue, pvItemToQueue, pxResult) \
        do { \
                *(pxResult) = xQueueDgCROverwrite((xQueue), (pvItemToQueue)); \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrQUEUE_OVERWRITE(xHandle, xQueue, pvItemToQueue, pxResult) \
        do { \
                *(pxResult) = xQueueDgCROverwrite((xQueue), (pvItemToQueue)); \
        } while (0)
#endif

/**
 * \brief Receive item from queue
 *
 * This function is used for the calling co-routine to receive and remove an item from a queue
 * (i.e. the front of the queue) by copy, not by reference, and place it to a buffer. Therefore,
 * the size of the buffer must be adequate. If there is no data available in the queue, the
 * co-routine blocks and waits for data to become available. The co-routine does not consume any
 * CPU time while it is in the Blocked state.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine (e.g. with higher
 * priority).
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xQueue handle of the queue from which the item will be received
 * \param [out] pvBuffer pointer to the buffer into which the received item will be copied
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for data to become available from the queue
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if the item
 * was successfully received from the queue, otherwise errQUEUE_EMPTY
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrQUEUE_RECEIVE(xHandle, xQueue, pvBuffer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xQueueDgCRReceive((xQueue), (pvBuffer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xQueueDgCRReceive((xQueue), (pvBuffer), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrQUEUE_RECEIVE(xHandle, xQueue, pvBuffer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xQueueDgCRReceive((xQueue), (pvBuffer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xQueueDgCRReceive((xQueue), (pvBuffer), 0); \
                } \
        } while (0)
#endif

/**
 * \brief Retrieve item from queue without removing it
 *
 * This function is used for the calling co-routine to retrieve an item from a queue (i.e. the
 * front of the queue) by copy, not by reference, and place it to a buffer. Therefore, the size of
 * the buffer must be adequate. If there is no data available in the queue, the co-routine blocks
 * and waits for data to become available. The co-routine does not consume any CPU time while it
 * is in the Blocked state. The item is not removed from the queue.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack,
 * and the calling co-routine may yield its execution.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xQueue handle of the queue from which the item will be retrieved
 * \param [out] pvBuffer pointer to the buffer into which the retrieved item will be copied
 * \param [in] xTicksToWait maximum amount of time in ticks the co-routine should wait in the
 * Blocked state for data to become available from the queue
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if the item
 * was successfully retrieved from the queue, otherwise errQUEUE_EMPTY
 */
#define dgcrQUEUE_PEEK(xHandle, xQueue, pvBuffer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xQueueDgCRPeek((xQueue), (pvBuffer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xQueueDgCRPeek((xQueue), (pvBuffer), 0); \
                } \
        } while (0)

/**
 * \brief Post item on queue from ISR
 *
 * This function is used for posting from interrupt service routine (ISR) an item on a queue
 * (i.e. the back of the queue) by copy, not by reference.
 *
 * \param [in] xQueue handle of the queue on which the item will be posted
 * \param [in] pvItemToQueue pointer to the item that will be placed on the queue
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by posting an item on the queue
 *
 * \return pdPASS if the item was successfully posted on the queue, otherwise errQUEUE_FULL
 */
#define dgcrQUEUE_SEND_FROM_ISR(xQueue, pvItemToQueue, pxCoRoutineWoken) \
        xQueueDgCRSendFromISR((xQueue), (pvItemToQueue), (pxCoRoutineWoken))

/**
 * \brief Overwrite item on queue from ISR
 *
 * This function is used for either posting from interrupt service routine (ISR) an item on a
 * queue if the queue is empty, or overwriting the last item on the queue, by copy, not by
 * reference.
 *
 * \param [in] xQueue handle of the queue on which the item will be posted
 * \param [in] pvItemToQueue pointer to the item that will be placed on the queue
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by posting an item on the queue
 *
 * \return pdPASS
 */
#define dgcrQUEUE_OVERWRITE_FROM_ISR(xQueue, pvItemToQueue, pxCoRoutineWoken) \
        xQueueDgCROverwriteFromISR((xQueue), (pvItemToQueue), (pxCoRoutineWoken))

/**
 * \brief Receive item from queue from ISR
 *
 * This function is used for receiving and removing from interrupt service routine (ISR) an item
 * from a queue (i.e. the front of the queue) by copy, not by reference, and placing it to a buffer.
 * Therefore, the size of the buffer must be adequate.
 *
 * \param [in] xQueue handle of the queue from which the item will be received
 * \param [out] pvBuffer pointer to the buffer into which the received item will be copied
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by receiving an item from the queue
 *
 * \return pdPASS if the item was successfully received from the queue, otherwise errQUEUE_EMPTY
 */
#define dgcrQUEUE_RECEIVE_FROM_ISR(xQueue, pvBuffer, pxCoRoutineWoken) \
        xQueueDgCRReceiveFromISR((xQueue), (pvBuffer), (pxCoRoutineWoken))

/**
 * \brief Retrieve item from queue from ISR without removing it
 *
 * This function is used for retrieving from interrupt service routine (ISR) an item from a queue
 * (i.e. the front of the queue) by copy, not by reference, and placing it to a buffer. Therefore,
 * the size of the buffer must be adequate. The item is not removed from the queue.
 *
 * \param [in] xQueue handle of the queue from which the item will be retrieved
 * \param [out] pvBuffer pointer to the buffer into which the retrieved item will be copied
 *
 * \return pdPASS if the item was successfully retrieved from the queue, otherwise errQUEUE_EMPTY
 */
#define dgcrQUEUE_PEEK_FROM_ISR(xQueue, pvBuffer) \
        xQueueDgCRPeekFromISR((xQueue), (pvBuffer))

/**
 * \brief Post item on queue
 *
 * This macro is used for the calling co-routine to post an item on a queue (i.e. the back of
 * the queue) by copy, not by reference, and indicates whether the co-routine needs to block and
 * wait for space to become available on the queue.
 *
 * This macro must not be called from an interrupt service routine (ISR).
 *
 * \param [in] xQueue handle of the queue on which the item will be posted
 * \param [in] pvItemToQueue pointer to the item that will be placed on the queue
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for space to become available on the queue
 *
 * \return pdPASS if the item was successfully posted on the queue, otherwise an error code as
 * defined within ProjDefs.h
 */
#define xQueueDgCRSend(xQueue, pvItemToQueue, xTicksToWait) \
        xQueueDgCRGenericSend((xQueue), (pvItemToQueue), (xTicksToWait), pdFALSE)

/**
 * \brief Overwrite item on queue
 *
 * This macro is used for the calling co-routine either to post an item on a queue if the queue
 * is empty, or overwrite the last item on the queue, by copy, not by reference.
 *
 * This macro must not be called from an interrupt service routine (ISR).
 *
 * \param [in] xQueue handle of the queue on which the item will be posted
 * \param [in] pvItemToQueue pointer to the item that will be placed on the queue
 *
 * \return pdPASS
 */
#define xQueueDgCROverwrite(xQueue, pvItemToQueue) \
        xQueueDgCRGenericSend((xQueue), (pvItemToQueue), 0, pdTRUE)

/**
 * \brief Post item on queue
 *
 * It is preferred that the macros xQueueDgCRSend() and xQueueDgCROverwrite() are used instead of
 * calling this function directly.
 *
 * This function is used for the calling co-routine to either post an item on a queue (i.e. the
 * back of the queue) by copy, not by reference.
 *
 * This function must not be called from an interrupt service routine (ISR).
 *
 * \param [in] xQueue handle of the queue on which the item will be posted
 * \param [in] pvItemToQueue pointer to the item that will be placed on the queue
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for space to become available on the queue
 * \param [in] xOverwrite value indicating whether the last item on the queue will be overwritten
 * if the queue is not empty
 *
 * \return pdPASS if the item was successfully posted on the queue, otherwise an error code as
 * defined within ProjDefs.h
 *
 * \sa xQueueDgCRSend
 * \sa xQueueDgCROverwrite
 */
BaseType_t xQueueDgCRGenericSend(QueueHandle_t xQueue, const void *pvItemToQueue,
        TickType_t xTicksToWait, UBaseType_t xOverwrite) PRIVILEGED_FUNCTION;

/**
 * \brief Receive item from queue
 *
 * This macro is used for the calling co-routine to receive and remove an item from a queue
 * (i.e. the front of the queue) by copy, not by reference, and indicates whether the co-routine
 * needs to block and wait for data to become available from the queue. The received item is placed
 * to a buffer, the size of which must be adequate.
 *
 * This macro must not be called from an interrupt service routine (ISR).
 *
 * \param [in] xQueue handle of the queue from which the item will be received
 * \param [out] pvBuffer pointer to the buffer into which the received item will be copied
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for data to become available from the queue
 *
 * \return pdPASS if the item was successfully received from the queue, otherwise an error code as
 * defined within ProjDefs.h
 */
#define xQueueDgCRReceive(xQueue, pvBuffer, xTicksToWait) \
        xQueueDgCRGenericReceive((xQueue), (pvBuffer), (xTicksToWait), pdFALSE)

/**
 * \brief Retrieve item from queue without removing it
 *
 * This macro is used for the calling co-routine to retrieve an item from a queue
 * (i.e. the front of the queue) by copy, not by reference, and indicates whether the co-routine
 * needs to block and wait for data to become available from the queue. The received item is placed
 * to a buffer, the size of which must be adequate. The item is not removed from the queue.
 *
 * This macro must not be called from an interrupt service routine (ISR).
 *
 * \param [in] xQueue handle of the queue from which the item will be retrieved
 * \param [out] pvBuffer pointer to the buffer into which the retrieved item will be copied
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for data to become available from the queue
 *
 * \return pdPASS if the item was successfully retrieved from the queue, otherwise an error code as
 * defined within ProjDefs.h
 */
#define xQueueDgCRPeek(xQueue, pvBuffer, xTicksToWait) \
        xQueueDgCRGenericReceive((xQueue), (pvBuffer), (xTicksToWait), pdTRUE)

/**
 * \brief Retrieve item from queue
 *
 * It is preferred that the macros xQueueDgCRReceive() and xQueueDgCRPeek() are used instead of
 * calling this function directly.
 *
 * This function is used for the calling co-routine to retrieve an item from a queue
 * (i.e. the front of the queue) by copy, not by reference, and indicates whether the co-routine
 * needs to block and wait for data to become available from the queue. The received item is placed
 * to a buffer, the size of which must be adequate.
 *
 * This function must not be called from an interrupt service routine (ISR).
 *
 * \param [in] xQueue handle of the queue from which the item will be received
 * \param [out] pvBuffer pointer to the buffer into which the received item will be copied
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for data to become available from the queue
 * \param [in] xPeek value indicating whether the received item will be removed-peeked from the queue
 *
 * \return pdPASS if the item was successfully received from the queue, otherwise an error code as
 * defined within ProjDefs.h
 */
BaseType_t xQueueDgCRGenericReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait,
        UBaseType_t xPeek) PRIVILEGED_FUNCTION;

/**
 * \brief Post item on queue from ISR
 *
 * This macro is used for posting from interrupt service routine (ISR) an item on a queue
 * (i.e. the back of the queue) by copy, not by reference.
 *
 * \param [in] xQueue handle of the queue on which the item will be posted
 * \param [in] pvItemToQueue pointer to the item that will be placed on the queue
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by posting an item on the queue
 *
 * \return pdPASS if the item was successfully posted on the queue, otherwise errQUEUE_FULL
 */
#define xQueueDgCRSendFromISR(xQueue, pvItemToQueue, pxCoRoutineWoken) \
        xQueueDgCRGenericSendFromISR((xQueue), (pvItemToQueue), pdFALSE, (pxCoRoutineWoken))

/**
 * \brief Overwrite item on queue from ISR
 *
 * This macro is used for either posting from interrupt service routine (ISR) an item on a
 * queue if the queue is empty, or overwriting the last item on the queue, by copy, not by
 * reference.
 *
 * \param [in] xQueue handle of the queue on which the item will be posted
 * \param [in] pvItemToQueue pointer to the item that will be placed on the queue
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by posting an item on the queue
 *
 * \return pdPASS
 */
#define xQueueDgCROverwriteFromISR(xQueue, pvItemToQueue, pxCoRoutineWoken) \
        xQueueDgCRGenericSendFromISR((xQueue), (pvItemToQueue), pdTRUE, (pxCoRoutineWoken))

/**
 * \brief Post item on queue from ISR
 *
 * It is preferred that the macros xQueueDgCRSendFromISR() and xQueueDgCROverwriteFromISR() are
 * used instead of calling this function directly.
 *
 * This function is used for posting from interrupt service routine (ISR) an item on a queue
 * (i.e. the back of the queue) by copy, not by reference.
 *
 * \param [in] xQueue handle of the queue on which the item will be posted
 * \param [in] pvItemToQueue pointer to the item that will be placed on the queue
 * \param [in] xOverwrite value indicating whether the last item on the queue will be overwritten
 * if the queue is not empty
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by posting an item on the queue
 *
 * \return pdPASS if the item was successfully posted on the queue, otherwise errQUEUE_FULL
 *
 * \sa xQueueDgCRSendFromISR
 * \sa xQueueDgCROverwriteFromISR
 */
BaseType_t xQueueDgCRGenericSendFromISR(QueueHandle_t xQueue, const void *pvItemToQueue,
        UBaseType_t xOverwrite, BaseType_t *pxCoRoutineWoken) PRIVILEGED_FUNCTION;

/**
 * \brief Receive item from queue from ISR
 *
 * This macro is used for receiving and removing an item from a queue (i.e. the front of the
 * queue) by copy, not by reference. The received item is placed to a buffer, the size of which
 * must be adequate.
 *
 * \param [in] xQueue handle of the queue from which the item will be received
 * \param [out] pvBuffer pointer to the buffer into which the received item will be copied
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by receiving an item from the queue
 *
 * \return pdPASS if the item was successfully received from the queue, otherwise errQUEUE_EMPTY
 */
#define xQueueDgCRReceiveFromISR(xQueue, pvBuffer, pxCoRoutineWoken) \
        xQueueDgCRGenericReceiveFromISR((xQueue), (pvBuffer), pdFALSE, (pxCoRoutineWoken))

/**
 * \brief Retrieve item from queue from ISR without removing it
 *
 * This macro is used for retrieving an item from a queue (i.e. the front of the queue) by copy,
 * not by reference. The retrieved item is placed to a buffer, the size of which must be adequate.
 * The item is not removed from the queue.
 *
 * \param [in] xQueue handle of the queue from which the item will be retrieved
 * \param [out] pvBuffer pointer to the buffer into which the retrieved item will be copied
 *
 * \return pdPASS if the item was successfully retrieved from the queue, otherwise errQUEUE_EMPTY
 */
#define xQueueDgCRPeekFromISR(xQueue, pvBuffer) \
        xQueueDgCRGenericReceiveFromISR((xQueue), (pvBuffer), pdTRUE, NULL)

/**
 * \brief Retrieve item from queue from ISR
 *
 * It is preferred that the macros xQueueDgCRReceiveFromISR() and xQueueDgCRPeekFromISR() are used
 * instead of calling this function directly.
 *
 * This function is used for retrieving an item from a queue (i.e. the front of the queue) by copy,
 * not by reference. The retrieved item is placed to a buffer, the size of which must be adequate.
 *
 * \param [in] xQueue handle of the queue from which the item will be received
 * \param [out] pvBuffer pointer to the buffer into which the received item will be copied
 * \param [in] xPeek value indicating whether the received item will be removed-peeked from the queue
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by receiving an item from the queue
 *
 * \return pdPASS if the item was successfully received from the queue, otherwise errQUEUE_EMPTY
 */
BaseType_t xQueueDgCRGenericReceiveFromISR(QueueHandle_t xQueue, void *pvBuffer, UBaseType_t xPeek,
        BaseType_t *pxCoRoutineWoken) PRIVILEGED_FUNCTION;


/*
 * Macros and functions beyond this part are not part of the public API and are intended for use
 * by the kernel only.
 */

/*
 * Give/take mutex recursively.
 *
 * Use xSemaphoreDgCRTakeRecursive() or xSemaphoreDgCRGiveRecursive() instead of these macros.
 */
#define xQueueDgCRTakeMutexRecursive(xMutex, xTicksToWait) \
        xQueueTakeMutexRecursive((xMutex), (xTicksToWait));
#define xQueueDgCRGiveMutexRecursive(xMutex) \
        xQueueGiveMutexRecursive(xMutex)

/*
 * Obtain semaphore using queue.
 *
 * Use xSemaphoreDgCRTake() instead of this macro.
 */
#define xQueueDgCRSemaphoreTake(xQueue, xTicksToWait) \
        xQueueDgCRReceive((xQueue), NULL, (xTicksToWait))

/*
 * Check semaphore using queue.
 *
 * Use xSemaphoreDgCRCheck() instead of this macro.
 */
#define xQueueDgCRSemaphoreCheck(xQueue, xTicksToWait) \
        xQueueDgCRPeek((xQueue), NULL, (xTicksToWait))

/*
 * Block calling co-routine to wait for messages to be received from the queue.
 */
void vQueueDgCRWaitForMessageRestricted(QueueHandle_t xQueue, TickType_t xTicksToWait,
        const BaseType_t xWaitIndefinitely) PRIVILEGED_FUNCTION;

#ifdef __cplusplus
}
#endif

#endif /* DIALOG_QUEUE_H_ */
