/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_LED LED Driver
 * \{
 * \brief LED Controller
 */

/**
 ****************************************************************************************
 *
 * @file hw_led.h
 *
 * @brief Definition of API for the LED Low Level Driver.
 *
 * Copyright (C) 2015-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_LED_H_
#define HW_LED_H_


#include <sdk_defs.h>



/***********************************************************************************/
/****************************** LOCAL DEFINITIONS **********************************/
/***********************************************************************************/
#define HW_LED_CLK_CYCLES       (32 * 1000 * 1000)      // 32 MHz

/**
 * \brief Led masks enum
 *
 */
typedef enum {
        HW_LED_MSK_LED_1    = REG_MSK(PWMLED, LEDS_DRV_CTRL_REG, LED1_EN),
        HW_LED_MSK_LED_2    = REG_MSK(PWMLED, LEDS_DRV_CTRL_REG, LED2_EN),
        HW_LED_MSK_LED_3    = REG_MSK(PWMLED, LEDS_DRV_CTRL_REG, LED3_EN),
        HW_LED_ALL_LED_MASK = (HW_LED_MSK_LED_1  | HW_LED_MSK_LED_2  | HW_LED_MSK_LED_3)
} HW_LED_MSK;

/**
 * \brief Led IDs enum
 *
 */
typedef enum {
        HW_LED_ID_LED_1 = 0,
        HW_LED_ID_LED_2 = 1,
        HW_LED_ID_LED_3 = 2,
        HW_LED_ID_MAX
} HW_LED_ID;

/**
 * \brief Led's PWM duty cycle configuration.
 *
 */
typedef struct {
        uint16_t hw_led_pwm_start; /**< Cycle at which the PWM signal becomes high */
        uint16_t hw_led_pwm_end;   /**< Cycle at which the PWM signal becomes low */
} hw_led_pwm_duty_cycle_t;

/**
 * \brief Initialization parameters for LEDs
 *
 */
typedef struct {
        uint32_t leds_pwm_duty_cycle[HW_LED_ID_MAX];  /**< LEDs PWM duty cycle in % of PWM period */
        uint32_t leds_pwm_start_cycle[HW_LED_ID_MAX]; /**< LEDs PWM start cycle in % of PWM period */
        uint32_t leds_pwm_frequency;                  /**< LEDs PWM frequency in Hz */
} hw_led_config;

/***********************************************************************************/
/********************************* LED DRIVERS *************************************/
/***********************************************************************************/
/**
 * \brief Enable the LED drivers.
 *
 * This function enables the drivers of the LEDs defined in the provided bitmap
 *
 * \param [in] led_mask A mask of all the LEDs that will have their driver state enabled.
 *
 */
__STATIC_INLINE void hw_led_on(HW_LED_MSK led_mask)
{
        REG_SET_MASKED(PWMLED, LEDS_DRV_CTRL_REG, led_mask & HW_LED_ALL_LED_MASK, HW_LED_ALL_LED_MASK);
}

/**
 * \brief Disable the LED drivers.
 *
 * This function disables the drivers of the LEDs defined in the provided bitmap
 *
 * \param [in] led_mask  A mask of all the LEDs that will have their driver state disabled.
 *
 */
__STATIC_INLINE void hw_led_off(HW_LED_MSK led_mask)
{
        REG_SET_MASKED(PWMLED, LEDS_DRV_CTRL_REG, led_mask & HW_LED_ALL_LED_MASK, 0x0);
}

/**
 * \brief Get the state of all LED drivers.
 *
 * This function returns a bitfield with the states of all LED drivers.
 *
 * \return LED driver state bitfield, 1 for enabled, 0 for disabled.
 *
 */
__STATIC_INLINE HW_LED_MSK hw_led_get_states(void)
{
        return (PWMLED->LEDS_DRV_CTRL_REG & HW_LED_ALL_LED_MASK);
}

/***********************************************************************************/
/****************************** PWM CONFIGURATION **********************************/
/***********************************************************************************/
/**
 * \brief Define the start and stop cycles of a LED's PWM duty cycle
 *
 * This function sets the duty cycle of the LED's PWM, using the start and
 * end values defined in the provided struct.
 *
 * \param [in] led_id     The LED to be configured
 * \param [in] duty_cycle Duty cycle configuration
 *
 */

void hw_led_pwm_set_duty_cycle(HW_LED_ID led_id, const hw_led_pwm_duty_cycle_t *duty_cycle);

/**
 * \brief Set the sinking current of a LED
 *
 * This function sets the sinking current level of a given LED to \p load_sel.
 * Subsequently, the sinking current is set accordingly:
 * - sinking current = 2.5 mA + (load_sel * 2.5 mA), max = 20 mA.
 *
 * \param [in] led_id     The LED to be configured
 * \param [in] load_sel   The LED sinking current level
 *
 */
void hw_led_pwm_set_load_sel(HW_LED_ID led_id, uint8_t load_sel);

/**
 * \brief Get the duty cycle of a LED
 *
 * This function provides the duty cycle configuration of a given LED.
 *
 * \param [in] led_id     The LED id.
 * \param [in] duty_cycle pointer to the structure that the duty cycle will be stored in.
 *
 */
void hw_led_pwm_get_duty_cycle(HW_LED_ID led_id, hw_led_pwm_duty_cycle_t *duty_cycle);

/**
 * \brief Get the sinking current of a LED
 *
 * This function returns the sinking current level of a given LED.
 * Knowing the sinking current level, the sinking current may be calculated accordingly:
 * - sinking current = 2.5 mA + (sinking current level * 2.5 mA).
 *
 * \param [in] led_id The LED id.
 *
 * \return The LED sinking current level requested
 *
 */
int hw_led_pwm_get_load_sel(HW_LED_ID led_id);

/***********************************************************************************/
/******************************** PWM FREQUENCY ************************************/
/***********************************************************************************/
/**
 * \brief Sets the PWM frequency period for LEDs 1-3.
 *
 * Sets the frequency period of the PWM signals for all LEDs.
 * The PWM frequency is equal to SYSTEM_CLOCK_FREQUENCY / ((PWM_LEDS_PERIOD + 1) * (PWM_LEDS_PRESCALE + 1)).
 * The PWM period is divided in (PWM_LEDS_PERIOD + 1) cycles.
 * A PWM signal becomes high on the PWMLED_START_CYCLE cycle,
 * and low on the PWMLED_STOP_CYCLE cycle.
 *
 * \param [in] period PWM frequency period
 *
 */
__STATIC_INLINE void hw_led_pwm_set_period(uint16_t period)
{
        ASSERT_WARNING(period <= (REG_MSK(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PERIOD) >> REG_POS(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PERIOD)));

        REG_SETF(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PERIOD, period);
}

/**
 * \brief Returns the PWM frequency period for LEDs 1-3.
 *
 * Returns the frequency period of the PWM signals for all LEDs.
 *
 * \return The PWM frequency period
 *
 */
__STATIC_INLINE uint16_t hw_led_pwm_get_period(void)
{
        return REG_GETF(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PERIOD);
}

/**
 * \brief Sets the PWM frequency prescaler for LEDs 1-3.
 *
 * Sets the frequency prescaler of the PWM signals for all LEDs.
 * This value is used to calculate the division factor for the input clock:
 * Division factor = \p prescale + 1,
 * if \p prescale = 0, frequency / 1,
 * if \p prescale = 1, frequency / 2, etc.
 *
 * \param [in] prescale PWM frequency prescaler
 *
 */
__STATIC_INLINE void hw_led_pwm_set_prescale(uint8_t prescale)
{
        ASSERT_WARNING(prescale <= (REG_MSK(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PRESCALE) >> REG_POS(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PRESCALE)));

        REG_SETF(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PRESCALE, prescale);
}

/**
 * \brief Returns the PWM frequency prescaler for LEDs 1-3.
 *
 * Returns the frequency prescaler of the PWM signals for all LEDs.
 *
 * \return The PWM frequency prescaler
 *
 */
__STATIC_INLINE uint8_t hw_led_pwm_get_prescale(void)
{
        return REG_GETF(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PRESCALE);
}

/***********************************************************************************/
/********************************* PWM CONTROL *************************************/
/***********************************************************************************/
/**
 * \brief Enable the LED PWM engines.
 *
 * This function enables the PWM engines of the LEDs defined in the provided bitmap.
 *
 * \param [in] led_mask A mask of all the LEDs that will have their PWM engine enabled.
 *
 */
__STATIC_INLINE void hw_led_pwm_on(HW_LED_MSK led_mask)
{
        REG_SET_MASKED(PWMLED, LEDS_PWM_CTRL_REG, led_mask & HW_LED_ALL_LED_MASK, HW_LED_ALL_LED_MASK);
}

/**
 * \brief Disable the LED PWM engines.
 *
 * This function disables the PWM engines of the LEDs defined in the provided bitmap.
 *
 * \param [in] led_mask A mask of all the LEDs that will have their PWM engine disabled.
 *
 */
__STATIC_INLINE void hw_led_pwm_off(HW_LED_MSK led_mask)
{
        REG_SET_MASKED(PWMLED, LEDS_PWM_CTRL_REG, led_mask & HW_LED_ALL_LED_MASK, 0x0);
}

/**
 * \brief Get the state of all LED PWM engines.
 *
 * This function returns a bitfield with the states of all LED PWM engines.
 *
 * \return LED PWM engine state bitfield, 1 for enabled, 0 for disabled.
 *
 */
__STATIC_INLINE HW_LED_MSK hw_led_pwm_get_states(void)
{
        return (PWMLED->LEDS_PWM_CTRL_REG & HW_LED_ALL_LED_MASK);
}

/**
 * \brief Pause LED 1-3 PWM engines.
 *
 * PWM engines 1-3 are paused when this bit is set by SW.
 *
 * \param [in] pause True to pause PWMs, false to resume.
 *
 */
__STATIC_INLINE void hw_led_pwm_set_sw_pause(bool pause)
{
        REG_SETF(PWMLED, LEDS_PWM_CTRL_REG, PWM_LEDS_SW_PAUSE, pause);
}

/**
 * \brief Get PWM's SW pause state for LEDs 1-3.
 *
 * This function gets PWM's the SW pause state.
 *
 * \return True:  PWM engines are paused.
 * \return False: PWM engines are not paused.
 *
 */
__STATIC_INLINE bool hw_led_pwm_get_sw_pause(void)
{
        return REG_GETF(PWMLED, LEDS_PWM_CTRL_REG, PWM_LEDS_SW_PAUSE);
}

/**
 * \brief Enable PWM engine HW pause for LEDs 1-3.
 *
 * When this bit is set, PWM engines 1-3 are paused when the radio enable is high.
 *
 * \param [in] pause True to pause the PWMs, false to resume.
 *
 */
__STATIC_INLINE void hw_led_pwm_set_hw_pause(bool pause)
{
        REG_SETF(PWMLED, LEDS_PWM_CTRL_REG, PWM_LEDS_HW_PAUSE_ENABLE, pause);
}

/**
 * \brief Get PWM's HW pause state for LEDs 1-3.
 *
 * This function gets PWM's the HW pause state.
 *
 * \return True:  PWM engines HW pause is enabled.
 * \return False: PWM engines HW pause is disabled.
 *
 */
__STATIC_INLINE bool hw_led_pwm_get_hw_pause(void)
{
        return REG_GETF(PWMLED, LEDS_PWM_CTRL_REG, PWM_LEDS_HW_PAUSE_ENABLE);
}

/***********************************************************************************/
/****************************** CURRENT TRIMMING ***********************************/
/***********************************************************************************/
/**
 * \brief Set Current trimming for a LED Group
 *
 * This function will set the current trimming parameter for all LEDs of a group.
 *
 * \param [in] id       The LED group to be configured
 * \param [in] trim     The trim value.
 *
 */
void hd_led_set_current_trim(HW_LED_ID id, uint32_t trim);

/***********************************************************************************/
/****************************** HELPER FUNCTIONS ***********************************/
/***********************************************************************************/
/**
 * \brief Define the duty cycle of a LED's PWM
 *
 * This function sets the duty cycle of the LED's PWM, using the start and
 * end values defined in the provided struct.
 *
 * \param [in] led_id     The LED to be configured
 * \param [in] dc         The percentage of the PWM's period that the LED PWM will remain high
 *                        The range of the parameter is 0(0.00%) --> 10000(100.00%)
 *                        If the parameter is larger than 10000, it will be truncated to 10000
 * \param [in] off        The offset within the PWM's period that that the signal will become high
 *                        The range of the parameter is 0(0.00%, start of period) --> 10000(100.00% end of period)
 *                        If the parameter is larger than 10000, it will be truncated to 10000
 *
 */
void hw_led_pwm_set_duty_cycle_pct_off(HW_LED_ID led_id, uint32_t dc, uint32_t off);

/**
 * \brief Define the frequency of the LED's PWM
 *
 * This function sets the frequency of the LED's PWM.
 *
 * \param [in] freq The requested frequency in Hz: min = 31 Hz; max = 7.8 kHz.
 *
 */
void hw_led_pwm_set_frequency_hz(uint32_t freq);

/**
 * \brief Initialize the LED PWM engines
 *
 * This function configures the PWM engines of LEDs 1-3. The configuration
 * parameters applied are PWM engine frequency and individual LED duty cycles.
 *
 * \param [in] conf Pointer to the configuration struct
 *
 */
void hw_led_init(const hw_led_config *conf);



#endif /* HW_LED_H_ */

/**
 * \}
 * \}
 */
