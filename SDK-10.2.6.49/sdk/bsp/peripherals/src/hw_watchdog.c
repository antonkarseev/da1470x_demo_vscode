/**
 ****************************************************************************************
 *
 * @file hw_watchdog.c
 *
 * @brief Implementation of the Watchdog timer Low Level Driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include "hw_watchdog.h"
#include "hw_sys.h"

#if (SNC_PROCESSOR_BUILD)
#include "snc.h"
#endif /* SNC_PROCESSOR_BUILD */

/*
 * Global variables
 */
volatile uint32_t nmi_event_data[9] __attribute__((section("nmi_info")));

/*
 * Local variables
 */
__RETAINED static hw_watchdog_interrupt_cb int_handler;

/*
 * This is the base address in Retention RAM where the stacked information will be copied.
 */
#if MAIN_PROCESSOR_BUILD
#define STATUS_BASE (0xF000200)
#elif SNC_PROCESSOR_BUILD
#define STATUS_BASE (0x00045600)
#endif /* PROCESSOR_BUILD */

/* MTB */
#if MAIN_PROCESSOR_BUILD
#define MTB_MASTER_REG                  ((uint32_t *) (0xE0043004))
#define MTB_MASTER_DISABLE_MSK          (0x00000008)
#elif SNC_PROCESSOR_BUILD
#define MTB_MASTER_REG                  ((uint32_t *) (0xF0002000))
#define MTB_MASTER_DISABLE_MSK          (0x00000008)
#endif /* PROCESSOR_BUILD */

#if (MAIN_PROCESSOR_BUILD)
__RETAINED_CODE bool hw_watchdog_freeze(void)
{
        GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SYS_WDOG_Msk;

        return (REG_GETF(SYS_WDOG, WATCHDOG_CTRL_REG, WDOG_FREEZE_EN) &&
                !REG_GETF(SYS_WDOG, WATCHDOG_CTRL_REG, NMI_RST));
}

__RETAINED_CODE bool hw_watchdog_unfreeze(void)
{
        GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SYS_WDOG_Msk;

        return (REG_GETF(SYS_WDOG, WATCHDOG_CTRL_REG, WDOG_FREEZE_EN) &&
                !REG_GETF(SYS_WDOG, WATCHDOG_CTRL_REG, NMI_RST));
}
#endif /* MAIN_PROCESSOR_BUILD */

#if (SNC_PROCESSOR_BUILD)
bool hw_watchdog_is_timer_expired(void)
{
        return REG_GETF(SNC, SNC_STATUS_REG, WDOG_HAS_EXPIRED);
}
#endif /* PROCESSOR_BUILD */

HW_WDG_RESET hw_watchdog_is_irq_or_rst_gen(void)
{
#if (MAIN_PROCESSOR_BUILD)
        if (REG_GETF(SYS_WDOG, WATCHDOG_CTRL_REG, NMI_RST)) {
                return HW_WDG_RESET_RST;
        }
#elif (SNC_PROCESSOR_BUILD)
        if (REG_GETF(SNC, SNC_WDOG_REG, SNC_WDOG_EXPIRE)) {
                return HW_WDG_RESET_RST;
        }
#endif /* PROCESSOR_BUILD */

        return HW_WDG_RESET_NMI;
}

void hw_watchdog_register_int(hw_watchdog_interrupt_cb handler)
{
        int_handler = handler;
}

void hw_watchdog_unregister_int(void)
{
        int_handler = NULL;
}

__RETAINED_CODE void hw_watchdog_handle_int(unsigned long *exception_args)
{
#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_DISABLE_MSK;
#endif /* dg_configENABLE_MTB */

        // Reached this point due to a WDOG timeout
#if (MAIN_PROCESSOR_BUILD)
        uint16_t pmu_ctrl_reg = CRG_TOP->PMU_CTRL_REG;
        pmu_ctrl_reg |= ((1 << CRG_TOP_PMU_CTRL_REG_RADIO_SLEEP_Pos)   |        /* turn off radio PD */
                         (1 << CRG_TOP_PMU_CTRL_REG_GPU_SLEEP_Pos)     |        /* turn off GPU PD */
                         (1 << CRG_TOP_PMU_CTRL_REG_AUD_SLEEP_Pos)     |        /* turn off Audio PD */
                         (1 << CRG_TOP_PMU_CTRL_REG_SNC_SLEEP_Pos));            /* turn off SNC PD */
        CRG_TOP->PMU_CTRL_REG = pmu_ctrl_reg;
#endif /* MAIN_PROCESSOR_BUILD */

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
#if (MAIN_PROCESSOR_BUILD)
        hw_watchdog_freeze();                           // Stop WDOG
        GPREG->SET_FREEZE_REG |= (GPREG_SET_FREEZE_REG_FRZ_SWTIM_Msk  |
                                  GPREG_SET_FREEZE_REG_FRZ_SWTIM2_Msk |
                                  GPREG_SET_FREEZE_REG_FRZ_SWTIM3_Msk |
                                  GPREG_SET_FREEZE_REG_FRZ_SWTIM4_Msk |
                                  GPREG_SET_FREEZE_REG_FRZ_SWTIM5_Msk |
                                  GPREG_SET_FREEZE_REG_FRZ_SWTIM6_Msk);
        ENABLE_DEBUGGER;
#elif (SNC_PROCESSOR_BUILD)
        hw_watchdog_set_pos_val(REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_CNT));      // Reset WDOG

        ENABLE_SNC_DEBUGGER;
#endif /* PROCESSOR_BUILD */

        if (exception_args != NULL) {
                *(volatile unsigned long *)(STATUS_BASE       ) = exception_args[0];    // R0
                *(volatile unsigned long *)(STATUS_BASE + 0x04) = exception_args[1];    // R1
                *(volatile unsigned long *)(STATUS_BASE + 0x08) = exception_args[2];    // R2
                *(volatile unsigned long *)(STATUS_BASE + 0x0C) = exception_args[3];    // R3
                *(volatile unsigned long *)(STATUS_BASE + 0x10) = exception_args[4];    // R12
                *(volatile unsigned long *)(STATUS_BASE + 0x14) = exception_args[5];    // LR
                *(volatile unsigned long *)(STATUS_BASE + 0x18) = exception_args[6];    // PC
                *(volatile unsigned long *)(STATUS_BASE + 0x1C) = exception_args[7];    // PSR
                *(volatile unsigned long *)(STATUS_BASE + 0x20) = (unsigned long)exception_args;    // Stack Pointer

#if (MAIN_PROCESSOR_BUILD)
                *(volatile unsigned long *)(STATUS_BASE + 0x24) = (*((volatile unsigned long *)(0xE000ED28)));    // CFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x28) = (*((volatile unsigned long *)(0xE000ED2C)));    // HFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x2C) = (*((volatile unsigned long *)(0xE000ED30)));    // DFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x30) = (*((volatile unsigned long *)(0xE000ED3C)));    // AFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x34) = (*((volatile unsigned long *)(0xE000ED34)));    // MMAR
                *(volatile unsigned long *)(STATUS_BASE + 0x38) = (*((volatile unsigned long *)(0xE000ED38)));    // BFAR
#endif /* MAIN_PROCESSOR_BUILD */
        }

        if (EXCEPTION_DEBUG == 1) {
                hw_sys_assert_trigger_gpio();
        }

#if (MAIN_PROCESSOR_BUILD)
        if (REG_GETF(CRG_TOP, SYS_STAT_REG, DBG_IS_ACTIVE)) {
                __BKPT(0);
        }
        else {
                while (1);
        }
#elif (SNC_PROCESSOR_BUILD)
        snc_set_shared_space_addr((void *)STATUS_BASE, SNC_SHARED_SPACE_EXCEPTION_NMI);

        /* Notify SYSCPU to indicate SNC error. */
        snc_signal_error(SNC_ERROR_NMI, (void *)STATUS_BASE);

        FREEZE_SNC_WATCHDOG_WHILE(1);
#endif /* PROCESSOR_BUILD */
#else // dg_configIMAGE_SETUP == DEVELOPMENT_MODE
        if (exception_args != NULL) {
                nmi_event_data[0] = NMI_MAGIC_NUMBER;
                nmi_event_data[1] = exception_args[0];          // R0
                nmi_event_data[2] = exception_args[1];          // R1
                nmi_event_data[3] = exception_args[2];          // R2
                nmi_event_data[4] = exception_args[3];          // R3
                nmi_event_data[5] = exception_args[4];          // R12
                nmi_event_data[6] = exception_args[5];          // LR
                nmi_event_data[7] = exception_args[6];          // PC
                nmi_event_data[8] = exception_args[7];          // PSR
        }

#if (MAIN_PROCESSOR_BUILD)
        // Wait for the reset to occur
        while (1);
#elif (SNC_PROCESSOR_BUILD)
        snc_set_shared_space_addr((void *)nmi_event_data, SNC_SHARED_SPACE_EXCEPTION_NMI);

        /* Notify SYSCPU to indicate SNC error. */
        snc_signal_error(SNC_ERROR_NMI, (void *)nmi_event_data);

        FREEZE_SNC_WATCHDOG_WHILE(1);
#endif /* PROCESSOR_BUILD */
#endif // dg_configIMAGE_SETUP == DEVELOPMENT_MODE
}

__RETAINED_CODE void NMI_HandlerC(unsigned long *exception_args);

void NMI_HandlerC(unsigned long *exception_args)
{
        if (int_handler) {
                int_handler(exception_args);
        }
        else {
                hw_watchdog_handle_int(exception_args);
        }
}


