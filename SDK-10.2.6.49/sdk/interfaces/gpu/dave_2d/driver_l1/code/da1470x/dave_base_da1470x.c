//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_base_da1470x.c (%version: 1 %)
//          created Tue Mar 24 13:41:37 2020 by markus.hertel
//
// Description:
//  %date_modified: Tue Mar 24 13:41:37 2020 %  (%derived_by:  by markus.hertel %)
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
#include "osal.h"
#include "hw_clk.h"
#include "hw_sys.h"

extern int d1_initirq_intern(d1_device *handle);
extern int d1_deinitirq_intern(d1_device *handle);

//--------------------------------------------------------------------------
// DA1470x specific handle for IRQ callback GPU_Handler
//
__RETAINED d1_device *g_d1_device;


//--------------------------------------------------------------------------
// Update the values of the GPU registers
//
static void d1_updategpuconfig(d1_device *handle, int index, uint32_t value)
{
        d1_device_da1470x *dev = (d1_device_da1470x*)handle;

        switch (index) {
        case DAVE2D_0_CONTROL3:
                dev->gpu_reg.d2_control3_reg = value;
                break;

        case DAVE2D_0_IRQ_CTRL:
                dev->gpu_reg.d2_irqctrl_reg = value;
                break;

        case DAVE2D_0_CACHECTL:
                dev->gpu_reg.d2_cachectl_reg = value;
                break;

        case DAVE2D_0_PERFCOUNT1:
                dev->gpu_reg.d2_perfcount1_reg = value;
                break;

        case DAVE2D_0_PERFCOUNT2:
                dev->gpu_reg.d2_perfcount2_reg = value;
                break;

        case DAVE2D_0_PERFTRIGGER:
                dev->gpu_reg.d2_perftrigger_reg = value;
                break;

        default:
                break; // unknown device
        }
}


//--------------------------------------------------------------------------
// Restore the GPU registers
//
static void d1_restoregpuconfig(d1_device *handle)
{
        d1_device_da1470x *dev = (d1_device_da1470x*)handle;

        WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_CONTROL3,    dev->gpu_reg.d2_control3_reg);
        WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_IRQ_CTRL,    dev->gpu_reg.d2_irqctrl_reg);
        WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_CACHECTL,    dev->gpu_reg.d2_cachectl_reg);
        WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_PERFCOUNT1,  dev->gpu_reg.d2_perfcount1_reg);
        WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_PERFCOUNT2,  dev->gpu_reg.d2_perfcount2_reg);
        WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_PERFTRIGGER, dev->gpu_reg.d2_perftrigger_reg);
}


//--------------------------------------------------------------------------
// Create a device handle (required for all other functions) to access hardware
//
d1_device* d1_opendevice(long flags)
{
        d1_device_da1470x *handle;
        d1_device *ret;

        // unused arguments
        (void)flags;

        // get new device context structure
        handle = (d1_device_da1470x*)OS_MALLOC(sizeof(d1_device_da1470x));
        if (handle == NULL) {
                return NULL;
        }
        g_d1_device = handle;

        // init device data
        handle->dlist_indirect = 0;

        hw_sys_pd_gpu_enable();                                 // Wake up the GPU

        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_GPU, CLK_GPU_REG, GPU_ENABLE, 0x1);        // Turn on gpu clock
        GLOBAL_INT_RESTORE();

        REG_SETF(GPU_REG, GPU_CTRL_REG, GPU_EN, 0x1);           // Turn on GPU

        d1_initirq_intern(handle);

        WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_CONTROL3, 3 | (3 << 8) | (3 << 16) | (3 << 24));
        d1_updategpuconfig(handle, DAVE2D_0_CONTROL3, 3 | (3 << 8) | (3 << 16) | (3 << 24));

        ret = (d1_device*)handle;

        /* Set dlist IRQ enable and clear */
        WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_IRQ_CTRL, DAVE2D_0_IRQ_CTRL_ENABLE_FINISH_DLIST | DAVE2D_0_IRQ_CTRL_CLR_FINISH_DLIST);
        d1_updategpuconfig(handle, DAVE2D_0_IRQ_CTRL, DAVE2D_0_IRQ_CTRL_ENABLE_FINISH_DLIST | DAVE2D_0_IRQ_CTRL_CLR_FINISH_DLIST);

        return ret;
}


//--------------------------------------------------------------------------
// Close a device handle
//
int d1_closedevice(d1_device *handle)
{
        if (!handle) {
                return 0;
        }

        /* Disable and clear IRQs */
        WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_IRQ_CTRL, DAVE2D_0_IRQ_CTRL_CLR_BUS_ERROR
                | DAVE2D_0_IRQ_CTRL_CLR_FINISH_DLIST | DAVE2D_0_IRQ_CTRL_CLR_FINISH_ENUM);

        d1_deinitirq_intern(handle);

        REG_SETF(GPU_REG, GPU_CTRL_REG, GPU_EN, 0x0);           // Turn off GPU

        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_GPU, CLK_GPU_REG, GPU_ENABLE, 0x0);        // Turn off gpu clock
        GLOBAL_INT_RESTORE();

        hw_sys_pd_gpu_disable();                                // Power down the GPU

        OS_FREE(handle);
        g_d1_device = NULL;

        return 1;
}


//--------------------------------------------------------------------------
// Write to hardware register
//
void d1_setregister(d1_device *handle, int deviceid, int index, long value)
{
        d1_device_da1470x *dev = (d1_device_da1470x*)handle;

        switch (deviceid) {
        case D1_DAVE2D:
                if (index == DAVE2D_0_DLISTSTART && dev->dlist_indirect) {
                        int *dlistp = (int*)value;
                        dev->dlist_start = (long*)(dlistp + 1);
                        WRITE_REG(DAVE2D_0_BASE, index, *dlistp);
                }
                else {
                        WRITE_REG(DAVE2D_0_BASE, index, value);
                        d1_updategpuconfig(handle, index, value);
                }
                break;

        case D1_DLISTINDIRECT:
#if DAVE2D_0_USE_DLIST_INDIRECT
                dev->dlist_indirect = value;
#else
                dev->dlist_indirect = 0;
#endif
                break;

        default:
                break; // unknown device
        }
}


//--------------------------------------------------------------------------
// Read from hardware register
//
long d1_getregister(d1_device *handle, int deviceid, int index)
{
        d1_device_da1470x *dev = (d1_device_da1470x*)handle;

        switch (deviceid) {
        case D1_DAVE2D:
                return READ_REG(DAVE2D_0_BASE, index);

        case D1_DLISTINDIRECT:
                return dev->dlist_indirect;

        case D1_TOUCHSCREEN:
                // Stub for handling the touch input later on
                switch (index) {
                case 1: // Touchscreen Pendown
                        return 0;

                case 2: // Touchscreen PenX
                        return 0;

                case 3: // Touchscreen PenY
                        return 0;

                default:
                        return 0;
                }
                break;
        default:
                return 0;
        }
}


//--------------------------------------------------------------------------
// Check if a specific registermap is available
//
int d1_devicesupported(d1_device *handle, int deviceid)
{
        // unused arguments
        (void)handle;

        switch (deviceid) {
        case D1_DAVE2D:
#if DAVE2D_0_USE_DLIST_INDIRECT
        case D1_DLISTINDIRECT:
#endif
                return 1;

        default:
                // unknown device
                return 0;
        }
}


//--------------------------------------------------------------------------
// Get device clock frequency stub
//
unsigned long d1_deviceclkfreq(d1_device *handle, int deviceid)
{
        // unused arguments
        (void)handle;
        (void)deviceid;

        return hw_clk_get_sysclk_freq();
}


//--------------------------------------------------------------------------
// Power off GPU
//
void d1_gpupowerdown(void)
{
        if (g_d1_device != NULL) {
                d1_updategpuconfig(g_d1_device, DAVE2D_0_PERFCOUNT1, READ_REG(DAVE2D_0_BASE, DAVE2D_0_PERFCOUNT1));
                d1_updategpuconfig(g_d1_device, DAVE2D_0_PERFCOUNT2, READ_REG(DAVE2D_0_BASE, DAVE2D_0_PERFCOUNT2));

                /* Disable and clear IRQs */
                WRITE_REG(DAVE2D_0_BASE, DAVE2D_0_IRQ_CTRL, DAVE2D_0_IRQ_CTRL_CLR_BUS_ERROR
                        | DAVE2D_0_IRQ_CTRL_CLR_FINISH_DLIST | DAVE2D_0_IRQ_CTRL_CLR_FINISH_ENUM);

                REG_SETF(GPU_REG, GPU_CTRL_REG, GPU_EN, 0x0);           // Turn off GPU

                GLOBAL_INT_DISABLE();
                REG_SETF(CRG_GPU, CLK_GPU_REG, GPU_ENABLE, 0x0);        // Turn off gpu clock
                GLOBAL_INT_RESTORE();

                hw_sys_pd_gpu_disable();                                // Power down the GPU
        }
}



//--------------------------------------------------------------------------
// Power on GPU and restore the configuration of the GPU registers
//
void d1_gpupowerup(void)
{
        if (g_d1_device != NULL) {
                hw_sys_pd_gpu_enable();                                 // Wake up the GPU

                GLOBAL_INT_DISABLE();
                REG_SETF(CRG_GPU, CLK_GPU_REG, GPU_ENABLE, 0x1);        // Turn on gpu clock
                GLOBAL_INT_RESTORE();

                REG_SETF(GPU_REG, GPU_CTRL_REG, GPU_EN, 0x1);           // Turn on GPU

                d1_restoregpuconfig(g_d1_device);
        }
}
