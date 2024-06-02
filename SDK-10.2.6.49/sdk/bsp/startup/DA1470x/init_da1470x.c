/*
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */


#if (MAIN_PROCESSOR_BUILD)

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/errno.h>
#include <stdlib.h>
#include "sdk_defs.h"
#include "interrupts.h"
#include "hw_bod.h"
#include "hw_cache.h"
#include "hw_clk.h"
#include "hw_gpio.h"
#include "hw_memctrl.h"
#include "hw_otpc.h"
#include "hw_pdc.h"
#include "hw_pd.h"
#include "hw_rtc.h"
#include "hw_qspi.h"
#include "hw_sys.h"
#include "sys_bsr.h"
#include "sys_boot.h"
#include "hw_pmu.h"
#include "qspi_automode.h"
#if dg_configUSE_HW_OQSPI
#include "oqspi_automode.h"
#endif
#if (dg_configUSE_HW_DCACHE == 1)
#include "hw_dcache.h"
#endif
#include "sys_tcs.h"
#include "sys_trng.h"
#include "sys_drbg.h"

#if defined(CONFIG_USE_SNC)
#include "snc.h"
#endif
#if (dg_configUSE_MAILBOX == 1)
#include "mailbox.h"
#endif

#if dg_configUSE_CLOCK_MGR
#include "sys_clock_mgr.h"
#include "../../system/sys_man/sys_clock_mgr_internal.h"
#endif

#include "../../peripherals/src/hw_sys_internal.h"
#include "../../system/sys_man/sys_timer_internal.h"


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

#if dg_configUSE_SYS_DRBG
/**
* \brief  SDK implementation of stdlib's rand().
*
*/
int rand(void)
{
        uint32_t rand_number = 0;
        sys_drbg_read_rand(&rand_number);
        return (int)(rand_number % ((uint32_t)RAND_MAX + 1));
}

/**
* \brief  SDK implementation of stdlib's srand().
*
*/
void srand (unsigned __seed)
{
}
#endif /* dg_configUSE_SYS_DRBG */

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
                CMAC2SYS_IRQn,
                CRYPTO_IRQn,
                RFDIAG_IRQn,
        PRIORITY_2, /* Start interrupts with priority 2 */
                SNC2SYS_IRQn,
                DMA_IRQn,
                I2C_IRQn,
                I2C2_IRQn,
                I2C3_IRQn,
                I3C_IRQn,
                SPI_IRQn,
                SPI2_IRQn,
                SPI3_IRQn,
                ADC_IRQn,
                ADC2_IRQn,
                SRC_IN_IRQn,
                SRC_OUT_IRQn,
                SRC2_IN_IRQn,
                SRC2_OUT_IRQn,
        PRIORITY_3, /* Start interrupts with priority 3 */
                SysTick_IRQn,
                UART_IRQn,
                UART2_IRQn,
                UART3_IRQn,
                M33_Cache_MRM_IRQn,
                XTAL32M_RDY_IRQn,
                PLL_LOCK_IRQn,
                CHARGER_STATE_IRQn,
                CHARGER_ERROR_IRQn,
                LCD_IRQn,
                KEY_WKUP_GPIO_IRQn,
                GPIO_P0_IRQn,
                GPIO_P1_IRQn,
                GPIO_P2_IRQn,
                TIMER_IRQn,
#ifndef OS_PRESENT
                TIMER2_IRQn,
#endif /* OS_PRESENT */
                TIMER3_IRQn,
                TIMER4_IRQn,
                TIMER5_IRQn,
                TIMER6_IRQn,
                CAPTIMER_IRQn,
                RTC_IRQn,
                RTC_EVENT_IRQn,
                USB_IRQn,
                PCM_IRQn,
                VBUS_IRQn,
                PLL48_LOCK_IRQn,
                DCDC_BOOST_IRQn,
                VAD_IRQn,
                GPU_IRQn,
                PDC_M33_IRQn,
                eMMC_IRQn,
                CHARGER_DET_IRQn,
                DCACHE_MRM_IRQn,
                CLK_CALIBRATION_IRQn,
                VSYS_GEN_IRQn,
        PRIORITY_4, /* Start interrupts with priority 4 */
        PRIORITY_5, /* Start interrupts with priority 5 */
        PRIORITY_6, /* Start interrupts with priority 6 */
        PRIORITY_7, /* Start interrupts with priority 7 */
        PRIORITY_8, /* Start interrupts with priority 8 */
        PRIORITY_9, /* Start interrupts with priority 9 */
        PRIORITY_10, /* Start interrupts with priority 10 */
        PRIORITY_11, /* Start interrupts with priority 11 */
        PRIORITY_12, /* Start interrupts with priority 12 */
        PRIORITY_13, /* Start interrupts with priority 13 */
        PRIORITY_14, /* Start interrupts with priority 14 */
        PRIORITY_15, /* Start interrupts with priority 15 (lowest) */
#ifdef OS_PRESENT
                TIMER2_IRQn,
#endif /* OS_PRESENT */
INTERRUPT_PRIORITY_CONFIG_END

void set_interrupt_priorities(const int8_t prios[])
{
        uint32_t old_primask, iser, iser2;
        int i = 0;
        uint32_t prio = 0;

        // Set interrupt sub-priority bits to minimum (required by the OS)
        NVIC_SetPriorityGrouping(0);

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
        iser2 = NVIC->ISER[1];
        NVIC->ICER[0] = iser;
        NVIC->ICER[1] = iser2;

        for (i = 0; prios[i] != PRIORITY_TABLE_END; ++i) {
                switch (prios[i]) {
                case PRIORITY_0:
                case PRIORITY_1:
                case PRIORITY_2:
                case PRIORITY_3:
                case PRIORITY_4:
                case PRIORITY_5:
                case PRIORITY_6:
                case PRIORITY_7:
                case PRIORITY_8:
                case PRIORITY_9:
                case PRIORITY_10:
                case PRIORITY_11:
                case PRIORITY_12:
                case PRIORITY_13:
                case PRIORITY_14:
                case PRIORITY_15:
                        prio = prios[i] - PRIORITY_0;
                        break;
                default:
                        NVIC_SetPriority(prios[i], prio);
                        break;
                }
        }

        NVIC->ISER[0] = iser;
        NVIC->ISER[1] = iser2;
        __set_PRIMASK(old_primask);

        // enable Usage-, Bus-, and MMU Fault
        SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk
                   |  SCB_SHCSR_BUSFAULTENA_Msk
                   |  SCB_SHCSR_MEMFAULTENA_Msk;
}

#if dg_configUSE_CLOCK_MGR == 0

void XTAL32M_Ready_Handler(void)
{
        while (!hw_clk_is_xtalm_started());
}

void PLL_Lock_Handler(void)
{
        ASSERT_WARNING(REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, PLL_LOCK_FINE));
}

void PLL48_Lock_Handler(void)
{
        ASSERT_WARNING(REG_GETF(CRG_XTAL, PLL_USB_STATUS_REG, PLL_LOCK_FINE));
}

/* carry out clock initialization sequence */
static void nortos_clk_setup(bool is_xtal32m_sysclk)
{
        /*
         * Low power clock
         */
        hw_clk_enable_lpclk(LP_CLK_IS_RCLP);
        hw_clk_set_lpclk(LP_CLK_IS_RCLP);

        NVIC_ClearPendingIRQ(XTAL32M_RDY_IRQn);
        NVIC_EnableIRQ(XTAL32M_RDY_IRQn);              // Activate XTAL32M Ready IRQ

        hw_clk_xtalm_irq_enable();
        if (is_xtal32m_sysclk) {
                hw_clk_enable_sysclk(SYS_CLK_IS_XTAL32M);      // Enable XTAL32M

                /* Wait for XTAL32M to settle */
                while (!hw_clk_is_xtalm_started());

                hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);
        }
        NVIC_ClearPendingIRQ(PLL_LOCK_IRQn);
        NVIC_EnableIRQ(PLL_LOCK_IRQn);                         // Activate PLL lock IRQ

        NVIC_ClearPendingIRQ(PLL48_LOCK_IRQn);
        NVIC_EnableIRQ(PLL48_LOCK_IRQn);                             // Activate PLL48M Lock IRQ
}

#endif  /* dg_configUSE_CLOCK_MGR == 0 */


static __RETAINED_HOT_CODE void configure_cache(void)
{
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_OQSPI_FLASH)

        uint32_t product_header_addr;
        uint32_t active_fw_image_addr;
        uint32_t active_fw_size;
        uint32_t cache_len;
        uint32_t scanned_sectors = 0;

        /* Configure cache according to the 'Active FW image address'
         * field of the product header and 'FW Size' field of the active
         * FW image header. */

        /* Product header is located either at start of FLASH or at the sector
         * boundary (0x4000) if a configuration script is used */

        product_header_addr = MEMORY_OQSPIC_S_BASE;
        while ((((uint8_t*) product_header_addr)[0] != 0x50) &&
                (((uint8_t*) product_header_addr)[1] != 0x70) &&
                (scanned_sectors < 10)) {
                product_header_addr += 0x1000;
                scanned_sectors++;
        }

        /* Get active_fw_image_addr */
        ASSERT_WARNING(((uint8_t*) product_header_addr)[0] == 0x50);
        ASSERT_WARNING(((uint8_t*) product_header_addr)[1] == 0x70);
        memcpy((uint8_t*) &active_fw_image_addr, &((uint8_t*) product_header_addr)[2], 4);
        active_fw_image_addr += MEMORY_OQSPIC_S_BASE;

        /* Get active_fw_size and align it to 64K boundary */
        ASSERT_WARNING(((uint8_t*) active_fw_image_addr)[0] == 0x51);
        ASSERT_WARNING(((uint8_t*) active_fw_image_addr)[1] == 0x71);
        memcpy((uint8_t*) &active_fw_size, &((uint8_t*) active_fw_image_addr)[2], 4);
        active_fw_size += (0x10000 - (active_fw_size % 0x10000)) % 0x10000;

        /* Calculate length of QSPI FLASH cacheable memory
         * (Cached area len will be (cache_len * 64) KBytes, cache_len can be 0 to 512) */
        cache_len = active_fw_size >> 16;

        hw_cache_disable();
        hw_cache_set_extflash_cacheable_len(cache_len);
        hw_cache_disable_cwf();
        hw_cache_enable();

#endif /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_OQSPI_FLASH) */
}

/* this function configures the PDC table only the first time it is called */
static void configure_pdc(void)
{
        uint32_t pdc_entry_index __UNUSED;
        bool no_syscpu_pdc_entries = true;
        NVIC_DisableIRQ(PDC_IRQn);
        NVIC_ClearPendingIRQ(PDC_IRQn);

#if (dg_configUSE_SYS_CHARGER) || (dg_configENABLE_DEBUGGER == 1)
        /* Set up PDC entry for VBUS IRQ or debugger */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_COMBO,
                                                HW_PDC_MASTER_CM33,
                                                (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
        /* cppcheck-suppress redundantAssignment */
        no_syscpu_pdc_entries = false;
#endif

#if defined(CONFIG_USE_BLE)
        /* Set up PDC entry for CMAC2SYS IRQ */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_CMAC2SYS,
                                                HW_PDC_MASTER_CM33,
                                                (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
        /* cppcheck-suppress redundantAssignment */
        no_syscpu_pdc_entries = false;
#endif

#if defined(CONFIG_USE_SNC)
        /* Set up PDC entry for SNC2SYS IRQ  */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_SNC2SYS,
                                                HW_PDC_MASTER_CM33,
                                                (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
        /* cppcheck-suppress redundantAssignment */
        no_syscpu_pdc_entries = false;
#endif

#if defined(CONFIG_USE_BLE)
        /*
         * Set up PDC entry for CMAC wakeup from MAC timer.
         * This entry is also used for the SYS2CMAC mailbox interrupt.
         */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                        HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                        HW_PDC_PERIPH_TRIG_ID_MAC_TIMER,
                                                        HW_PDC_MASTER_CMAC,
                                                        HW_PDC_LUT_ENTRY_EN_XTAL));

        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
#endif

#if defined(CONFIG_USE_SNC)
        /*
         * Set up PDC entry for SNC wake-up from Timer 3.
         */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                        HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                        HW_PDC_PERIPH_TRIG_ID_TIMER3,
                                                        HW_PDC_MASTER_SNC,
                                                        0));

        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);

        /*
         * Store the PDC start up entry for M33.
         * When snc_start() is called M33 will set this entry to pending in PDC, thus PDC
         * will keep PD_SNC power domain enabled and SNC will be able to execute cold start.
         */
        snc_set_prevent_power_down_pdc_entry_index(pdc_entry_index);

        /* Set up PDC entry for SYS2SNC IRQ  */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_SYS2SNC,
                                                HW_PDC_MASTER_SNC,
                                                0));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);

#endif

#ifdef OS_PRESENT
        /*
         * Set up PDC entry for CM33 wake-up from Timer 2.
         */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_TIMER2,
                                                HW_PDC_MASTER_CM33,
                                                (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
        /* cppcheck-suppress redundantAssignment */
        no_syscpu_pdc_entries = false;
#endif /* OS_PRESENT */

        /* Let SYSCPU goto sleep when needed */
        if (!no_syscpu_pdc_entries) {
                REG_SETF(CRG_TOP, PMU_CTRL_REG, SYS_SLEEP, 1);
        }

        /* clear the PDC IRQ since it will be pending here */
        NVIC_ClearPendingIRQ(PDC_IRQn);
}

#if dg_configUSE_CLOCK_MGR && dg_configUSE_HW_RTC
/* this function configures the RTC clock and RTC_KEEP_RTC_REG*/
static void configure_rtc(void)
{
        uint16_t div_int;
        uint16_t div_frac;
#ifdef OS_PRESENT
        div_int = lp_clock_hz / 100;
        div_frac = 10 * (lp_clock_hz - (div_int * 100));
#else
        uint32_t rcx_clk_hz =  cm_get_rcx_clock_hz_acc() / RCX_ACCURACY_LEVEL;
        div_int = rcx_clk_hz / 100;
        div_frac = 10 * (rcx_clk_hz - (div_int * 100));

#endif /* OS_PRESENT */
        hw_rtc_clk_config(RTC_DIV_DENOM_1000, div_int, div_frac);

        hw_rtc_set_keep_reg_on_reset(true);
}
#endif


__STATIC_FORCEINLINE void enable_debuggers(void)
{
        /*
         * Enable M33 debugger.
         */
        if (dg_configENABLE_DEBUGGER) {
                ENABLE_DEBUGGER;
        } else {
                DISABLE_DEBUGGER;
        }

        /*
         * Enable CMAC debugger.
         */
        if (dg_configENABLE_CMAC_DEBUGGER) {
                ENABLE_CMAC_DEBUGGER;
        } else {
                DISABLE_CMAC_DEBUGGER;
        }

        /*
         * Enable SNC debugger.
         */
        if (dg_configENABLE_SNC_DEBUGGER) {
                ENABLE_SNC_DEBUGGER;
        } else {
                DISABLE_SNC_DEBUGGER;
        }
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
        REG_SETF(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, RAD_IS_DOWN));
        REG_SETF(CRG_TOP, PMU_CTRL_REG, SNC_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, SNC_IS_DOWN));
#ifdef OS_PRESENT
        /* OS Timer requires PD_TIM to be always on */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP, 0);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP));
#endif /* OS_PRESENT */
        GLOBAL_INT_RESTORE();
}

__STATIC_FORCEINLINE void disable_unused_peripherals(void)
{
#if (dg_configUSE_HW_OQSPI == 0)
        /*
         * Since we are executing from RAM, OQSPI can be disabled.
         */
        REG_SETF(CRG_TOP, CLK_AMBA_REG, OQSPIF_ENABLE, 0);
#endif

#if (dg_configUSE_HW_QSPI == 0)
        REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPIC_ENABLE, 0);
#endif

#if (dg_configUSE_HW_QSPI2 == 0)
        REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPIC2_ENABLE, 0);
#endif

        REG_SETF(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_AMBA_REG, OTP_ENABLE, 0);
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

        // Populate device information attributes
        ASSERT_WARNING(hw_sys_device_info_init());

        apply_cs_register_values_for_untrimmed_samples();
        enable_debuggers();

        /*
         * Bandgap has already been set by the bootloader.
         * Use fast clocks from now on.
         */
        hw_clk_set_hclk_div(0);
        hw_clk_set_pclk_div(0);
        hw_clk_set_pclk_slow_div(0);

        /*
         * Disable pad latches
         */
        hw_gpio_pad_latch_disable_all();

        /*
         * Check that the firmware and the chip that it runs on are compatible with each other.
         */
        ASSERT_WARNING(hw_sys_is_compatible_chip());

        check_copy_and_zero_tables();

        /*
         * Clear all PDC entries and make sure SYS_SLEEP is 0.
         */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, SYS_SLEEP, 0);
        hw_pdc_lut_reset();

        /*
         * Reset memory controller.
         */
        hw_memctrl_reset();

        /*
         * Copy the previous boot status to a safe location.
         */
        sys_boot_secure_copy_boot_result();

        /*
         * Initialize power domains
         */
        init_power_domains();

        /*
         * Keep CMAC core under reset
         */
#if (dg_configUSE_CMAC_RAM9_SIZE == 0) && (dg_configUSE_CMAC_RAM10_SIZE == 0)
        REG_SETF(CRG_TOP, CLK_RADIO_REG, CMAC_CLK_ENABLE, 0);
#endif
        REG_SETF(CRG_TOP, CLK_RADIO_REG, CMAC_SYNCH_RESET, 1);

        disable_unused_peripherals();
}

__STATIC_FORCEINLINE void external_memories_automode_init(void)
{
#if IS_CACHED_FLASH
        /* Disable cache before reinitializing OQSPI */
        hw_cache_disable();
#endif

#if(dg_configUSE_HW_OQSPI == 1)
        /* The bootloader may have left the Flash in wrong mode */
        oqspi_automode_init();
#endif

#if IS_CACHED_FLASH
        hw_cache_enable();
#endif

        /* Initialize QSPI1/2 controllers */
#if (dg_configUSE_HW_QSPI == 1 || dg_configUSE_HW_QSPI2 == 1)
        qspi_automode_init();
#endif

        /* Initialize dCache controller */
#if(dg_configUSE_HW_DCACHE == 1)
        hw_dcache_set_cacheable_base(0);
        hw_dcache_enable();
        hw_dcache_init();
        hw_dcache_set_cacheable_len(HW_DCACHE_CACHEABLE_LEN_MAX);
#else
        /* If the dCache controller is disabled at compile-time, then it must be explicitly bypassed by
         * routing around it all target data memory accesses if we want to achieve optimal performance. */
        REG_SET_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_BYPASS);
#endif
}

__STATIC_FORCEINLINE void fetch_trim_values_from_tcs(void)
{
        /* enable OTP to read TCS values */
        hw_otpc_init();
        hw_otpc_set_speed(HW_OTPC_SYS_CLK_FREQ_32MHz);
        hw_otpc_enter_mode(HW_OTPC_MODE_READ);

        /* get TCS values */
        sys_tcs_get_trim_values_from_cs();

        /*
         * Populate device variant information. This function must be called after retrieving the
         * TCS values from CS otherwise the relevant information is not available.
         */
        ASSERT_WARNING(hw_sys_device_variant_init());

        /* Apply trimmed values for xtal32m in case no entry exists in OTP */
        hw_sys_apply_default_values();

        /* disable OTP */
        hw_otpc_close();
}

__STATIC_FORCEINLINE void setup_bod(void)
{
#if (dg_configUSE_BOD == 1)
        /* BOD has already been enabled at this point but it must be reconfigured */
        hw_bod_configure();
#else
        hw_bod_deactivate();
#endif
}

__STATIC_FORCEINLINE void setup_clocks_and_pdc(bool is_xtal32m_sysclk)
{
        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC == 0) {
                hw_clk_xtalm_configure_irq();
        }
#if dg_configUSE_CLOCK_MGR
        cm_sysclk_init_low_level_internal();

        configure_pdc();

        /*
         * Note: XTAL32M should be set as system clock for RCHS calibration. The system clock will
         *       switch back to RCHS when calibration is done.
         */
        cm_enable_xtalm();
        while (!cm_poll_xtalm_ready());                 // Wait for XTAL32M to settle
        hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);          // Set XTAL32M as sys_clk
        cm_rchs_calibrate();
        cm_lpclk_init_low_level_internal();

        if (!is_xtal32m_sysclk) {
                hw_clk_set_sysclk(SYS_CLK_IS_RCHS);
        }

#if dg_configUSE_HW_RTC
        configure_rtc();
#endif

#else

        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC != 0) {
                uint16_t rdy_cnt = XTAL32M_USEC_TO_250K_CYCLES(dg_configXTAL32M_SETTLE_TIME_IN_USEC);
                hw_clk_set_xtalm_settling_time(rdy_cnt, true);
        }

        configure_pdc();

        /* perform clock initialization here, as there is no clock manager to do it later for us */
        nortos_clk_setup(is_xtal32m_sysclk);

#endif
}

__STATIC_FORCEINLINE void apply_tcs_settings(void)
{
        /* Default settings to be used if no CS setting is available */
        *(volatile uint32_t *)0x51000604 = DEFAULT_CHARGER_TEST_CTRL_REG;

        /* Store the TCS entries for trimming the power rails. */
        hw_pmu_store_trim_values();

        /*
         * Apply tcs settings.
         * They need to be re-applied when the blocks they contain are enabled.
         * PD_MEM is by default enabled.
         * PD_AON settings are applied by the booter
         */
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_MEM);
        /* In non baremetal apps PD_SNC will be enabled by the power manager */
#ifndef OS_PRESENT
#if (dg_configPM_ENABLES_PD_SNC_WHILE_ACTIVE == 1)
        hw_sys_pd_com_enable();
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_SNC);
#endif
#endif
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_SYS);
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_TMR);

        /*
         * Apply preferred settings on top of tcs settings.
         */
        hw_sys_set_preferred_values(HW_PD_SLP);
        hw_sys_set_preferred_values(HW_PD_SYS);
        hw_sys_set_preferred_values(HW_PD_AON);
}


static void da1470x_SystemInit(void)
{
        /*
         * By now, the assert function pointers should have been updated
         * (via the copy table) to point to the "init" assert function
         * implementations.
         * However, the copy-table mechanism is not used for RAM builds, so
         * we must explicitly set the assert function pointers.
         */
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
        assertion_functions_set_to_init();
#endif /* dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE */

        // Populate device information attributes
        ASSERT_WARNING(hw_sys_device_info_init());

#if (dg_configUSE_SYS_TRNG == 1)  && (dg_configUSE_SYS_DRBG == 1)
        if (sys_trng_can_run()) {
                /* After a power cycle the TRNG module can be fed with random data.
                 * This is a prerequisite for the generation of a random seed.
                 */
                ASSERT_WARNING(SYS_TRNG_ERROR_NONE == sys_trng_init());

                /* Set the seed for the random number generator function (it runs only once). */
                sys_drbg_srand();

                /* Generate random numbers */
                sys_drbg_init();
        } else {
                /* Should not end up here after a power cycle! */
                sys_drbg_init();
        }
#else
#if (dg_configUSE_SYS_DRBG == 1)
        if (sys_drbg_can_run()) {
                /* Set the seed for the random number generator function (it runs only once). */
                sys_drbg_srand();
        }
        /* Generate random numbers */
        sys_drbg_init();
#endif /* dg_configUSE_SYS_DRBG */
#endif /* dg_configUSE_SYS_TRNG && dg_configUSE_SYS_DRBG */


        sys_bsr_initialize();

        /* Apply default priorities to interrupts */
        set_interrupt_priorities(__dialog_interrupt_priorities);

        SystemLPClock = dg_configXTAL32K_FREQ;

        external_memories_automode_init();

#ifdef OS_PRESENT
        /*
         * Already up in SystemInitPre()
         * OS Timer requires PD_TIM to be always on
         */
        ASSERT_WARNING(hw_pd_check_tim_status());
#endif

        bool xtal32m_sysclk = (hw_clk_get_sysclk() == SYS_CLK_IS_XTAL32M);
        if (xtal32m_sysclk) {
                // booter has already enabled xtal32m so 1v4 rail should be enabled
                ASSERT_ERROR(REG_GETF(CRG_TOP, ANA_STATUS_REG, BUCK_DCDC_V14_OK));
                hw_clk_enable_sysclk(SYS_CLK_IS_RCHS);
                hw_clk_set_sysclk(SYS_CLK_IS_RCHS);
        } else {
                // xtal32m configuration requires 1v4 rail to be enabled
                hw_pmu_1v4_onwakeup_enable(HW_PMU_1V4_MAX_LOAD_20);
        }
        fetch_trim_values_from_tcs();

#if dg_configUSE_SYS_BOOT
        sys_boot_restore_product_headers();
#endif

        configure_cache();

#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT) || defined(CONFIG_SEMIHOSTING)
        /*
         * This is needed to initialize stdout, so that it can be used by putchar
         * (which doesn't initialize stdout, contrary to printf).
         * Furthermore, it disables line buffering at stdout so there is no need
         * for explicit fflush(stdout) calls.
         */
        setvbuf(stdout, NULL, _IONBF, 0);
#endif

        apply_tcs_settings();
        setup_clocks_and_pdc(xtal32m_sysclk);
        setup_bod();
        hw_sys_enable_ivt_mem_protection();

#if dg_configUSE_MAILBOX
        mailbox_init();
#endif /* dg_configUSE_MAILBOX */

}

typedef void (* init_func_ptr)(void);
/*
 * Add pointer to da1470x_SystemInit() in an array that will go in the .preinit_array section.
 * __libc_init_array() (which is called by _start()) calls all function pointers in .preinit_array.
 */
__attribute__ ((__used__, section(".preinit_array"), aligned(__alignof__(init_func_ptr))))
static init_func_ptr __da1470x_SystemInit_init_array_entry[] = {
        da1470x_SystemInit,
};

__STATIC_FORCEINLINE void get_flash_region(uint32_t *base_offset, uint32_t *size)
{
        uint32_t t;
#define KiB     1024
#define MiB     (1024 * KiB)
        static const uint32_t flash_region_sizes[] = {
                256 * KiB,
                512 * KiB,
                  1 * MiB,
                  2 * MiB,
                  4 * MiB,
                  8 * MiB,
                 16 * MiB,
                 32 * MiB,
                 64 * MiB,
                128 * MiB,
                  0,
                  0,
                  0,
                  0,
                  0,
                  0
        };

        t = hw_cache_flash_get_region_base();
        *base_offset = t << CACHE_CACHE_FLASH_REG_FLASH_REGION_BASE_Pos;
        t = hw_cache_flash_get_region_offset();
        *base_offset += t << 2;
        *size = flash_region_sizes[hw_cache_flash_get_region_size()];
}

uint32_t black_orca_phy_addr(uint32_t addr)
{
        uint32_t phy_addr;
        uint32_t flash_region_base_offset;
        uint32_t flash_region_size;
        HW_SYS_REMAP_ADDRESS_0 remap_addr0;
        static const uint32_t remap[] = {
                MEMORY_ROM_BASE,
                MEMORY_OTP_BASE,
                MEMORY_OQSPIC_BASE,
                MEMORY_SYSRAM_BASE,
                MEMORY_OQSPIC_S_BASE,
                MEMORY_SYSRAM3_BASE,
                MEMORY_CACHERAM_BASE,
                0
        };

        remap_addr0 = hw_sys_get_memory_remapping();

        if (remap_addr0 != HW_SYS_REMAP_ADDRESS_0_TO_OQSPI_FLASH) {
                if (addr >= MEMORY_REMAPPED_END) {
                        phy_addr = addr;
                } else {
                        phy_addr = addr + remap[remap_addr0];
                }
        } else {
                get_flash_region(&flash_region_base_offset, &flash_region_size);

                if (addr < MEMORY_REMAPPED_END) {
                        /*
                         * In the remapped region, accesses are only allowed when
                         * 0 <= addr < flash_region_size.
                         */
                        ASSERT_ERROR(addr < flash_region_size);

                        phy_addr = flash_region_base_offset + addr;
                } else if (IS_OQSPIC_ADDRESS(addr)) {
                        /*
                         * In QSPI AHB-C bus, accesses are only allowed when
                         * flash_region_base_offset <= addr
                         *   AND
                         * addr < flash_region_base_offset + flash_region_base_offset
                         */
                        ASSERT_ERROR(addr >= flash_region_base_offset);
                        ASSERT_ERROR(addr < flash_region_base_offset + flash_region_size);
                        phy_addr = addr;
                } else {
                        phy_addr = addr;
                }
        }

        return phy_addr;
}

#endif /* MAIN_PROCESSOR_BUILD */
