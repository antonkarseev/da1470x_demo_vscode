/**
 * \addtogroup PLA_DRI_PER_OTHER
 * \{
 * \addtogroup HW_WKUP Wakeup Controller Driver
 * \{
 * \brief Wakeup Controller LLD API
 */

/**
 *****************************************************************************************
 *
 * @file hw_wkup_v2.h
 *
 * @brief Definition of API for the Wakeup Controller Low Level Driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_WKUP_V2_H_
#define HW_WKUP_V2_H_


#if dg_configUSE_HW_WKUP

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>
#include "hw_gpio.h"

#define HW_WKUP_POL_P0_BASE_REG            (volatile uint32_t *)(&WAKEUP->WKUP_POL_P0_REG)
#define HW_WKUP_SELECT_KEY_P0_BASE_REG     (volatile uint32_t *)(&WAKEUP->WKUP_SELECT_P0_REG)
#define HW_WKUP_SELECT_GPIO_P0_BASE_REG    (volatile uint32_t *)(&WAKEUP->WKUP_SEL_GPIO_P0_REG)
#define HW_WKUP_SELECT1_GPIO_P0_BASE_REG    (volatile uint32_t *)(&WAKEUP->WKUP_SEL1_GPIO_P0_REG)

/**
 * \brief Get the mask of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_WKUP_REG_FIELD_MASK(reg, field) \
                (WAKEUP_WKUP_##reg##_REG_##field##_Msk)

/**
 * \brief Get the bit position of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_WKUP_REG_FIELD_POS(reg, field) \
                (WAKEUP_WKUP_##reg##_REG_##field##_Pos)

/**
 * \brief Get the value of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 *
 * \return the value of the register field
 *
 */
#define HW_WKUP_REG_GETF(reg, field) \
                ((WAKEUP->WKUP_##reg##_REG & (WAKEUP_WKUP_##reg##_REG_##field##_Msk)) >> (WAKEUP_WKUP_##reg##_REG_##field##_Pos))

/**
 * \brief Set the value of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] new_val is the value to write
 *
 */
#define HW_WKUP_REG_SETF(reg, field, new_val) \
               WAKEUP->WKUP_##reg##_REG = ((WAKEUP->WKUP_##reg##_REG & ~(WAKEUP_WKUP_##reg##_REG_##field##_Msk)) | \
                        ((WAKEUP_WKUP_##reg##_REG_##field##_Msk) & ((new_val) << (WAKEUP_WKUP_##reg##_REG_##field##_Pos))))

/**
 * \brief key and gpio trigger types
 *
 */
typedef enum {
        HW_WKUP_TRIG_DISABLED,          /**< Disabled*/
        HW_WKUP_TRIG_LEVEL_HI_DEB,      /**< Debounced (KEY), level sensitivity, polarity HIGH trigger */
        HW_WKUP_TRIG_LEVEL_LO_DEB,      /**< Debounced (KEY), level sensitivity, polarity LOW trigger */
        HW_WKUP_TRIG_LEVEL_HI,          /**< Non- debounced (GPIO), level sensitivity, polarity HIGH trigger */
        HW_WKUP_TRIG_LEVEL_LO,          /**< Non- debounced (GPIO), level sensitivity, polarity LOW trigger */
        HW_WKUP_TRIG_EDGE_HI,           /**< Non- debounced (GPIO), edge sensitivity, polarity HIGH trigger */
        HW_WKUP_TRIG_EDGE_LO            /**< Non- debounced (GPIO), edge sensitivity, polarity LOW trigger */
} HW_WKUP_TRIGGER;

/**
 * \brief Wakeup Controller configuration
 *
 *
 */
typedef struct {
        uint8_t debounce;                              /**< debounce time in ms */
        uint32_t pin_wkup_state[HW_GPIO_PORT_MAX];     /**< indicates per GPIO port if the index of a bitmasked pin is '1' that we want to associate a key event to that pin, should be '0' otherwise. */
        uint32_t pin_gpio_state[HW_GPIO_PORT_MAX];     /**< indicates per GPIO port if the index of a bitmasked pin is '1' that we want to associate a gpio event to that pin, should be '0' otherwise. */
        uint32_t pin_trigger[HW_GPIO_PORT_MAX];        /**< pin triggers in each port, bitmasks each bit describes HI (when set to '0') or LOW (when set to '1') trigger of corresponding pin in port*/
        uint32_t gpio_sense[HW_GPIO_PORT_MAX];         /**< gpio sensitivity in each port, 0 means level 1 means edge*/
} wkup_config;

/**
 * \brief Wakeup Controller pin configuration to wake from hibernation
 *
 */
typedef enum {
        HW_WKUP_HIBERN_TRIGGER_NONE = 0,                /**< Resets hibernation pins only vbus can wake from hibernation */
        HW_WKUP_HIBERN_TRIGGER_P0_20 = 0x1 << 0,        /**< If set Pin P0_20 can to used to wake up from hibernation */
        HW_WKUP_HIBERN_TRIGGER_P0_29 = 0x1 << 1,        /**< If set Pin P0_29 can to used to wake up from hibernation */
        HW_WKUP_HIBERN_TRIGGER_P1_04 = 0x1 << 2,        /**< If set Pin P1_04 can to used to wake up from hibernation */
        HW_WKUP_HIBERN_TRIGGER_P0_28 = 0x1 << 3         /**< If set Pin P0_28 can to used to wake up from hibernation */
} HW_WKUP_HIBERN_PIN;

/**
 * \brief Wakeup Controller external pull down hibernation pin configuration
 *
 */
typedef enum {
        HW_WKUP_HIBERN_PD_EN_PIN_NONE = 0,              /**< If set no hibernation pin is connected to external pull down */
        HW_WKUP_HIBERN_PD_EN_PIN_P0_20 = 0x1 << 0,      /**< If set P0_20 pin is connected to external pull down */
        HW_WKUP_HIBERN_PD_EN_PIN_P0_29 = 0x1 << 1,      /**< If set P0_29 pin is connected to external pull down */
        HW_WKUP_HIBERN_PD_EN_PIN_P1_04 = 0x1 << 2,      /**< If set P1_04 pin is connected to external pull down */
        HW_WKUP_HIBERN_PD_EN_PIN_P0_28 = 0x1 << 3       /**< If set P0_28 pin is connected to external pull down */
} HW_WKUP_HIBERN_PD_EN_PIN;

typedef void (*hw_wkup_interrupt_cb)(void);

/**
 * \brief Initialize peripheral
 *
 * Resets Wakeup Controller to initial state, i.e. interrupt is disabled and all pin triggers are
 * disabled.
 *
 * \p cfg can be NULL - no configuration is performed in such case.
 *
 * \param [in] cfg configuration
 *
 */
void hw_wkup_init(const wkup_config *cfg);

/**
 * \brief Configure peripheral
 *
 * Shortcut to call appropriate configuration function. If \p cfg is NULL, this function does
 * nothing.
 *
 * \param [in] cfg configuration
 *
 */
void hw_wkup_configure(const wkup_config *cfg);

/**
 * \brief Register KEY interrupt handler
 *
 * A callback function is registered to be called when an interrupt is generated. Interrupt is
 * automatically enabled after calling this function. Application should reset
 * interrupt in callback function using hw_wkup_reset_key_interrupt(). If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb callback function
 * \param [in] prio the priority of the interrupt
 *
 */
void hw_wkup_register_key_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio);

/**
 * \brief Register GPIO P0 interrupt handler
 *
 * Callback function is called when interrupt is generated. Interrupt is automatically enabled
 * after calling this function. Application should reset
 * interrupt in callback function using hw_wkup_reset_key_interrupt(). If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb callback function
 * \param [in] prio the priority of the interrupt
 *
 */

void hw_wkup_register_gpio_p0_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio);

/**
 * \brief Register GPIO P1 interrupt handler
 *
 * Callback function is called when interrupt is generated. Interrupt is automatically enabled
 * after calling this function. Application should reset
 * interrupt in callback function using hw_wkup_reset_key_interrupt(). If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb callback function
 * \param [in] prio the priority of the interrupt
 *
 */

void hw_wkup_register_gpio_p1_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio);

/**
 * \brief Register GPIO P2 interrupt handler
 *
 * Callback function is called when interrupt is generated. Interrupt is automatically enabled
 * after calling this function. Application should reset
 * interrupt in callback function using hw_wkup_reset_key_interrupt(). If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb callback function
 * \param [in] prio the priority of the interrupt
 *
 */
void hw_wkup_register_gpio_p2_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio);

/**
 * \brief Unregister interrupt handler
 *
 * Interrupt is automatically disabled after calling this function.
 *
 */
void hw_wkup_unregister_interrupts(void);

/**
 * \brief Reset key interrupt
 *
 * \warning Function MUST be called by any user-specified interrupt callback, to clear the interrupt.
 *
 */
__STATIC_INLINE void hw_wkup_reset_key_interrupt(void)
{
        WAKEUP->WKUP_RESET_IRQ_REG = 1;
}
/**
 * \brief Interrupt handler
 *
 */
void hw_wkup_handler(void);

/**
 * \brief Set debounce time
 *
 * Setting debounce time to 0 will disable hardware debouncing. Maximum debounce time is 63ms.
 *
 * \param [in] time_ms debounce time in milliseconds
 *
 */
__STATIC_INLINE void hw_wkup_set_key_debounce_time(uint8_t time_ms)
{
        ASSERT_WARNING(time_ms <= 63);
        HW_WKUP_REG_SETF(CTRL, WKUP_DEB_VALUE, time_ms);
}

/**
 * \brief Get current debounce time
 *
 * \return debounce time in milliseconds
 *
 */
__STATIC_INLINE uint8_t hw_wkup_get_key_debounce_time(void)
{
        return HW_WKUP_REG_GETF(CTRL, WKUP_DEB_VALUE);
}

/**
 * \brief Configure a gpio or key trigger event
 *
 * \param [in] port port number
 * \param [in] pin pin number
 * \param [in] trigger gpio or key trigger setting
 *
 */
void hw_wkup_set_trigger(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_WKUP_TRIGGER trigger);

/**
 * \brief Get gpio or key trigger configuration
 *
 * \param [in] port port number
 * \param [in] pin pin number
 * \return gpio or key trigger settings
 *
 */
__STATIC_INLINE HW_WKUP_TRIGGER hw_wkup_get_trigger(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        uint32_t polarity  = (*(HW_WKUP_POL_P0_BASE_REG + port) & (0x1 << pin)) >> pin;
        uint32_t key_enabled = (*(HW_WKUP_SELECT_KEY_P0_BASE_REG + port) & (0x1 << pin)) >> pin;
        uint32_t gpio_enabled = (*(HW_WKUP_SELECT_GPIO_P0_BASE_REG + port) & (0x1 << pin)) >> pin;

        if (key_enabled) {
                return polarity ? HW_WKUP_TRIG_LEVEL_LO_DEB : HW_WKUP_TRIG_LEVEL_HI_DEB;
        }
        else if (gpio_enabled) {
                if ((*(HW_WKUP_SELECT1_GPIO_P0_BASE_REG + port) & (0x1 << pin)) >> pin) {
                        return polarity ? HW_WKUP_TRIG_EDGE_LO : HW_WKUP_TRIG_EDGE_HI;
                }
                return polarity ? HW_WKUP_TRIG_LEVEL_LO : HW_WKUP_TRIG_LEVEL_HI;
        }
        else {
                return HW_WKUP_TRIG_DISABLED;
        }
}

/**
 * \brief Emulate key hit
 *
 * Simulate Key event wake up trigger in case debounce time is set to 0
 *
 */
__STATIC_INLINE void hw_wkup_emulate_key_hit(void)
{
        HW_WKUP_REG_SETF(CTRL, WKUP_SFT_KEYHIT, 1);
        HW_WKUP_REG_SETF(CTRL, WKUP_SFT_KEYHIT, 0);
}

/**
 * \brief Enable WKUP Key interrupts
 *
 * \note Differs from enabling the IRQs reception on M33 side (NVIC_EnableIRQ)
 *       that takes place during the hw_wkup_register_key_interrupt().
 */
__STATIC_INLINE void hw_wkup_enable_key_irq(void)
{
        HW_WKUP_REG_SETF(CTRL, WKUP_ENABLE_IRQ, 1);
}

/**
 * \brief Disable WKUP interrupts
 *
 */
__STATIC_INLINE void hw_wkup_disable_key_irq(void)
{
        HW_WKUP_REG_SETF(CTRL, WKUP_ENABLE_IRQ, 0);
}

/**
 * \brief Freeze wakeup timer
 *
 */
__STATIC_INLINE void hw_wkup_freeze_key_timer(void)
{
        GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_WKUPTIM_Msk;
}

/**
 * \brief Unfreeze wakeup controller timer
 *
 */
__STATIC_INLINE void hw_wkup_unfreeze_key_timer(void)
{
        GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_WKUPTIM_Msk;
}

/**
 * \brief Get port status on last wake up
 *
 * Meaning of bits in returned bitmask is the same as in hw_wkup_set_trigger().
 *
 * \return port pin event state bitmask
 *
 * \sa hw_wkup_set_trigger
 *
 */
__STATIC_INLINE uint32_t hw_wkup_get_gpio_status(HW_GPIO_PORT port)
{
        switch (port) {
        case HW_GPIO_PORT_0:
                return HW_WKUP_REG_GETF(STATUS_P0, WKUP_STAT_P0);
        case HW_GPIO_PORT_1:
                return HW_WKUP_REG_GETF(STATUS_P1, WKUP_STAT_P1);
        case HW_GPIO_PORT_2:
                return HW_WKUP_REG_GETF(STATUS_P2, WKUP_STAT_P2);
        default:
                ASSERT_WARNING(0);// Invalid argument
                return 0;         // Should never reach here
        }
}

/**
 * \brief Clear latch status
 *
 * \param [in] port port number
 * \param [in] status pin status bitmask
 *
 * \warning Function MUST be called by any user-specified interrupt callback, to clear the interrupt latch status
 *
 * \sa hw_wkup_get_gpio_status
 */

__STATIC_INLINE void hw_wkup_clear_gpio_status(HW_GPIO_PORT port, uint32_t status)
{
        switch (port) {
        case HW_GPIO_PORT_0:
                HW_WKUP_REG_SETF(CLEAR_P0, WKUP_CLEAR_P0, status);
                break;
        case HW_GPIO_PORT_1:
                HW_WKUP_REG_SETF(CLEAR_P1, WKUP_CLEAR_P1, status);
                break;
        case HW_GPIO_PORT_2:
                HW_WKUP_REG_SETF(CLEAR_P2, WKUP_CLEAR_P2, status);
                break;
        default:
                ASSERT_WARNING(0);//Invalid argument
        }
}

/**
 * \brief sets hibernation mode
 *
 * \param [in] active if is true then device is able to go to hibernation, otherwise device cannot go to hibernation
 *
 */
__STATIC_FORCEINLINE void hw_wkup_set_hibernation_mode(bool active)
{
        GLOBAL_INT_DISABLE();
        if (active) {
                REG_SET_BIT(CRG_TOP, WAKEUP_HIBERN_REG, HIBERNATION_ENABLE);
        } else {
                REG_CLR_BIT(CRG_TOP, WAKEUP_HIBERN_REG, HIBERNATION_ENABLE);
        }
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Configures wake up hibernation controller
 *
 * \param [in] pin hibernation pin to be used to wake up system. It is bitmask multiple pins can be used
 * \param [in] pd_enabled pin is connected to external pull down. It is bitmask multiple pull down can be configured
 *
 */
void hw_wkup_configure_hibernation(HW_WKUP_HIBERN_PIN pin, HW_WKUP_HIBERN_PD_EN_PIN pd_enabled);


#endif /* dg_configUSE_HW_WKUP */
#endif /* HW_WKUP_V2_H_ */

/**
 * \}
 * \}
 */
