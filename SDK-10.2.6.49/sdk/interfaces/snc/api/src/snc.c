/**
 ****************************************************************************************
 *
 * @file snc.c
 *
 * @brief Sensor Node Controller (SNC) driver
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifdef CONFIG_USE_SNC

#include <stdint.h>
#include <string.h>
#include "sdk_defs.h"
#include "hw_pdc.h"
#include "snc.h"

#if (MAIN_PROCESSOR_BUILD)
#include "snc_fw_embed.h"
#include "hw_pdc.h"
#endif /* MAIN_PROCESSOR_BUILD */

#if (dg_configSYSTEMVIEW == 1)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif /* (dg_configSYSTEMVIEW == 1) */

#if (MAIN_PROCESSOR_BUILD)
#ifndef SNC_SHARED_SPACE_INFO_ADDRESS
#error "No SNC-SYSCPU shared space has been defined."
#endif /* SNC_SHARED_SPACE_INFO_ADDRESS */
#endif /* MAIN_PROCESSOR_BUILD */


/*
 * MACROS
 *****************************************************************************************
 */

/* The start address of the SNC code/data RAM cells, in each processor's address space. */
#define SNC_CODE_DATA_RAM_START_SNC_PROC        0x00000000U
#define SNC_CODE_DATA_RAM_START_MAIN_PROC       0x20000000U
/* The size of the SNC code/data RAM cells. */
#define SNC_CODE_DATA_RAM_SIZE                  0x10000U

/* The start address of the shared RAM cell, in each processor's address space. */
#define SHARED_RAM_START_SNC_PROC               0x00030000U
#define SHARED_RAM_START_MAIN_PROC              0x20110000U
/* The size of the shared RAM cell. */
#define SHARED_RAM_SIZE                         0x20000U

/* The offset (from the base of the shared RAM cell) where the "SNC shared space" starts. */
#define SHARED_RAM_OFFSET                       0
_Static_assert(SHARED_RAM_OFFSET < 128 * 1024,
                "SNC shared space starts outside the shared RAM cell");

#define SHARED_RAM_SNC_BASE_ADDRESS             (SHARED_RAM_START_SNC_PROC + SHARED_RAM_OFFSET)
#define SHARED_RAM_MAIN_BASE_ADDRESS            (SHARED_RAM_START_MAIN_PROC + SHARED_RAM_OFFSET)

#if (MAIN_PROCESSOR_BUILD)
#define SNC_SHARED_SPACE_BASE_ADDRESS           SHARED_RAM_MAIN_BASE_ADDRESS
#elif (SNC_PROCESSOR_BUILD)
#define SNC_SHARED_SPACE_BASE_ADDRESS           SHARED_RAM_SNC_BASE_ADDRESS
#endif

#define SNC_CONVERT_SNC2SYS_SHARED_RAM_ADDR(snc_addr) \
        ((uintptr_t)(snc_addr) - SHARED_RAM_START_SNC_PROC + SHARED_RAM_START_MAIN_PROC)
#define SNC_CONVERT_SYS2SNC_SHARED_RAM_ADDR(sys_addr) \
        ((uintptr_t)(sys_addr) - SHARED_RAM_START_MAIN_PROC + SHARED_RAM_START_SNC_PROC)

#define HAVE_APP_DEFINED_HANDLES                (SNC_SHARED_SPACE_APP_COUNT > 0)


/*
 * DATA TYPE DEFINITIONS
 *****************************************************************************************
 */

/*
 * SNC firmware image header
 */
typedef struct {
        uint32_t identifier;                    /* Identifier */
        uint32_t size;                          /* Size (in bytes) */
        char version[16];                       /* Version (string) */
        uint32_t timestamp;                     /* Timestamp of SNC firmware image creation */
        uint32_t data[0];                       /* SNC firmware image binary */
} __attribute__((packed)) snc_fw_image_header_t;

/*
 * SNC-SYSCPU shared space information
 */
typedef struct {
        /* Indication of SNC correct start-up */
        volatile uint16_t snc_is_ready  :  1;

        /* Indication of SNC error */
        volatile uint16_t snc_error_val;
        volatile uintptr_t snc_error_args;

#if (HAVE_APP_DEFINED_HANDLES)
        /* Shared space areas defined by the application */
        volatile uintptr_t app_info[SNC_SHARED_SPACE_APP_COUNT];
#endif

        /* Shared space areas defined by the system */
        volatile uintptr_t sys_info[SNC_SHARED_SPACE_HANDLE_MAX - SNC_SHARED_SPACE_PREDEFINED_START];
} snc_shared_space_info_t;

/*
 * DATA STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/*
 * Callback for handling the SNC2SNC or SYS2SNC hardware interrupt
 */
__RETAINED static snc_interrupt_cb_t snc_intr_cb;

/* SNC-SYSCPU shared space information definition */
#if (MAIN_PROCESSOR_BUILD)
static snc_shared_space_info_t * const snc_shared_space_info_ptr =
        (void *)SNC_CONVERT_SNC2SYS_SHARED_RAM_ADDR(SNC_SHARED_SPACE_INFO_ADDRESS);
#elif (SNC_PROCESSOR_BUILD)
__SNC_SHARED static snc_shared_space_info_t             snc_shared_space_info;
#define snc_shared_space_info_ptr                       ( &snc_shared_space_info )
#endif /* PROCESSOR_BUILD */

/* PDC entry used for triggering PDC to keep PD_SNC enabled and SNC powered on */
#if (MAIN_PROCESSOR_BUILD)
__RETAINED_RW static uint32_t prevent_power_down_pdc_entry_index =  HW_PDC_INVALID_LUT_INDEX;
#endif

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

//======================== Helper functions ====================================

/*
 * Check whether the given SNC-SYSCPU shared space handle is defined by the application.
 */
__STATIC_INLINE bool is_app_defined_handle(SNC_SHARED_SPACE_HANDLE handle)
{
#if (HAVE_APP_DEFINED_HANDLES)
        return handle < SNC_SHARED_SPACE_APP_COUNT;
#else
        return false;
#endif
}

/*
 * Check whether the given SNC-SYSCPU shared space handle is valid.
 */
__STATIC_INLINE bool is_valid_handle(SNC_SHARED_SPACE_HANDLE handle)
{
        return WITHIN_RANGE(handle, SNC_SHARED_SPACE_PREDEFINED_START, SNC_SHARED_SPACE_HANDLE_MAX);
}

/*
 * Set the address of the memory area associated to an application SNC-SYSCPU shared space handle.
 */
__STATIC_INLINE void set_app_info(SNC_SHARED_SPACE_HANDLE handle, uintptr_t addr)
{
#if (HAVE_APP_DEFINED_HANDLES)
        snc_shared_space_info_ptr->app_info[handle] = addr;
#endif
}

/*
 * Get the address of memory area associated to an application SNC-SYSCPU shared space handle
 */
__STATIC_INLINE uintptr_t get_app_info(SNC_SHARED_SPACE_HANDLE handle)
{
#if (HAVE_APP_DEFINED_HANDLES)
        return snc_shared_space_info_ptr->app_info[handle];
#else
        return 0;
#endif
}

/*
 * Set the address of the memory area associated to a system SNC-SYSCPU shared space handle.
 */
__STATIC_INLINE void set_sys_info(SNC_SHARED_SPACE_HANDLE handle, uintptr_t addr)
{
        ASSERT_ERROR(is_valid_handle(handle));

        snc_shared_space_info_ptr->sys_info[handle - SNC_SHARED_SPACE_PREDEFINED_START] = addr;
}

/*
 * Get the address of memory area associated to an system SNC-SYSCPU shared space handle
 */
__STATIC_INLINE uintptr_t get_sys_info(SNC_SHARED_SPACE_HANDLE handle)
{
        ASSERT_ERROR(is_valid_handle(handle));

        return snc_shared_space_info_ptr->sys_info[handle - SNC_SHARED_SPACE_PREDEFINED_START];
}

/*
 * Check whether the given pointer is valid.
 */
__STATIC_INLINE bool is_valid_shared_space_pointer(const void *p)
{
        uintptr_t addr = (uintptr_t)p;
        return (addr >= SNC_SHARED_SPACE_BASE_ADDRESS)
                && (addr < (SNC_SHARED_SPACE_BASE_ADDRESS + SHARED_RAM_SIZE));
}

/*
 * Convert pointer to the corresponding address in the SNC address space.
 */
__STATIC_INLINE uintptr_t normalize_pointer(const void *p)
{
#if (MAIN_PROCESSOR_BUILD)
        return SNC_CONVERT_SYS2SNC_SHARED_RAM_ADDR(p);
#elif (SNC_PROCESSOR_BUILD)
        return (uintptr_t)p;
#endif
}

/*
 * Covert address to the corresponding address in the address space of the current processor.
 */
__STATIC_INLINE uintptr_t denormalize_addr(uintptr_t addr)
{
#if (MAIN_PROCESSOR_BUILD)
        return SNC_CONVERT_SNC2SYS_SHARED_RAM_ADDR(addr);
#elif (SNC_PROCESSOR_BUILD)
        return addr;
#endif
}

#if (MAIN_PROCESSOR_BUILD)
/*
 * Copy SNC firmware to the SNC code/data RAM starting address.
 */
static void copy_snc_firmware(void)
{
        static void * const snc_fw = (void *)SNC_CODE_DATA_RAM_START_MAIN_PROC;
        const snc_fw_image_header_t *image_ptr = (snc_fw_image_header_t *)snc_fw_area;

        memmove(snc_fw, &image_ptr->data, image_ptr->size);
}
#endif

//==================== Initialization function =================================

void snc_init(void)
{
#if (SNC_PROCESSOR_BUILD)
        /* Indicate that SNC has finished start-up process. */
        snc_shared_space_info_ptr->snc_is_ready = true;
#elif (MAIN_PROCESSOR_BUILD)
        /* Copy SNC firmware image. */
        copy_snc_firmware();
#endif /* PROCESSOR_BUILD */
}

//==================== Control functions =======================================

void snc_start(void)
{
#if (MAIN_PROCESSOR_BUILD)
        uint32_t snc_pdc_entry = snc_get_prevent_power_down_pdc_entry_index();
        if (snc_pdc_entry != HW_PDC_INVALID_LUT_INDEX) {
                hw_pdc_set_pending(snc_pdc_entry);
        } else {
                /* There should be an SNC start up PDC entry */
                ASSERT_WARNING(0);
        }

        /* Enable clock and release SNC from the reset state. */
        REG_SETF(CRG_TOP, CLK_SNC_CTRL_REG, SNC_CLK_ENABLE, 1);
        REG_SETF(CRG_TOP, CLK_SNC_CTRL_REG, SNC_RESET_REQ, 0);
#endif /* MAIN_PROCESSOR_BUILD */
}

void snc_freeze(void)
{
#if (MAIN_PROCESSOR_BUILD)
        REG_SETF(CRG_TOP, CLK_SNC_CTRL_REG, SNC_CLK_ENABLE, 0);
#endif /* MAIN_PROCESSOR_BUILD */
}

void snc_reset(void)
{
#if (MAIN_PROCESSOR_BUILD)
        /* Temporarily disable SNC, to avoid spurious interrupts. */
        REG_SETF(CRG_TOP, CLK_SNC_CTRL_REG, SNC_CLK_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_SNC_CTRL_REG, SNC_RESET_REQ, 1);

        // Reset SNC debug status
        *(uint32_t*) __SNC_GDB_STATUS = GDB_SNC_NEVER_STARTED;

        snc_shared_space_info_ptr->snc_is_ready = false;
        snc_shared_space_info_ptr->snc_error_val = 0;
        snc_shared_space_info_ptr->snc_error_args = 0;

        set_sys_info(SNC_SHARED_SPACE_EXCEPTION_NMI, 0);
        set_sys_info(SNC_SHARED_SPACE_EXCEPTION_HF, 0);
#endif /* MAIN_PROCESSOR_BUILD */
}

#if (MAIN_PROCESSOR_BUILD)
SNC_STOP_ERROR snc_stop(bool force)
{
        uint32_t pdc_snc_entry = hw_pdc_get_pending_snc();

        // check if there is pending SNC PDC entry. SNC PDC entries are acknowledged in goto_deepsleep() function.
        if ((pdc_snc_entry == 0) || force) {
                /* reset SNC */
                snc_reset();
                REG_SETF(CRG_TOP, CLK_SNC_CTRL_REG, SNC_STATE_RETAINED, 0);

                return SNC_STOP_ERROR_NONE;
        } else {
                // This means that SNC is still active, needs time to go to sleep.
                return SNC_STOP_ERROR_PDC_ENTRY_PENDING;
        }
}
#endif /* MAIN_PROCESSOR_BUILD */

#if (SNC_PROCESSOR_BUILD)
void snc_signal_error(uint16_t err, const void *err_args)
{
        if (err_args != NULL) {
                ASSERT_WARNING(is_valid_shared_space_pointer(err_args));
                snc_shared_space_info_ptr->snc_error_args = normalize_pointer(err_args);
        } else {
                snc_shared_space_info_ptr->snc_error_args = 0;
        }

        snc_shared_space_info_ptr->snc_error_val = err;

        if (err != SNC_ERROR_NONE) {
                /* Notify SYSCPU to indicate SNC error. */
                snc_set_snc2sys_int();
        }
}
#endif /* SNC_PROCESSOR_BUILD */

void *snc_convert_snc2sys_addr(const void *snc_addr)
{
        uintptr_t uintptr = (uintptr_t)snc_addr;

        /*
         * This assertion is here to indicate that the given address does not belong to
         * the SNC shared space.
         */
        ASSERT_ERROR(uintptr >= SHARED_RAM_SNC_BASE_ADDRESS
                        && uintptr < (SHARED_RAM_START_SNC_PROC + SHARED_RAM_SIZE));

        return (void *)SNC_CONVERT_SNC2SYS_SHARED_RAM_ADDR(uintptr);
}

void *snc_convert_sys2snc_addr(const void *sys_addr)
{
        uintptr_t uintptr = (uintptr_t)sys_addr;

        /*
         * This assertion is here to indicate that the given address does not belong to
         * the SNC shared space.
         */
        ASSERT_ERROR(uintptr >= SHARED_RAM_MAIN_BASE_ADDRESS
                        && uintptr < (SHARED_RAM_START_MAIN_PROC + SHARED_RAM_SIZE));

        return (void *)SNC_CONVERT_SYS2SNC_SHARED_RAM_ADDR(uintptr);
}

//==================== Configuration functions =================================

#if (MAIN_PROCESSOR_BUILD)
void snc_register_snc2sys_int(snc_interrupt_cb_t cb)
{
        ASSERT_ERROR(cb != NULL);

        GLOBAL_INT_DISABLE();
        snc_intr_cb = cb;
        NVIC_ClearPendingIRQ(SNC2SYS_IRQn);
        snc_clear_snc2sys_int();
        GLOBAL_INT_RESTORE();
        NVIC_EnableIRQ(SNC2SYS_IRQn);
}

void snc_unregister_snc2sys_int(void)
{
        NVIC_DisableIRQ(SNC2SYS_IRQn);
        NVIC_ClearPendingIRQ(SNC2SYS_IRQn);
        snc_intr_cb = NULL;
}

void snc_set_prevent_power_down_pdc_entry_index(uint32_t value)
{
        prevent_power_down_pdc_entry_index = value;
}

uint32_t snc_get_prevent_power_down_pdc_entry_index(void)
{
        return prevent_power_down_pdc_entry_index;
}
#elif (SNC_PROCESSOR_BUILD)
void snc_register_sys2snc_int(snc_interrupt_cb_t cb)
{
        ASSERT_ERROR(cb != NULL);

        GLOBAL_INT_DISABLE();
        snc_intr_cb = cb;
        NVIC_ClearPendingIRQ(SYS2SNC_IRQn);
        snc_clear_sys2snc_int();
        GLOBAL_INT_RESTORE();
        NVIC_EnableIRQ(SYS2SNC_IRQn);
}

void snc_unregister_sys2snc_int(void)
{
        NVIC_DisableIRQ(SYS2SNC_IRQn);
        NVIC_ClearPendingIRQ(SYS2SNC_IRQn);
        snc_intr_cb = NULL;
}
#endif /* PROCESSOR_BUILD */

void snc_set_shared_space_addr(const void *addr, SNC_SHARED_SPACE_HANDLE handle)
{
        uintptr_t norm_addr;

        if (addr != NULL) {
                ASSERT_WARNING(is_valid_shared_space_pointer(addr));

                /* Store "normalized" addresses. */
                norm_addr = normalize_pointer(addr);
        } else {
                norm_addr = 0;
        }

        if (is_app_defined_handle(handle)) {
                set_app_info(handle, norm_addr);
        } else {
                set_sys_info(handle, norm_addr);
        }
}

void *snc_get_shared_space_addr(SNC_SHARED_SPACE_HANDLE handle)
{
        uintptr_t ret = 0;

        if (is_app_defined_handle(handle)) {
                ret = get_app_info(handle);
        } else {
                ret = get_sys_info(handle);
        }

        if (ret > 0) {
                ret = denormalize_addr(ret);
        }

        return (void *)ret;
}

//==================== State Acquisition functions =============================

bool snc_is_ready(void)
{
        return snc_shared_space_info_ptr->snc_is_ready;
}

SNC_ERROR_STAT snc_get_error(void)
{
        return snc_shared_space_info_ptr->snc_error_val;
}

SNC_CORE_STAT snc_get_core_status(void)
{
        if (snc_is_enabled()) {
                uint32_t snc_stat = SNC->SNC_STATUS_REG;

                if (snc_stat & REG_MSK(SNC, SNC_STATUS_REG, CPU_LOCKED)) {
                        return SNC_CORE_LOCKED;
                }
                if (snc_stat & REG_MSK(SNC, SNC_STATUS_REG, CPU_IDLE)) {
                        return SNC_CORE_IDLE;
                }

                return SNC_CORE_ACTIVE;
        }

        return SNC_CORE_DISABLED;
}

SNC_WDOG_STAT snc_get_wdog_status(void)
{
        uint32_t wdog_stat = SNC->SNC_STATUS_REG;

        if (wdog_stat & REG_MSK(SNC, SNC_STATUS_REG, WDOG_EARLY_NOTICE)) {
                return SNC_WDOG_EARLY_NOTICE;
        }
        if (wdog_stat & REG_MSK(SNC, SNC_STATUS_REG, WDOG_HAS_EXPIRED)) {
                return SNC_WDOG_EXPIRED;
        }

        return SNC_WDOG_NO_NOTICE;
}

//==================== IRQ Handlers ============================================

#if (MAIN_PROCESSOR_BUILD)

__WEAK void snc_exception_error_handler(SNC_ERROR_STAT err, unsigned long *exception_args)
{
        volatile SNC_ERROR_STAT ex_err = err;
        unsigned long * volatile ex_args = exception_args;

        (void) ex_err;
        (void) ex_args;

        ASSERT_ERROR(0);
}

/*
 * SNC2SYS Interrupt Handler
 */
void SNC2SYS_Handler(void)
{
        SNC_ERROR_STAT err;

        SEGGER_SYSTEMVIEW_ISR_ENTER();

        snc_clear_snc2sys_int();

        err = snc_shared_space_info_ptr->snc_error_val;

        if (err != SNC_ERROR_NONE) {
                uintptr_t ex_args = snc_shared_space_info_ptr->snc_error_args;

                if (ex_args > 0) {
                        ex_args = denormalize_addr(ex_args);
                }

                snc_exception_error_handler(err, (unsigned long *)ex_args);
        }

        if (snc_intr_cb != NULL) {
                snc_intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
#elif (SNC_PROCESSOR_BUILD)
/*
 * SYS2SNC Interrupt Handler
 */
void SYS2SNC_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        snc_clear_sys2snc_int();

        if (snc_intr_cb != NULL) {
                snc_intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
#endif /* PROCESSOR_BUILD */

#endif /* CONFIG_USE_SNC */

