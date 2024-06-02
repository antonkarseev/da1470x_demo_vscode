/*
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

#if (SNC_PROCESSOR_BUILD)
        .syntax unified
        .arch   armv6-m


        .equ PDC_ACKNOWLEDGE_REG,           0x50000280
        .equ PDC_PENDING_SNC_REG,           0x50000288
        .equ PDC_MAX_ENTRIES,               16
        .equ PDC_MAX_ENTRIES_IDX,           (PDC_MAX_ENTRIES - 1)

        .equ CLK_SNC_CTRL_REG,              0x5000002C

        .equ SCB_BASE,                      0xE000ED00

#if dg_configENABLE_MTB
        .equ MTB_POSITION_REG,              0xF0002000
        .equ MTB_MASTER_REG,                0xF0002004
        .equ MTB_FLOW_REG,                  0xF0002008
#endif


#ifndef DEEPSLEEP_KEEP_COUNTER
#define DEEPSLEEP_KEEP_COUNTER  (1)
#endif


/*
 *  The processor state is stored here before going to deep sleep,
 *  so that it is restored after wakeup.
 */
        .section .bss.retained_state
        .align 3
        .globl  __retained_state
#if DEEPSLEEP_KEEP_COUNTER
        .globl deepsleep_counter
        .type deepsleep_counter, %object
#endif
        /* reserve space for stacking, in case an exception occurs while restoring the state */
        .space  64
__retained_state:
.stored_SP:
        .space  4
.stored_R1:
        .space  4
.stored_R2:
        .space  4
.stored_R3:
        .space  4
.return_PC:
        .space  4
.stored_PRIMASK:
        .space  4
.stored_PSR:
        .space  4
.stored_CONTROL:
        .space  4
.stored_R4:
        .space  4
.stored_R5:
        .space  4
.stored_R6:
        .space  4
.stored_R7:
        .space  4
.stored_R8:
        .space  4
.stored_R9:
        .space  4
.stored_R10:
        .space  4
.stored_R11:
        .space  4
.stored_R12:
        .space  4
.stored_R14:
        .space  4
.stored_ISER:
        .space  4
.stored_PRIORITY0:
        .space  4
.stored_PRIORITY1:
        .space  4
.stored_PRIORITY2:
        .space  4
.stored_PRIORITY3:
        .space  4
.stored_SCR:
        .space  4
.stored_SHPR2:
        .space  4
.stored_SHPR3:
        .space  4

#if dg_configENABLE_MTB
.stored_MTB_POSITION:   .space  4
.stored_MTB_MASTER:     .space  4
.stored_MTB_FLOW:       .space  4
#endif

#if DEEPSLEEP_KEEP_COUNTER
deepsleep_counter:
        .space  4
#endif

.system_slept:
        .space  4 /* 1 sleep occurred, 0 not slept, any other value is an error */


        .size   __retained_state, . - __retained_state


/*
 *  This function stores the state of the processor and uses WFI to go
 *  to deep sleep, which does not retain the processor state.
 *  Just before executing WFI it sets CLK_SNC_CTRL_REG.SNC_STATE_RETAINED
 *  so that the Reset_Handler detects that the reset is due to waking
 *  up from deep sleep and call wakeup_from_deepsleep(), to restore the
 *  processor state.
 *  The processor state is stored @__retained_state.
 */
        .text
        .thumb
        .thumb_func
        .align  1
        .globl  goto_deepsleep
        .type   goto_deepsleep, %function
goto_deepsleep:
        /* push r1-r3; they will be used to read and store special regs */
        push    {r1-r3}

        /* store PRIMASK, PSR, CONTROL and disable interrupts */
        mrs     r0, PRIMASK
        cpsid   i
        mrs     r1, PSR
        mrs     r2, CONTROL
        ldr     r3, =.stored_PRIMASK
        stm     r3!, {r0-r2}

        /*
         * Acknowledge all the pending PDC entries towards the SNC
         * NOTE: only r0, r1, r2 are touched in order to do this
         */
        ldr     r0, =PDC_PENDING_SNC_REG
        ldr     r1, [r0]                    // r1 = [PDC_PENDING_SNC_REG]

        movs    r0, #0                      // loop init: r0 = i = 0
        b.n     .L_loop_cond
.L_loop_next:
        adds    r0, #1                      // i = i + 1
.L_loop_cond:
        cmp     r0, PDC_MAX_ENTRIES_IDX
        bgt.n   .L_loop_exit                // if i > PDC_MAX_ENTRIES_IDX then exit loop

        movs    r2, #1
        lsls    r2, r0                      // r2 = 1 << i
        tst     r1, r2                      // check if bit at pos i is set
        beq.n   .L_loop_next                // if not then continue loop
                                            // else
        ldr     r2, =PDC_ACKNOWLEDGE_REG    // r2 = [PDC_ACKNOWLEDGE_REG]
        str     r0, [r2]                    // [PDC_ACKNOWLEDGE_REG] = i

        b.n     .L_loop_next                // continue loop
.L_loop_exit:

        /* store R4-R7 */
        stm     r3!, {r4-r7}

        /* store R8-R12, R14 */
        mov     r0, r8
        mov     r1, r9
        mov     r2, r10
        mov     r4, r11
        mov     r5, r12
        mov     r6, r14
        stm     r3!, {r0-r2, r4-r6}

        mov     r8, r3
        /* get value of NVIC->ISER */
        ldr     r7, =0xE000E100
        ldr     r0, [r7]
        /* get values of NVIC->PRIORITY{0,1,2,3} */
        ldr     r7, =0xE000E400
        ldr     r1, [r7, #0]
        ldr     r2, [r7, #4]
        ldr     r3, [r7, #8]
        ldr     r4, [r7, #12]

        /*
         * store values of NVIC->ISER, NVIC->PRIORITY{0,1,2,3},
         */
        mov     r7, r8
        stm     r7!, {r0-r4}
        mov     r8, r7

        /* get values of SCB->SCR, SCB->SHR[0] (SHPR2) and SCB->SHR[1] (SHPR3) */
        ldr     r7, =SCB_BASE
        ldr     r4, [r7, #0x10]
        ldr     r5, [r7, #0x1C]
        ldr     r6, [r7, #0x20]
        mov     r9, r7
        mov     r10, r4

        /*
         * store values of SCB->SCR, SCB->SHR[0] (SHPR2) and SCB->SHR[1] (SHPR3)
         */
        mov     r7, r8
        stm     r7!, {r4-r6}

#if dg_configENABLE_MTB
        /* Store MTB registers (POSITION, MASTER and FLOW) */
        ldr     r0, =#MTB_POSITION_REG          // load MTB_POSITION_REG address
        ldm     r0!, {r4-r6}                    // value of MTB_POSITION_REG MTB_MASTER_REG, MTB_FLOW_REG
        stm     r7!, {r4-r6}
#endif

#if DEEPSLEEP_KEEP_COUNTER
        /* increment deepsleep_counter */
        ldr     r0, [r7]
        adds    r0, #1
        str     r0, [r7]
#endif

        /* store SP (we assume MSP), R1-R3 and LR (return PC) */
        ldr     r7, =.stored_SP
        pop     {r1-r3}
        mov     r0, sp
        mov     r4, lr
        stm     r7!, {r0-r4}

        /*
         *  Set CLK_SNC_CTRL_REG.SNC_STATE_RETAINED.
         */
        ldr     r1, =CLK_SNC_CTRL_REG
        ldr     r2, [r1]
        movs    r0, #4
        orrs    r2, r0
        str     r2, [r1]

        /*
         * Set SCB->SCR.SLEEPDEEP
         * Its previous value will be restored when we restore SCB->SCR:
         *  - right after wfi below (i.e. when deep sleep doesn't go through)
         *  - in wakeup_from_deepsleep (i.e. when deep sleep happens)
         */
        mov     r7, r9
        mov     r1, r10
        movs    r0, #4
        orrs    r1, r0
        str     r1, [r7, #0x10]

        /*
         * set the sleep flag to "slept"
         */
        ldr     r7, =.system_slept
        movs    r0, #1
        str     r0, [r7]

#if dg_configENABLE_SNC_SLEEP_STATUS
        /*
         * set sleep status to sleep via goto_deepsleep
         */
        ldr     r7, =__SNC_sleep_status
        movs    r0, #SNC_SLEPT_GOTO_DEEPSLEEP
        str     r0, [r7]
#endif
        /*
         * Attempt to go to deep sleep; if this actually happens, we will
         * wake up in Reset_Handler, which will detect what happened and
         * jump to wakeup_from_deepsleep.
         */
        wfi

#if dg_configENABLE_SNC_SLEEP_STATUS
        /*
         * set sleep status to awake via failure in goto_deepsleep
         */
        ldr     r7, =__SNC_sleep_status
        movs    r0, #SNC_ACTIVE_AFTER_DEEPSLEEP
        str     r0, [r7]
#endif

        /* WE REACH THIS POINT IF DEEP SLEEP DIDN'T ACTUALLY HAPPEN */

        /*
         * clear NVIC->ISER so that it can be properly restored in
         * wakeup_from_deepsleep
         */
        ldr     r7, =0xE000E100
        adds    r7, r7, #0x80
        str     r1, [r7]

        /* restore SCB->SCR (for SLEEPDEEP) */
        mov     r7, r9
        mov     r2, r10
        str     r2, [r7, #0x10]

        /*
         * set the sleep flag to "not slept"
         */
        ldr     r7, =.system_slept
        movs    r0, #0
        str     r0, [r7]

        /* load r1 and r2 as expected by wakeup_from_deepsleep */
        ldr     r1, =CLK_SNC_CTRL_REG
        ldr     r2, [r1]
        b       wakeup_from_deepsleep

        .pool
        .size   goto_deepsleep, . - goto_deepsleep
/*
 *  This function runs when PD_SNC is powered up by other masters and not PDC.
 *  In this case PD_SNC is powered ON but there is not any pending PDC SNC entry.
 *  After execution of this function SNC in set back to deep sleep in order to reduce power consumption.
 *  SNC retained state CLK_SNC_CTRL_REG.SNC_STATE_RETAINED is not changed so in case SNC is triggered by PDC,
 *  wakeup_from_deepsleep() is called and SNC is able to continue the execution of application code.
 */
        .text
        .thumb
        .thumb_func
        .align  1
        .globl  unintended_wakeup
        .type   unintended_wakeup, %function
unintended_wakeup:

        /* save PRIMASK */
        mrs     r0, PRIMASK

        /* disable interrupts */
        cpsid   i

        /* get value of NVIC->ISER */
        ldr     r1, =0xE000E100
        ldr     r2, [r1]
        cmp     r2, #0
        /* check for active IRQ in NVIC. If there is none then SNC was
         * never started up or PD_SNC was powered OFF and is powered ON
         * without pending SNC PDC entry. Enable PDC_SNC_Irq so SNC to
         * be able to start executing application code in case PD_SNC remains ON
         * and SNC PDC will be pending in the future.
         */
        bne     .Skip_PDC_irq
        movs    r2, #4
        str     r2, [r1]

.Skip_PDC_irq:

        /*
         * Set SCB->SCR.SLEEPDEEP, is lost after powering off PD_SNC
         * Its previous value will be restored when we restore SCB->SCR
         * in wakeup_from_deepsleep (i.e. when deep sleep happens)
         */
        ldr     r1, =SCB_BASE
        ldr     r3, [r1, #0x010]
        movs    r2, #4
        orrs    r2, r3
        str     r2, [r1, #0x10]

#if dg_configENABLE_SNC_SLEEP_STATUS
        /*
         * set sleep status to sleep via unintended_wakeup
         */
        ldr     r1, =__SNC_sleep_status
        movs    r2, #SNC_SLEPT_UNINTENDED_WKUP
        str     r2, [r1]
#endif
        /*
         * Attempt to go to deep sleep; if this actually happens, we will
         * wake up in Reset_Handler, which will detect what happened and
         * jump to wakeup_from_deepsleep.
         */
        wfi

        /* We have reached this point if PD_SNC was not powered off and an interrupt triggered SNC to escape from WFI */

        /* disable PDC_SNC_Irq in NVIC->ICER */
        ldr     r1, =0xE000E180
        movs    r2, #4
        str     r2, [r1]
        /* clear pending PDC_SNC_Irq in NVIC->ICPR */
        ldr     r1, =0xE000E280
        str     r2, [r1]

        /* restore PRIMASK */
        msr     PRIMASK, r0
        /* enable interrupts */
        cpsie   i

        bx      lr
        .pool
        .size   unintended_wakeup, . - unintended_wakeup
/*
 *  The Reset_Handler jumps to this subroutine if it detects that we
 *  are waking up from deep sleep.
 *  It is expected to receive the following input:
 *   - r1: the address of CLK_SNC_CTRL_REG.SNC_STATE_RETAINED (0x5000002C)
 *   - r2: the value of CLK_SNC_CTRL_REG
 *
 *  The subroutune will clear CLK_SNC_CTRL_REG.SNC_STATE_RETAINED and
 *  restore the saved processor state (stored @__retained_state).
 */
        .text
        .thumb
        .thumb_func
        .align  1
        .globl  wakeup_from_deepsleep
        .type   wakeup_from_deepsleep, %function
wakeup_from_deepsleep:
        /* disable interrupts, final state will be set when PRIMASK is restored */
        cpsid   i

        /* set SP to a safe value, we have reserved some space for this case */
        ldr     r0, =__retained_state
        mov     sp, r0

        /* clear CLK_SNC_CTRL_REG.SNC_STATE_RETAINED */
        movs    r0, #4
        bics    r2, r0
        str     r2, [r1]

#if dg_configENABLE_SNC_SLEEP_STATUS
        /*
         * set sleep status to awake from wakeup_from_deepsleep
         */
        ldr     r7, =__SNC_sleep_status
        movs    r0, #SNC_ACTIVE_WAKUP_FROM_DEEPSLEEP
        str     r0, [r7]
#endif
        /*
         * restore values of NVIC->ISER, NVIC->PRIORITY{0,1,2},
         *                   SCB->SCR, SCB->SHR[0] (SHPR2) and SCB->SHR[1] (SHPR3)
         */
        ldr     r7, =.stored_ISER
        ldm     r7!, {r0-r4}
        /* set value of NVIC->ISER */
        ldr     r7, =0xE000E100
        str     r0, [r7]
        /* set values of NVIC->PRIORITY{0,1,2,3} */
        ldr     r7, =0xE000E400
        str     r1, [r7, #0]
        str     r2, [r7, #4]
        str     r3, [r7, #8]
        str     r4, [r7, #12]

        ldm     r7!, {r4-r6}
        /* set values of SCB->SCR, SCB->SHR[0] (SHPR2) and SCB->SHR[1] (SHPR3) */
        ldr     r7, =0xE000ED00
        str     r4, [r7, #0x10]
        str     r5, [r7, #0x1C]
        str     r6, [r7, #0x20]

#if dg_configENABLE_MTB
        /* restore MTB */
        ldm     r7!, {r4-r6}
        ldr     r2, =#MTB_MASTER_REG
        str     r5, [r2]
        ldr     r2, =#MTB_POSITION_REG
        str     r4, [r2]
        ldr     r2, =#MTB_FLOW_REG
        str     r6, [r2]
#endif

        /* restore R8-R12, R14 */
        ldr     r0, =.stored_R8
        ldm     r0!, {r1-r6}
        mov     r8, r1
        mov     r9, r2
        mov     r10, r3
        mov     r11, r4
        mov     r12, r5
        mov     r14, r6

        /* restore CONTROL and R4-R7 */
        ldr     r3, =.stored_CONTROL
        ldm     r3!, {r2, r4-r7}

        /* restore SP and create a return stack frame */
        ldr     r0, =.stored_SP
        ldr     r1, [r0]
        msr     CONTROL, r2
        mov     sp, r1
        /*
         *  Create a stack frame in the return stack:
         *                  +--------------------+
         *  (high address)  | restored PC        |
         *                  +--------------------+
         *  (low  address)  | R0 (=system_slept) |
         *                  +--------------------+
         *  This will be popped at the end of the restore procedure, to return
         *  to the caller of goto_deepsleep().
         */
        ldr     r1, =.system_slept
        ldr     r1, [r1]
        ldr     r2, [r0, #(.return_PC - .stored_SP)]
        push    {r1-r2}

        /* restore R2-R3 */
        ldr     r2, [r0, #(.stored_R2 - .stored_SP)]
        ldr     r3, [r0, #(.stored_R3 - .stored_SP)]

        /* restore PSR, R1 and PRIMASK */
        ldr     r1, [r0, #(.stored_PSR - .stored_SP)]
        msr     PSR_nzcvq, r1
        ldr     r1, [r0, #(.stored_R1 - .stored_SP)]
        ldr     r0, [r0, #(.stored_PRIMASK - .stored_SP)]
        msr     PRIMASK, r0
        /* interrupts might be enabled now, are we vulnerable? */

        pop     {r0, pc}

        .pool
        .size   wakeup_from_deepsleep, . - wakeup_from_deepsleep

#endif /* SNC_PROCESSOR_BUILD */
