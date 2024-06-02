/*
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

#if (SNC_PROCESSOR_BUILD)

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/errno.h>
#include "sdk_defs.h"
#include "interrupts.h"
#include "hw_pd.h"
#include "sys_bsr.h"
#include "sys_tcs.h"
#if (dg_configUSE_MAILBOX == 1)
#include "mailbox.h"
#endif

#ifdef OS_PRESENT
#include "../../sys_man/sys_timer_internal.h"
#endif

/*
 * Linker symbols
 *
 * Note: if any of them is missing, please correct your linker script. Please refer to the linker
 * script of pxp_reporter.
 */
extern uint32_t __copy_table_start__;
extern uint32_t __copy_table_end__;
extern uint32_t __zero_table_start__;
extern uint32_t __zero_table_end__;
extern uint8_t end;
extern uint8_t __HeapLimit;

/*
 * Global variables
 */
__RETAINED_RW static uint8_t *heapend = &end;
__RETAINED_RW uint32_t SystemLPClock = dg_configXTAL32K_FREQ;   /*!< System Low Power Clock Frequency (LP Clock) */


/**
 * @brief  Memory safe implementation of newlib's _sbrk().
 *
 */
__LTO_EXT
void *_sbrk(int incr)
{
        uint8_t *newheapstart;

        if (heapend + incr > &__HeapLimit) {
                /* Hitting this, means that the value of _HEAP_SIZE is too small.
                 * The value of incr is in stored_incr at this point. By checking the equation
                 * above, it is straightforward to determine the missing space.
                 */
                volatile int stored_incr __UNUSED;

                stored_incr = incr;
                ASSERT_ERROR(0);

                errno = ENOMEM;
                return (void *)-1;
        }

        newheapstart = heapend;
        heapend += incr;

        return newheapstart;
}


/*
 * Dialog default priority configuration table.
 *
 * Content of this table will be applied at system start.
 *
 * \note If interrupt priorities provided by Dialog need to be changed, do not modify this file.
 * Create similar table with SAME name instead somewhere inside code without week attribute,
 * and it will be used instead of table below.
 */
#pragma weak __dialog_interrupt_priorities
INTERRUPT_PRIORITY_CONFIG_START(__dialog_interrupt_priorities)
        PRIORITY_0, /* Start interrupts with priority 0 (highest) */
                /*
                 * Note: Interrupts with priority 0 are not
                 * allowed to perform OS calls.
                 */
        PRIORITY_1, /* Start interrupts with priority 1 */
                CMAC2SNC_IRQn,
                SYS2SNC_IRQn,
                I2C_IRQn,
                I2C2_IRQn,
                I2C3_IRQn,
                I3C_IRQn,
                SPI_IRQn,
                SPI2_IRQn,
                SPI3_IRQn,
                ADC_IRQn,
                SRC_IN_IRQn,
                SRC_OUT_IRQn,
                SRC2_IN_IRQn,
                SRC2_OUT_IRQn,
        PRIORITY_2, /* Start interrupts with priority 2 */
                SysTick_IRQn,
                UART_IRQn,
                UART2_IRQn,
                UART3_IRQn,
                KEY_WKUP_GPIO_IRQn,
                GPIO_P0_IRQn,
                GPIO_P1_IRQn,
                GPIO_P2_IRQn,
                TIMER_IRQn,
#ifndef OS_PRESENT
                TIMER3_IRQn,
#endif /* OS_PRESENT */
                TIMER4_IRQn,
                TIMER5_IRQn,
                TIMER6_IRQn,
                CAPTIMER_IRQn,
                RTC_IRQn,
                RTC_EVENT_IRQn,
                PCM_IRQn,
                VAD_IRQn,
                PDC_SNC_IRQn,
        PRIORITY_3, /* Start interrupts with priority 3 (lowest) */
#ifdef OS_PRESENT
                TIMER3_IRQn,
#endif /* OS_PRESENT */
INTERRUPT_PRIORITY_CONFIG_END

void set_interrupt_priorities(const int8_t prios[])
{
        uint32_t old_primask, iser;
        int i = 0;
        uint32_t prio = 0;

        /*
         * We shouldn't change the priority of an enabled interrupt.
         *  1. Globally disable interrupts, saving the global interrupts disable state.
         *  2. Explicitly disable all interrupts, saving the individual interrupt enable state.
         *  3. Set interrupt priorities.
         *  4. Restore individual interrupt enables.
         *  5. Restore global interrupt enable state.
         */
        old_primask = __get_PRIMASK();
        __disable_irq();
        iser  = NVIC->ISER[0];
        NVIC->ICER[0] = iser;

        for (i = 0; prios[i] != PRIORITY_TABLE_END; ++i) {
                switch (prios[i]) {
                case PRIORITY_0:
                case PRIORITY_1:
                case PRIORITY_2:
                case PRIORITY_3:
                        prio = prios[i] - PRIORITY_0;
                        break;
                default:
                        NVIC_SetPriority(prios[i], prio);
                        break;
                }
        }

        NVIC->ISER[0] = iser;
        __set_PRIMASK(old_primask);
}

__STATIC_FORCEINLINE void disable_pdc_irq(void)
{
        NVIC_DisableIRQ(PDC_SNC_IRQn);
        NVIC_ClearPendingIRQ(PDC_SNC_IRQn);
}

__STATIC_FORCEINLINE void enable_debuggers(void)
{
}

__STATIC_FORCEINLINE void check_copy_and_zero_tables(void)
{
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                const uint32_t copy_table_size = (uintptr_t) &__copy_table_end__ - (uintptr_t)&__copy_table_start__;
                const uint32_t *copy_table_end = &__copy_table_start__ + (copy_table_size / sizeof(uint32_t));
                const uint32_t zero_table_size = (uintptr_t) &__zero_table_end__ - (uintptr_t)&__zero_table_start__;
                const uint32_t *zero_table_end = &__zero_table_start__ + (zero_table_size / sizeof(uint32_t));
                uint32_t *p;

                /*
                 * Ensure 4-byte alignment for all elements of each entry in the Copy Table.
                 * If any of the assertions below hits, please correct your linker script
                 * file accordingly!
                 */
                for (p = &__copy_table_start__; p < copy_table_end;) {
                        ASSERT_WARNING( (*p++ & 0x3) == 0 );    // from
                        ASSERT_WARNING( (*p++ & 0x3) == 0 );    // to
                        ASSERT_WARNING( (*p++ & 0x3) == 0 );    // size
                }

                /*
                 * Ensure 4-byte alignment for all elements of each entry in the Zero Table.
                 * If any of the assertions below hits, please correct your linker script
                 * file accordingly!
                 */
                for (p = &__zero_table_start__; p < zero_table_end;) {
                        ASSERT_WARNING( (*p++ & 0x3) == 0 );    // start at
                        ASSERT_WARNING( (*p++ & 0x3) == 0 );    // size
                }
        }
}

__STATIC_FORCEINLINE void init_power_domains(void)
{
        GLOBAL_INT_DISABLE();
        GLOBAL_INT_RESTORE();
}

__STATIC_FORCEINLINE void disable_unused_peripherals(void)
{
}

/**
 * @brief Early system setup
 *
 * Set up the AMBA clocks.
 * Ensure proper alignment of copy and zero table entries.
 *
 * @note   No variable initialization should take place here, since copy & zero tables
 *         have not yet been initialized yet and any modifications to variables will
 *         be discarded. For the same reason, functions that initialize or are
 *         using initialized variables should not be called from here.
 */
void SystemInitPre(void) __attribute__((section("text_reset")));
void SystemInitPre(void)
{
        assertion_functions_set_to_uninit();

        enable_debuggers();

        check_copy_and_zero_tables();

        init_power_domains();

        disable_unused_peripherals();

        disable_pdc_irq();
}

static void da1470x_snc_SystemInit(void)
{
        /*
         * By now, the assertion function pointers should have been updated (via
         * the copy table) to point to the "init" assert function implementations.
         * But in the special case that the LMA of data is equal to their VMA (i.e.
         * the copy table copies data from address X to address X), the "initial"
         * values of the function pointers will be overwritten. To be on the safe
         * side we explicitly set the function pointers to what we want.
         */
        assertion_functions_set_to_init();

        sys_bsr_initialize();

        /* Apply default priorities to interrupts */
        set_interrupt_priorities(__dialog_interrupt_priorities);

        SystemLPClock = dg_configXTAL32K_FREQ;

#ifdef OS_PRESENT
        sys_timer_retrieve_shared_timer_vars();
        /*
         * OS Timer requires PD_TIM to be always on.
         */
        ASSERT_WARNING(hw_pd_check_tim_status());
#endif

        /* Get TCS values */
        sys_tcs_get_trim_values_from_cs();


#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT) || defined(CONFIG_SEMIHOSTING)
        /*
         * This is needed to initialize stdout, so that it can be used by putchar
         * (which doesn't initialize stdout, contrary to printf).
         * This also has the side effect of changing stdout to unbuffered
         * (which seems more reasonable).
         */
        setvbuf(stdout, NULL, _IONBF, 0);
#endif

#if dg_configUSE_MAILBOX
        mailbox_init();
#endif /* dg_configUSE_MAILBOX */
}

typedef void (* init_func_ptr)(void);
/*
 * Add pointer to da1470x_snc_SystemInit() in an array that will go in the .preinit_array section.
 * __libc_init_array() (which is called by _start()) calls all function pointers in .preinit_array.
 */
__attribute__ ((__used__, section(".preinit_array"), aligned(__alignof__(init_func_ptr))))
static init_func_ptr __da1470x_SystemInit_init_array_entry[] = {
        da1470x_snc_SystemInit,
};

#endif /* SNC_PROCESSOR_BUILD */
