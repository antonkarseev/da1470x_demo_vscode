//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_timer_da1470x.c (%version: 1 %)
//          created Tue Mar 24 13:28:43 2020 by markus.hertel
//
// Description:
//  %date_modified: Tue Mar 24 13:28:43 2020 %  (%derived_by:  by markus.hertel %)
//
// Changes:
//  2020-06-16 MHe  started
//
// Copyright by TES Electronic Solutions GmbH, www.tes-dst.com. All rights reserved.
//--------------------------------------------------------------------------
//
/* Copyright (c) 2020-2022 Modified by Dialog Semiconductor */

#include <stdio.h>
#include <dave_base.h>
#include <sdk_defs.h>

#ifdef OS_PRESENT
#include "osal.h"
#include "sys_timer.h"
#include "interrupts.h"
#else
#include <time.h>
#endif

//--------------------------------------------------------------------------
//
#ifdef OS_PRESENT
static uint64_t timer_start;
#else
static time_t timer_start;
#endif

//--------------------------------------------------------------------------
// Get the resolution of the timer (in us)
//
__WEAK unsigned long d1_timerres(d1_device *handle)
{
        /* unused arguments */
        (void)handle;

#ifdef OS_PRESENT
        return 1000000 / OS_TICK_CLOCK_HZ;
#else
        return 1000000; // s to us
#endif
}

//--------------------------------------------------------------------------
// Get the maximum value of the timer
//
__WEAK unsigned long d1_timerlimit(d1_device *handle)
{
        /* unused arguments */
        (void)handle;

        return (unsigned long)(-1) - timer_start;
}

//--------------------------------------------------------------------------
// Reset the timer to zero
//
__WEAK void d1_timerreset(d1_device *handle)
{
        /* unused arguments */
        (void)handle;

        /* save current tick count as a reference */
#ifdef OS_PRESENT
        timer_start = (in_interrupt() ?
                                        sys_timer_get_uptime_usec_fromISR() :
                                        sys_timer_get_uptime_usec());
#else
        time(&timer_start);
#endif
}

//--------------------------------------------------------------------------
// Get the current timer value in microseconds
//
__WEAK unsigned long d1_timervalue(d1_device *handle)
{
        /* unused arguments */
        (void)handle;

#ifdef OS_PRESENT
        uint64_t timer_end = (in_interrupt() ?
                                               sys_timer_get_uptime_usec_fromISR() :
                                               sys_timer_get_uptime_usec());
        return (unsigned long)(timer_end - timer_start);
#else
        time_t timer_end;

        time(&timer_end);
        return (unsigned long)(difftime(timer_end, timer_start) * 1000000.0f);
#endif
}
