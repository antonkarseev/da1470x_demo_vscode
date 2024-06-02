/**
 * \addtogroup MID_RTOS
 * \{
 * \addtogroup MID_RTO_OSAL
 *
 * \brief OS Abstraction Layer
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file osal.h
 *
 * @brief OS abstraction layer API
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef OSAL_H_
#define OSAL_H_

#ifdef OS_PRESENT

#if defined(OS_FREERTOS)
#include "osal_freertos.h"
#elif defined(OS_DGCOROUTINES)
#include "osal_dgcoroutines.h"
#else
#error "No Operating System is defined."
#endif /* OS defined */

/*
 * OSAL CONFIGURATION FORWARD MACROS
 *****************************************************************************************
 */

/**
 * \brief Enable use of low power tickless mode
 *
 * If this macro is set to a non-zero value, then the use of OS low power tickless mode is enabled,
 * otherwise the OS tick interrupt is kept running at all times.
 */
#define OS_USE_TICKLESS_IDLE            _OS_USE_TICKLESS_IDLE

/**
 * \brief Total size of heap memory available for the OS
 */
#define OS_TOTAL_HEAP_SIZE              _OS_TOTAL_HEAP_SIZE

/**
 * \brief Word size used for the items stored to the stack
 */
#define OS_STACK_WORD_SIZE              _OS_STACK_WORD_SIZE

/**
 * \brief Minimal stack size (in bytes) defined for an OS task
 *
 * This macro defines the minimal stack size that can be defined for an OS task.
 * The value must be such, so that idle OS task and timer daemon OS task can run.
 */
#define OS_MINIMAL_TASK_STACK_SIZE      _OS_MINIMAL_TASK_STACK_SIZE

/**
 * \brief Priority of timer daemon OS task
 */
#define OS_DAEMON_TASK_PRIORITY         _OS_DAEMON_TASK_PRIORITY

/*
 * OSAL DATA TYPE AND ENUMERATION FORWARD MACROS
 *****************************************************************************************
 */

/*
 * OS task priority values
 */
#define OS_TASK_PRIORITY_LOWEST         _OS_TASK_PRIORITY_LOWEST        ///< Lowest (idle) task priority
#define OS_TASK_PRIORITY_NORMAL         _OS_TASK_PRIORITY_NORMAL        ///< Normal task priority
#define OS_TASK_PRIORITY_HIGHEST        _OS_TASK_PRIORITY_HIGHEST       ///< Highest task priority

/*
 * Data types and enumerations for OS tasks and functions that operate on them
 */
#define OS_TASK                         _OS_TASK                        ///< Task handle type
#define OS_TASK_STATUS                  _OS_TASK_STATUS                 ///< Task status type
#define OS_TASK_CREATE_SUCCESS          _OS_TASK_CREATE_SUCCESS         ///< Task created successfully
#define OS_TASK_NOTIFY_SUCCESS          _OS_TASK_NOTIFY_SUCCESS         ///< Task notified successfully
#define OS_TASK_NOTIFY_FAIL             _OS_TASK_NOTIFY_FAIL            ///< Task failed to be notified
#define OS_TASK_NOTIFY_NO_WAIT          _OS_TASK_NOTIFY_NO_WAIT         ///< No-blocking and waiting for task to be notified
#define OS_TASK_NOTIFY_FOREVER          _OS_TASK_NOTIFY_FOREVER         ///< Maximum time (in OS ticks) to block while waiting for task to be notified
#define OS_TASK_NOTIFY_NONE             _OS_TASK_NOTIFY_NONE            ///< Bitmask value to notify NO other task
#define OS_TASK_NOTIFY_ALL_BITS         _OS_TASK_NOTIFY_ALL_BITS        ///< Bitmask value to update all notification bits

/*
 * Data types and enumerations for OS mutexes and functions that operate on them
 */
#define OS_MUTEX                        _OS_MUTEX                       ///< Mutex handle type
#define OS_MUTEX_CREATE_SUCCESS         _OS_MUTEX_CREATE_SUCCESS        ///< Mutex created successfully
#define OS_MUTEX_CREATE_FAIL            _OS_MUTEX_CREATE_FAIL           ///< Mutex failed to be created
DEPRECATED_MACRO(OS_MUTEX_CREATE_FAILED, "Macro no longer supported, use OS_MUTEX_CREATE_FAIL instead")
#define OS_MUTEX_CREATE_FAILED \
        ((void)OS_MUTEX_CREATE_FAILED,  OS_MUTEX_CREATE_FAIL)           ///< Mutex failed to be created \deprecated Use OS_MUTEX_CREATE_FAIL instead
#define OS_MUTEX_TAKEN                  _OS_MUTEX_TAKEN                 ///< Mutex acquired successfully
#define OS_MUTEX_NOT_TAKEN              _OS_MUTEX_NOT_TAKEN             ///< Mutex failed to be acquired
#define OS_MUTEX_NO_WAIT                _OS_MUTEX_NO_WAIT               ///< No-blocking and waiting for mutex to be acquired
#define OS_MUTEX_FOREVER                _OS_MUTEX_FOREVER               ///< Maximum time to block while waiting for mutex to be acquired

/*
 * Data types and enumerations for OS events and functions that operate on them
 */
#define OS_EVENT                        _OS_EVENT                       ///< Event handle type
#define OS_EVENT_CREATE_SUCCESS         _OS_EVENT_CREATE_SUCCESS        ///< Event created successfully
#define OS_EVENT_CREATE_FAIL            _OS_EVENT_CREATE_FAIL           ///< Event failed to be created
DEPRECATED_MACRO(OS_EVENT_CREATE_FAILED, "Macro no longer supported, use OS_EVENT_CREATE_FAIL instead")
#define OS_EVENT_CREATE_FAILED \
        ((void)OS_EVENT_CREATE_FAILED,  OS_EVENT_CREATE_FAIL)           ///< Event failed to be created \deprecated Use OS_EVENT_CREATE_FAIL instead
#define OS_EVENT_SIGNALED               _OS_EVENT_SIGNALED              ///< Event signaled
#define OS_EVENT_NOT_SIGNALED           _OS_EVENT_NOT_SIGNALED          ///< Event not signaled
#define OS_EVENT_NO_WAIT                _OS_EVENT_NO_WAIT               ///< No-blocking and waiting for event to be signaled
#define OS_EVENT_FOREVER                _OS_EVENT_FOREVER               ///< Maximum time (in OS ticks) to block while waiting for event to be signaled

/*
 * Data types and enumerations for OS event groups and functions that operate on them
 */
#define OS_EVENT_GROUP                  _OS_EVENT_GROUP                 ///< Event group handle type
#define OS_EVENT_GROUP_OK               _OS_EVENT_GROUP_OK              ///< Event group attribute is set
#define OS_EVENT_GROUP_FAIL             _OS_EVENT_GROUP_FAIL            ///< Event group attribute is cleared
#define OS_EVENT_GROUP_NO_WAIT          _OS_EVENT_GROUP_NO_WAIT         ///< No-blocking and waiting for event group bits to be set
#define OS_EVENT_GROUP_FOREVER          _OS_EVENT_GROUP_FOREVER         ///< Maximum time (in OS ticks) to block while waiting for event group bits to be set

/*
 * Data types and enumerations for OS queues and functions that operate on them
 */
#define OS_QUEUE                        _OS_QUEUE                       ///< Event group handle type
#define OS_QUEUE_OK                     _OS_QUEUE_OK                    ///< Queue operation completed successfully
#define OS_QUEUE_FULL                   _OS_QUEUE_FULL                  ///< Queue operation failed; queue is full
#define OS_QUEUE_EMPTY                  _OS_QUEUE_EMPTY                 ///< Queue operation failed; queue is empty
#define OS_QUEUE_NO_WAIT                _OS_QUEUE_NO_WAIT               ///< No-blocking and waiting for queue operation to complete
#define OS_QUEUE_FOREVER                _OS_QUEUE_FOREVER               ///< Maximum time (in OS ticks) to block while waiting for queue operation to complete

/*
 * Data types and enumerations for OS timers and functions that operate on them
 */
#define OS_TIMER                        _OS_TIMER                       ///< Timer handle type
#define OS_TIMER_SUCCESS                _OS_TIMER_SUCCESS               ///< Timer operation completed successfully
#define OS_TIMER_FAIL                   _OS_TIMER_FAIL                  ///< Timer operation failed
#define OS_TIMER_RELOAD                 _OS_TIMER_RELOAD                ///< Timer is periodic
#define OS_TIMER_ONCE                   _OS_TIMER_ONCE                  ///< Timer is one-shot
#define OS_TIMER_NO_WAIT                _OS_TIMER_NO_WAIT               ///< No-blocking and waiting for timer operation to complete
#define OS_TIMER_FOREVER                _OS_TIMER_FOREVER               ///< Maximum time (in OS ticks) to block while waiting for timer operation to complete

/*
 * Base data types matching underlying architecture
 */
#define OS_BASE_TYPE                    _OS_BASE_TYPE                   ///< Base type
#define OS_UBASE_TYPE                   _OS_UBASE_TYPE                  ///< Unsigned base type

/*
 * Enumeration values indicating successful or not OS operation
 */
#define OS_OK                           _OS_OK                          ///< Successful operation
#define OS_FAIL                         _OS_FAIL                        ///< Unsuccessful operation

/*
 * Boolean enumeration values
 */
#define OS_TRUE                         _OS_TRUE                        ///< Boolean True
#define OS_FALSE                        _OS_FALSE                       ///< Boolean False

/**
 * \brief Maximum OS delay (in OS ticks)
 */
#define OS_MAX_DELAY                    _OS_MAX_DELAY

/**
 * \brief OS tick time (i.e. time expressed in OS ticks) data type
 */
#define OS_TICK_TIME                    _OS_TICK_TIME

/**
 * \brief OS tick period (in cycles of source clock used for the OS timer)
 */
#define OS_TICK_PERIOD                  _OS_TICK_PERIOD

/**
 * \brief OS tick period (in msec)
 */
#define OS_TICK_PERIOD_MS               _OS_TICK_PERIOD_MS

/**
 * \brief OS tick period (in msec)
 *
 * \deprecated This macro is deprecated. Use OS_TICK_PERIOD_MS instead
 */
DEPRECATED_MACRO(OS_PERIOD_MS, "Macro no longer supported, use OS_TICK_PERIOD_MS instead")
#define OS_PERIOD_MS \
        ((void)OS_PERIOD_MS,            OS_TICK_PERIOD_MS)

/**
 * \brief Frequency (in Hz) of the source clock used for the OS timer
 */
#define OS_TICK_CLOCK_HZ                _OS_TICK_CLOCK_HZ

/**
 * \brief Data type of OS task function (i.e. OS_TASK_FUNCTION) argument
 *
 * \sa OS_TASK_FUNCTION
 */
#define OS_TASK_ARG_TYPE                _OS_TASK_ARG_TYPE

/*
 * Enumeration values indicating successful or not OS Atomic compare and swap operation
 */
#define OS_ATOMIC_COMPARE_AND_SWAP_SUCCESS     _OS_ATOMIC_COMPARE_AND_SWAP_SUCCESS
#define OS_ATOMIC_COMPARE_AND_SWAP_FAILURE     _OS_ATOMIC_COMPARE_AND_SWAP_FAILURE

/* Data type containing heap memory statistics */
#define OS_HEAP_STATISTICS_TYPE         _OS_HEAP_STATISTICS_TYPE

/*
 * OSAL ENUMERATIONS
 *****************************************************************************************
 */

/**
 * \brief OS task notification action
 */
typedef enum {
        OS_NOTIFY_NO_ACTION             = _OS_NOTIFY_NO_ACTION,                 /**< Subject task receives event, but its notification value is not updated */
        OS_NOTIFY_SET_BITS              = _OS_NOTIFY_SET_BITS,                  /**< Notification value of subject task will be bitwise ORed with task value */
        OS_NOTIFY_INCREMENT             = _OS_NOTIFY_INCREMENT,                 /**< Notification value of subject task will be incremented by one */
        OS_NOTIFY_VAL_WITH_OVERWRITE    = _OS_NOTIFY_VAL_WITH_OVERWRITE,        /**< Notification value of subject task is unconditionally set to task value */
        OS_NOTIFY_VAL_WITHOUT_OVERWRITE = _OS_NOTIFY_VAL_WITHOUT_OVERWRITE,     /**< If subject task has a notification pending then notification value
                                                                                     will be set to task value otherwise task value is not updated */
} OS_NOTIFY_ACTION;

/**
 * \brief OS task state
 */
typedef enum {
        OS_TASK_RUNNING                 = _OS_TASK_RUNNING,                     /**< Task is in running state, a task is querying the state of itself */
        OS_TASK_READY                   = _OS_TASK_READY,                       /**< Task is in a ready state */
        OS_TASK_BLOCKED                 = _OS_TASK_BLOCKED,                     /**< Task is in blocked state */
        OS_TASK_SUSPENDED               = _OS_TASK_SUSPENDED,                   /**< Task is in the suspended state, or is in the blocked state with an infinite time out */
        OS_TASK_DELETED                 = _OS_TASK_DELETED,                     /**< Task is deleted, but its TCB has not yet been freed */
} OS_TASK_STATE;

/**
 * \brief OS scheduler state
 */
typedef enum {
        OS_SCHEDULER_RUNNING            = _OS_SCHEDULER_RUNNING,                /**< Scheduler is in running state */
        OS_SCHEDULER_NOT_STARTED        = _OS_SCHEDULER_NOT_STARTED,            /**< Scheduler is not started */
        OS_SCHEDULER_SUSPENDED          = _OS_SCHEDULER_SUSPENDED,              /**< Scheduler is in the suspended state */
} OS_SCHEDULER_STATE;

/*
 * OSAL MACRO FUNCTION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief OS task function begin point of execution.
 */
#ifdef _OS_TASK_BEGIN
#define OS_TASK_BEGIN() _OS_TASK_BEGIN()
#else
#define OS_TASK_BEGIN() do { } while (0)
#endif /* _OS_TASK_BEGIN */

/**
 * \brief OS task function end point of execution.
 */
#ifdef _OS_TASK_END
#define OS_TASK_END() _OS_TASK_END()
#else
#define OS_TASK_END() do { } while (0)
#endif /* _OS_TASK_END */

/**
 * \brief Declare an OS task function
 *
 * \param [in] func name of the function
 * \param [in] arg function argument
 */
#define OS_TASK_FUNCTION(func, arg) _OS_TASK_FUNCTION(func, arg)

/**
 * \brief Run the OS task scheduler
 */
#define OS_TASK_SCHEDULER_RUN() _OS_TASK_SCHEDULER_RUN()

/**
 * \brief Convert a time in milliseconds to a time in ticks
 *
 * \param [in] time_in_ms time in milliseconds
 *
 * \return time in ticks
 *
 */
#define OS_TIME_TO_TICKS(time_in_ms) _OS_TIME_TO_TICKS(time_in_ms)

/**
 * \brief Return current OS task handle
 *
 * \return current OS task handle of type OS_TASK
 *
 */
#define OS_GET_CURRENT_TASK() _OS_GET_CURRENT_TASK()

/**
 * \brief Create OS task
 *
 * Function creates OS task. OS task is ready to run.
 *
 * \param [in] name task name
 * \param [in] task_func starting point of task
 * \param [in] arg parameter past to \p task_func on task start
 * \param [in] stack_size stack size allocated for task in bytes
 * \param [in] priority number specifying task priority
 * \param [in,out] task OS specific task handle
 *
 * \return OS_TASK_CREATE_SUCCESS if task was created successfully
 *
 */
#define OS_TASK_CREATE(name, task_func, arg, stack_size, priority, task) \
        _OS_TASK_CREATE((name), (task_func), (OS_TASK_ARG_TYPE)(arg), \
                         MAX(stack_size + dg_configSYSTEMVIEW_STACK_OVERHEAD, OS_MINIMAL_TASK_STACK_SIZE ) , \
                        (priority), (task))

/**
 * \brief Delete OS task
 *
 * Function deletes OS task.
 *
 * \param [in,out] task OS specific task handle
 *
 */
#define OS_TASK_DELETE(task) _OS_TASK_DELETE(task)

/**
 * \brief Get the priority of an OS task
 *
 * Function returns the priority of an OS task.
 *
 * \param [in] task OS specific task handle. if NULL, the running task is assumed.
 *
 * \return the priority of the task (OS_UBASE_TYPE)
 *
 */
#define OS_TASK_PRIORITY_GET(task) _OS_TASK_PRIORITY_GET(task)

/**
 * \brief Get the priority of an OS task from ISR
 *
 * A version of OS_TASK_PRIORITY_GET() that can be called from ISR.
 *
 * \param [in] task OS specific task handle. if NULL, the running task is assumed.
 *
 * \return the priority of the task (OS_UBASE_TYPE)
 *
 * \sa OS_TASK_PRIORITY_GET
 */
#define OS_TASK_PRIORITY_GET_FROM_ISR(task) _OS_TASK_PRIORITY_GET_FROM_ISR(task)

/**
 * \brief Set the priority of an OS task
 *
 * Function sets the priority of an OS task.
 *
 * \param [in] task OS specific task handle
 * \param [in] prio OS new priority
 *
 */
#define OS_TASK_PRIORITY_SET(task, prio) _OS_TASK_PRIORITY_SET((task), (prio))

/**
 * \brief The running OS task yields control to the scheduler
 *
 * Function requests a context switch to another OS task.
 *
 */
#define OS_TASK_YIELD() _OS_TASK_YIELD()

/**
 * \brief The running OS task yields control to the scheduler from ISR
 *
 * Function requests a context switch to another OS task from interrupt service routine (ISR).
 *
 */
#define OS_TASK_YIELD_FROM_ISR() _OS_TASK_YIELD_FROM_ISR()

/**
 * \brief Send notification to OS task, updating its notification value
 *
 * Must not be called from ISR!
 *
 * \param [in] task handle of task to notify
 * \param [in] value used to update the notification value of the subject \p task
 * \param [in] action action that performs when notify is occurred (OS_NOTIFY_ACTION)
 *
 * \return OS_TASK_NOTIFY_FAIL if \p action is set as OS_NOTIFY_VAL_WITHOUT_OVERWRITE and \p task
 *         already had a notification pending, OS_TASK_NOTIFY_SUCCESS otherwise
 *
 */
#define OS_TASK_NOTIFY(task, value, action) _OS_TASK_NOTIFY((task), (value), (action))

/**
 * \brief Send notification to OS task, updating one notification index value
 *
 * Must not be called from ISR!
 *
 * \param [in] task handle of task to notify
 * \param [in] index the index in the notification array that the notification will be sent
 * \param [in] value used to update the notification value of the subject \p task
 * \param [in] action action that performs when notify is occurred (OS_NOTIFY_ACTION)
 *
 * \return OS_TASK_NOTIFY_FAIL if \p action is set as OS_NOTIFY_VAL_WITHOUT_OVERWRITE and \p task
 *         already had a notification pending, OS_TASK_NOTIFY_SUCCESS otherwise
 *
 */
#define OS_TASK_NOTIFY_INDEXED(task, index, value, action) \
        _OS_TASK_NOTIFY_INDEXED((task), (index), (value), (action))

/**
 * \brief Send notification to OS task, updating its notification value and returning previous value
 *
 * Must not be called from ISR!
 *
 * \param [in] task handle of task to notify
 * \param [in] value used to update the notification value of the subject \p task
 * \param [in] action action that performs when notify is occurred (OS_NOTIFY_ACTION)
 * \param [out] prev_value pointer to previous notification value - optional - can be set to NULL
 *
 * \return OS_TASK_NOTIFY_FAIL if \p action is set as OS_NOTIFY_VAL_WITHOUT_OVERWRITE and \p task
 *         already had a notification pending, OS_TASK_NOTIFY_SUCCESS otherwise
 *
 */
#define OS_TASK_NOTIFY_AND_QUERY(task, value, action, prev_value) \
        _OS_TASK_NOTIFY_AND_QUERY((task), (value), (action), (prev_value))

/**
 * \brief Send notification to OS task, updating a notification index value and returning previous value
 *
 * Must not be called from ISR!
 *
 * \param [in] task handle of task to notify
 * \param [in] index the index in the notification array that the notification will be sent
 * \param [in] value used to update the notification value of the subject \p task
 * \param [in] action action that performs when notify is occurred (OS_NOTIFY_ACTION)
 * \param [out] prev_value pointer to previous notification value - optional - can be set to NULL
 *
 * \return OS_TASK_NOTIFY_FAIL if \p action is set as OS_NOTIFY_VAL_WITHOUT_OVERWRITE and \p task
 *         already had a notification pending, OS_TASK_NOTIFY_SUCCESS otherwise
 *
 */
#define OS_TASK_NOTIFY_AND_QUERY_INDEXED(task, index, value, action, prev_value) \
        _OS_TASK_NOTIFY_AND_QUERY_INDEXED((task), (index), (value), (action), (prev_value))

/**
 * \brief Send notification to OS task from ISR, updating its notification value
 *
 * A version of OS_TASK_NOTIFY() that can be called from ISR.
 *
 * \param [in] task handle of task to notify
 * \param [in] value used to update the notification value of the subject \p task
 * \param [in] action action that performs when notify is occurred (OS_NOTIFY_ACTION)
 *
 * \return OS_TASK_NOTIFY_FAIL if \p action is set as OS_NOTIFY_VAL_WITHOUT_OVERWRITE and \p task
 *         already had a notification pending, OS_TASK_NOTIFY_SUCCESS otherwise
 *
 * \sa OS_TASK_NOTIFY
 */
#define OS_TASK_NOTIFY_FROM_ISR(task, value, action) \
        _OS_TASK_NOTIFY_FROM_ISR((task), (value), (action))

/**
 * \brief Send notification to OS task from ISR, updating a notification index value
 *
 * A version of OS_TASK_NOTIFY_INDEXED() that can be called from ISR.
 *
 * \param [in] task handle of task to notify
 * \param [in] index the index in the notification array that the notification will be sent
 * \param [in] value used to update the notification value of the subject \p task
 * \param [in] action action that performs when notify is occurred (OS_NOTIFY_ACTION)
 *
 * \return OS_TASK_NOTIFY_FAIL if \p action is set as OS_NOTIFY_VAL_WITHOUT_OVERWRITE and \p task
 *         already had a notification pending, OS_TASK_NOTIFY_SUCCESS otherwise
 *
 * \sa OS_TASK_NOTIFY_INDEXED
 */
#define OS_TASK_NOTIFY_INDEXED_FROM_ISR(task, index, value, action) \
        _OS_TASK_NOTIFY_INDEXED_FROM_ISR((task), (index), (value), (action))

/**
 * \brief Send notification to OS task from ISR, updating its notification value and returning
 *        previous value
 *
 * A version of OS_TASK_NOTIFY_AND_QUERY() that can be called from ISR.
 *
 * \param [in] task handle of task to notify
 * \param [in] value used to update the notification value of the subject \p task
 * \param [in] action action that performs when notify is occurred (OS_NOTIFY_ACTION)
 * \param [out] prev_value pointer to previous notification value - optional - can be set to NULL
 *
 * \return OS_TASK_NOTIFY_FAIL if \p action is set as OS_NOTIFY_VAL_WITHOUT_OVERWRITE and \p task
 *         already had a notification pending, OS_TASK_NOTIFY_SUCCESS otherwise
 *
 * \sa OS_TASK_NOTIFY_AND_QUERY
 */
#define OS_TASK_NOTIFY_AND_QUERY_FROM_ISR(task, value, action, prev_value) \
        _OS_TASK_NOTIFY_AND_QUERY_FROM_ISR((task), (value), (action), (prev_value))

/**
 * \brief Send notification to OS task from ISR, updating a notification index value and returning
 *        previous value
 *
 * A version of OS_TASK_NOTIFY_AND_QUERY_INDEXED() that can be called from ISR.
 *
 * \param [in] task handle of task to notify
 * \param [in] index the index in the notification array that the notification will be sent
 * \param [in] value used to update the notification value of the subject \p task
 * \param [in] action action that performs when notify is occurred (OS_NOTIFY_ACTION)
 * \param [out] prev_value pointer to previous notification value - optional - can be set to NULL
 *
 * \return OS_TASK_NOTIFY_FAIL if \p action is set as OS_NOTIFY_VAL_WITHOUT_OVERWRITE and \p task
 *         already had a notification pending, OS_TASK_NOTIFY_SUCCESS otherwise
 *
 * \sa OS_TASK_NOTIFY_AND_QUERY_INDEXED
 */
#define OS_TASK_NOTIFY_AND_QUERY_INDEXED_FROM_ISR(task, index, value, action, prev_value) \
        _OS_TASK_NOTIFY_AND_QUERY_INDEXED_FROM_ISR((task), (index), (value), (action), (prev_value))

/**
 * \brief Send a notification event to OS task, incrementing its notification value
 *
 * Must not be called from ISR! Use OS_TASK_NOTIFY_GIVE_FROM_ISR() instead.
 *
 * \param [in] task handle of task to notify
 *
 * \return OS_TASK_NOTIFY_GIVE calls OS_TASK_NOTIFY with action set to OS_NOTIFY_INCREMENT resulting
 *         in all calls returning OS_TASK_NOTIFY_SUCCESS
 *
 * \sa OS_TASK_NOTIFY_GIVE_FROM_ISR
 */
#define OS_TASK_NOTIFY_GIVE(task) _OS_TASK_NOTIFY_GIVE(task)

/**
 * \brief Send a notification event to OS task, incrementing a notification index value
 *
 * Must not be called from ISR! Use OS_TASK_NOTIFY_GIVE_FROM_ISR() instead.
 *
 * \param [in] task handle of task to notify
 * \param [in] index the index in the notification array that the notification will be sent
 *
 * \return OS_TASK_NOTIFY_GIVE calls OS_TASK_NOTIFY with action set to OS_NOTIFY_INCREMENT resulting
 *         in all calls returning OS_TASK_NOTIFY_SUCCESS
 *
 * \sa OS_TASK_NOTIFY_GIVE_INDEXED_FROM_ISR
 */
#define OS_TASK_NOTIFY_GIVE_INDEXED(task, index) _OS_TASK_NOTIFY_GIVE_INDEXED((task), (index))

/**
 * \brief Send a notification event to OS task from ISR, incrementing its notification value
 *
 * Send notification event from interrupt service routine (ISR) to OS task that can unblock the
 * receiving OS task, incrementing the receiving OS task's notification value.
 * This function is safe to call from ISR.
 *
 * \param [in] task handle of task to notify
 *
 */
#define OS_TASK_NOTIFY_GIVE_FROM_ISR(task) _OS_TASK_NOTIFY_GIVE_FROM_ISR(task)

/**
 * \brief Send a notification event to OS task from ISR, incrementing a notification index value
 *
 * Send notification event from interrupt service routine (ISR) to OS task that can unblock the
 * receiving OS task, incrementing the receiving OS task's notification value.
 * This function is safe to call from ISR.
 *
 * \param [in] task handle of task to notify
 * \param [in] index the index in the notification array that the notification will be sent
 *
 */
#define OS_TASK_NOTIFY_GIVE_INDEXED_FROM_ISR(task, index) \
        _OS_TASK_NOTIFY_GIVE_INDEXED_FROM_ISR((task), (index))

/**
 * \brief Wait for the calling OS task to receive a notification event, clearing to zero or
 *        decrementing task notification value on exit
 *
 * This function waits for the calling OS task to receive a notification event and unblocks it
 * if its notification value is equal to zero. OS task notification value is updated for each
 * received notification according to clear_on_exit parameter value passed to the function.
 *
 * \param [in] clear_on_exit = OS_FALSE: task's notification value is decremented before
 *                             OS_TASK_NOTIFY_TAKE() exits,
 *                             OS_TRUE: task's notification value is reset to 0 before
 *                             OS_TASK_NOTIFY_TAKE() exits.
 * \param [in] time_to_wait maximum time to wait in the blocked state for a notification event
 *                          to be received if a notification is not already pending when
 *                          OS_TASK_NOTIFY_TAKE() is called
 *
 * \return the value of the task's notification value before it is decremented or cleared
 *
 */
#define OS_TASK_NOTIFY_TAKE(clear_on_exit, time_to_wait) \
        _OS_TASK_NOTIFY_TAKE((clear_on_exit), (time_to_wait))

/**
 * \brief Wait for the calling OS task to receive a notification event, clearing to zero or
 *        decrementing a task notification index value on exit
 *
 * This function waits for the calling OS task to receive a notification event and unblocks it
 * if its notification value is equal to zero. OS task notification value is updated for each
 * received notification according to clear_on_exit parameter value passed to the function.
 *
 * \param [in] index the index in the notification array that the notification will be sent
 * \param [in] clear_on_exit = OS_FALSE: task's notification value is decremented before
 *                             OS_TASK_NOTIFY_TAKE() exits,
 *                             OS_TRUE: task's notification value is reset to 0 before
 *                             OS_TASK_NOTIFY_TAKE() exits.
 * \param [in] time_to_wait maximum time to wait in the blocked state for a notification event
 *                          to be received if a notification is not already pending when
 *                          OS_TASK_NOTIFY_TAKE() is called
 *
 * \return the value of the task's notification value before it is decremented or cleared
 *
 */
#define OS_TASK_NOTIFY_TAKE_INDEXED(index, clear_on_exit, time_to_wait) \
        _OS_TASK_NOTIFY_TAKE_INDEXED((index), (clear_on_exit), (time_to_wait))

/**
 * \brief Clear the notification state of an OS task
 *
 * This function will clear the notification value of an OS task
 *
 * \param [in] task handle of task. Set to NULL to clear the notification
 *                  state of the calling task.
 *
 * \return @return OS_TASK_NOTIFY_SUCCESS if the task's notification state was set to
 *                 eNotWaitingNotification, otherwise OS_TASK_NOTIFY_FAIL.
 */
#define OS_TASK_NOTIFY_STATE_CLEAR(task) _OS_TASK_NOTIFY_STATE_CLEAR(task)

/**
 * \brief Clear a notification index state of an OS task
 *
 * This function will clear the value of one entry in the notification array
 * of an OS task
 *
 * \param [in] task handle of task. Set to NULL to clear the notification
 *                  state of the calling task.
 * \param [in] index the index in the notification value that will be cleared
 *
 * \return @return OS_TASK_NOTIFY_SUCCESS if the task's notification state was set to
 *                 eNotWaitingNotification, otherwise OS_TASK_NOTIFY_FAIL.
 */
#define OS_TASK_NOTIFY_STATE_CLEAR_INDEXED(task, index) \
        _OS_TASK_NOTIFY_STATE_CLEAR_INDEXED((task), (index))

/**
 * \brief Clear specific bits in the notification state of an OS task
 *
 * This function clears the bits specified in the provided bitmask from
 * the task's notification value.
 *
 * \param [in] task handle of task. Set to NULL to clear the notification
 *                  state of the calling task.
 * \param [in] bits_to_clear bits of the notification value that will be cleared
 *
 * \return The value of the target task's notification value before the specified
 *         bits were cleared.
 */
#define OS_TASK_NOTIFY_VALUE_CLEAR(task, bits_to_clear) \
        _OS_TASK_NOTIFY_VALUE_CLEAR((task), (bits_to_clear))

/**
 * \brief Clear specific bits in a notification index state of an OS task
 *
 * This function clears the bits specified in the provided bitmask from
 * one of the task's notification array values.
 *
 * \param [in] task handle of task. Set to NULL to clear the notification
 *                  state of the calling task.
 * \param [in] index the index in the notification value that will be modified
 * \param [in] bits_to_clear bits of the notification value that will be cleared
 *
 * \return The value of the target task's notification value before the specified
 *         bits were cleared.
 */
#define OS_TASK_NOTIFY_VALUE_CLEAR_INDEXED(task, index,bits_to_clear) \
        _OS_TASK_NOTIFY_VALUE_CLEAR_INDEXED((task), (index), (bits_to_clear))

/**
 * \brief Wait for the calling OS task to receive a notification, updating task notification value
 *        on exit
 *
 * This function waits for the calling OS task to receive a notification, updating OS task
 * notification value according to entry_bits and exit_bits parameter values passed to the function.
 *
 * \param [in] entry_bits any bits set in entry_bits will be cleared in calling notification value
 *                        before the task enters to OS_TASK_NOTIFY_WAIT
 * \param [in] exit_bits any bits set in exit_bits will be cleared in calling notification value
 *                       before OS_TASK_NOTIFY_WAIT function exits if a notification was received
 * \param [out] value pointer to task's notification value, if not required can be set to NULL
 * \param [in] ticks_to_wait maximum time to wait in the blocked state for a notification to be
 *                           received if a notification is not already pending when
 *                           OS_TASK_NOTIFY_WAIT is called
 *
 * \return OS_TASK_NOTIFY_SUCCESS if notification was received or was already pending when
 *         OS_TASK_NOTIFY_WAIT was called
 *         OS_TASK_NOTIFY_FAIL if the call OS_TASK_NOTIFY_WAIT timed out before notification was
 *         received
 *
 */
#define OS_TASK_NOTIFY_WAIT(entry_bits, exit_bits, value, ticks_to_wait) \
        _OS_TASK_NOTIFY_WAIT((entry_bits), (exit_bits), (value), (ticks_to_wait))

/**
 * \brief Wait for the calling OS task to receive a notification, updating a task notification index
 * value on exit
 *
 * This function waits for the calling OS task to receive a notification, updating OS task
 * notification value according to entry_bits and exit_bits parameter values passed to the function.
 *
 * \param [in] index the index in the notification array that the notification will be sent
 * \param [in] entry_bits any bits set in entry_bits will be cleared in calling notification value
 *                        before the task enters to OS_TASK_NOTIFY_WAIT
 * \param [in] exit_bits any bits set in exit_bits will be cleared in calling notification value
 *                       before OS_TASK_NOTIFY_WAIT function exits if a notification was received
 * \param [out] value pointer to task's notification value, if not required can be set to NULL
 * \param [in] ticks_to_wait maximum time to wait in the blocked state for a notification to be
 *                           received if a notification is not already pending when
 *                           OS_TASK_NOTIFY_WAIT is called
 *
 * \return OS_TASK_NOTIFY_SUCCESS if notification was received or was already pending when
 *         OS_TASK_NOTIFY_WAIT was called
 *         OS_TASK_NOTIFY_FAIL if the call OS_TASK_NOTIFY_WAIT timed out before notification was
 *         received
 *
 */
#define OS_TASK_NOTIFY_WAIT_INDEXED(index, entry_bits, exit_bits, value, ticks_to_wait) \
        _OS_TASK_NOTIFY_WAIT_INDEXED((index), (entry_bits), (exit_bits), (value), (ticks_to_wait))

/**
 * \brief Resume OS task
 *
 * Make OS task ready to run.
 *
 * \param [in] task handle of task to resume
 *
 */
#define OS_TASK_RESUME(task) _OS_TASK_RESUME(task)

/**
 * \brief Resume OS task from ISR
 *
 * Make OS task ready to run. This function is safe to call from ISR.
 *
 * \param [in] task handle of task to resume
 *
 */
#define OS_TASK_RESUME_FROM_ISR(task) _OS_TASK_RESUME_FROM_ISR(task)

/**
 * \brief Suspend OS task
 *
 * Remove OS task from execution queue. OS task will not be run
 * until OS_TASK_RESUME() or OS_TASK_RESUME_FROM_ISR() is called.
 *
 * \param [in] task handle of task to suspend
 *
 * \sa OS_TASK_RESUME
 * \sa OS_TASK_RESUME_FROM_ISR
 */
#define OS_TASK_SUSPEND(task) _OS_TASK_SUSPEND(task)

/**
 * \brief Create OS mutex
 *
 * Function creates OS mutex.
 *
 * \param [in,out] mutex mutex handle
 *
 * \return OS_MUTEX_CREATE_SUCCESS when mutex was created successfully, OS_MUTEX_CREATE_FAIL
 *         otherwise
 */
#define OS_MUTEX_CREATE(mutex) _OS_MUTEX_CREATE(mutex)

/**
 * \brief Delete OS mutex
 *
 * Function deletes OS mutex.
 *
 * \param [in] mutex handle of mutex to delete
 *
 */
#define OS_MUTEX_DELETE(mutex) _OS_MUTEX_DELETE(mutex)

/**
 * \brief Release OS mutex
 *
 * Decrease OS mutex count, and when number of calls to OS_MUTEX_GET() equals number of calls to
 * OS_MUTEX_PUT(), OS mutex can be acquired by other OS task.
 *
 * \param [in] mutex handle of mutex to release
 *
 * \sa OS_MUTEX_GET
 */
#define OS_MUTEX_PUT(mutex) _OS_MUTEX_PUT(mutex)

/**
 * \brief Acquire OS mutex
 *
 * Access to shared resource can be guarded by OS mutex. When OS task wants to get access
 * to this resource, call OS_MUTEX_GET(). If mutex was not taken by any OS task yet, call
 * will succeed and OS mutex will be assigned to calling OS task. Next call to already acquired
 * OS mutex from same OS task will succeed. If OS mutex is already taken by other OS task, calling
 * task will wait for specified time before failing.
 * For non-blocking acquire \p timeout can be OS_MUTEX_NO_WAIT, for infinite wait till
 * OS mutex is released OS_MUTEX_FOREVER should be used.
 *
 * \param [in] mutex handle of mutex to acquire
 * \param [in] timeout number of ticks that to acquire mutex
 *
 */
#define OS_MUTEX_GET(mutex, timeout) _OS_MUTEX_GET((mutex), (timeout))

/**
 * \brief Get OS task owner of OS mutex
 *
 * \param [in] mutex handle of mutex
 *
 * \return OS task owner of mutex
 */
#define OS_MUTEX_GET_OWNER(mutex) _OS_MUTEX_GET_OWNER(mutex)

/**
 * \brief Get OS task owner of OS mutex from ISR
 *
 * A version of OS_MUTEX_GET_OWNER() that can be called from ISR.
 *
 * \param [in] mutex handle of mutex
 *
 * \return OS task owner of mutex
 *
 * \sa OS_MUTEX_GET_OWNER
 */
#define OS_MUTEX_GET_OWNER_FROM_ISR(mutex) _OS_MUTEX_GET_OWNER_FROM_ISR(mutex)

/**
 * \brief Get OS mutex current count value
 *
 * \param [in] mutex handle of mutex
 *
 * \return mutex current count value
 */
#define OS_MUTEX_GET_COUNT(mutex) _OS_MUTEX_GET_COUNT(mutex)

/**
 * \brief Get OS mutex current count value from ISR
 *
 * A version of OS_MUTEX_GET_COUNT() that can be called from ISR.
 *
 * \param [in] mutex handle of mutex
 *
 * \return mutex current count value
 *
 * \sa OS_MUTEX_GET_COUNT
 */
#define OS_MUTEX_GET_COUNT_FROM_ISR(mutex) _OS_MUTEX_GET_COUNT_FROM_ISR(mutex)

/**
 * \brief Create OS event
 *
 * Function creates OS event that can be used to synchronize.
 *
 * \param [in,out] event event handle
 *
 */
#define OS_EVENT_CREATE(event) _OS_EVENT_CREATE(event)

/**
 * \brief Delete OS event
 *
 * Function destroys OS event.
 *
 * \param [in] event handle of event to delete
 *
 */
#define OS_EVENT_DELETE(event) _OS_EVENT_DELETE(event)

/**
 * \brief Set OS event in signaled state
 *
 * Set OS event in signaled state so OS_EVENT_WAIT() will release waiting OS task if any.
 * OS event will remain in signaled state till call to OS_EVENT_WAIT() releases one OS task.
 * This function should not be called from ISR.
 *
 * \param [in] event handle of event to signal
 *
 * \return OS_OK if event was signaled, otherwise OS_FAIL
 *
 * \sa OS_EVENT_WAIT
 */
#define OS_EVENT_SIGNAL(event) _OS_EVENT_SIGNAL(event)

/**
 * \brief Set OS event in signaled state from ISR
 *
 * Set OS event in signaled state so OS_EVENT_WAIT() will release waiting OS task if any.
 * OS event will remain in signaled state till call to OS_EVENT_WAIT() releases one OS task.
 * This function is safe to call from ISR.
 *
 * \param [in] event handle of event to signal
 *
 * \return OS_OK if event was signaled, otherwise OS_FAIL
 *
 * \sa OS_EVENT_WAIT
 */
#define OS_EVENT_SIGNAL_FROM_ISR(event) _OS_EVENT_SIGNAL_FROM_ISR(event)

/**
 * \brief Set OS event in signaled state from ISR without requesting running OS task to yield
 *
 * Set OS event in signaled state so OS_EVENT_WAIT() will release waiting OS task if any, without
 * requesting running OS task to yield.
 * OS event will remain in signaled state till call to OS_EVENT_WAIT() releases one OS task.
 * This function is safe to call from ISR.
 *
 * \param [in] event handle of event to signal
 * \param [in, out] need_yield indication whether task yield is needed
 *
 * \return OS_OK if event was signaled, otherwise OS_FAIL
 *
 *\sa OS_EVENT_WAIT
 */
#define OS_EVENT_SIGNAL_FROM_ISR_NO_YIELD(event, need_yield) \
        _OS_EVENT_SIGNAL_FROM_ISR_NO_YIELD((event), (need_yield))

/**
 * \brief Wait for OS event to be signaled
 *
 * This function waits for OS event to be in signaled state.
 * If OS event was already in signaled state or become signaled in specified time, function
 * will return OS_EVENT_SIGNALED, and the state of OS event will change to not signaled.
 * To check if OS event is already signaled use OS_EVENT_NO_WAIT as timeout.
 * To wait till OS event is signaled use OS_EVENT_FOREVER.
 * This function can't be used in ISR.
 *
 * \param [in] event handle of event to wait
 * \param [in] timeout number of ticks to wait
 *
 * \return OS_EVENT_SIGNALED if event was signaled, otherwise OS_EVENT_NOT_SIGNALED
 *
 */
#define OS_EVENT_WAIT(event, timeout) _OS_EVENT_WAIT((event), (timeout))

/**
 * \brief Check if OS event is signaled and clear it
 *
 * This function will return immediately with OS_EVENT_SIGNALED if OS event
 * was in signaled state already. In case OS event is signaled, its state
 * changes to not signaled after calling this function.
 *
 * \param [in] event handle of event to check
 *
 * \return OS_EVENT_SIGNALED if event was signaled, otherwise OS_EVENT_NOT_SIGNALED
 *
 */
#define OS_EVENT_CHECK(event) _OS_EVENT_CHECK(event)

/**
 * \brief Check from ISR if OS event is signaled and clear it
 *
 * This function will return immediately with OS_EVENT_SIGNALED if OS event
 * was in signaled state already. In case OS event is signaled, its state
 * changes to not signaled after calling this function.
 *
 * \param [in] event handle of event to check
 *
 * \return OS_EVENT_SIGNALED if event was signaled, otherwise OS_EVENT_NOT_SIGNALED
 *
 */
#define OS_EVENT_CHECK_FROM_ISR(event) _OS_EVENT_CHECK_FROM_ISR(event)

/**
 * \brief Check from ISR if OS event is signaled and clear it, without requesting running
 *        OS task to yield
 *
 * This function will return immediately with OS_EVENT_SIGNALED if OS event
 * was in signaled state already, without requesting running OS task to yield.
 * In case OS event is signaled, its state changes to not signaled after
 * calling this function.
 *
 * \param [in] event handle of event to check
 * \param [in, out] need_yield indication whether task yield is needed
 *
 * \return OS_EVENT_SIGNALED if event was signaled, otherwise OS_EVENT_NOT_SIGNALED
 *
 */
#define OS_EVENT_CHECK_FROM_ISR_NO_YIELD(event, need_yield) \
        _OS_EVENT_CHECK_FROM_ISR_NO_YIELD((event), (need_yield))

/**
 * \brief Get OS event status
 *
 * This function will return immediately with OS_EVENT_SIGNALED if OS event
 * was in signaled state already. In case OS event is signaled, its state
 * remains to signaled after calling this function.
 *
 * \param [in] event handle of event to check
 *
 * \return OS_EVENT_SIGNALED if event was signaled, otherwise OS_EVENT_NOT_SIGNALED
 *
 */
#define OS_EVENT_GET_STATUS(event) _OS_EVENT_GET_STATUS(event)

/**
 * \brief Get OS event status from ISR
 *
 * This function will return immediately with OS_EVENT_SIGNALED if OS event
 * was in signaled state already. In case OS event is signaled, its state
 * remains to signaled after calling this function.
 *
 * This function is safe to call from ISR.
 *
 * \param [in] event handle of event to check
 *
 * \return OS_EVENT_SIGNALED if event was signaled, otherwise OS_EVENT_NOT_SIGNALED
 *
 */
#define OS_EVENT_GET_STATUS_FROM_ISR(event) _OS_EVENT_GET_STATUS_FROM_ISR(event)

/**
 * \brief The running OS task yields control to the scheduler from ISR
 *
 * Function requests a context switch to another OS task.
 * This function is safe to call from ISR.
 *
 * \deprecated This macro is deprecated. User shall use OS_TASK_YIELD_FROM_ISR instead
 *
 * \sa OS_TASK_YIELD_FROM_ISR
 */
DEPRECATED_MACRO(OS_EVENT_YIELD, "Macro no longer supported, use OS_TASK_YIELD_FROM_ISR instead")
#define OS_EVENT_YIELD(higherPriorityTaskWoken) \
        do { \
                (void)OS_EVENT_YIELD; \
                if (higherPriorityTaskWoken != OS_FALSE) { \
                        OS_TASK_YIELD_FROM_ISR(); \
                } \
        } while (0)

/**
 * \brief Create OS event group
 *
 * Function creates OS event group
 *
 * \return event group handle if successful, otherwise NULL
 *
 */
#define OS_EVENT_GROUP_CREATE() _OS_EVENT_GROUP_CREATE()

/**
 * \brief Wait for OS event group bits to become set
 *
 * Function reads bits within an OS event group optionally entering the Blocked state
 * (with a timeout) to wait for a bit or group of bits to become set.
 *
 * \param [in] event_group handle of event group in which the bits are being tested
 * \param [in] bits_to_wait a bitwise value to test inside the event group
 * \param [in] clear_on_exit = OS_TRUE: any bits set in the value passed as the bits_to_wait
 *                             parameter will be cleared in the event group before
 *                             OS_EVENT_GROUP_WAIT_BITS() returns
 *                             OS_FALSE: bits in the event group are not altered when the call
 *                             to OS_EVENT_GROUP_WAIT_BITS() returns
 * \param [in] wait_for_all = OS_TRUE: OS_EVENT_GROUP_WAIT_BITS() will return when either *all* bits
 *                            set in the value passed as the bits_to_wait parameter are set in
 *                            the event group
 *                            OS_FALSE: OS_EVENT_GROUP_WAIT_BITS() will return when *any* of the bits
 *                            set in the value passed as the bits_to_wait parameter are set in
 *                            the event group
 * \param [in] timeout maximum amount of time to wait for one/all of the bits specified
 *                     by bits_to_wait to become set
 *
 * \return the value of the event group bits at the time either the event bits being waited for
 *         became set, or the timeout expired
 *
 */
#define OS_EVENT_GROUP_WAIT_BITS(event_group, bits_to_wait, clear_on_exit, wait_for_all, timeout) \
        _OS_EVENT_GROUP_WAIT_BITS((event_group), (bits_to_wait), (clear_on_exit), (wait_for_all), \
                                  (timeout))

/**
 * \brief Set OS event group bits
 *
 * Set bits (flags) within an OS event group
 *
 * \param [in] event_group handle of event group in which the bits are to be set
 * \param [in] bits_to_set a bitwise value that indicates the bit or bits to set in the event group
 *
 * \return the value of the event group bits at the time the call to OS_EVENT_GROUP_SET_BITS() returns
 *
 */
#define OS_EVENT_GROUP_SET_BITS(event_group, bits_to_set) \
        _OS_EVENT_GROUP_SET_BITS((event_group), (bits_to_set))

/**
 * \brief Set OS event group bits from ISR
 *
 * Set bits (flags) within an OS event group that can be called from an ISR.
 *
 * \param [in] event_group handle of event group in which the bits are to be set
 * \param [in] bits_to_set a bitwise value that indicates the bit or bits to set in the event group
 *
 * \return if the message was sent to the daemon OS task, then OS_OK is returned,
 *         otherwise OS_FAIL is returned
 *
 */
#define OS_EVENT_GROUP_SET_BITS_FROM_ISR(event_group, bits_to_set) \
        _OS_EVENT_GROUP_SET_BITS_FROM_ISR((event_group), (bits_to_set))

/**
 * \brief Set OS event group bits from ISR without requesting running OS task to yield
 *
 * Set bits (flags) within an OS event group that can be called from an ISR without requesting
 * running OS task to yield.
 *
 * \param [in] event_group handle of event group in which the bits are to be set
 * \param [in] bits_to_set a bitwise value that indicates the bit or bits to set in the event group
 * \param [in, out] need_yield indication whether task yield is needed
 *
 * \return if the message was sent to the daemon OS task, then OS_OK is returned,
 *         otherwise OS_FAIL is returned
 *
 */
#define OS_EVENT_GROUP_SET_BITS_FROM_ISR_NO_YIELD(event_group, bits_to_set, need_yield) \
        _OS_EVENT_GROUP_SET_BITS_FROM_ISR_NO_YIELD((event_group), (bits_to_set), (need_yield))

/**
 * \brief Clear OS event group bits
 *
 * Function clears bits (flags) within an OS event group.
 *
 * \param [in] event_group handle of event group in which the bits are to be cleared
 * \param [in] bits_to_clear a bitwise value that indicates the bit or bits to clear
 *                           in the event group
 *
 * \return value of the event group bits before the specified bits were cleared
 *
 */
#define OS_EVENT_GROUP_CLEAR_BITS(event_group, bits_to_clear) \
        _OS_EVENT_GROUP_CLEAR_BITS((event_group), (bits_to_clear))

/**
 * \brief Clear OS event group bits from an interrupt
 *
 * Function clears bits (flags) within an OS event group from an interrupt.
 *
 * \param [in] event_group handle of event group in which the bits are to be cleared
 * \param [in] bits_to_clear a bitwise value that indicates the bit or bits to clear
 *                           in the event group
 *
 * \return value of the event group bits before the specified bits were cleared
 *
 */
#define OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR(event_group, bits_to_clear) \
        _OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR((event_group), (bits_to_clear))

/**
 * \brief Get OS event group bits
 *
 * Function returns the current value of the event bits (event flags) in an OS event group.
 *
 * \param [in] event_group handle of event group being queried
 *
 * \return value of the event group bits in the event group at the time
 *         OS_EVENT_GROUP_GET_BITS() was called
 *
 */
#define OS_EVENT_GROUP_GET_BITS(event_group) _OS_EVENT_GROUP_GET_BITS(event_group)

/**
 * \brief Get OS event group bits from an interrupt
 *
 * Function returns the current value of the event bits (event flags) in an OS event group from an
 * interrupt.
 *
 * \param [in] event_group handle of event group being queried
 *
 * \return value of the event group bits in the event group at the time
 *         OS_EVENT_GROUP_GET_BITS_FROM_ISR() was called
 *
 */
#define OS_EVENT_GROUP_GET_BITS_FROM_ISR(event_group) _OS_EVENT_GROUP_GET_BITS_FROM_ISR(event_group)

/**
 * \brief Synchronize OS event group bits
 *
 * Atomically set bits (flags) within an OS event group, then wait for a combination of bits to be
 * set within the same OS event group.
 *
 * \param [in] event_group handle of event group in which the bits are being set and tested
 * \param [in] bits_to_set bit or bits to set in the event group before determining if all
 *                         the bits specified by the bits_to_wait parameter are set
 * \param [in] bits_to_wait a bitwise value that indicates the bit or bits to test inside
 *                          the event group
 * \param [in] timeout maximum amount of time (specified in 'ticks') to wait for all the bits
 *                     specified by the bits_to_wait parameter value to become set
 *
 * \return value of the event group bits at the time either the bits being waited for became set,
 *         or the block time expired
 *
 */
#define OS_EVENT_GROUP_SYNC(event_group, bits_to_set, bits_to_wait, timeout) \
        _OS_EVENT_GROUP_SYNC((event_group), (bits_to_set), (bits_to_wait), (timeout))

/**
 * \brief Delete OS event group
 *
 * Function deletes an OS event group.
 *
 * \param [in] event_group handle of event group being deleted
 *
 */
#define OS_EVENT_GROUP_DELETE(event_group) _OS_EVENT_GROUP_DELETE(event_group)

/**
 * \brief Create OS queue
 *
 * Function creates OS queue that can contain \p max_items of specified size.
 *
 * \param [in,out] queue handle of queue to initialize
 * \param [in] item_size queue element size
 * \param [in] max_items max number of items that queue can store
 *
 */
#define OS_QUEUE_CREATE(queue, item_size, max_items) \
        _OS_QUEUE_CREATE((queue), (item_size), (max_items))

/**
 * \brief Deletes OS queue
 *
 * Function deletes OS queue.
 *
 * \param [in] queue handle of queue to delete
 *
 */
#define OS_QUEUE_DELETE(queue) _OS_QUEUE_DELETE(queue)

/**
 * \brief Put element in OS queue
 *
 * Function adds element into OS queue if there is enough room for it.
 * If there is no room in OS queue for \p timeout ticks element is not
 * put in OS queue and error is returned.
 *
 * \param [in] queue handle of queue to put item to
 * \param [in] item pointer to element to enqueue
 * \param [in] timeout max time in ticks to wait for space in queue
 *
 * \return OS_QUEUE_FULL if there was no place in queue
 *         OS_QUEUE_OK if message was put in queue
 *
 */
#define OS_QUEUE_PUT(queue, item, timeout) _OS_QUEUE_PUT((queue), (item), (timeout))

/**
 * \brief Put element in OS queue
 *
 * Function adds element into OS queue if there is enough room for it.
 * If there is no room in OS queue error is returned immediately.
 *
 * This is safe to call from ISR.
 *
 * \param [in] queue handle of queue to put item to
 * \param [in] item pointer to element to enqueue
 *
 * \return OS_QUEUE_FULL if there was no place in queue
 *         OS_QUEUE_OK if message was put in queue
 *
 */
#define OS_QUEUE_PUT_FROM_ISR(queue, item) _OS_QUEUE_PUT_FROM_ISR((queue), (item))

/**
 * \brief Replace element in OS queue of one element
 *
 * Function adds element into OS queue if it is empty.
 * If OS queue is not empty, the element is replaced/overwritten.
 *
 * \param [in] queue handle of queue to put item to
 * \param [in] item pointer to element to enqueue
 *
 * \return OS_QUEUE_OK is always returned
 *
 * \note Macro is intended for use with queues that have a length of one,
 *       meaning the queue is either empty or full.
 */
#define OS_QUEUE_REPLACE(queue, item) _OS_QUEUE_REPLACE((queue), (item))

/**
 * \brief Replace element in OS queue of one element from ISR
 *
 * Function adds element into OS queue if it is empty.
 * If OS queue is not empty, the element is replaced/overwritten.
 *
 * This is safe to call from ISR.
 *
 * \param [in] queue handle of queue to put item to
 * \param [in] item pointer to element to enqueue
 *
 * \return OS_QUEUE_OK is always returned
 *
 * \note Macro is intended for use with queues that have a length of one,
 *       meaning the queue is either empty or full.
 */
#define OS_QUEUE_REPLACE_FROM_ISR(queue, item) _OS_QUEUE_REPLACE_FROM_ISR((queue), (item))

/**
 * \brief Replace element in OS queue of one element from ISR without requesting running OS task
 *        to yield
 *
 * Function adds element into OS queue if it is empty.
 * If OS queue is not empty, the element is replaced/overwritten.
 *
 * This is safe to call from ISR, without requesting current OS task to yield.
 *
 * \param [in] queue handle of queue to put item to
 * \param [in] item pointer to element to enqueue
 * \param [in, out] need_yield indication whether task yield is needed
 *
 * \return OS_QUEUE_OK is always returned
 *
 * \note Macro is intended for use with queues that have a length of one,
 *       meaning the queue is either empty or full.
 */
#define OS_QUEUE_REPLACE_FROM_ISR_NO_YIELD(queue, item, need_yield) \
        _OS_QUEUE_REPLACE_FROM_ISR_NO_YIELD((queue), (item), (need_yield))

/**
 * \brief Get element from OS queue
 *
 * Function gets element from OS queue and removes it.
 * If there is nothing in OS queue for \p timeout ticks error is returned.
 * Use OS_QUEUE_NO_WAIT for \p timeout to get message without waiting.
 * Use OS_QUEUE_FOREVER to wait till message arrives.
 *
 * \param [in] queue handle of queue to get item from
 * \param [out] item pointer to buffer that will receive element from queue
 * \param [in] timeout max time in ticks to wait for element in queue
 *
 * \return OS_QUEUE_EMPTY if there was nothing in queue
 *         OS_QUEUE_OK if element was received from queue
 *
 */
#define OS_QUEUE_GET(queue, item, timeout) _OS_QUEUE_GET((queue), (item), (timeout))

/**
 * \brief Get element from OS queue from ISR
 *
 * Function gets element from OS queue and removes it.
 * If there is nothing in OS queue, indication that OS queue is empty is returned immediately.
 *
 * This is safe to call from ISR.
 *
 * \param [in] queue handle of queue to get item from
 * \param [out] item pointer to buffer that will receive element from queue
 *
 * \return OS_QUEUE_EMPTY if there was nothing in queue
 *         OS_QUEUE_OK if element was received from queue
 *
 */
#define OS_QUEUE_GET_FROM_ISR(queue, item) _OS_QUEUE_GET_FROM_ISR((queue), (item))

/**
 * \brief Get element from OS queue from ISR without requesting running OS task to yield
 *
 * Function gets element from OS queue and removes it.
 * If there is nothing in OS queue, indication that OS queue is empty is returned immediately.
 *
 * This is safe to call from ISR, without requesting current OS task to yield.
 *
 * \param [in] queue handle of queue to get item from
 * \param [out] item pointer to buffer that will receive element from queue
 * \param [in, out] need_yield indication whether task yield is needed
 *
 * \return OS_QUEUE_EMPTY if there was nothing in queue
 *         OS_QUEUE_OK if element was received from queue
 *
 */
#define OS_QUEUE_GET_FROM_ISR_NO_YIELD(queue, item, need_yield) \
        _OS_QUEUE_GET_FROM_ISR_NO_YIELD((queue), (item), (need_yield))

/**
 * \brief Peek element from OS queue
 *
 * Function gets element from OS queue without removing it.
 * If there is nothing in OS queue for \p timeout ticks error is returned.
 * Use OS_QUEUE_NO_WAIT for \p timeout to get message without waiting.
 * Use OS_QUEUE_FOREVER to wait till message arrives.
 *
 * \param [in] queue handle of queue to peek item from
 * \param [out] item pointer to buffer that will receive element from queue
 * \param [in] timeout max time in ticks to wait for element in queue
 *
 * \return OS_QUEUE_EMPTY if there was nothing in queue
 *         OS_QUEUE_OK if element was received from queue
 *
 */
#define OS_QUEUE_PEEK(queue, item, timeout) _OS_QUEUE_PEEK((queue), (item), (timeout))

/**
 * \brief Peek element from OS queue from ISR
 *
 * Function gets element from OS queue without removing it.
 * If there is nothing in OS queue, indication that OS queue is empty is returned immediately.
 *
 * This is safe to call from ISR.
 *
 * \param [in] queue handle of queue to peek item from
 * \param [out] item pointer to buffer that will receive element from queue
 *
 * \return OS_QUEUE_EMPTY if there was nothing in queue
 *         OS_QUEUE_OK if element was received from queue
 *
 */
#define OS_QUEUE_PEEK_FROM_ISR(queue, item) _OS_QUEUE_PEEK_FROM_ISR((queue), (item))

/**
 * \brief Get the number of messages stored in OS queue
 *
 * \param [in] queue handle of the queue to check.
 *
 * \return the number of messages available in the queue
 */
#define OS_QUEUE_MESSAGES_WAITING(queue) _OS_QUEUE_MESSAGES_WAITING(queue)

/**
 * \brief Get the number of messages stored in OS queue from ISR
 *
 * A version of OS_QUEUE_MESSAGES_WAITING() that can be called from ISR.
 *
 * \param [in] queue handle of the queue to check.
 *
 * \return the number of messages available in the queue
 *
 * \sa OS_QUEUE_MESSAGES_WAITING
 */
#define OS_QUEUE_MESSAGES_WAITING_FROM_ISR(queue) _OS_QUEUE_MESSAGES_WAITING_FROM_ISR(queue)

/**
 * \brief Get the number of free spaces in OS queue
 *
 * Function gets the number of free spaces in OS queue, that is the
 * number of items that can be sent to OS queue before OS queue becomes full
 * if no items are removed.
 *
 * \param [in] queue handle of the queue to check
 *
 * \return the number of free spaces available in the queue
 */
#define OS_QUEUE_SPACES_AVAILABLE(queue) _OS_QUEUE_SPACES_AVAILABLE(queue)

/**
 * \brief Create OS timer
 *
 * Function creates software OS timer with given timeout.
 *
 * \param [in] name     timer name
 * \param [in] period   timer period in ticks
 * \param [in] reload   indicates if callback will be called once or multiple times
 *             OS_TIMER_RELOAD - the callback will be called multiple times
 *             OS_TIMER_ONCE   - the callback will be called once
 * \param [in] timer_id identifier which can be used to identify timer in callback function
 * \param [in] callback callback called when timer expires
 *
 * \return handle of the timer if timer created successfully, otherwise NULL
 */
#define OS_TIMER_CREATE(name, period, reload, timer_id, callback) \
        _OS_TIMER_CREATE((name), (period), (reload), (timer_id), (callback))

/**
 * \brief Get OS timer ID
 *
 * Function returns OS timer id assigned in OS_TIMER_CREATE().
 *
 * \param [in] timer timer handle
 *
 * \return timer id
 *
 * \sa OS_TIMER_CREATE
 */
#define OS_TIMER_GET_TIMER_ID(timer) _OS_TIMER_GET_TIMER_ID(timer)

/**
 * \brief Check if OS timer is active
 *
 * Function checks OS timer status.
 *
 * \param [in] timer timer handle
 *
 * \return true if timer is active, otherwise false
 */
#define OS_TIMER_IS_ACTIVE(timer) _OS_TIMER_IS_ACTIVE(timer)

/**
 * \brief Start OS timer
 *
 * Function starts OS timer.
 *
 * \param [in] timer timer handle
 * \param [in] timeout max time in ticks to wait until command is sent
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL if timeout has occured
 *
 * \sa OS_TIMER_CREATE
 */
#define OS_TIMER_START(timer, timeout) _OS_TIMER_START((timer), (timeout))

/**
 * \brief Stop OS timer
 *
 * Function stops OS timer.
 *
 * \param [in] timer timer handle
 * \param [in] timeout max time in ticks to wait until command is sent
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL if timeout has occured
 *
 * \sa OS_TIMER_CREATE
 */
#define OS_TIMER_STOP(timer, timeout) _OS_TIMER_STOP((timer), (timeout))

/**
 * \brief Change OS timer's period
 *
 * Functions updates OS timer's period.
 *
 * \param [in] timer timer handle
 * \param [in] period new timer's period
 * \param [in] timeout max time in ticks to wait until command is sent
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL if timeout has occured
 */
#define OS_TIMER_CHANGE_PERIOD(timer, period, timeout) \
        _OS_TIMER_CHANGE_PERIOD((timer), (period), (timeout))

/**
 * \brief Delete OS timer
 *
 * Function deletes previously created OS timer.
 *
 * \param [in] timer timer handle
 * \param [in] timeout max time in ticks to wait until command is sent
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL if timeout has occured
 */
#define OS_TIMER_DELETE(timer, timeout) _OS_TIMER_DELETE((timer), (timeout))

/**
 * \brief Reset OS timer
 *
 * Function restarts previously created OS timer.
 *
 * \param [in] timer timer handle
 * \param [in] timeout max time in ticks to wait until command is sent
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL if timeout has occured
 */
#define OS_TIMER_RESET(timer, timeout) _OS_TIMER_RESET((timer), (timeout))

/**
 * \brief Start OS timer from ISR
 *
 * Version of OS_TIMER_START() that can be called from an interrupt service
 * routine.
 *
 * \param [in] timer timer handle
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL otherwise
 *
 * \sa OS_TIMER_START
 */
#define OS_TIMER_START_FROM_ISR(timer) _OS_TIMER_START_FROM_ISR(timer)

/**
 * \brief Stop OS timer from ISR
 *
 * Version of OS_TIMER_STOP() that can be called from an interrupt service
 * routine.
 *
 * \param [in] timer timer handle
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL otherwise
 *
 * \sa OS_TIMER_STOP
 */
#define OS_TIMER_STOP_FROM_ISR(timer) _OS_TIMER_STOP_FROM_ISR(timer)

/**
 * \brief Change OS timer period from ISR
 *
 * Version of OS_TIMER_CHANGE_PERIOD() that can be called from an interrupt service
 * routine.
 *
 * \param [in] timer timer handle
 * \param [in] period new timer period
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL otherwise
 *
 * \sa OS_TIMER_CHANGE_PERIOD
 */
#define OS_TIMER_CHANGE_PERIOD_FROM_ISR(timer, period) \
        _OS_TIMER_CHANGE_PERIOD_FROM_ISR((timer), (period))

/**
 * \brief Reset OS timer from ISR
 *
 * Version of OS_TIMER_RESET() that can be called from an interrupt service
 * routine.
 *
 * \param [in] timer timer handle
 *
 * \return OS_TIMER_SUCCESS if command has been sent successfully
 *         OS_TIMER_FAIL otherwise
 *
 * \sa OS_TIMER_RESET
 */
#define OS_TIMER_RESET_FROM_ISR(timer) _OS_TIMER_RESET_FROM_ISR(timer)

/**
 * \brief Set timer auto-reload mode
 *
 * Sets timer to auto-reload or oneshot mode. If auto_reload is equal to OS_TIMER_RELOAD, then
 * the timer will reload after it expires. If auto_reload is equal to OS_TIMER_ONCE, then
 * the timer will only expire once.
 *
 * \param [in] timer       timer handle
 * \param [in] auto_reload automatic reload mode. OS_TIMER_RELOAD sets the timer to auto-reload.
 *                         OS_TIMER_ONCE sets the timer to expire once.
 */
#define OS_TIMER_SET_RELOAD_MODE(timer, auto_reload) _OS_TIMER_SET_RELOAD_MODE(timer, auto_reload)

/**
 * \brief Get timer auto-reload mode
 *
 * \param [in] timer timer handle
 *
 * \return OS_TIMER_RELOAD if timer is set to auto reload after expiration, OS_TIMER_ONCE otherwise
 */
#define OS_TIMER_GET_RELOAD_MODE(timer) _OS_TIMER_GET_RELOAD_MODE(timer)

/**
 * \brief Delay execution of OS task for specified time
 *
 * This function delays in OS specific way execution of current OS task
 * for specified amount of time.
 *
 * \param [in] ticks number of ticks to wait
 *
 */
#define OS_DELAY(ticks) _OS_DELAY(ticks)

/**
 * \brief Delay execution of OS task until specified time
 *
 * This function delays in OS specific way execution of current OS task
 * until specified time is reached.
 *
 * \param [in] ticks absolute time to wait until in ticks
 *
 */
#define OS_DELAY_UNTIL(ticks) _OS_DELAY_UNTIL(ticks)

/**
 * \brief Get current OS tick count
 *
 * \return current tick count
 *
 */
#define OS_GET_TICK_COUNT() _OS_GET_TICK_COUNT()

/**
 * \brief Get current OS tick count from ISR
 *
 * Version of OS_GET_TICK_COUNT() that can be called from an interrupt service
 * routine.
 *
 * \return current tick count
 *
 * \sa OS_GET_TICK_COUNT
 */
#define OS_GET_TICK_COUNT_FROM_ISR() _OS_GET_TICK_COUNT_FROM_ISR()

/**
 * \brief Convert from OS ticks to ms
 *
 * \param [in] ticks tick count to convert
 *
 * \return value in ms
 *
 */
#define OS_TICKS_2_MS(ticks) _OS_TICKS_2_MS(ticks)

/**
 * \brief Convert from ms to OS ticks
 *
 * \param [in] ms milliseconds to convert
 *
 * \return value in OS ticks
 *
 */
#define OS_MS_2_TICKS(ms) _OS_MS_2_TICKS(ms)

/**
 * \brief Delay execution of OS task for specified time
 *
 * This function delays in OS specific way execution of current OS task.
 *
 * \param [in] ms number of ms to wait
 *
 */
#define OS_DELAY_MS(ms) _OS_DELAY_MS(ms)

/**
 * \brief Enter critical section from non-ISR context
 *
 * This allows to enter critical section from non-ISR context.
 * Implementation will disable interrupts with nesting counter.
 * This function can be called several times by OS task but requires same
 * number of OS_LEAVE_CRITICAL_SECTION() calls to allow OS task switching and interrupts
 * again.
 *
 * \sa OS_LEAVE_CRITICAL_SECTION
 *
 */
#define OS_ENTER_CRITICAL_SECTION() _OS_ENTER_CRITICAL_SECTION()

/**
 * \brief Enter critical section from ISR context
 *
 * This function allows to enter critical section from ISR context.
 * It can be called several times from within ISR context but requires same
 * number of OS_LEAVE_CRITICAL_SECTION_FROM_ISR() calls to restore interrupt status.
 *
 * \sa OS_LEAVE_CRITICAL_SECTION_FROM_ISR
 *
 */
#define OS_ENTER_CRITICAL_SECTION_FROM_ISR(critical_section_status) \
        _OS_ENTER_CRITICAL_SECTION_FROM_ISR(critical_section_status)

/**
 * \brief Leave critical section from non-ISR context
 *
 * Function restores interrupts and OS task switching.
 * Number of calls to this function must match number of calls to OS_ENTER_CRITICAL_SECTION().
 *
 * \sa OS_ENTER_CRITICAL_SECTION
 *
 */
#define OS_LEAVE_CRITICAL_SECTION() _OS_LEAVE_CRITICAL_SECTION()

/**
 * \brief Leave critical section from ISR context
 *
 * Function restores interrupts from ISR context.
 * Number of calls to this function must match number of calls to
 * OS_ENTER_CRITICAL_SECTION_FROM_ISR().
 *
 * \sa OS_ENTER_CRITICAL_SECTION_FROM_ISR
 *
 */
#define OS_LEAVE_CRITICAL_SECTION_FROM_ISR(critical_section_status) \
        _OS_LEAVE_CRITICAL_SECTION_FROM_ISR(critical_section_status)


/**
 * \brief Name for OS memory allocation function
 *
 * \sa OS_MALLOC
 *
 */
#define OS_MALLOC_FUNC _OS_MALLOC_FUNC

/**
 * \brief Name for non-retain memory allocation function
 *
 * \sa OS_MALLOC_FUNC
 *
 */
#define OS_MALLOC_NORET_FUNC _OS_MALLOC_NORET_FUNC

/**
 * \brief Allocate memory from OS provided heap
 *
 * \param [in] size size of memory to allocate
 *
 * \sa OS_FREE
 *
 */
#define OS_MALLOC(size) _OS_MALLOC(size)

/**
 * \brief Allocate memory from non-retain heap
 *
 * \param [in] size size of memory to allocate
 *
 * \sa OS_FREE
 *
 */
#define OS_MALLOC_NORET(size) _OS_MALLOC_NORET(size)


/**
 * \brief Name for OS memory reallocation function
 *
 * \sa OS_MALLOC
 *
 */
#define OS_REALLOC_FUNC _OS_REALLOC_FUNC

/**
 * \brief Name for non-retain memory reallocation function
 *
 * \sa OS_MALLOC_FUNC
 *
 */
#define OS_REALLOC_NORET_FUNC _OS_REALLOC_NORET_FUNC

/**
 * \brief Rellocate memory from OS provided heap
 *
 * \param [in] addr address of the allocated memory
 * \param [in] size size of memory to allocate
 *
 * \sa OS_FREE
 *
 */
#define OS_REALLOC(addr, size) _OS_REALLOC(addr, size)

/**
 * \brief Allocate memory from non-retain heap
 * 
 * \param [in] addr address of the allocated memory
 * \param [in] size size of memory to allocate
 *
 * \sa OS_FREE
 *
 */
#define OS_REALLOC_NORET(addr, size) _OS_REALLOC_NORET(addr, size)

/**
 * \brief Name for OS free memory function
 *
 * \sa OS_FREE
 *
 */
#define OS_FREE_FUNC _OS_FREE_FUNC

/**
 * \brief Name for non-retain memory free function
 *
 * \sa OS_FREE_NORET
 * \sa OS_MALLOC_NORET
 *
 */
#define OS_FREE_NORET_FUNC _OS_FREE_NORET_FUNC

/**
 * \brief Free memory allocated by OS_MALLOC()
 *
 * \param [in] addr address of the allocated memory
 *
 * \sa OS_MALLOC
 *
 */
#define OS_FREE(addr) _OS_FREE(addr)

/**
 * \brief Free memory allocated by OS_MALLOC_NORET()
 *
 * \param [in] addr address of the allocated memory
 *
 * \sa OS_MALLOC_NORET
 *
 */
#define OS_FREE_NORET(addr) _OS_FREE_NORET(addr)

#ifdef _OS_ASSERT
/**
 * \brief OS assertion
 *
 * \param [in] cond conditional expression
 */
#define OS_ASSERT(cond) _OS_ASSERT(cond)
#else
#define OS_ASSERT(cond)
#endif /* _OS_ASSERT */

#ifdef _OS_PRECONDITION
/**
 * \brief OS precondition
 *
 * \param [in] cond conditional expression
 */
#define OS_PRECONDITION(cond) _OS_PRECONDITION(cond)
#else
#define OS_PRECONDITION(cond)
#endif /* OS_PRECONDITION */

#ifdef _OS_MEMORY_BARRIER
/**
 * \brief OS memory barrier
 */
#define OS_MEMORY_BARRIER() _OS_MEMORY_BARRIER()
#else
#define OS_MEMORY_BARRIER()
#endif /* OS_MEMORY_BARRIER */

#ifdef _OS_SOFTWARE_BARRIER
/**
 * \brief OS software barrier
 */
#define OS_SOFTWARE_BARRIER() _OS_SOFTWARE_BARRIER()
#else
#define OS_SOFTWARE_BARRIER()
#endif /* OS_SOFTWARE_BARRIER */

#ifdef _OS_GET_TASKS_STATUS
/**
 * \brief Get monitored OS tasks' status
 *
 * Function gets the status of the monitored OS tasks in the system.
 *
 * \param [in] task_status where the status of the monitored tasks is stored
 * \param [in] status_size the size of the above container
 *
 * \return the number of the monitored tasks
 *
 */
#define OS_GET_TASKS_STATUS(task_status, status_size) \
        _OS_GET_TASKS_STATUS((task_status), (status_size))
#endif /* _OS_GET_TASKS_STATUS */

#ifdef _OS_GET_TASK_STACK_WATERMARK
/**
 * \brief Get the high water mark of the stack associated with an OS task
 *
 * Function returns the high water mark of the stack associated with an OS task. That is,
 * the minimum free stack space (in bytes) there has been since the OS task started.
 *
 * \param [in] task handle of the tracked task
 *
 * \return high water mark of the stack in bytes
 *
 */
#define OS_GET_TASK_STACK_WATERMARK(task) _OS_GET_TASK_STACK_WATERMARK(task)
#endif /* _OS_GET_TASK_STACK_WATERMARK */

#ifdef _OS_GET_TASK_STACK_WATERMARK
/**
 * \brief Get the high water mark of the stack associated with an OS task
 *
 * Function returns the high water mark of the stack associated with an OS task. That is,
 * the minimum free stack space (in bytes) there has been since the OS task started.
 *
 * \param [in] task handle of the tracked task
 *
 * \return high water mark of the stack in bytes
 *
 * \deprecated This macro is deprecated. User shall use OS_GET_TASK_STACK_WATERMARK instead
 */
DEPRECATED_MACRO(OS_GET_STACK_WATERMARK, "Macro no longer supported, use OS_GET_TASK_STACK_WATERMARK instead")
#define OS_GET_STACK_WATERMARK(task) \
        ({ \
                (void)OS_GET_STACK_WATERMARK; \
                OS_GET_TASK_STACK_WATERMARK(task); \
        })
#endif /* _OS_GET_TASK_STACK_WATERMARK */

/**
 * \brief Get the high water mark of heap
 *
 * Function gets the high water mark of heap. That is, the minimum free heap space (in bytes)
 * there has been since heap was initiated.
 *
 * \return heap high water mark in bytes
 *
 */
#define OS_GET_HEAP_WATERMARK() _OS_GET_HEAP_WATERMARK()

/**
 * \brief Get current free heap size
 *
 * Function gets heap current available size.
 *
 * \return current free heap size in bytes
 *
 */
#define OS_GET_FREE_HEAP_SIZE() _OS_GET_FREE_HEAP_SIZE()

/**
 * \brief Get current number of OS tasks
 *
 * Function gets current number of OS tasks.
 *
 * \return the current number of OS tasks
 *
 */
#define OS_GET_TASKS_NUMBER() _OS_GET_TASKS_NUMBER()

#ifdef _OS_GET_TASK_NAME
/**
 * \brief Get OS task name
 *
 * Function gets OS task name.
 *
 * \param [in] task handle of the monitored task
 *
 * \return a string pointer, points to the name of task
 *
 */
#define OS_GET_TASK_NAME(task) _OS_GET_TASK_NAME(task)
#endif /* _OS_GET_TASK_NAME */

#ifdef _OS_GET_TASK_STATE
/**
 * \brief Get OS task state
 *
 * Function gets OS task state.
 *
 * \param [in] task handle of the monitored task
 *
 * \return the task state in OS_TASK_STATE
 *
 */
#define OS_GET_TASK_STATE(task) _OS_GET_TASK_STATE(task)
#endif /* _OS_GET_TASK_STATE */

#ifdef _OS_GET_TASK_PRIORITY
/**
 * \brief Get OS task priority
 *
 * Function gets OS task priority.
 *
 * \param [in] task handle of the monitored task
 *
 * \return the task priority
 *
 */
#define OS_GET_TASK_PRIORITY(task) _OS_GET_TASK_PRIORITY(task)
#endif /* _OS_GET_TASK_PRIORITY */

#ifdef _OS_GET_TASK_SCHEDULER_STATE
/**
 * \brief Get OS task scheduler state
 *
 * \return task scheduler state (OS_SCHEDULER_STATE)
 */
#define OS_GET_TASK_SCHEDULER_STATE() _OS_GET_TASK_SCHEDULER_STATE()
#endif /* _OS_GET_TASK_SCHEDULER_STATE */

#ifdef _OS_GET_IDLE_TASK_HANDLE
/**
 * \brief Get OS task handle associated with the Idle OS task
 *
 * \return the task handle associated with the Idle task
 */
#define OS_GET_IDLE_TASK_HANDLE() _OS_GET_IDLE_TASK_HANDLE()
#endif /* _OS_GET_IDLE_TASK_HANDLE */

#ifdef _OS_GET_TASK_HANDLE
/**
 * \brief Get OS task handle by name
 *
 * \return the task handle that has the name given as value in task_name argument
 */
#define OS_GET_TASK_HANDLE(task_name) _OS_GET_TASK_HANDLE(task_name)
#endif /* _OS_GET_TASK_HANDLE */

#ifdef _OS_ATOMIC_COMPARE_AND_SWAP_U32
 /**
  * \brief Atomic compare and swap
  *
  * Change the contents of provided pointer with the exchange_value, if the provided
  * condition is true.
  *
  * \param [in, out] value_location Pointer to memory location from where a pointer
  *                                 value is to be loaded and written back to.
  * \param [in]      exchange_value The new value that will be swapped
  * \param [in]      swap_condition The condition that need to be true for the swap to be performed
  *
  * \return ATOMIC_COMPARE_AND_SWAP_SUCCESS if the value was swapped, ATOMIC_COMPARE_AND_SWAP_FAILURE otherwise
  *
  * \note This function only swaps *value_location with exchange_value, if previous
  *       *value_location value equals swap_condition.
  */
#define OS_ATOMIC_COMPARE_AND_SWAP_U32(value_location, exchange_value, swap_condition) \
        _OS_ATOMIC_COMPARE_AND_SWAP_U32(value_location, exchange_value, swap_condition)
#endif /* _OS_ATOMIC_COMPARE_AND_SWAP_U32 */

#ifdef _OS_ATOMIC_SWAP_POINTERS_P32
/**
 * \brief Atomic swap (pointers)
 *
 * Set the address pointed to by destination_pointer to the value of *exchange_pointer
 *
 * \param [in, out] destination_pointer  Pointer to memory location from where a pointer
 *                                       value is to be loaded and written back to.
 * \param [in]      exchange_pointer     Pointer value to be written to *destination_pointer.
 *
 * \return The initial value of *destination_pointer.
 */
#define OS_ATOMIC_SWAP_POINTERS_P32(destination_pointer, exchange_pointer) \
        _OS_ATOMIC_SWAP_POINTERS_P32(destination_pointer, exchange_pointer)
#endif /* _OS_ATOMIC_SWAP_POINTERS_P32 */

#ifdef _OS_ATOMIC_COMPARE_AND_SWAP_POINTERS_P32
/**
 * \brief Atomic compare and swap (pointers)
 *
 * Set the address pointed to by destination_pointer to the value of *exchange_pointer, if the provided
 * condition is true.
 *
 * \param[in, out] destination_pointer  Pointer to memory location from where a pointer
 *                                      value is to be loaded and written back to.
 * \param [in]     exchange_pointer     Pointer value to be written to *destination_pointer.
 * \param [in]     swap_condition       The condition that need to be true for the swap to be performed
 *
 * \return ATOMIC_COMPARE_AND_SWAP_SUCCESS if the value was swapped, ATOMIC_COMPARE_AND_SWAP_FAILURE otherwise
 *
 * \note This function only swaps *destination_pointer with exchange_pointer, if previous
 *       *destination_pointer value equals swap_condition.
 */
#define OS_ATOMIC_COMPARE_AND_SWAP_POINTERS_P32(destination_pointer, exchange_pointer, swap_condition) \
        _OS_ATOMIC_COMPARE_AND_SWAP_POINTERS_P32(destination_pointer, exchange_pointer, swap_condition)
#endif /* _OS_ATOMIC_COMPARE_AND_SWAP_POINTERS_P32 */

#ifdef _OS_ATOMIC_ADD_U32
/**
 * \brief Atomic add
 *
 * Add add_value to value located at value_location
 *
 * \param [in,out] value_location  Pointer to memory location from where value is to be
 *                                 loaded and written back to.
 * \param [in]     add_value       Value to be added to *value_location.
 *
 * \return previous *value_location value.
 */
#define OS_ATOMIC_ADD_U32(value_location, add_value) \
        _OS_ATOMIC_ADD_U32(value_location, add_value)
#endif /* _OS_ATOMIC_ADD_U32 */

#ifdef _OS_ATOMIC_SUBTRACT_U32
/**
 * \brief Atomic subtract
 *
 *  Subtract subtract_value from value located at value_location
 *
 * \param [in,out] value_location  Pointer to memory location from where value is to be
 *                                 loaded and written back to.
 * \param [in]     subtract_value  Value to be subtracted from *value_location.
 *
 * \return previous *value_location value.
 */
#define OS_ATOMIC_SUBTRACT_U32(value_location, subtract_value) \
        _OS_ATOMIC_SUBTRACT_U32(value_location, subtract_value)
#endif /* _OS_ATOMIC_SUBTRACT_U32 */

#ifdef _OS_ATOMIC_INCREMENT_U32
/**
 * \brief Atomic increment
 *
 * Increment value located at value_location by 1
 *
 * \param [in,out] value_location  Pointer to memory location from where value is to be
 *                                 loaded and written back to.
 *
 * \return previous *value_location value.
 */
#define OS_ATOMIC_INCREMENT_U32(value_location) _OS_ATOMIC_INCREMENT_U32(value_location)
#endif /* _OS_ATOMIC_INCREMENT_U32 */

#ifdef _OS_ATOMIC_DECREMENT_U32
  /**
   * \brief Atomic decrement
   *
   * Decrement value located at value_location by 1
   *
   * \param [in,out] value_location  Pointer to memory location from where value is to be
   *                                 loaded and written back to.
   *
   * \return previous *value_location value.
   */
#define OS_ATOMIC_DECREMENT_U32(value_location) _OS_ATOMIC_DECREMENT_U32(value_location)
#endif /* _OS_ATOMIC_DECREMENT_U32 */

#ifdef _OS_ATOMIC_OR_U32
/**
 * \brief Atomic OR
 *
 * Perform OR calculation on value at value_location with or_mask
 *
 * \param [in, out] value_location Pointer to memory location from where value is
 *                                 to be loaded and written back to.
 * \param [in]      or_mask        Value to be ORed with *value_location.
 *
 * \return previous *value_location value.
 */
#define OS_ATOMIC_OR_U32(value_location, or_mask) _OS_ATOMIC_OR_U32(value_location, or_mask)
#endif /* _OS_ATOMIC_OR_U32 */

#ifdef _OS_ATOMIC_AND_U32
 /**
  * \brief Atomic AND
  *
  * Perform AND calculation on value at value_location with and_mask
  *
 * \param [in, out] value_location Pointer to memory location from where value is
 *                                 to be loaded and written back to.
 * \param [in]      and_mask       Value to be ANDed with *value_location.
  *
  * \return previous *value_location value.
  */
#define OS_ATOMIC_AND_U32(value_location, and_mask) _OS_ATOMIC_AND_U32(value_location, and_mask)
#endif /* _OS_ATOMIC_AND_U32 */

#ifdef _OS_ATOMIC_NAND_U32
 /**
  * \brief Atomic NAND
  *
  * Perform NAND calculation on value at value_location with nand_mask
  *
  * \param [in, out] value_location Pointer to memory location from where value is
  *                                 to be loaded and written back to.
  * \param [in]      nand_mask      Value to be NANDed with *value_location.
  *
  * \return previous *value_location value.
  */
#define OS_ATOMIC_NAND_U32(value_location, nand_mask) _OS_ATOMIC_NAND_U32(value_location, nand_mask)
#endif /* _OS_ATOMIC_NAND_U32 */

#ifdef _OS_ATOMIC_XOR_U32
 /**
  * \brief Atomic XOR
  *
  * Perform XOR calculation on value at value_location with xor_mask
  *
  * \param [in, out] value_location Pointer to memory location from where value is
  *                                 to be loaded and written back to.
  * \param [in]      xor_mask       Value to be XORed with *value_location.
  *
  * \return previous *value_location value.
  */
#define OS_ATOMIC_XOR_U32(value_location, xor_mask) _OS_ATOMIC_XOR_U32(value_location, xor_mask)
#endif /* _OS_ATOMIC_XOR_U32 */

#ifdef _OS_GET_HEAP_STATISTICS
 /**
  * \brief Get heap statistics
  *
  * Returns a OS_HEAP_STATISTICS_TYPE structure filled with information about the current
  * heap state.
  *
  * \param [in, out] results_pointer Pointer to memory location where heap statistics data
  *                                  will be stored.
  */
#define OS_GET_HEAP_STATISTICS(results_pointer) _OS_GET_HEAP_STATISTICS(results_pointer)
#endif /* _OS_GET_HEAP_STATISTICS */

/* ****************************************************** */
/* The following macro functions are called by the system */
/* ****************************************************** */

#if !defined(OS_SYS_PRE_STOP_PROCESSING) && defined(_OS_SYS_PRE_STOP_PROCESSING)
/**
 * \brief Perform processing prior to system stopping (e.g. entering hibernation)
 *
 * Function performs processing prior to system stopping (e.g. entering hibernation).
 * This function is called only by the system.
 *
 * \note If OS_SYS_PRE_STOP_PROCESSING is already defined, it is application responsibility to
 * call also _OS_SYS_PRE_STOP_PROCESSING function since it may be needed by the OS.
 */
#define OS_SYS_PRE_STOP_PROCESSING() _OS_SYS_PRE_STOP_PROCESSING()
#endif /* OS_SYS_PRE_STOP_PROCESSING && _OS_SYS_PRE_STOP_PROCESSING */

#if !defined(OS_SYS_PRE_SLEEP_PROCESSING) && defined(_OS_SYS_PRE_SLEEP_PROCESSING)
/**
 * \brief Perform processing prior to system entering sleep
 *
 * Function performs processing prior to system entering sleep.
 * This function is called only by the system.
 *
 * \param [in] sleep_period sleep period in cycles of source clock used for the OS timer
 *
 * \note If OS_SYS_PRE_SLEEP_PROCESSING is already defined, it is application responsibility to
 * call also _OS_SYS_PRE_SLEEP_PROCESSING function since it may be needed by the OS.
 *
 * \note Adding application code by defining OS_SYS_PRE_SLEEP_PROCESSING() will have an impact
 * on minimum sleep time, as implied by dg_configMIN_SLEEP_TIME.
 *
 * \sa dg_configMIN_SLEEP_TIME
 */
#define OS_SYS_PRE_SLEEP_PROCESSING(sleep_period) _OS_SYS_PRE_SLEEP_PROCESSING(sleep_period)
#endif /* OS_SYS_PRE_SLEEP_PROCESSING && _OS_SYS_PRE_SLEEP_PROCESSING */

#if !defined(OS_SYS_POST_SLEEP_PROCESSING) && defined(_OS_SYS_POST_SLEEP_PROCESSING)
/**
 * \brief Perform processing after system waking-up
 *
 * Function performs processing after system waking-up.
 * This function is called only by the system.
 *
 * \note If OS_SYS_POST_SLEEP_PROCESSING is already defined, it is application responsibility to
 * call also _OS_SYS_POST_SLEEP_PROCESSING function since it may be needed by the OS.
 */
#define OS_SYS_POST_SLEEP_PROCESSING() _OS_SYS_POST_SLEEP_PROCESSING()
#endif /* OS_SYS_POST_SLEEP_PROCESSING && _OS_SYS_POST_SLEEP_PROCESSING */

#if !defined(OS_SYS_PRE_IDLE_PROCESSING) && defined(_OS_SYS_PRE_IDLE_PROCESSING)
/**
 * \brief Perform processing prior to system entering idle state
 *
 * Function performs processing prior to system entering idle state.
 * This function is called only by the system.
 *
 * \param [in] sleep_period sleep period in cycles of source clock used for the OS timer
 *
 * \note If OS_SYS_PRE_IDLE_PROCESSING is already defined, it is application responsibility to
 * call also _OS_SYS_PRE_IDLE_PROCESSING function since it may be needed by the OS.
 */
#define OS_SYS_PRE_IDLE_PROCESSING(sleep_period) _OS_SYS_PRE_IDLE_PROCESSING(sleep_period)
#endif /* OS_SYS_PRE_IDLE_PROCESSING && _OS_SYS_PRE_IDLE_PROCESSING */

#if !defined(OS_SYS_POST_IDLE_PROCESSING) && defined(_OS_SYS_POST_IDLE_PROCESSING)
/**
 * \brief Perform processing after system exiting idle state
 *
 * Function performs processing after system exiting idle state.
 * This function is called only by the system.
 *
 * \param [in] sleep_period sleep period in cycles of source clock used for the OS timer
 *
 * \note If OS_SYS_POST_IDLE_PROCESSING is already defined, it is application responsibility to
 * call also _OS_SYS_POST_IDLE_PROCESSING function since it may be needed by the OS.
 */
#define OS_SYS_POST_IDLE_PROCESSING(sleep_period) _OS_SYS_POST_IDLE_PROCESSING(sleep_period)
#endif /* OS_SYS_POST_IDLE_PROCESSING && _OS_SYS_POST_IDLE_PROCESSING */

/**
 * \brief Hook function to handle memory allocation failures
 *
 * This is a hook function that is called only if a call to OS_MALLOC() fails.
 * OS_GET_FREE_HEAP_SIZE() function can be used to query the size of free
 * heap space that remains.
 *
 * \sa OS_MALLOC
 * \sa OS_GET_FREE_HEAP_SIZE
 */
#ifdef _OS_APP_MALLOC_FAILED
#define OS_APP_MALLOC_FAILED(...) _OS_APP_MALLOC_FAILED(__VA_ARGS__)
#else
#define OS_APP_MALLOC_FAILED(...) void dummyMallocFailedHook(__VA_ARGS__) __UNUSED; \
                                  void dummyMallocFailedHook(__VA_ARGS__)
#endif /* _OS_APP_MALLOC_FAILED */

/**
 * \brief Hook function to be called on each iteration of the idle OS task
 *
 * This is a hook function that is called on each iteration of the idle OS
 * task. It is essential that code added to this hook function never attempts
 * to block in any way (for example, call OS_QUEUE_GET() with a block time
 * specified, or call OS_DELAY()). If the application makes use of the
 * OS_TASK_DELETE() API function then it is also important that
 * OS_APP_IDLE() is permitted to return to its calling function,
 * because it is the responsibility of the idle OS task to clean up memory
 * allocated by the kernel to any task that has since been deleted.
 *
 */
#ifdef _OS_APP_IDLE
#define OS_APP_IDLE(...) _OS_APP_IDLE(__VA_ARGS__)
#else
#define OS_APP_IDLE(...) void dummyIdleHook(__VA_ARGS__) __UNUSED; \
                         void dummyIdleHook(__VA_ARGS__)
#endif /* _OS_APP_IDLE */

/**
 * \brief Hook function to be called upon stack overflow
 *
 * This is a hook function that is called if a stack overflow is detected.
 */
#ifdef _OS_APP_STACK_OVERFLOW
#define OS_APP_STACK_OVERFLOW(...) _OS_APP_STACK_OVERFLOW(__VA_ARGS__)
#else
#define OS_APP_STACK_OVERFLOW(...) void dummyStackOverflowHook(__VA_ARGS__) __UNUSED; \
                                   void dummyStackOverflowHook(__VA_ARGS__)
#endif /* _OS_APP_STACK_OVERFLOW */

/**
 * \brief Hook function to be called on every OS tick
 *
 * This is a hook function that is called by each tick interrupt,
 * i.e. from an interrupt context. It is essential that code added to this
 * hook function never attempts to block in any way (for example, call OS_DELAY())
 * and only the interrupt-safe OSAL API functions can be used.
 *
 */
#ifdef _OS_APP_TICK
#define OS_APP_TICK(...) _OS_APP_TICK(__VA_ARGS__)
#else
#define OS_APP_TICK(...) void dummyTickHook(__VA_ARGS__) __UNUSED; \
                         void dummyTickHook(__VA_ARGS__)
#endif /* _OS_APP_TICK */

/**
 * \brief Hook function to be called at the point the daemon OS task starts executing
 *
 * This is a hook function that is called at the point the daemon OS task starts executing.
 *
 */
#ifdef _OS_APP_DAEMON_TASK
#define OS_APP_DAEMON_TASK(...) _OS_APP_DAEMON_TASK(__VA_ARGS__)
#else
#define OS_APP_DAEMON_TASK(...) void dummyDaemonTaskHook(__VA_ARGS__) __UNUSED; \
                                void dummyDaemonTaskHook(__VA_ARGS__)
#endif /* _OS_APP_DAEMON_TASK */

/* *************************************************************** */
/* The following macro functions are used internally by the system */
/* *************************************************************** */

/**
 * \brief Advance OS tick count
 *
 * Function advances OS tick count, updates the list of blocked OS tasks waiting
 * for a specific time period to pass, and triggers context switch, accordingly.
 *
 * This function is intended for internal use only.
 */
#define OS_TICK_ADVANCE() _OS_TICK_ADVANCE()

/**
 * \brief Update OS tick count by adding a given number of OS ticks
 *
 * Function updates OS tick count by adding a given number of OS ticks after a period during which
 * the system was in sleep or power down mode.
 *
 * This function is intended for internal use only.
 */
#define OS_TICK_INCREMENT(ticks) _OS_TICK_INCREMENT(ticks)

/*
 * CHECKS DEFINITIONS OF MANDATORY OSAL MACRO FUNCTIONS
 *****************************************************************************************
 */


/* *************** Mandatory OSAL configuration forward macros *************** */

/* Enable use of low power tickless mode */
#ifndef _OS_USE_TICKLESS_IDLE
#error "OS_USE_TICKLESS_IDLE not defined"
#endif

/* Total size of heap memory available for the OS */
#ifndef _OS_TOTAL_HEAP_SIZE
#error "OS_TOTAL_HEAP_SIZE not defined"
#endif

/* Word size used for the items stored to the stack */
#ifndef _OS_STACK_WORD_SIZE
#error "OS_STACK_WORD_SIZE not defined"
#endif

/* Minimal stack size (in bytes) defined for an OS task */
#ifndef _OS_MINIMAL_TASK_STACK_SIZE
#error "OS_MINIMAL_TASK_STACK_SIZE not defined"
#endif

/* Priority of timer daemon OS task */
#ifndef _OS_DAEMON_TASK_PRIORITY
#error "OS_DAEMON_TASK_PRIORITY not defined"
#endif

/* ********* Mandatory OSAL data type and enumeration forward macros ********* */

/* OS task priority values */

#ifndef _OS_TASK_PRIORITY_LOWEST
#error "OS_TASK_PRIORITY_LOWEST not defined"
#endif

#ifndef _OS_TASK_PRIORITY_NORMAL
#error "OS_TASK_PRIORITY_NORMAL not defined"
#endif

#ifndef _OS_TASK_PRIORITY_HIGHEST
#error "OS_TASK_PRIORITY_HIGHEST not defined"
#endif

/* Data types and enumerations for OS tasks and functions that operate on them */

#ifndef _OS_TASK
#error "OS_TASK not defined"
#endif

#ifndef _OS_TASK_CREATE_SUCCESS
#error "OS_TASK_CREATE_SUCCESS not defined"
#endif

#ifndef _OS_TASK_NOTIFY_SUCCESS
#error "OS_TASK_NOTIFY_SUCCESS not defined"
#endif

#ifndef _OS_TASK_NOTIFY_FAIL
#error "OS_TASK_NOTIFY_FAIL not defined"
#endif

#ifndef _OS_TASK_NOTIFY_NO_WAIT
#error "OS_TASK_NOTIFY_NO_WAIT not defined"
#endif

#ifndef _OS_TASK_NOTIFY_FOREVER
#error "OS_TASK_NOTIFY_FOREVER not defined"
#endif

#ifndef _OS_TASK_NOTIFY_NONE
#error "OS_TASK_NOTIFY_NONE not defined"
#endif

#ifndef _OS_TASK_NOTIFY_ALL_BITS
#error "OS_TASK_NOTIFY_ALL_BITS not defined"
#endif

/* Data types and enumerations for OS mutexes and functions that operate on them */

#ifndef _OS_MUTEX
#error "OS_MUTEX not defined"
#endif

#ifndef _OS_MUTEX_CREATE_SUCCESS
#error "OS_MUTEX_CREATE_SUCCESS not defined"
#endif

#ifndef _OS_MUTEX_CREATE_FAIL
#error "OS_MUTEX_CREATE_FAIL not defined"
#endif

#ifndef _OS_MUTEX_TAKEN
#error "OS_MUTEX_TAKEN not defined"
#endif

#ifndef _OS_MUTEX_NOT_TAKEN
#error "OS_MUTEX_NOT_TAKEN not defined"
#endif

#ifndef _OS_MUTEX_NO_WAIT
#error "OS_MUTEX_NO_WAIT not defined"
#endif

#ifndef _OS_MUTEX_FOREVER
#error "OS_MUTEX_FOREVER not defined"
#endif

/* Data types and enumerations for OS events and functions that operate on them */

#ifndef _OS_EVENT
#error "OS_EVENT not defined"
#endif

#ifndef _OS_EVENT_CREATE_SUCCESS
#error "OS_EVENT_CREATE_SUCCESS not defined"
#endif

#ifndef _OS_EVENT_CREATE_FAIL
#error "OS_EVENT_CREATE_FAIL not defined"
#endif

#ifndef _OS_EVENT_SIGNALED
#error "OS_EVENT_SIGNALED not defined"
#endif

#ifndef _OS_EVENT_NOT_SIGNALED
#error "OS_EVENT_NOT_SIGNALED not defined"
#endif

#ifndef _OS_EVENT_NO_WAIT
#error "OS_EVENT_NO_WAIT not defined"
#endif

#ifndef _OS_EVENT_FOREVER
#error "OS_EVENT_FOREVER not defined"
#endif

/* Data types and enumerations for OS event groups and functions that operate on them */

#ifndef _OS_EVENT_GROUP
#error "OS_EVENT_GROUP not defined"
#endif

#ifndef _OS_EVENT_GROUP_OK
#error "OS_EVENT_GROUP_OK not defined"
#endif

#ifndef _OS_EVENT_GROUP_FAIL
#error "OS_EVENT_GROUP_FAIL not defined"
#endif

#ifndef _OS_EVENT_GROUP_NO_WAIT
#error "OS_EVENT_GROUP_NO_WAIT not defined"
#endif

#ifndef _OS_EVENT_GROUP_FOREVER
#error "OS_EVENT_GROUP_FOREVER not defined"
#endif

/* Data types and enumerations for OS queues and functions that operate on them */

#ifndef _OS_QUEUE
#error "OS_QUEUE not defined"
#endif

#ifndef _OS_QUEUE_OK
#error "OS_QUEUE_OK not defined"
#endif

#ifndef _OS_QUEUE_FULL
#error "OS_QUEUE_FULL not defined"
#endif

#ifndef _OS_QUEUE_EMPTY
#error "OS_QUEUE_EMPTY not defined"
#endif

#ifndef _OS_QUEUE_NO_WAIT
#error "OS_QUEUE_NO_WAIT not defined"
#endif

#ifndef _OS_QUEUE_FOREVER
#error "OS_QUEUE_FOREVER not defined"
#endif

/* Data types and enumerations for OS timers and functions that operate on them */

#ifndef _OS_TIMER
#error "OS_TIMER not defined"
#endif

#ifndef _OS_TIMER_SUCCESS
#error "OS_TIMER_SUCCESS not defined"
#endif

#ifndef _OS_TIMER_FAIL
#error "OS_TIMER_FAIL not defined"
#endif

#ifndef _OS_TIMER_RELOAD
#error "OS_TIMER_RELOAD not defined"
#endif

#ifndef _OS_TIMER_ONCE
#error "OS_TIMER_ONCE not defined"
#endif

#ifndef _OS_TIMER_NO_WAIT
#error "OS_TIMER_NO_WAIT not defined"
#endif

#ifndef _OS_TIMER_FOREVER
#error "OS_TIMER_FOREVER not defined"
#endif

/* Base data types matching underlying architecture */

#ifndef _OS_BASE_TYPE
#error "OS_BASE_TYPE not defined"
#endif

#ifndef _OS_UBASE_TYPE
#error "OS_UBASE_TYPE not defined"
#endif

/* Enumeration values indicating successful or not OS operation */

#ifndef _OS_OK
#error "OS_OK not defined"
#endif

#ifndef _OS_FAIL
#error "OS_FAIL not defined"
#endif

/* Boolean enumeration values */

#ifndef _OS_TRUE
#error "OS_TRUE not defined"
#endif

#ifndef _OS_FALSE
#error "OS_FALSE not defined"
#endif

/* Maximum OS delay (in OS ticks) */
#ifndef _OS_MAX_DELAY
#error "OS_MAX_DELAY not defined"
#endif

/* OS tick time (i.e. time expressed in OS ticks) data type */
#ifndef _OS_TICK_TIME
#error "OS_TICK_TIME not defined"
#endif

/* OS tick period (in cycles of source clock used for the OS timer) */
#ifndef _OS_TICK_PERIOD
#error "OS_TICK_PERIOD not defined"
#endif

/* OS tick period (in msec) */
#ifndef _OS_TICK_PERIOD_MS
#error "OS_TICK_PERIOD_MS not defined"
#endif

/* Frequency (in Hz) of the source clock used for the OS timer */
#ifndef _OS_TICK_CLOCK_HZ
#error "OS_TICK_CLOCK_HZ not defined"
#endif

/* Data type of OS task function (i.e. OS_TASK_FUNCTION) argument */
#ifndef _OS_TASK_ARG_TYPE
#error "OS_TASK_ARG_TYPE not defined"
#endif

/* *********************** Mandatory OSAL enumerations *********************** */

/* OS task notification action */

#ifndef _OS_NOTIFY_NO_ACTION
#error "OS_NOTIFY_NO_ACTION not defined"
#endif

#ifndef _OS_NOTIFY_SET_BITS
#error "OS_NOTIFY_SET_BITS not defined"
#endif

#ifndef _OS_NOTIFY_INCREMENT
#error "OS_NOTIFY_INCREMENT not defined"
#endif

#ifndef _OS_NOTIFY_VAL_WITH_OVERWRITE
#error "OS_NOTIFY_VAL_WITH_OVERWRITE not defined"
#endif

#ifndef _OS_NOTIFY_VAL_WITHOUT_OVERWRITE
#error "OS_NOTIFY_VAL_WITHOUT_OVERWRITE not defined"
#endif

/* OS task state */

#ifndef _OS_TASK_RUNNING
#error "OS_TASK_RUNNING not defined"
#endif

#ifndef _OS_TASK_READY
#error "OS_TASK_READY not defined"
#endif

#ifndef _OS_TASK_BLOCKED
#error "OS_TASK_BLOCKED not defined"
#endif

#ifndef _OS_TASK_SUSPENDED
#error "OS_TASK_SUSPENDED not defined"
#endif

#ifndef _OS_TASK_DELETED
#error "OS_TASK_DELETED not defined"
#endif

/* OS scheduler state */

#ifndef _OS_SCHEDULER_RUNNING
#error "OS_SCHEDULER_RUNNING not defined"
#endif

#ifndef _OS_SCHEDULER_NOT_STARTED
#error "OS_SCHEDULER_NOT_STARTED not defined"
#endif

#ifndef _OS_SCHEDULER_SUSPENDED
#error "OS_SCHEDULER_SUSPENDED not defined"
#endif

/* **************** Mandatory OSAL macro function definitions **************** */

/* Declare an OS task function */
#ifndef _OS_TASK_FUNCTION
#error "OS_TASK_FUNCTION not defined"
#endif

/* Run the OS task scheduler */
#ifndef _OS_TASK_SCHEDULER_RUN
#error "OS_TASK_SCHEDULER_RUN not defined"
#endif

/* Convert a time in milliseconds to a time in OS ticks */
#ifndef _OS_TIME_TO_TICKS
#error "OS_TIME_TO_TICKS not defined"
#endif

/* Return current OS task handle */
#ifndef _OS_GET_CURRENT_TASK
#error "OS_GET_CURRENT_TASK not defined"
#endif

/* Create OS task */
#ifndef _OS_TASK_CREATE
#error "OS_TASK_CREATE not defined"
#endif

/* Delete OS task */
#ifndef _OS_TASK_DELETE
#error "OS_TASK_DELETE not defined"
#endif

/* Get the priority of an OS task */
#ifndef _OS_TASK_PRIORITY_GET
#error "OS_TASK_PRIORITY_GET not defined"
#endif

/* Get the priority of an OS task from ISR */
#ifndef _OS_TASK_PRIORITY_GET_FROM_ISR
#error "OS_TASK_PRIORITY_GET_FROM_ISR not defined"
#endif

/* Set the priority of an OS task */
#ifndef _OS_TASK_PRIORITY_SET
#error "OS_TASK_PRIORITY_SET not defined"
#endif

/* The running OS task yields control to the scheduler */
#ifndef _OS_TASK_YIELD
#error "OS_TASK_YIELD not defined"
#endif

/* The running OS task yields control to the scheduler from ISR */
#ifndef _OS_TASK_YIELD_FROM_ISR
#error "OS_TASK_YIELD_FROM_ISR not defined"
#endif

/* Send notification to OS task, updating its notification value */
#ifndef _OS_TASK_NOTIFY
#error "OS_TASK_NOTIFY not defined"
#endif

/* Send notification to OS task, updating its notification value and returning previous value */
#ifndef _OS_TASK_NOTIFY_AND_QUERY
#error "OS_TASK_NOTIFY_AND_QUERY not defined"
#endif

/* Send notification to OS task from ISR, updating its notification value */
#ifndef _OS_TASK_NOTIFY_FROM_ISR
#error "OS_TASK_NOTIFY_FROM_ISR not defined"
#endif

/* Send notification to OS task from ISR, updating its notification value and returning
 * previous value */
#ifndef _OS_TASK_NOTIFY_AND_QUERY_FROM_ISR
#error "OS_TASK_NOTIFY_AND_QUERY_FROM_ISR not defined"
#endif

/* Send a notification event to OS task, incrementing its notification value */
#ifndef _OS_TASK_NOTIFY_GIVE
#error "OS_TASK_NOTIFY_GIVE not defined"
#endif

/* Send a notification event to OS task from ISR, incrementing its notification value */
#ifndef _OS_TASK_NOTIFY_GIVE_FROM_ISR
#error "OS_TASK_NOTIFY_GIVE_FROM_ISR not defined"
#endif

/* Wait for the calling OS task to receive a notification event, clearing to zero or
 * decrementing task notification value on exit */
#ifndef _OS_TASK_NOTIFY_TAKE
#error "OS_TASK_NOTIFY_TAKE not defined"
#endif

/* Wait for the calling OS task to receive a notification, updating task notification value
 * on exit */
#ifndef _OS_TASK_NOTIFY_WAIT
#error "OS_TASK_NOTIFY_WAIT not defined"
#endif

/* Resume OS task */
#ifndef _OS_TASK_RESUME
#error "OS_TASK_RESUME not defined"
#endif

/* Resume OS task from ISR */
#ifndef _OS_TASK_RESUME_FROM_ISR
#error "OS_TASK_RESUME_FROM_ISR not defined"
#endif

/* Suspend OS task */
#ifndef _OS_TASK_SUSPEND
#error "OS_TASK_SUSPEND not defined"
#endif

/* Create OS mutex */
#ifndef _OS_MUTEX_CREATE
#error "OS_MUTEX_CREATE not defined"
#endif

/* Delete OS mutex */
#ifndef _OS_MUTEX_DELETE
#error "OS_MUTEX_DELETE not defined"
#endif

/* Release OS mutex */
#ifndef _OS_MUTEX_PUT
#error "OS_MUTEX_PUT not defined"
#endif

/* Acquire OS mutex */
#ifndef _OS_MUTEX_GET
#error "OS_MUTEX_GET not defined"
#endif

/* Get OS task owner of OS mutex */
#ifndef _OS_MUTEX_GET_OWNER
#error "OS_MUTEX_GET_OWNER not defined"
#endif

/* Get OS task owner of OS mutex from ISR */
#ifndef _OS_MUTEX_GET_OWNER_FROM_ISR
#error "OS_MUTEX_GET_OWNER_FROM_ISR not defined"
#endif

/* Get OS mutex current count value */
#ifndef _OS_MUTEX_GET_COUNT
#error "OS_MUTEX_GET_COUNT not defined"
#endif

/* Get OS mutex current count value from ISR */
#ifndef _OS_MUTEX_GET_COUNT_FROM_ISR
#error "OS_MUTEX_GET_COUNT_FROM_ISR not defined"
#endif

/* Create OS event */
#ifndef _OS_EVENT_CREATE
#error "OS_EVENT_CREATE not defined"
#endif

/* Delete OS event */
#ifndef _OS_EVENT_DELETE
#error "OS_EVENT_DELETE not defined"
#endif

/* Set OS event in signaled state */
#ifndef _OS_EVENT_SIGNAL
#error "OS_EVENT_SIGNAL not defined"
#endif

/* Set OS event in signaled state from ISR */
#ifndef _OS_EVENT_SIGNAL_FROM_ISR
#error "OS_EVENT_SIGNAL_FROM_ISR not defined"
#endif

/* Set OS event in signaled state from ISR without requesting running OS task to yield */
#ifndef _OS_EVENT_SIGNAL_FROM_ISR_NO_YIELD
#error "OS_EVENT_SIGNAL_FROM_ISR_NO_YIELD not defined"
#endif

/* Wait for OS event to be signaled */
#ifndef _OS_EVENT_WAIT
#error "OS_EVENT_WAIT not defined"
#endif

/* Check if OS event is signaled and clear it */
#ifndef _OS_EVENT_CHECK
#error "OS_EVENT_CHECK not defined"
#endif

/* Check from ISR if OS event is signaled and clear it */
#ifndef _OS_EVENT_CHECK_FROM_ISR
#error "OS_EVENT_CHECK_FROM_ISR not defined"
#endif

/* Check from ISR if OS event is signaled and clear it, without requesting
 * running OS task to yield */
#ifndef _OS_EVENT_CHECK_FROM_ISR_NO_YIELD
#error "OS_EVENT_CHECK_FROM_ISR_NO_YIELD not defined"
#endif

/* Get OS event status */
#ifndef _OS_EVENT_GET_STATUS
#error "OS_EVENT_GET_STATUS not defined"
#endif

/* Get OS event status from ISR */
#ifndef _OS_EVENT_GET_STATUS_FROM_ISR
#error "OS_EVENT_GET_STATUS_FROM_ISR not defined"
#endif

/* Create OS event group */
#ifndef _OS_EVENT_GROUP_CREATE
#error "OS_EVENT_GROUP_CREATE not defined"
#endif

/* Wait for OS event group bits to become set */
#ifndef _OS_EVENT_GROUP_WAIT_BITS
#error "OS_EVENT_GROUP_WAIT_BITS not defined"
#endif

/* Set OS event group bits */
#ifndef _OS_EVENT_GROUP_SET_BITS
#error "OS_EVENT_GROUP_SET_BITS not defined"
#endif

/* Set OS event group bits from ISR */
#ifndef _OS_EVENT_GROUP_SET_BITS_FROM_ISR
#error "OS_EVENT_GROUP_SET_BITS_FROM_ISR not defined"
#endif

/* Set OS event group bits from ISR without requesting running OS task to yield */
#ifndef _OS_EVENT_GROUP_SET_BITS_FROM_ISR_NO_YIELD
#error "OS_EVENT_GROUP_SET_BITS_FROM_ISR_NO_YIELD not defined"
#endif

/* Clear OS event group bits */
#ifndef _OS_EVENT_GROUP_CLEAR_BITS
#error "OS_EVENT_GROUP_CLEAR_BITS not defined"
#endif

/* Clear OS event group bits from an interrupt */
#ifndef _OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR
#error "OS_EVENT_GROUP_CLEAR_BITS_FROM_ISR not defined"
#endif

/* Get OS event group bits */
#ifndef _OS_EVENT_GROUP_GET_BITS
#error "OS_EVENT_GROUP_GET_BITS not defined"
#endif

/* Get OS event group bits from an interrupt */
#ifndef _OS_EVENT_GROUP_GET_BITS_FROM_ISR
#error "OS_EVENT_GROUP_GET_BITS_FROM_ISR not defined"
#endif

/* Synchronize OS event group bits */
#ifndef _OS_EVENT_GROUP_SYNC
#error "OS_EVENT_GROUP_SYNC not defined"
#endif

/* Delete OS event group */
#ifndef _OS_EVENT_GROUP_DELETE
#error "OS_EVENT_GROUP_DELETE not defined"
#endif

/* Create OS queue */
#ifndef _OS_QUEUE_CREATE
#error "OS_QUEUE_CREATE not defined"
#endif

/* Deletes OS queue */
#ifndef _OS_QUEUE_DELETE
#error "OS_QUEUE_DELETE not defined"
#endif

/* Put element in OS queue */
#ifndef _OS_QUEUE_PUT
#error "OS_QUEUE_PUT not defined"
#endif

/* Put element in OS queue */
#ifndef _OS_QUEUE_PUT_FROM_ISR
#error "OS_QUEUE_PUT_FROM_ISR not defined"
#endif

/* Replace element in OS queue of one element */
#ifndef _OS_QUEUE_REPLACE
#error "OS_QUEUE_REPLACE not defined"
#endif

/* Replace element in OS queue of one element from ISR */
#ifndef _OS_QUEUE_REPLACE_FROM_ISR
#error "OS_QUEUE_REPLACE_FROM_ISR not defined"
#endif

/* Replace element in OS queue of one element from ISR without requesting running OS task to yield */
#ifndef _OS_QUEUE_REPLACE_FROM_ISR_NO_YIELD
#error "OS_QUEUE_REPLACE_FROM_ISR_NO_YIELD not defined"
#endif

/* Get element from OS queue */
#ifndef _OS_QUEUE_GET
#error "OS_QUEUE_GET not defined"
#endif

/* Get element from OS queue from ISR */
#ifndef _OS_QUEUE_GET_FROM_ISR
#error "OS_QUEUE_GET_FROM_ISR not defined"
#endif

/* Get element from OS queue from ISR without requesting running OS task to yield */
#ifndef _OS_QUEUE_GET_FROM_ISR_NO_YIELD
#error "OS_QUEUE_GET_FROM_ISR_NO_YIELD not defined"
#endif

/* Peek element from OS queue */
#ifndef _OS_QUEUE_PEEK
#error "OS_QUEUE_PEEK not defined"
#endif

/* Peek element from OS queue from ISR */
#ifndef _OS_QUEUE_PEEK_FROM_ISR
#error "OS_QUEUE_PEEK_FROM_ISR not defined"
#endif

/* Get the number of messages stored in OS queue */
#ifndef _OS_QUEUE_MESSAGES_WAITING
#error "OS_QUEUE_MESSAGES_WAITING not defined"
#endif

/* Get the number of messages stored in OS queue from ISR */
#ifndef _OS_QUEUE_MESSAGES_WAITING_FROM_ISR
#error "OS_QUEUE_MESSAGES_WAITING_FROM_ISR not defined"
#endif

/* Get the number of free spaces in OS queue */
#ifndef _OS_QUEUE_SPACES_AVAILABLE
#error "OS_QUEUE_SPACES_AVAILABLE not defined"
#endif

/* Create OS timer */
#ifndef _OS_TIMER_CREATE
#error "OS_TIMER_CREATE not defined"
#endif

/* Get OS timer ID */
#ifndef _OS_TIMER_GET_TIMER_ID
#error "OS_TIMER_GET_TIMER_ID not defined"
#endif

/* Check if OS timer is active */
#ifndef _OS_TIMER_IS_ACTIVE
#error "OS_TIMER_IS_ACTIVE not defined"
#endif

/* Start OS timer */
#ifndef _OS_TIMER_START
#error "OS_TIMER_START not defined"
#endif

/* Stop OS timer */
#ifndef _OS_TIMER_STOP
#error "OS_TIMER_STOP not defined"
#endif

/* Change OS timer's period */
#ifndef _OS_TIMER_CHANGE_PERIOD
#error "OS_TIMER_CHANGE_PERIOD not defined"
#endif

/* Delete OS timer */
#ifndef _OS_TIMER_DELETE
#error "OS_TIMER_DELETE not defined"
#endif

/* Reset OS timer */
#ifndef _OS_TIMER_RESET
#error "OS_TIMER_RESET not defined"
#endif

/* Start OS timer from ISR */
#ifndef _OS_TIMER_START_FROM_ISR
#error "OS_TIMER_START_FROM_ISR not defined"
#endif

/* Stop OS timer from ISR */
#ifndef _OS_TIMER_STOP_FROM_ISR
#error "OS_TIMER_STOP_FROM_ISR not defined"
#endif

/* Change OS timer period from ISR */
#ifndef _OS_TIMER_CHANGE_PERIOD_FROM_ISR
#error "OS_TIMER_CHANGE_PERIOD_FROM_ISR not defined"
#endif

/* Reset OS timer from ISR */
#ifndef _OS_TIMER_RESET_FROM_ISR
#error "OS_TIMER_RESET_FROM_ISR not defined"
#endif

/* Delay execution of OS task for specified time */
#ifndef _OS_DELAY
#error "OS_DELAY not defined"
#endif

/* Delay execution of OS task until specified time */
#ifndef _OS_DELAY_UNTIL
#error "OS_DELAY_UNTIL not defined"
#endif

/* Get current OS tick count */
#ifndef _OS_GET_TICK_COUNT
#error "OS_GET_TICK_COUNT not defined"
#endif

/* Get current OS tick count from ISR */
#ifndef _OS_GET_TICK_COUNT_FROM_ISR
#error "OS_GET_TICK_COUNT_FROM_ISR not defined"
#endif

/* Convert from OS ticks to ms */
#ifndef _OS_TICKS_2_MS
#error "OS_TICKS_2_MS not defined"
#endif

/* Convert from ms to OS ticks */
#ifndef _OS_MS_2_TICKS
#error "OS_MS_2_TICKS not defined"
#endif

/* Delay execution of OS task for specified time */
#ifndef _OS_DELAY_MS
#error "OS_DELAY_MS not defined"
#endif

/* Enter critical section from non-ISR context */
#ifndef _OS_ENTER_CRITICAL_SECTION
#error "OS_ENTER_CRITICAL_SECTION not defined"
#endif

/* Enter critical section from ISR context */
#ifndef _OS_ENTER_CRITICAL_SECTION_FROM_ISR
#error "OS_ENTER_CRITICAL_SECTION_FROM_ISR not defined"
#endif

/* Leave critical section from non-ISR context */
#ifndef _OS_LEAVE_CRITICAL_SECTION
#error "OS_LEAVE_CRITICAL_SECTION not defined"
#endif

/* Leave critical section from ISR context */
#ifndef _OS_LEAVE_CRITICAL_SECTION_FROM_ISR
#error "OS_LEAVE_CRITICAL_SECTION_FROM_ISR not defined"
#endif

/* Name for OS memory allocation function */
#ifndef _OS_MALLOC_FUNC
#error "OS_MALLOC_FUNC not defined"
#endif

/* Name for non-retain memory allocation function */
#ifndef _OS_MALLOC_NORET_FUNC
#error "OS_MALLOC_NORET_FUNC not defined"
#endif

/* Allocate memory from OS provided heap */
#ifndef _OS_MALLOC
#error "OS_MALLOC not defined"
#endif

/* Allocate memory from non-retain heap */
#ifndef _OS_MALLOC_NORET
#error "OS_MALLOC_NORET not defined"
#endif

/* Name for OS free memory function */
#ifndef _OS_FREE_FUNC
#error "OS_FREE_FUNC not defined"
#endif

/* Name for non-retain memory free function */
#ifndef _OS_FREE_NORET_FUNC
#error "OS_FREE_NORET_FUNC not defined"
#endif

/* Free memory allocated by OS_MALLOC() */
#ifndef _OS_FREE
#error "OS_FREE not defined"
#endif

/* Free memory allocated by OS_MALLOC_NORET() */
#ifndef _OS_FREE_NORET
#error "OS_FREE_NORET not defined"
#endif

/* Get the high water mark of heap */
#ifndef _OS_GET_HEAP_WATERMARK
#error "OS_GET_HEAP_WATERMARK not defined"
#endif

/* Get current free heap size */
#ifndef _OS_GET_FREE_HEAP_SIZE
#error "OS_GET_FREE_HEAP_SIZE not defined"
#endif

/* Get current number of OS tasks */
#ifndef _OS_GET_TASKS_NUMBER
#error "OS_GET_TASKS_NUMBER not defined"
#endif

/* Advance OS tick count */
#ifndef _OS_TICK_ADVANCE
#error "OS_TICK_ADVANCE not defined"
#endif

/* Update OS tick count by adding a given number of OS ticks */
#ifndef _OS_TICK_INCREMENT
#error "OS_TICK_INCREMENT not defined"
#endif

#else

#include <stdlib.h>
/*
 * Basic set of macros that can be used in non OS environment.
 */
#define PRIVILEGED_DATA
#define OS_MALLOC               malloc
#define OS_FREE                 free
#ifndef RELEASE_BUILD
#define OS_ASSERT(a)            do { if (!(a)) {__BKPT(0);} } while (0)
#else
#define OS_ASSERT(a)            ((void) (a))
#endif

#endif /* OS_PRESENT */

/**
 * \brief Cast any pointer to unsigned int value
 */
#define OS_PTR_TO_UINT(p) ((unsigned) (void *) (p))

/**
 * \brief Cast any pointer to signed int value
 */
#define OS_PTR_TO_INT(p) ((int) (void *) (p))

/**
 * \brief Cast any unsigned int value to pointer
 */
#define OS_UINT_TO_PTR(u) ((void *) (unsigned) (u))

/**
 * \brief Cast any signed int value to pointer
 */
#define OS_INT_TO_PTR(i) ((void *) (int) (i))

#endif /* OSAL_H_ */

/**
 * \}
 * \}
 */
