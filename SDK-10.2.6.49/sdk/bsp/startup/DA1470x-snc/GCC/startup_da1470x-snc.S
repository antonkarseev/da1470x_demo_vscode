/**************************************************************************//**
 * @file     startup_da1470x-snc.S
 * @brief    CMSIS Core Device Startup File for DA1470x Device
 * @version  V1.1.0
 * @date     17. May 2019
 ******************************************************************************/
/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved
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

/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
*/

#if (SNC_PROCESSOR_BUILD)

#define SYS_CTRL_REG                            0x50000024
#define SYS_CTRL_REG_SNC_DEBUGGER_ENABLE_MSK    0x20

        .syntax unified
        .arch   armv6-m

        .section text_reset
        .text
        .align  2
        .thumb_func
        .type   Default_Handler, %function
        .weak   Default_Handler
Default_Handler:
        /* Enable debugger */
        ldr     r1, =SYS_CTRL_REG
        ldrh    r2, [r1, #0]
        movs    r3, #SYS_CTRL_REG_SNC_DEBUGGER_ENABLE_MSK
        orrs    r2, r2, r3
        strh    r2, [r1, #0]
        b       .
        .size   Default_Handler, . - Default_Handler

/* Macro to define default exception/interrupt handlers.
 * Default handler are weak symbols with an endless loop.
 * They can be overwritten by real handlers.
 */
        .macro  def_irq_handler  handler_name
        .weak   \handler_name
        .set    \handler_name, Default_Handler
        .endm

        .thumb
        .thumb_func
        .align  1
        .type   SVC_Handler, %function
        .weak   SVC_Handler
SVC_Handler:
        b       .
        .size   SVC_Handler, . - SVC_Handler
        def_irq_handler PendSV_Handler
        def_irq_handler SysTick_Handler

        def_irq_handler SYS2SNC_Handler
        def_irq_handler CMAC2SNC_Handler
        def_irq_handler PDC_SNC_Handler
        def_irq_handler Key_Wkup_GPIO_Handler
        def_irq_handler GPIO_P0_Handler
        def_irq_handler GPIO_P1_Handler
        def_irq_handler GPIO_P2_Handler
        def_irq_handler Timer_Handler
        def_irq_handler Timer3_Handler
        def_irq_handler Timer4_Handler
        def_irq_handler Timer5_Handler
        def_irq_handler Timer6_Handler
        def_irq_handler RTC_Handler
        def_irq_handler RTC_Event_Handler
        def_irq_handler CAPTIMER_Handler
        def_irq_handler GPADC_Handler
        def_irq_handler UART_Handler
        def_irq_handler UART2_Handler
        def_irq_handler UART3_Handler
        def_irq_handler SPI_Handler
        def_irq_handler SPI2_Handler
        def_irq_handler SPI3_Handler
        def_irq_handler I2C_Handler
        def_irq_handler I2C2_Handler
        def_irq_handler I2C3_Handler
        def_irq_handler I3C_Handler
        def_irq_handler PCM_Handler
        def_irq_handler SRC_IN_Handler
        def_irq_handler SRC_OUT_Handler
        def_irq_handler SRC2_IN_Handler
        def_irq_handler SRC2_OUT_Handler
        def_irq_handler VAD_Handler

        .end
#endif /* SNC_PROCESSOR_BUILD */
