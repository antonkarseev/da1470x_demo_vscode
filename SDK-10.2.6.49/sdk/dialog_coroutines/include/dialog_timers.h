/**
 ****************************************************************************************
 *
 * @file dialog_timers.h
 *
 * @brief Extensions to FreeRTOS timers.h
 *
 * Copyright (C) 2020-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DIALOG_TIMERS_H_
#define DIALOG_TIMERS_H_

#include "croutine.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Start timer
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for the calling co-routine to start a timer that was previously created
 * (e.g. using xTimerCreate()). If the timer had already been started and was already in the active
 * state, the timer is restarted. The callback function associated with the timer will be called
 * 'n' ticks after dgcrTIMER_START() was called, where 'n' is the timer's defined period.
 *
 * Since the start command is sent through a queue to the timer service/daemon co-routine,
 * if there is no space available on the queue, the co-routine blocks and waits for space to become
 * available. The co-routine does not consume any CPU time while it is in the Blocked state.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine (e.g. the timer
 * service/daemon co-routine having higher priority).
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xTimer handle of the timer being started/restarted
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the start command to be successfully posted to the timer command queue
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if the
 * command was successfully posted on the timer command queue, otherwise pdFAIL
 *
 * \sa xTimerCreate
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrTIMER_START(xHandle, xTimer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRStart((xTimer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRStart((xTimer), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrTIMER_START(xHandle, xTimer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRStart((xTimer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRStart((xTimer), 0); \
                } \
        } while (0)
#endif

/**
 * \brief Stop timer
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for the calling co-routine to stop an already started timer (e.g. using
 * dgcrTIMER_START()).
 *
 * Since the stop command is sent through a queue to the timer service/daemon co-routine,
 * if there is no space available on the queue, the co-routine blocks and waits for space to become
 * available. The co-routine does not consume any CPU time while it is in the Blocked state.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine (e.g. the timer
 * service/daemon co-routine having higher priority).
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xTimer handle of the timer being stopped
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the stop command to be successfully posted to the timer command queue
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if the
 * command was successfully posted on the timer command queue, otherwise pdFAIL
 *
 * \sa dgcrTIMER_START
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrTIMER_STOP(xHandle, xTimer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRStop((xTimer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRStop((xTimer), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrTIMER_STOP(xHandle, xTimer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRStop((xTimer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRStop((xTimer), 0); \
                } \
        } while (0)
#endif

/**
 * \brief Change timer period
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for the calling co-routine to change the period of a timer that was
 * previously created (e.g. using xTimerCreate()), and if the timer is not already active, it
 * starts it. The callback function associated with the timer will be called xNewPeriod ticks after
 * the period change command is processed by the timer service/daemon co-routine.
 *
 * Since the period change command is sent through a queue to the timer service/daemon co-routine,
 * if there is no space available on the queue, the co-routine blocks and waits for space to become
 * available. The co-routine does not consume any CPU time while it is in the Blocked state.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine (e.g. the timer
 * service/daemon co-routine having higher priority).
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xTimer handle of the timer of which the period is changed
 * \param [in] xNewPeriod new period in ticks for the timer
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the period change command to be successfully posted to the timer command
 * queue
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if the
 * command was successfully posted on the timer command queue, otherwise pdFAIL
 *
 * \sa xTimerCreate
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrTIMER_CHANGE_PERIOD(xHandle, xTimer, xNewPeriod, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRChangePeriod((xTimer), (xNewPeriod), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRChangePeriod((xTimer), (xNewPeriod), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrTIMER_CHANGE_PERIOD(xHandle, xTimer, xNewPeriod, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRChangePeriod((xTimer), (xNewPeriod), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRChangePeriod((xTimer), (xNewPeriod), 0); \
                } \
        } while (0)
#endif

/**
 * \brief Delete timer
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for the calling co-routine to delete a timer that was previously created
 * (e.g. using xTimerCreate()).
 *
 * Since the delete command is sent through a queue to the timer service/daemon co-routine,
 * if there is no space available on the queue, the co-routine blocks and waits for space to become
 * available. The co-routine does not consume any CPU time while it is in the Blocked state.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine (e.g. the timer
 * service/daemon co-routine having higher priority).
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xTimer handle of the timer being deleted
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the delete command to be successfully posted to the timer command queue
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if the
 * command was successfully posted on the timer command queue, otherwise pdFAIL
 *
 * \sa xTimerCreate
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrTIMER_DELETE(xHandle, xTimer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRDelete((xTimer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRDelete((xTimer), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrTIMER_DELETE(xHandle, xTimer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRDelete((xTimer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRDelete((xTimer), 0); \
                } \
        } while (0)
#endif

/**
 * \brief Reset timer
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for the calling co-routine to restart a timer that was previously created
 * (e.g. using xTimerCreate()). If the timer is not already active, it starts it, otherwise it
 * re-evaluates the time it will expire, making the the callback function associated with the timer
 * be called 'n' ticks after dgcrTIMER_RESET() was called, where 'n' is the timer's defined period.
 *
 * Since the reset command is sent through a queue to the timer service/daemon co-routine,
 * if there is no space available on the queue, the co-routine blocks and waits for space to become
 * available. The co-routine does not consume any CPU time while it is in the Blocked state.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine (e.g. the timer
 * service/daemon co-routine having higher priority).
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xTimer handle of the timer being reset
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the reset command to be successfully posted to the timer command queue
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if the
 * command was successfully posted on the timer command queue, otherwise pdFAIL
 *
 * \sa xTimerCreate
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrTIMER_RESET(xHandle, xTimer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRReset((xTimer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRReset((xTimer), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrTIMER_RESET(xHandle, xTimer, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRReset((xTimer), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRReset((xTimer), 0); \
                } \
        } while (0)
#endif

/**
 * \brief Defer function execution to timer daemon co-routine
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for the calling co-routine to defer the execution of a specific function
 * to the timer service/daemon co-routine.
 *
 * Since the function execution command is sent through a queue to the timer service/daemon
 * if there is no space available on the queue, the co-routine blocks and waits for space to become
 * available. The co-routine does not consume any CPU time while it is in the Blocked state.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine (e.g. the timer
 * service/daemon co-routine having higher priority).
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xFunctionToPend function of PendedFunction_t type to execute from the timer
 * service/daemon co-routine
 * \param [in] pvParameter1 value of the first parameter of the function the execution of which
 * is deferred
 * \param [in] ulParameter2 value of the second parameter of the function the execution of which
 * is deferred
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the function execution command to be successfully posted to the timer
 * command queue
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if the
 * command was successfully posted on the timer command queue, otherwise pdFAIL
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrTIMER_PEND_FUNCTION_CALL(xHandle, xFunctionToPend, pvParameter1, ulParameter2, \
                xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRPendFunctionCall((xHandle), (xFunctionToPend), \
                                (pvParameter1), (ulParameter2), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRPendFunctionCall((xHandle), (xFunctionToPend), \
                                        (pvParameter1), (ulParameter2), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrTIMER_PEND_FUNCTION_CALL(xHandle, xFunctionToPend, pvParameter1, ulParameter2, \
                xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xTimerDgCRPendFunctionCall((xHandle), (xFunctionToPend), \
                                (pvParameter1), (ulParameter2), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xTimerDgCRPendFunctionCall((xHandle), (xFunctionToPend), \
                                        (pvParameter1), (ulParameter2), 0); \
                } \
        } while (0)
#endif

/**
 * \brief Start timer from ISR
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for starting from interrupt service routine (ISR) a timer that was
 * previously created (e.g. using xTimerCreate()). If the timer had already been started and was
 * already in the active state, the timer is restarted. The callback function associated with the
 * timer will be called 'n' ticks after dgcrTIMER_START_FROM_ISR() was called, where 'n' is the
 * timer's defined period.
 *
 * Since the start command is sent through a queue to the timer service/daemon co-routine,
 * the function may fail to post the start command to the queue.
 *
 * \param [in] xTimer handle of the timer being started/restarted
 * \param [in,out] pxTimerCoRoutineWoken indication whether timer service/daemon co-routine has
 * already been unblocked from the current ISR by posting a command to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * pdFAIL
 *
 * \sa xTimerCreate
 */
#define dgcrTIMER_START_FROM_ISR(xTimer, pxTimerCoRoutineWoken) \
        xTimerDgCRStartFromISR((xTimer), (pxTimerCoRoutineWoken))

/**
 * \brief Stop timer from ISR
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for stopping from interrupt service routine (ISR) an already started timer
 * (e.g. using dgcrTIMER_START()).
 *
 * Since the stop command is sent through a queue to the timer service/daemon co-routine,
 * the function may fail to post the stop command to the queue.
 *
 * \param [in] xTimer handle of the timer being stopped
 * \param [in,out] pxTimerCoRoutineWoken indication whether timer service/daemon co-routine has
 * already been unblocked from the current ISR by posting a command to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * pdFAIL
 *
 * \sa dgcrTIMER_START
 */
#define dgcrTIMER_STOP_FROM_ISR(xTimer, pxTimerCoRoutineWoken) \
        xTimerDgCRStopFromISR((xTimer), (pxTimerCoRoutineWoken))

/**
 * \brief Change timer period from ISR
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for changing from interrupt service routine (ISR) the period of a timer
 * that was previously created (e.g. using xTimerCreate()), and if the timer is not already active,
 * it starts it. The callback function associated with the timer will be called xNewPeriod ticks
 * after the period change command is processed by the timer service/daemon co-routine.
 *
 * Since the period change command is sent through a queue to the timer service/daemon co-routine,
 * the function may fail to post the period change command to the queue.
 *
 * \param [in] xTimer handle of the timer of which the period is changed
 * \param [in] xNewPeriod new period in ticks for the timer
 * \param [in,out] pxTimerCoRoutineWoken indication whether timer service/daemon co-routine has
 * already been unblocked from the current ISR by posting a command to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * pdFAIL
 *
 * \sa xTimerCreate
 */
#define dgcrTIMER_CHANGE_PERIOD_FROM_ISR(xTimer, xNewPeriod, pxTimerCoRoutineWoken) \
        xTimerDgCRChangePeriodFromISR((xTimer), (xNewPeriod), (pxTimerCoRoutineWoken))

/**
 * \brief Reset timer from ISR
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for restarting from interrupt service routine (ISR) a timer that was
 * previously created (e.g. using xTimerCreate()). If the timer is not already active, it starts it,
 * otherwise it re-evaluates the time it will expire, making the the callback function associated
 * with the timer be called 'n' ticks after dgcrTIMER_RESET_FROM_ISR() was called, where 'n' is the
 * timer's defined period.
 *
 * Since the reset command is sent through a queue to the timer service/daemon co-routine,
 * the function may fail to post the reset command to the queue.
 *
 * \param [in] xTimer handle of the timer being reset
 * \param [in,out] pxTimerCoRoutineWoken indication whether timer service/daemon co-routine has
 * already been unblocked from the current ISR by posting a command to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * pdFAIL
 *
 * \sa xTimerCreate
 */
#define dgcrTIMER_RESET_FROM_ISR(xTimer, pxTimerCoRoutineWoken) \
        xTimerDgCRResetFromISR((xTimer), (pxTimerCoRoutineWoken))

/**
 * \brief Defer function execution from ISR to timer daemon co-routine
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function is used for deferring from interrupt service routine (ISR) the execution of a
 * specific function to the timer service/daemon co-routine. It addresses cases where an interrupt
 * service routine (ISR) needs to be kept as short as possible, and some of its involved
 * processing/functionality can be postponed in order to execute from the timer service/daemon
 * co-routine.
 *
 * Since the function execution command is sent through a queue to the timer service/daemon
 * co-routine, the function may fail to post the reset command to the queue.
 *
 * \param [in] xFunctionToPend function of PendedFunction_t type to execute from the timer
 * service/daemon co-routine
 * \param [in] pvParameter1 value of the first parameter of the function the execution of which
 * is deferred
 * \param [in] ulParameter2 value of the second parameter of the function the execution of which
 * is deferred
 * \param [in,out] pxTimerCoRoutineWoken indication whether timer service/daemon co-routine has
 * already been unblocked from the current ISR by posting a command to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * pdFAIL
 */
#define dgcrTIMER_PEND_FUNCTION_CALL_FROM_ISR(xFunctionToPend, pvParameter1, ulParameter2, \
                pxTimerCoRoutineWoken) \
        xTimerDgCRPendFunctionCallFromISR((xFunctionToPend), (pvParameter1), (ulParameter2), \
                pxTimerCoRoutineWoken)

/**
 * \brief Get handle associated with the timer service/daemon co-routine
 *
 * This function returns the handle of the timer service/daemon co-routine. It it not valid
 * to call xTimerGetTimerDaemonDgCoRoutineHandle() before the scheduler has been started.
 *
 * \return handle of the timer service/daemon co-routine
 */
CoRoutineHandle_t xTimerGetTimerDaemonDgCoRoutineHandle(void) PRIVILEGED_FUNCTION;

/**
 * \brief Start timer
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function starts a timer that was previously created (e.g. using xTimerCreate()). If the
 * timer had already been started and was already in the active state, the timer is restarted.
 * The callback function associated with the timer will be called 'n' ticks after xTimerDgCRStart()
 * was called, where 'n' is the timer's defined period.
 *
 * Since the start command is sent through a queue to the timer service/daemon co-routine,
 * the function will indicate whether the calling co-routine needs to block and wait for the
 * command to be posted to the queue.
 *
 * \param [in] xTimer handle of the timer being started/restarted
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the start command to be successfully posted to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * an error code as defined within ProjDefs.h
 *
 * \sa xTimerCreate
 */
#define xTimerDgCRStart(xTimer, xTicksToWait) \
        xTimerDgCRGenericCommand((xTimer), tmrCOMMAND_START, (xDgCoRoutineGetTickCount()), \
                NULL, (xTicksToWait))

/**
 * \brief Stop timer
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function stops an already started timer (e.g. using xTimerDgCRStart()).
 *
 * Since the stop command is sent through a queue to the timer service/daemon co-routine,
 * the function will indicate whether the calling co-routine needs to block and wait for the
 * command to be posted to the queue.
 *
 * \param [in] xTimer handle of the timer being stopped
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the stop command to be successfully posted to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * an error code as defined within ProjDefs.h
 *
 * \sa xTimerDgCRStart
 */
#define xTimerDgCRStop(xTimer, xTicksToWait) \
        xTimerDgCRGenericCommand((xTimer), tmrCOMMAND_STOP, 0U, \
                NULL, (xTicksToWait))

/**
 * \brief Change timer period
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function changes the period of a timer that was previously created (e.g. using
 * xTimerCreate()), and if the timer is not already active, it starts it. The callback function
 * associated with the timer will be called xNewPeriod ticks after the period change command is
 * processed by the timer service/daemon co-routine.
 *
 * Since the period change command is sent through a queue to the timer service/daemon co-routine,
 * the function will indicate whether the calling co-routine needs to block and wait for the
 * command to be posted to the queue.
 *
 * \param [in] xTimer handle of the timer of which the period is changed
 * \param [in] xNewPeriod new period in ticks for the timer
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the period change command to be successfully posted to the timer command
 * queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * an error code as defined within ProjDefs.h
 *
 * \sa xTimerCreate
 */
#define xTimerDgCRChangePeriod(xTimer, xNewPeriod, xTicksToWait) \
        xTimerDgCRGenericCommand((xTimer), tmrCOMMAND_CHANGE_PERIOD, (xNewPeriod), \
                NULL, (xTicksToWait))

/**
 * \brief Delete timer
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function deletes a timer that was previously created (e.g. using xTimerCreate()).
 *
 * Since the delete command is sent through a queue to the timer service/daemon co-routine,
 * the function will indicate whether the calling co-routine needs to block and wait for the
 * command to be posted to the queue.
 *
 * \param [in] xTimer handle of the timer being deleted
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the delete command to be successfully posted to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * an error code as defined within ProjDefs.h
 *
 * \sa xTimerCreate
 */
#define xTimerDgCRDelete(xTimer, xTicksToWait) \
        xTimerDgCRGenericCommand((xTimer), tmrCOMMAND_DELETE, 0U, \
                NULL, (xTicksToWait))

/**
 * \brief Reset timer
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function restarts a timer that was previously created (e.g. using xTimerCreate()). If the
 * timer is not already active, it starts it, otherwise it re-evaluates the time it will expire,
 * making the the callback function associated with the timer be called 'n' ticks after
 * xTimerDgCRReset() was called, where 'n' is the timer's defined period.
 *
 * Since the reset command is sent through a queue to the timer service/daemon co-routine,
 * the function will indicate whether the calling co-routine needs to block and wait for the
 * command to be posted to the queue.
 *
 * \param [in] xTimer handle of the timer being reset
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the reset command to be successfully posted to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * an error code as defined within ProjDefs.h
 *
 * \sa xTimerCreate
 */
#define xTimerDgCRReset(xTimer, xTicksToWait) \
        xTimerDgCRGenericCommand((xTimer), tmrCOMMAND_RESET, (xDgCoRoutineGetTickCount()), \
                NULL, (xTicksToWait))

/**
 * \brief Defer function execution to timer daemon co-routine
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function defers the execution of a specific function to the timer service/daemon co-routine,
 * the priority of which is set using configTIMER_DG_COROUTINE_PRIORITY in FreeRTOSConfig.h.
 *
 * Since the function execution command is sent through a queue to the timer service/daemon
 * co-routine, the function will indicate whether the calling co-routine needs to block and wait
 * for the command to be posted to the queue.
 *
 * \param [in] xFunctionToPend function of PendedFunction_t type to execute from the timer
 * service/daemon co-routine
 * \param [in] pvParameter1 value of the first parameter of the function the execution of which
 * is deferred
 * \param [in] ulParameter2 value of the second parameter of the function the execution of which
 * is deferred
 * \param [in] xTicksToWait maximum amount of time in ticks the calling co-routine should wait in
 * the Blocked state for the function execution command to be successfully posted to the timer
 * command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * an error code as defined within ProjDefs.h
 */
BaseType_t xTimerDgCRPendFunctionCall(PendedFunction_t xFunctionToPend, void *pvParameter1,
        uint32_t ulParameter2, TickType_t xTicksToWait) PRIVILEGED_FUNCTION;

/**
 * \brief Start timer from ISR
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function starts from interrupt service routine (ISR) a timer that was previously created
 * (e.g. using xTimerCreate()). If the timer had already been started and was already in the
 * active state, the timer is restarted. The callback function associated with the timer will be
 * called 'n' ticks after xTimerDgCRStartFromISR() was called, where 'n' is the timer's defined
 * period.
 *
 * Since the start command is sent through a queue to the timer service/daemon co-routine,
 * the function may fail to post the start command to the queue.
 *
 * \param [in] xTimer handle of the timer being started/restarted
 * \param [in,out] pxTimerCoRoutineWoken indication whether timer service/daemon co-routine has
 * already been unblocked from the current ISR by posting a command to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * pdFAIL
 *
 * \sa xTimerCreate
 */
#define xTimerDgCRStartFromISR(xTimer, pxTimerCoRoutineWoken) \
        xTimerDgCRGenericCommand((xTimer), tmrCOMMAND_START_FROM_ISR, (xDgCoRoutineGetTickCountFromISR()), \
                (pxTimerCoRoutineWoken), 0U)

/**
 * \brief Stop timer from ISR
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function stops from interrupt service routine (ISR) an already started timer
 * (e.g. using xTimerDgCRStart()).
 *
 * Since the stop command is sent through a queue to the timer service/daemon co-routine,
 * the function may fail to post the stop command to the queue.
 *
 * \param [in] xTimer handle of the timer being stopped
 * \param [in,out] pxTimerCoRoutineWoken indication whether timer service/daemon co-routine has
 * already been unblocked from the current ISR by posting a command to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * pdFAIL
 *
 * \sa xTimerDgCRStart
 */
#define xTimerDgCRStopFromISR(xTimer, pxTimerCoRoutineWoken) \
        xTimerDgCRGenericCommand((xTimer), tmrCOMMAND_STOP_FROM_ISR, 0U, \
                (pxTimerCoRoutineWoken), 0U)

/**
 * \brief Change timer period from ISR
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function changes from interrupt service routine (ISR) the period of a timer that was
 * previously created (e.g. using xTimerCreate()), and if the timer is not already active,
 * it starts it. The callback function associated with the timer will be called xNewPeriod ticks
 * after the period change command is processed by the timer service/daemon co-routine.
 *
 * Since the period change command is sent through a queue to the timer service/daemon co-routine,
 * the function may fail to post the period change command to the queue.
 *
 * \param [in] xTimer handle of the timer of which the period is changed
 * \param [in] xNewPeriod new period in ticks for the timer
 * \param [in,out] pxTimerCoRoutineWoken indication whether timer service/daemon co-routine has
 * already been unblocked from the current ISR by posting a command to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * pdFAIL
 *
 * \sa xTimerCreate
 */
#define xTimerDgCRChangePeriodFromISR(xTimer, xNewPeriod, pxTimerCoRoutineWoken) \
        xTimerDgCRGenericCommand((xTimer), tmrCOMMAND_CHANGE_PERIOD_FROM_ISR, (xNewPeriod), \
                (pxTimerCoRoutineWoken), 0U)

/**
 * \brief Reset timer from ISR
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function restarts from interrupt service routine (ISR) a timer that was previously created
 * (e.g. using xTimerCreate()). If the timer is not already active, it starts it, otherwise it
 * re-evaluates the time it will expire, making the the callback function associated with the timer
 * be called 'n' ticks after xTimerDgCRResetFromISR() was called, where 'n' is the timer's defined
 * period.
 *
 * Since the reset command is sent through a queue to the timer service/daemon co-routine,
 * the function may fail to post the reset command to the queue.
 *
 * \param [in] xTimer handle of the timer being reset
 * \param [in,out] pxTimerCoRoutineWoken indication whether timer service/daemon co-routine has
 * already been unblocked from the current ISR by posting a command to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * pdFAIL
 *
 * \sa xTimerCreate
 */
#define xTimerDgCRResetFromISR(xTimer, pxTimerCoRoutineWoken) \
        xTimerDgCRGenericCommand((xTimer), tmrCOMMAND_RESET_FROM_ISR, (xDgCoRoutineGetTickCountFromISR()), \
                (pxTimerCoRoutineWoken), 0U)

/**
 * \brief Defer function execution from ISR to timer daemon co-routine
 *
 * Timer functionality is provided by a timer service/daemon co-routine. The corresponding to
 * Dialog co-routines public timer API functions send commands to the timer service co-routine
 * through a queue, comprising the timer command queue. The length of the timer command queue
 * is set by the configTIMER_QUEUE_LENGTH configuration constant.
 *
 * This function defers from interrupt service routine (ISR) the execution of a specific function
 * to the timer service/daemon co-routine, the priority of which is set using
 * configTIMER_DG_COROUTINE_PRIORITY in FreeRTOSConfig.h.
 *
 * This function addresses cases where an interrupt service routine (ISR) needs to be kept as short
 * as possible, and some of its involved processing/functionality can be postponed in order to
 * execute from the timer service/daemon co-routine.
 *
 * Since the function execution command is sent through a queue to the timer service/daemon
 * co-routine, the function may fail to post the function execution command to the queue.
 *
 * \param [in] xFunctionToPend function of PendedFunction_t type to execute from the timer
 * service/daemon co-routine
 * \param [in] pvParameter1 value of the first parameter of the function the execution of which
 * is deferred
 * \param [in] ulParameter2 value of the second parameter of the function the execution of which
 * is deferred
 * \param [in,out] pxTimerCoRoutineWoken indication whether timer service/daemon co-routine has
 * already been unblocked from the current ISR by posting a command to the timer command queue
 *
 * \return pdPASS if the command was successfully posted to the timer command queue, otherwise
 * pdFAIL
 */
BaseType_t xTimerDgCRPendFunctionCallFromISR(PendedFunction_t xFunctionToPend, void *pvParameter1,
        uint32_t ulParameter2, BaseType_t *pxTimerCoRoutineWoken) PRIVILEGED_FUNCTION;


/*
 * Macros and functions beyond this part are not part of the public API and are intended for use
 * by the kernel only.
 */

/*
 * Create timer service/daemon co-routine.
 */
BaseType_t xTimerCreateTimerDgCoRoutine(void) PRIVILEGED_FUNCTION;

/*
 * Send command to timer service/daemon co-routine.
 */
#define xTimerDgCRGenericCommand(xTimer, xCommandID, xOptionalValue, pxTimerCoRoutineWoken, \
                xTicksToWait) \
        xTimerGenericCommand((xTimer), (xCommandID), (xOptionalValue), (pxTimerCoRoutineWoken), \
                (xTicksToWait))

#ifdef __cplusplus
}
#endif

#endif /* DIALOG_TIMERS_H_ */
