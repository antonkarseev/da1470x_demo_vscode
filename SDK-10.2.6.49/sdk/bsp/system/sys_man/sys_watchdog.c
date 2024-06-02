/**
 ****************************************************************************************
 *
 * @file sys_watchdog.c
 *
 * @brief Watchdog Service
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "sdk_defs.h"
#include "hw_watchdog.h"

#if dg_configUSE_WDOG

#include "sys_watchdog.h"
#include "sys_watchdog_internal.h"
#include "osal.h"

#if defined(OS_PRESENT)
#if defined(OS_FEATURE_SINGLE_STACK)
#define WATCHDOG_MUTEX_CREATE()
#define WATCHDOG_MUTEX_GET()
#define WATCHDOG_MUTEX_PUT()
#else
/* mutex to synchronize access to watchdog data */
__RETAINED static OS_MUTEX watchdog_mutex;

#define WATCHDOG_MUTEX_CREATE()         OS_ASSERT(watchdog_mutex == NULL); \
                                        OS_MUTEX_CREATE(watchdog_mutex);   \
                                        OS_ASSERT(watchdog_mutex)
#define WATCHDOG_MUTEX_GET()            OS_ASSERT(watchdog_mutex); \
                                        OS_MUTEX_GET(watchdog_mutex, OS_MUTEX_FOREVER)
#define WATCHDOG_MUTEX_PUT()            OS_MUTEX_PUT(watchdog_mutex)
#endif
#else
#define WATCHDOG_MUTEX_CREATE()
#define WATCHDOG_MUTEX_GET()
#define WATCHDOG_MUTEX_PUT()
#endif /* OS_PRESENT */

/* number of tasks registered for wdog */
__RETAINED static int8_t max_task_id;

/* bitmask of tasks identifiers which are registered */
__RETAINED static uint32_t tasks_mask;

/* bitmask of tasks identifiers which are monitored (registered and not suspended) */
__RETAINED static uint32_t tasks_monitored_mask;

/* bitmask of tasks which notified during last period */
__RETAINED volatile static uint32_t notified_mask;

/* allowed latency set by tasks, if any */
__RETAINED static uint8_t tasks_latency[dg_configWDOG_MAX_TASKS_CNT];

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
#if defined(OS_PRESENT)
/* handles of monitored tasks */
__RETAINED OS_TASK tasks_handle[dg_configWDOG_MAX_TASKS_CNT];
#endif
#endif

/* the wdog id of the IDLE task */
__RETAINED_RW static int8_t idle_task_id = -1;

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
/* bitmask of tasks which requested notification trigger from common timer */
__RETAINED static uint32_t tasks_notify_mask;

/* task handle for tasks which requested notification trigger from common timer */
__RETAINED static OS_TASK tasks_notify_handle[dg_configWDOG_MAX_TASKS_CNT];

/* timer handle for common notification trigger */
__RETAINED static OS_TIMER tasks_notify_timer;
#endif

/* Store the last reload value set to the watchdog */
__RETAINED static uint16_t watchdog_reload_value;

#define VALIDATE_ID(id) \
        do { \
                if ((id) < 0 || (id) >= dg_configWDOG_MAX_TASKS_CNT) { \
                        OS_ASSERT(0); \
                        return; \
                } \
        } while (0)

#if dg_configUSE_WDOG
__RETAINED_CODE static void reset_watchdog(void)
{
        notified_mask = 0;
        sys_watchdog_set_pos_val(dg_configWDOG_RESET_VALUE);
}

__RETAINED_CODE static void watchdog_cb(unsigned long *exception_args)
{
        uint32_t tmp_mask = tasks_monitored_mask;
        uint32_t latency_mask = 0;
        int i;

        /*
         * watchdog is reset immediately when we detect that all tasks notified during period so
         * no need to check this here
         *
         * but if we're here, then check if some tasks have non-zero latency and remove them from
         * notify mask check and also decrease latency for each task
         */
        for (i = 0; i <= max_task_id; i++) {
                if (tasks_latency[i] == 0) {
                        continue;
                }

                tasks_latency[i]--;

                latency_mask |= (1 << i);
        }

        /*
         * check if all remaining tasks notified and reset hw_watchdog in such case
         */
        tmp_mask &= ~latency_mask;
        if ((notified_mask & tmp_mask) == tmp_mask) {
                goto reset_wdog;
        }

        /*
         * latency for all tasks expired and some of them still did not notify sys_watchdog
         * we'll let watchdog reset the system
         *
         * note that hw_watchdog_handle_int() never returns
         */
        hw_watchdog_handle_int(exception_args);

reset_wdog:
        reset_watchdog();
        /*
         *  Wait until the programmed WDOG_VAL is updated in the Watchdog timer.
         *  If we do not wait then the NMI IRQ will be raised continuously calling
         *  NMI Handler and watchdog_cb
         */
        while (hw_watchdog_check_write_busy());
}
#endif

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
static void watchdog_auto_notify_cb(OS_TIMER timer)
{
        int i;

        WATCHDOG_MUTEX_GET();

        for (i = 0; i <= max_task_id; i++) {
                if (tasks_notify_handle[i]) {
                        OS_TASK_NOTIFY(tasks_notify_handle[i], SYS_WATCHDOG_TRIGGER, OS_NOTIFY_SET_BITS);
                }
        }

        WATCHDOG_MUTEX_PUT();
}
#endif

#endif

void sys_watchdog_init(void)
{
#if dg_configUSE_WDOG
        max_task_id = 0;
        notified_mask = 0;

        sys_watchdog_set_pos_val(dg_configWDOG_RESET_VALUE);

        WATCHDOG_MUTEX_CREATE();

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
        tasks_notify_timer = OS_TIMER_CREATE("wdog",
                        OS_MS_2_TICKS(dg_configWDOG_NOTIFY_TRIGGER_TMO),
                        OS_TIMER_RELOAD, NULL, watchdog_auto_notify_cb);
#endif
#endif
}

int8_t sys_watchdog_register(bool notify_trigger)
{
#if dg_configUSE_WDOG
        int8_t id = 0;

        WATCHDOG_MUTEX_GET();

        while (tasks_mask & (1 << id)) {
                id++;
        }

        if (id >= dg_configWDOG_MAX_TASKS_CNT) {
                /* Don't allow registration of more than dg_configWDOG_MAX_TASKS_CNT */
                OS_ASSERT(0);
                return -1;
        }

        tasks_mask |= (1 << id);
        tasks_monitored_mask |= (1 << id);

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
#if defined(OS_PRESENT)
        tasks_handle[id] = OS_GET_CURRENT_TASK();
#endif
#endif

        if (id > max_task_id) {
                max_task_id = id;
        }

        if (id == 0) {
                hw_watchdog_register_int(watchdog_cb);
        }

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
        if (notify_trigger) {
                /* this is first task to request trigger - start timer */
                if (!tasks_notify_mask) {
                        OS_TIMER_START(tasks_notify_timer, OS_TIMER_FOREVER);
                }

                tasks_notify_mask |= (1 << id);
                tasks_notify_handle[id] = OS_GET_CURRENT_TASK();
        }
#endif

        WATCHDOG_MUTEX_PUT();

        return id;
#else
        return 0;
#endif
}

void sys_watchdog_unregister(int8_t id)
{
#if dg_configUSE_WDOG
        uint32_t tmp_mask;
        int8_t new_max = 0;

        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        tasks_mask &= ~(1 << id);
        tasks_monitored_mask &= ~(1 << id);
        tasks_latency[id] = 0;

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
#if defined(OS_PRESENT)
        tasks_handle[id] = NULL;
#endif
#endif

#if dg_configWDOG_NOTIFY_TRIGGER_TMO
        tasks_notify_handle[id] = 0;
        tasks_notify_mask &= ~(1 << id);

        /* this was last task to request trigger - stop timer */
        if (!tasks_notify_mask) {
                OS_TIMER_STOP(tasks_notify_timer, OS_TIMER_FOREVER);
        }
#endif

        /* recalculate max task id */
        tmp_mask = tasks_mask;
        while (tmp_mask) {
                tmp_mask >>= 1;
                new_max++;
        }

        max_task_id = new_max;

        WATCHDOG_MUTEX_PUT();
#endif
}

void sys_watchdog_configure_idle_id(int8_t id)
{
#if dg_configUSE_WDOG
        idle_task_id = id;

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
#if defined(OS_PRESENT)
        tasks_handle[id] = OS_GET_IDLE_TASK_HANDLE();
#endif
#endif

#endif
}

void sys_watchdog_suspend(int8_t id)
{
#if dg_configUSE_WDOG
        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        tasks_monitored_mask &= ~(1 << id);

        WATCHDOG_MUTEX_PUT();
#endif
}

#if dg_configUSE_WDOG
__STATIC_INLINE void resume_monitoring(int8_t id)
{
        tasks_monitored_mask |= (1 << id);
        tasks_monitored_mask &= tasks_mask;
}
#endif

void sys_watchdog_resume(int8_t id)
{
#if dg_configUSE_WDOG
        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        resume_monitoring(id);

        WATCHDOG_MUTEX_PUT();
#endif
}

#if dg_configUSE_WDOG
__STATIC_INLINE void notify_about_task(int8_t id)
{
        /* Make sure that the requested task is one of the watched tasks */
        OS_ASSERT(tasks_mask & (1 << id));

        if (tasks_mask & (1 << id)) {
                notified_mask |= (1 << id);

                /*
                 * we also reset latency here because it's ok for app to notify before latency
                 * expired, but it should start with zero latency for next notification interval
                 */
                tasks_latency[id] = 0;

                if ((notified_mask & tasks_monitored_mask) == tasks_monitored_mask) {
                        reset_watchdog();
                }
        }

}

__STATIC_INLINE void notify_idle(int8_t id)
{
        /* Notify the IDLE task every time one of the monitored tasks notifies the service. */
        if ((id != idle_task_id) && (idle_task_id != -1)) {
                sys_watchdog_notify(idle_task_id);
        }
}
#endif

__RETAINED_HOT_CODE void sys_watchdog_notify(int8_t id)
{
#if dg_configUSE_WDOG
        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        notify_about_task(id);

        WATCHDOG_MUTEX_PUT();

        notify_idle(id);
#endif
}

void sys_watchdog_notify_and_resume(int8_t id)
{
#if dg_configUSE_WDOG
        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        resume_monitoring(id);
        notify_about_task(id);

        WATCHDOG_MUTEX_PUT();

        notify_idle(id);
#endif
}

void sys_watchdog_set_latency(int8_t id, uint8_t latency)
{
#if dg_configUSE_WDOG
        VALIDATE_ID(id);

        WATCHDOG_MUTEX_GET();

        tasks_latency[id] = latency;

        WATCHDOG_MUTEX_PUT();
#endif
}

__RETAINED_CODE bool sys_watchdog_monitor_mask_empty()
{
#if dg_configUSE_WDOG
        return ((idle_task_id != -1) && (tasks_monitored_mask == (1 << idle_task_id)));
#else
        return true;
#endif
}

__RETAINED_CODE void sys_watchdog_set_pos_val(uint16_t value)
{
#if dg_configUSE_WDOG
        if (hw_watchdog_check_write_busy()) {
                // WATCHDOG is still busy writing the previous value

                // Previous value was written using sys_watchdog_set_pos_val()
                ASSERT_WARNING(watchdog_reload_value != 0);

                if (watchdog_reload_value == value) {
                       /* The previous value was the same. No need to wait for
                          WRITE_BUSY and write it again. */
                        return;
                }
        }

        watchdog_reload_value = value;
#endif
        hw_watchdog_set_pos_val(value);
}

__RETAINED_HOT_CODE uint16_t sys_watchdog_get_val(void)
{
#if dg_configUSE_WDOG
        if (hw_watchdog_check_write_busy()) {
                // WATCHDOG is still busy writing the previous value

                // Previous value was written using sys_watchdog_set_pos_val()
                ASSERT_WARNING(watchdog_reload_value != 0);

                return watchdog_reload_value;
        }
#else
        while (hw_watchdog_check_write_busy());
#endif
        return hw_watchdog_get_val();
}
