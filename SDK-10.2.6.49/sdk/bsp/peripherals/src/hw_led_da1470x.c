/**
 ****************************************************************************************
 *
 * @file hw_led_da1470x.c
 *
 * @brief Implementation of the LED Low Level Driver.
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#include "hw_led.h"

/***********************************************************************************/
/****************************** LOCAL DEFINITIONS **********************************/
/***********************************************************************************/
#define HW_LED_PWM_REG_INTERVAL 0x4

#define LOAD_SEL_Msk            REG_MSK(PWMLED, LED_LOAD_SEL_REG, LED1_LOAD_SEL)
#define LOAD_SEL_Pos            REG_POS(PWMLED, LED_LOAD_SEL_REG, LED1_LOAD_SEL)
#define LOAD_SEL_REG_Length     REG_POS(PWMLED, LED_LOAD_SEL_REG, LED2_LOAD_SEL)

#define START_CYCLE_Pos         REG_POS(PWMLED, LED1_PWM_CONF_REG, PWMLED_START_CYCLE)
#define START_CYCLE_Msk         REG_MSK(PWMLED, LED1_PWM_CONF_REG, PWMLED_START_CYCLE)
#define STOP_CYCLE_Pos          REG_POS(PWMLED, LED1_PWM_CONF_REG, PWMLED_STOP_CYCLE)
#define STOP_CYCLE_Msk          REG_MSK(PWMLED, LED1_PWM_CONF_REG, PWMLED_STOP_CYCLE)

/***********************************************************************************/
/****************************** PWM CONFIGURATION **********************************/
/***********************************************************************************/
void hw_led_pwm_set_duty_cycle(HW_LED_ID led_id, const hw_led_pwm_duty_cycle_t *duty_cycle)
{
        ASSERT_ERROR(duty_cycle);

        ASSERT_WARNING(led_id < HW_LED_ID_MAX);

        ASSERT_WARNING(duty_cycle->hw_led_pwm_start <= (START_CYCLE_Msk >> START_CYCLE_Pos));
        ASSERT_WARNING(duty_cycle->hw_led_pwm_end <= (STOP_CYCLE_Msk >> STOP_CYCLE_Pos));

        volatile uint32_t *reg = REG_GET_ADDR_INDEXED(PWMLED, LED1_PWM_CONF_REG, HW_LED_PWM_REG_INTERVAL, led_id);

        RAW_SET_MASKED(reg, START_CYCLE_Msk, duty_cycle->hw_led_pwm_start << START_CYCLE_Pos);
        RAW_SET_MASKED(reg, STOP_CYCLE_Msk, duty_cycle->hw_led_pwm_end << STOP_CYCLE_Pos);
}

void hw_led_pwm_set_load_sel(HW_LED_ID led_id, uint8_t load_sel)
{
        ASSERT_WARNING(led_id < HW_LED_ID_MAX);
        ASSERT_WARNING(load_sel <= (LOAD_SEL_Msk >> LOAD_SEL_Pos));

        uint32_t mask = LOAD_SEL_Msk;
        uint8_t pos;

        pos = led_id * LOAD_SEL_REG_Length;
        mask = mask << pos;
        REG_SET_MASKED(PWMLED, LED_LOAD_SEL_REG, mask, load_sel << pos);
}

void hw_led_pwm_get_duty_cycle(HW_LED_ID led_id, hw_led_pwm_duty_cycle_t *duty_cycle)
{
        ASSERT_ERROR(duty_cycle);

        ASSERT_WARNING(led_id < HW_LED_ID_MAX);

        uint32_t reg_val = *REG_GET_ADDR_INDEXED(PWMLED, LED1_PWM_CONF_REG, HW_LED_PWM_REG_INTERVAL, led_id);

        duty_cycle->hw_led_pwm_start = ((reg_val & START_CYCLE_Msk) >> START_CYCLE_Pos);
        duty_cycle->hw_led_pwm_end   = ((reg_val & STOP_CYCLE_Msk) >> STOP_CYCLE_Pos);
}

int hw_led_pwm_get_load_sel(HW_LED_ID led_id)
{
        ASSERT_WARNING(led_id < HW_LED_ID_MAX);

        uint32_t mask = LOAD_SEL_Msk;
        uint8_t pos;

        pos = led_id * LOAD_SEL_REG_Length;
        mask = mask << pos;

        return (PWMLED->LED_LOAD_SEL_REG & mask) >> pos;
}

/***********************************************************************************/
/******************************** PWM FREQUENCY ************************************/
/***********************************************************************************/

/***********************************************************************************/
/********************************* PWM CONTROL *************************************/
/***********************************************************************************/

/***********************************************************************************/
/****************************** CURRENT TRIMMING ***********************************/
/***********************************************************************************/
void hd_led_set_current_trim(HW_LED_ID id, uint32_t trim)
{
        switch (id) {
        case HW_LED_ID_LED_1:
                REG_SETF(PWMLED, LED_CURR_TRIM_REG, LED1_CURR_TRIM, trim);
                break;
        case HW_LED_ID_LED_2:
                REG_SETF(PWMLED, LED_CURR_TRIM_REG, LED2_CURR_TRIM, trim);
                break;
        case HW_LED_ID_LED_3:
                REG_SETF(PWMLED, LED_CURR_TRIM_REG, LED3_CURR_TRIM, trim);
                break;
        default:
                break;
        }
}

/***********************************************************************************/
/****************************** HELPER FUNCTIONS ***********************************/
/***********************************************************************************/
void hw_led_pwm_set_duty_cycle_pct_off(HW_LED_ID led_id, uint32_t dc, uint32_t off)
{
        ASSERT_WARNING(led_id < HW_LED_ID_MAX);
        ASSERT_WARNING(dc <= 10000);
        ASSERT_WARNING(off <= 10000);

        hw_led_pwm_duty_cycle_t duty_cycle;
        uint32_t period = hw_led_pwm_get_period();

        if (dc > 10000) {
                dc = 10000;
        }

        if (off > 10000) {
                off = 10000;
        }

        if (dc == 10000) {
                duty_cycle.hw_led_pwm_start = 0;
                duty_cycle.hw_led_pwm_end   = 0;
        } else {
                uint32_t offset = (period * off) / 10000;
                /* uint32_t won't be overflowed here, do not use floating points round */
                uint32_t duration = (1 + period) * dc;
                duration = (duration + 5000) / 10000;

                if (duration == 0 && dc != 0) {
                        duration = 1;
                }

                duty_cycle.hw_led_pwm_start = (uint16_t) offset;
                duty_cycle.hw_led_pwm_end   = (uint16_t)(offset + duration) % (period + 1);
        }
        hw_led_pwm_set_duty_cycle(led_id, &duty_cycle);
}

void hw_led_pwm_set_frequency_hz(uint32_t freq)
{
        ASSERT_WARNING((freq >= 31) && (freq <= 7800));

        uint32_t div, max_div = 1 + (REG_MSK(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PRESCALE) >> REG_POS(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PRESCALE));
        uint32_t per, max_per = 1 + (REG_MSK(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PERIOD) >> REG_POS(PWMLED, LEDS_FREQUENCY_REG, PWM_LEDS_PERIOD));

        div = 1 + HW_LED_CLK_CYCLES / (max_per * freq);
        if (div > max_div) {
                div = max_div;
        }

        per = (HW_LED_CLK_CYCLES / (div * freq));
        if (per > max_per) {
                per = max_per;
        }

        hw_led_pwm_set_period(per - 1);
        hw_led_pwm_set_prescale(div - 1);
}

void hw_led_init(const hw_led_config *conf)
{
        ASSERT_ERROR(conf);

        hw_led_pwm_set_frequency_hz(conf->leds_pwm_frequency);

        for (int i = 0; i < HW_LED_ID_MAX; i++){
                hw_led_pwm_set_duty_cycle_pct_off(i, conf->leds_pwm_duty_cycle[i], conf->leds_pwm_start_cycle[i]);
        }
}

