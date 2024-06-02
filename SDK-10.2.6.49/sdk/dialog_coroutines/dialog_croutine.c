/**
 ****************************************************************************************
 *
 * @file dialog_croutine.c
 *
 * @brief Extensions to FreeRTOS croutine.c
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
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

#define _NON_STANDALONE_CROUTINE_C
#include "croutine.c"

#include "croutine.h"
#include "timers.h"

#if( configUSE_DG_COROUTINE_DEBUG_FACILITY == 1 )
        /* Macros referring to Dialog CoRoutines symbols are defined to corresponding ones of FreeRTOS. */
        #define uxCurrentNumberOfDgCoRoutines   uxCurrentNumberOfTasks
        #define uxDgCoRoutineNumber             uxTaskNumber
#endif /* configUSE_DG_COROUTINE_DEBUG_FACILITY */

/*
 * The value used to fill the system stack before a co-routine is called by the scheduler.
 * This is used purely for checking the high water mark of the stack utilized by a co-routine.
 */
#define dgcrSTACK_FILL_BYTE                     ( 0xa5U )

/*
 * If any of the following are set, stack tracing is enabled by filling system stack after
 * position indicated by SP with a known value, so that the high water mark of the stack utilized
 * by a co-routine can be determined.
 */
#if (configUSE_TRACE_FACILITY == 1) || (INCLUDE_uxDgCoRoutineGetStackHighWaterMark == 1)
        #define dgcrSTACK_TRACING               ( 1 )
#else
        #define dgcrSTACK_TRACING               ( 0 )
#endif

/* Values that can be assigned to the ucNotifyState member of the CRCB. */
#define dgcrNOT_WAITING_NOTIFICATION            ((uint8_t) 0)
#define dgcrWAITING_NOTIFICATION                ((uint8_t) 1)
#define dgcrNOTIFICATION_RECEIVED               ((uint8_t) 2)

/* Scheduler running states */
#define dgcrSCHEDULER_NOT_RUNNING               ((UBaseType_t) 0)
#define dgcrSCHEDULER_RUNNING_SLEEP             ((UBaseType_t) 1)
#define dgcrSCHEDULER_RUNNING_IN_BACKGROUND     ((UBaseType_t) 2)
#define dgcrSCHEDULER_RUNNING_CONTEXT_SWITCH    ((UBaseType_t) 3)

/* Member of CRCB structure pointing to the start (i.e. highest address) of the co-routine's stack. */
#if (portSTACK_GROWTH < 0)
        #define pxStartOfStack                  pxEndOfStack
#else
        #define pxStartOfStack                  pxStack
#endif

/*
 * Some kernel aware debuggers require the data the debugger needs access to be global,
 * rather than file scope.
 */
#ifdef portREMOVE_STATIC_QUALIFIER
        #define static
#endif

/*
 * The name allocated to the Idle co-routine. This can be overridden by defining
 * configIDLE_DG_COROUTINE_NAME in FreeRTOSConfig.h.
 */
#ifndef configIDLE_DG_COROUTINE_NAME
        #define configIDLE_DG_COROUTINE_NAME "IDLE"
#endif

/*-----------------------------------------------------------*/

/*
 * Check if given co-routine handle is NULL and return a pointer either to the control block data
 * structure of the calling co-routine or the one indicated by the handle.
 */
#define prvGetDgCRCBFromHandle(pxHandle) \
        (((pxHandle) == NULL) ? (CRCB_t *)pxCurrentCoRoutine : (CRCB_t *)(pxHandle))

/* File private variables. ----------------------------------*/
PRIVILEGED_DATA static volatile UBaseType_t uxCurrentNumberOfDgCoRoutines = (UBaseType_t) 0U;
PRIVILEGED_DATA static volatile TickType_t xTickCount = (TickType_t) configINITIAL_TICK_COUNT;
PRIVILEGED_DATA static volatile BaseType_t xSchedulerRunning = dgcrSCHEDULER_NOT_RUNNING;
PRIVILEGED_DATA static volatile UBaseType_t uxPendedTicks = (UBaseType_t) 0U;
PRIVILEGED_DATA static volatile BaseType_t xYieldPending = pdFALSE;
PRIVILEGED_DATA static volatile UBaseType_t xYieldContextSwitchMaxPriority = dgcrIDLE_PRIORITY;
#if (INCLUDE_vDgCoRoutineDelete == 1)
PRIVILEGED_DATA static CoRoutineHandle_t xDeleteDgCoRoutineHandle = NULL;
#endif /* INCLUDE_vDgCoRoutineDelete */
PRIVILEGED_DATA static CoRoutineHandle_t xIdleDgCoRoutineHandle = NULL;
#if (configUSE_TRACE_FACILITY == 1)
PRIVILEGED_DATA static UBaseType_t uxDgCoRoutineNumber = (UBaseType_t) 0U;
#endif
PRIVILEGED_DATA static volatile TickType_t xNextDgCoRoutineUnblockTime = (TickType_t) 0U;

#if (configSUPPORT_STATIC_ALLOCATION == 1)
/* Space reserved for storing control block data structure of Idle co-routine. */
PRIVILEGED_DATA static StaticDgCoRoutine_t xIdleDgCoRoutineBuffer;
#endif /* configSUPPORT_STATIC_ALLOCATION */

#if (configGENERATE_RUN_TIME_STATS == 1)
/* Holds the value of a timer/counter the last time a co-routine was switched in. */
PRIVILEGED_DATA static uint32_t ulDgCoRoutineSwitchedInTime = 0UL;
/* Holds the total amount of execution time as defined by the run time counter clock. */
PRIVILEGED_DATA static uint32_t ulTotalRunTime = 0UL;
#endif

/* The initial state of the co-routine when it is created. */
#define dgcrINITIAL_STATE       ( 0 )

/* File private functions ------------------------------------*/

/*
 * Idle co-routine. It is automatically created and added to the ready lists when
 * scheduler is started.
 */
static void prvIdleDgCoRoutine(CoRoutineHandle_t xHandle, UBaseType_t uxIndex) PRIVILEGED_FUNCTION;

#if (INCLUDE_vDgCoRoutineDelete == 1)
/*
 * Remove co-routine from RTOS scheduler's management and free (dynamically allocated) memory for
 * its control block structure.
 */
static void prvDeleteDgCRCB(CRCB_t *pxCRCB) PRIVILEGED_FUNCTION;
#endif /* INCLUDE_vDgCoRoutineDelete */

#if (configUSE_TRACE_FACILITY == 1)
/*
 * Fill a DgCoRoutineStatus_t structure with information for each co-routine pxList list.
 */
static UBaseType_t prvListDgCoRoutinesWithinSingleList(DgCoRoutineStatus_t *pxCoRoutineStatusArray,
        List_t *pxList, eDgCoRoutineState eState) PRIVILEGED_FUNCTION;
#endif

#if (INCLUDE_xDgCoRoutineGetHandle == 1) && (configMAX_DG_COROUTINE_NAME_LEN > 0)
/*
 * Search for a co-routine with name pcNameToQuery in pxList.
 */
static CRCB_t *prvSearchForNameWithinSingleList(List_t *pxList, const char pcNameToQuery[]) PRIVILEGED_FUNCTION;
#endif

#if ( configUSE_TICKLESS_IDLE != 0 )
/*
 * Return expected amount of time, in ticks, that will pass before the kernel will next move a
 * co-routine from the Blocked state to the Running state.
 */
static TickType_t prvGetExpectedIdleTime(void) PRIVILEGED_FUNCTION;
#endif

/*
 * Initialise a co-routine control block data structure.
 */
static void prvInitialiseNewDgCoRoutine(crCOROUTINE_CODE pxCoRoutineCode, const char * const pcName,
        UBaseType_t uxIndex, UBaseType_t uxPriority, CRCB_t *pxNewCRCB) PRIVILEGED_FUNCTION;

/*
 * Place a co-routine into the appropriate ready queue based on its priority, after initialising
 * its control data structure.
 */
static void prvAddNewCoRoutineToReadyQueue(CRCB_t *pxNewCRCB) PRIVILEGED_FUNCTION;

#ifdef FREERTOS_TASKS_C_ADDITIONS_INIT
/*
 * freertos_tasks_c_additions_init() should only be called if the user definable
 * macro FREERTOS_TASKS_C_ADDITIONS_INIT() is defined, as that is the only macro
 * called by the function.
 */
static void freertos_tasks_c_additions_init(void) PRIVILEGED_FUNCTION;
#endif

#if (configCHECK_FOR_STACK_OVERFLOW > 0) || (dgcrSTACK_TRACING == 1)
/*
 * Initialise stack content from a position and on, till specified address
 */
static portFORCE_INLINE void prvInitialiseStack(uint8_t *pucStackStart,
        uint8_t *pucStackEnd)
{
#if (portSTACK_GROWTH < 0)
        uint8_t *pucStart = pucStackEnd;
        uint8_t *pucEnd = pucStackStart;
#else
        uint8_t *pucStart = pucStackStart + sizeof(StackType_t);
        uint8_t *pucEnd = pucStackEnd + sizeof(StackType_t);
#endif

        (void) memset(pucStart, (int) dgcrSTACK_FILL_BYTE, (size_t)(pucEnd - pucStart));
}
#endif /* configCHECK_FOR_STACK_OVERFLOW || dgcrSTACK_TRACING */

#if (dgcrSTACK_TRACING == 1)
/*
 * Initialise stack content from a position and on, till specified address or till the known value
 * is found into four consecutive bytes.
 */
/* cppcheck-suppress constParameter */
static portFORCE_INLINE void prvCheckAndInitialiseStack(uint8_t *pucStackStart,
        const uint8_t *pucStackEnd)
{
        int xStackFillByteCount = 0;

#if (portSTACK_GROWTH < 0)
        uint8_t *pucStart = pucStackStart - 1;
        const uint8_t *pucEnd = pucStackEnd - 1;

        while ((pucStart > pucEnd) && (xStackFillByteCount < 4)) {
#else
        uint8_t *pucStart = pucStackStart + sizeof(StackType_t);
        const uint8_t *pucEnd = pucStackEnd + sizeof(StackType_t);

        while ((pucStart < pucEnd) && (xStackFillByteCount < 4)) {
#endif
                if (*pucStart == (uint8_t) dgcrSTACK_FILL_BYTE) {
                        xStackFillByteCount++;
                } else {
                        xStackFillByteCount = 0;
                        *pucStart = (uint8_t) dgcrSTACK_FILL_BYTE;
                }
                pucStart += portSTACK_GROWTH;
        }
}

/*
 * Get free stack space for a co-routine.
 */
static uint16_t prvGetFreeStackSpace(void) PRIVILEGED_FUNCTION;
#endif /* dgcrSTACK_TRACING */

/*-----------------------------------------------------------*/

void vDgCoRoutineSchedule(void)
{
#if (dgcrSTACK_TRACING == 1)
        {
                uint8_t *pucDgCRStackStart;

                dgcrENTER_CRITICAL();

                if (pxCurrentCoRoutine->pxStartOfStack != NULL) {
                        pucDgCRStackStart = (uint8_t *)pxCurrentCoRoutine->pxStartOfStack;
                } else {
                        pucDgCRStackStart = (uint8_t *)(portGET_SP());
                }

                /*
                 * Initialise stack by checking first and then filling byte-by-byte with a known
                 * value to assist debugging. If the known value is found into four consecutive
                 * bytes, initialization stops.
                 */
#if (portSTACK_GROWTH < 0)
                prvCheckAndInitialiseStack(pucDgCRStackStart, (uint8_t *)(portSTACK_LIMIT));
#else
                prvCheckAndInitialiseStack(pucDgCRStackStart, (uint8_t *)(portSTACK_LIMIT) - sizeof(StackType_t));
#endif

                dgcrEXIT_CRITICAL();
        }
#endif /* dgcrSTACK_TRACING */

#if (configGENERATE_RUN_TIME_STATS == 1)
#ifdef portALT_GET_RUN_TIME_COUNTER_VALUE
        portALT_GET_RUN_TIME_COUNTER_VALUE(ulTotalRunTime);
#else
        ulTotalRunTime = portGET_RUN_TIME_COUNTER_VALUE();
#endif

        /*
         * Add the amount of time the co-routine has been running to the accumulated time so far.
         * Note that there is no overflow protection here so count values are only valid until the
         * timer overflows.
         */
        if (ulTotalRunTime > ulDgCoRoutineSwitchedInTime) {
                pxCurrentCoRoutine->ulRunTimeCounter +=
                        (ulTotalRunTime - ulDgCoRoutineSwitchedInTime);
        } else {
                mtCOVERAGE_TEST_MARKER();
        }
        ulDgCoRoutineSwitchedInTime = ulTotalRunTime;
#endif /* configGENERATE_RUN_TIME_STATS */

#if (INCLUDE_vDgCoRoutineDelete == 1)
        /* Check if the current co-routine has deleted itself. */
        if (xDeleteDgCoRoutineHandle != NULL) {
                /* Set idle co-routine as current one. */
                pxCurrentCoRoutine = xIdleDgCoRoutineHandle;

                /* Delete current co-routine. */
                prvDeleteDgCRCB((CRCB_t *)xDeleteDgCoRoutineHandle);
                xDeleteDgCoRoutineHandle = NULL;
        }
#endif /* INCLUDE_vDgCoRoutineDelete */

        /* Scheduler starts performing context switch. */
        vDgCoRoutineSchedulerEnterContextSwitch();

        prvCoRoutineSchedule();

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
        {
#if (portSTACK_GROWTH < 0)
                const uint32_t * const pulStack = (uint32_t *)(portSTACK_LIMIT);
#else
                const uint32_t * const pulStack = (uint32_t *)(portSTACK_LIMIT) - 4;
#endif
                const uint32_t ulCheckValue =
                        (((uint32_t) dgcrSTACK_FILL_BYTE) << 24)
                        | (((uint32_t) dgcrSTACK_FILL_BYTE) << 16)
                        | (((uint32_t) dgcrSTACK_FILL_BYTE) << 8)
                        | ((uint32_t) dgcrSTACK_FILL_BYTE);

                extern void vApplicationStackOverflowHook(CoRoutineHandle_t xCoRoutine,
                                char *pcCoRoutineName);

                /* Check for stack overflow and call application hook function. */
                if ((*pulStack != ulCheckValue) || (*(pulStack + 1) != ulCheckValue)
                    || (*(pulStack + 2) != ulCheckValue) || (*(pulStack + 3) != ulCheckValue)) {
                        vApplicationStackOverflowHook((CoRoutineHandle_t) pxCurrentCoRoutine,
#if (configMAX_DG_COROUTINE_NAME_LEN > 0)
                                pxCurrentCoRoutine->pcCoRoutineName);
#else
                                NULL);
#endif
                }
        }
#endif /* configCHECK_FOR_STACK_OVERFLOW */

#if (dgcrSTACK_TRACING == 1)
        {
                uint16_t uxDgCRStackHighWaterMark;
                uint8_t *pucDgCRStackStart;

                dgcrENTER_CRITICAL();

                uxDgCRStackHighWaterMark = prvGetFreeStackSpace();
                pucDgCRStackStart = (uint8_t *)(portGET_SP());

                if (uxDgCRStackHighWaterMark < pxCurrentCoRoutine->usStackHighWaterMark) {
                        pxCurrentCoRoutine->usStackHighWaterMark = uxDgCRStackHighWaterMark;
                }

                /* Initialise stack up to stack high water mark position. */
#if (portSTACK_GROWTH < 0)
                prvInitialiseStack(pucDgCRStackStart,
                        (uint8_t *)(portSTACK_LIMIT) + ((uint32_t)uxDgCRStackHighWaterMark) * sizeof(StackType_t));
#else
                prvInitialiseStack(pucDgCRStackStart,
                        (uint8_t *)(portSTACK_LIMIT) - sizeof(StackType_t) - ((uint32_t)uxDgCRStackHighWaterMark) * sizeof(StackType_t));
#endif

                dgcrEXIT_CRITICAL();
        }
#endif /* dgcrSTACK_TRACING */
}
/*-----------------------------------------------------------*/

#if (configSUPPORT_STATIC_ALLOCATION == 1)

CoRoutineHandle_t xDgCoRoutineCreateStatic(crCOROUTINE_CODE pxCoRoutineCode,
        const char * const pcName, UBaseType_t uxPriority, UBaseType_t uxIndex,
        StaticDgCoRoutine_t * const pxCoRoutineBuffer)
{
        CoRoutineHandle_t xReturn;

        configASSERT(pxCoRoutineBuffer != NULL);

        if (pxCoRoutineBuffer != NULL) {
                CRCB_t *pxCoRoutine = (CRCB_t *)pxCoRoutineBuffer;

                /* Initialise co-routine. */
                prvInitialiseNewDgCoRoutine(pxCoRoutineCode, pcName, uxIndex, uxPriority,
                        pxCoRoutine);

#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
                /* Indicate this co-routine was created statically in case it is later deleted. */
                pxCoRoutine->ucStaticallyAllocated = pdTRUE;
#endif /* configSUPPORT_DYNAMIC_ALLOCATION */

                /* Add co-routine to the ready list at the correct priority. */
                prvAddNewCoRoutineToReadyQueue(pxCoRoutine);

                xReturn = pxCoRoutine;
        } else {
                xReturn = NULL;
        }

        return xReturn;
}
#endif /* configSUPPORT_STATIC_ALLOCATION */
/*-----------------------------------------------------------*/

#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)

BaseType_t xDgCoRoutineCreate(crCOROUTINE_CODE pxCoRoutineCode,
        const char * const pcName, UBaseType_t uxPriority, UBaseType_t uxIndex,
        CoRoutineHandle_t * const pxCreatedCoRoutine)
{
        BaseType_t xReturn;
        CRCB_t *pxCoRoutine;

        /* Allocate the memory for the co-routine control block. */
        pxCoRoutine = (CRCB_t *)pvPortMalloc(sizeof(CRCB_t));

        if (pxCoRoutine != NULL) {
                /* Initialise co-routine. */
                prvInitialiseNewDgCoRoutine(pxCoRoutineCode, pcName, uxIndex, uxPriority,
                        pxCoRoutine);

#if (configSUPPORT_STATIC_ALLOCATION == 1)
                /* Indicate this co-routine was not created statically in case it is later deleted. */
                pxCoRoutine->ucStaticallyAllocated = pdFALSE;
#endif /* configSUPPORT_STATIC_ALLOCATION */

                /* Add co-routine to the ready list at the correct priority. */
                prvAddNewCoRoutineToReadyQueue(pxCoRoutine);

                if (pxCreatedCoRoutine != NULL) {
                        *pxCreatedCoRoutine = (CoRoutineHandle_t)pxCoRoutine;
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }

                xReturn = pdPASS;
        } else {
                xReturn = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
        }

        return xReturn;
}
#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
/*-----------------------------------------------------------*/

static void prvInitialiseNewDgCoRoutine(crCOROUTINE_CODE pxCoRoutineCode, const char * const pcName,
        UBaseType_t uxIndex, UBaseType_t uxPriority, CRCB_t *pxNewCRCB)
{
        /* Initialise basic co-routine control block structure. */
        prvInitialiseNewCoRoutine(pxCoRoutineCode, uxIndex, uxPriority,
                (CoRoutineHandle_t)pxNewCRCB);

#if (configUSE_DG_COROUTINE_DEBUG_FACILITY == 1)
        portDGCOROUTINE_DEBUG_FACILITY_SET_TOPOFSTACK(pxNewCRCB);
#endif
#if (configRECORD_DG_COROUTINE_BLOCKED_PC == 1)
        pxNewCRCB->pxBlockedPC = NULL;
#endif

#if (configMAX_DG_COROUTINE_NAME_LEN > 0)
        {
                UBaseType_t x;

                /* Store the co-routine name in the CRCB. */
                for (x = 0; x < (UBaseType_t) configMAX_DG_COROUTINE_NAME_LEN; x++) {
                        pxNewCRCB->pcCoRoutineName[x] = pcName[x];

                        if (pcName[x] == 0x00) {
                                break;
                        }
                }

                /* Ensure string is terminated in all cases. */
                pxNewCRCB->pcCoRoutineName[configMAX_DG_COROUTINE_NAME_LEN - 1] = '\0';
        }
#endif /* configMAX_DG_COROUTINE_NAME_LEN */

#if (configUSE_DG_COROUTINE_NOTIFICATIONS == 1)
        pxNewCRCB->ulNotifiedValue = 0;
        pxNewCRCB->ucNotifyState = dgcrNOT_WAITING_NOTIFICATION;
#endif /* configUSE_DG_COROUTINE_NOTIFICATIONS */
#if (configUSE_MUTEXES == 1)
        pxNewCRCB->uxBasePriority = uxPriority;
        pxNewCRCB->uxMutexesHeld = 0;
#endif /* configUSE_MUTEXES */
#if (configGENERATE_RUN_TIME_STATS == 1)
        pxNewCRCB->ulRunTimeCounter = 0UL;
#endif /* configGENERATE_RUN_TIME_STATS */

#if ((dgcrSTACK_TRACING == 1) || (INCLUDE_pxDgCoRoutineGetStackStart == 1))
#if (portSTACK_GROWTH < 0)
        pxNewCRCB->pxEndOfStack = NULL;
#if (configUSE_TRACE_FACILITY == 1)
        pxNewCRCB->pxStack = (StackType_t *)(portSTACK_LIMIT);
#endif
#else
#if (configRECORD_STACK_HIGH_ADDRESS == 1)
        pxNewCRCB->pxEndOfStack = (StackType_t *)(portSTACK_LIMIT);
#endif
        pxNewCRCB->pxStack = NULL;
#endif /* portSTACK_GROWTH */
#endif /* dgcrSTACK_TRACING || INCLUDE_pxDgCoRoutineGetStackStart */

#if (dgcrSTACK_TRACING == 1)
        pxNewCRCB->usStackHighWaterMark = (uint16_t)(-1);
#endif /* dgcrSTACK_TRACING */
}
/*-----------------------------------------------------------*/

static void prvAddNewCoRoutineToReadyQueue(CRCB_t *pxNewCRCB)
{
        /* Disable interrupts while co-routine lists are being updated. */
        dgcrENTER_CRITICAL();
        {
                uxCurrentNumberOfDgCoRoutines++;

                /*
                 * If pxCurrentCoRoutine is NULL, then this is the first co-routine to be created.
                 * Make this the current co-routine and initialise co-routine data structures.
                 */
                if (pxCurrentCoRoutine == NULL) {
                        pxCurrentCoRoutine = pxNewCRCB;
                        prvInitialiseCoRoutineLists();
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }

#if (configUSE_TRACE_FACILITY == 1)
                uxDgCoRoutineNumber++;

                /* Set CRCB number for tracing only. */
                pxNewCRCB->uxCRCBNumber = uxDgCoRoutineNumber;
#endif /* configUSE_TRACE_FACILITY */

                prvAddCoRoutineToReadyQueue(pxNewCRCB);

                if (xSchedulerRunning != dgcrSCHEDULER_NOT_RUNNING) {
                        /*
                         * If the created co-routine is of a higher priority than the current
                         * co-routine, then pend yielding.
                         */
                        if (pxCurrentCoRoutine->uxPriority < pxNewCRCB->uxPriority) {
                                vDgCoRoutineMissedYield();
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        }
        dgcrEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

#if (INCLUDE_vDgCoRoutineDelete == 1)

void vDgCoRoutineDelete(CoRoutineHandle_t xCoRoutine)
{
        CRCB_t *pxCRCB;

        /* If xCoRoutine is NULL, then the calling co-routine is implied. */
        pxCRCB = prvGetDgCRCBFromHandle(xCoRoutine);

        /* If the co-routine is not the calling one, delete it, otherwise the scheduler will do so. */
        if (pxCRCB != pxCurrentCoRoutine) {
                prvDeleteDgCRCB(pxCRCB);
        } else {
                xDeleteDgCoRoutineHandle = pxCurrentCoRoutine;
        }
}
#endif /* INCLUDE_vDgCoRoutineDelete */
/*-----------------------------------------------------------*/

#if (INCLUDE_vDgCoRoutineDelayUntil == 1)

TickType_t vDgCoRoutineCalcTimeUntil(TickType_t * const pxRefTime, const TickType_t xTimeIncrement)
{
        TickType_t xResultTime;
        TickType_t xTimeDiff = 0;

        configASSERT(pxRefTime);
        configASSERT(xTimeIncrement > 0U);

        const TickType_t xConstTickCount = xDgCoRoutineGetTickCount();

        /* Calculate the tick time implied by reference time and given offset. */
        xResultTime = *pxRefTime + xTimeIncrement;

        /* Check if tick time has overflowed, and calculate time difference accordingly. */
        if (xConstTickCount < *pxRefTime) {
                if ((xResultTime < *pxRefTime) && (xResultTime > xConstTickCount)) {
                        xTimeDiff = xResultTime - xConstTickCount;
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        } else {
                if ((xResultTime < *pxRefTime) || (xResultTime > xConstTickCount)) {
                        xTimeDiff = xResultTime - xConstTickCount;
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        }

        /* Update the reference time for the next call. */
        *pxRefTime = xResultTime;

        return xTimeDiff;
}
#endif /* INCLUDE_vDgCoRoutineDelayUntil */
/*-----------------------------------------------------------*/

#if (INCLUDE_eDgCoRoutineGetState == 1) || (configUSE_TRACE_FACILITY == 1)

eDgCoRoutineState eDgCoRoutineGetState(CoRoutineHandle_t xCoRoutine)
{
        eDgCoRoutineState eReturn;
        List_t *pxStateList;
        const CRCB_t * const pxCRCB = (CRCB_t *)xCoRoutine;

        configASSERT(pxCRCB);

        if (pxCRCB == pxCurrentCoRoutine) {
                /* The calling co-routine is querying its own state. */
                eReturn = eDgCrRunning;
        } else {
                dgcrENTER_CRITICAL();
                {
                        pxStateList = (List_t *)listLIST_ITEM_CONTAINER(&(pxCRCB->xGenericListItem));
                }
                dgcrEXIT_CRITICAL();

#if (INCLUDE_vDgCoRoutineDelete == 1)
                if (pxStateList == NULL) {
                        eReturn = eDgCrDeleted;
                } else
#endif /* INCLUDE_vDgCoRoutineDelete */
                if ((pxStateList == pxDelayedCoRoutineList)
                        || (pxStateList == pxOverflowDelayedCoRoutineList)) {
                        /* The co-routine being queried is referenced by a Blocked list. */
                        eReturn = eDgCrBlocked;
                } else  {
                        eReturn = eDgCrReady;
                }
        }

        return eReturn;
}
#endif /* INCLUDE_eDgCoRoutineGetState || configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if (INCLUDE_uxDgCoRoutinePriorityGet == 1)

UBaseType_t uxDgCoRoutinePriorityGet(CoRoutineHandle_t xCoRoutine)
{
        CRCB_t *pxCRCB;
        UBaseType_t uxReturn;

        dgcrENTER_CRITICAL();
        {
                /* If xCoRoutine is NULL, then get the priority of the calling co-routine. */
                pxCRCB = prvGetDgCRCBFromHandle(xCoRoutine);
                uxReturn = pxCRCB->uxPriority;
        }
        dgcrEXIT_CRITICAL();

        return uxReturn;
}
#endif /* INCLUDE_uxDgCoRoutinePriorityGet */
/*-----------------------------------------------------------*/

#if (INCLUDE_uxDgCoRoutinePriorityGet == 1)

UBaseType_t uxDgCoRoutinePriorityGetFromISR(CoRoutineHandle_t xCoRoutine)
{
        CRCB_t *pxCRCB;
        UBaseType_t uxReturn, uxSavedInterruptState;

        /* Safety check imposed by FreeRTOS kernel */
        portASSERT_IF_INTERRUPT_PRIORITY_INVALID();

        uxSavedInterruptState = dgcrENTER_CRITICAL_FROM_ISR();
        {
                /* If xCoRoutine is NULL, then get the priority of the current co-routine. */
                pxCRCB = prvGetDgCRCBFromHandle(xCoRoutine);
                uxReturn = pxCRCB->uxPriority;
        }
        dgcrEXIT_CRITICAL_FROM_ISR(uxSavedInterruptState);

        return uxReturn;
}
#endif /* INCLUDE_uxDgCoRoutinePriorityGet */
/*-----------------------------------------------------------*/

void vDgCoRoutineStartScheduler(void)
{
        BaseType_t xReturn;

        /* Add the idle co-routine at the lowest priority. */
#if (configSUPPORT_STATIC_ALLOCATION == 1)
        {
                /*
                 * The Idle co-routine is created with statically allocated control block
                 * data structure.
                 */
                xIdleDgCoRoutineHandle = xDgCoRoutineCreateStatic(prvIdleDgCoRoutine,
                                                configIDLE_DG_COROUTINE_NAME,
                                                dgcrIDLE_PRIORITY,
                                                0,
                                                &xIdleDgCoRoutineBuffer);
                if (xIdleDgCoRoutineHandle != NULL) {
                        xReturn = pdPASS;
                } else {
                        xReturn = pdFAIL;
                }
        }
#else
        {
                /*
                 * The Idle co-routine is created with dynamically allocated control block
                 * data structure.
                 */
                xReturn = xDgCoRoutineCreate(prvIdleDgCoRoutine,
                                        configIDLE_DG_COROUTINE_NAME,
                                        dgcrIDLE_PRIORITY,
                                        0,
                                        &xIdleDgCoRoutineHandle);
        }
#endif /* configSUPPORT_STATIC_ALLOCATION */

#if (configUSE_TIMERS == 1)
        {
                if (xReturn == pdPASS) {
                        xReturn = xTimerCreateTimerDgCoRoutine();
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        }
#endif /* configUSE_TIMERS */

        if (xReturn == pdPASS) {
                /* freertos_tasks_c_additions_init() should only be called if the user
                definable macro FREERTOS_TASKS_C_ADDITIONS_INIT() is defined, as that is
                the only macro called by the function. */
                #ifdef FREERTOS_TASKS_C_ADDITIONS_INIT
                {
                        freertos_tasks_c_additions_init();
                }
                #endif

                /* Disable interrupts, ensuring a tick does not occur when the scheduler is started. */
                dgcrDISABLE_INTERRUPTS();

#if (configCHECK_FOR_STACK_OVERFLOW > 0) || (dgcrSTACK_TRACING == 1)
                /* Initialise stack by filling it with a known value to assist debugging. */
#if (portSTACK_GROWTH < 0)
                prvInitialiseStack((uint8_t *)(portGET_SP()), (uint8_t *)(portSTACK_LIMIT));
#else
                prvInitialiseStack((uint8_t *)(portGET_SP()), (uint8_t *)(portSTACK_LIMIT) - sizeof(StackType_t));
#endif
#endif /* configCHECK_FOR_STACK_OVERFLOW || dgcrSTACK_TRACING */

                xNextDgCoRoutineUnblockTime = portMAX_DELAY;
                xSchedulerRunning = dgcrSCHEDULER_RUNNING_IN_BACKGROUND;
                xTickCount = (TickType_t) 0U;

                /* Configure the timer/counter used to generate the run time counter time base. */
                portCONFIGURE_TIMER_FOR_RUN_TIME_STATS();

                /* Setup timer tick and call vCoRoutineSchedule(). Interrupts have been disabled. */
                xPortStartScheduler();

                /*
                 * The scheduler should not be stopped. This assertion is here to indicate that
                 * xPortStartScheduler() has returned.
                 */
                configASSERT(0);
        } else {
                /*
                 * This assertion is here to indicate that there was not enough heap to create the
                 * Idle co-routine or the timer daemon co-routine.
                 */
                configASSERT(xReturn != errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY);
        }

        /*
         * Prevent compiler warnings if INCLUDE_xDgCoRoutineGetIdleCoRoutineHandle is set to 0,
         * that is xIdleDgCoRoutineHandle is not used anywhere else.
         */
         (void) xIdleDgCoRoutineHandle;
}
/*-----------------------------------------------------------*/

#if (configUSE_TICKLESS_IDLE != 0)

static TickType_t prvGetExpectedIdleTime(void)
{
        TickType_t xReturn;
        const TickType_t xConstTickCount = xDgCoRoutineGetTickCount();

        /*
         * If there is only one idle priority co-routine (i.e. the idle co-routine), there is no
         * pending yielding and next co-routine unblock time is greater than current tick count,
         * then calculate expected idle time.
         */
        if ((listCURRENT_LIST_LENGTH(&(pxReadyCoRoutineLists[dgcrIDLE_PRIORITY])) < 2)
                && (xYieldPending == pdFALSE) && (xNextDgCoRoutineUnblockTime > xConstTickCount)) {
                xReturn = xNextDgCoRoutineUnblockTime - xConstTickCount;
        } else {
                xReturn = 0;
        }

        return xReturn;
}
#endif /* configUSE_TICKLESS_IDLE */
/*-----------------------------------------------------------*/

TickType_t xDgCoRoutineGetTickCount(void)
{
        TickType_t xTicks;

        /* Critical section is required if running on a 16 bit processor. */
        portTICK_TYPE_ENTER_CRITICAL();
        {
                xTicks = xTickCount;
        }
        portTICK_TYPE_EXIT_CRITICAL();

        return xTicks;
}
/*-----------------------------------------------------------*/

TickType_t xDgCoRoutineGetTickCountFromISR(void)
{
        TickType_t xReturn;
        UBaseType_t uxSavedInterruptStatus;

        /* Safety check imposed by FreeRTOS kernel */
        portASSERT_IF_INTERRUPT_PRIORITY_INVALID();

        /* Critical section is required if running on a 16 bit processor. */
        uxSavedInterruptStatus = portTICK_TYPE_SET_INTERRUPT_MASK_FROM_ISR();
        {
                xReturn = xTickCount;
        }
        portTICK_TYPE_CLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedInterruptStatus );

        return xReturn;
}
/*-----------------------------------------------------------*/

UBaseType_t uxDgCoRoutineGetNumberOfCoRoutines(void)
{
        return uxCurrentNumberOfDgCoRoutines;
}
/*-----------------------------------------------------------*/

#if (INCLUDE_pcDgCoRoutineGetName == 1) && (configMAX_DG_COROUTINE_NAME_LEN > 0)

char *pcDgCoRoutineGetName(CoRoutineHandle_t xCoRoutineToQuery)
{
        CRCB_t *pxCRCB;

        /* If xCoRoutineToQuery is NULL, then get the name of the calling co-routine. */
        pxCRCB = prvGetDgCRCBFromHandle(xCoRoutineToQuery);
        configASSERT(pxCRCB);
        return &(pxCRCB->pcCoRoutineName[0]);
}
#endif /* INCLUDE_pcDgCoRoutineGetName && configMAX_DG_COROUTINE_NAME_LEN */
/*-----------------------------------------------------------*/

#if (INCLUDE_xDgCoRoutineGetHandle == 1) && (configMAX_DG_COROUTINE_NAME_LEN > 0)

static CRCB_t *prvSearchForNameWithinSingleList(List_t *pxList, const char pcNameToQuery[])
{
        CRCB_t *pxNextCRCB, *pxFirstCRCB, *pxReturn = NULL;
        UBaseType_t x;
        char cNextChar;

        if (listCURRENT_LIST_LENGTH(pxList) > (UBaseType_t) 0) {
                listGET_OWNER_OF_NEXT_ENTRY(pxFirstCRCB, pxList);

                do {
                        listGET_OWNER_OF_NEXT_ENTRY(pxNextCRCB, pxList);

                        /* Check co-routine name for a match charachter-by-charachter. */
                        for (x = 0; x < (UBaseType_t) configMAX_DG_COROUTINE_NAME_LEN; x++) {
                                cNextChar = pxNextCRCB->pcCoRoutineName[x];

                                if (cNextChar != pcNameToQuery[x]) {
                                        /* Character does not match. */
                                        break;
                                } else if (cNextChar == 0x00) {
                                        /* A match has been found, since both strings have terminated. */
                                        pxReturn = pxNextCRCB;
                                        break;
                                } else {
                                        mtCOVERAGE_TEST_MARKER();
                                }
                        }

                        if (pxReturn != NULL) {
                                /* Co-routine has been found. */
                                break;
                        }
                } while (pxNextCRCB != pxFirstCRCB);
        } else {
                mtCOVERAGE_TEST_MARKER();
        }

        return pxReturn;
}
#endif /* INCLUDE_xDgCoRoutineGetHandle && configMAX_DG_COROUTINE_NAME_LEN */
/*-----------------------------------------------------------*/

#if (INCLUDE_xDgCoRoutineGetHandle == 1) && (configMAX_DG_COROUTINE_NAME_LEN > 0)

CoRoutineHandle_t xDgCoRoutineGetHandle(const char *pcNameToQuery)
{
        UBaseType_t uxQueue = configMAX_CO_ROUTINE_PRIORITIES;
        CRCB_t* pxCRCB;

        /* Co-routine names will be truncated to configMAX_DG_COROUTINE_NAME_LEN - 1 bytes. */
        configASSERT(strlen(pcNameToQuery) < configMAX_DG_COROUTINE_NAME_LEN);

        /* Search the ready lists. */
        do {
                uxQueue--;
                pxCRCB = prvSearchForNameWithinSingleList((List_t *)&(pxReadyCoRoutineLists[uxQueue]),
                                pcNameToQuery);

                if (pxCRCB != NULL) {
                        /* Co-routine has been found. */
                        break;
                }
        } while (uxQueue > (UBaseType_t) dgcrIDLE_PRIORITY);

        /* Search the delayed lists. */
        if (pxCRCB == NULL) {
                pxCRCB = prvSearchForNameWithinSingleList((List_t *)pxDelayedCoRoutineList,
                                pcNameToQuery);
        }

        if (pxCRCB == NULL) {
                pxCRCB = prvSearchForNameWithinSingleList((List_t *)pxOverflowDelayedCoRoutineList,
                                pcNameToQuery);
        }

        return (CoRoutineHandle_t) pxCRCB;
}

#endif /* INCLUDE_xDgCoRoutineGetHandle && configMAX_DG_COROUTINE_NAME_LEN */
/*-----------------------------------------------------------*/

#if (configUSE_TRACE_FACILITY == 1)

UBaseType_t uxDgCoRoutineGetSystemState(DgCoRoutineStatus_t * const pxCoRoutineStatusArray,
        const UBaseType_t uxArraySize, uint32_t * const pulTotalRunTime)
{
        UBaseType_t uxCoRoutineIdx = 0, uxQueue = configMAX_CO_ROUTINE_PRIORITIES;

        if (uxArraySize < uxCurrentNumberOfDgCoRoutines) {
                return 0;
        }

        dgcrENTER_CRITICAL();

        /* For each co-routine in the Ready state fill in a DgCoRoutineStatus_t structure. */
        do {
                uxQueue--;
                uxCoRoutineIdx += prvListDgCoRoutinesWithinSingleList(&(pxCoRoutineStatusArray[uxCoRoutineIdx]),
                                        &(pxReadyCoRoutineLists[uxQueue]), eDgCrReady);
        } while (uxQueue > (UBaseType_t) dgcrIDLE_PRIORITY);

        /* For each co-routine in the Blocked state fill in a DgCoRoutineStatus_t structure. */
        uxCoRoutineIdx += prvListDgCoRoutinesWithinSingleList(&(pxCoRoutineStatusArray[uxCoRoutineIdx]),
                                (List_t *) pxDelayedCoRoutineList, eDgCrBlocked);
        uxCoRoutineIdx += prvListDgCoRoutinesWithinSingleList(&(pxCoRoutineStatusArray[uxCoRoutineIdx]),
                                (List_t *) pxOverflowDelayedCoRoutineList, eDgCrBlocked);

        if (pulTotalRunTime != NULL) {
#if (configGENERATE_RUN_TIME_STATS == 1)
#ifdef portALT_GET_RUN_TIME_COUNTER_VALUE
                portALT_GET_RUN_TIME_COUNTER_VALUE(*pulTotalRunTime);
#else
                *pulTotalRunTime = portGET_RUN_TIME_COUNTER_VALUE();
#endif
#else
                *pulTotalRunTime = 0;
#endif
        }

        dgcrEXIT_CRITICAL();

        return uxCoRoutineIdx;
}
#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if (INCLUDE_xDgCoRoutineGetIdleCoRoutineHandle == 1)

CoRoutineHandle_t xDgCoRoutineGetIdleCoRoutineHandle(void)
{
        /*
         * This assertion is to indicate that xDgCoRoutineGetIdleCoRoutineHandle() should be
         * called before the scheduler is started.
         */
        configASSERT((xIdleDgCoRoutineHandle != NULL));
        return xIdleDgCoRoutineHandle;
}
#endif /* INCLUDE_xDgCoRoutineGetIdleCoRoutineHandle */
/*-----------------------------------------------------------*/

#if (configUSE_TICKLESS_IDLE != 0)

void vDgCoRoutineStepTick(const TickType_t xTicksToJump)
{
        const TickType_t xConstTickCount = xDgCoRoutineGetTickCount();

        configASSERT((xConstTickCount + xTicksToJump) <= xNextDgCoRoutineUnblockTime);
        xTickCount = xConstTickCount + xTicksToJump;
        traceINCREASE_TICK_COUNT(xTicksToJump);
}
#endif /* configUSE_TICKLESS_IDLE */
/*-----------------------------------------------------------*/

void xDgCoRoutineIncrementTick(void)
{
        /* Check if tick count is to be updated or suppressed. */
        if (xSchedulerRunning == dgcrSCHEDULER_RUNNING_SLEEP) {
                ++uxPendedTicks;

                vDgCoRoutineMissedYield();

#if (configUSE_TICK_HOOK == 1)
                {
                        extern void vApplicationTickHook(void);

                        /* Call tick application hook function. */
                        vApplicationTickHook();
                }
#endif
        } else {
                /* Increment tick. */
                const TickType_t xConstTickCount = xDgCoRoutineGetTickCount() + (TickType_t) 1;
                xTickCount = xConstTickCount;

                /*
                 * If scheduler does not currently perform context switch, decide on marking that
                 * yielding is pending.
                 */
                if ((xSchedulerRunning != dgcrSCHEDULER_RUNNING_CONTEXT_SWITCH)
                        && (xYieldPending == pdFALSE)) {
                        /* If next co-routine unblock time is reached, pend yielding. */
                        if (xConstTickCount >= xNextDgCoRoutineUnblockTime) {
                                vDgCoRoutineMissedYield();
                        }
#if (configUSE_TIME_SLICING == 1)
                        else if (xDgCoRoutineGetTickCount() - xLastTickCount > 0) {
                                UBaseType_t uxPriority = pxCurrentCoRoutine->uxPriority;
                                UBaseType_t uxNumHighPriorityCoRoutines = 0;

                                while (uxPriority < configMAX_CO_ROUTINE_PRIORITIES) {
                                        uxNumHighPriorityCoRoutines +=
                                                listCURRENT_LIST_LENGTH(&(pxReadyCoRoutineLists[uxPriority]));
                                        if (uxNumHighPriorityCoRoutines > 1) {
                                                vDgCoRoutineMissedYield();
                                                break;
                                        }
                                        uxPriority++;
                                }
                        }
#endif /* configUSE_TIME_SLICING */
                }

#if (configUSE_TICK_HOOK == 1)
                /* Call tick application hook function only fot ticks that are not pending. */
                if (uxPendedTicks == (UBaseType_t) 0U) {
                        extern void vApplicationTickHook(void);

                        /* Call tick application hook function. */
                        vApplicationTickHook();
                }
#endif
        }
}
/*-----------------------------------------------------------*/

#if (configUSE_TIMERS == 1)

void vDgCoRoutinePlaceOnEventListRestricted(List_t * const pxEventList, TickType_t xTicksToWait)
{
        /*
         * Assuming that the calling co-routine is the only to wait on this event list, place its
         * event list item in the appropriate event list using the faster vListInsertEnd() function
         * instead of vListInsert().
         */
        vListInsertEnd(pxEventList, &(pxCurrentCoRoutine->xEventListItem));

        /* Add co-routine in the delayed list for the specified period. */
        vCoRoutineAddToDelayedList(xTicksToWait, NULL);
}
#endif /* configUSE_TIMERS */
/*-----------------------------------------------------------*/

void vDgCoRoutineMissedYield(void)
{
        xYieldPending = pdTRUE;
}
/*-----------------------------------------------------------*/

void vDgCoRoutineMissedYieldForPriority(UBaseType_t uxPriority)
{
        if (xSchedulerRunning != dgcrSCHEDULER_RUNNING_CONTEXT_SWITCH) {
                xYieldPending = pdTRUE;
        } else {
                UBaseType_t uxSavedInterruptStatus;

                /* Safety check imposed by FreeRTOS kernel */
                portASSERT_IF_INTERRUPT_PRIORITY_INVALID();

                uxSavedInterruptStatus = dgcrENTER_CRITICAL_FROM_ISR();

                if (xYieldContextSwitchMaxPriority < uxPriority) {
                        xYieldContextSwitchMaxPriority = uxPriority;
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }

                dgcrEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
        }
}
/*-----------------------------------------------------------*/

void vDgCoRoutineClearPendingYield(void)
{
        xYieldPending = pdFALSE;
}
/*-----------------------------------------------------------*/

BaseType_t vDgCoRoutineIsPendingYield(void)
{
        return xYieldPending;
}
/*-----------------------------------------------------------*/

void vDgCoRoutineSchedulerEnterContextSwitch(void)
{
        xYieldContextSwitchMaxPriority = dgcrIDLE_PRIORITY;

        xSchedulerRunning = dgcrSCHEDULER_RUNNING_CONTEXT_SWITCH;

        xYieldPending = pdFALSE;
}
/*-----------------------------------------------------------*/

void vDgCoRoutineSchedulerLeaveContextSwitch(void)
{
        xSchedulerRunning = dgcrSCHEDULER_RUNNING_IN_BACKGROUND;

        if (pxCurrentCoRoutine->uxPriority < xYieldContextSwitchMaxPriority) {
                xYieldPending = pdTRUE;
        }
}
/*-----------------------------------------------------------*/

#if (configUSE_TRACE_FACILITY == 1)

UBaseType_t uxDgCoRoutineGetCoRoutineNumber(CoRoutineHandle_t xCoRoutine)
{
        return ((xCoRoutine != NULL) ? ((CRCB_t *)xCoRoutine)->uxCoRoutineNumber : 0U);
}
#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if (configUSE_TRACE_FACILITY == 1)

void vDgCoRoutineSetCoRoutineNumber(CoRoutineHandle_t xCoRoutine, const UBaseType_t uxHandle)
{
        if (xCoRoutine != NULL) {
                ((CRCB_t *)xCoRoutine)->uxCoRoutineNumber = uxHandle;
        }
}
#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

/*
 * -----------------------------------------------------------
 * The Idle co-routine.
 * ----------------------------------------------------------
 */
static void prvIdleDgCoRoutine(CoRoutineHandle_t xHandle, UBaseType_t uxIndex)
{
        /* CoRoutine start point. */
        crSTART(xHandle);

        for ( ;; ) {
                /* Since there is no preemption, keep forcing a co-routine switch. */
                crSET_STATE0(xHandle);

#if (configIDLE_SHOULD_YIELD == 1)
                /*
                 * If a co-routine of idle priority other than the idle co-routine is ready to
                 * execute, then the idle co-routine should yield.
                 */
                if (listCURRENT_LIST_LENGTH(&(pxReadyCoRoutineLists[dgcrIDLE_PRIORITY])) > (UBaseType_t) 1) {
                        crSET_STATE0(xHandle);
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
#endif

#if (configUSE_IDLE_HOOK == 1)
                {
                        extern void vApplicationIdleHook(void);

                        /*
                         * Execute application code (if any) in the context of this co-routine at
                         * the point it starts executing.
                         */
                        vApplicationIdleHook();
                }
#endif /* configUSE_IDLE_HOOK */

#if (configUSE_TICKLESS_IDLE != 0)
                {
                        TickType_t xExpectedIdleTime;

                        /* Get expected idle time and check whether the system can go to sleep. */
                        xExpectedIdleTime = prvGetExpectedIdleTime();

                        if (xExpectedIdleTime >= configEXPECTED_IDLE_TIME_BEFORE_SLEEP) {
                                xSchedulerRunning = dgcrSCHEDULER_RUNNING_SLEEP;

                                /*
                                 * Now tick counter will not change. Get again expected idle time
                                 * and check whether the system can go to sleep.
                                 */
                                xExpectedIdleTime = prvGetExpectedIdleTime();

                                /*
                                 * Define the following macro to change xExpectedIdleTime and
                                 * control whether portSUPPRESS_TICKS_AND_SLEEP() will be called.
                                 */
                                configPRE_SUPPRESS_TICKS_AND_SLEEP_PROCESSING(xExpectedIdleTime);

                                if (xExpectedIdleTime >= configEXPECTED_IDLE_TIME_BEFORE_SLEEP) {
                                        traceLOW_POWER_IDLE_BEGIN();
                                        portSUPPRESS_TICKS_AND_SLEEP(xExpectedIdleTime);
                                        traceLOW_POWER_IDLE_END();
                                } else {
                                        mtCOVERAGE_TEST_MARKER();
                                }

                                dgcrENTER_CRITICAL();
                                {
                                        UBaseType_t uxPendedTicksCount = uxPendedTicks;

                                        xSchedulerRunning = dgcrSCHEDULER_RUNNING_IN_BACKGROUND;

                                        /*
                                         * If any ticks occurred while being in sleep state, then
                                         * they should be processed now. This ensures the tick count
                                         * is correct.
                                         */
                                        while (uxPendedTicksCount > (UBaseType_t) 0U) {
                                                xDgCoRoutineIncrementTick();
                                                --uxPendedTicksCount;
                                        }

                                        uxPendedTicks = 0;
                                }
                                dgcrEXIT_CRITICAL();
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
                }
#endif /* configUSE_TICKLESS_IDLE */
        }

        /* CoRoutine end point. */
        crEND();
}
/*-----------------------------------------------------------*/

#if (configUSE_TICKLESS_IDLE != 0)

eDgCoRoutineSleepModeStatus eDgCoRoutineConfirmSleepModeStatus(void)
{
        /* The idle co-routine exists in addition to the application co-routines. */
        eDgCoRoutineSleepModeStatus eReturn = eDgCrStandardSleep;

        if (listCURRENT_LIST_LENGTH(&xPendingReadyCoRoutineList) != 0) {
                /* There are pending co-routines to execute. */
                eReturn = eDgCrAbortSleep;
        } else if (xYieldPending != pdFALSE) {
                /* Yielding has been pended. */
                eReturn = eDgCrAbortSleep;
        } else {
                mtCOVERAGE_TEST_MARKER();
        }

        return eReturn;
}
#endif /* configUSE_TICKLESS_IDLE */
/*-----------------------------------------------------------*/

#if (configUSE_TRACE_FACILITY == 1)

void vDgCoRoutineGetInfo(CoRoutineHandle_t xCoRoutine, DgCoRoutineStatus_t *pxCoRoutineStatus,
        eDgCoRoutineState eState)
{
        CRCB_t *pxCRCB;

        /* If xCoRoutine is NULL, then the calling co-routine is implied. */
        pxCRCB = prvGetDgCRCBFromHandle(xCoRoutine);

        pxCoRoutineStatus->xHandle = (CoRoutineHandle_t)pxCRCB;
#if (configMAX_DG_COROUTINE_NAME_LEN > 0)
        pxCoRoutineStatus->pcCoRoutineName = (const char *)&(pxCRCB->pcCoRoutineName[0]);
#else
        pxCoRoutineStatus->pcCoRoutineName = NULL;
#endif
        pxCoRoutineStatus->uxPriority = pxCRCB->uxPriority;
        pxCoRoutineStatus->xCoRoutineNumber = pxCRCB->uxCRCBNumber;

#if (configUSE_MUTEXES == 1)
        pxCoRoutineStatus->uxBasePriority = pxCRCB->uxBasePriority;
#else
        pxCoRoutineStatus->uxBasePriority = 0;
#endif

#if (configGENERATE_RUN_TIME_STATS == 1)
        pxCoRoutineStatus->ulRunTimeCounter = pxCRCB->ulRunTimeCounter;
#else
        pxCoRoutineStatus->ulRunTimeCounter = 0;
#endif

        /*
         * Obtain the co-routine state if eState is eDgCrInvalid, otherwise the state is just set
         * to whatever value is passed in.
         */
        if (eState != eDgCrInvalid) {
                if (pxCRCB == pxCurrentCoRoutine) {
                        pxCoRoutineStatus->eCurrentState = eDgCrRunning;
                } else {
                        pxCoRoutineStatus->eCurrentState = eState;
                }
        } else {
                pxCoRoutineStatus->eCurrentState = eDgCoRoutineGetState(pxCRCB);
        }

        if (pxCRCB->usStackHighWaterMark == (uint16_t)(-1)) {
                pxCRCB->usStackHighWaterMark = prvGetFreeStackSpace();
        }
#if (configRECORD_STACK_HIGH_ADDRESS == 1)
        pxCoRoutineStatus->pxStackEnd = pxCRCB->pxEndOfStack;
#else
        pxCoRoutineStatus->pxStackEnd = NULL;
#endif
        pxCoRoutineStatus->pxStackBase = pxCRCB->pxStack;
        pxCoRoutineStatus->usStackHighWaterMark = pxCRCB->usStackHighWaterMark;
}
#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if (configUSE_TRACE_FACILITY == 1)

static UBaseType_t prvListDgCoRoutinesWithinSingleList(DgCoRoutineStatus_t *pxCoRoutineStatusArray,
        List_t *pxList, eDgCoRoutineState eState)
{
        UBaseType_t uxCoRoutine = 0;

        if (listCURRENT_LIST_LENGTH(pxList) > (UBaseType_t) 0) {
                configLIST_VOLATILE CRCB_t *pxNextCRCB, *pxFirstCRCB;
                listGET_OWNER_OF_NEXT_ENTRY(pxFirstCRCB, pxList);

                /*
                 * Populate a DgCoRoutineStatus_t structure within the pxCoRoutineStatusArray
                 * array for each co-routine in pxList.
                 */
                do {
                        listGET_OWNER_OF_NEXT_ENTRY(pxNextCRCB, pxList);
                        vDgCoRoutineGetInfo((CoRoutineHandle_t)pxNextCRCB,
                                &(pxCoRoutineStatusArray[uxCoRoutine]), eState);
                        uxCoRoutine++;
                } while (pxNextCRCB != pxFirstCRCB);
        }

        return uxCoRoutine;
}
#endif /* configUSE_TRACE_FACILITY */
/*-----------------------------------------------------------*/

#if (dgcrSTACK_TRACING == 1)

static uint16_t prvGetFreeStackSpace(void)
{
        uint32_t ulCount = 0;
#if (portSTACK_GROWTH < 0)
        uint8_t *pucStackByte = (uint8_t*)(portSTACK_LIMIT);
#else
        uint8_t *pucStackByte = (uint8_t*)(portSTACK_LIMIT) - 1;
#endif

        while (*pucStackByte == (uint8_t) dgcrSTACK_FILL_BYTE) {
                pucStackByte -= portSTACK_GROWTH;
                ulCount++;
        }

        ulCount /= (uint32_t) sizeof(StackType_t);

        return (uint16_t) ulCount;
}
#endif /* dgcrSTACK_TRACING */
/*-----------------------------------------------------------*/

#if (INCLUDE_uxDgCoRoutineGetStackHighWaterMark == 1)

UBaseType_t uxDgCoRoutineGetStackHighWaterMark(CoRoutineHandle_t xCoRoutine)
{
        CRCB_t *pxCRCB;
        uint16_t uxDgCRStackHighWaterMark;

        pxCRCB = prvGetDgCRCBFromHandle(xCoRoutine);

        if (pxCurrentCoRoutine == pxCRCB) {
                uxDgCRStackHighWaterMark = prvGetFreeStackSpace();
        } else {
                uxDgCRStackHighWaterMark = pxCRCB->usStackHighWaterMark;
        }

        return (UBaseType_t) uxDgCRStackHighWaterMark;
}
#endif /* INCLUDE_uxDgCoRoutineGetStackHighWaterMark */
/*-----------------------------------------------------------*/

#if (INCLUDE_pxDgCoRoutineGetStackStart == 1)

uint8_t *pxDgCoRoutineGetStackStart(CoRoutineHandle_t xCoRoutine)
{
        CRCB_t *pxCRCB;

        pxCRCB = prvGetDgCRCBFromHandle(xCoRoutine);

        return (uint8_t *)pxCRCB->pxStartOfStack;
}
#endif /* INCLUDE_pxDgCoRoutineGetStackStart */
/*-----------------------------------------------------------*/

#if (INCLUDE_vDgCoRoutineDelete == 1)

static void prvDeleteDgCRCB(CRCB_t *pxCRCB)
{
        dgcrENTER_CRITICAL();
        {
                /* Remove co-routine from the ready list. */
                uxListRemove(&(pxCRCB->xGenericListItem));

                /* Check if the co-routine is also waiting for an event. */
                if (listLIST_ITEM_CONTAINER(&(pxCRCB->xEventListItem)) != NULL) {
                        /* Remove co-routine from the event list. */
                        (void) uxListRemove(&(pxCRCB->xEventListItem));
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }

#if (configUSE_TRACE_FACILITY == 1)
                /*
                 * Increment uxCoRoutineNumber so that kernel aware debuggers can detect that the
                 * co-routine lists need re-generating.
                 */
                uxDgCoRoutineNumber++;
#endif
                --uxCurrentNumberOfDgCoRoutines;

#if (configSUPPORT_DYNAMIC_ALLOCATION == 1)
#if (configSUPPORT_STATIC_ALLOCATION == 1)
                /* Check if the co-routine control block has been dynamically allocated. */
                if (pxCRCB->ucStaticallyAllocated == pdFALSE)
#endif
                {
                        /*
                         * Free dynamically allocated memory for co-routine control block
                         * structure.
                         */
                        vPortFree(pxCRCB);
                }
#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
        }
        dgcrEXIT_CRITICAL();
}
#endif /* INCLUDE_vDgCoRoutineDelete */
/*-----------------------------------------------------------*/

void vResetNextDgCoRoutineUnblockTime(void)
{
        if (listLIST_IS_EMPTY(pxDelayedCoRoutineList) != pdFALSE) {
                /*
                 * Set xNextDgCoRoutineUnblockTime to the maximum possible value, because
                 * the new current delayed list is empty.
                 */
                xNextDgCoRoutineUnblockTime = portMAX_DELAY;
        } else {
                /*
                 * Set xNextDgCoRoutineUnblockTime to the time at which the co-routine at the head
                 * of the delayed list should be removed from the Blocked state.
                 */
                CRCB_t *pxCRCB = (CRCB_t *)listGET_OWNER_OF_HEAD_ENTRY(pxDelayedCoRoutineList);
                xNextDgCoRoutineUnblockTime = listGET_LIST_ITEM_VALUE(&(pxCRCB->xGenericListItem));
        }
}
/*-----------------------------------------------------------*/

#if (INCLUDE_xDgCoRoutineGetCurrentCoRoutineHandle == 1) || (configUSE_MUTEXES == 1)

CoRoutineHandle_t xDgCoRoutineGetCurrentCoRoutineHandle(void)
{
        CoRoutineHandle_t xReturn;

        xReturn = pxCurrentCoRoutine;

        return xReturn;
}
#endif /* INCLUDE_xDgCoRoutineGetCurrentCoRoutineHandle || configUSE_MUTEXES */
/*-----------------------------------------------------------*/

#if (INCLUDE_xDgCoRoutineGetSchedulerState == 1) || (configUSE_TIMERS == 1)

BaseType_t xDgCoRoutineGetSchedulerState(void)
{
        BaseType_t xReturn;

        if (xSchedulerRunning == dgcrSCHEDULER_NOT_RUNNING) {
                xReturn = dgcrSCHEDULER_NOT_STARTED;
        } else if (xSchedulerRunning == dgcrSCHEDULER_RUNNING_SLEEP) {
                xReturn = dgcrSCHEDULER_SUSPENDED;
        } else {
                xReturn = dgcrSCHEDULER_RUNNING;
        }

        return xReturn;
}

#endif /* INCLUDE_xDgCoRoutineGetSchedulerState || configUSE_TIMERS */
/*-----------------------------------------------------------*/

#if (configUSE_MUTEXES == 1)

BaseType_t xDgCoRoutinePriorityInherit(CoRoutineHandle_t const pxMutexHolder)
{
        CRCB_t * const pxMutexHolderCRCB = (CRCB_t *)pxMutexHolder;
        BaseType_t xReturn = pdFALSE;

        /* Check that it is a valid mutex holder. */
        if (pxMutexHolder != NULL) {
                /*
                 * If the mutex holder has less priority than the priority of the calling
                 * co-routine, then it will temporarily inherit the priority of the
                 * calling co-routine.
                 */
                if (pxMutexHolderCRCB->uxPriority < pxCurrentCoRoutine->uxPriority) {
                        /* Adjust mutex holder state based on its new priority. */
                        listSET_LIST_ITEM_VALUE(&(pxMutexHolderCRCB->xEventListItem),
                                (UBaseType_t)configMAX_CO_ROUTINE_PRIORITIES - pxCurrentCoRoutine->uxPriority);

                        /*
                         * If the co-routine being modified is in the ready state, it will need to
                         * be moved into a new list.
                         */
                        if (listIS_CONTAINED_WITHIN(&(pxReadyCoRoutineLists[pxMutexHolderCRCB->uxPriority]),
                                                    &(pxMutexHolderCRCB->xGenericListItem)) != pdFALSE) {
                                /* Remove co-routine from current ready list. */
                                uxListRemove(&(pxMutexHolderCRCB->xGenericListItem));
                                /* Inherit the priority. */
                                pxMutexHolderCRCB->uxPriority = pxCurrentCoRoutine->uxPriority;
                                /* Move co-routine to new ready list based on the inherited priority. */
                                prvAddCoRoutineToReadyQueue(pxMutexHolderCRCB);
                        } else {
                                /* Inherit only the priority. */
                                pxMutexHolderCRCB->uxPriority = pxCurrentCoRoutine->uxPriority;
                        }

                        /* Inheritance occurred. */
                        xReturn = pdTRUE;
                } else {
                        if (pxMutexHolderCRCB->uxBasePriority < pxCurrentCoRoutine->uxPriority) {
                                /* The mutex holder has already inherited a higher priority. */
                                xReturn = pdTRUE;
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
                }
        } else {
                mtCOVERAGE_TEST_MARKER();
        }

        return xReturn;
}
#endif /* configUSE_MUTEXES */
/*-----------------------------------------------------------*/

#if (configUSE_MUTEXES == 1)

BaseType_t xDgCoRoutinePriorityDisinherit(CoRoutineHandle_t const pxMutexHolder)
{
        CRCB_t * const pxMutexHolderCRCB = (CRCB_t *)pxMutexHolder;
        BaseType_t xReturn = pdFALSE;

        /* Check that it is a valid mutex holder. */
        if (pxMutexHolder != NULL) {
                /* Decrement number of mutexes held by the calling co-routine. */
                (pxMutexHolderCRCB->uxMutexesHeld)--;

                /* Check whether the mutex holder has inherited a priority of another co-routine. */
                if (pxMutexHolderCRCB->uxPriority != pxMutexHolderCRCB->uxBasePriority) {
                        /* If no other mutexes held, disinherit the priority of other co-routines. */
                        if (pxMutexHolderCRCB->uxMutexesHeld == (UBaseType_t) 0) {
                                /* Remove co-routine from current ready list. */
                                uxListRemove(&(pxMutexHolderCRCB->xGenericListItem));
                                /* Inherit the priority. */
                                pxMutexHolderCRCB->uxPriority = pxMutexHolderCRCB->uxBasePriority;
                                /* Move co-routine to new ready list based on the inherited priority. */
                                prvAddCoRoutineToReadyQueue(pxMutexHolderCRCB);

                                xReturn = pdTRUE;
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        } else {
                mtCOVERAGE_TEST_MARKER();
        }

        return xReturn;
}
#endif /* configUSE_MUTEXES */
/*-----------------------------------------------------------*/

#if (configUSE_MUTEXES == 1)

void *pvDgCoRoutineIncrementMutexHeldCount(void)
{
        if (pxCurrentCoRoutine != NULL) {
                (pxCurrentCoRoutine->uxMutexesHeld)++;
        }

        return pxCurrentCoRoutine;
}
#endif /* configUSE_MUTEXES */
/*-----------------------------------------------------------*/

#if (configUSE_DG_COROUTINE_NOTIFICATIONS == 1)

BaseType_t xDgCoRoutineNotifyTake(BaseType_t xClearCountOnExit, uint32_t *pulNotificationValue,
        TickType_t xTicksToWait)
{
        BaseType_t xReturn;

        dgcrENTER_CRITICAL();
        {
                /* Only block if the notification value is not already non-zero. */
                if (pxCurrentCoRoutine->ulNotifiedValue == 0UL) {
                        /* Mark this co-routine as waiting for a notification. */
                        pxCurrentCoRoutine->ucNotifyState = dgcrWAITING_NOTIFICATION;

                        if (xTicksToWait > (TickType_t) 0) {
                                /* Add co-routine to the Blocked list. */
                                vCoRoutineAddToDelayedList(xTicksToWait, NULL);
                                dgcrEXIT_CRITICAL();
                                return errQUEUE_BLOCKED;
                        } else {
                                dgcrEXIT_CRITICAL();
                                return errQUEUE_YIELD;
                        }
                }
        }
        dgcrEXIT_CRITICAL();

        dgcrENTER_CRITICAL();
        {
                if (pulNotificationValue != NULL) {
                        /* Output the current notification value. */
                        *pulNotificationValue = pxCurrentCoRoutine->ulNotifiedValue;
                }

                /* Check whether the notification value is non-zero. */
                if (pxCurrentCoRoutine->ulNotifiedValue != 0UL) {
                        /* Clear notification value or decrement it, accordingly. */
                        if (xClearCountOnExit != pdFALSE) {
                                pxCurrentCoRoutine->ulNotifiedValue = 0UL;
                        } else {
                                (pxCurrentCoRoutine->ulNotifiedValue)--;
                        }

                        xReturn = pdPASS;
                } else {
                        xReturn = pdFAIL;
                }

                pxCurrentCoRoutine->ucNotifyState = dgcrNOT_WAITING_NOTIFICATION;
        }
        dgcrEXIT_CRITICAL();

        return xReturn;
}
#endif /* configUSE_DG_COROUTINE_NOTIFICATIONS */
/*-----------------------------------------------------------*/

#if (configUSE_DG_COROUTINE_NOTIFICATIONS == 1)

BaseType_t xDgCoRoutineNotifyWait(uint32_t ulBitsToClearOnEntry, uint32_t ulBitsToClearOnExit,
        uint32_t *pulNotificationValue, TickType_t xTicksToWait)
{
        BaseType_t xReturn;

        dgcrENTER_CRITICAL();
        {
                /* Only block if a notification is not already pending. */
                if (pxCurrentCoRoutine->ucNotifyState != dgcrNOTIFICATION_RECEIVED) {
                        /* Clear bits in the co-routine's notification value. */
                        pxCurrentCoRoutine->ulNotifiedValue &= ~ulBitsToClearOnEntry;

                        /* Mark this co-routine as waiting for a notification. */
                        pxCurrentCoRoutine->ucNotifyState = dgcrWAITING_NOTIFICATION;

                        if (xTicksToWait > (TickType_t) 0) {
                                /* Add co-routine to the Blocked list. */
                                vCoRoutineAddToDelayedList(xTicksToWait, NULL);
                                dgcrEXIT_CRITICAL();
                                return errQUEUE_BLOCKED;
                        } else {
                                dgcrEXIT_CRITICAL();
                                return errQUEUE_YIELD;
                        }
                }
        }
        dgcrEXIT_CRITICAL();

        dgcrENTER_CRITICAL();
        {
                if (pulNotificationValue != NULL) {
                        /* Output the current notification value. */
                        *pulNotificationValue = pxCurrentCoRoutine->ulNotifiedValue;
                }

                /* Check whether a notification was received. */
                if (pxCurrentCoRoutine->ucNotifyState != dgcrNOTIFICATION_RECEIVED) {
                        /* A notification was not received. */
                        xReturn = pdFAIL;
                } else {
                        /* A notification was already pending or a notification was received. */
                        pxCurrentCoRoutine->ulNotifiedValue &= ~ulBitsToClearOnExit;
                        xReturn = pdPASS;
                }

                pxCurrentCoRoutine->ucNotifyState = dgcrNOT_WAITING_NOTIFICATION;
        }
        dgcrEXIT_CRITICAL();

        return xReturn;
}
#endif /* configUSE_DG_COROUTINE_NOTIFICATIONS */
/*-----------------------------------------------------------*/

#if (configUSE_DG_COROUTINE_NOTIFICATIONS == 1)

BaseType_t xDgCoRoutineGenericNotify(CoRoutineHandle_t xCoRoutineToNotify, uint32_t ulValue,
        eDgCoRoutineNotifyAction eAction, uint32_t *pulPreviousNotificationValue)
{
        CRCB_t *pxCRCB;
        BaseType_t xReturn = pdPASS;
        uint8_t ucOriginalNotifyState;

        configASSERT(xCoRoutineToNotify);
        pxCRCB = (CRCB_t *)xCoRoutineToNotify;

        dgcrENTER_CRITICAL();
        {
                if (pulPreviousNotificationValue != NULL) {
                        *pulPreviousNotificationValue = pxCRCB->ulNotifiedValue;
                }

                ucOriginalNotifyState = pxCRCB->ucNotifyState;
                pxCRCB->ucNotifyState = dgcrNOTIFICATION_RECEIVED;

                switch (eAction) {
                        case eDgCrSetBits:
                                pxCRCB->ulNotifiedValue |= ulValue;
                                break;
                        case eDgCrIncrement:
                                (pxCRCB->ulNotifiedValue)++;
                                break;
                        case eDgCrSetValueWithOverwrite:
                                pxCRCB->ulNotifiedValue = ulValue;
                                break;
                        case eDgCrSetValueWithoutOverwrite:
                                if (ucOriginalNotifyState != dgcrNOTIFICATION_RECEIVED) {
                                        pxCRCB->ulNotifiedValue = ulValue;
                                } else {
                                        /* The value could not be written to the co-routine. */
                                        xReturn = pdFAIL;
                                }
                                break;
                        case eDgCrNoAction:
                                /* The co-routine is notified with no update on its notify value. */
                                break;
                }

                /* If the co-routine is blocked waiting for a notification, then unblock it now. */
                if (ucOriginalNotifyState == dgcrWAITING_NOTIFICATION) {
                        /* The co-routine should not have been on an event list. */
                        configASSERT(listLIST_ITEM_CONTAINER(&(pxCRCB->xEventListItem)) == NULL);

                        vListInsertEnd((List_t *)&xPendingReadyCoRoutineList,
                                &(pxCRCB->xEventListItem));

                        if (pxCRCB->uxPriority > pxCurrentCoRoutine->uxPriority) {
                                /*
                                 * The notified co-routine has a priority above the currently
                                 * executing co-routine, so indicate that a yield might be
                                 * appropriate.
                                 */
#if (configUSE_PREEMPTION == 1)
                                xReturn = errQUEUE_YIELD;
#endif
                                vDgCoRoutineMissedYield();
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        }
        dgcrEXIT_CRITICAL();

        return xReturn;
}
#endif /* configUSE_DG_COROUTINE_NOTIFICATIONS */
/*-----------------------------------------------------------*/

#if (configUSE_DG_COROUTINE_NOTIFICATIONS == 1)

BaseType_t xDgCoRoutineGenericNotifyFromISR(CoRoutineHandle_t xCoRoutineToNotify, uint32_t ulValue,
        eDgCoRoutineNotifyAction eAction, uint32_t *pulPreviousNotificationValue)
{
        CRCB_t *pxCRCB;
        BaseType_t xReturn = pdPASS;
        UBaseType_t uxSavedInterruptStatus;

        configASSERT(xCoRoutineToNotify);

        /* Safety check imposed by FreeRTOS kernel */
        portASSERT_IF_INTERRUPT_PRIORITY_INVALID();

        pxCRCB = (CRCB_t *)xCoRoutineToNotify;

        uxSavedInterruptStatus = dgcrENTER_CRITICAL_FROM_ISR();
        {
                if (pulPreviousNotificationValue != NULL) {
                        *pulPreviousNotificationValue = pxCRCB->ulNotifiedValue;
                }

                uint8_t ucOriginalNotifyState = pxCRCB->ucNotifyState;
                pxCRCB->ucNotifyState = dgcrNOTIFICATION_RECEIVED;

                switch (eAction) {
                        case eDgCrSetBits:
                                pxCRCB->ulNotifiedValue |= ulValue;
                                break;
                        case eDgCrIncrement:
                                (pxCRCB->ulNotifiedValue)++;
                                break;
                        case eDgCrSetValueWithOverwrite:
                                pxCRCB->ulNotifiedValue = ulValue;
                                break;
                        case eDgCrSetValueWithoutOverwrite:
                                if (ucOriginalNotifyState != dgcrNOTIFICATION_RECEIVED) {
                                        pxCRCB->ulNotifiedValue = ulValue;
                                } else {
                                        /* The value could not be written to the co-routine. */
                                        xReturn = pdFAIL;
                                }
                                break;
                        case eDgCrNoAction:
                                /* The co-routine is notified with no update on its notify value. */
                                break;
                }

                /* If the co-routine is blocked waiting for a notification, then unblock it now. */
                if (ucOriginalNotifyState == dgcrWAITING_NOTIFICATION) {
                        /* The co-routine should not have been on an event list. */
                        configASSERT(listLIST_ITEM_CONTAINER(&(pxCRCB->xEventListItem)) == NULL);

                        vListInsertEnd((List_t *)&xPendingReadyCoRoutineList,
                                &(pxCRCB->xEventListItem));

                        if (pxCRCB->uxPriority > pxCurrentCoRoutine->uxPriority) {
                                /*
                                 * The notified co-routine has a priority above the currently
                                 * executing co-routine, so indicate that a yield might be
                                 * appropriate.
                                 */
                                vDgCoRoutineMissedYieldForPriority(pxCRCB->uxPriority);
                        } else {
                                mtCOVERAGE_TEST_MARKER();
                        }
                } else {
                        mtCOVERAGE_TEST_MARKER();
                }
        }
        dgcrEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

        return xReturn;
}
#endif /* configUSE_DG_COROUTINE_NOTIFICATIONS */
/*-----------------------------------------------------------*/

#if (configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H == 1)
#include "freertos_tasks_c_additions.h"

static void freertos_tasks_c_additions_init(void)
{
        #ifdef FREERTOS_TASKS_C_ADDITIONS_INIT
                FREERTOS_TASKS_C_ADDITIONS_INIT();
        #endif
}
#endif
/*-----------------------------------------------------------*/

#endif /* configUSE_DIALOG_CO_ROUTINES */
