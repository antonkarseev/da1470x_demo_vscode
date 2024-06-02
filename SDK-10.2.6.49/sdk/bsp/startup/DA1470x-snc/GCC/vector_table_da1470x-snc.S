/**
 ****************************************************************************************
 *
 * @file vector_table_da1470x-snc.S
 *
 * @brief Interrupt Vector Table for DA1470x SNC
 *
 * Copyright (C) 2015-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (SNC_PROCESSOR_BUILD)

        .syntax unified
        .arch   armv6-m


#if (dg_configSYSTEMVIEW == 0)
        #ifdef __STACK_SIZE
                .equ    Stack_Size, __STACK_SIZE
        #else
                .equ    Stack_Size, 0xC00
        #endif
#else
        #ifdef __STACK_SIZE
                .equ    Stack_Size, (__STACK_SIZE + dg_configSYSTEMVIEW_STACK_OVERHEAD)
        #else
                .equ    Stack_Size, (0xC00 + dg_configSYSTEMVIEW_STACK_OVERHEAD)
        #endif
#endif /* (dg_configSYSTEMVIEW == 0) */
        .section .stack
        .align  3
        .globl  __StackTop
        .globl  __StackLimit
__StackLimit:
        .space  Stack_Size
        .size   __StackLimit, . - __StackLimit
__StackTop:
        .size   __StackTop, . - __StackTop

#ifdef __HEAP_SIZE
        .equ    Heap_Size, __HEAP_SIZE
#else
        .equ    Heap_Size, 0
#endif

        .if      Heap_Size != 0                     /* Heap is provided */
        .section .heap
        .align  3
        .globl  __HeapBase
        .globl  __HeapLimit
__HeapBase:
        .space  Heap_Size
        .size   __HeapBase, . - __HeapBase
__HeapLimit:
        .size   __HeapLimit, . - __HeapLimit
        .endif

        .section .isr_vector, "a"
        .align 2
        .globl  __isr_vector
        .globl  __isr_vector_end
        .globl  __isr_vector_size
__isr_vector:
        .long   __StackTop              /* Top of Stack */
        .long   Reset_Handler           /* Reset Handler */
        .long   NMI_Handler             /* NMI Handler */
        .long   HardFault_Handler       /* Hard Fault Handler */
        .long   0                       /* Reserved */
        .long   0                       /* Reserved */
        .long   0                       /* Reserved */
        .long   0                       /* Reserved */
        .long   0                       /* Reserved */
        .long   0                       /* Reserved */
        .long   0                       /* Reserved */
        .long   SVC_Handler             /* SVCall Handler */
        .long   0                       /* Reserved */
        .long   0                       /* Reserved */
        .long   PendSV_Handler          /* PendSV Handler */
        .long   SysTick_Handler         /* SysTick Handler */

        /* External interrupts */
        .long   SYS2SNC_Handler         /*  0:                  */
        .long   CMAC2SNC_Handler        /*  1:                  */
        .long   PDC_SNC_Handler         /*  2:                  */
        .long   Key_Wkup_GPIO_Handler   /*  3:                  */
        .long   GPIO_P0_Handler         /*  4:                  */
        .long   GPIO_P1_Handler         /*  5:                  */
        .long   GPIO_P2_Handler         /*  6:                  */
        .long   Timer_Handler           /*  7:                  */
        .long   Timer3_Handler          /*  8:                  */
        .long   Timer4_Handler          /*  9:                  */
        .long   Timer5_Handler          /* 10:                  */
        .long   Timer6_Handler          /* 11:                  */
        .long   RTC_Handler             /* 12:                  */
        .long   RTC_Event_Handler       /* 13:                  */
        .long   CAPTIMER_Handler        /* 14:                  */
        .long   GPADC_Handler           /* 15:                  */
        .long   UART_Handler            /* 16:                  */
        .long   UART2_Handler           /* 17:                  */
        .long   UART3_Handler           /* 18:                  */
        .long   SPI_Handler             /* 19:                  */
        .long   SPI2_Handler            /* 20:                  */
        .long   SPI3_Handler            /* 21:                  */
        .long   I2C_Handler             /* 22:                  */
        .long   I2C2_Handler            /* 23:                  */
        .long   I2C3_Handler            /* 24:                  */
        .long   I3C_Handler             /* 25:                  */
        .long   PCM_Handler             /* 26:                  */
        .long   SRC_IN_Handler          /* 27:                  */
        .long   SRC_OUT_Handler         /* 28:                  */
        .long   SRC2_IN_Handler         /* 29:                  */
        .long   SRC2_OUT_Handler        /* 30:                  */
        .long   VAD_Handler             /* 31:                  */

__isr_vector_end:
        .equ     __isr_vector_size, __isr_vector - __isr_vector
        .size   __isr_vector, . - __isr_vector

#endif /* SNC_PROCESSOR_BUILD */
