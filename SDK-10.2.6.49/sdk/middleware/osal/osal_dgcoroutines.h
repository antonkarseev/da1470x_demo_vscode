/**
 * \addtogroup MID_RTO_OSAL
 * \{
 * \addtogroup MID_RTO_OSAL_DGCOROUTINES
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file osal_dgcoroutines.h
 *
 * @brief OS abstraction layer API for Dialog CoRoutines (based on FreeRTOS CoRoutines)
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef OSAL_DGCOROUTINES_H_
#define OSAL_DGCOROUTINES_H_

#if defined(OS_DGCOROUTINES)

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <croutine.h>
#include <queue.h>
#include <timers.h>
#include <interrupts.h>

#define OS_FEATURE_SINGLE_STACK

#define CONFIG_AD_SPI_LOCKING                   ( 0 )
#define CONFIG_AD_I2C_LOCKING                   ( 0 )
#define CONFIG_AD_I3C_LOCKING                   ( 0 )
#define CONFIG_AD_UART_LOCKING                  ( 0 )
#define CONFIG_AD_GPADC_LOCKING                 ( 0 )

#ifdef CONFIG_SPI_USE_SYNC_TRANSACTIONS
# if (CONFIG_SPI_USE_SYNC_TRANSACTIONS == 1)
#  error "SPI synchronous transactions API is not supported when Dialog CoRoutines is used."
# endif
#else
# define CONFIG_SPI_USE_SYNC_TRANSACTIONS       ( 0 )
#endif /* CONFIG_SPI_USE_SYNC_TRANSACTIONS */

#ifdef CONFIG_I2C_USE_SYNC_TRANSACTIONS
# if (CONFIG_I2C_USE_SYNC_TRANSACTIONS == 1)
#  error "I2C synchronous transactions API is not supported when Dialog CoRoutines is used."
# endif
#else
# define CONFIG_I2C_USE_SYNC_TRANSACTIONS       ( 0 )
#endif /* CONFIG_I2C_USE_SYNC_TRANSACTIONS */

#ifdef CONFIG_I3C_USE_SYNC_TRANSACTIONS
# if (CONFIG_I3C_USE_SYNC_TRANSACTIONS == 1)
#  error "I3C synchronous transactions API is not supported when Dialog CoRoutines is used."
# endif
#else
# define CONFIG_I3C_USE_SYNC_TRANSACTIONS       ( 0 )
#endif /* CONFIG_I3C_USE_SYNC_TRANSACTIONS */

#ifdef CONFIG_UART_USE_SYNC_TRANSACTIONS
# if (CONFIG_UART_USE_SYNC_TRANSACTIONS == 1)
#  error "UART synchronous transactions API is not supported when Dialog CoRoutines is used."
# endif
#else
# define CONFIG_UART_USE_SYNC_TRANSACTIONS      ( 0 )
#endif /* CONFIG_UART_USE_SYNC_TRANSACTIONS */

#ifdef CONFIG_GPADC_USE_SYNC_TRANSACTIONS
# if (CONFIG_GPADC_USE_SYNC_TRANSACTIONS == 1)
#  error "GPADC synchronous transactions API is not supported when Dialog CoRoutines is used."
# endif
#else
# define CONFIG_GPADC_USE_SYNC_TRANSACTIONS     ( 0 )
#endif /* CONFIG_GPADC_USE_SYNC_TRANSACTIONS */

/*
 * OSAL CONFIGURATION FORWARD MACROS
 *****************************************************************************************
 */

/* Enable use of low power tickless mode */
#define _OS_USE_TICKLESS_IDLE           ( configUSE_TICKLESS_IDLE > 0 )
/* Total size of heap memory available for the OS */
#define _OS_TOTAL_HEAP_SIZE             ( configTOTAL_HEAP_SIZE )
/* Word size used for the items stored to the stack */
#define _OS_STACK_WORD_SIZE             sizeof(StackType_t)
/* Minimal stack size (in bytes) defined for a task */
#define _OS_MINIMAL_TASK_STACK_SIZE     'Non-applicable'
/* Priority of timer daemon task */
#define _OS_DAEMON_TASK_PRIORITY        ( configTIMER_DG_COROUTINE_PRIORITY )

/*
 * OSAL DATA TYPE AND ENUMERATION FORWARD MACROS
 *****************************************************************************************
 */

/* OS task priority values */
#define _OS_TASK_PRIORITY_LOWEST        ( dgcrIDLE_PRIORITY )
#define _OS_TASK_PRIORITY_NORMAL        ( dgcrIDLE_PRIORITY + 1 )
#define _OS_TASK_PRIORITY_HIGHEST       ( configMAX_CO_ROUTINE_PRIORITIES - 1 )

/* Data types and enumerations for OS tasks and functions that operate on them */
#define _OS_TASK                        CoRoutineHandle_t
#define _OS_TASK_STATUS                 DgCoRoutineStatus_t
#define _OS_TASK_CREATE_SUCCESS         pdPASS
#define _OS_TASK_NOTIFY_SUCCESS         pdPASS
#define _OS_TASK_NOTIFY_FAIL            pdFALSE
#define _OS_TASK_NOTIFY_NO_WAIT         0
#define _OS_TASK_NOTIFY_FOREVER         portMAX_DELAY
#define _OS_TASK_NOTIFY_NONE            0
#define _OS_TASK_NOTIFY_ALL_BITS        0xFFFFFFFF

/* Data types and enumerations for OS mutexes and functions that operate on them */
#define _OS_MUTEX                       SemaphoreHandle_t
#define _OS_MUTEX_CREATE_SUCCESS        1
#define _OS_MUTEX_CREATE_FAIL           0
#define _OS_MUTEX_TAKEN                 pdTRUE
#define _OS_MUTEX_NOT_TAKEN             pdFALSE
#define _OS_MUTEX_NO_WAIT               0
#define _OS_MUTEX_FOREVER               portMAX_DELAY

/* Data types and enumerations for OS events and functions that operate on them */
#define _OS_EVENT                       SemaphoreHandle_t
#define _OS_EVENT_CREATE_SUCCESS        1
#define _OS_EVENT_CREATE_FAIL           0
#define _OS_EVENT_SIGNALED              pdTRUE
#define _OS_EVENT_NOT_SIGNALED          pdFALSE
#define _OS_EVENT_NO_WAIT               0
#define _OS_EVENT_FOREVER               portMAX_DELAY

/* Data types and enumerations for OS event groups and functions that operate on them */
#define _OS_EVENT_GROUP                 'Non-applicable'
#define _OS_EVENT_GROUP_OK              'Non-applicable'
#define _OS_EVENT_GROUP_FAIL            'Non-applicable'
#define _OS_EVENT_GROUP_NO_WAIT         'Non-applicable'
#define _OS_EVENT_GROUP_FOREVER         'Non-applicable'

/* Data types and enumerations for OS queues and functions that operate on them */
#define _OS_QUEUE                       QueueHandle_t
#define _OS_QUEUE_OK                    pdTRUE
#define _OS_QUEUE_FULL                  errQUEUE_FULL
#define _OS_QUEUE_EMPTY                 errQUEUE_EMPTY
#define _OS_QUEUE_NO_WAIT               0
#define _OS_QUEUE_FOREVER               portMAX_DELAY

/* Data types and enumerations for OS timers and functions that operate on them */
#define _OS_TIMER                       TimerHandle_t
#define _OS_TIMER_SUCCESS               pdPASS
#define _OS_TIMER_FAIL                  pdFAIL
#define _OS_TIMER_RELOAD                pdTRUE
#define _OS_TIMER_ONCE                  pdFALSE
#define _OS_TIMER_NO_WAIT               0
#define _OS_TIMER_FOREVER               portMAX_DELAY

/* Base data types matching underlying architecture */
#define _OS_BASE_TYPE                   BaseType_t
#define _OS_UBASE_TYPE                  UBaseType_t

/* Enumeration values indicating successful or not OS operation */
#define _OS_OK                          pdPASS
#define _OS_FAIL                        pdFAIL

/* Boolean enumeration values */
#define _OS_TRUE                        pdTRUE
#define _OS_FALSE                       pdFALSE

/* Maximum OS delay (in OS ticks) */
#define _OS_MAX_DELAY                   portMAX_DELAY

/* OS tick time (i.e. time expressed in OS ticks) data type */
#define _OS_TICK_TIME                   TickType_t

/* OS tick period (in cycles of source clock used for the OS timer) */
#define _OS_TICK_PERIOD                 TICK_PERIOD

/* OS tick period (in msec) */
#define _OS_TICK_PERIOD_MS              portTICK_PERIOD_MS

/* Frequency (in Hz) of the source clock used for the OS timer */
#define _OS_TICK_CLOCK_HZ               configSYSTICK_CLOCK_HZ

/* Data type of OS task function (i.e. OS_TASK_FUNCTION) argument */
#define _OS_TASK_ARG_TYPE               UBaseType_t

/*
 * OSAL ENUMERATIONS
 *****************************************************************************************
 */

/* OS task notification action */
#define _OS_NOTIFY_NO_ACTION                    eDgCrNoAction
#define _OS_NOTIFY_SET_BITS                     eDgCrSetBits
#define _OS_NOTIFY_INCREMENT                    eDgCrIncrement
#define _OS_NOTIFY_VAL_WITH_OVERWRITE           eDgCrSetValueWithOverwrite
#define _OS_NOTIFY_VAL_WITHOUT_OVERWRITE        eDgCrSetValueWithoutOverwrite

/* OS task state */
#define _OS_TASK_RUNNING                        eDgCrRunning
#define _OS_TASK_READY                          eDgCrReady
#define _OS_TASK_BLOCKED                        eDgCrBlocked
#define _OS_TASK_SUSPENDED                      eDgCrInvalid
#define _OS_TASK_DELETED                        eDgCrDeleted

/* OS scheduler state */
#define _OS_SCHEDULER_RUNNING                   dgcrSCHEDULER_RUNNING
#define _OS_SCHEDULER_NOT_STARTED               dgcrSCHEDULER_NOT_STARTED
#define _OS_SCHEDULER_SUSPENDED                 dgcrSCHEDULER_SUSPENDED

/*
 * OSAL MACRO FUNCTION DEFINITIONS
 *****************************************************************************************
 */

/* OS task function begin point of execution. */
#define _OS_TASK_BEGIN() crSTART(xHandle)

/* OS task function end point of execution. */
#define _OS_TASK_END() crEND()

/* Declare an OS task function */
#define _OS_TASK_FUNCTION(func, arg) void func(CoRoutineHandle_t xHandle, OS_TASK_ARG_TYPE arg)

/* Run the OS task scheduler */
#define _OS_TASK_SCHEDULER_RUN() vDgCoRoutineStartScheduler()

/* Convert a time in milliseconds to a time in OS ticks */
#define _OS_TIME_TO_TICKS(time_in_ms) pdMS_TO_TICKS(time_in_ms)

/* Return current OS task handle */
#if (INCLUDE_xDgCoRoutineGetCurrentCoRoutineHandle == 1) || (configUSE_MUTEXES == 1)
#define _OS_GET_CURRENT_TASK() xDgCoRoutineGetCurrentCoRoutineHandle()
#else
#define _OS_GET_CURRENT_TASK()
#endif /* INCLUDE_xDgCoRoutineGetCurrentCoRoutineHandle || configUSE_MUTEXES */

/* Create OS task */
#define _OS_TASK_CREATE(name, task_func, arg, stack_size, priority, task) \
        xDgCoRoutineCreate((task_func), (name), (priority), (arg), &(task))

/* Delete OS task */
#define _OS_TASK_DELETE(task) vDgCoRoutineDelete(task)

/* Get the priority of an OS task */
#define _OS_TASK_PRIORITY_GET(task) \
        ({ \
                UBaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = uxDgCoRoutinePriorityGet(task); \
                ret; \
        })

/* Get the priority of an OS task from ISR */
#define _OS_TASK_PRIORITY_GET_FROM_ISR(task) uxDgCoRoutinePriorityGetFromISR(task)

/* Set the priority of an OS task */
#define _OS_TASK_PRIORITY_SET(task, prio)                       'Non-applicable'

/* The running OS task yields control to the scheduler */
#define _OS_TASK_YIELD() \
        do { \
                _OS_ASSERT(!in_interrupt()); \
                dgcrYIELD(xHandle); \
        } while (0)

/* The running OS task yields control to the scheduler from ISR */
#define _OS_TASK_YIELD_FROM_ISR() dgcrYIELD_FROM_ISR()

/* Send notification to OS task, updating its notification value */
#define _OS_TASK_NOTIFY(task, value, action) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xDgCoRoutineNotify((task), (value), (action)); \
                ret; \
        })

/* Send notification to OS task, updating its notification value and returning previous value */
#define _OS_TASK_NOTIFY_AND_QUERY(task, value, action, prev_value) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xDgCoRoutineNotifyAndQuery((task), (value), (action), (prev_value)); \
                ret; \
        })

/* Send notification to OS task from ISR, updating its notification value */
#define _OS_TASK_NOTIFY_FROM_ISR(task, value, action) \
        dgcrCOROUTINE_NOTIFY_FROM_ISR((task), (value), (action))

/* Send notification to OS task from ISR, updating its notification value and returning
 * previous value */
#define _OS_TASK_NOTIFY_AND_QUERY_FROM_ISR(task, value, action, prev_value) \
        dgcrCOROUTINE_NOTIFY_AND_QUERY_FROM_ISR((task), (value), (action), (prev_value))

/* Send a notification event to OS task, incrementing its notification value */
#define _OS_TASK_NOTIFY_GIVE(task) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrCOROUTINE_NOTIFY_GIVE(xHandle, (task), &ret); \
        } while (0)

/* Send a notification event to OS task from ISR, incrementing its notification value */
#define _OS_TASK_NOTIFY_GIVE_FROM_ISR(task) \
        dgcrCOROUTINE_NOTIFY_GIVE_FROM_ISR((task))

/* Wait for the calling OS task to receive a notification event, clearing to zero or
 * decrementing task notification value on exit */
#define _OS_TASK_NOTIFY_TAKE(clear_on_exit, time_to_wait) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrCOROUTINE_NOTIFY_TAKE(xHandle, (clear_on_exit), NULL, (time_to_wait), &ret); \
        } while (0)

/* Wait for the calling OS task to receive a notification, updating task notification value
 * on exit */
#define _OS_TASK_NOTIFY_WAIT(entry_bits, exit_bits, value, ticks_to_wait) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                dgcrCOROUTINE_NOTIFY_WAIT(xHandle, (entry_bits), (exit_bits), (value), \
                        (ticks_to_wait), &ret); \
        } while (0)

/* Resume OS task */
#define _OS_TASK_RESUME(task)                                   'Non-applicable'

/* Resume OS task from ISR */
#define _OS_TASK_RESUME_FROM_ISR(task)                          'Non-applicable'

/* Suspend OS task */
#define _OS_TASK_SUSPEND(task)                                  'Non-applicable'

/* Create OS mutex */
#define _OS_MUTEX_CREATE(mutex) \
        ({ \
                (mutex) = xSemaphoreCreateRecursiveMutex(); \
                (mutex) != NULL ? OS_MUTEX_CREATE_SUCCESS : OS_MUTEX_CREATE_FAIL; \
        })

/* Delete OS mutex */
#define _OS_MUTEX_DELETE(mutex) vSemaphoreDelete(mutex)

/* Release OS mutex */
#define _OS_MUTEX_PUT(mutex) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrSEMAPHORE_GIVE_RECURSIVE(xHandle, (mutex), &ret); \
        } while (0)

/* Acquire OS mutex */
#define _OS_MUTEX_GET(mutex, timeout) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrSEMAPHORE_TAKE_RECURSIVE(xHandle, (mutex), (timeout), &ret); \
        } while (0)

/* Get OS task owner of OS mutex */
#define _OS_MUTEX_GET_OWNER(mutex) \
        ({ \
                TaskHandle_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xQueueGetMutexHolder(mutex); \
                ret; \
        })

/* Get OS task owner of OS mutex from ISR */
#define _OS_MUTEX_GET_OWNER_FROM_ISR(mutex) xQueueGetMutexHolderFromISR(mutex)

/* Get OS mutex current count value */
#define _OS_MUTEX_GET_COUNT(mutex) \
        ({ \
                UBaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = uxSemaphoreGetCount(mutex); \
                ret; \
        })

/* Get OS mutex current count value from ISR */
#define _OS_MUTEX_GET_COUNT_FROM_ISR(mutex) uxQueueMessagesWaitingFromISR(mutex)

/* Create OS event */
#define _OS_EVENT_CREATE(event) do { (event) = xSemaphoreCreateBinary(); } while (0)

/* Delete OS event */
#define _OS_EVENT_DELETE(event) vSemaphoreDelete(event)

/* Set OS event in signaled state */
#define _OS_EVENT_SIGNAL(event) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrSEMAPHORE_GIVE(xHandle, (event), &ret); \
        } while (0)

/* Set OS event in signaled state from ISR */
#define _OS_EVENT_SIGNAL_FROM_ISR(event) \
        dgcrSEMAPHORE_GIVE_FROM_ISR((event), NULL)

/* Set OS event in signaled state from ISR without requesting running OS task to yield */
#define _OS_EVENT_SIGNAL_FROM_ISR_NO_YIELD(event, need_yield) \
        dgcrSEMAPHORE_GIVE_FROM_ISR((event), (need_yield))

/* Wait for OS event to be signaled */
#define _OS_EVENT_WAIT(event, timeout) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrSEMAPHORE_TAKE(xHandle, (event), (timeout), &ret); \
        } while (0)

/* Check if OS event is signaled and clear it */
#define _OS_EVENT_CHECK(event) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xSemaphoreDgCRTake((event), OS_EVENT_NO_WAIT); \
                ret; \
        })

/* Check from ISR if OS event is signaled and clear it */
#define _OS_EVENT_CHECK_FROM_ISR(event) \
        dgcrSEMAPHORE_TAKE_FROM_ISR((event), NULL)

/* Check from ISR if OS event is signaled and clear it, without requesting running
 * OS task to yield */
#define _OS_EVENT_CHECK_FROM_ISR_NO_YIELD(event, need_yield) \
        dgcrSEMAPHORE_TAKE_FROM_ISR((event), (need_yield))

/* Get OS event status */
#define _OS_EVENT_GET_STATUS(event) \
        ({ \
                UBaseType_t ret ; \
                _OS_ASSERT(!in_interrupt()); \
                ret = ((uxSemaphoreGetCount(event) > 0) ? OS_EVENT_SIGNALED : OS_EVENT_NOT_SIGNALED); \
                ret; \
        })

/* Get OS event status from ISR */
#define _OS_EVENT_GET_STATUS_FROM_ISR(event) \
        ((xQueueIsQueueEmptyFromISR(event) != pdFALSE) ? OS_EVENT_NOT_SIGNALED : OS_EVENT_SIGNALED)

/* Create OS event group */
#define _OS_EVENT_GROUP_CREATE()                                'Non-applicable'

/* Wait for OS event group bits to become set */
#define _OS_EVENT_GROUP_WAIT_BITS(event_group, bits_to_wait, clear_on_exit, wait_for_all, timeout) \
                                                                'Non-applicable'

/* Set OS event group bits */
#define _OS_EVENT_GROUP_SET_BITS(event_group, bits_to_set)      'Non-applicable'

/* Set OS event group bits from ISR */
#define _OS_EVENT_GROUP_SET_BITS_FROM_ISR(event_group, bits_to_set) 'Non-applicable'

/* Set OS event group bits from ISR without requesting running OS task to yield */
#define _OS_EVENT_GROUP_SET_BITS_FROM_ISR_NO_YIELD(event_group, bits_to_set, need_yield) \
                                                                'Non-applicable'

/* Clear OS event group bits */
#define _OS_EVENT_GROUP_CLEAR_BITS(event_group, bits_to_clear)  'Non-applicable'

/* Clear OS event group bits from an interrupt */
#define _OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR(event_group, bits_to_clear) 'Non-applicable'

/* Get OS event group bits */
#define _OS_EVENT_GROUP_GET_BITS(event_group)                   'Non-applicable'

/* Get OS event group bits from an interrupt */
#define _OS_EVENT_GROUP_GET_BITS_FROM_ISR(event_group)          'Non-applicable'

/* Synchronize OS event group bits */
#define _OS_EVENT_GROUP_SYNC(event_group, bits_to_set, bits_to_wait, timeout) 'Non-applicable'

/* Delete OS event group */
#define _OS_EVENT_GROUP_DELETE(event_group)                     'Non-applicable'

/* Create OS queue */
#define _OS_QUEUE_CREATE(queue, item_size, max_items) \
        do { (queue) = xQueueCreate((max_items), (item_size)); } while (0)

/* Deletes OS queue */
#define _OS_QUEUE_DELETE(queue) vQueueDelete(queue)

/* Put element in OS queue */
#define _OS_QUEUE_PUT(queue, item, timeout) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrQUEUE_SEND(xHandle, (queue), (item), (timeout), &ret); \
        } while (0)

/* Put element in OS queue */
#define _OS_QUEUE_PUT_FROM_ISR(queue, item) \
        dgcrQUEUE_SEND_FROM_ISR((queue), (item), NULL)

/* Replace element in OS queue of one element */
#define _OS_QUEUE_REPLACE(queue, item) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrQUEUE_OVERWRITE(xHanlde, (queue), (item), &ret); \
        } while (0)

/* Replace element in OS queue of one element from ISR */
#define _OS_QUEUE_REPLACE_FROM_ISR(queue, item) \
        dgcrQUEUE_OVERWRITE_FROM_ISR((queue), (item), NULL)

/* Replace element in OS queue of one element from ISR without requesting running OS task to yield */
#define _OS_QUEUE_REPLACE_FROM_ISR_NO_YIELD(queue, item, need_yield) \
        dgcrQUEUE_OVERWRITE_FROM_ISR((queue), (item), (need_yield))

/* Get element from OS queue */
#define _OS_QUEUE_GET(queue, item, timeout) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrQUEUE_RECEIVE(xHandle, (queue), (item), (timeout), &ret); \
        } while (0)

/* Get element from OS queue from ISR */
#define _OS_QUEUE_GET_FROM_ISR(queue, item) \
        dgcrQUEUE_RECEIVE_FROM_ISR((queue), (item), NULL)

/* Get element from OS queue from ISR without requesting running OS task to yield */
#define _OS_QUEUE_GET_FROM_ISR_NO_YIELD(queue, item, need_yield) \
        dgcrQUEUE_RECEIVE_FROM_ISR((queue), (item), (need_yield))

/* Peek element from OS queue */
#define _OS_QUEUE_PEEK(queue, item, timeout) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrQUEUE_PEEK(xHandle, (queue), (item), (timeout), &ret); \
        } while (0)

/* Peek element from OS queue from ISR */
#define _OS_QUEUE_PEEK_FROM_ISR(queue, item) \
        dgcrQUEUE_PEEK_FROM_ISR((queue), (item))

/* Get the number of messages stored in OS queue */
#define _OS_QUEUE_MESSAGES_WAITING(queue) \
        ({ \
                UBaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = uxQueueMessagesWaiting(queue); \
                ret; \
        })

/* Get the number of messages stored in OS queue from ISR */
#define _OS_QUEUE_MESSAGES_WAITING_FROM_ISR(queue) uxQueueMessagesWaitingFromISR(queue)

/* Get the number of free spaces in OS queue */
#define _OS_QUEUE_SPACES_AVAILABLE(queue) uxQueueSpacesAvailable(queue)

/* Create OS timer */
#define _OS_TIMER_CREATE(name, period, reload, timer_id, callback) \
        xTimerCreate((name), (period), (((reload) == OS_TIMER_ONCE) ? pdFALSE : pdTRUE), \
                     ((void *) (timer_id)), (callback))

/* Get OS timer ID */
#define _OS_TIMER_GET_TIMER_ID(timer) pvTimerGetTimerID(timer)

/* Check if OS timer is active */
#define _OS_TIMER_IS_ACTIVE(timer) xTimerIsTimerActive(timer)

/* Start OS timer */
#define _OS_TIMER_START(timer, timeout) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrTIMER_START(xHandle, (timer), (timeout), &ret); \
        } while (0)

/* Start OS timer without waiting */
#define _OS_TIMER_START_NO_WAIT(timer) \
        ( (xTimerDgCRStart((timer), 0) == pdFAIL) ? OS_TIMER_FAIL : OS_TIMER_SUCCESS )

/* Stop OS timer */
#define _OS_TIMER_STOP(timer, timeout) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrTIMER_STOP(xHandle, (timer), (timeout), &ret); \
        } while (0)

/* Change OS timer's period */
#define _OS_TIMER_CHANGE_PERIOD(timer, period, timeout) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrTIMER_CHANGE_PERIOD(xHandle, (timer), (period), (timeout), &ret); \
        } while (0)

/* Delete OS timer */
#define _OS_TIMER_DELETE(timer, timeout) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrTIMER_DELETE(xHandle, (timer), (timeout), &ret); \
        } while (0)

/* Reset OS timer */
#define _OS_TIMER_RESET(timer, timeout) \
        do { \
                OS_BASE_TYPE ret __UNUSED; \
                _OS_ASSERT(!in_interrupt()); \
                dgcrTIMER_RESET(xHandle, (timer), (timeout), &ret); \
        } while (0)

/* Start OS timer from ISR */
#define _OS_TIMER_START_FROM_ISR(timer) \
        dgcrTIMER_START_FROM_ISR((timer), NULL)

/* Stop OS timer from ISR */
#define _OS_TIMER_STOP_FROM_ISR(timer) \
        dgcrTIMER_STOP_FROM_ISR((timer), NULL)

/* Change OS timer period from ISR */
#define _OS_TIMER_CHANGE_PERIOD_FROM_ISR(timer, period) \
        dgcrTIMER_CHANGE_PERIOD_FROM_ISR((timer), (period), NULL)

/* Reset OS timer from ISR */
#define _OS_TIMER_RESET_FROM_ISR(timer) \
        dgcrTIMER_RESET_FROM_ISR((timer), NULL)

/* Delay execution of OS task for specified time */
#define _OS_DELAY(ticks) \
        do { \
                _OS_ASSERT(!in_interrupt()); \
                dgcrDELAY(xHandle, (ticks)); \
        } while (0)

/* Delay execution of OS task until specified time */
#define _OS_DELAY_UNTIL(ticks) \
        do { \
                TickType_t prev_wake_time = xDgCoRoutineGetTickCount(); \
                _OS_ASSERT(!in_interrupt()); \
                dgcrDELAY_UNTIL(xHandle, &prev_wake_time, ticks - prev_wake_time); \
        } while (0)

/* Get current OS tick count */
#define _OS_GET_TICK_COUNT() \
        ({ \
                TickType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xDgCoRoutineGetTickCount(); \
                ret; \
        })

/* Get current OS tick count from ISR */
#define _OS_GET_TICK_COUNT_FROM_ISR() xDgCoRoutineGetTickCountFromISR()

/* Convert from OS ticks to ms */
#define _OS_TICKS_2_MS(ticks) portCONVERT_TICKS_2_MS(ticks)

/* Convert from ms to OS ticks */
#define _OS_MS_2_TICKS(ms) portCONVERT_MS_2_TICKS(ms)

/* Delay execution of OS task for specified time */
#define _OS_DELAY_MS(ms) _OS_DELAY(_OS_MS_2_TICKS(ms))

/* Enter critical section from non-ISR context */
#define _OS_ENTER_CRITICAL_SECTION() \
        do { \
                _OS_ASSERT(!in_interrupt()); \
                portENTER_CRITICAL(); \
        } while (0)

/* Enter critical section from ISR context */
#define _OS_ENTER_CRITICAL_SECTION_FROM_ISR(critical_section_status) \
        do { \
                critical_section_status = portSET_INTERRUPT_MASK_FROM_ISR(); \
        } while (0)

/* Leave critical section from non-ISR context */
#define _OS_LEAVE_CRITICAL_SECTION() \
        do { \
                _OS_ASSERT(!in_interrupt()); \
                portEXIT_CRITICAL(); \
        } while (0)

/* Leave critical section from ISR context */
#define _OS_LEAVE_CRITICAL_SECTION_FROM_ISR(critical_section_status) \
        do { \
                portCLEAR_INTERRUPT_MASK_FROM_ISR(critical_section_status); \
        } while (0)


/* Name for OS memory allocation function */
#define _OS_MALLOC_FUNC pvPortMalloc

/* Name for non-retain memory allocation function */
#define _OS_MALLOC_NORET_FUNC pvPortMalloc

/* Allocate memory from OS provided heap */
#define _OS_MALLOC(size) _OS_MALLOC_FUNC(size)

/* Allocate memory from non-retain heap */
#define _OS_MALLOC_NORET(size) _OS_MALLOC_NORET_FUNC(size)

/* Name for OS free memory function */
#define _OS_FREE_FUNC vPortFree

/* Name for non-retain memory free function */
#define _OS_FREE_NORET_FUNC vPortFree

/* Free memory allocated by OS_MALLOC() */
#define _OS_FREE(addr) _OS_FREE_FUNC(addr)

/* Free memory allocated by OS_MALLOC_NORET() */
#define _OS_FREE_NORET(addr) _OS_FREE_NORET_FUNC(addr)

/* OS assertion */
#if (configASSERT_DEFINED == 1)
#define _OS_ASSERT(cond) configASSERT(cond)
#endif /* configASSERT_DEFINED */

/* Get OS task status */
#if (configUSE_TRACE_FACILITY == 1)
#define _OS_GET_TASKS_STATUS(task_status, status_size) \
        uxDgCoRoutineGetSystemState((task_status), (status_size), NULL)
#else
#define _OS_GET_TASKS_STATUS(task_status, status_size)
#endif /* configUSE_TRACE_FACILITY */

/* Get the high water mark of the stack associated with an OS task */
#if (INCLUDE_uxDgCoRoutineGetStackHighWaterMark == 1)
#define _OS_GET_TASK_STACK_WATERMARK(task) uxDgCoRoutineGetStackHighWaterMark(task)
#else
#define _OS_GET_TASK_STACK_WATERMARK(task)
#endif /* INCLUDE_uxDgCoRoutineGetStackHighWaterMark */

/* Get the high water mark of heap */
#define _OS_GET_HEAP_WATERMARK() xPortGetMinimumEverFreeHeapSize()

/* Get current free heap size */
#define _OS_GET_FREE_HEAP_SIZE() xPortGetFreeHeapSize()

/* Get current number of OS tasks */
#define _OS_GET_TASKS_NUMBER() uxDgCoRoutineGetNumberOfCoRoutines()

/* Get OS task name */
#if (INCLUDE_pcDgCoRoutineGetName == 1) && (configMAX_DG_COROUTINE_NAME_LEN > 0)
#define _OS_GET_TASK_NAME(task) pcDgCoRoutineGetName(task)
#else
#define _OS_GET_TASK_NAME(task)
#endif /* INCLUDE_pcDgCoRoutineGetName && configMAX_DG_COROUTINE_NAME_LEN */

/* Get OS task state */
#if (INCLUDE_eDgCoRoutineGetState == 1)
#define _OS_GET_TASK_STATE(task) eDgCoRoutineGetState(task)
#else
#define _OS_GET_TASK_STATE(task)
#endif /* INCLUDE_eDgCoRoutineGetState */

/* Get OS task priority */
#if (INCLUDE_uxDgCoRoutinePriorityGet == 1)
#define _OS_GET_TASK_PRIORITY(task) uxDgCoRoutinePriorityGet(task)
#else
#define _OS_GET_TASK_PRIORITY(task)
#endif /* INCLUDE_uxDgCoRoutinePriorityGet */

/* Get OS task scheduler state */
#if ((INCLUDE_xDgCoRoutineGetSchedulerState == 1) || (configUSE_TIMERS == 1))
#define _OS_GET_TASK_SCHEDULER_STATE() xDgCoRoutineGetSchedulerState()
#endif /* INCLUDE_xDgCoRoutineGetSchedulerState || configUSE_TIMERS */

/* Get OS task handle associated with the Idle OS task */
#if (INCLUDE_xDgCoRoutineGetIdleCoRoutineHandle == 1)
#define _OS_GET_IDLE_TASK_HANDLE() xDgCoRoutineGetIdleCoRoutineHandle()
#else
#define _OS_GET_IDLE_TASK_HANDLE()
#endif /* INCLUDE_xDgCoRoutineGetIdleCoRoutineHandle */

/* Get OS task handle by name */
#if (INCLUDE_xDgCoRoutineGetHandle == 1) && (configMAX_DG_COROUTINE_NAME_LEN > 0)
#define _OS_GET_TASK_HANDLE(task_name) xDgCoRoutineGetHandle(task_name)
#else
#define _OS_GET_TASK_HANDLE(task_name)
#endif /* INCLUDE_xDgCoRoutineGetHandle && configMAX_DG_COROUTINE_NAME_LEN */

/* ****************************************************** */
/* The following macro functions are called by the system */
/* ****************************************************** */

/* Perform processing prior to system stopping (e.g. entering hibernation) */
#define _OS_SYS_PRE_STOP_PROCESSING() configPRE_STOP_PROCESSING()

/* Perform processing prior to system entering sleep */
#define _OS_SYS_PRE_SLEEP_PROCESSING(sleep_period) configPRE_SLEEP_PROCESSING(sleep_period)

/* Perform processing after system waking-up */
#define _OS_SYS_POST_SLEEP_PROCESSING() configPOST_SLEEP_PROCESSING()

/* Perform processing prior to system entering idle state */
#define _OS_SYS_PRE_IDLE_PROCESSING(sleep_period) configPRE_IDLE_ENTRY(sleep_period)

/* Perform processing after system exiting idle state */
#define _OS_SYS_POST_IDLE_PROCESSING(sleep_period) configPOST_IDLE_ENTRY(sleep_period)

/* Hook function to handle memory allocation failures */
#if (configUSE_MALLOC_FAILED_HOOK == 1)
#define _OS_APP_MALLOC_FAILED(...) void vApplicationMallocFailedHook(__VA_ARGS__)
#endif /* configUSE_MALLOC_FAILED_HOOK */

/* Hook function to be called on each iteration of the idle OS task */
#if (configUSE_IDLE_HOOK == 1)
#define _OS_APP_IDLE(...) void vApplicationIdleHook(__VA_ARGS__)
#endif /* configUSE_IDLE_HOOK */

/* Hook function to be called upon stack overflow */
#if (configCHECK_FOR_STACK_OVERFLOW > 0)
#define _OS_APP_STACK_OVERFLOW(...) void vApplicationStackOverflowHook(__VA_ARGS__)
#endif /* configCHECK_FOR_STACK_OVERFLOW */

/* Hook function to be called on every OS tick */
#if (configUSE_TICK_HOOK == 1)
#define _OS_APP_TICK(...) void vApplicationTickHook(__VA_ARGS__)
#endif /* configUSE_TICK_HOOK */

/* Hook function to be called at the point the daemon OS task starts executing */
#if (configUSE_DAEMON_DG_COROUTINE_STARTUP_HOOK == 1)
#define _OS_APP_DAEMON_TASK(...) void vApplicationDaemonDgCoRoutineStartupHook(__VA_ARGS__)
#endif /* configUSE_DAEMON_DG_COROUTINE_STARTUP_HOOK */

/* *************************************************************** */
/* The following macro functions are used internally by the system */
/* *************************************************************** */

/* Advance OS tick count */
#define _OS_TICK_ADVANCE() xPortTickAdvance()

/* Update OS tick count by adding a given number of OS ticks */
#define _OS_TICK_INCREMENT(ticks) \
        do { \
                xDgCoRoutineIncrementTick(); \
                vDgCoRoutineStepTick(ticks); \
        } while (0)

#endif /* OS_DGCOROUTINES */

#endif /* OSAL_DGCOROUTINES_H_ */

/**
 * \}
 * \}
 */
