//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_base_da1470x.h (%version: 1 %)
//          created Tue Mar 24 13:41:07 2020 by markus.hertel
//
// Description:
//  %date_modified: Tue Mar 24 13:41:07 2020 %  (%derived_by:  by markus.hertel %)
//
// Changes:
//  2020-06-16 MHe  started
//
// Copyright by TES Electronic Solutions GmbH, www.tes-dst.com. All rights reserved.
//--------------------------------------------------------------------------
//
/* Copyright (c) 2020-2022 Modified by Dialog Semiconductor */

#ifndef __1_dave_base_da1470x_h_H
#define __1_dave_base_da1470x_h_H

//---------------------------------------------------------------------------
// da1470x specific include for RAW_SET / RAW_GET functions
//
#include <sdk_defs.h>
#include "dave_base.h"

#ifndef DAVE2D_0_USE_DLIST_INDIRECT
#define DAVE2D_0_USE_DLIST_INDIRECT             (1)
#endif

#ifndef DAVE2D_0_ENABLE_BREAK_IRQ
#define DAVE2D_0_ENABLE_BREAK_IRQ               (0)
#endif

#ifndef DAVE2D_0_ENABLE_VBI_IRQ
#define DAVE2D_0_ENABLE_VBI_IRQ                 (0)
#endif

#define DAVE2D_0_BASE                           GPU_CORE_BASE
#define DAVE2D_0_PERFTRIGGER                    53U
#define DAVE2D_0_PERFCOUNT2                     52U
#define DAVE2D_0_PERFCOUNT1                     51U
#define DAVE2D_0_DLISTSTART                     50U
#define DAVE2D_0_CACHECTL                       49U
#define DAVE2D_0_IRQ_CTRL                       48U
#define DAVE2D_0_CONTROL3                       2U
#define DAVE2D_0_STATUS                         0U

#define DAVE2D_0_STATUS_IRQ_BUS_ERROR           REG_MSK(GPU_CORE, D2_STATUS, D2C_IRQ_BUS_ERROR)
#define DAVE2D_0_STATUS_IRQ_DLIST               REG_MSK(GPU_CORE, D2_STATUS, D2C_IRQ_DLIST)
#define DAVE2D_0_STATUS_IRQ_ENUM                REG_MSK(GPU_CORE, D2_STATUS, D2C_IRQ_ENUM)
#define DAVE2D_0_STATUS_DLISTACTIVE             REG_MSK(GPU_CORE, D2_STATUS, D2C_DLISTACTIVE)
#define DAVE2D_0_STATUS_CACHE_DIRTY             REG_MSK(GPU_CORE, D2_STATUS, D2C_CACHE_DIRTY)
#define DAVE2D_0_STATUS_BUSY_WRITE              REG_MSK(GPU_CORE, D2_STATUS, D2C_BUSY_WRITE)
#define DAVE2D_0_STATUS_BUSY_ENUM               REG_MSK(GPU_CORE, D2_STATUS, D2C_BUSY_ENUM)

#define DAVE2D_0_IRQ_CTRL_CLR_BUS_ERROR         REG_MSK(GPU_CORE, D2_IRQCTL, D2IRQCTL_CLR_BUS_ERROR)
#define DAVE2D_0_IRQ_CTRL_ENABLE_BUS_ERROR      REG_MSK(GPU_CORE, D2_IRQCTL, D2IRQCTL_ENABLE_BUS_ERROR)
#define DAVE2D_0_IRQ_CTRL_CLR_FINISH_DLIST      REG_MSK(GPU_CORE, D2_IRQCTL, D2IRQCTL_CLR_FINISH_DLIST)
#define DAVE2D_0_IRQ_CTRL_CLR_FINISH_ENUM       REG_MSK(GPU_CORE, D2_IRQCTL, D2IRQCTL_CLR_FINISH_ENUM)
#define DAVE2D_0_IRQ_CTRL_ENABLE_FINISH_DLIST   REG_MSK(GPU_CORE, D2_IRQCTL, D2IRQCTL_ENABLE_FINISH_DLIST)
#define DAVE2D_0_IRQ_CTRL_ENABLE_FINISH_ENUM    REG_MSK(GPU_CORE, D2_IRQCTL, D2IRQCTL_ENABLE_FINISH_ENUM)

//--------------------------------------------------------------------------
// RAW_SET / RAW_GET macro
//
#define WRITE_REG( BASE, OFFSET, DATA ) \
        RAW_SETF( ((BASE)+((OFFSET) << 2 )), (0xFFFFFFFF), (DATA) )


#define READ_REG( BASE, OFFSET ) \
        RAW_GETF( ((BASE)+((OFFSET) << 2 )), (0xFFFFFFFF) )


//---------------------------------------------------------------------------
//
typedef struct _d1_device_da1470x_reg
{
        int d2_control3_reg;
        int d2_irqctrl_reg;
        int d2_cachectl_reg;
        int d2_perfcount1_reg;
        int d2_perfcount2_reg;
        int d2_perftrigger_reg;
} d1_device_da1470x_reg;

typedef struct _d1_device_da1470x
{
        volatile long *dlist_start; /* dlist start addresses */
        int dlist_indirect;
        d1_device_da1470x_reg gpu_reg;
} d1_device_da1470x;

//---------------------------------------------------------------------------
//
extern int d1_initdisplay_intern( d1_device *handle );
extern int d1_shutdowndisplay_intern( d1_device *handle );

//--------------------------------------------------------------------------
// Power off GPU
//
void d1_gpupowerdown(void);

//--------------------------------------------------------------------------
// Power on GPU and restore the configuration of the GPU registers
//
void d1_gpupowerup(void);

#endif /* __1_dave_base_da1470x_h_H */
