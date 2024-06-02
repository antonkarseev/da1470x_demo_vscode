//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_irq_da1470x.c (%version: 1 %)
//          created Tue Mar 24 13:40:41 2020 by markus.hertel
//
// Description:
//  %date_modified: Tue Mar 24 13:40:41 2020 %  (%derived_by:  by markus.hertel %)
//
// Changes:
//  2020-06-16 MHe  started
//
// Copyright by TES Electronic Solutions GmbH, www.tes-dst.com. All rights reserved.
//--------------------------------------------------------------------------
//
/* Copyright (c) 2020-2022 Modified by Dialog Semiconductor */

#include <stdlib.h>
#include <dave_base.h>
#include <dave_base_da1470x.h>

#ifdef OS_PRESENT
#include "osal.h"
#include "sys_power_mgr.h"
#endif

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

//--------------------------------------------------------------------------
//
enum d1_irqslots {
#if DAVE2D_0_ENABLE_BREAK_IRQ
        d1_irqslot_break,
#endif
#if DAVE2D_0_ENABLE_VBI_IRQ
        d1_irqslot_vbi,
#endif
        d1_irqslot_dlist,
        d1_irqcount
};


//--------------------------------------------------------------------------
// Static IRQ variables
//
__RETAINED static d1_interrupt g_irq_handler[d1_irqcount];
__RETAINED static void *g_irq_data[d1_irqcount];
#ifdef OS_PRESENT
__RETAINED static OS_EVENT g_irq_event[d1_irqcount];
#else
__RETAINED static volatile int g_irq_triggered[d1_irqcount];
#endif

//--------------------------------------------------------------------------
// DA1470x specific external handle for IRQ callback GPU_Handler
//
extern d1_device *g_d1_device;

//--------------------------------------------------------------------------
// DA1470x specific callback function
//
void d1_dave2d_isr(void *context);

void GPU_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        d1_dave2d_isr(g_d1_device);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}


//--------------------------------------------------------------------------
// internal
//
static int d1_mapirq_intern(int irqtype)
{
        int index;

        // map irqtype bit to slot index
        switch (irqtype) {
#if DAVE2D_0_ENABLE_BREAK_IRQ
        case d1_irq_break:
                index = d1_irqslot_break;
                break;
#endif
#if DAVE2D_0_ENABLE_VBI_IRQ
        case d1_irq_vbi:
                index = d1_irqslot_vbi;
                break;
#endif
        case d1_irq_dlist:
                index = d1_irqslot_dlist;
                break;

        default:
                index = -1;
                break;
        }

        return index;
}


//--------------------------------------------------------------------------
//
void d1_dave2d_isr(void *context)
{
        uint32_t intReg;
        d1_device_da1470x *dev = (d1_device_da1470x*)context;

        intReg = READ_REG(DAVE2D_0_BASE, DAVE2D_0_STATUS);

        if (intReg & (DAVE2D_0_STATUS_IRQ_ENUM | DAVE2D_0_STATUS_IRQ_DLIST)) {  // any DAVE interrupt?
                // clear/enable all DAVE interrupts
                WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_IRQ_CTRL, DAVE2D_0_IRQ_CTRL_ENABLE_FINISH_DLIST
                        | DAVE2D_0_IRQ_CTRL_CLR_FINISH_ENUM | DAVE2D_0_IRQ_CTRL_CLR_FINISH_DLIST);

                if (dev == NULL) {
                        return;
                }

                if (intReg & DAVE2D_0_STATUS_IRQ_DLIST) { // display list interrupt?
                        if (dev->dlist_indirect && *(dev->dlist_start) != 0) {
                                long dlist_addr;
                                dlist_addr = *(dev->dlist_start);
                                /* get next dlist start address */
                                dev->dlist_start++;
                                /* starting Dave */
                                WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_DLISTSTART, dlist_addr);
                        } else {
                                if (g_irq_handler[d1_irqslot_dlist]) {
                                        g_irq_handler[d1_irqslot_dlist](d1_irq_dlist, 0,
                                                g_irq_data[d1_irqslot_dlist]);
                                }
#ifdef OS_PRESENT
                                OS_EVENT_SIGNAL_FROM_ISR(g_irq_event[d1_irqslot_dlist]);
#else
                                g_irq_triggered[d1_irqslot_dlist] = 1;
#endif
                        }
                }
        }
}


//--------------------------------------------------------------------------
// Register an IRQ handler
//
void d1_setirqhandler(d1_device *handle, int irqtype, d1_interrupt code, void *data)
{
        int irqslot;

        // unused arguments
        (void)handle;

        // get slot
        irqslot = d1_mapirq_intern(irqtype);
        if (irqslot < 0)
                return;

        // store irq handler and pointer
        g_irq_handler[irqslot] = code;
        g_irq_data[irqslot] = data;
}


//--------------------------------------------------------------------------
// Retrieve an IRQ handler
//
d1_interrupt d1_getirqhandler(d1_device *handle, int irqtype)
{
        int irqslot;

        // unused arguments
        (void)handle;

        // get slot
        irqslot = d1_mapirq_intern(irqtype);
        if (irqslot < 0)
                return NULL;

        return g_irq_handler[irqslot];
}


//--------------------------------------------------------------------------
// Retrieve userdefined data of specified IRQ
//
void* d1_getirqdata(d1_device *handle, int irqtype)
{
        int irqslot;

        // unused arguments
        (void)handle;

        // get slot
        irqslot = d1_mapirq_intern(irqtype);
        if (irqslot < 0)
                return NULL;

        return g_irq_data[irqslot];
}


//--------------------------------------------------------------------------
// Wait for next execution of specified IRQ
//
int d1_queryirq(d1_device *handle, int irqmask, int timeout)
{
        // unused arguments
        (void)handle;

        // break + vip interrupts are not supported
        if (irqmask & ~d1_irq_break & ~d1_irq_vip) {
#ifdef OS_PRESENT
                OS_TICK_TIME os_timeout;
                if (timeout == d1_to_no_wait) {
                        os_timeout = OS_MUTEX_NO_WAIT;
                } else if (timeout == d1_to_wait_forever) {
                        os_timeout = OS_MUTEX_FOREVER;
                } else {
                        os_timeout = OS_TIME_TO_TICKS(timeout);
                }

                pm_sleep_mode_request(pm_mode_idle);

                // wait for irq
#if DAVE2D_0_ENABLE_VBI_IRQ
                if (irqmask & d1_irq_vbi) {
                        if (OS_EVENT_WAIT(g_irq_event[d1_irqslot_vbi], os_timeout) == OS_EVENT_SIGNALED) {
                                pm_sleep_mode_release(pm_mode_idle);
                                return d1_irq_vbi;
                        }
                }
#endif
                if (irqmask & d1_irq_dlist) {
                        if (OS_EVENT_WAIT(g_irq_event[d1_irqslot_dlist], os_timeout) == OS_EVENT_SIGNALED) {
                                pm_sleep_mode_release(pm_mode_idle);
                                return d1_irq_dlist;
                        }
                }

                pm_sleep_mode_release(pm_mode_idle);
#else
                while (1) {
#if DAVE2D_0_ENABLE_VBI_IRQ
                        if ((irqmask & d1_irq_vbi) && g_irq_triggered[d1_irqslot_vbi]) {
                                g_irq_triggered[d1_irqslot_vbi] = 0;
                                return d1_irq_vbi;
                        }
#endif
                        if ((irqmask & d1_irq_dlist) && g_irq_triggered[d1_irqslot_dlist]) {
                                g_irq_triggered[d1_irqslot_dlist] = 0;
                                return d1_irq_dlist;
                        }
                        if (timeout == d1_to_no_wait)
                                return 0;
                }
#endif
        }
        return 0;
}

//--------------------------------------------------------------------------
// Call IRQ handler
//
int d1_callirqhandler(d1_device *handle, int irqtype, void *irqdata)
{
        int irqslot;

        // unused arguments
        (void)handle;

        // get slot
        irqslot = d1_mapirq_intern(irqtype);
        if (irqslot < 0)
                return 0;

        // call irq handler
        if (g_irq_handler[irqslot]) {
                g_irq_handler[irqslot](irqtype, irqdata, g_irq_data[irqslot]);
        }

        return 1;
}

//--------------------------------------------------------------------------
// initializaton + deinitialization
//
int d1_initirq_intern(d1_device *handle)
{
        int i;
        int error = 0;

        for (i = 0; i < d1_irqcount; ++i) {
                g_irq_handler[i] = NULL;
                g_irq_data[i] = NULL;
#ifdef OS_PRESENT
                OS_EVENT_CREATE(g_irq_event[i]);
#else
                g_irq_triggered[i] = 0;
#endif
        }

        NVIC_EnableIRQ(GPU_IRQn);

        return !error;
}

int d1_deinitirq_intern(d1_device *handle)
{
        int i;
        int error = 0;

        NVIC_DisableIRQ(GPU_IRQn);

        for (i = 0; i < d1_irqcount; ++i) {
                g_irq_handler[i] = NULL;
                g_irq_data[i] = NULL;
#ifdef OS_PRESENT
                OS_EVENT_DELETE(g_irq_event[i]);
                g_irq_event[i] = NULL;
#else
                g_irq_triggered[i] = 0;
#endif
        }

        return !error;
}

