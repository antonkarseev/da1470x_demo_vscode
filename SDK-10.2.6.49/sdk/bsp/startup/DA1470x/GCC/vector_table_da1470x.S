/**
 ****************************************************************************************
 *
 * @file vector_table.S
 *
 * @brief Interrupt Vector Table
 *
 * Copyright (C) 2015-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if (MAIN_PROCESSOR_BUILD)

        .syntax unified
        .arch   armv8-m.main


/*
;<h> Stack Configuration
;  <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
;</h>
*/
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


/*
;<h> Heap Configuration
;  <o> Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
;</h>
*/
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
        .long   MemManage_Handler       /* MPU Fault Handler */
        .long   BusFault_Handler        /* Bus Fault Handler */
        .long   UsageFault_Handler      /* Usage Fault Handler */
        .long   SecureFault_Handler     /* Secure Fault Handler */
        .long   0                       /* Reserved */
        .long   0                       /* Reserved */
        .long   0                       /* Reserved */
        .long   SVC_Handler             /* SVCall Handler */
        .long   DebugMon_Handler        /* Debug Monitor Handler */
        .long   0                       /* Reserved */
        .long   PendSV_Handler          /* PendSV Handler */
        .long   SysTick_Handler         /* SysTick Handler */

        /* External interrupts */
        .long   CMAC2SYS_Handler                /*  0: CMAC and mailbox                */
        .long   SNC2SYS_Handler                 /*  1: SNC M0+ interrupt to Cortex M33 */
        .long   MRM_Handler                     /*  2: Cache Miss Rate Monitor         */
        .long   PDC_Handler                     /*  3: Wakeup from PDC                 */
        .long   Key_Wkup_GPIO_Handler           /*  4: Debounced button pressed        */
        .long   VBUS_Handler                    /*  5: VBUS Presence                   */
        .long   Charger_State_Handler           /*  6: Charger FSM interrupt request   */
        .long   Charger_Error_Handler           /*  7: Charger Error interrupt request */
        .long   DCDC_Boost_Handler              /*  8: DCDC boost controller           */
        .long   PLL48_Lock_Handler              /*  9: PLL48 locked                    */
        .long   Crypto_Handler                  /* 10: Crypto (AES or HASH)            */
        .long   PLL_Lock_Handler                /* 11: PLL locked                      */
        .long   XTAL32M_Ready_Handler           /* 12: XTAL32M osc trimmed & settled   */
        .long   RFDIAG_Handler                  /* 13: Baseband/Radio diagnostics      */
        .long   GPIO_P0_Handler                 /* 14: GPIO port 0                     */
        .long   GPIO_P1_Handler                 /* 15: GPIO port 1                     */
        .long   GPIO_P2_Handler                 /* 16: GPIO port 2                     */
        .long   Timer_Handler                   /* 17: Timer                           */
        .long   Timer2_Handler                  /* 18: Timer2                          */
        .long   Timer3_Handler                  /* 19: Timer3                          */
        .long   Timer4_Handler                  /* 20: Timer4                          */
        .long   Timer5_Handler                  /* 21: Timer5                          */
        .long   Timer6_Handler                  /* 22: Timer6                          */
        .long   RTC_Handler                     /* 23: RTC                             */
        .long   RTC_Event_Handler               /* 24: RTC event                       */
        .long   CAPTIMER_Handler                /* 25: GPIO triggered Timer Capture    */
        .long   GPADC_Handler                   /* 26: General Purpose ADC             */
        .long   APADC_Handler                   /* 27: Application ADC                 */
        .long   DMA_Handler                     /* 28: General Purpose DMA             */
        .long   UART_Handler                    /* 29: UART                            */
        .long   UART2_Handler                   /* 30: UART2                           */
        .long   UART3_Handler                   /* 31: UART3                           */
        .long   SPI_Handler                     /* 32: SPI                             */
        .long   SPI2_Handler                    /* 33: SPI2                            */
        .long   SPI3_Handler                    /* 34: SPI3                            */
        .long   I2C_Handler                     /* 35: I2C                             */
        .long   I2C2_Handler                    /* 36: I2C2                            */
        .long   I2C3_Handler                    /* 37: I2C3                            */
        .long   I3C_Handler                     /* 38: I3C                             */
        .long   USB_Handler                     /* 39: USB                             */
        .long   PCM_Handler                     /* 40: PCM                             */
        .long   SRC_IN_Handler                  /* 41: SRC input                       */
        .long   SRC_OUT_Handler                 /* 42: SRC output                      */
        .long   SRC2_IN_Handler                 /* 43: SRC2 input                      */
        .long   SRC2_OUT_Handler                /* 44: SRC output                      */
        .long   VAD_Handler                     /* 45: VAD                             */
        .long   EMMC_Handler                    /* 46: eMMC Controller                 */
        .long   RESERVED47_Handler              /* 47: Software interrupt              */
        .long   GPU_Handler                     /* 48: GPU request to Cortex M33       */
        .long   LCD_Handler                     /* 49: LCD Controller                  */
        .long   RESERVED50_Handler              /* 50: Software interrupt              */
        .long   CHARGER_DET_Handler             /* 51: Charger detection               */
        .long   DCACHE_MRM_Handler              /* 52: Data cache MRM                  */
        .long   CLK_CALIBRATION_Handler         /* 53: Clock calibration               */
        .long   VSYS_GEN_Handler                /* 54: VSYS Generator                  */
        .long   RESERVED55_Handler              /* 55: Software interrupt              */

__isr_vector_end:
        .equ     __isr_vector_size, __isr_vector - __isr_vector
        .size   __isr_vector, . - __isr_vector

#endif /* MAIN_PROCESSOR_BUILD */
