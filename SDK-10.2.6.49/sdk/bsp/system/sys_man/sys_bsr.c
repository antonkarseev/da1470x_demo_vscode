/**
****************************************************************************************
*
* @file sys_bsr.c
*
* @brief Busy Status Register (BSR) manager
*
* Copyright (C) 2018-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#include "sdk_defs.h"
#include "sys_bsr.h"

#ifdef OS_PRESENT
#include "osal.h"
#endif /* OS_PRESENT */

typedef uint8_t sys_bsr_entry_t;

#ifdef CONFIG_USE_SNC
#include "snc.h"
#define HW_SYS_RETAINED                 __SNC_SHARED
/**
 * Shared environment with SNC
 */
typedef struct {
#if (USE_SW_BSR)
        volatile sys_bsr_entry_t *sys_bsr_sw_bsr;
#endif
        volatile uint8_t *sys_bsr_cnt;
} sys_bsr_shared_env_t;
#if (MAIN_PROCESSOR_BUILD)
HW_SYS_RETAINED static sys_bsr_shared_env_t sys_bsr_shared_env;
#endif
#else
#define HW_SYS_RETAINED                 __RETAINED
#endif /* CONFIG_USE_SNC */

#if (MAIN_PROCESSOR_BUILD)
#if (USE_SW_BSR)
/*
 * Indicates which master has currently access to a certain peripheral.
 * sys_bsr_sw_bsr[periph_id] == SYS_BSR_MASTER_X: Indicates that master X has currently access to that peripheral.
 * This allows for more efficient calculations on SNC side.
 */
HW_SYS_RETAINED static sys_bsr_entry_t sys_bsr_sw_bsr[SYS_BSR_SW_BSR_SIZE];
#endif

/*
 * Indicates for certain peripheral how many pending peripheral accesses are in progress for a given master
 * processing unit (SNC, SYSCPU, CMAC).
 */
HW_SYS_RETAINED static uint8_t sys_bsr_cnt[SYS_BSR_SW_BSR_SIZE];
#elif (SNC_PROCESSOR_BUILD)
#if (USE_SW_BSR)
HW_SYS_RETAINED static sys_bsr_entry_t *sys_bsr_sw_bsr;
#endif
HW_SYS_RETAINED static uint8_t *sys_bsr_cnt;
#endif /* PROCESSOR_BUILD */

#if defined(OS_PRESENT)
#if defined(OS_FEATURE_SINGLE_STACK)
#define BSR_MUTEX_CREATE()
#define BSR_MUTEX_GET()
#define BSR_MUTEX_PUT()
#else
__RETAINED static OS_MUTEX sys_bsr_mutex;

#define BSR_MUTEX_CREATE()      OS_ASSERT(sys_bsr_mutex == NULL); \
                                OS_MUTEX_CREATE(sys_bsr_mutex); \
                                OS_ASSERT(sys_bsr_mutex)
#define BSR_MUTEX_GET()         OS_ASSERT(sys_bsr_mutex); \
                                OS_MUTEX_GET(sys_bsr_mutex, OS_MUTEX_FOREVER)
#define BSR_MUTEX_PUT()         OS_MUTEX_PUT(sys_bsr_mutex)
#endif
#else
#define BSR_MUTEX_CREATE()
#define BSR_MUTEX_GET()
#define BSR_MUTEX_PUT()
#endif /* OS_PRESENT */

__UNUSED __STATIC_INLINE void init_sw_bsr_tables(void)
{
        int i;

        hw_bsr_init();

        for (i = 0; i < SYS_BSR_SW_BSR_SIZE; i++) {
#if (USE_SW_BSR)
                sys_bsr_sw_bsr[i] = SYS_BSR_MASTER_NONE;
#endif
                sys_bsr_cnt[i] = 0;
        }
}

void sys_bsr_initialize(void)
{
#if (MAIN_PROCESSOR_BUILD)
        init_sw_bsr_tables();
#ifdef CONFIG_USE_SNC
        /* Publish addresses of sys_bsr_sw_bsr and sys_bsr_cnt to SNC. */
#if (USE_SW_BSR)
        sys_bsr_shared_env.sys_bsr_sw_bsr = sys_bsr_sw_bsr;
#endif
        sys_bsr_shared_env.sys_bsr_cnt = sys_bsr_cnt;

        snc_set_shared_space_addr(&sys_bsr_shared_env, SNC_SHARED_SPACE_SYS_BSR);
#endif
#elif (SNC_PROCESSOR_BUILD)
        {
                sys_bsr_shared_env_t *shared_env = snc_get_shared_space_addr(SNC_SHARED_SPACE_SYS_BSR);
#if (USE_SW_BSR)
                sys_bsr_sw_bsr = snc_convert_sys2snc_addr((void *)shared_env->sys_bsr_sw_bsr);
#endif
                sys_bsr_cnt = snc_convert_sys2snc_addr((void *)shared_env->sys_bsr_cnt);
        }
#endif /* PROCESSOR_BUILD */
}

#if (USE_SW_BSR)
static bool sw_bsr_try_acquire(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
        bool acquired = false;

        ASSERT_ERROR (periph_id < SYS_BSR_PERIPH_ID_MAX);

        while (!hw_bsr_try_lock(bsr_master_id, HW_BSR_SW_POS)) {}

        if ((sys_bsr_sw_bsr[periph_id] == SYS_BSR_MASTER_NONE) || (sys_bsr_sw_bsr[periph_id] == bsr_master_id)) {
                /* Update SW BSR internal administration */
                sys_bsr_sw_bsr[periph_id] = bsr_master_id;
                ASSERT_ERROR(sys_bsr_cnt[periph_id] != 0xFF);
                sys_bsr_cnt[periph_id]++;
                acquired = true;
        }

        hw_bsr_unlock(bsr_master_id, HW_BSR_SW_POS);

        return acquired;
}

static bool sw_bsr_acquired(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
        bool acquired = false;

        ASSERT_ERROR (periph_id < SYS_BSR_PERIPH_ID_MAX);

        while (!hw_bsr_try_lock(bsr_master_id, HW_BSR_SW_POS)) {}

        if (sys_bsr_sw_bsr[periph_id] == bsr_master_id) {
                acquired = true;
        }

        hw_bsr_unlock(bsr_master_id, HW_BSR_SW_POS);

        return acquired;
}

static void sw_bsr_release(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
        ASSERT_ERROR (periph_id < SYS_BSR_PERIPH_ID_MAX);

        while (!hw_bsr_try_lock(bsr_master_id, HW_BSR_SW_POS)) {}

        ASSERT_ERROR (sys_bsr_sw_bsr[periph_id] == bsr_master_id);
        ASSERT_ERROR (sys_bsr_cnt[periph_id]);

        if (--sys_bsr_cnt[periph_id] == 0) {
                sys_bsr_sw_bsr[periph_id] = SYS_BSR_MASTER_NONE;
        }

        hw_bsr_unlock(bsr_master_id, HW_BSR_SW_POS);
}

static void sys_sw_bsr_acquire(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
        BSR_MUTEX_GET();
        while (!sw_bsr_try_acquire(bsr_master_id, periph_id)) {
        }
        BSR_MUTEX_PUT();
}

static bool sys_sw_bsr_acquired(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
        bool acquired;

        BSR_MUTEX_GET();
        acquired = sw_bsr_acquired(bsr_master_id, periph_id);
        BSR_MUTEX_PUT();

        return acquired;
}

static void sys_sw_bsr_release(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
        BSR_MUTEX_GET();
        sw_bsr_release(bsr_master_id, periph_id);
        BSR_MUTEX_PUT();
}

static bool sys_sw_bsr_try_acquire(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
        bool ret = false;

        BSR_MUTEX_GET();
        ret = sw_bsr_try_acquire(bsr_master_id, periph_id);
        BSR_MUTEX_PUT();

        return ret;
}
#else
static void sys_hw_bsr_acquire(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
        while (!hw_bsr_try_lock(bsr_master_id, periph_id)) {}
        BSR_MUTEX_GET();
        ASSERT_ERROR(sys_bsr_cnt[periph_id >> 1] != 0xFF);
        sys_bsr_cnt[periph_id >> 1]++; // HW BSR Peripheral ids are always even
        BSR_MUTEX_PUT();
}

static bool sys_hw_bsr_try_acquire(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id) {
        if (hw_bsr_try_lock(bsr_master_id, periph_id)) {
                BSR_MUTEX_GET();
                ASSERT_ERROR(sys_bsr_cnt[periph_id >> 1] != 0xFF);
                sys_bsr_cnt[periph_id >> 1]++; // HW BSR Peripheral i
                BSR_MUTEX_PUT();
                return true;
        } else {
                return false;
        }
}

static bool sys_hw_bsr_acquired(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
        return hw_bsr_is_locked(bsr_master_id, periph_id);
}

static void sys_hw_bsr_release(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
        ASSERT_ERROR(sys_bsr_cnt[periph_id >> 1]);

        BSR_MUTEX_GET();
        if (--sys_bsr_cnt[periph_id >> 1] == 0) { // HW BSR Peripheral ids are always even
                hw_bsr_unlock(bsr_master_id, periph_id);
        }
        BSR_MUTEX_PUT();
}
#endif /* USE_SW_BSR */

void sys_bsr_init(void)
{
        BSR_MUTEX_CREATE();     // Create mutex. Called only once!
}

void sys_bsr_acquire(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
#if (USE_SW_BSR)
        sys_sw_bsr_acquire(bsr_master_id, periph_id);
#else
        sys_hw_bsr_acquire(bsr_master_id, periph_id);
#endif
}

bool sys_bsr_try_acquire(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
#if (USE_SW_BSR)
        return sys_sw_bsr_try_acquire(bsr_master_id, periph_id);
#else
        return sys_hw_bsr_try_acquire(bsr_master_id, periph_id);
#endif
}

bool sys_bsr_acquired(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
#if (USE_SW_BSR)
        return sys_sw_bsr_acquired(bsr_master_id, periph_id);
#else
        return sys_hw_bsr_acquired(bsr_master_id, periph_id);
#endif
}

void sys_bsr_release(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id)
{
#if (USE_SW_BSR)
        sys_sw_bsr_release(bsr_master_id, periph_id);
#else
        sys_hw_bsr_release(bsr_master_id, periph_id);
#endif
}
