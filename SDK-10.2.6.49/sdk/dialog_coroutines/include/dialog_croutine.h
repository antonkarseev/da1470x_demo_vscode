/**
 ****************************************************************************************
 *
 * @file dialog_croutine.h
 *
 * @brief Extensions to FreeRTOS croutine.h
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DIALOG_CO_ROUTINE_H_
#define DIALOG_CO_ROUTINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if (configUSE_DIALOG_CO_ROUTINES == 1)
/**
 * \brief Co-routine control block
 */
typedef struct corCoRoutineControlBlock
{
#if (configUSE_DG_COROUTINE_DEBUG_FACILITY == 1)
        /**
         * Points a the location required for thread aware debugging to be supported.
         * THIS MUST BE THE FIRST MEMBER OF THE CRCB STRUCT.
         */
        volatile StackType_t    *pxTopOfStack;
#endif /* configUSE_DG_COROUTINE_DEBUG_FACILITY */
        /** List item used to place the CRCB in ready and blocked queues. */
        ListItem_t              xGenericListItem;
        /** List item used to place the CRCB in event lists. */
        ListItem_t              xEventListItem;
        /** The priority of the co-routine in relation to other co-routines. */
        UBaseType_t             uxPriority;
        /** Used to distinguish between co-routines when multiple co-routines use the same co-routine function. */
        UBaseType_t             uxIndex;
#if (configMAX_DG_COROUTINE_NAME_LEN > 0)
        /** Descriptive name given to the co-routine when created.  Facilitates debugging only. */
        char                    pcCoRoutineName[configMAX_DG_COROUTINE_NAME_LEN];
#endif
        crCOROUTINE_CODE        pxCoRoutineFunction;    /**< Co-routine function. */
#if (configUSE_DG_COROUTINE_DEBUG_FACILITY == 1)
        portDGCOROUTINE_DEBUG_FACILITY_CRCB_INFO
#endif
#if (configRECORD_DG_COROUTINE_BLOCKED_PC == 1)
        /** Last program counter of co-routine before entering blocked state. */
        void                    *pxBlockedPC;
#endif
        /** Used internally by the co-routine implementation. */
        uint16_t                uxState;
#if (configUSE_DG_COROUTINE_NOTIFICATIONS == 1)
        volatile uint8_t        ucNotifyState;          /**< Co-routine's notification state. */
        volatile uint32_t       ulNotifiedValue;        /**< Co-routine's notification value. */
#endif
#if (configUSE_TRACE_FACILITY == 1)
        /**
         * Stores a number that increments each time a CRCB is created. It allows debuggers
         * to determine when a co-routine has been deleted and then recreated.
         */
        UBaseType_t             uxCRCBNumber;
        /** Stores a number specifically for use by third party trace code. */
        UBaseType_t             uxCoRoutineNumber;
#endif
#if (configUSE_MUTEXES == 1)
        /** The last priority assigned to the co-routine. */
        UBaseType_t             uxBasePriority;
        /** Number of mutexes held by the co-routine. */
        UBaseType_t             uxMutexesHeld;
#endif
#if (configGENERATE_RUN_TIME_STATS == 1)
        /**< Stores the amount of time the co-routine has spent in the Running state. */
        uint32_t                ulRunTimeCounter;
#endif
#if (configUSE_TRACE_FACILITY == 1) || (INCLUDE_uxDgCoRoutineGetStackHighWaterMark == 1) || (INCLUDE_pxDgCoRoutineGetStackStart == 1)
#if (configRECORD_STACK_HIGH_ADDRESS == 1) || (portSTACK_GROWTH < 0)
        /** Points to the highest valid address for the co-routine stack. */
        StackType_t             *pxEndOfStack;
#endif
#if (configUSE_TRACE_FACILITY == 1) || (portSTACK_GROWTH > 0)
        /** Points to the lowest valid address for the co-routine stack. */
        StackType_t             *pxStack;
#endif
#endif
#if (configUSE_TRACE_FACILITY == 1) || (INCLUDE_uxDgCoRoutineGetStackHighWaterMark == 1)
        /** Minimum amount of stack space remained for co-routine. */
        uint16_t                usStackHighWaterMark;
#endif
#if (configSUPPORT_STATIC_ALLOCATION == 1) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
        /** If pdTRUE, it indicates that the co-routine is statically allocated. */
        uint8_t                 ucStaticallyAllocated;
#endif
} CRCB_t; /* Co-routine control block.  Note must be identical in size down to uxPriority with TCB_t. */

#endif /* configUSE_DIALOG_CO_ROUTINES */

/**
 * \brief Co-routine states
 *
 * Co-routine state enumeration values returned by eDgCoRoutineGetState()
 *
 * \sa eDgCoRoutineGetState
 */
typedef enum
{
        eDgCrRunning = 0,               /**< A co-routine is querying the state of itself,
                                             so must be running */
        eDgCrReady,                     /**< The co-routine being queried is in a read or
                                             pending ready list */
        eDgCrBlocked,                   /**< The co-routine being queried is in the Blocked state */
        eDgCrDeleted,                   /**< The co-routine being queried has been deleted */
        eDgCrInvalid                    /**< Used as an 'invalid state' value */
} eDgCoRoutineState;

/**
 * \brief Co-routine notification actions
 *
 * Enumeration values for co-routine notification actions that can be performed when
 * vDgCoRoutineNotify() is called
 *
 * \sa vDgCoRoutineNotify
 */
typedef enum
{
        eDgCrNoAction = 0,              /**< Notify the co-routine without updating
                                             its notify value */
        eDgCrSetBits,                   /**< Set bits in the co-routine's notification value */
        eDgCrIncrement,                 /**< Increment the co-routine's notification value */
        eDgCrSetValueWithOverwrite,     /**< Set the co-routine's notification value to a
                                             specific value even if the previous value has not
                                             yet been read by the co-routine */
        eDgCrSetValueWithoutOverwrite   /**< Set the co-routine's notification value if the
                                             previous value has been read by the co-routine */
} eDgCoRoutineNotifyAction;

/**
 * \brief Execution status of a co-routine
 *
 * This structure is used with the uxDgCoRoutineGetSystemState() function in order to acquire the
 * execution status of co-routines in the system
 *
 * \sa uxDgCoRoutineGetSystemState
 */
typedef struct xDG_COROUTINE_STATUS
{
        CoRoutineHandle_t xHandle;              /**< The handle of the co-routine to which the rest
                                                     of the information in the structure relates */
        const char *pcCoRoutineName;            /**< A pointer to the co-routine's name  */
        UBaseType_t xCoRoutineNumber;           /**< A number unique to the co-routine */
        eDgCoRoutineState eCurrentState;        /**< The state in which the co-routine existed when
                                                     the structure was populated */
        UBaseType_t uxPriority;                 /**< The priority of the co-routine in relation to
                                                     other co-routines */
        UBaseType_t uxBasePriority;             /**< The priority to which the co-routine will
                                                     return in case co-routine's current priority
                                                     has been inherited when obtaining a mutex.
                                                     Only valid if configUSE_MUTEXES is set to 1
                                                     in FreeRTOSConfig.h. */
        uint32_t ulRunTimeCounter;              /**< The total run time allocated to the co-routine
                                                     so far, as defined by the run time stats clock.
                                                     See http://www.freertos.org/rtos-run-time-stats.html.
                                                     Only valid when configGENERATE_RUN_TIME_STATS
                                                     is defined as 1 in FreeRTOSConfig.h. */
        StackType_t *pxStackEnd;                /**< The highest address of the co-routine's stack
                                                     area when minimum amount of stack space was
                                                     recorded. */
        StackType_t *pxStackBase;               /**< The lowest address of the co-routine's stack
                                                     area when minimum amount of stack space was
                                                     recorded. */
        uint16_t usStackHighWaterMark;          /**< The minimum amount of stack space that has
                                                     remained for the co-routine since the
                                                     co-routine was created */
} DgCoRoutineStatus_t;

/**
 * \brief Sleep mode status
 *
 * Sleep mode status enumeration values returned by eDgCoRoutineConfirmSleepModeStatus()
 *
 * \sa eDgCoRoutineConfirmSleepModeStatus
 */
typedef enum
{
        eDgCrAbortSleep = 0,            /**< A co-routine has been made ready or triggered in an
                                             ISR since portSUPPRESS_TICKS_AND_SLEEP() was called
                                             - entering a sleep mode is aborted */
        eDgCrStandardSleep              /**< Sleep mode will not last any longer than the expected
                                             idle time */
} eDgCoRoutineSleepModeStatus;

/**
 * \brief Priority of the idle co-routine
 */
#define dgcrIDLE_PRIORITY               ((UBaseType_t) 0U)

/**
 * \brief Macro for performing a context switch
 *
 * This function is used for the calling co-routine to yield its execution and return execution
 * flow back to the scheduler only when another co-routine of higher priority is ready to run.
 * Use either dgcrDELAY() with zero ticks delay as argument or crSET_STATE0() when context switch
 * needs to be forced.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine yields its execution.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 *
 * \sa dgcrDELAY
 * \sa crSET_STATE0
 */
#define dgcrYIELD(xHandle) \
        do { \
                if (vDgCoRoutineIsPendingYield() != pdFALSE) { \
                        crSET_STATE0(xHandle); \
                } \
        } while (0)

/**
 * \brief Macro for indicating from ISR that a context switch has to be performed
 *
 * This function is used for indicating from interrupt service routine (ISR) that the current
 * co-routine has to yield its execution and return execution flow back to the scheduler.
 */
#define dgcrYIELD_FROM_ISR() \
        do { \
                vDgCoRoutineMissedYieldForPriority(((CRCB_t *)xDgCoRoutineGetCurrentCoRoutineHandle())->uxPriority + 1); \
        } while (0)

/**
 * \brief Macro to mark the start of a critical code region
 */
#define dgcrENTER_CRITICAL()            portENTER_CRITICAL()

/**
 * \brief Macro to mark the start of a critical code region in ISR context
 *
 * \return interrupt status
 */
#define dgcrENTER_CRITICAL_FROM_ISR()   portSET_INTERRUPT_MASK_FROM_ISR()

/**
 * \brief Macro to mark the end of a critical code region
 */
#define dgcrEXIT_CRITICAL()             portEXIT_CRITICAL()

/**
 * \brief Macro to mark the end of a critical code region in ISR context
 *
 * \param [in] uxInterruptStatus previously returned interrupt status by
 * dgcrENTER_CRITICAL_FROM_ISR()
 *
 * \sa dgcrENTER_CRITICAL_FROM_ISR
 */
#define dgcrEXIT_CRITICAL_FROM_ISR(uxInterruptStatus) \
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxInterruptStatus)

/**
 * \brief Macro to disable all maskable interrupts
 */
#define dgcrDISABLE_INTERRUPTS()        portDISABLE_INTERRUPTS()

/**
 * \brief Macro to enable all interrupts
 */
#define dgcrENABLE_INTERRUPTS()         portENABLE_INTERRUPTS()

/* Scheduler state */
#define dgcrSCHEDULER_SUSPENDED         ((BaseType_t) 0)        ///< Scheduler is in the suspended state
#define dgcrSCHEDULER_NOT_STARTED       ((BaseType_t) 1)        ///< Scheduler is not started
#define dgcrSCHEDULER_RUNNING           ((BaseType_t) 2)        ///< Scheduler is in running state

#if (configUSE_DIALOG_CO_ROUTINES == 1)
/*
 * These macros are intended for internal use by the co-routine implementation
 * only.  The macros should not be used directly by application writers.
 */
#if (configRECORD_DG_COROUTINE_BLOCKED_PC == 1)
#define crSET_STATE0(xHandle) \
        ((CRCB_t *)(xHandle))->pxBlockedPC = ((void *) portGET_PC()); \
        portDGCOROUTINE_DEBUG_FACILITY_UPDATE_INFO((CRCB_t *)(xHandle)); \
        ((CRCB_t *)(xHandle))->uxState = (__LINE__ * 2); return; case (__LINE__ * 2):
#define crSET_STATE1(xHandle) \
        ((CRCB_t *)(xHandle))->pxBlockedPC = ((void *) portGET_PC()); \
        portDGCOROUTINE_DEBUG_FACILITY_UPDATE_INFO((CRCB_t *)(xHandle)); \
        ((CRCB_t *)(xHandle))->uxState = ((__LINE__ * 2) + 1); return; case ((__LINE__ * 2) + 1):
#else
#define crSET_STATE0(xHandle) \
        portDGCOROUTINE_DEBUG_FACILITY_UPDATE_INFO((CRCB_t *)(xHandle)); \
        ((CRCB_t *)(xHandle))->uxState = (__LINE__ * 2); return; case (__LINE__ * 2):
#define crSET_STATE1(xHandle) \
        portDGCOROUTINE_DEBUG_FACILITY_UPDATE_INFO((CRCB_t *)(xHandle)); \
        ((CRCB_t *)(xHandle))->uxState = ((__LINE__ * 2) + 1); return; case ((__LINE__ * 2) + 1):
#endif /* configRECORD_DG_COROUTINE_BLOCKED_PC */

#endif /* configUSE_DIALOG_CO_ROUTINES */

/**
 * \brief Delay co-routine for a fixed period of time
 *
 * This function is used for the calling co-routine to delay for a fixed period of time,
 * as implied by xTicksToDelay. The calling co-routine yields its execution and is blocked for
 * the given period of time.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine yields its execution.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xTicksToDelay number of ticks that the calling co-routine should block for
 */
#define dgcrDELAY(xHandle, xTicksToDelay) \
        do { \
                crDELAY((xHandle), (xTicksToDelay)); \
        } while (0)

/**
 * \brief Delay co-routine until a specified time
 *
 * This function is used for the calling co-routine to delay until a specific time is reached,
 * as implied by *pxPreviousWakeTime + xTimeIncrement. The value of the variable pointed by
 * pxPreviousWakeTime is automatically updated within vDgCoRoutineCalcTimeUntil() function.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine yields its execution.
 *
 * INCLUDE_vDgCoRoutineDelayUntil must be set to 1 in FreeRTOSConfig.h for this function to
 * be available.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] pxPreviousWakeTime time at which the co-routine was last updated
 * \param [in] xTimeIncrement number of ticks after pxPreviousWakeTime that the co-routine should
 * delay until
 *
 *\sa vDgCoRoutineCalcTimeUntil
 */
#define dgcrDELAY_UNTIL(xHandle, pxPreviousWakeTime, xTimeIncrement) \
        do { \
                TickType_t xTicksToDelay = vDgCoRoutineCalcTimeUntil((pxPreviousWakeTime), \
                                                (xTimeIncrement)); \
                if (xTicksToDelay > 0) { \
                        vCoRoutineAddToDelayedList(xTicksToDelay, NULL); \
                } \
                crSET_STATE0((xHandle)); \
        } while (0)

/**
 * \brief Send notification to co-routine
 *
 * This function sends a notification to a co-routine and defines an optional action to be
 * performed, such as update, overwrite or increment the co-routine's notification value.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine with higher priority.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xCoRoutineToNotify handle of the co-routine being notified
 * \param [in] ulValue notification value sent with the notification
 * \param [in] eAction action on co-routine's notification value when notification is received
 * \param [out] pxResult pointer to store the result of this function, that is, pdFAIL if
 * eDgCrSetValueWithoutOverwrite is used as action and the co-routine being notified already
 * has a notification pending, otherwise pdPASS
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrCOROUTINE_NOTIFY(xHandle, xCoRoutineToNotify, ulValue, eAction, pxResult) \
        do { \
                *(pxResult) = xDgCoRoutineNotify((xCoRoutineToNotify), (ulValue), (eAction)); \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrCOROUTINE_NOTIFY(xHandle, xCoRoutineToNotify, ulValue, eAction, pxResult) \
        do { \
                *(pxResult) = xDgCoRoutineNotify((xCoRoutineToNotify), (ulValue), (eAction)); \
        } while (0)
#endif

/**
 * \brief Send notification to co-routine and retrieve previous notification value
 *
 * This function sends a notification to a co-routine and defines an optional action to be
 * performed, such as update, overwrite or increment the co-routine's notification value.
 * Furthermore, co-routine's previous notification value is retrieved.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine with higher priority.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xCoRoutineToNotify handle of the co-routine being notified
 * \param [in] ulValue notification value sent with the notification
 * \param [in] eAction action on co-routine's notification value when notification is received
 * \param [out] pulPreviousNotificationValue pointer to store co-routine's notification value
 * before any bits are modified by the notify function
 * \param [out] pxResult pointer to store the result of this function, that is, pdFAIL if
 * eDgCrSetValueWithoutOverwrite is used as action and the co-routine being notified already
 * has a notification pending, otherwise pdPASS
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrCOROUTINE_NOTIFY_AND_QUERY(xHandle, xCoRoutineToNotify, ulValue, eAction, \
                pulPreviousNotificationValue, pxResult) \
        do { \
                *(pxResult) = xDgCoRoutineNotifyAndQuery((xCoRoutineToNotify), (ulValue), \
                                (eAction), (pulPreviousNotificationValue)); \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrCOROUTINE_NOTIFY_AND_QUERY(xHandle, xCoRoutineToNotify, ulValue, eAction, \
                pulPreviousNotificationValue, pxResult) \
        do { \
                *(pxResult) = xDgCoRoutineNotifyAndQuery((xCoRoutineToNotify), (ulValue), \
                                (eAction), (pulPreviousNotificationValue)); \
        } while (0)
#endif

/**
 * \brief Send notification to co-routine from ISR
 *
 * This function sends a notification to a co-routine from interrupt service routine (ISR)
 * and defines an optional action to be performed, such as update, overwrite or increment the
 * co-routine's notification value.
 *
 * \param [in] xCoRoutineToNotify handle of the co-routine being notified
 * \param [in] ulValue notification value sent with the notification
 * \param [in] eAction action on co-routine's notification value when notification is received
 * \param [out] pulPreviousNotificationValue pointer to store co-routine's notification value
 * before any bits are modified by the notify function
 *
 * \return pdFAIL if eDgCrSetValueWithoutOverwrite is used as action and the co-routine being
 * notified already has a notification pending, otherwise pdPASS
 */
#define dgcrCOROUTINE_NOTIFY_FROM_ISR(xCoRoutineToNotify, ulValue, eAction) \
        xDgCoRoutineNotifyFromISR((xCoRoutineToNotify), (ulValue), (eAction))

/**
 * \brief Send notification to co-routine from ISR and retrieve previous notification value
 *
 * This function sends a notification to a co-routine from interrupt service routine (ISR)
 * and defines an optional action to be performed, such as update, overwrite or increment the
 * co-routine's notification value. Furthermore, co-routine's previous notification value is
 * retrieved.
 *
 * \param [in] xCoRoutineToNotify handle of the co-routine being notified
 * \param [in] ulValue notification value sent with the notification
 * \param [in] eAction action on co-routine's notification value when notification is received
 * \param [out] pulPreviousNotificationValue pointer to store co-routine's notification value
 * before any bits are modified by the notify function
 *
 * \return pdFAIL if eDgCrSetValueWithoutOverwrite is used as action and the co-routine being
 * notified already has a notification pending, otherwise pdPASS
 */
#define dgcrCOROUTINE_NOTIFY_AND_QUERY_FROM_ISR(xCoRoutineToNotify, ulValue, eAction, \
                pulPreviousNotificationValue) \
        xDgCoRoutineNotifyAndQueryFromISR((xCoRoutineToNotify), (ulValue), (eAction), \
                (pulPreviousNotificationValue))

/**
 * \brief Wait for the calling co-routine to receive notification
 *
 * This function blocks the calling co-routine to wait for a notification to be received.
 * The co-routine does not consume any CPU time while it is in the Blocked state. Co-routine's
 * notification value will be updated as indicated by ulBitsToClearOnEntry and ulBitsToClearOnExit
 * parameter values passed to the function. Furthermore, co-routine's notification value
 * is retrieved.
 *
 * This function can only be called from the co-routine function itself - not from within a
 * function called by the co-routine function. This is because co-routines do not maintain their
 * own stack, and the calling co-routine may yield its execution.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] ulBitsToClearOnEntry any bits set in ulBitsToClearOnEntry will be cleared in the
 * calling co-routine's notification value before the co-routine checks pending notifications
 * \param [in] ulBitsToClearOnExit any bits set in ulBitsToClearOnExit will be cleared in the
 * co-routine's notification value before the function exits
 * \param [out] pulNotificationValue pointer to store co-routine's notification value
 * before any bits are modified by the notification wait function
 * \param [in] xTicksToWait maximum amount of time in ticks the co-routine should wait in the
 * Blocked state for a notification to be received
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if a
 * notification was received by the co-routine, otherwise pdFAIL
 */
#define dgcrCOROUTINE_NOTIFY_WAIT(xHandle, ulBitsToClearOnEntry, ulBitsToClearOnExit, \
                pulNotificationValue, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xDgCoRoutineNotifyWait((ulBitsToClearOnEntry), \
                                (ulBitsToClearOnExit), (pulNotificationValue), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xDgCoRoutineNotifyWait((ulBitsToClearOnEntry), \
                                        (ulBitsToClearOnExit), (pulNotificationValue), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)

/**
 * \brief Send notification event to co-routine
 *
 * This function sends a notification event to a co-routine, incrementing the co-routine's
 * notification value. In that way co-routine notifications are used as a faster and lighter
 * weight binary or counting semaphore alternative.
 *
 * The notification sent to a co-routine will remain pending until it is cleared by the co-routine
 * calling dgcrCOROUTINE_NOTIFY_TAKE(). dgcrCOROUTINE_NOTIFY_TAKE() can either clear the
 * co-routine's notification value to zero on exit, in which case the notification value acts like
 * a binary semaphore, or decrement the co-routine's notification value on exit, in which case the
 * notification value acts like a counting semaphore.
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine with higher priority.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xCoRoutineToNotify handle of the co-routine being notified
 * \param [out] pxResult pointer to store the result of this function, that is, always pdPASS
 *
 * \sa dgcrCOROUTINE_NOTIFY_TAKE
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrCOROUTINE_NOTIFY_GIVE(xHandle, xCoRoutineToNotify, pxResult) \
        do { \
                *(pxResult) = xDgCoRoutineNotifyGive(xCoRoutineToNotify); \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrCOROUTINE_NOTIFY_GIVE(xHandle, xCoRoutineToNotify, pxResult) \
        do { \
                *(pxResult) = xDgCoRoutineNotifyGive(xCoRoutineToNotify); \
        } while (0)
#endif

/**
 * \brief Send notification event to co-routine from ISR
 *
 * This function sends a notification event to a co-routine from interrupt service routine (ISR),
 * incrementing the co-routine's notification value. In that way co-routine notifications are used
 * as a faster and lighter weight binary or counting semaphore alternative.
 *
 * The notification sent to a co-routine will remain pending until it is cleared by the co-routine
 * calling dgcrCOROUTINE_NOTIFY_TAKE(). dgcrCOROUTINE_NOTIFY_TAKE() can either clear the co-routine's
 * notification value to zero on exit, in which case the notification value acts like a binary
 * semaphore, or decrement the co-routine's notification value on exit, in which case the
 * notification value acts like a counting semaphore.
 *
 * \param [in] xCoRoutineToNotify handle of the co-routine being notified
 *
 * \return pdPASS
 *
 * \sa dgcrCOROUTINE_NOTIFY_TAKE
 */
#define dgcrCOROUTINE_NOTIFY_GIVE_FROM_ISR(xCoRoutineToNotify) \
        xDgCoRoutineNotifyGiveFromISR((xCoRoutineToNotify))

/**
 * \brief Wait for the calling co-routine to receive notification event
 *
 * This function checks for any pending notification for the calling co-routine and blocks the
 * co-routine to wait for its notification value to become equal to zero on exit.
 *
 * If the co-routine was already in the Blocked state to wait for a notification, when the
 * notification arrives, then the co-routine will automatically be removed from the Blocked state
 * (unblocked) if co-routine's notification value is equal to zero on exit. Co-routine's
 * notification value will be updated as indicated by xClearCountOnExit parameter value passed to
 * the function. Furthermore, co-routine's notification value is retrieved.
 *
 * This function can only be called from the co-routine function itself - not from within a
 * function called by the co-routine function. This is because co-routines do not maintain their
 * own stack, and the calling co-routine may yield its execution.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xClearCountOnExit if pdFALSE, co-routine's notification value is decremented when
 * the function exits (acting as counting semaphore), otherwise co-routine's notification value is
 * cleared to zero (acting as binary semaphore)
 * \param [out] pulNotificationValue pointer to store co-routine's notification value
 * before any bits are modified by the notification take function
 * \param [in] xTicksToWait maximum amount of time in ticks the co-routine should wait in the
 * Blocked state for a notification to be received and co-routine's notification value to be equal
 * to zero on exit
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if a
 * notification was received and co-routine's notification value became equal to zero on exit,
 * otherwise pdFAIL
 */
#define dgcrCOROUTINE_NOTIFY_TAKE(xHandle, xClearCountOnExit, pulNotificationValue, \
                xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xDgCoRoutineNotifyTake((xSemaphore), (xClearCountOnExit), \
                                (pulNotificationValue), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xDgCoRoutineNotifyTake((xSemaphore), (xClearCountOnExit), \
                                        (pulNotificationValue), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)


/**
 * \brief Run a co-routine
 *
 * This function executes the highest priority co-routine that is able to run. The co-routine
 * will execute until it either blocks or yields its execution.
 */
void vDgCoRoutineSchedule(void) PRIVILEGED_FUNCTION;

#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
/**
 * \brief Create new co-routine with dynamically allocated resources and add it to the
 *        list of co-routines that are ready to run
 *
 * This function creates a new co-routine and adds it to the list of co-routines that are ready
 * to run. The data structure for holding the co-routine's control block data is dynamically
 * allocated.
 *
 * \param [in] pxCoRoutineCode pointer to the co-routine function
 * \param [in] pcName co-routine name
 * \param [in] uxPriority co-routine priority
 * \param [in] uxIndex parameter past to co-routine function as unique number distinguishing
 *             co-routines executing the same function
 * \param [out] pxCreatedCoRoutine handle by which the created co-routine can be referenced
 *
 * \return pdPASS if the co-routine is successfully created and added to a ready list
 */
BaseType_t xDgCoRoutineCreate(crCOROUTINE_CODE pxCoRoutineCode,
        const char * const pcName, UBaseType_t uxPriority, UBaseType_t uxIndex,
        CoRoutineHandle_t * const pxCreatedCoRoutine) PRIVILEGED_FUNCTION;
#endif /* configSUPPORT_DYNAMIC_ALLOCATION */

#if (configSUPPORT_STATIC_ALLOCATION == 1)
/**
 * \brief Create new co-routine with statically allocated resources and add it to the
 *        list of co-routines that are ready to run
 *
 * This function creates a new co-routine and adds it to the list of co-routines that are ready
 * to run. The address of the data structure used for holding the co-routine's control block data
 * is provided as argument to the function.
 *
 * \param [in] pxCoRoutineCode pointer to the co-routine function
 * \param [in] pcName co-routine name
 * \param [in] uxPriority co-routine priority
 * \param [in] uxIndex parameter past to co-routine function as unique number distinguishing
 *             co-routines executing the same function
 * \param [in] pxCoRoutineBuffer pointer to the co-routine's control block data structure
 *
 * \return handle by which the created co-routine can be referenced, otherwise NULL
 */
CoRoutineHandle_t xDgCoRoutineCreateStatic(crCOROUTINE_CODE pxCoRoutineCode,
        const char * const pcName, UBaseType_t uxPriority, UBaseType_t uxIndex,
        StaticDgCoRoutine_t * const pxCoRoutineBuffer) PRIVILEGED_FUNCTION;
#endif /* configSUPPORT_STATIC_ALLOCATION */

/**
 * \brief Delete co-routine
 *
 * This function removes a co-routine from the RTOS kernel's management. The co-routine is removed
 * from all maintained lists, namely ready, pending ready, event and blocked lists.
 *
 * INCLUDE_vDgCoRoutineDelete must be set to 1 in FreeRTOSConfig.h for this function to
 * be available.
 *
 * \param [in] xCoRoutine handle of the co-routine to delete (if NULL, then calling co-routine
 * is implied)
 */
void vDgCoRoutineDelete(CoRoutineHandle_t xCoRoutine) PRIVILEGED_FUNCTION;

/**
 * \brief Calculate time interval until specified time
 *
 * This function calculates the time interval until a specified time, as implied by
 * *pxRefTime + xTimeIncrement. The value of the variable pointed by pxRefTime is
 * automatically updated to a new reference time, that is, it is incremented by xTimeIncrement.
 *
 * INCLUDE_vDgCoRoutineDelayUntil must be set to 1 in FreeRTOSConfig.h for this function to
 * be available.
 *
 * \param [in] pxRefTime reference time of the specified time in ticks
 * \param [in] xTimeIncrement number of ticks after pxRefTime
 *
 *\sa vDgCoRoutineCalcTimeUntil
 */
TickType_t vDgCoRoutineCalcTimeUntil(TickType_t * const pxRefTime, const TickType_t xTimeIncrement) PRIVILEGED_FUNCTION;

/**
 * \brief Obtain priority of co-routine
 *
 * This function gets the priority of a co-routine. If NULL is given as handle value, then the
 * calling co-routine is considered.
 *
 * INCLUDE_uxDgCoRoutinePriorityGet must be set to 1 in FreeRTOSConfig.h for this function to
 * be available.
 *
 * \param [in] xCoRoutine handle of the co-routine to query
 *
 * \return the priority of the co-routine
 */
UBaseType_t uxDgCoRoutinePriorityGet(CoRoutineHandle_t xCoRoutine) PRIVILEGED_FUNCTION;

/**
 * \brief Obtain priority of co-routine from ISR
 *
 * This function gets from interrupt service routine (ISR) the priority of a co-routine. If NULL
 * is given as handle value, then the calling co-routine is considered.
 *
 * INCLUDE_uxDgCoRoutinePriorityGet must be set to 1 in FreeRTOSConfig.h for this function to
 * be available.
 *
 * \param [in] xCoRoutine handle of the co-routine to query
 *
 * \return the priority of the co-routine
 */
UBaseType_t uxDgCoRoutinePriorityGetFromISR(CoRoutineHandle_t xCoRoutine) PRIVILEGED_FUNCTION;

/**
 * \brief Get co-routine state
 *
 * This function gets the state of a co-routine, as indicated by the eDgCoRoutineState
 * enumeration type.
 *
 * INCLUDE_eDgCoRoutineGetState must be set to 1 in FreeRTOSConfig.h for this function to
 * be available.
 *
 * \param [in] xCoRoutine handle of the co-routine to query
 *
 * \return the state of the co-routine at the time the function is called
 */
eDgCoRoutineState eDgCoRoutineGetState(CoRoutineHandle_t xCoRoutine) PRIVILEGED_FUNCTION;

/**
 * \brief Get current co-routine handle
 *
 * This function gets the handle of the calling co-routine.
 *
 * INCLUDE_xDgCoRoutineGetCurrentCoRoutineHandle must be set to 1 in FreeRTOSConfig.h for this
 * function to be available.
 *
 * \return the handle of the current co-routine
 */
CoRoutineHandle_t xDgCoRoutineGetCurrentCoRoutineHandle(void) PRIVILEGED_FUNCTION;

/**
 * \brief Get monitored co-routine's status
 *
 * This function populates a DgCoRoutineStatus_t structure for a monitored co-routine in the system,
 * given its handle. If the given handle value is NULL, then the co-routine currently being
 * executed is considered.
 * eState parameter has to be set to eDgCrInvalid in order to also obtain co-routine's state,
 * otherwise the value passed in eState will finally comprise the state of the co-routine in the
 * populated DgCoRoutineStatus_t structure.
 *
 * \param [in] xCoRoutine handle of the co-routine to query
 * \param [in,out] pxCoRoutineStatus pointer to the DgCoRoutineStatus_t structure that will be
 * filled with status information about the co-routine
 * \param [in] eState input co-routine state
 */
void vDgCoRoutineGetInfo(CoRoutineHandle_t xCoRoutine, DgCoRoutineStatus_t *pxCoRoutineStatus,
        eDgCoRoutineState eState) PRIVILEGED_FUNCTION;

/**
 * \brief Start the scheduler
 *
 * This function creates idle and timer (daemon) co-routines, initiates OS resources and starts the
 * scheduler.
 */
void vDgCoRoutineStartScheduler(void) PRIVILEGED_FUNCTION;

/**
 * \brief Get OS tick count
 *
 * This function acquires the OS tick count since vDgCoRoutineStartScheduler() was called.
 *
 * \return current tick count
 *
 * \sa vDgCoRoutineStartScheduler
 */
TickType_t xDgCoRoutineGetTickCount(void) PRIVILEGED_FUNCTION;

/**
 * \brief Get OS tick count from ISR
 *
 * This function acquires from interrupt service routine (ISR) the OS tick count since
 * vDgCoRoutineStartScheduler() was called.
 *
 * \return current tick count
 *
 * \sa vDgCoRoutineStartScheduler
 */
TickType_t xDgCoRoutineGetTickCountFromISR(void) PRIVILEGED_FUNCTION;

/**
 * \brief Get number of co-routines currently controlled by the RTOS
 *
 * This function returns the number of co-routines currently controlled by the RTOS kernel.
 * This includes all co-routines the resources of which are currently maintained in the system.
 *
 * \return number of co-routines currently controlled by the RTOS
 */
UBaseType_t uxDgCoRoutineGetNumberOfCoRoutines(void) PRIVILEGED_FUNCTION;

/**
 * \brief Get co-routine name
 *
 * This function returns the name of a co-routine based on the given handle. If NULL is given as
 * handle value, then the calling co-routine is considered.
 *
 * \param [in] xCoRoutineToQuery handle of the co-routine to query
 *
 * \return string pointer pointing to the text name of the co-routine
 */
char *pcDgCoRoutineGetName(CoRoutineHandle_t xCoRoutineToQuery) PRIVILEGED_FUNCTION;

/**
 * \brief Get co-routine handle by name
 *
 * This function returns the handle of a co-routine based on the given name.
 *
 * INCLUDE_xDgCoRoutineGetHandle must be set to 1 in FreeRTOSConfig.h for this function to
 * be available.
 *
 * \param [in] pcNameToQuery string pointer pointing to the text name to query
 *
 * \return handle of the co-routine which has the name pcNameToQuery, otherwise NULL if no matching
 * name is found
 *
 * \note This function takes a relatively long time to complete and should be used sparingly.
 */
CoRoutineHandle_t xDgCoRoutineGetHandle(const char *pcNameToQuery) PRIVILEGED_FUNCTION;

/**
 * \brief Get co-routine stack high water mark
 *
 * This function returns the high water mark of the stack used by a co-routine, that is the
 * minimum free stack space there has been remained for co-routine (in words, so on a 32 bit
 * machine a value of 1 means 4 bytes) since the co-routine was created. If NULL is given as handle
 * value, then the calling co-routine is considered.
 *
 * INCLUDE_uxDgCoRoutineGetStackHighWaterMark must be set to 1 in FreeRTOSConfig.h for this
 * function to be available.
 *
 * \param [in] xCoRoutine handle of the co-routine to query
 *
 * \return the smallest amount of free stack space there has been remained for co-routine
 * (in words, so actual spaces on the stack rather than bytes) since the co-routine was created
 */
UBaseType_t uxDgCoRoutineGetStackHighWaterMark(CoRoutineHandle_t xCoRoutine) PRIVILEGED_FUNCTION;

/**
 * \brief Get co-routine stack high water mark
 *
 * This function returns the start of the stack space used by co-routine. That is, the highest
 * stack memory address on architectures where the stack grows down from high memory, and the
 * lowest memory address on architectures where the stack grows up from low memory.
 * If NULL is given as handle value, then the calling co-routine is considered.
 *
 * INCLUDE_pxDgCoRoutineGetStackStart must be set to 1 in FreeRTOSConfig.h for this function to
 * be available.
 *
 * \param [in] xCoRoutine handle of the co-routine to query
 *
 * \return pointer to the start of the stack
 */
uint8_t *pxDgCoRoutineGetStackStart(CoRoutineHandle_t xCoRoutine) PRIVILEGED_FUNCTION;

/**
 * \brief Get co-routine scheduler state
 *
 * This function returns the state of co-routine scheduler.
 *
 * INCLUDE_xDgCoRoutineGetSchedulerState must be set to 1 in FreeRTOSConfig.h for this function to
 * be available.
 *
 * \return co-routine scheduler state as dgcrSCHEDULER_RUNNING, dgcrSCHEDULER_NOT_STARTED or
 * dgcrSCHEDULER_SUSPENDED
 */
BaseType_t xDgCoRoutineGetSchedulerState(void) PRIVILEGED_FUNCTION;

/**
 * \brief Get idle co-routine handle
 *
 * This functions returns the handle of the idle co-routine. If this function is called before
 * the scheduler has been started, it returns NULL.
 *
 * INCLUDE_xDgCoRoutineGetIdleCoRoutineHandle must be set to 1 in FreeRTOSConfig.h for this
 * function to be available.
 *
 * \return the handle of idle co-routine
 */
CoRoutineHandle_t xDgCoRoutineGetIdleCoRoutineHandle(void) PRIVILEGED_FUNCTION;

/**
 * \brief Get monitored co-routines' status
 *
 * This function populates a DgCoRoutineStatus_t structure for each monitored co-routine in
 * the system. The given array must contain at least one DgCoRoutineStatus_t structure for each
 * co-routine that is under the control of the RTOS. The number of co-routines that are currently
 * controlled by the RTOS can be determined using the pcDgCoRoutineGetNumberOfCoRoutines function.
 *
 * \param [in] pxCoRoutineStatusArray pointer to an array of DgCoRoutineStatus_t structures
 * \param [in] uxArraySize size of the array in number of DgCoRoutineStatus_t structures,
 * pointed to by the pxCoRoutineStatusArray parameter
 * \param [in] pulTotalRunTime pointer to store the total run time (as defined by the run time
 * stats clock, see http://www.freertos.org/rtos-run-time-stats.html) since the target booted. If
 * NULL, total run time information is omitted
 *
 * \return number of populated DgCoRoutineStatus_t structures
 *
 * \note This function is intended for debugging use only
 *
 * \sa pcDgCoRoutineGetNumberOfCoRoutines
 */
UBaseType_t uxDgCoRoutineGetSystemState(DgCoRoutineStatus_t * const pxCoRoutineStatusArray,
        const UBaseType_t uxArraySize, uint32_t * const pulTotalRunTime) PRIVILEGED_FUNCTION;

/**
 * \brief Send notification to co-routine
 *
 * This function sends a notification to a co-routine and defines an optional action to be
 * performed, such as update, overwrite or increment the co-routine's notification value.
 * In that way co-routine notifications can be used to send data to a co-routine, or be used as
 * light weight and fast binary or counting semaphores.
 *
 * A notification sent to a co-routine will remain pending until it is cleared by the co-routine
 * calling xDgCoRoutineNotifyWait() or xDgCoRoutineNotifyTake().
 *
 * configUSE_DG_COROUTINE_NOTIFICATIONS must be undefined or defined as 1 for this function to be
 * available.
 *
 * \param [in] xCoRoutineToNotify handle of the co-routine being notified
 * \param [in] ulValue notification value sent with the notification
 * \param [in] eAction action on co-routine's notification value when notification is received.
 * Valid values for eAction are as follows:
 * eDgCrSetBits                  - The co-routine's notification value is bitwise ORed with ulValue.
 * eDgCrIncrement                - The co-routine's notification value is incremented.
 * eDgCrSetValueWithOverwrite    - The co-routine's notification value is set to the value of ulValue.
 * eDgCrSetValueWithoutOverwrite - If the co-routine being notified did not already have a
 *                                 notification pending, then the co-routine's notification value
 *                                 is set to ulValue, otherwise no action is performed.
 * eDgCrNoAction                 - The co-routine receives a notification without its notification
 *                                 value being updated.
 * \param [out] pulPreviousNotificationValue pointer to store co-routine's notification value
 * before any bits are modified by the notify function
 *
 * \return pdFAIL if eDgCrSetValueWithoutOverwrite is used as action and the co-routine being
 * notified already has a notification pending, otherwise pdPASS
 *
 * \sa xDgCoRoutineNotifyWait
 * \sa xDgCoRoutineNotifyTake
 */
BaseType_t xDgCoRoutineGenericNotify(CoRoutineHandle_t xCoRoutineToNotify, uint32_t ulValue,
        eDgCoRoutineNotifyAction eAction, uint32_t *pulPreviousNotificationValue) PRIVILEGED_FUNCTION;
#define xDgCoRoutineNotify(xCoRoutineToNotify, ulValue, eAction) \
        xDgCoRoutineGenericNotify((xCoRoutineToNotify), (ulValue), (eAction), NULL)
#define xDgCoRoutineNotifyAndQuery(xCoRoutineToNotify, ulValue, eAction, pulPreviousNotificationValue) \
        xDgCoRoutineGenericNotify((xCoRoutineToNotify), (ulValue), (eAction), (pulPreviousNotificationValue))

/**
 * \brief Send notification to co-routine from ISR
 *
 * This function sends a notification to a co-routine from interrupt service routine (ISR) and
 * defines an optional action to be performed, such as update, overwrite or increment the
 * co-routine's notification value. In that way co-routine notifications can be used to send data
 * to a co-routine, or be used as light weight and fast binary or counting semaphores.
 *
 * A notification sent to a co-routine will remain pending until it is cleared by the co-routine
 * calling xDgCoRoutineNotifyWait() or xDgCoRoutineNotifyTake().
 *
 * configUSE_DG_COROUTINE_NOTIFICATIONS must be undefined or defined as 1 for this function to be
 * available.
 *
 * \param [in] xCoRoutineToNotify handle of the co-routine being notified
 * \param [in] ulValue notification value sent with the notification
 * \param [in] eAction action on co-routine's notification value when notification is received.
 * Valid values for eAction are as follows:
 * eDgCrSetBits                  - The co-routine's notification value is bitwise ORed with ulValue.
 * eDgCrIncrement                - The co-routine's notification value is incremented.
 * eDgCrSetValueWithOverwrite    - The co-routine's notification value is set to the value of ulValue.
 * eDgCrSetValueWithoutOverwrite - If the co-routine being notified did not already have a
 *                                 notification pending, then the co-routine's notification value
 *                                 is set to ulValue, otherwise no action is performed.
 * eDgCrNoAction                 - The co-routine receives a notification without its notification
 *                                 value being updated.
 * \param [out] pulPreviousNotificationValue pointer to store co-routine's notification value
 * before any bits are modified by the notify function
 *
 * \return pdFAIL if eDgCrSetValueWithoutOverwrite is used as action and the co-routine being
 * notified already has a notification pending, otherwise pdPASS
 *
 * \sa xDgCoRoutineNotifyWait
 * \sa xDgCoRoutineNotifyTake
 */
BaseType_t xDgCoRoutineGenericNotifyFromISR(CoRoutineHandle_t xCoRoutineToNotify, uint32_t ulValue,
        eDgCoRoutineNotifyAction eAction, uint32_t *pulPreviousNotificationValue) PRIVILEGED_FUNCTION;
#define xDgCoRoutineNotifyFromISR(xCoRoutineToNotify, ulValue, eAction) \
        xDgCoRoutineGenericNotifyFromISR((xCoRoutineToNotify), (ulValue), (eAction), NULL)
#define xDgCoRoutineNotifyAndQueryFromISR(xCoRoutineToNotify, ulValue, eAction, \
                pulPreviousNotificationValue) \
        xDgCoRoutineGenericNotifyFromISR((xCoRoutineToNotify), (ulValue), (eAction), \
                (pulPreviousNotificationValue))

/**
 * \brief Wait for the calling co-routine to receive notification
 *
 * This function checks for any pending notification for a co-routine and indicates whether the
 * co-routine needs to block and wait for a notification to be received.
 *
 * If the co-routine was already in the Blocked state to wait for a notification, when the
 * notification arrives, then the co-routine will automatically be removed from the Blocked state
 * (unblocked) and the notification will be cleared. Co-routine's notification value will be updated
 * as indicated by ulBitsToClearOnEntry and ulBitsToClearOnExit parameter values passed to the
 * function. Furthermore, co-routine's notification value is retrieved.
 *
 * configUSE_DG_COROUTINE_NOTIFICATIONS must be undefined or defined as 1 for this function to be
 * available.
 *
 * \param [in] ulBitsToClearOnEntry any bits set in ulBitsToClearOnEntry will be cleared in the
 * calling co-routine's notification value before the co-routine checks pending notifications
 * \param [in] ulBitsToClearOnExit any bits set in ulBitsToClearOnExit will be cleared in the
 * co-routine's notification value before the function exits
 * \param [out] pulNotificationValue pointer to store co-routine's notification value
 * before any bits are modified by the notification wait function
 * \param [in] xTicksToWait maximum amount of time in ticks the co-routine should wait in the
 * Blocked state for a notification to be received
 *
 * \return pdPASS if a notification was received, otherwise an error code as defined within ProjDefs.h
 */
BaseType_t xDgCoRoutineNotifyWait(uint32_t ulBitsToClearOnEntry, uint32_t ulBitsToClearOnExit,
        uint32_t *pulNotificationValue, TickType_t xTicksToWait) PRIVILEGED_FUNCTION;

/**
 * \brief Send notification event to co-routine
 *
 * This function sends a notification event to a co-routine, incrementing the co-routine's
 * notification value. In that way co-routine notifications are used as a faster and lighter
 * weight binary or counting semaphore alternative.
 *
 * The notification sent to a co-routine will remain pending until it is cleared by the co-routine
 * calling xDgCoRoutineNotifyTake(). xDgCoRoutineNotifyTake() can either clear the co-routine's
 * notification value to zero on exit, in which case the notification value acts like a binary
 * semaphore, or decrement the co-routine's notification value on exit, in which case the
 * notification value acts like a counting semaphore.
 *
 * configUSE_DG_COROUTINE_NOTIFICATIONS must be undefined or defined as 1 for this function to be
 * available.
 *
 * \param [in] xCoRoutineToNotify handle of the co-routine being notified
 *
 * \return pdPASS
 *
 * \sa xDgCoRoutineNotifyTake
 */
#define xDgCoRoutineNotifyGive(xCoRoutineToNotify) \
        xDgCoRoutineGenericNotify((xCoRoutineToNotify), 0, eDgCrIncrement, NULL)

/**
 * \brief Send notification event to co-routine from ISR
 *
 * This function sends a notification event to a co-routine from interrupt service routine (ISR),
 * incrementing the co-routine's notification value. In that way co-routine notifications are used
 * as a faster and lighter weight binary or counting semaphore alternative.
 *
 * The notification sent to a co-routine will remain pending until it is cleared by the co-routine
 * calling xDgCoRoutineNotifyTake(). xDgCoRoutineNotifyTake() can either clear the co-routine's
 * notification value to zero on exit, in which case the notification value acts like a binary
 * semaphore, or decrement the co-routine's notification value on exit, in which case the
 * notification value acts like a counting semaphore.
 *
 * configUSE_DG_COROUTINE_NOTIFICATIONS must be undefined or defined as 1 for this function to be
 * available.
 *
 * \param [in] xCoRoutineToNotify handle of the co-routine being notified
 *
 * \return pdPASS
 *
 * \sa xDgCoRoutineNotifyTake
 */
#define xDgCoRoutineNotifyGiveFromISR(xCoRoutineToNotify) \
        xDgCoRoutineGenericNotifyFromISR((xCoRoutineToNotify), 0, eDgCrIncrement, NULL)

/**
 * \brief Wait for the calling co-routine to receive notification event
 *
 * This function checks for any pending notification for the calling co-routine and indicates
 * whether the co-routine needs to block and wait for its notification value to become equal to
 * zero on exit.
 *
 * If the co-routine was already in the Blocked state to wait for a notification, when the
 * notification arrives, then the co-routine will automatically be removed from the Blocked state
 * (unblocked) if co-routine's notification value is equal to zero on exit. Co-routine's
 * notification value will be updated as indicated by xClearCountOnExit parameter value passed to
 * the function. Furthermore, co-routine's notification value is retrieved.
 *
 * configUSE_DG_COROUTINE_NOTIFICATIONS must be undefined or defined as 1 for this function to be
 * available.
 *
 * \param [in] xClearCountOnExit if pdFALSE, co-routine's notification value is decremented when
 * the function exits (acting as counting semaphore), otherwise co-routine's notification value is
 * cleared to zero (acting as binary semaphore)
 * \param [out] pulNotificationValue pointer to store co-routine's notification value
 * before any bits are modified by the notification take function
 * \param [in] xTicksToWait maximum amount of time in ticks the co-routine should wait in the
 * Blocked state for a notification to be received and co-routine's notification value to become
 * equal to zero on exit
 *
 * \return pdPASS if a notification was received and co-routine's notification value became equal
 * to zero on exit, otherwise an error code as defined within ProjDefs.h
 */
BaseType_t xDgCoRoutineNotifyTake(BaseType_t xClearCountOnExit, uint32_t *pulNotificationValue,
                TickType_t xTicksToWait) PRIVILEGED_FUNCTION;

/*
 * Functions beyond this part are not part of the public API and are intended for use by the
 * kernel only.
 */

/*
 * Increment tick count.
 */
void xDgCoRoutineIncrementTick(void) PRIVILEGED_FUNCTION;

/*
 * Place calling co-routine on event list.
 */
void vDgCoRoutinePlaceOnEventListRestricted(List_t * const pxEventList, TickType_t xTicksToWait) PRIVILEGED_FUNCTION;

/*
 * Indicate that yield is pending (for a co-routine with higher priority).
 * It must be called with disabled interrupts.
 */
void vDgCoRoutineMissedYield(void) PRIVILEGED_FUNCTION;

/*
 * Indicate that yield is pending for a co-routine of a given priority.
 * It must be called with disabled interrupts.
 */
void vDgCoRoutineMissedYieldForPriority(UBaseType_t uxPriority) PRIVILEGED_FUNCTION;

/*
 * Clear pending yield.
 */
void vDgCoRoutineClearPendingYield(void) PRIVILEGED_FUNCTION;

/*
 * Check whether yield is pending.
 */
BaseType_t vDgCoRoutineIsPendingYield(void) PRIVILEGED_FUNCTION;

/*
 * Scheduler enters state of performing co-routine context switch.
 */
void vDgCoRoutineSchedulerEnterContextSwitch(void) PRIVILEGED_FUNCTION;

/*
 * Scheduler leaves state of performing co-routine context switch.
 */
void vDgCoRoutineSchedulerLeaveContextSwitch(void) PRIVILEGED_FUNCTION;

/*
 * Confirm sleep mode status.
 */
eDgCoRoutineSleepModeStatus eDgCoRoutineConfirmSleepModeStatus(void) PRIVILEGED_FUNCTION;

/*
 * If the mutex holder has a priority less than the calling co-routine, raise the priority of the
 * mutex holder to that of the calling co-routine.
 */
BaseType_t xDgCoRoutinePriorityInherit(CoRoutineHandle_t const pxMutexHolder) PRIVILEGED_FUNCTION;

/*
 * Disnherit the priority the mutex holder has been set.
 */
BaseType_t xDgCoRoutinePriorityDisinherit(CoRoutineHandle_t const pxMutexHolder) PRIVILEGED_FUNCTION;

/*
 * Increment mutex held count of the calling co-routine.
 */
void *pvDgCoRoutineIncrementMutexHeldCount(void) PRIVILEGED_FUNCTION;

/*
 * Get the uxCRCBNumber assigned to co-routine.
 */
UBaseType_t uxDgCoRoutineGetCoRoutineNumber(CoRoutineHandle_t xCoRoutine) PRIVILEGED_FUNCTION;

/*
 * Set the uxCoRoutineNumber of co-routine to uxHandle.
 */
void vDgCoRoutineSetCoRoutineNumber(CoRoutineHandle_t xCoRoutine, const UBaseType_t uxHandle) PRIVILEGED_FUNCTION;

/*
 * Correct tick count after a period during which the tick was suppressed.
 */
void vDgCoRoutineStepTick(const TickType_t xTicksToJump) PRIVILEGED_FUNCTION;

/*
 * Update next co-routine unblock time.
 */
void vResetNextDgCoRoutineUnblockTime(void) PRIVILEGED_FUNCTION;

#ifdef __cplusplus
}
#endif

#endif /* DIALOG_CO_ROUTINE_H_ */
