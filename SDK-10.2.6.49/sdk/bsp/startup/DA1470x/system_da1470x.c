/**************************************************************************//**
 * @file     system_da1470x.c
 * @brief    CMSIS Device System Source File for DA1470x Device
 * @version  V5.3.1
 * @date     17. May 2019
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* Copyright (c) 2017-2021 Modified by Dialog Semiconductor */

#if (MAIN_PROCESSOR_BUILD)

#include "system_DA1470x.h"
#include "sdk_defs.h"


/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#ifndef __SYSTEM_CLOCK
# define __SYSTEM_CLOCK                 (dg_configRCHS_32M_FREQ)
#endif

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
__RETAINED_RW uint32_t SystemCoreClock = __SYSTEM_CLOCK;        /*!< System Clock Frequency (Core Clock) */


/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
  SystemCoreClock = __SYSTEM_CLOCK;
}

/*----------------------------------------------------------------------------
  System initialization function
  @brief  Setup the microcontroller system.
          Initialize the System.
 *----------------------------------------------------------------------------*/
void SystemInit(void)
{
        SystemCoreClock = __SYSTEM_CLOCK;

#if defined(__FPU_USED) && (__FPU_USED == 1U)
        SCB->CPACR |= ((3U << 10U * 2U) |           /* enable CP10 Full Access */
                       (3U << 11U * 2U));           /* enable CP11 Full Access */

        /* ASPEN = 1 ==> Hardware should automatically preserve floating point
         * context on exception entry and restore on exception return.
         * LSPEN = 1 ==> Enable lazy context save of FP state. */
        FPU->FPCCR |= (FPU_FPCCR_ASPEN_Msk | FPU_FPCCR_LSPEN_Msk);
#endif
}


#endif /* MAIN_PROCESSOR_BUILD */
