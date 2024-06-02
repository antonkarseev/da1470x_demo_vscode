/**
 * \addtogroup MID_RTO_OSAL
 * \{
 * \addtogroup MID_RTO_OSAL_FREERTOS
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file osal_freertos.h
 *
 * @brief OS abstraction layer API for FreeRTOS
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef OSAL_FREERTOS_H_
#define OSAL_FREERTOS_H_

#if defined(OS_FREERTOS)

#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <interrupts.h>
#include <atomic.h>

/*
 * OSAL CONFIGURATION FORWARD MACROS
 *****************************************************************************************
 */

/* Enable use of low power tickless mode */
#define _OS_USE_TICKLESS_IDLE           ( configUSE_TICKLESS_IDLE > 0 )
/* Total size of heap memory available for the OS */
#define _OS_TOTAL_HEAP_SIZE             ( configTOTAL_HEAP_SIZE )
/* Word size used for the items stored to the stack */
#define _OS_STACK_WORD_SIZE             ( sizeof(StackType_t) )
/* Minimal stack size (in bytes) defined for an OS task */
#define _OS_MINIMAL_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * _OS_STACK_WORD_SIZE )
/* Priority of timer daemon OS task */
#define _OS_DAEMON_TASK_PRIORITY        ( configTIMER_TASK_PRIORITY )

/*
 * OSAL DATA TYPE AND ENUMERATION FORWARD MACROS
 *****************************************************************************************
 */

/* OS task priority values */
#define _OS_TASK_PRIORITY_LOWEST        ( tskIDLE_PRIORITY )
#define _OS_TASK_PRIORITY_NORMAL        ( tskIDLE_PRIORITY + 1 )
#define _OS_TASK_PRIORITY_HIGHEST       ( configMAX_PRIORITIES - 1 )

/* Data types and enumerations for OS tasks and functions that operate on them */
#define _OS_TASK                        TaskHandle_t
#define _OS_TASK_STATUS                 TaskStatus_t
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
#define _OS_EVENT_GROUP                 EventGroupHandle_t
#define _OS_EVENT_GROUP_OK              pdTRUE
#define _OS_EVENT_GROUP_FAIL            pdFALSE
#define _OS_EVENT_GROUP_NO_WAIT         0
#define _OS_EVENT_GROUP_FOREVER         portMAX_DELAY

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
#define _OS_TASK_ARG_TYPE               void *

/* Data types and enumerations for OS Atomic operations  */
#define _OS_ATOMIC_COMPARE_AND_SWAP_SUCCESS     ATOMIC_COMPARE_AND_SWAP_SUCCESS
#define _OS_ATOMIC_COMPARE_AND_SWAP_FAILURE     ATOMIC_COMPARE_AND_SWAP_FAILURE

/* Data type about the output of _OS_GET_HEAP_STATISTICS() */
#define _OS_HEAP_STATISTICS_TYPE        HeapStats_t

/*
 * OSAL ENUMERATIONS
 *****************************************************************************************
 */

/* OS task notification action */
#define _OS_NOTIFY_NO_ACTION                    eNoAction
#define _OS_NOTIFY_SET_BITS                     eSetBits
#define _OS_NOTIFY_INCREMENT                    eIncrement
#define _OS_NOTIFY_VAL_WITH_OVERWRITE           eSetValueWithOverwrite
#define _OS_NOTIFY_VAL_WITHOUT_OVERWRITE        eSetValueWithoutOverwrite

/* OS task state */
#define _OS_TASK_RUNNING                        eRunning
#define _OS_TASK_READY                          eReady
#define _OS_TASK_BLOCKED                        eBlocked
#define _OS_TASK_SUSPENDED                      eSuspended
#define _OS_TASK_DELETED                        eDeleted

/* OS scheduler state */
#define _OS_SCHEDULER_RUNNING                   taskSCHEDULER_RUNNING
#define _OS_SCHEDULER_NOT_STARTED               taskSCHEDULER_NOT_STARTED
#define _OS_SCHEDULER_SUSPENDED                 taskSCHEDULER_SUSPENDED

/*
 * OSAL MACRO FUNCTION DEFINITIONS
 *****************************************************************************************
 */

/* Declare an OS task function */
#define _OS_TASK_FUNCTION(func, arg) void func(OS_TASK_ARG_TYPE arg)

/* Run the OS task scheduler */
#define _OS_TASK_SCHEDULER_RUN() vTaskStartScheduler()

/* Convert a time in milliseconds to a time in OS ticks */
#define _OS_TIME_TO_TICKS(time_in_ms) pdMS_TO_TICKS(time_in_ms)

/* Return current OS task handle */
#if (INCLUDE_xTaskGetCurrentTaskHandle == 1) || (configUSE_MUTEXES == 1)
#define _OS_GET_CURRENT_TASK() xTaskGetCurrentTaskHandle()
#else
#define _OS_GET_CURRENT_TASK()
#endif /* INCLUDE_xTaskGetCurrentTaskHandle || configUSE_MUTEXES */

/* Create OS task */
#define _OS_TASK_CREATE(name, task_func, arg, stack_size, priority, task) \
        xTaskCreate((task_func), (name), (((stack_size) - 1) / _OS_STACK_WORD_SIZE + 1), \
                    (arg), (priority), &(task))

/* Delete OS task */
#define _OS_TASK_DELETE(task) vTaskDelete(task)

/* Get the priority of an OS task */
#define _OS_TASK_PRIORITY_GET(task) \
        ({ \
                UBaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = uxTaskPriorityGet(task); \
                ret; \
        })

/* Get the priority of an OS task from ISR */
#define _OS_TASK_PRIORITY_GET_FROM_ISR(task) uxTaskPriorityGetFromISR(task)

/* Set the priority of an OS task */
#define _OS_TASK_PRIORITY_SET(task, prio) vTaskPrioritySet((task), (prio))

/* The running OS task yields control to the scheduler */
#define _OS_TASK_YIELD() \
        do { \
                _OS_ASSERT(!in_interrupt()); \
                portYIELD(); \
        } while (0)

/* The running OS task yields control to the scheduler from ISR */
#define _OS_TASK_YIELD_FROM_ISR() portYIELD_FROM_ISR(pdTRUE)

/* Send notification to OS task, updating its notification value */
#define _OS_TASK_NOTIFY(task, value, action) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xTaskNotify((task), (value), (action)); \
                ret; \
        })

/* Send notification to OS task, updating one notification index value */
#define _OS_TASK_NOTIFY_INDEXED(task, index, value, action) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                xTaskNotifyIndexed((task), (index), (value), (action)); \
                ret; \
        })

/* Send notification to OS task, updating its notification value and returning previous value */
#define _OS_TASK_NOTIFY_AND_QUERY(task, value, action, prev_value) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                xTaskNotifyAndQuery((task), (value), (action), (prev_value)); \
                ret; \
        })

/* Send notification to OS task, updating one notification index value and returning previous value */
#define _OS_TASK_NOTIFY_AND_QUERY_INDEXED(task, index, value, action, prev_value) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                xTaskNotifyAndQueryIndexed((task), (index), (value), (action), (prev_value)); \
                ret; \
        })

/* Send notification to OS task from ISR, updating its notification value */
#define _OS_TASK_NOTIFY_FROM_ISR(task, value, action) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xTaskNotifyFromISR((task), (value), (action), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Send notification to OS task from ISR, updating one notification index value */
#define _OS_TASK_NOTIFY_INDEXED_FROM_ISR(task, index, value, action) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xTaskNotifyIndexedFromISR((task), (index), (value), (action), &need_switch) \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Send notification to OS task from ISR, updating its notification value and returning
 * previous value */
#define _OS_TASK_NOTIFY_AND_QUERY_FROM_ISR(task, value, action, prev_value) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xTaskNotifyAndQueryFromISR((task), (value), (action), (prev_value), \
                        &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Send notification to OS task from ISR, updating one notification index value and returning
 * previous value */
#define _OS_TASK_NOTIFY_AND_QUERY_INDEXED_FROM_ISR(task, index, value, action, prev_value) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xTaskNotifyAndQueryIndexedFromISR((task), (index), (value), (action), \
                        (prev_value), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Send a notification event to OS task, incrementing its notification value */
#define _OS_TASK_NOTIFY_GIVE(task) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xTaskNotifyGive(task); \
                ret; \
        })

/* Send a notification event to OS task, incrementing a notification index value */
#define _OS_TASK_NOTIFY_GIVE_INDEXED(task, index) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xTaskNotifyGiveIndexed((task), (index)); \
                ret; \
        })

/* Send a notification event to OS task from ISR, incrementing its notification value */
#define _OS_TASK_NOTIFY_GIVE_FROM_ISR(task) \
        ({ \
                BaseType_t need_switch = pdFALSE; \
                vTaskNotifyGiveFromISR((task), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
        })

/* Send a notification event to OS task from ISR, incrementing a notification index value */
#define _OS_TASK_NOTIFY_GIVE_INDEXED_FROM_ISR(task, index) \
        ({ \
                BaseType_t need_switch = pdFALSE; \
                vTaskNotifyGiveIndexedFromISR((task), (index), &need_switch) \
                portEND_SWITCHING_ISR(need_switch); \
        })

/* Wait for the calling OS task to receive a notification event, clearing to zero or
 * decrementing task notification value on exit */
#define _OS_TASK_NOTIFY_TAKE(clear_on_exit, time_to_wait) \
        ulTaskNotifyTake((clear_on_exit), (time_to_wait))

/* Wait for the calling OS task to receive a notification event, clearing to zero or
 * decrementing task notification index value on exit */
#define _OS_TASK_NOTIFY_TAKE_INDEXED(index, clear_on_exit, time_to_wait) \
        ulTaskNotifyTakeIndexed((index), (clear_on_exit), (time_to_wait))

/* Clear the notification state of an aS task */
#define _OS_TASK_NOTIFY_STATE_CLEAR(task) xTaskNotifyStateClear(task)

/* Clear a notification index state of an OS task */
#define _OS_TASK_NOTIFY_STATE_CLEAR_INDEXED(task, index) \
        xTaskNotifyStateClearIndexed((task), (index))

/* Clear specific bits in the notification value of an OS task */
#define _OS_TASK_NOTIFY_VALUE_CLEAR(task, bits_to_clear) \
        ulTaskNotifyValueClear((task), (bits_to_clear))

/* Clear specific bits in one notification index value of an OS task */
#define _OS_TASK_NOTIFY_VALUE_CLEAR_INDEXED(task, index,bits_to_clear) \
        ulTaskNotifyValueClearIndexed((task), (index), (bits_to_clear))

/* Wait for the calling OS task to receive a notification, updating task notification value
 * on exit */
#define _OS_TASK_NOTIFY_WAIT(entry_bits, exit_bits, value, ticks_to_wait) \
        xTaskNotifyWait((entry_bits), (exit_bits), (value), (ticks_to_wait))

/* Wait for the calling OS task to receive a notification index, updating notification value
 * on exit */
#define _OS_TASK_NOTIFY_WAIT_INDEXED(index, entry_bits, exit_bits, value, ticks_to_wait) \
        xTaskNotifyWaitIndexed((index), (entry_bits), (exit_bits), (value), (ticks_to_wait))

/* Resume OS task */
#define _OS_TASK_RESUME(task) \
        do { \
                _OS_ASSERT(!in_interrupt()); \
                vTaskResume(task); \
        } while (0)

/* Resume OS task from ISR */
#define _OS_TASK_RESUME_FROM_ISR(task) xTaskResumeFromISR(task)

/* Suspend OS task */
#define _OS_TASK_SUSPEND(task) \
        do { \
                _OS_ASSERT(!in_interrupt()); \
                vTaskSuspend(task); \
        } while (0)

/* Create OS mutex */
#define _OS_MUTEX_CREATE(mutex) \
        ({ \
                (mutex) = xSemaphoreCreateRecursiveMutex(); \
                (mutex) != NULL ? OS_MUTEX_CREATE_SUCCESS : OS_MUTEX_CREATE_FAIL; \
        })

/* Delete OS mutex */
#define _OS_MUTEX_DELETE(mutex) vSemaphoreDelete(mutex)

/* Release OS mutex */
#define _OS_MUTEX_PUT(mutex) xSemaphoreGiveRecursive(mutex)

/* Acquire OS mutex */
#define _OS_MUTEX_GET(mutex, timeout) xSemaphoreTakeRecursive((mutex), (timeout))

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
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xSemaphoreGive(event); \
                ret; \
        })

/* Set OS event in signaled state from ISR */
#define _OS_EVENT_SIGNAL_FROM_ISR(event) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xSemaphoreGiveFromISR((event), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Set OS event in signaled state from ISR without requesting running OS task to yield */
#define _OS_EVENT_SIGNAL_FROM_ISR_NO_YIELD(event, need_yield) \
        xSemaphoreGiveFromISR((event), (need_yield))

/* Wait for OS event to be signaled */
#define _OS_EVENT_WAIT(event, timeout) xSemaphoreTake((event), (timeout))

/* Check if OS event is signaled and clear it */
#define _OS_EVENT_CHECK(event) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xSemaphoreTake((event), OS_EVENT_NO_WAIT); \
                ret; \
        })

/* Check from ISR if OS event is signaled and clear it */
#define _OS_EVENT_CHECK_FROM_ISR(event) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xSemaphoreTakeFromISR((event), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Check from ISR if OS event is signaled and clear it, without requesting running
 * OS task to yield */
#define _OS_EVENT_CHECK_FROM_ISR_NO_YIELD(event, need_yield) \
        xSemaphoreTakeFromISR((event), (need_yield))

/* Get OS event status */
#define _OS_EVENT_GET_STATUS(event) \
        ({ \
                _OS_ASSERT(!in_interrupt()); \
                ((uxSemaphoreGetCount(event) > 0) ? OS_EVENT_SIGNALED : OS_EVENT_NOT_SIGNALED); \
        })

/* Get OS event status from ISR */
#define _OS_EVENT_GET_STATUS_FROM_ISR(event) \
        ((xQueueIsQueueEmptyFromISR(event) != pdFALSE) ? OS_EVENT_NOT_SIGNALED : OS_EVENT_SIGNALED)

/* Create OS event group */
#define _OS_EVENT_GROUP_CREATE() xEventGroupCreate()

/* Wait for OS event group bits to become set */
#define _OS_EVENT_GROUP_WAIT_BITS(event_group, bits_to_wait, clear_on_exit, wait_for_all, timeout) \
        ({ \
                EventBits_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xEventGroupWaitBits((event_group), (bits_to_wait), (clear_on_exit), (wait_for_all), (timeout)); \
                ret; \
        })

/* Set OS event group bits */
#define _OS_EVENT_GROUP_SET_BITS(event_group, bits_to_set) \
        ({ \
                EventBits_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xEventGroupSetBits((event_group), (bits_to_set)); \
                ret; \
        })

/* Set OS event group bits from ISR */
#define _OS_EVENT_GROUP_SET_BITS_FROM_ISR(event_group, bits_to_set) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xEventGroupSetBitsFromISR((event_group), (bits_to_set), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Set OS event group bits from ISR without requesting running OS task to yield */
#define _OS_EVENT_GROUP_SET_BITS_FROM_ISR_NO_YIELD(event_group, bits_to_set, need_yield) \
        xEventGroupSetBitsFromISR((event_group), (bits_to_set), (need_yield))

/* Clear OS event group bits */
#define _OS_EVENT_GROUP_CLEAR_BITS(event_group, bits_to_clear) \
        ({ \
                EventBits_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xEventGroupClearBits((event_group), (bits_to_clear)); \
                ret; \
        })

/* Clear OS event group bits from an interrupt */
#define _OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR(event_group, bits_to_clear) \
        xEventGroupClearBitsFromISR((event_group), (bits_to_clear))

/* Get OS event group bits */
#define _OS_EVENT_GROUP_GET_BITS(event_group) \
        ({ \
                EventBits_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xEventGroupGetBits(event_group); \
                ret; \
        })

/* Get OS event group bits from an interrupt */
#define _OS_EVENT_GROUP_GET_BITS_FROM_ISR(event_group) xEventGroupGetBitsFromISR(event_group)

/* Synchronize OS event group bits */
#define _OS_EVENT_GROUP_SYNC(event_group, bits_to_set, bits_to_wait, timeout) \
        ({ \
                EventBits_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xEventGroupSync((event_group), (bits_to_set), (bits_to_wait), (timeout)); \
                ret; \
        })

/* Delete OS event group */
#define _OS_EVENT_GROUP_DELETE(event_group) vEventGroupDelete(event_group)

/* Create OS queue */
#define _OS_QUEUE_CREATE(queue, item_size, max_items) \
        do { (queue) = xQueueCreate((max_items), (item_size)); } while (0)

/* Deletes OS queue */
#define _OS_QUEUE_DELETE(queue) vQueueDelete(queue)

/* Put element in OS queue */
#define _OS_QUEUE_PUT(queue, item, timeout) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xQueueSendToBack((queue), (item), (timeout)); \
                ret; \
        })

/* Put element in OS queue */
#define _OS_QUEUE_PUT_FROM_ISR(queue, item) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xQueueSendToBackFromISR((queue), (item), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Replace element in OS queue of one element */
#define _OS_QUEUE_REPLACE(queue, item) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xQueueOverwrite((queue), (item)); \
                ret; \
        })

/* Replace element in OS queue of one element from ISR */
#define _OS_QUEUE_REPLACE_FROM_ISR(queue, item) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xQueueOverwriteFromISR((queue), (item), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Replace element in OS queue of one element from ISR without requesting running OS task to yield */
#define _OS_QUEUE_REPLACE_FROM_ISR_NO_YIELD(queue, item, need_yield) \
        xQueueOverwriteFromISR((queue), (item), (need_yield))

/* Get element from OS queue */
#define _OS_QUEUE_GET(queue, item, timeout) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xQueueReceive((queue), (item), (timeout)); \
                ret; \
        })

/* Get element from OS queue from ISR */
#define _OS_QUEUE_GET_FROM_ISR(queue, item) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xQueueReceiveFromISR((queue), (item), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Get element from OS queue from ISR without requesting running OS task to yield */
#define _OS_QUEUE_GET_FROM_ISR_NO_YIELD(queue, item, need_yield) \
        xQueueReceiveFromISR((queue), (item), (need_yield))

/* Peek element from OS queue */
#define _OS_QUEUE_PEEK(queue, item, timeout) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xQueuePeek((queue), (item), (timeout)); \
                ret; \
        })

/* Peek element from OS queue from ISR */
#define _OS_QUEUE_PEEK_FROM_ISR(queue, item) xQueuePeekFromISR((queue), (item))

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
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xTimerStart((timer), (timeout)); \
                ret; \
        })

/* Stop OS timer */
#define _OS_TIMER_STOP(timer, timeout) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xTimerStop((timer), (timeout)); \
                ret; \
        })

/* Change OS timer's period */
#define _OS_TIMER_CHANGE_PERIOD(timer, period, timeout) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xTimerChangePeriod((timer), (period), (timeout)); \
                ret; \
        })

/* Delete OS timer */
#define _OS_TIMER_DELETE(timer, timeout) xTimerDelete((timer), (timeout))

/* Reset OS timer */
#define _OS_TIMER_RESET(timer, timeout) \
        ({ \
                BaseType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xTimerReset((timer), (timeout)); \
                ret; \
        })

/* Start OS timer from ISR */
#define _OS_TIMER_START_FROM_ISR(timer) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xTimerStartFromISR((timer), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Stop OS timer from ISR */
#define _OS_TIMER_STOP_FROM_ISR(timer) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xTimerStopFromISR((timer), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Change OS timer period from ISR */
#define _OS_TIMER_CHANGE_PERIOD_FROM_ISR(timer, period) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xTimerChangePeriodFromISR((timer), (period), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Reset OS timer from ISR */
#define _OS_TIMER_RESET_FROM_ISR(timer) \
        ({ \
                BaseType_t need_switch = pdFALSE, ret; \
                ret = xTimerResetFromISR((timer), &need_switch); \
                portEND_SWITCHING_ISR(need_switch); \
                ret; \
        })

/* Set OS timer auto-reload mode */
#define _OS_TIMER_SET_RELOAD_MODE(timer, auto_reload) vTimerSetReloadMode((timer), (auto_reload))

/* Get OS timer auto-reload mode */
#define _OS_TIMER_GET_RELOAD_MODE(timer) uxTimerGetReloadMode(timer)

/* Delay execution of OS task for specified time */
#define _OS_DELAY(ticks) vTaskDelay(ticks)

/* Delay execution of OS task until specified time */
#define _OS_DELAY_UNTIL(ticks) \
        { \
                TickType_t prev_wake_time = xTaskGetTickCount(); \
                vTaskDelayUntil(&prev_wake_time, ticks - prev_wake_time); \
        }

/* Get current OS tick count */
#define _OS_GET_TICK_COUNT() \
        ({ \
                TickType_t ret; \
                _OS_ASSERT(!in_interrupt()); \
                ret = xTaskGetTickCount(); \
                ret; \
        })

/* Get current OS tick count from ISR */
#define _OS_GET_TICK_COUNT_FROM_ISR() xTaskGetTickCountFromISR()

/* Convert from OS ticks to ms */
#define _OS_TICKS_2_MS(ticks) portCONVERT_TICKS_2_MS(ticks)

/* Convert from ms to OS ticks */
#define _OS_MS_2_TICKS(ms) pdMS_TO_TICKS(ms)

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

/* Name for OS memory allocation function */
#define _OS_REALLOC_FUNC pvPortRealloc

/* Name for non-retain memory allocation function */
#define _OS_REALLOC_NORET_FUNC pvPortRealloc

/* Reallocate memory from OS provided heap */
#define _OS_REALLOC(addr, size) _OS_REALLOC_FUNC(addr, size)

/* Reallocate memory from non-retain heap */
#define _OS_REALLOC_NORET(addr, size) _OS_REALLOC_NORET_FUNC(addr, size)

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

/* OS precondition */
#define _OS_PRECONDITION(cond) configPRECONDITION(cond)

/* OS memory barrier */
#ifdef portMEMORY_BARRIER
#define _OS_MEMORY_BARRIER() portMEMORY_BARRIER()
#endif

/*OS software barrier */
#ifdef portSOFTWARE_BARRIER
#define _OS_SOFTWARE_BARRIER() portSOFTWARE_BARRIER()
#endif

/* Get OS task status */
#if (configUSE_TRACE_FACILITY == 1)
#define _OS_GET_TASKS_STATUS(task_status, status_size) \
        uxTaskGetSystemState((task_status), (status_size), NULL)
#else
#define _OS_GET_TASKS_STATUS(task_status, status_size)
#endif /* configUSE_TRACE_FACILITY */

/* Get the high water mark of the stack associated with an OS task */
#if (INCLUDE_uxTaskGetStackHighWaterMark == 1)
#define _OS_GET_TASK_STACK_WATERMARK(task) uxTaskGetStackHighWaterMark(task)
#else
#define _OS_GET_TASK_STACK_WATERMARK(task)
#endif /* INCLUDE_uxTaskGetStackHighWaterMark */

/* Get the high water mark of heap */
#define _OS_GET_HEAP_WATERMARK() xPortGetMinimumEverFreeHeapSize()

/* Get current free heap size */
#define _OS_GET_FREE_HEAP_SIZE() xPortGetFreeHeapSize()

/* Get current number of OS tasks */
#define _OS_GET_TASKS_NUMBER() uxTaskGetNumberOfTasks()

/* Get OS task name */
#define _OS_GET_TASK_NAME(task) pcTaskGetTaskName(task)

/* Get OS task state */
#if (INCLUDE_eTaskGetState == 1)
#define _OS_GET_TASK_STATE(task) eTaskGetState(task)
#else
#define _OS_GET_TASK_STATE(task)
#endif /* INCLUDE_eTaskGetState */

/* Get OS task priority */
#if (INCLUDE_uxTaskPriorityGet == 1)
#define _OS_GET_TASK_PRIORITY(task) uxTaskPriorityGet(task)
#else
#define _OS_GET_TASK_PRIORITY(task)
#endif /* INCLUDE_uxTaskPriorityGet */

/* Get OS task scheduler state */
#if ((INCLUDE_xTaskGetSchedulerState == 1) || (configUSE_TIMERS == 1))
#define _OS_GET_TASK_SCHEDULER_STATE() xTaskGetSchedulerState()
#endif /* INCLUDE_xTaskGetSchedulerState || configUSE_TIMERS */

/* Get OS task handle associated with the Idle OS task */
#if (INCLUDE_xTaskGetIdleTaskHandle == 1)
#define _OS_GET_IDLE_TASK_HANDLE() xTaskGetIdleTaskHandle()
#else
#define _OS_GET_IDLE_TASK_HANDLE()
#endif /* INCLUDE_xTaskGetIdleTaskHandle */

/* Get OS task handle by name */
#if (INCLUDE_xTaskGetHandle == 1)
#define _OS_GET_TASK_HANDLE(task_name) xTaskGetHandle(task_name)
#else
#define _OS_GET_TASK_HANDLE(task_name)
#endif /* INCLUDE_xTaskGetHandle */

/* Conditionally change contents of value_location with exchange_value */
#define _OS_ATOMIC_COMPARE_AND_SWAP_U32(value_location, exchange_value, swap_condition) \
        Atomic_CompareAndSwap_u32((value_location), (exchange_value), (swap_condition))

/* Set the address pointed to by destination_pointer to the value of *exchange_pointer */
#define _OS_ATOMIC_SWAP_POINTERS_P32(destination_pointer, exchange_pointer) \
        Atomic_SwapPointers_p32((destination_pointer), (exchange_pointer))

/* Conditionally set the address pointed to by destination_pointer to the value of *exchange_pointer */
#define _OS_ATOMIC_COMPARE_AND_SWAP_POINTERS_P32(destination_pointer, exchange_pointer, swap_condition) \
        Atomic_CompareAndSwapPointers_p32((destination_pointer), (exchange_pointer), (swap_condition))

/* Add add_value to value located at value_location */
#define _OS_ATOMIC_ADD_U32(value_location, add_value) \
        Atomic_Add_u32((value_location), (add_value))

/* Subtract subtract_value from value located at value_location */
#define _OS_ATOMIC_SUBTRACT_U32(value_location, subtract_value) \
        Atomic_Subtract_u32((value_location), (subtract_value))

/* Increment value located at value_location by 1*/
#define _OS_ATOMIC_INCREMENT_U32(value_location) \
        Atomic_Increment_u32(value_location)

/* Decrement value located at value_location by 1*/
#define _OS_ATOMIC_DECREMENT_U32(value_location) \
        Atomic_Decrement_u32(value_location)

/* Perform OR calculation on value at value_location with or_mask */
#define _OS_ATOMIC_OR_U32(value_location, or_mask) \
        Atomic_OR_u32((value_location), (or_mask))

/* Perform AND calculation on value at value_location with and_mask */
#define _OS_ATOMIC_AND_U32(value_location, and_mask) \
        Atomic_AND_u32((value_location), (and_mask))

/* Perform NAND calculation on value at value_location with nand_mask */
#define _OS_ATOMIC_NAND_U32(value_location, nand_mask) \
        Atomic_NAND_u32((value_location), (nand_mask))

/* Perform XOR calculation on value at value_location with xor_mask */
#define _OS_ATOMIC_XOR_U32(value_location, xor_mask) \
        Atomic_XOR_u32((value_location), (xor_mask))

/* Get information about the current heap state */
#define _OS_GET_HEAP_STATISTICS(results_pointer) vPortGetHeapStats(results_pointer)

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
#if (configUSE_DAEMON_TASK_STARTUP_HOOK == 1)
#define _OS_APP_DAEMON_TASK(...) void vApplicationDaemonTaskStartupHook(__VA_ARGS__)
#endif /* configUSE_DAEMON_TASK_STARTUP_HOOK */

/* *************************************************************** */
/* The following macro functions are used internally by the system */
/* *************************************************************** */

/* Advance OS tick count */
#define _OS_TICK_ADVANCE() xPortTickAdvance()

/* Update OS tick count by adding a given number of OS ticks */
#define _OS_TICK_INCREMENT(ticks) \
        { \
                xTaskIncrementTick(); \
                vTaskStepTick(ticks); \
        }

#endif /* OS_FREERTOS */

#endif /* OSAL_FREERTOS_H_ */

/**
 * \}
 * \}
 */
