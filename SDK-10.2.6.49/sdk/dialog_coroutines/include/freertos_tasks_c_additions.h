/*
 * Copyright (C) 2018-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

#ifndef FREERTOS_TASKS_C_ADDITIONS_H
#define FREERTOS_TASKS_C_ADDITIONS_H

#include <stdint.h>

#if (configUSE_TRACE_FACILITY == 0)
#error "configUSE_TRACE_FACILITY must be enabled"
#endif

#define FREERTOS_DEBUG_CONFIG_MAJOR_VERSION 1
#define FREERTOS_DEBUG_CONFIG_MINOR_VERSION 1

#define configFRTOS_MEMORY_SCHEME     4

#ifdef __cplusplus
extern "C" {
#endif

#if ( configUSE_DIALOG_CO_ROUTINES == 1 )

__RETAINED_RW uint8_t FreeRTOSDebugConfig[] =
{
        FREERTOS_DEBUG_CONFIG_MAJOR_VERSION,
        FREERTOS_DEBUG_CONFIG_MINOR_VERSION,
        tskKERNEL_VERSION_MAJOR,
        tskKERNEL_VERSION_MINOR,
        tskKERNEL_VERSION_BUILD,
        configFRTOS_MEMORY_SCHEME,
        offsetof(struct corCoRoutineControlBlock, pxTopOfStack),
        offsetof(struct corCoRoutineControlBlock, xGenericListItem),
        offsetof(struct corCoRoutineControlBlock, xEventListItem),
        offsetof(struct corCoRoutineControlBlock, pxEndOfStack),
#if (configMAX_DG_COROUTINE_NAME_LEN > 0)
        offsetof(struct corCoRoutineControlBlock, pcCoRoutineName),
#else
        0,
#endif
        offsetof(struct corCoRoutineControlBlock, uxCRCBNumber),
        offsetof(struct corCoRoutineControlBlock, uxCoRoutineNumber),
        configMAX_DG_COROUTINE_NAME_LEN,
        configMAX_CO_ROUTINE_PRIORITIES,
        0 /* pad to 32-bit boundary */
};

#else

__RETAINED_RW uint8_t FreeRTOSDebugConfig[] =
{
        FREERTOS_DEBUG_CONFIG_MAJOR_VERSION,
        FREERTOS_DEBUG_CONFIG_MINOR_VERSION,
        tskKERNEL_VERSION_MAJOR,
        tskKERNEL_VERSION_MINOR,
        tskKERNEL_VERSION_BUILD,
        configFRTOS_MEMORY_SCHEME,
        offsetof(struct tskTaskControlBlock, pxTopOfStack),
        offsetof(struct tskTaskControlBlock, xStateListItem),
        offsetof(struct tskTaskControlBlock, xEventListItem),
        offsetof(struct tskTaskControlBlock, pxStack),
        offsetof(struct tskTaskControlBlock, pcTaskName),
        offsetof(struct tskTaskControlBlock, uxTCBNumber),
        offsetof(struct tskTaskControlBlock, uxTaskNumber),
        configMAX_TASK_NAME_LEN,
        configMAX_PRIORITIES,
        0 /* pad to 32-bit boundary */
};

#endif /* configUSE_DIALOG_CO_ROUTINES */

#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_TASKS_C_ADDITIONS_H */
