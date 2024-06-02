/**
 ****************************************************************************************
 *
 * @file hw_timer.c
 *
 * @brief Implementation of the Timer, Timer2, Timer3 and Timer4 Low Level Driver.
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_TIMER

#include <stdio.h>
#include <sdk_defs.h>
#include <hw_timer.h>
#include <hw_gpio.h>

#if (dg_configSYSTEMVIEW == 1)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif /* (dg_configSYSTEMVIEW == 1) */

// Timer2 interrupt is not present on SNC
#if MAIN_PROCESSOR_BUILD
#       define HAVE_TIMER2
#endif

// Timers 1 to 5 are in PD_TMR and can be active during sleep. Callback needs to be retained
static hw_timer_handler_cb tmr_intr_cb __RETAINED;
#ifdef HAVE_TIMER2
static hw_timer_handler_cb tmr2_intr_cb __RETAINED;     // Timer2 interrupt is not present in SNC
#endif
static hw_timer_handler_cb tmr3_intr_cb __RETAINED;
static hw_timer_handler_cb tmr4_intr_cb __RETAINED;
static hw_timer_handler_cb tmr5_intr_cb __RETAINED;
// Timer6 is in PD_SNC and can be active during sleep. Callback needs to be retained
static hw_timer_handler_cb tmr6_intr_cb __RETAINED;

// Capture irq is a feature of Timer which can be active during sleep. Callback needs to be retained
static hw_timer_capture_handler_cb tmr_capture_intr_cb __RETAINED;

void hw_timer_init(HW_TIMER_ID id, const timer_config *cfg)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP) == 0);
        hw_timer_disable(id);                        // disable timer

        /* Reset control register, i.e. disable timer */
        TBA(id)->TIMER_CTRL_REG = 0;

        if (id == HW_TIMER) {
                /* Disable NVIC interrupt */
                NVIC_DisableIRQ(TIMER_IRQn);
                tmr_intr_cb = NULL;
                NVIC_ClearPendingIRQ(TIMER_IRQn);
                hw_timer_clear_interrupt(HW_TIMER);
#ifdef HAVE_TIMER2
        } else if (id == HW_TIMER2) {
                NVIC_DisableIRQ(TIMER2_IRQn);
                tmr2_intr_cb = NULL;
                NVIC_ClearPendingIRQ(TIMER2_IRQn);
                hw_timer_clear_interrupt(HW_TIMER2);
#endif
        } else if (id == HW_TIMER3) {
                NVIC_DisableIRQ(TIMER3_IRQn);
                tmr3_intr_cb = NULL;
                NVIC_ClearPendingIRQ(TIMER3_IRQn);
                hw_timer_clear_interrupt(HW_TIMER3);
        } else if (id == HW_TIMER4) {
                NVIC_DisableIRQ(TIMER4_IRQn);
                tmr4_intr_cb = NULL;
                NVIC_ClearPendingIRQ(TIMER4_IRQn);
                hw_timer_clear_interrupt(HW_TIMER4);
        } else if (id == HW_TIMER5) {
                NVIC_DisableIRQ(TIMER5_IRQn);
                tmr5_intr_cb = NULL;
                NVIC_ClearPendingIRQ(TIMER5_IRQn);
                hw_timer_clear_interrupt(HW_TIMER5);
        } else if (id == HW_TIMER6) {
                NVIC_DisableIRQ(TIMER6_IRQn);
                tmr6_intr_cb = NULL;
                NVIC_ClearPendingIRQ(TIMER6_IRQn);
                hw_timer_clear_interrupt(HW_TIMER6);
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }

        hw_timer_configure(id, cfg);
}

void hw_timer_configure(HW_TIMER_ID id, const timer_config *cfg)
{
        if (cfg) {
                hw_timer_set_clk(id, cfg->clk_src);
                hw_timer_set_prescaler(id, cfg->prescaler);
                hw_timer_enable_clk(id);

                if (cfg->mode != HW_TIMER_MODE_EDGE_DETECTION) {
                        // Edge detection mode need to be set after edge detection configuration
                        hw_timer_set_mode(id, cfg->mode);
                }

                if (id == HW_TIMER || id == HW_TIMER4) {
                        hw_timer_set_oneshot_auto_switch(id, cfg->autoswitch_to_counter_mode);
                }

                if (cfg->mode == HW_TIMER_MODE_ONESHOT) {
                        hw_timer_configure_oneshot(id, &cfg->oneshot);
                } else if (cfg->mode == HW_TIMER_MODE_EDGE_DETECTION) {
                        hw_timer_configure_edge_detection(id, &cfg->edge);
                        hw_timer_set_mode(id, cfg->mode);
                } else {
                        hw_timer_configure_timer(id, &cfg->timer);
                }

                if (cfg->pwm.frequency != 0) {
                        hw_timer_configure_pwm(id, &cfg->pwm);
                }
        }
}

void hw_timer_configure_timer(HW_TIMER_ID id, const timer_config_timer_capture *cfg)
{
        hw_timer_set_direction(id, cfg->direction);
        hw_timer_set_reload(id, cfg->reload_val);
        // If timer is set to count down, wait for the value to load
        if (cfg->direction == HW_TIMER_DIR_DOWN) {
                while (hw_timer_get_count(id) != cfg->reload_val);
        }
        hw_timer_set_freerun(id, cfg->free_run);
        if (id == HW_TIMER || id == HW_TIMER4) {
                hw_timer_set_single_event_capture(id, cfg->single_event);
        }
        hw_timer_set_event1_gpio(id, cfg->gpio1);
        hw_timer_set_event1_trigger(id, cfg->trigger1);
        hw_timer_set_event2_gpio(id, cfg->gpio2);
        hw_timer_set_event2_trigger(id, cfg->trigger2);
        if (id == HW_TIMER || id == HW_TIMER4) { // only Timer and Timer4 support 4 capture events
                hw_timer_set_event3_gpio(id, cfg->gpio3);
                hw_timer_set_event3_trigger(id, cfg->trigger3);
                hw_timer_set_event4_gpio(id, cfg->gpio4);
                hw_timer_set_event4_trigger(id, cfg->trigger4);
        }
}

void hw_timer_configure_oneshot(HW_TIMER_ID id, const timer_config_oneshot *cfg)
{
        hw_timer_set_reload(id, cfg->delay);
        hw_timer_set_shot_width(id, cfg->shot_width);
        hw_timer_set_event1_gpio(id, cfg->gpio);
        hw_timer_set_event1_trigger(id, cfg->trigger);
        if (id == HW_TIMER || id == HW_TIMER4) {
                hw_timer_set_oneshot_trigger(id, cfg->mode);
        }
}
void hw_timer_configure_edge_detection(HW_TIMER_ID id, const timer_config_edge_detection *cfg)
{
        hw_timer_set_pulse_counter_gpio(id, cfg->gpio);
        hw_timer_set_pulse_counter_threshold(id, cfg->threshold);
        hw_timer_set_edge_detection_count_on_falling(id, cfg->trigger);
}

__RETAINED_CODE void hw_timer_register_int(const HW_TIMER_ID id, hw_timer_handler_cb handler)
{
        if (id == HW_TIMER) {
                tmr_intr_cb = handler;
                HW_TIMER_REG_SETF(HW_TIMER, TIMER_CTRL_REG, TIM_IRQ_EN, 1);
                NVIC_EnableIRQ(TIMER_IRQn);
#ifdef HAVE_TIMER2
        } else if (id == HW_TIMER2) {
                tmr2_intr_cb = handler;
                HW_TIMER_REG_SETF(HW_TIMER2, TIMER_CTRL_REG, TIM_IRQ_EN, 1);
                NVIC_EnableIRQ(TIMER2_IRQn);
#endif
        } else if (id == HW_TIMER3) {
                tmr3_intr_cb = handler;
                HW_TIMER_REG_SETF(HW_TIMER3, TIMER_CTRL_REG, TIM_IRQ_EN, 1);
                NVIC_EnableIRQ(TIMER3_IRQn);
        } else if (id == HW_TIMER4) {
                tmr4_intr_cb = handler;
                HW_TIMER_REG_SETF(HW_TIMER4, TIMER_CTRL_REG, TIM_IRQ_EN, 1);
                NVIC_EnableIRQ(TIMER4_IRQn);
        } else if (id == HW_TIMER5) {
                tmr5_intr_cb = handler;
                HW_TIMER_REG_SETF(HW_TIMER5, TIMER_CTRL_REG, TIM_IRQ_EN, 1);
                NVIC_EnableIRQ(TIMER5_IRQn);
        } else if (id == HW_TIMER6) {
                tmr6_intr_cb = handler;
                HW_TIMER_REG_SETF(HW_TIMER6, TIMER_CTRL_REG, TIM_IRQ_EN, 1);
                NVIC_EnableIRQ(TIMER6_IRQn);
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }
}

void hw_timer_register_capture_int(hw_timer_capture_handler_cb handler, uint8_t gpio_mask)
{
        tmr_capture_intr_cb = handler;
        hw_timer_set_gpio_event_int(gpio_mask);
        NVIC_EnableIRQ(CAPTIMER_IRQn);
}

__RETAINED_HOT_CODE void hw_timer_unregister_int(const HW_TIMER_ID id)
{
        if (id == HW_TIMER) {
                tmr_intr_cb = NULL;
                HW_TIMER_REG_SETF(HW_TIMER, TIMER_CTRL_REG, TIM_IRQ_EN, 0);
                NVIC_DisableIRQ(TIMER_IRQn);
#ifdef HAVE_TIMER2
        } else if (id == HW_TIMER2) {
                tmr2_intr_cb = NULL;
                HW_TIMER_REG_SETF(HW_TIMER2, TIMER_CTRL_REG, TIM_IRQ_EN, 0);
                NVIC_DisableIRQ(TIMER2_IRQn);
#endif
        } else if (id == HW_TIMER3) {
                tmr3_intr_cb = NULL;
                HW_TIMER_REG_SETF(HW_TIMER3, TIMER_CTRL_REG, TIM_IRQ_EN, 0);
                NVIC_DisableIRQ(TIMER3_IRQn);
        } else if (id == HW_TIMER4) {
                tmr4_intr_cb = NULL;
                HW_TIMER_REG_SETF(HW_TIMER4, TIMER_CTRL_REG, TIM_IRQ_EN, 0);
                NVIC_DisableIRQ(TIMER4_IRQn);
        } else if (id == HW_TIMER5) {
                tmr5_intr_cb = NULL;
                HW_TIMER_REG_SETF(HW_TIMER5, TIMER_CTRL_REG, TIM_IRQ_EN, 0);
                NVIC_DisableIRQ(TIMER5_IRQn);
        } else if (id == HW_TIMER6) {
                tmr6_intr_cb = NULL;
                HW_TIMER_REG_SETF(HW_TIMER6, TIMER_CTRL_REG, TIM_IRQ_EN, 0);
                NVIC_DisableIRQ(TIMER6_IRQn);
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }
}

void hw_timer_unregister_capture_int(void)
{
        tmr_capture_intr_cb = NULL;
        hw_timer_set_gpio_event_int(0x0);
        NVIC_DisableIRQ(CAPTIMER_IRQn);
}

void Timer_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        TIMER->TIMER_CLEAR_IRQ_REG = 0;
        if (tmr_intr_cb != NULL) {
                tmr_intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#ifdef HAVE_TIMER2
void Timer2_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        TIMER2->TIMER2_CLEAR_IRQ_REG = 0;
        if (tmr2_intr_cb != NULL) {
                tmr2_intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
#endif

void Timer3_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        TIMER3->TIMER3_CLEAR_IRQ_REG = 0;
        if (tmr3_intr_cb != NULL) {
                tmr3_intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void Timer4_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        TIMER4->TIMER4_CLEAR_IRQ_REG = 0;
        if (tmr4_intr_cb != NULL) {
                tmr4_intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void Timer5_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        TIMER5->TIMER5_CLEAR_IRQ_REG = 0;
        if (tmr5_intr_cb != NULL) {
                tmr5_intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void Timer6_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        TIMER6->TIMER6_CLEAR_IRQ_REG = 0;
        if (tmr6_intr_cb != NULL) {
                tmr6_intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}


void CAPTIMER_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();
        uint8_t event = hw_timer_get_gpio_event_pending();
        if (tmr_capture_intr_cb != NULL) {
                tmr_capture_intr_cb(event);
        }
        hw_timer_clear_gpio_event(event);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void hw_timer_configure_pwm(HW_TIMER_ID id, const timer_config_pwm *cfg)
{
        HW_GPIO_FUNC func = HW_GPIO_FUNC_LAST;

        hw_timer_set_pwm_freq(id, cfg->frequency);
        hw_timer_set_pwm_duty_cycle(id, cfg->duty_cycle);

#if dg_configIMAGE_SETUP == DEVELOPMENT_MODE
        if (cfg->port >= HW_GPIO_NUM_PORTS || cfg->pin >= hw_gpio_port_num_pins[cfg->port]) {
                /* invalid port or pin number specified */
                ASSERT_WARNING(0);
        }
#endif // dg_configIMAGE_SETUP

        if (id == HW_TIMER) {
                func = HW_GPIO_FUNC_TIM_PWM;
        } else if (id == HW_TIMER2) {
                func = HW_GPIO_FUNC_TIM2_PWM;
        } else if (id == HW_TIMER3) {
                func = HW_GPIO_FUNC_TIM3_PWM;
        } else if (id == HW_TIMER4) {
                func = HW_GPIO_FUNC_TIM4_PWM;
        } else if (id == HW_TIMER5) {
                func = HW_GPIO_FUNC_TIM5_PWM;
        } else if (id == HW_TIMER6) {
                func = HW_GPIO_FUNC_TIM6_PWM;
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }

        hw_gpio_pad_latch_enable(cfg->port, cfg->pin);
        hw_gpio_set_pin_function(cfg->port, cfg->pin, HW_GPIO_MODE_OUTPUT, func);

        if (cfg->pwm_active_in_sleep) {
                if (id == HW_TIMER) {
                        /* For TIMER, only P0_30 supports PWM during sleep */
                        if (cfg->port == HW_GPIO_PORT_0 && cfg->pin == HW_GPIO_PIN_30) {
                                REG_SET_BIT(CRG_TOP, SLP_MAP_REG, TMR_PWM_SLP_MAP);
                        } else {
                                ASSERT_WARNING(0);
                        }
                } else if (id == HW_TIMER3) {
                        /* For TIMER3, only P1_30 supports PWM during sleep */
                        if (cfg->port == HW_GPIO_PORT_1 && cfg->pin == HW_GPIO_PIN_30) {
                                REG_SET_BIT(CRG_TOP, SLP_MAP_REG, TMR3_PWM_SLP_MAP);
                        } else {
                                ASSERT_WARNING(0);
                        }
                } else if (id == HW_TIMER4) {
                        /* For TIMER4, only P1_31 supports PWM during sleep */
                        if (cfg->port == HW_GPIO_PORT_1 && cfg->pin == HW_GPIO_PIN_31) {
                                REG_SET_BIT(CRG_TOP, SLP_MAP_REG, TMR4_PWM_SLP_MAP);
                        } else {
                                ASSERT_WARNING(0);
                        }
                }
                hw_gpio_pad_latch_disable(cfg->port, cfg->pin);
        } else {
                if (id == HW_TIMER) {
                        REG_CLR_BIT(CRG_TOP, SLP_MAP_REG, TMR_PWM_SLP_MAP);
                } else if (id == HW_TIMER3) {
                        REG_CLR_BIT(CRG_TOP, SLP_MAP_REG, TMR3_PWM_SLP_MAP);
                } else if (id == HW_TIMER4) {
                        REG_CLR_BIT(CRG_TOP, SLP_MAP_REG, TMR4_PWM_SLP_MAP);
                }
        }
}

#endif /* dg_configUSE_HW_TIMER */

