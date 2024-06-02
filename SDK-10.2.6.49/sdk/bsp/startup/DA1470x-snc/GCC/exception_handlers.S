/*
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

#if (SNC_PROCESSOR_BUILD)

/* Micro Trace Buffer registers */
#if dg_configENABLE_MTB
                .equ MTB_POSITION_REG,              0xF0002000
                .equ MTB_MASTER_REG,                0xF0002004
                .equ MTB_FLOW_REG,                  0xF0002008
                .equ MTB_MASTER_REG_ENABLE_MSK,     0x80000008  /* Enable MTB, 4KB MTB buffer size */
#endif

                .syntax unified
                .arch   armv6-m

                .equ CLK_SNC_CTRL_REG, 0x5000002C
                .equ CLK_SNC_CTRL_REG_SNC_STATE_RETAINED_BIT, 2
                .equ PDC_PENDING_SNC_REG,           0x50000288

                .section .text
                .thumb
                .thumb_func
                .align  2
                .globl  Reset_Handler
                .type   Reset_Handler, %function
                .globl __SNC_sleep_status
                .type __SNC_sleep_status, %object
Reset_Handler:
                /*
                 *  Check if wakeup was done due to SNC PDC entry trigger.
                 *  If wake up is triggered by PD_SNC power ON
                 *  SNC should go to deep sleep as soon as possible
                 */
.start_over:
#if dg_configENABLE_SNC_SLEEP_STATUS
               /*
                * set sleep status to awake by Reset_handler
                */
                ldr     r3, =__SNC_sleep_status
                movs    r4, #SNC_ACTIVE_RESET_HANDLER
                str     r4, [r3]
#endif

                ldr     r3, =PDC_PENDING_SNC_REG
                ldr     r4, [r3]
                movs    r3, #0
                cmp     r4, r3
                beq     .no_valid_wakeup

                /*
                 *  Determine if this is a cold reset (e.g. POR, watchdog reset etc.) or
                 *  a wakeup from deep sleep.
                 *
                 *  If CLK_SNC_CTRL_REG.SNC_STATE_RETAINED is set, it is a wakeup from deep
                 *  sleep without state retention -- the bit is set by goto_deepsleep().
                 *  In that case, the Reset_Handler jumps to wakeup_from_deepsleep,
                 *  instead of going through the normal startup procedure.
                 */
                ldr     r1, =CLK_SNC_CTRL_REG
                ldr     r2, [r1]
                lsls    r0, r2, 31 - CLK_SNC_CTRL_REG_SNC_STATE_RETAINED_BIT
                bpl     .cold_reset

                ldr     r3, =wakeup_from_deepsleep
                bx      r3
.no_valid_wakeup:
                ldr     r3, =unintended_wakeup
                blx     r3
                b       .start_over

.cold_reset:
#if (dg_configENABLE_MTB)
                ldr     r0, =MTB_POSITION_REG
                ldr     r1, =0
                str     r1, [r0]
                ldr     r0, =MTB_FLOW_REG
                ldr     r1, =0
                str     r1, [r0]
                ldr     r0, =MTB_MASTER_REG
                ldr     r1, =MTB_MASTER_REG_ENABLE_MSK
                str     r1, [r0]
#endif

                bl      SystemInitPre

                ldr      r4, =__copy_table_start__
                ldr      r5, =__copy_table_end__

.L_loop0:
                cmp      r4, r5
                bge      .L_loop0_done
                ldr      r1, [r4]
                ldr      r2, [r4, #4]
                ldr      r3, [r4, #8]

.L_loop0_0:
                subs     r3, #4
                blt      .L_loop0_0_done
                ldr      r0, [r1, r3]
                str      r0, [r2, r3]
                b        .L_loop0_0

.L_loop0_0_done:
                adds     r4, #12
                b        .L_loop0

.L_loop0_done:

                ldr      r3, =__zero_table_start__
                ldr      r4, =__zero_table_end__

.L_loop2:
                cmp      r3, r4
                bge      .L_loop2_done
                ldr      r1, [r3]
                ldr      r2, [r3, #4]
                movs     r0, 0

.L_loop2_0:
                subs     r2, #4
                blt      .L_loop2_0_done
                str      r0, [r1, r2]
                b        .L_loop2_0
.L_loop2_0_done:

                adds     r3, #8
                b        .L_loop2
.L_loop2_done:

                bl      SystemInit

                bl      _start

                .pool
                .size   Reset_Handler, . - Reset_Handler


                .text
                .thumb
                .thumb_func
                .align  2
                .globl  NMI_Handler
                .type   NMI_Handler, %function
NMI_Handler:
                movs    r0, #4
                mov     r1, lr
                tst     r0, r1
                beq     NMI_stacking_used_MSP
                mrs     r0, psp
                ldr     r1, =NMI_HandlerC
                bx      r1
NMI_stacking_used_MSP:
                mrs     r0, msp
                ldr     r1, =NMI_HandlerC
                bx      r1

                .pool
                .size   NMI_Handler, . - NMI_Handler

                .text
                .thumb
                .thumb_func
                .align  2
                .globl  HardFault_Handler
                .type   HardFault_Handler, %function
HardFault_Handler:
                movs    r0, #4
                mov     r1, lr
                tst     r0, r1
                beq     HardFault_stacking_used_MSP
                mrs     r0, psp
                ldr     r1, =HardFault_HandlerC
                bx      r1
HardFault_stacking_used_MSP:
                mrs     r0, msp
                ldr     r1, =HardFault_HandlerC
                bx      r1

                .pool
                .size   HardFault_Handler, . - HardFault_Handler

                .end

#endif /* SNC_PROCESSOR_BUILD */
