/**
 * \addtogroup PLA_DRI_PER_TIMERS
 * \{
 * \addtogroup HW_WATCHDOG_TIMER Watchdog Timer Driver
 * \{
 * \brief Watchdog Timer
 */

/**
 ****************************************************************************************
 *
 * @file hw_watchdog.h
 *
 * @brief Definition of API for the Watchdog timer Low Level Driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_WATCHDOG_H_
#define HW_WATCHDOG_H_


#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

#define NMI_MAGIC_NUMBER                0xDEADBEEF

/**
 * \brief Holds the stack contents when an NMI occurs.
 *
 * \details The stack contents are copied at this variable when an NMI occurs. The first position is
 *        marked with a special "flag" (0xDEADBEEF) to indicate that the data that follow are valid.
 */
extern volatile uint32_t nmi_event_data[9];

/**
 * \brief Types of generated states if reload value is 0
 *
 * Generate NMI (non-maskable interrupt) or RST (reset of the system)
 *
 */
typedef enum {
        HW_WDG_RESET_NMI = 0,     /**< Generate NMI if the watchdog reaches 0 and WDOG reset if the counter become less or equal to -16 */
        HW_WDG_RESET_RST = 1      /**< Generate WDOG reset it the counter becomes less or equal than 0 */
} HW_WDG_RESET;

/**
 * \brief Watchdog timer interrupt callback
 *
 * \param [in] hardfault_args pointer to call stack
 *
 */
typedef void (*hw_watchdog_interrupt_cb)(unsigned long *exception_args);

#if (SNC_PROCESSOR_BUILD)
/**
 * \brief Freeze SNC watchdog by continuously setting maximum counter value
 *
 * This macro function is used in SNC context only, internally by the system, in order to prevent
 * SNC watchdog timer from expiring, while a condition holds.
 *
 * \param [in] cond conditional expression
 *
 * \note This function is blocking, continuously setting the maximum SNC watchdog counter value
 * \note Once called, this function disables SNC watchdog control that is performed using
 *       GPREG registers in PS_SYS domain: SET_FREEZE_REG and RESET_FREEZE_REG, till SNC resets
 */
#define FREEZE_SNC_WATCHDOG_WHILE(cond) \
        do { \
                SNC->SNC_WDOG_REG = REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_WRITE_VALID) |  \
                        REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_CNT) |                      \
                        REG_MSK(SNC, SNC_WDOG_REG, SYS2SNC_WDOG_FREEZE_DIS);            \
        } while ((cond))
#endif /* SNC_PROCESSOR_BUILD */

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Freeze the watchdog
 *
 * \return true if operation is allowed, else false
 *
 */
__RETAINED_CODE bool hw_watchdog_freeze(void);

/**
 * \brief Unfreeze the watchdog
 *
 * \return true if operation is allowed, else false
 *
 */
__RETAINED_CODE bool hw_watchdog_unfreeze(void);
#endif /* MAIN_PROCESSOR_BUILD */

/**
 * \brief Check if watchdog is busy writing the watchdog counter
 */
__STATIC_FORCEINLINE bool hw_watchdog_check_write_busy(void)
{
#if (MAIN_PROCESSOR_BUILD)
        return (!!(REG_GETF(SYS_WDOG, WATCHDOG_CTRL_REG, WRITE_BUSY)));
#elif (SNC_PROCESSOR_BUILD)
        /*
         *  There is no WRITE_BUSY field in SNC watchdog register
         *  so this function returns false in case of SNC_PROCCESSOR_BUILD
         */
        return false;
#endif
}

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Enable/disable writing the Watchdog timer reload value.
 * This filter prevents unintentionally setting the watchdog with a SW run-away.
 *
 * \param [in] enable   true = write enable for Watchdog reload value
 *                      false = write disable for Watchdog reload value
 *
 * \sa hw_watchdog_set_pos_val
 * \sa hw_watchdog_set_neg_val
 *
 */
__STATIC_INLINE void hw_watchdog_write_value_ctrl(bool enable)
{
        while (hw_watchdog_check_write_busy());
        if (enable) {
                REG_SETF(SYS_WDOG, WATCHDOG_REG, WDOG_WEN, 0x0);
        }
        else {
                REG_SETF(SYS_WDOG, WATCHDOG_REG, WDOG_WEN, 0xff);
        }
}
#endif /* MAIN_PROCESSOR_BUILD */

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Set positive reload value of the watchdog timer
 *
 * \param [in] value reload value for 13 bits down counter in the PD_AON power domain
 *             which is running on either a 10,24 ms clock or a 20,5 ms clock period
 *             and can operate for 84 sec or 3 minutes (depending on the clock).
 *
 * \sa hw_watchdog_write_value_ctrl
 *
 */
__STATIC_FORCEINLINE void hw_watchdog_set_pos_val(uint16_t value)
{
        uint32_t tmp;

        ASSERT_WARNING(SYS_WDOG_WATCHDOG_REG_WDOG_VAL_Msk >= value); // check if reload value is greater than max allowed value
        ASSERT_WARNING(!REG_GETF(SYS_WDOG, WATCHDOG_REG, WDOG_WEN)); // can not write register if WDOG_WEN is not zero
        tmp = SYS_WDOG->WATCHDOG_REG;
        REG_SET_FIELD(SYS_WDOG, WATCHDOG_REG, WDOG_VAL_NEG, tmp, 0);
        REG_SET_FIELD(SYS_WDOG, WATCHDOG_REG, WDOG_VAL, tmp, value);

        /* Wait until a new WDOG_VAL can be written in the Watchdog timer */
        while (hw_watchdog_check_write_busy());

        /* Write a new WDOG_VAL in the Watchdog timer */
        SYS_WDOG->WATCHDOG_REG = tmp;

}
#elif (SNC_PROCESSOR_BUILD)
/**
 * \brief Set positive reload value of the watchdog timer
 *
 * \param [in] value reload value for 13 bits down counter in the PD_SNC power domain
 *             which is running on a 10,24 ms clock period and can operate for 84 sec.
 *
 * \note If generation of NMI when the counter reaches zero is enabled, then generation
 *       of a reset signal of the system will be enabled and the counter will be auto-loaded
 *       with value 16.
 *
 */
__STATIC_FORCEINLINE void hw_watchdog_set_pos_val(uint16_t value)
{
        uint32_t tmp;

        ASSERT_WARNING(REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_CNT) >= value); // check if reload value is greater than max allowed value

        tmp = SNC->SNC_WDOG_REG;
        REG_SET_FIELD(SNC, SNC_WDOG_REG, SNC_WDOG_CNT, tmp, value);
        SNC->SNC_WDOG_REG = (tmp | REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_WRITE_VALID));
}
#endif /* PROCESSOR_BUILD */

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Set negative reload value of the watchdog timer
 *
 * \param [in] value reload value from 0x1FFF to 0x00
 *
 * \sa hw_watchdog_write_value_ctrl
 *
 */
__STATIC_INLINE void hw_watchdog_set_neg_val(uint16_t value)
{
        uint32_t tmp;

        ASSERT_WARNING(SYS_WDOG_WATCHDOG_REG_WDOG_VAL_Msk >= value); // check if reload value is greater than max allowed value
        ASSERT_WARNING(!REG_GETF(SYS_WDOG, WATCHDOG_REG, WDOG_WEN)); // can not write register if WDOG_WEN is not zero
        tmp = SYS_WDOG->WATCHDOG_REG;
        REG_SET_FIELD(SYS_WDOG, WATCHDOG_REG, WDOG_VAL_NEG, tmp, 1);
        REG_SET_FIELD(SYS_WDOG, WATCHDOG_REG, WDOG_VAL, tmp, value);

        /* Wait until a new WDOG_VAL can be written in the Watchdog timer */
        while (hw_watchdog_check_write_busy());

        /* Write a new WDOG_VAL in the Watchdog timer */
        SYS_WDOG->WATCHDOG_REG = tmp;

}
#endif /* PROCESSOR_BUILD */

/**
 * \brief Get reload value of the watchdog timer
 *
 */
__STATIC_INLINE uint16_t hw_watchdog_get_val(void)
{
#if (MAIN_PROCESSOR_BUILD)
        // The watchdog value cannot be read while watchdog is busy writing a new value
        while (hw_watchdog_check_write_busy());

        return REG_GETF(SYS_WDOG, WATCHDOG_REG, WDOG_VAL);
#elif (SNC_PROCESSOR_BUILD)
        return REG_GETF(SNC, SNC_WDOG_REG, SNC_WDOG_CNT);
#endif /* PROCESSOR_BUILD */
}

/**
 * \brief Generate a reset signal of the system when reload value reaches 0
 *
 */
__STATIC_FORCEINLINE void hw_watchdog_gen_RST(void)
{
#if (MAIN_PROCESSOR_BUILD)
        REG_SET_BIT(SYS_WDOG, WATCHDOG_CTRL_REG, NMI_RST);
#elif (SNC_PROCESSOR_BUILD)
        uint32_t tmp = (SNC->SNC_WDOG_REG | REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_EXPIRE));
        SNC->SNC_WDOG_REG = (tmp | REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_WRITE_VALID));
#endif /* PROCESSOR_BUILD */
}

/**
 * \brief Generate an NMI when reload value reaches 0
 *
 */
__STATIC_FORCEINLINE void hw_watchdog_gen_NMI(void)
{
#if (MAIN_PROCESSOR_BUILD)
        REG_CLR_BIT(SYS_WDOG, WATCHDOG_CTRL_REG, NMI_RST);
#elif (SNC_PROCESSOR_BUILD)
        uint32_t tmp = (SNC->SNC_WDOG_REG & (~REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_EXPIRE)));
        SNC->SNC_WDOG_REG = (tmp | REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_WRITE_VALID));
#endif /* PROCESSOR_BUILD */
}

#if (MAIN_PROCESSOR_BUILD)
/**
 * \brief Enable/disable Watchdog freeze functionality
 *
 * \param [in] enable   true = Watchdog timer can not be frozen when NMI_RST=0.
 *                      false = Watchdog timer can be frozen/resumed when NMI_RST=0
 *
 * \sa hw_watchdog_freeze
 * \sa hw_watchdog_unfreeze
 * \sa hw_watchdog_gen_RST
 *
 */
__STATIC_INLINE void hw_watchdog_freeze_ctrl(bool enable)
{
        if (enable) {
                REG_SET_BIT(SYS_WDOG, WATCHDOG_CTRL_REG, WDOG_FREEZE_EN);
        }
        else {
                REG_CLR_BIT(SYS_WDOG, WATCHDOG_CTRL_REG, WDOG_FREEZE_EN);
        }
}
#elif (SNC_PROCESSOR_BUILD)
/**
 * \brief Disable Watchdog freeze functionality controlled by SYSCPU
 *
 *
 */
__STATIC_INLINE void hw_watchdog_disable_freeze_ctrl(void)
{
        uint32_t tmp = (SNC->SNC_WDOG_REG | REG_MSK(SNC, SNC_WDOG_REG, SYS2SNC_WDOG_FREEZE_DIS));
        SNC->SNC_WDOG_REG = (tmp | REG_MSK(SNC, SNC_WDOG_REG, SNC_WDOG_WRITE_VALID));
}
#endif /* PROCESSOR_BUILD */

/**
 * \brief Register an interrupt handler
 *
 * \param [in] handler function pointer to handler to call when an interrupt occurs
 *
 */
void hw_watchdog_register_int(hw_watchdog_interrupt_cb handler);

/**
 * \brief Unregister an interrupt handler
 *
 */
__RETAINED_CODE void hw_watchdog_unregister_int(void);

/**
 * \brief Handle NMI interrupt.
 *
 * \param [in] hardfault_args pointer to call stack
 *
 */
__RETAINED_CODE void hw_watchdog_handle_int(unsigned long *hardfault_args);

/**
 * \brief Check whether the timer has expired
 *
 * \return true, if the timer has expired, false otherwise
 *
 */
bool hw_watchdog_is_timer_expired(void);

/**
 * \brief Check what is generated when watchdog reaches 0 value
 *
 * If it is NMI (interrupt) or RST (system/wdog reset).
 *
 * \return HW_WDG_RESET_NMI if NMI interrupt is generated, otherwise HW_WDG_RESET_RST
 *
 */
HW_WDG_RESET hw_watchdog_is_irq_or_rst_gen(void);


#endif /* HW_WATCHDOG_H_ */


/**
 * \}
 * \}
 */
