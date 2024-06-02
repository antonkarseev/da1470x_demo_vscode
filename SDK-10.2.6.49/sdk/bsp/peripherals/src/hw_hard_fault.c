/**
 ****************************************************************************************
 *
 * @file hw_hard_fault.c
 *
 * @brief HardFault handler.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include "sdk_defs.h"
#include "hw_hard_fault.h"
#include "hw_watchdog.h"
#include "hw_cpm.h"
#include "hw_sys.h"

#if (SNC_PROCESSOR_BUILD)
#include "snc.h"
#endif /* SNC_PROCESSOR_BUILD */

#if MAIN_PROCESSOR_BUILD
#define MTB_MASTER_REG                  ((uint32_t *) (0xE0043004))
#define MTB_MASTER_DISABLE_MSK          (0x00000008)
#elif SNC_PROCESSOR_BUILD
#define MTB_MASTER_REG                  ((uint32_t *) (0xF0002000))
#define MTB_MASTER_DISABLE_MSK          (0x00000008)
#endif /* PROCESSOR_BUILD */



/*
 * Global variables
 */
volatile uint32_t hardfault_event_data[9] __attribute__((section("hard_fault_info")));

/*
 * Local variables
 */


/*
 * This is the base address in Retention RAM where the stacked information will be copied.
 */
#if MAIN_PROCESSOR_BUILD
#define STATUS_BASE (0xF000200)
#elif SNC_PROCESSOR_BUILD
#define STATUS_BASE (0x00045600)
#endif /* PROCESSOR_BUILD */

/*
 * Compilation switch to enable verbose output on HardFault
 */
#ifndef VERBOSE_HARDFAULT
#       define VERBOSE_HARDFAULT        0
#endif

/*
 * Function definitions
 */

/**
* \brief HardFault handler implementation. During development it will copy the system's status
*        to a predefined location in memory. In release mode, it will cause a system reset.
*
* \param [in] hardfault_args The system's status when the HardFault event occurred.
*
* \return void
*
*/
__RETAINED_CODE void HardFault_HandlerC(unsigned long *hardfault_args)
{
#if defined(CONFIG_SEMIHOSTING)
        /* If a BKPT is executed during semihosting and no debugger is attached, a hard fault will
         * be caused. This case is handled here by exiting the handler after moving PC to the next
         * command */
        if ((SCB->HFSR & SCB_HFSR_DEBUGEVT_Msk)) {
                if (*(volatile uint16_t*)hardfault_args[6] == (uint16_t)(0xBEAB)) {
                        SCB->HFSR = SCB_HFSR_DEBUGEVT_Msk;      // Reset Hard Fault status
                        hardfault_args[6] += 2;                 // Increment PC by 2 to skip break instruction.
                        return;                                 // Return to interrupted application
                }
        }
#endif

#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_DISABLE_MSK;
#endif /* dg_configENABLE_MTB */

        // Stack frame contains:
        // r0, r1, r2, r3, r12, r14, the return address and xPSR
        // - Stacked R0 = hf_args[0]
        // - Stacked R1 = hf_args[1]
        // - Stacked R2 = hf_args[2]
        // - Stacked R3 = hf_args[3]
        // - Stacked R12 = hf_args[4]
        // - Stacked LR = hf_args[5]
        // - Stacked PC = hf_args[6]
        // - Stacked xPSR= hf_args[7]
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
#if (MAIN_PROCESSOR_BUILD)
                hw_watchdog_freeze();                           // Stop WDOG

                ENABLE_DEBUGGER;
#elif (SNC_PROCESSOR_BUILD)
                hw_watchdog_set_pos_val(REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_CNT));      // Reset WDOG

                ENABLE_SNC_DEBUGGER;
#endif /* PROCESSOR_BUILD */

                *(volatile unsigned long *)(STATUS_BASE       ) = hardfault_args[0];    // R0
                *(volatile unsigned long *)(STATUS_BASE + 0x04) = hardfault_args[1];    // R1
                *(volatile unsigned long *)(STATUS_BASE + 0x08) = hardfault_args[2];    // R2
                *(volatile unsigned long *)(STATUS_BASE + 0x0C) = hardfault_args[3];    // R3
                *(volatile unsigned long *)(STATUS_BASE + 0x10) = hardfault_args[4];    // R12
                *(volatile unsigned long *)(STATUS_BASE + 0x14) = hardfault_args[5];    // LR
                *(volatile unsigned long *)(STATUS_BASE + 0x18) = hardfault_args[6];    // PC
                *(volatile unsigned long *)(STATUS_BASE + 0x1C) = hardfault_args[7];    // PSR
                *(volatile unsigned long *)(STATUS_BASE + 0x20) = (unsigned long)hardfault_args;    // Stack Pointer

#if (MAIN_PROCESSOR_BUILD)
                *(volatile unsigned long *)(STATUS_BASE + 0x24) = (*((volatile unsigned long *)(0xE000ED28)));    // CFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x28) = (*((volatile unsigned long *)(0xE000ED2C)));    // HFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x2C) = (*((volatile unsigned long *)(0xE000ED30)));    // DFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x30) = (*((volatile unsigned long *)(0xE000ED3C)));    // AFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x34) = (*((volatile unsigned long *)(0xE000ED34)));    // MMAR
                *(volatile unsigned long *)(STATUS_BASE + 0x38) = (*((volatile unsigned long *)(0xE000ED38)));    // BFAR
#endif /* MAIN_PROCESSOR_BUILD */

                if (VERBOSE_HARDFAULT) {
                        printf("HardFault Handler:\r\n");
                        printf("- R0  = 0x%08lx\r\n", hardfault_args[0]);
                        printf("- R1  = 0x%08lx\r\n", hardfault_args[1]);
                        printf("- R2  = 0x%08lx\r\n", hardfault_args[2]);
                        printf("- R3  = 0x%08lx\r\n", hardfault_args[3]);
                        printf("- R12 = 0x%08lx\r\n", hardfault_args[4]);
                        printf("- LR  = 0x%08lx\r\n", hardfault_args[5]);
                        printf("- PC  = 0x%08lx\r\n", hardfault_args[6]);
                        printf("- xPSR= 0x%08lx\r\n", hardfault_args[7]);
                }

                if (EXCEPTION_DEBUG == 1) {
                        hw_sys_assert_trigger_gpio();
                }

#if (MAIN_PROCESSOR_BUILD)
                while (1);
#elif (SNC_PROCESSOR_BUILD)
                snc_set_shared_space_addr((void *)STATUS_BASE, SNC_SHARED_SPACE_EXCEPTION_HF);

                /* Notify SYSCPU to indicate SNC error. */
                snc_signal_error(SNC_ERROR_HF, (void *)STATUS_BASE);

                FREEZE_SNC_WATCHDOG_WHILE(1);
#endif /* PROCESSOR_BUILD */
        }
        else {
# ifdef PRODUCTION_DEBUG_OUTPUT
# if (USE_WDOG)
                WDOG->WATCHDOG_REG = 0xC8;                      // Reset WDOG! 200 * 10.24ms active time for UART to finish printing!
# endif

                dbg_prod_output(1, hardfault_args);
# endif // PRODUCTION_DEBUG_OUTPUT

                hardfault_event_data[0] = HARDFAULT_MAGIC_NUMBER;
                hardfault_event_data[1] = hardfault_args[0];    // R0
                hardfault_event_data[2] = hardfault_args[1];    // R1
                hardfault_event_data[3] = hardfault_args[2];    // R2
                hardfault_event_data[4] = hardfault_args[3];    // R3
                hardfault_event_data[5] = hardfault_args[4];    // R12
                hardfault_event_data[6] = hardfault_args[5];    // LR
                hardfault_event_data[7] = hardfault_args[6];    // PC
                hardfault_event_data[8] = hardfault_args[7];    // PSR

#if (MAIN_PROCESSOR_BUILD)
                hw_cpm_reboot_system();                         // Force reset

                while (1);
#elif (SNC_PROCESSOR_BUILD)
                snc_set_shared_space_addr((void *)hardfault_event_data, SNC_SHARED_SPACE_EXCEPTION_HF);

                /* Notify SYSCPU to indicate SNC error. */
                snc_signal_error(SNC_ERROR_HF, (void *)hardfault_event_data);

                FREEZE_SNC_WATCHDOG_WHILE(1);
#endif /* PROCESSOR_BUILD */
        }
}

#if (MAIN_PROCESSOR_BUILD)

__RETAINED_CODE void MemManage_Handler(void)
{
        volatile uint8_t mem_fault_status_reg;
        volatile uint32_t mem_fault_addr __UNUSED;

#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_DISABLE_MSK;
#endif
        mem_fault_status_reg = (SCB->CFSR & SCB_CFSR_MEMFAULTSR_Msk) >> SCB_CFSR_MEMFAULTSR_Pos;
        if (mem_fault_status_reg & 0x80) {
                mem_fault_addr = SCB->MMFAR;
        }
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                        hw_watchdog_freeze();                           // Stop WDOG
        }
        while (1) {}
}

__RETAINED_CODE void BusFault_Handler(void)
{
        volatile uint8_t bus_fault_status_reg;
        volatile uint32_t bus_fault_addr __UNUSED;

#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_DISABLE_MSK;
#endif

        bus_fault_status_reg = (SCB->CFSR & SCB_CFSR_BUSFAULTSR_Msk) >> SCB_CFSR_BUSFAULTSR_Pos;
        if (bus_fault_status_reg & 0x80) {
                bus_fault_addr = SCB->BFAR;
        } else {
                bus_fault_addr = 0x0;
        }
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                        hw_watchdog_freeze();                           // Stop WDOG
        }
        while (1) {}
}

__RETAINED_CODE void UsageFault_Handler(void)
{
        volatile uint16_t usage_fault_status_reg __UNUSED;

        usage_fault_status_reg = (SCB->CFSR & SCB_CFSR_USGFAULTSR_Msk) >> SCB_CFSR_USGFAULTSR_Pos;

#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_DISABLE_MSK;
#endif

        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                        hw_watchdog_freeze();                           // Stop WDOG
        }

        while (1) {}
}

__RETAINED_CODE void DebugMon_Handler(void)
{
#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_DISABLE_MSK;
#endif

        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                        hw_watchdog_freeze();                           // Stop WDOG
        }

        while (1) {}
}

#endif /* MAIN_PROCESSOR_BUILD */


