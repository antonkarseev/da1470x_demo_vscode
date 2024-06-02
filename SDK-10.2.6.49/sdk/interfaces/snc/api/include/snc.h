/**
 ****************************************************************************************
 *
 * @file snc.h
 *
 * @brief Sensor Node Controller (SNC) driver header file
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_H_
#define SNC_H_


#ifdef CONFIG_USE_SNC

#include "sdk_defs.h"

/*
 * MACROS
 *****************************************************************************************
 */

/**
 * \brief Number of shared space handles defined by the application
 */
#define SNC_SHARED_SPACE_APP_COUNT              dg_configSNC_SHARED_SPACE_APP_HANDLES

#define SNC_SHARED_SPACE_PREDEFINED_START       128

_Static_assert(SNC_SHARED_SPACE_APP_COUNT < SNC_SHARED_SPACE_PREDEFINED_START,
                "Too many shared space areas have been defined");

/**
 * \brief Attribute to mark data that should go in the SNC shared space
 */
#define __SNC_SHARED                            __RETAINED_SHARED

/**
 * \brief Get the handle to an application-defined shared space area, from its index
 *
 * \param [in] id       the id of the application-defined shared space area
 */
#define SNC_SHARED_SPACE_APP(id)                ((id) & (SNC_SHARED_SPACE_PREDEFINED_START - 1))

#define __SNC_GDB_STATUS                        (0x200000C0)

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Handles for SNC-SYSCPU shared space areas
 */
typedef enum {
#if (SNC_SHARED_SPACE_APP_COUNT > 0)
        /** Number of shared space handles defined by the application */
        SNC_SHARED_SPACE_APP_MAX_HANDLE = SNC_SHARED_SPACE_APP_COUNT - 1,
#endif /* SNC_SHARED_SPACE_APP_LEN */

        /** Handle for NMI exception information */
        SNC_SHARED_SPACE_EXCEPTION_NMI = SNC_SHARED_SPACE_PREDEFINED_START,
        /** Handle for Hard Fault exception information */
        SNC_SHARED_SPACE_EXCEPTION_HF,

        /** Handle for BSR information */
        SNC_SHARED_SPACE_SYS_BSR,

        /** Handle for TCS information */
        SNC_SHARED_SPACE_SYS_TCS,

        /** Handle for LP_CLK information */
        SNC_SHARED_SPACE_SYS_LPCLK,

#if dg_configUSE_MAILBOX
        /** Handle for Mailbox information */
        SNC_SHARED_SPACE_MAILBOX,
#endif /* dg_configUSE_MAILBOX */

#if dg_configUSE_RPMSG_LITE
        /** Handle for RPMsg-Lite base address information */
        SNC_SHARED_SPACE_RPMSG_LITE_BASE_ADDR,

        /** Handle for RPMsg-Lite pending interrupt information */
        SNC_SHARED_SPACE_RPMSG_LITE_ISR_PENDING,
#endif /* dg_configUSE_RPMSG_LITE */

        /** Max. handle for shared space information */
        SNC_SHARED_SPACE_HANDLE_MAX
} SNC_SHARED_SPACE_HANDLE;

/**
 * \brief SNC core status
 */
typedef enum {
        SNC_CORE_DISABLED = 0,                  /**< SNC core is disabled (i.e. in reset state) */
        SNC_CORE_ACTIVE,                        /**< SNC core is active */
        SNC_CORE_LOCKED,                        /**< SNC core is locked-up */
        SNC_CORE_IDLE                           /**< SNC core is in idle mode */
} SNC_CORE_STAT;

/**
 * \brief SNC sleep status
 */
typedef enum {
        GDB_SNC_NEVER_STARTED = SNC_NEVER_STARTED,                              /**< SNC is disabled or SNC debug status is not enabled */
        GDB_SNC_ACTIVE_RESET_HANDLER = SNC_ACTIVE_RESET_HANDLER,                /**< SNC is still active after calling Reset_Handler() */
        GDB_SNC_ACTIVE_AFTER_DEEPSLEEP = SNC_ACTIVE_AFTER_DEEPSLEEP,            /**< SNC is unable to go to sleep after calling goto_deepsleep() */
        GDB_SNC_ACTIVE_WAKUP_FROM_DEEPSLEEP = SNC_ACTIVE_WAKUP_FROM_DEEPSLEEP,  /**< SNC woke up from wakeup_from_deepsleep() */
        GDB_SNC_SLEPT_GOTO_DEEPSLEEP = SNC_SLEPT_GOTO_DEEPSLEEP,                /**< SNC is slept by goto_deepsleep() */
        GDB_SNC_SLEPT_UNINTENDED_WKUP = SNC_SLEPT_UNINTENDED_WKUP               /**< SNC is slept by unintended_wakeup() */
} SNC_SLEEP_STAT;

/**
 * \brief SNC watchdog counter status
 */
typedef enum {
        SNC_WDOG_NO_NOTICE = 0,                 /**< SNC watchdog counter is still greater than 16 */
        SNC_WDOG_EARLY_NOTICE,                  /**< SNC watchdog counter has reached the value 16 */
        SNC_WDOG_EXPIRED                        /**< SNC watchdog counter has expired */
} SNC_WDOG_STAT;

/**
 * \brief SNC exception error
 */
typedef enum {
        SNC_ERROR_NONE          = 0,            /**< No error */
        SNC_ERROR_NMI           = 1,            /**< NMI exception */
        SNC_ERROR_HF            = 2,            /**< Hardfault exception */
        SNC_ERROR_ASSERT        = 3,            /**< Assertion exception */

        SNC_ERROR_MAX           = 0xFFFF        /**< Max. error value */
} SNC_ERROR_STAT;

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief stop SNC errors
 */
typedef enum {
        SNC_STOP_ERROR_NONE = 0,                /**< No error */
        SNC_STOP_ERROR_PDC_ENTRY_PENDING        /**< SNC is still active, pending SNC PDC entry */
} SNC_STOP_ERROR;
#endif /* PROCESSOR_BUILD */

/*
 * DATA TYPE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief SNC interrupt callback
 */
typedef void (*snc_interrupt_cb_t)(void);


/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//===================== Register Read/Write functions ==========================
/**
 * \brief Write a value to an SNC register field
 *
 * \param [in] reg      the SNC register
 * \param [in] field    the SNC register field
 * \param [in] val      value to be written
 *
 * \sa SNC_REG_GETF
 */
#define SNC_REG_SETF(reg, field, val) \
        SNC->reg = ((SNC->reg & ~(SNC_##reg##_##field##_Msk)) | \
        ((SNC_##reg##_##field##_Msk) & ((val) << (SNC_##reg##_##field##_Pos))))

/**
 * \brief Get the value of an SNC register field
 *
 * \param [in] reg      the SNC register
 * \param [in] field    the SNC register field
 *
 * \sa SNC_REG_SETF
 */
#define SNC_REG_GETF(reg, field) \
        ((SNC->reg & (SNC_##reg##_##field##_Msk)) >> (SNC_##reg##_##field##_Pos))

//==================== Initialization function =================================

/**
 * \brief Initialize SNC
 *
 * In SYSCPU context, the SNC firmware is copied to the appropriate memory space,
 * and the shared space environment between SYSCPU and SNC is initiated.
 *
 * In SNC context, SNC system environment is initiated.
 */
void snc_init(void);

//==================== Control functions =======================================

/**
 * \brief Start SNC
 *
 * In SYSCPU context, SNC starts code execution.
 * In SNC context, there is no action.
 */
void snc_start(void);

/**
 * \brief Freeze SNC
 *
 * SNC core clock is disabled. In order to be enabled again, snc_start() must be used in
 * SYSCPU context.
 *
 * \sa snc_start
 */
void snc_freeze(void);

/**
 * \brief Keep SNC in reset state
 *
 * SNC is kept in reset state till snc_start() is called in SYSCPU context.
 *
 * \sa snc_start
 */
void snc_reset(void);

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Gracefully shutdown SNC
 *
 * If SNC is sleeping then SNC is kept in reset state and snc retain state is cleared.
 * If SNC is active error SNC_STOP_ERROR_PDC_ENTRY_PENDING is returned.
 *
 * \param [in] force if is true SNC is stopped while it is active and snc retain state is cleared
 *
 * \sa snc_start
 */
SNC_STOP_ERROR snc_stop(bool force);
#endif

#if (SNC_PROCESSOR_BUILD)
/**
 * \brief Signal SNC error
 *
 * An SNC error is signaled, by notifying SYSCPU
 *
 * \param [in] err      error value (SNC_ERROR_STAT)
 * \param [in] err_args error arguments
 *
 * \note Non-applicable in SYSCPU context
 *
 * \sa SNC_ERROR_STAT
 */
void snc_signal_error(uint16_t err,  const void *err_args);
#endif /* SNC_PROCESSOR_BUILD */

/**
 * \brief Set SNC-to-SYS CPU interrupt
 */
__STATIC_INLINE void snc_set_snc2sys_int(void)
{
        CRG_XTAL->SET_SYS_IRQ_CTRL_REG = REG_MSK(CRG_XTAL, SET_SYS_IRQ_CTRL_REG, SNC2SYS_IRQ_BIT);
}

/**
 * \brief Clear SNC-to-SYS CPU interrupt
 */
__STATIC_INLINE void snc_clear_snc2sys_int(void)
{
        CRG_XTAL->RESET_SYS_IRQ_CTRL_REG = REG_MSK(CRG_XTAL, RESET_SYS_IRQ_CTRL_REG, SNC2SYS_IRQ_BIT);
}

/**
 * \brief Set SYS-to-SNC CPU interrupt
 */
__STATIC_INLINE void snc_set_sys2snc_int(void)
{
        CRG_XTAL->SET_SYS_IRQ_CTRL_REG = REG_MSK(CRG_XTAL, SET_SYS_IRQ_CTRL_REG, SYS2SNC_IRQ_BIT);
}

/**
 * \brief Clear SYS-to-SNC CPU interrupt
 */
__STATIC_INLINE void snc_clear_sys2snc_int(void)
{
        CRG_XTAL->RESET_SYS_IRQ_CTRL_REG = REG_MSK(CRG_XTAL, RESET_SYS_IRQ_CTRL_REG, SYS2SNC_IRQ_BIT);
}

/**
 * \brief Convert address in the SNC address space to the corresponding address in the
 *        SYSCPU address space
 *
 * It checks whether the given address in the SNC address space is a valid address in the SNC
 * shared space, and it converts it to the corresponding address in the SYSCPU
 * address space, accordingly.
 *
 * \param [in] snc_addr memory address in the SNC address space
 *
 * \return corresponding memory address in the SYSCPU address space
 */
void *snc_convert_snc2sys_addr(const void *snc_addr);

/**
 * \brief Convert address in the SYSCPU address space to the corresponding address in the
 *        SNC address space
 *
 * It checks whether the given address in the SYSCPU address space is a valid address in the SNC
 * shared space, and it converts it to the corresponding address in the SNC
 * address space, accordingly.
 *
 * \param [in] sys_addr memory address in the SYSCPU address space
 *
 * \return corresponding memory address in the SNC address space
 */
void *snc_convert_sys2snc_addr(const void *sys_addr);

//==================== Configuration functions =================================

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Register an interrupt handler for SNC2SYS hardware interrupt
 *
 * \param [in] cb       callback function to be registered
 *
 * \note Non-applicable in SNC context
 */
void snc_register_snc2sys_int(snc_interrupt_cb_t cb);

/**
 * \brief Unregister an interrupt handler for SNC2SYS hardware interrupt
 *
 * \note Non-applicable in SNC context
 */
void snc_unregister_snc2sys_int(void);

/**
 * \brief Set PDC entry index triggered by SNC on wake-up
 *
 * It sets the PDC entry index used for triggering PDC when SNC wakes-up, in order to prevent
 * PD_SNC from being powered down till SNC goes to sleep.
 *
 * \param [in] idx      PDC entry index to set
 */
void snc_set_prevent_power_down_pdc_entry_index(uint32_t idx);

/**
 * \brief Get PDC entry index triggered by SNC on wake-up
 *
 * It returns the PDC entry index used for triggering PDC when SNC wakes-up, in order to prevent
 * PD_SNC from being powered down till SNC goes to sleep.
 *
 * \return PDC entry index
 */
uint32_t snc_get_prevent_power_down_pdc_entry_index(void);

#elif (SNC_PROCESSOR_BUILD)
/**
 * \brief Register an interrupt handler for SYS2SNC hardware interrupt
 *
 * \param [in] cb       callback function to be registered
 *
 * \note Non-applicable in SYSCPU context
 */
void snc_register_sys2snc_int(snc_interrupt_cb_t cb);

/**
 * \brief Unregister an interrupt handler for SYS2SNC hardware interrupt
 *
 * \note Non-applicable in SYSCPU context
 */
void snc_unregister_sys2snc_int(void);
#endif /* PROCESSOR_BUILD */

/**
 * \brief Set the address of the memory area associated to an SNC-SYSCPU shared space handle
 *
 * \param [in] addr     base address of the shared space area
 * \param [in] handle   handle to a shared space area
 */
void snc_set_shared_space_addr(const void *addr, SNC_SHARED_SPACE_HANDLE handle);

/**
 * \brief Get address of shared memory area associated to an SNC-SYSCPU shared space handle
 *
 * \param [in] handle   handle to a shared space area
 *
 * \return the base address of the shared space area
 */
void *snc_get_shared_space_addr(SNC_SHARED_SPACE_HANDLE handle);

//==================== State Acquisition functions =============================

/**
 * \brief Check if SNC is enabled
 *
 * It returns true if SNC is enabled (i.e. snc_start() has been called), otherwise it returns
 * false (i.e. snc_freeze() or snc_reset() has been called).
 *
 * \return true  - if SNC is enabled
 *         false - if SNC is disabled
 *
 * \sa snc_start
 * \sa snc_freeze
 * \sa snc_reset
 */
__STATIC_INLINE bool snc_is_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_SNC_CTRL_REG, SNC_CLK_ENABLE)
                && !REG_GETF(CRG_TOP, CLK_SNC_CTRL_REG, SNC_RESET_REQ);
}

/**
 * \brief Check if SNC is freezed
 *
 * It returns true if SNC is freezed (i.e. snc_freeze() has been called).
 *
 * \return true  - if SNC is freezed
 *         false - if SNC is not freezed
 */
__STATIC_INLINE bool snc_is_freezed(void)
{
        return !REG_GETF(CRG_TOP, CLK_SNC_CTRL_REG, SNC_CLK_ENABLE);
}

/**
 * \brief Check if SNC is locked
 *
 * It returns true if SNC is locked-up, indicated by the corresponding LOCKUP output of the
 * processor (e.g. a fault has occurred while executing in HardFault handler).
 *
 * \return true  - if SNC is locked-up
 *         false - if SNC is not locked-up
 */
__STATIC_INLINE bool snc_is_locked(void)
{
        return REG_GETF(SNC, SNC_STATUS_REG, CPU_LOCKED);
}

/**
 * \brief Check if SNC is idle
 *
 * It returns true if SNC is in idle mode, activated when the 'sleeping' bit of the processor is set
 * (i.e. waiting for an interrupt - WFI).
 *
 * \return true  - if SNC is idle
 *         false - if SNC is not idle
 */
__STATIC_INLINE bool snc_is_idle(void)
{
        return REG_GETF(SNC, SNC_STATUS_REG, CPU_IDLE);
}

/**
 * \brief Check if SNC is stopped
 *
 * It returns true if SNC is stopped (halted) (e.g. using the debugger).
 *
 * \return true  - if SNC is stopped/halted
 *         false - if SNC is not stopped (e.g. executing a target program, being idle)
 */
__STATIC_INLINE bool snc_is_stopped(void)
{
        return REG_GETF(SNC, SNC_STATUS_REG, CPU_HALTED);
}

/**
 * \brief Check if SNC is ready
 *
 * It returns true if SNC is ready, after performing correct start-up
 * (i.e Reset handler execution started and finished) and initialization.
 *
 * \return true  - if SNC is ready
 *         false - if SNC is not ready (i.e. an error occurred during start-up or initialization)
 */
bool snc_is_ready(void);

/**
 * \brief Get SNC error
 *
 * It returns non-zero value if an error has occurred in SNC.
 *
 * \return value > 0 indicating the occurred errors (defined in SNC_ERROR_STAT), 0 otherwise
 *
 * \sa SNC_ERROR_STAT
 */
SNC_ERROR_STAT snc_get_error(void);

/**
 * \brief Get SNC core status
 *
 * It returns the status of SNC CPU (i.e. disabled, active, locked and idle) as implied by
 * SNC_CORE_STAT.
 *
 * \return the current SNC core status
 */
SNC_CORE_STAT snc_get_core_status(void);

/**
 * \brief Get SNC sleep status
 *
 * It returns the sleep status of SNC CPU as implied by SNC_SLEEP_STAT.
 *
 * \return the current SNC sleep status
 */
__STATIC_INLINE SNC_SLEEP_STAT snc_get_sleep_status(void)
{
        return (volatile uint32_t)(*((uint32_t* )__SNC_GDB_STATUS));
}

/**
 * \brief Get SNC watchdog counter status
 *
 * It returns the status of SNC watchdog counter while down-counting from the programmed value
 * (i.e. no-notice, early notice and expired) as implied by SNC_WDOG_STAT.
 *
 * \return the current SNC watchdog counter status
 */
SNC_WDOG_STAT snc_get_wdog_status(void);

/**
 * \brief Check if SNC-to-SYSCPU interrupt is pending
 *
 * It returns the status of SNC-to-SYSCPU interrupt.
 *
 * \return true  - if SNC-to-SYSCPU interrupt is pending
 *         false - if SNC-to-SYSCPU interrupt is not pending
 */
__STATIC_INLINE bool snc_is_snc2sys_int_pending(void)
{
        return REG_GETF(CRG_XTAL, SYS_IRQ_CTRL_REG, SNC2SYS_IRQ_BIT);
}

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief SNC exception error hook function
 *
 * This is a hook function that is called when an exception error has occurred in SNC
 * (e.g. hardfault, expired-watchdog). SNC is performing a busy loop, just continuously resetting
 * its watchdog.
 *
 * In its default implementation, the function will issue a breakpoint, leading to a halt, if a
 * debugger is attached to SYSCPU, or a hardfault.
 *
 * \note The application can override the default implementation by providing a non-weak
 *       implementation of this function. In case of a hardfault or an expired-wachdog exception,
 *       as indicated by err argument, exception_args will point to the stored status of SNC core
 *       when the exception occurred, which can be processed accordingly in SYSCPU context
 *       (e.g. logging the information and reset the system)
 *
 * \note Non-applicable in SNC context
 */
void snc_exception_error_handler(SNC_ERROR_STAT err, unsigned long *exception_args);
#endif /* MAIN_PROCESSOR_BUILD */

#endif /* CONFIG_USE_SNC */


#endif /* SNC_H_ */
