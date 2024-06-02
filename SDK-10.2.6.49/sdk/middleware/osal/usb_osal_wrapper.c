/**
 ****************************************************************************************
 *
 * @file usb_osal_wrapper.c
 *
 * @brief OS abstraction layer API
 *
 * Copyright (C) 2016-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if defined(OS_PRESENT) && (dg_configUSE_HW_USB == 1)
#include "osal.h"

#if (dg_configUSE_SYS_CHARGER == 1)
void sys_usb_charger_enumeration_done(void);
#endif

void wrapper_os_queue_create(OS_QUEUE *queue, OS_UBASE_TYPE item_size, OS_UBASE_TYPE max_items)
{
        OS_QUEUE_CREATE(*queue, item_size, max_items);
}

void wrapper_vQueueDelete(OS_QUEUE *queue)
{
        OS_QUEUE_DELETE(*queue);
}

void wrapper_os_queue_overwrite_from_isr(OS_QUEUE *queue, unsigned *TransactCnt,
        OS_BASE_TYPE *xHigherPriorityTaskWoken)
{
        OS_QUEUE_REPLACE_FROM_ISR_NO_YIELD(*queue, TransactCnt, xHigherPriorityTaskWoken);
        //
        // If xHigherPriorityTaskWoken is now set to OS_TRUE then a context
        // switch should be requested.
        //
        if (xHigherPriorityTaskWoken != OS_FALSE) {
                OS_TASK_YIELD_FROM_ISR();
        }
}

void wrapper_os_queue_overwrite(OS_QUEUE *queue, unsigned *TransactCnt)
{
        OS_QUEUE_REPLACE(*queue, TransactCnt);
}

OS_TICK_TIME wrapper_os_ms_2_ticks(unsigned ms)
{
        return OS_MS_2_TICKS(ms);
}

OS_BASE_TYPE wrapper_os_queue_receive(OS_QUEUE *queue, unsigned *Cnt, OS_TICK_TIME Ticks)
{
        return OS_QUEUE_GET(*queue, Cnt, Ticks);
}

void wrapper_os_leave_critical_section(void)
{
        OS_LEAVE_CRITICAL_SECTION();
}

void wrapper_os_enter_critical_section(void)
{
        OS_ENTER_CRITICAL_SECTION();
}

void wrapper_os_delay_ms(int ms)
{
        OS_DELAY_MS(ms);
}

OS_TICK_TIME wrapper_os_get_tick_count(void)
{
        return OS_GET_TICK_COUNT();
}

uint32_t wrapper_os_ticks_2_ms(OS_TICK_TIME ticks)
{
        return OS_TICKS_2_MS(ticks);
}
void wrapper_usb_charger_connected(void)
{
#if (dg_configUSE_SYS_CHARGER == 1)
        sys_usb_charger_enumeration_done();
#endif
}
#endif /* defined(OS_PRESENT) && (dg_configUSE_HW_USB == 1) */

