/**
 ****************************************************************************************
 *
 * @file dialog_coroutines.h
 *
 * @brief Extensions to FreeRTOS main header file (i.e. FreeRTOS.h) for co-routines
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef INC_DIALOG_COROUTINES_H_
#define INC_DIALOG_COROUTINES_H_

#if (configUSE_DIALOG_CO_ROUTINES == 1)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Check definition of all the required application specific macros defined within FreeRTOSConfig.h
 */

#ifndef INCLUDE_uxDgCoRoutinePriorityGet
        #define INCLUDE_uxDgCoRoutinePriorityGet 0
#endif

#ifndef INCLUDE_vDgCoRoutineDelete
        #define INCLUDE_vDgCoRoutineDelete 0
#endif

#ifndef INCLUDE_vDgCoRoutineDelayUntil
        #define INCLUDE_vDgCoRoutineDelayUntil 0
#endif

#ifndef INCLUDE_xDgCoRoutineGetIdleCoRoutineHandle
        #define INCLUDE_xDgCoRoutineGetIdleCoRoutineHandle 1
#endif

#ifndef INCLUDE_xDgCoRoutineGetHandle
        #define INCLUDE_xDgCoRoutineGetHandle 0
#endif

#ifndef INCLUDE_uxDgCoRoutineGetStackHighWaterMark
        #define INCLUDE_uxDgCoRoutineGetStackHighWaterMark 0
#endif

#ifndef INCLUDE_pxDgCoRoutineGetStackStart
        #define INCLUDE_pxDgCoRoutineGetStackStart 0
#endif

#ifndef INCLUDE_eDgCoRoutineGetState
        #define INCLUDE_eDgCoRoutineGetState 0
#endif

#ifndef INCLUDE_xDgCoRoutineGetSchedulerState
        #define INCLUDE_xDgCoRoutineGetSchedulerState 0
#endif

#ifndef INCLUDE_xDgCoRoutineGetCurrentCoRoutineHandle
        #define INCLUDE_xDgCoRoutineGetCurrentCoRoutineHandle 0
#endif

#ifndef configUSE_DAEMON_DG_COROUTINE_STARTUP_HOOK
        #define configUSE_DAEMON_DG_COROUTINE_STARTUP_HOOK 0
#endif

#ifndef configMAX_DG_COROUTINE_NAME_LEN
        #define configMAX_DG_COROUTINE_NAME_LEN 4
#endif

#if (configUSE_TIMERS == 1)
        #ifndef configTIMER_DG_COROUTINE_PRIORITY
                #error If configUSE_TIMERS is set to 1 then configTIMER_DG_COROUTINE_PRIORITY must also be defined.
        #endif /* configTIMER_DG_COROUTINE_PRIORITY */

        #ifndef configTIMER_QUEUE_LENGTH
                #error If configUSE_TIMERS is set to 1 then configTIMER_QUEUE_LENGTH must also be defined.
        #endif /* configTIMER_QUEUE_LENGTH */
#endif /* configUSE_TIMERS */

#ifndef configUSE_DG_COROUTINE_NOTIFICATIONS
        #define configUSE_DG_COROUTINE_NOTIFICATIONS 1
#endif

#ifndef configRECORD_DG_COROUTINE_BLOCKED_PC
        #define configRECORD_DG_COROUTINE_BLOCKED_PC 1
#endif

#ifndef configUSE_DG_COROUTINE_DEBUG_FACILITY
        #define configUSE_DG_COROUTINE_DEBUG_FACILITY 0
#endif

#if (configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H == 1) && (configUSE_DG_COROUTINE_DEBUG_FACILITY == 0)
        #error If configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H is set to 1 then configUSE_DG_COROUTINE_DEBUG_FACILITY must also be set to 1.
#endif

#if (configUSE_DG_COROUTINE_DEBUG_FACILITY == 0)
        #undef portDGCOROUTINE_DEBUG_FACILITY_UPDATE_INFO
        #define portDGCOROUTINE_DEBUG_FACILITY_UPDATE_INFO(pxCRCB)
#endif

/*
 * Obfuscated version of CRCB_t structure
 */
typedef struct xSTATIC_DG_CRCB
{
#if (configUSE_DG_COROUTINE_DEBUG_FACILITY == 1)
        void                    *pxDummy1;
#endif
        StaticListItem_t        xDummy2[2];
        UBaseType_t             uxDummy3[2];
#if (configMAX_DG_COROUTINE_NAME_LEN > 0)
        uint8_t                 ucDummy4[configMAX_DG_COROUTINE_NAME_LEN];
#endif
        void                    *pxDummy5;
#if (configUSE_DG_COROUTINE_DEBUG_FACILITY == 1)
        portDGCOROUTINE_DEBUG_FACILITY_CRCB_INFO
#endif
#if (configRECORD_DG_COROUTINE_BLOCKED_PC == 1)
        void                    *pxDummy6;
#endif
        uint16_t                uxDummy7;
#if (configUSE_DG_COROUTINE_NOTIFICATIONS == 1)
        uint8_t                 ucDummy8;
        uint32_t                ulDummy9;
#endif
#if (configUSE_TRACE_FACILITY == 1)
        UBaseType_t             uxDummy10[2];
#endif
#if (configUSE_MUTEXES == 1)
        UBaseType_t             uxDummy11[2];
#endif
#if (configGENERATE_RUN_TIME_STATS == 1)
        uint32_t                ulDummy12;
#endif
#if (configUSE_TRACE_FACILITY == 1) || (INCLUDE_uxDgCoRoutineGetStackHighWaterMark == 1) || (INCLUDE_pxDgCoRoutineGetStackStart == 1)
#if (configRECORD_STACK_HIGH_ADDRESS == 1) || (portSTACK_GROWTH < 0)
        void                    *pxDummy13;
#endif
#if (configUSE_TRACE_FACILITY == 1) || (portSTACK_GROWTH > 0)
        void                    *pxDummy14;
#endif
#endif
#if (configUSE_TRACE_FACILITY == 1) || (INCLUDE_uxDgCoRoutineGetStackHighWaterMark == 1)
        uint16_t                uxDummy15;
#endif
#if (configSUPPORT_STATIC_ALLOCATION == 1) && (configSUPPORT_DYNAMIC_ALLOCATION == 1)
        uint8_t                 ucDummy16;
#endif
} StaticDgCoRoutine_t;

#ifdef __cplusplus
}
#endif

#endif /* configUSE_DIALOG_CO_ROUTINES */

#endif /* INC_DIALOG_COROUTINES_H_ */
