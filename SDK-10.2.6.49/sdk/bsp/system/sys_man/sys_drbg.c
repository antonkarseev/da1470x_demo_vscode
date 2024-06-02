/**
 ****************************************************************************************
 *
 * @file sys_drbg.c
 *
 * @brief System deterministic random bit generator
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if (MAIN_PROCESSOR_BUILD)

#if dg_configUSE_SYS_DRBG

#include <stdint.h>
#include <stdlib.h>
#include "bsp_defaults.h"
#include "sdk_defs.h"
#include "hw_bsr.h"
#include "sys_drbg.h"
#include "sdk_crc16.h"
#if dg_configUSE_SYS_TRNG
#include "sys_trng.h"
#include "sys_trng_internal.h"
#endif /* dg_configUSE_SYS_TRNG */

#if dg_configUSE_CHACHA20_RAND
#include "chacha20.h"
#endif /* dg_configUSE_CHACHA20_RAND */

#if defined(OS_PRESENT)
#include "osal.h"
#include "interrupts.h"
#endif /* OS_PRESENT */

/*
 * MACROS
 *****************************************************************************************
 */

#if defined(OS_PRESENT)
#if defined(OS_FEATURE_SINGLE_STACK)
#define SYS_DRBG_MUTEX_CREATE()         do {} while (0)
#define SYS_DRBG_MUTEX_GET()            do {} while (0)
#define SYS_DRBG_MUTEX_PUT()            do {} while (0)
#else
__RETAINED static OS_MUTEX sys_drbg_mutex;

#define SYS_DRBG_MUTEX_CREATE()         do { \
                                                OS_ASSERT(sys_drbg_mutex == NULL);                      \
                                                OS_MUTEX_CREATE(sys_drbg_mutex);                        \
                                                OS_ASSERT(sys_drbg_mutex);                              \
                                        } while (0)
// The following macros check if sys_drbg_mutex is not NULL before OS_MUTEX_GET/PUT.
// In order to prevent software from running with interrupts disabled if scheduler
// has not been started.
#define SYS_DRBG_MUTEX_GET()            do { \
                                                if (sys_drbg_mutex) {                                   \
                                                        OS_MUTEX_GET(sys_drbg_mutex, OS_MUTEX_FOREVER); \
                                                }                                                       \
                                        } while (0)
#define SYS_DRBG_MUTEX_PUT()            do { \
                                                if (sys_drbg_mutex) {                                   \
                                                        OS_MUTEX_PUT(sys_drbg_mutex);                   \
                                                }                                                       \
                                        } while (0)
#endif /* OS_FEATURE_SINGLE_STACK */
#else
#define SYS_DRBG_MUTEX_CREATE()         do {} while (0)
#define SYS_DRBG_MUTEX_GET()            do {} while (0)
#define SYS_DRBG_MUTEX_PUT()            do {} while (0)
#endif /* OS_PRESENT */

#if (MAIN_PROCESSOR_BUILD)
#define SYS_DRBG_HW_MUTEX_GET(pos)      ({ while (!hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, pos)) {} })
#define SYS_DRBG_HW_MUTEX_PUT(pos)      (hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, pos))
#elif (SNC_PROCESSOR_BUILD)
#define SYS_DRBG_HW_MUTEX_GET(pos)      ({ while (!hw_bsr_try_lock(HW_BSR_MASTER_SNC, pos)) {} })
#define SYS_DRBG_HW_MUTEX_PUT(pos)      (hw_bsr_unlock(HW_BSR_MASTER_SNC, pos))
#endif /* PROCESSOR_BUILD */

#if defined(OS_PRESENT)
#define SYS_DRBG_PRIORITY                (OS_TASK_PRIORITY_LOWEST)
#endif /* OS_PRESENT */

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief sys drbg data structure
 */
typedef struct {
        uint32_t buffer[SYS_DRBG_BUFFER_LENGTH];  /**< the buffer which holds the random numbers */
        uint32_t threshold;                       /**< the threshold level in the buffer */
        uint32_t index;                           /**< the index in the buffer */
        uint8_t  request;                         /**< the request for buffer update */
        uint8_t  hw_bsr;                          /**< the HW BSR number used */
} sys_drbg_t;

/*
 * VARIABLE DECLARATIONS
 *****************************************************************************************
 */

__IN_CMAC_MEM1 static sys_drbg_t sys_drbg;

#if dg_configUSE_STDLIB_RAND
__IN_CMAC_MEM1_UNINIT static unsigned rand_r_state;
#endif /* dg_configUSE_STDLIB_RAND */

#if defined(OS_PRESENT)
__RETAINED static OS_TASK sys_drbg_handle;
#endif /* OS_PRESENT */

#if (dg_configUSE_SYS_TRNG == 0)
__IN_CMAC_MEM1_UNINIT static uint32_t drbg_id;
#endif /* dg_configUSE_SYS_TRNG */

/*
 * LOCAL FUNCTION DECLARATIONS
 *****************************************************************************************
 */

static const uint8_t *dg_get_seed(void);
static uint32_t dg_rand(void);

#if defined(OS_PRESENT)
static void sys_drbg_update(void);
static OS_TASK_FUNCTION(sys_drbg_task, pvParameters);
#endif /* OS_PRESENT */

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static const uint8_t *dg_get_seed(void)
{
#if dg_configUSE_SYS_TRNG
        return sys_trng_get_seed();
#else
        const uint32_t offset = MEMORY_SYSRAM11_BASE - 16;
        ASSERT_WARNING((offset & 0x00000003) == 0);
        return (uint8_t *) offset;
#endif
}

static uint32_t dg_rand(void)
{
#if dg_configUSE_CHACHA20_RAND
        return csprng_get_next_uint32();
#elif dg_configUSE_STDLIB_RAND
        return (uint32_t) rand_r(&rand_r_state);
#endif
}

#if defined(OS_PRESENT)
static OS_TASK_FUNCTION(sys_drbg_task, pvParameters)
{
        /* Task loop */
        while (1) {
                /* Wait to be notified for drbg buffer update */
                OS_TASK_NOTIFY_TAKE(OS_TRUE, OS_TASK_NOTIFY_FOREVER);

                sys_drbg_update();
        }
}
#endif /* OS_PRESENT */

#if (dg_configUSE_SYS_TRNG == 0)
bool sys_drbg_can_run(void)
{
        const uint8_t SYS_DRBG_SEED_SIZE = 16;
        if (drbg_id != (uint32_t) crc16_calculate(dg_get_seed(), SYS_DRBG_SEED_SIZE)) {
                return true;
        }
        return false;
}
#endif /* dg_configUSE_SYS_TRNG */

void sys_drbg_srand(void)
{
#if dg_configUSE_CHACHA20_RAND
        csprng_seed(dg_get_seed());
#elif dg_configUSE_STDLIB_RAND
        srand(* (const unsigned *) (dg_get_seed()));
#endif
}

#if defined(OS_PRESENT)
void sys_drbg_create_os_objects(void)
{
        /* Check scheduler's state */
        OS_ASSERT(OS_GET_TASK_SCHEDULER_STATE() != OS_SCHEDULER_NOT_STARTED);

        /* Create mutex. Called only once! */
        SYS_DRBG_MUTEX_CREATE();


        OS_BASE_TYPE status;

        /* Create the sys_drbg task */
        status = OS_TASK_CREATE("sys_drbg",                   /* The text name assigned to the task, for
                                                                 debug only; not used by the kernel. */
                                sys_drbg_task,                /* The function that implements the task. */
                                NULL,                         /* The parameter passed to the task. */
                                OS_MINIMAL_TASK_STACK_SIZE,
                                                              /* The number of bytes to allocate to the
                                                                 stack of the task. */
                                SYS_DRBG_PRIORITY,            /* The priority assigned to the task. */
                                sys_drbg_handle );            /* The task handle */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);
}
#endif /* OS_PRESENT */

void sys_drbg_init(void)
{
        /* Generate random numbers */
        for (int i = 0; i < ARRAY_LENGTH(sys_drbg.buffer); i++) {
                sys_drbg.buffer[i] = dg_rand();
        }

        sys_drbg.threshold = SYS_DRBG_BUFFER_THRESHOLD;
        sys_drbg.index     = 0;
        sys_drbg.request   = 0;

        sys_drbg.hw_bsr    = HW_BSR_PERIPH_ID_DRBG;

}

SYS_DRBG_ERROR sys_drbg_read_rand(uint32_t *rand_number)
{
        SYS_DRBG_MUTEX_GET();
        SYS_DRBG_HW_MUTEX_GET(sys_drbg.hw_bsr);

        SYS_DRBG_ERROR ret = SYS_DRBG_ERROR_NONE;

        if (sys_drbg.index < ARRAY_LENGTH(sys_drbg.buffer)) {
                /* Check if threshold has been reached or passed */
                if (sys_drbg.index >= sys_drbg.threshold ) {
                        sys_drbg.request = 1;
                }
                /* Get random number */
                *rand_number = sys_drbg.buffer[sys_drbg.index];
                sys_drbg.index++;
        } else {
                /* Buffer has been exhausted */
                sys_drbg.request = 1;
                ret = SYS_DRBG_ERROR_BUFFER_EXHAUSTED;
#if defined(OS_PRESENT)
                /* Notify the sys_drbg task */
                if (in_interrupt()) {
                        OS_TASK_NOTIFY_GIVE_FROM_ISR(sys_drbg_handle);
                } else {
                        OS_TASK_NOTIFY_GIVE(sys_drbg_handle);
                }
#endif /* OS_PRESENT */
        }

        SYS_DRBG_HW_MUTEX_PUT(sys_drbg.hw_bsr);
        SYS_DRBG_MUTEX_PUT();

        return ret;
}

#if defined(OS_PRESENT)
static void sys_drbg_update(void)
#else
void sys_drbg_update(void)
#endif /* OS_PRESENT */
{
        SYS_DRBG_MUTEX_GET();
        SYS_DRBG_HW_MUTEX_GET(sys_drbg.hw_bsr);

        if (sys_drbg.request == 1) {
                /* Update the consumed random numbers */
                for (int i = 0; i < sys_drbg.index; i++) {
                        sys_drbg.buffer[i] = dg_rand();
                }
                sys_drbg.index   = 0;
                sys_drbg.request = 0;
        }

        SYS_DRBG_HW_MUTEX_PUT(sys_drbg.hw_bsr);
        SYS_DRBG_MUTEX_PUT();
}

uint32_t sys_drbg_read_index(void)
{
        SYS_DRBG_MUTEX_GET();
        SYS_DRBG_HW_MUTEX_GET(sys_drbg.hw_bsr);

        uint32_t index = sys_drbg.index;

        SYS_DRBG_HW_MUTEX_PUT(sys_drbg.hw_bsr);
        SYS_DRBG_MUTEX_PUT();

        return index;
}

uint32_t sys_drbg_read_threshold(void)
{
        SYS_DRBG_MUTEX_GET();
        SYS_DRBG_HW_MUTEX_GET(sys_drbg.hw_bsr);

        uint32_t threshold = sys_drbg.threshold;

        SYS_DRBG_HW_MUTEX_PUT(sys_drbg.hw_bsr);
        SYS_DRBG_MUTEX_PUT();

        return threshold;
}

uint8_t sys_drbg_read_request(void)
{
        SYS_DRBG_MUTEX_GET();
        SYS_DRBG_HW_MUTEX_GET(sys_drbg.hw_bsr);

        uint8_t request = sys_drbg.request;

        SYS_DRBG_HW_MUTEX_PUT(sys_drbg.hw_bsr);
        SYS_DRBG_MUTEX_PUT();

        return request;
}

#endif /* dg_configUSE_SYS_DRBG */

#endif /* MAIN_PROCESSOR_BUILD */


