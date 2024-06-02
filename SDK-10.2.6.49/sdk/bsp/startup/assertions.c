/**
 ****************************************************************************************
 *
 * @file assertions.c
 *
 * @brief Assertion functions implementation.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "sdk_defs.h"
#include "hw_watchdog.h"
#ifdef OS_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif
#ifdef OS_DGCOROUTINES
#include "FreeRTOS.h"
#include "croutine.h"
#endif
#include "hw_sys.h"
#if (SNC_PROCESSOR_BUILD)
#include "snc.h"
#endif


#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)

#if MAIN_PROCESSOR_BUILD
#define MTB_MASTER_REG                  ((uint32_t *) (0xE0043004))
#define MTB_MASTER_DISABLE_MSK          (0x00000008)
#elif SNC_PROCESSOR_BUILD
#define MTB_MASTER_REG                  ((uint32_t *) (0xF0002000))
#define MTB_MASTER_DISABLE_MSK          (0x00000008)
#endif /* PROCESSOR_BUILD */

/* Store r0-r3 registers during assert function calls to improve debugging.
 * scratch_registers[0] = r0
 * scratch_registers[1] = r1
 * scratch_registers[2] = r2
 * scratch_registers[3] = r3
 */
static volatile __RETAINED uint32_t  scratch_registers[4];

/**
 * When MTB is enabled, stop the tracing to prevent polluting the MTB
 * buffer with the while(1) in the assert_warning_uninit(), config_assert()
 * and assert_warning()
 */
__STATIC_FORCEINLINE void disable_tracing(void)
{
#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_DISABLE_MSK;
#endif /* dg_configENABLE_MTB */
}

__STATIC_FORCEINLINE void store_scratch_regs(const uint32_t* args)
{
        scratch_registers[0] = args[0];
        scratch_registers[1] = args[1];
        scratch_registers[2] = args[2];
        scratch_registers[3] = args[3];
}
/**
 * The assert_warning() is used from anywhere in the code and is placed in
 * retention RAM to be safely called in all cases
 */
__RETAINED_CODE static void assert_warning(void* args)
{
        __disable_irq();
        store_scratch_regs((uint32_t*)args);
        disable_tracing();
#if (MAIN_PROCESSOR_BUILD)
        hw_watchdog_freeze();
        if (EXCEPTION_DEBUG == 1) {
                hw_sys_assert_trigger_gpio();
        }
        do {} while (1);
#elif (SNC_PROCESSOR_BUILD)
        if (EXCEPTION_DEBUG == 1) {
                hw_sys_assert_trigger_gpio();
        }
        snc_signal_error(SNC_ERROR_ASSERT, NULL);
        FREEZE_SNC_WATCHDOG_WHILE (1);
#endif /* PROCESSOR_BUILD */
}

/**
 * The assert_warning_uninit() is used only during boot in SystemInitPre()
 * while the RAM is not initialized yet, thus is selected to run from FLASH.
 */
static void assert_warning_uninit(void* args)
{
        __disable_irq();
        store_scratch_regs((uint32_t *)args);
        disable_tracing();
#if (MAIN_PROCESSOR_BUILD)
        GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SYS_WDOG_Msk;
        do {} while (1);
#elif (SNC_PROCESSOR_BUILD)
        snc_signal_error(SNC_ERROR_ASSERT, NULL);
        FREEZE_SNC_WATCHDOG_WHILE (1);
#endif /* PROCESSOR_BUILD */
}

__RETAINED_CODE void config_assert(void* args)
{
#ifdef OS_FREERTOS
        taskDISABLE_INTERRUPTS();
#endif
#ifdef OS_DGCOROUTINES
        dgcrDISABLE_INTERRUPTS();
#endif
        store_scratch_regs((uint32_t*)args);
        disable_tracing();
#if (MAIN_PROCESSOR_BUILD)
        hw_watchdog_freeze();
        do {} while (1);
#elif (SNC_PROCESSOR_BUILD)
        snc_signal_error(SNC_ERROR_ASSERT, NULL);
        FREEZE_SNC_WATCHDOG_WHILE (1);
#endif /* PROCESSOR_BUILD */
}

#else /* dg_configIMAGE_SETUP == DEVELOPMENT_MODE */

__RETAINED_CODE static void assert_error(__UNUSED void* args)
{
                __disable_irq();
                __BKPT(2);
}

static void assert_error_uninit(__UNUSED void* args)
{
                __disable_irq();
                __BKPT(2);
}

#endif /* dg_configIMAGE_SETUP == DEVELOPMENT_MODE */


/* Pointers to assertion functions to use
 * In the SystemInitPre these will be initialized to
 * assert_warning_uninit() and assert_error_uninit() (PRODUCTION_MODE)
 * Then the RAM will be initialized and they will take the normal assignment
 * below pointing to assert_warning() and assert_error() (PRODUCTION_MODE)
 */
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
__RETAINED_RW assertion_func_t assert_warning_func = assert_warning;
__RETAINED_RW assertion_func_t assert_error_func = assert_warning;
#else
__RETAINED_RW assertion_func_t assert_error_func = assert_error;
#endif /* (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) */


void assertion_functions_set_to_init(void)
{
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
        assert_warning_func = assert_warning;
        assert_error_func = assert_warning;
#else
        assert_error_func = assert_error;
#endif
}

void assertion_functions_set_to_uninit(void)
{
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
        assert_warning_func = assert_warning_uninit;
        assert_error_func = assert_warning_uninit;
#else
        assert_error_func = assert_error_uninit;
#endif
}
