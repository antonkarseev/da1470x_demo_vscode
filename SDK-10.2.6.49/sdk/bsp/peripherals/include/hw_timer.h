/**
 * \addtogroup PLA_DRI_PER_TIMERS
 * \{
 * \addtogroup HW_TIMER Timer 1/2/3/4 Driver
 * \{
 * \brief Timer
 */

/**
 *****************************************************************************************
 *
 * @file hw_timer.h
 *
 * @brief Definition of API for the Timer, Timer2, Timer3 and Timer4 Low Level Driver.
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_TIMER_H_
#define HW_TIMER_H_


#if dg_configUSE_HW_TIMER

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>
#include <hw_gpio.h>


/* Timer Base Address */
#define TBA(id)                         ((TIMER_Type *)id)

/**
 * \brief Definition for the PWM synchronization bitfield
 *
 */
#define PWM_SYNC_TIMER          0b000001
#define PWM_SYNC_TIMER2         0b000010
#define PWM_SYNC_TIMER3         0b000100
#define PWM_SYNC_TIMER6         0b100000
#define PWM_SYNC_TIMER_ALL      0b101111

/**
 * \brief Get the value of a field of a TIMER register.
 *
 * \param [in] id identifies TIMER, TIMER2, TIMER3 or TIMER4
 * \param [in] reg is the register to access
 * \param [in] field is the register field to read
 *
 * \return the value of the register field
 *
 */
#define HW_TIMER_REG_GETF(id, reg, field) \
        ((TBA(id)->reg & (TIMER_##reg##_##field##_Msk)) >> (TIMER_##reg##_##field##_Pos))

/**
 * \brief Set the value of a field of a TIMER register.
 *
 * \param [in] id identifies TIMER, TIMER2, TIMER3 or TIMER4
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] val is the value to write
 *
 */
#define HW_TIMER_REG_SETF(id, reg, field, val) \
        TBA(id)->reg = ((TBA(id)->reg & ~(TIMER_##reg##_##field##_Msk)) | \
        ((TIMER_##reg##_##field##_Msk) & ((val) << (TIMER_##reg##_##field##_Pos))))

/**
 * \brief Maximum value for timer pre-scaler (5bits).
 *
 */
#define TIMER_MAX_PRESCALER_VAL   TIMER_TIMER_SETTINGS_REG_TIM_PRESCALER_Msk

/**
 * \brief Maximum value for timer reload value (24bits).
 *
 */
#define TIMER_MAX_RELOAD_VAL   TIMER_TIMER_SETTINGS_REG_TIM_RELOAD_Msk

/**
 * \brief Maximum value for timer shot phase duration value in oneshot mode (24bits).
 *
 */
#define TIMER_MAX_SHOTWIDTH_VAL   TIMER_TIMER_SHOTWIDTH_REG_TIM_SHOTWIDTH_Msk


/**
 * \brief Maximum value for timer PWM Frequency (16bits).
 *
 */
#define TIMER_MAX_PWM_FREQ_VAL   TIMER_TIMER_PWM_CTRL_REG_TIM_PWM_FREQ_Msk

/**
 * \brief Maximum value for timer PWM duty cycle (16bits).
 *
 */
#define TIMER_MAX_PWM_DC_VAL   TIMER_TIMER_PWM_CTRL_REG_TIM_PWM_DC_Msk

/**
 * \brief Timer id
 *
 */
#define HW_TIMER          ((void *)TIMER_BASE)
#define HW_TIMER2         ((void *)TIMER2_BASE)
#define HW_TIMER3         ((void *)TIMER3_BASE)
#define HW_TIMER4         ((void *)TIMER4_BASE)
#define HW_TIMER5         ((void *)TIMER5_BASE)
#define HW_TIMER6         ((void *)TIMER6_BASE)
typedef void * HW_TIMER_ID;


/**
 * \brief Mode of operation
 *
 * \note PWM is enabled in both modes.
 *
 */
typedef enum {
        HW_TIMER_MODE_TIMER = 0,          /**< timer/capture mode. Supported by all timers */
        HW_TIMER_MODE_ONESHOT = 1,        /**< one-shot mode. */
        HW_TIMER_MODE_EDGE_DETECTION = 2, /**< Edge detection mode */
} HW_TIMER_MODE;

/**
 * \brief Clock source for timer
 *
 */
typedef enum {
        HW_TIMER_CLK_SRC_INT = 0,      /**< Timer uses the low power clock */
        HW_TIMER_CLK_SRC_EXT = 1       /**< Timer uses the DIVN */
} HW_TIMER_CLK_SRC;

/**
 * \brief Counting direction
 *
 */
typedef enum {
        HW_TIMER_DIR_UP = 0,           /**< Timer counts up (counter is incremented) */
        HW_TIMER_DIR_DOWN = 1          /**< Timer counts down (counter is decremented) */
} HW_TIMER_DIR;

/**
 * \brief Type of triggering events
 *
 */
typedef enum {
        HW_TIMER_TRIGGER_RISING = 0,   /**< Event activated rising edge */
        HW_TIMER_TRIGGER_FALLING = 1   /**< Event activated falling edge */
} HW_TIMER_TRIGGER;

/**
 * \brief Type of oneshot triggering events
 *
 */
typedef enum {
        HW_TIMER_ONESHOT_TRIGGER_GPIO = 0,      /**< Event activated by GPIO */
        HW_TIMER_ONESHOT_TRIGGER_REGISTER = 1,  /**< Event activated by register write */
        HW_TIMER_ONESHOT_TRIGGER_BOTH = 2,      /**< Event activated by register write or GPIO */
        HW_TIMER_ONESHOT_TRIGGER_NONE = 3,      /**< Event not activated */
} HW_TIMER_ONESHOT_TRIGGER_MODE;

/**
 * \brief One shot mode phases
 *
 */
typedef enum {
        HW_TIMER_ONESHOT_WAIT = 0,     /**< Wait for the event */
        HW_TIMER_ONESHOT_DELAY = 1,    /**< Delay before started */
        HW_TIMER_ONESHOT_STARTED = 2,  /**< Start shot */
        HW_TIMER_ONESHOT_ACTIVE = 3    /**< Shot is active */
} HW_TIMER_ONESHOT;

/**
 * \brief GPIOs for timer trigger
 *
 * In HW_TIMER_MODE_TIMER mode use to select which GPIO will trigger capture time.
 * In HW_TIMER_MODE_ONESHOT mode use to select which GPIO will trigger the programmable pulse.
 *
 */

typedef enum {
        HW_TIMER_GPIO_GPIO_NONE = 0,          /**< No GPIO */
        HW_TIMER_GPIO_0_0 = 1,      /**< GPIO Port 0 Pin 0 */
        HW_TIMER_GPIO_0_1 = 2,      /**< GPIO Port 0 Pin 1 */
        HW_TIMER_GPIO_0_2 = 3,      /**< GPIO Port 0 Pin 2 */
        HW_TIMER_GPIO_0_3 = 4,      /**< GPIO Port 0 Pin 3 */
        HW_TIMER_GPIO_0_4 = 5,      /**< GPIO Port 0 Pin 4 */
        HW_TIMER_GPIO_0_5 = 6,      /**< GPIO Port 0 Pin 5 */
        HW_TIMER_GPIO_0_6 = 7,      /**< GPIO Port 0 Pin 6 */
        HW_TIMER_GPIO_0_7 = 8,      /**< GPIO Port 0 Pin 7 */
        HW_TIMER_GPIO_0_8 = 9,      /**< GPIO Port 0 Pin 8 */
        HW_TIMER_GPIO_0_9 = 10,     /**< GPIO Port 0 Pin 9 */
        HW_TIMER_GPIO_0_10 = 11,    /**< GPIO Port 0 Pin 10 */
        HW_TIMER_GPIO_0_11 = 12,    /**< GPIO Port 0 Pin 11 */
        HW_TIMER_GPIO_0_12 = 13,    /**< GPIO Port 0 Pin 12 */
        HW_TIMER_GPIO_0_13 = 14,    /**< GPIO Port 0 Pin 13 */
        HW_TIMER_GPIO_1_0 = 15,     /**< GPIO Port 1 Pin 0 */
        HW_TIMER_GPIO_1_1 = 16,     /**< GPIO Port 1 Pin 1 */
        HW_TIMER_GPIO_1_2 = 17,     /**< GPIO Port 1 Pin 2 */
        HW_TIMER_GPIO_1_3 = 18,     /**< GPIO Port 1 Pin 3 */
        HW_TIMER_GPIO_1_4 = 19,     /**< GPIO Port 1 Pin 4 */
        HW_TIMER_GPIO_1_5 = 20,     /**< GPIO Port 1 Pin 5 */
        HW_TIMER_GPIO_1_6 = 21,     /**< GPIO Port 1 Pin 6 */
        HW_TIMER_GPIO_1_7 = 22,     /**< GPIO Port 1 Pin 7 */
        HW_TIMER_GPIO_1_8 = 23,     /**< GPIO Port 1 Pin 8 */
        HW_TIMER_GPIO_1_9 = 24,     /**< GPIO Port 1 Pin 9 */
        HW_TIMER_GPIO_1_10 = 25,    /**< GPIO Port 1 Pin 10 */
        HW_TIMER_GPIO_1_11 = 26,    /**< GPIO Port 1 Pin 11 */
        HW_TIMER_GPIO_1_12 = 27,    /**< GPIO Port 1 Pin 12 */
        HW_TIMER_GPIO_1_13 = 28,    /**< GPIO Port 1 Pin 13 */
        HW_TIMER_GPIO_1_14 = 29,    /**< GPIO Port 1 Pin 14 */
        HW_TIMER_GPIO_1_15 = 30,    /**< GPIO Port 1 Pin 15 */
        HW_TIMER_GPIO_1_16 = 31,    /**< GPIO Port 1 Pin 16 */
        HW_TIMER_GPIO_1_17 = 32,    /**< GPIO Port 1 Pin 17 */
} HW_TIMER_GPIO;

/**
 * \brief GPIO configuration for timer edge detection mode
 *
 */
typedef struct {
        HW_TIMER_GPIO    gpio;          /**< GPIO for capture mode */
        HW_TIMER_TRIGGER trigger;       /**< GPIO capture trigger */
        uint32_t         threshold;     /**< Number of pulses required to fire interrupt */
} timer_config_edge_detection;

/**
 * \brief Timer interrupt callback
 *
 */
typedef void (*hw_timer_handler_cb)(void);

/**
 * \brief Timer capture interrupt callback
 *
 * \param [in] event bitmask of capture time event GPIOs. "1" means capture event occurred on GPIO
 * \parblock
 *         Bit:      |   3   |  2    |  1    |   0   |
 *                   +-------+-------+-------+-------+
 *                   | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                   +-------+-------+-------+-------+
 * \endparblock
 */
typedef void (*hw_timer_capture_handler_cb)(uint8_t gpio_event);

/**
 * \brief Timer configuration for timer/capture mode
 *
 * \sa timer_config
 * \sa hw_timer_configure_timer
 *
 */
typedef struct {
        HW_TIMER_DIR        direction;    /**< counting direction */
        uint32_t            reload_val;   /**< reload value */
        bool                free_run;     /**< free-running mode state */
        bool                single_event; /**< configuration for single event capture mode */

        HW_TIMER_GPIO       gpio1;        /**< 1st GPIO for capture mode */
        HW_TIMER_TRIGGER    trigger1;     /**< 1st GPIO capture trigger */
        HW_TIMER_GPIO       gpio2;        /**< 2nd GPIO for capture mode */
        HW_TIMER_TRIGGER    trigger2;     /**< 2nd GPIO capture trigger */
        HW_TIMER_GPIO       gpio3;        /**< 3rd GPIO for capture mode. Only valid for Timer */
        HW_TIMER_TRIGGER    trigger3;     /**< 3rd GPIO capture trigger. Only valid for Timer  */
        HW_TIMER_GPIO       gpio4;        /**< 4th GPIO for capture mode. Only valid for Timer */
        HW_TIMER_TRIGGER    trigger4;     /**< 4th GPIO capture trigger. Only valid for Timer  */
} timer_config_timer_capture;

/**
 * \brief Timer configuration for oneshot mode
 *
 * \sa timer_config
 * \sa hw_timer_configure_oneshot
 *
 */
typedef struct {
        uint32_t            delay;              /**< delay (ticks) between GPIO event and output pulse */
        uint32_t            shot_width;         /**< width (ticks) of generated pulse */
        HW_TIMER_GPIO       gpio;               /**< GPIO to wait for event */
        HW_TIMER_TRIGGER    trigger;            /**< GPIO trigger */
        HW_TIMER_ONESHOT_TRIGGER_MODE mode;     /**< Oneshot trigger mode */
} timer_config_oneshot;

/**
 * \brief Timer PWM configuration
 *
 * \sa timer_config
 * \sa hw_timer_configure_pwm
 * \sa hw_timer_set_pwm_freq
 * \sa hw_timer_set_pwm_duty_cycle
 */
typedef struct {
        HW_GPIO_PIN     pin;          /**< Defines the pin of the GPIO with PWM function */
        HW_GPIO_PORT    port;         /**< Defines the port of the GPIO with PWM function */
        /**< When true, Timer or Timer2 will keep PWM output on P1_01 or P1_06, respectively, during deep sleep */
        bool            pwm_active_in_sleep;
        /**< Defines the PWM frequency. Timer clock frequency / (frequency + 1) */
        uint16_t        frequency;
        /**< Defines the PWM duty cycle. duty_cycle / ( frequency + 1) */
        uint16_t        duty_cycle;
} timer_config_pwm;

/**
 * \brief Timer configuration
 *
 * Only one of \p timer and \p oneshot configuration can be set since they are stored in the same
 * union (and will overwrite each other). Proper configuration structure is selected depending on
 * timer mode set.
 *
 * \sa timer_config_timer_capture
 * \sa timer_config_oneshot
 * \sa timer_config_pwm
 * \sa hw_timer_configure
 *
 */
typedef struct {
        HW_TIMER_CLK_SRC    clk_src;                    /**< clock source */
        uint8_t             prescaler;                  /**< clock prescaler */
        bool autoswitch_to_counter_mode;                /**< Automatically switch from oneshot mode to counter mode */
        HW_TIMER_MODE       mode;                       /**< timer/capture mode or oneshot mode */
        union {
                timer_config_timer_capture    timer;    /**< configuration for timer/capture mode */
                timer_config_oneshot          oneshot;  /**< configuration for oneshot mode*/
                timer_config_edge_detection   edge;     /**< configuration for edge detection counter mode */
        };
        timer_config_pwm   pwm;                         /**< PWM configuration */
} timer_config;

/**
 * \brief Timer initialization
 *
 * Turn on clock for timer and configure timer. After initialization both timer and its interrupt
 * are disabled. \p cfg can be NULL - no configuration is performed in such case.
 *
 * \param [in] id timer id
 * \param [in] cfg configuration
 *
 */
void hw_timer_init(HW_TIMER_ID id, const timer_config *cfg);

/**
 * \brief Timer configuration
 *
 * Shortcut to call appropriate configuration function. If \p cfg is NULL, this function does
 * nothing except switching timer to selected mode.
 *
 * \param [in] id timer id
 * \param [in] cfg configuration
 *
 */
void hw_timer_configure(HW_TIMER_ID id, const timer_config *cfg);

/**
 * \brief Timer configuration for timer/capture mode
 *
 * Shortcut to call appropriate configuration function. This does not switch timer to timer/capture
 * mode, it should be done separately using hw_timer_set_mode().
 *
 * \param [in] id timer id
 * \param [in] cfg configuration
 *
 * \note This function will enable the timer clock before loading the registers
 *       and it will leave the timer clock enabled
 *
 * \sa hw_timer_enable_clk
 *
 */
void hw_timer_configure_timer(HW_TIMER_ID id, const timer_config_timer_capture *cfg);

/**
 * \brief Timer configuration for oneshot mode
 *
 * Shortcut to call appropriate configuration function. This does not switch timer to oneshot
 * mode, it should be done separately using hw_timer_set_mode().
 *
 * \param [in] id timer id. Valid values for one shot mode are HW_TIMER and HW_TIMER2
 * \param [in] cfg configuration
 *
 */
void hw_timer_configure_oneshot(HW_TIMER_ID id, const timer_config_oneshot *cfg);

/**
 * \brief Timer configuration for edge detection mode
 *
 * Shortcut to call appropriate configuration function. This does not switch timer to edge detection
 * mode, it should be done separately using hw_timer_set_mode().
 *
 * \param [in] id timer id.
 * \param [in] cfg configuration
 *
 */
void hw_timer_configure_edge_detection(HW_TIMER_ID id, const timer_config_edge_detection *cfg);

/**
 * \brief Freeze timer
 *
 * \param [in] id timer id
 */
__STATIC_INLINE void hw_timer_freeze(const HW_TIMER_ID id)
{
        if (id == HW_TIMER) {
                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM_Msk;
        } else if (id == HW_TIMER2) {
                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM2_Msk;
        } else if (id == HW_TIMER3) {
                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM3_Msk;
        } else if (id == HW_TIMER4) {
                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM4_Msk;
        } else if (id == HW_TIMER5) {
                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM5_Msk;
        } else if (id == HW_TIMER6) {
                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SWTIM6_Msk;
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }
}

/**
 * \brief Unfreeze timer
 *
 * \param [in] id timer id
 */
__STATIC_INLINE void hw_timer_unfreeze(const HW_TIMER_ID id)
{
        if (id == HW_TIMER) {
                GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SWTIM_Msk;
        } else if (id == HW_TIMER2) {
                GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SWTIM2_Msk;
        } else if (id == HW_TIMER3) {
                GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SWTIM3_Msk;
        } else if (id == HW_TIMER4) {
                GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SWTIM4_Msk;
        } else if (id == HW_TIMER5) {
                GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SWTIM5_Msk;
        } else if (id == HW_TIMER6) {
                GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_SWTIM6_Msk;
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }
}

/**
 * \brief Check if timer is frozen
 *
 * \param [in] id timer id
 *
 * \return true if it is frozen else false
 *
 */
__STATIC_INLINE bool hw_timer_frozen(const HW_TIMER_ID id)
{
        if (id == HW_TIMER) {
                return (GPREG->SET_FREEZE_REG & GPREG_SET_FREEZE_REG_FRZ_SWTIM_Msk);
        } else if (id == HW_TIMER2) {
                return (GPREG->SET_FREEZE_REG & GPREG_SET_FREEZE_REG_FRZ_SWTIM2_Msk);
        } else if (id == HW_TIMER3) {
                return (GPREG->SET_FREEZE_REG & GPREG_SET_FREEZE_REG_FRZ_SWTIM3_Msk);
        } else if (id == HW_TIMER4) {
                return (GPREG->SET_FREEZE_REG & GPREG_SET_FREEZE_REG_FRZ_SWTIM4_Msk);
        } else if (id == HW_TIMER5) {
                return (GPREG->SET_FREEZE_REG & GPREG_SET_FREEZE_REG_FRZ_SWTIM5_Msk);
        } else if (id == HW_TIMER6) {
                return (GPREG->SET_FREEZE_REG & GPREG_SET_FREEZE_REG_FRZ_SWTIM6_Msk);
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }
}

/**
 * \brief Set clock source of the timer
 *
 * \param [in] id timer id
 * \param [in] clk clock source of the timer, external or internal
 *
 */
__STATIC_INLINE void hw_timer_set_clk(HW_TIMER_ID id, HW_TIMER_CLK_SRC clk)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_SYS_CLK_EN, clk);
}

/**
 * \brief Set timer clock prescaler
 *
 * Actual timer frequency is \p timer_freq = \p freq_clock / (\p value + 1)
 *
 * \param [in] id timer id
 * \param [in] value prescaler. 5 bits long, shall not be greater than 0x1f
 *
 */
__STATIC_INLINE void hw_timer_set_prescaler(HW_TIMER_ID id, uint8_t value)
{
        ASSERT_WARNING(TIMER_MAX_PRESCALER_VAL >= value);
        HW_TIMER_REG_SETF(id, TIMER_SETTINGS_REG, TIM_PRESCALER, value);
        while (HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_TIMER_BUSY));
}

/**
 * \brief Set timer reload value
 *
 * \note This changes the same register value as hw_timer_set_oneshot_delay() since both parameters
 * share the same register (value is interpreted differently depending on timer mode).
 *
 * \param [in] id timer id
 * \param [in] value reload value. 24 bits long, shall not be greater than 0xffffff
 *
 * \sa hw_timer_set_oneshot_delay
 *
 */
__STATIC_FORCEINLINE void hw_timer_set_reload(HW_TIMER_ID id, uint32_t value)
{
        ASSERT_WARNING(TIMER_MAX_RELOAD_VAL >= value);
        HW_TIMER_REG_SETF(id, TIMER_SETTINGS_REG, TIM_RELOAD, value);
        while (HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_TIMER_BUSY));
}

/**
 * \brief Set pulse delay in oneshot mode
 *
 * \note This changes the same register value as hw_timer_set_reload() since both parameters share
 * the same register (value is interpreted differently depending on timer mode).
 *
 * \param [in] id timer id
 * \param [in] delay delay (ticks). 24 bits long, shall not be greater than 0xffffff
 *
 * \sa hw_timer_set_reload
 *
 */
__STATIC_INLINE void hw_timer_set_oneshot_delay(HW_TIMER_ID id, uint32_t delay)
{
        ASSERT_WARNING(TIMER_MAX_RELOAD_VAL >= delay);
        HW_TIMER_REG_SETF(id, TIMER_SETTINGS_REG, TIM_RELOAD, delay);
        while (HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_TIMER_BUSY));
}

/**
 * \brief Set shot width
 *
 * This applies only to one-shot mode.
 *
 * \param [in] id timer id
 * \param [in] duration shot phase duration. 24 bits long, shall not be greater than 0xffffff
 *
 */
__STATIC_INLINE void hw_timer_set_shot_width(HW_TIMER_ID id, uint32_t duration)
{
        ASSERT_WARNING(TIMER_MAX_SHOTWIDTH_VAL >= duration);
        TBA(id)->TIMER_SHOTWIDTH_REG = duration;
        while (HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_TIMER_BUSY));
}

/**
 * \brief Turn on free run mode of the timer
 *
 * This mode is valid only when timer is counting up
 *
 * \param [in] id timer id
 * \param [in] enable if it is '1' timer does not zero when it reaches the reload value.
 */
__STATIC_INLINE void hw_timer_set_freerun(HW_TIMER_ID id, bool enable)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_FREE_RUN_MODE_EN, enable);
}

/**
 * \brief Set a type of the edge which triggers event1
 *
 * \param [in] id timer id
 * \param [in] edge type of edge, rising or falling
 *
 */
__STATIC_INLINE void hw_timer_set_event1_trigger(HW_TIMER_ID id, HW_TIMER_TRIGGER edge)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_IN1_EVENT_FALL_EN, edge);
}

/**
 * \brief Set a type of the edge which triggers event2
 *
 * \param [in] id timer id
 * \param [in] edge type of edge, rising or falling
 *
 */
__STATIC_INLINE void hw_timer_set_event2_trigger(HW_TIMER_ID id, HW_TIMER_TRIGGER edge)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_IN2_EVENT_FALL_EN, edge);
}

/**
 * \brief Set a type of the edge which triggers event3
 *
 * \param [in] id timer id
 * \param [in] edge type of edge, rising or falling
 *
 * \note Valid only for Timer & Timer4
 *
 */
__STATIC_INLINE void hw_timer_set_event3_trigger(HW_TIMER_ID id, HW_TIMER_TRIGGER edge)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_IN3_EVENT_FALL_EN, edge);
}

/**
 * \brief Set a type of the edge which triggers event4
 *
 * \param [in] id timer id
 * \param [in] edge type of edge, rising or falling
 *
 * \note Valid only for Timer & Timer4
 *
 */
__STATIC_INLINE void hw_timer_set_event4_trigger(HW_TIMER_ID id, HW_TIMER_TRIGGER edge)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_IN4_EVENT_FALL_EN, edge);
}

/**
 * \brief Select which time capture event GPIOs will create a capture IRQ
 *
 * \param [in] mask bitmask of capture time event GPIOs. Set "1" to enable capture interrupt on GPIOx event
 * \parblock
 *         Bit:      |   3   |  2    |  1    |   0   |
 *                   +-------+-------+-------+-------+
 *         IRQ_EN    | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                   +-------+-------+-------+-------+
 * \endparblock
 *
 * \note Valid only for Timer4
 *
 */
__STATIC_INLINE void hw_timer_set_gpio_event_int(uint8_t mask)
{
        uint32_t tmp = TIMER4->TIMER4_CTRL_REG;
        REG_SET_FIELD(TIMER4, TIMER4_CTRL_REG, TIM_CAP_GPIO1_IRQ_EN, tmp, (mask & 0x1));
        REG_SET_FIELD(TIMER4, TIMER4_CTRL_REG, TIM_CAP_GPIO2_IRQ_EN, tmp, ((mask & 0x2) >> 1));
        REG_SET_FIELD(TIMER4, TIMER4_CTRL_REG, TIM_CAP_GPIO3_IRQ_EN, tmp, ((mask & 0x4) >> 2));
        REG_SET_FIELD(TIMER4, TIMER4_CTRL_REG, TIM_CAP_GPIO4_IRQ_EN, tmp, ((mask & 0x8) >> 3));
        TIMER4->TIMER4_CTRL_REG = tmp;
}

/**
 * \brief Set a GPIO input which triggers event1
 *
 * \param [in] id timer id
 * \param [in] gpio GPIO input
 *
 */
__STATIC_INLINE void hw_timer_set_event1_gpio(HW_TIMER_ID id, HW_TIMER_GPIO gpio)
{
        TBA(id)->TIMER_GPIO1_CONF_REG = gpio;
}

/**
 * \brief Set a GPIO input which triggers event2
 *
 * \param [in] id timer id
 * \param [in] gpio GPIO input
 *
 */
__STATIC_INLINE void hw_timer_set_event2_gpio(HW_TIMER_ID id, HW_TIMER_GPIO gpio)
{
        TBA(id)->TIMER_GPIO2_CONF_REG = gpio;
}

/**
 * \brief Set a GPIO input which triggers event3
 *
 * \param [in] id timer id
 * \param [in] gpio GPIO input
 *
 * \note Valid only for Timer & Timer4
 *
 */
__STATIC_INLINE void hw_timer_set_event3_gpio(HW_TIMER_ID id, HW_TIMER_GPIO gpio)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        TBA(id)->TIMER_GPIO3_CONF_REG = gpio;
}

/**
 * \brief Set a GPIO input which triggers event4
 *
 * \param [in] id timer id
 * \param [in] gpio GPIO input
 *
 * \note Valid only for Timer & Timer4
 *
 */
__STATIC_INLINE void hw_timer_set_event4_gpio(HW_TIMER_ID id, HW_TIMER_GPIO gpio)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        TBA(id)->TIMER_GPIO4_CONF_REG = gpio;
}

/**
 * \brief Get clock source of the timer
 *
 * \param [in] id timer id
 * \return clock source
 *
 */
__STATIC_INLINE HW_TIMER_CLK_SRC hw_timer_get_clk(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_SYS_CLK_EN);
}

/**
 * \brief Get timer clock prescaler
 *
 * Actual timer frequency is \p timer_freq = \p freq_clock / (\p retval + 1)
 *
 * \param [in] id timer id
 *
 * \return prescaler value
 *
 * \sa hw_timer_get_prescaler_val
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_prescaler(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_SETTINGS_REG, TIM_PRESCALER);
}

/**
 * \brief Get timer reload value
 *
 * \param [in] id timer id
 *
 * \return reload value
 *
 * \sa hw_timer_set_reload
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_reload(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_SETTINGS_REG, TIM_RELOAD);
}

/**
 * \brief Get pulse delay in oneshot mode
 *
 * \param [in] id timer id
 *
 * \return delay (ticks)
 *
 * \sa hw_timer_get_oneshot_delay
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_oneshot_delay(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_SETTINGS_REG, TIM_RELOAD);
}

/**
 * \brief Get shot width
 *
 * This applies only to one-shot mode.
 *
 * \param [in] id timer id
 *
 * \return shot width value
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_shot_width(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_SHOTWIDTH_REG;
}

/**
 * \brief Get free-running mode state
 *
 * \param [in] id timer id
 *
 * \return free-running mode state
 *
 */
__STATIC_INLINE bool hw_timer_get_freerun(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_FREE_RUN_MODE_EN);
}

/**
 * \brief Get a type of the edge which triggers event1
 *
 * \param [in] id timer id
 *
 * \return edge type
 *
 */
__STATIC_INLINE HW_TIMER_TRIGGER hw_timer_get_event1_trigger(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_IN1_EVENT_FALL_EN);
}

/**
 * \brief Get a type of the edge which triggers event2
 *
 * \param [in] id timer id
 *
 * \return edge type
 *
 */
__STATIC_INLINE HW_TIMER_TRIGGER hw_timer_get_event2_trigger(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_IN2_EVENT_FALL_EN);
}

/**
 * \brief Get a type of the edge which triggers event3. Valid only for Timer & Timer4
 *
 * \param [in] id timer id
 *
 * \return edge type
 *
 */
__STATIC_INLINE HW_TIMER_TRIGGER hw_timer_get_event3_trigger(HW_TIMER_ID id)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_IN3_EVENT_FALL_EN);
}

/**
 * \brief Get a type of the edge which triggers event4. Valid only for Timer & Timer4
 *
 * \param [in] id timer id
 *
 * \return edge type
 *
 */
__STATIC_INLINE HW_TIMER_TRIGGER hw_timer_get_event4_trigger(HW_TIMER_ID id)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_IN4_EVENT_FALL_EN);
}

/**
 * \brief Get a GPIO input which triggers event1.
 *
 * \param [in] id timer id
 *
 * \return GPIO input
 *
 */
__STATIC_INLINE HW_TIMER_GPIO hw_timer_get_event1_gpio(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_GPIO1_CONF_REG;
}

/**
 * \brief Get a GPIO input which triggers event2
 *
 * \param [in] id timer id
 *
 * \return GPIO input
 *
 */
__STATIC_INLINE HW_TIMER_GPIO hw_timer_get_event2_gpio(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_GPIO2_CONF_REG;
}

/**
 * \brief Get a GPIO input which triggers event3. Valid only for Timer
 *
 * \return GPIO input
 *
 */
__STATIC_INLINE HW_TIMER_GPIO hw_timer_get_event3_gpio(void)
{
        return TIMER->TIMER_GPIO3_CONF_REG;
}

/**
 * \brief Get a GPIO input which triggers event4. Valid only for Timer
 *
 * \return GPIO input
 *
 */
__STATIC_INLINE HW_TIMER_GPIO hw_timer_get_event4_gpio(void)
{
        return TIMER->TIMER_GPIO4_CONF_REG;
}

/**
 * \brief Get the capture time for event on GPIO1
 *
 * \param [in] id timer id
 *
 * \return time for event on GPIO1
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_capture1(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_CAPTURE_GPIO1_REG;
}

/**
 * \brief Get the capture time for event on GPIO2
 *
 * \param [in] id timer id
 *
 * \return time for event on GPIO2
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_capture2(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_CAPTURE_GPIO2_REG;
}

/**
 * \brief Get the capture time for event on GPIO3. Valid only for Timer & Timer4
 *
 * \param [in] id timer id
 *
 * \return time for event on GPIO3
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_capture3(HW_TIMER_ID id)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        return TBA(id)->TIMER_CAPTURE_GPIO3_REG;
}

/**
 * \brief Get the capture time for event on GPIO4. Valid only for Timer & Timer4
 *
 * \param [in] id timer id
 *
 * \return time for event on GPIO4
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_capture4(HW_TIMER_ID id)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        return TBA(id)->TIMER_CAPTURE_GPIO4_REG;
}

/**
 * \brief Set the direction of timer counting
 *
 * \param [in] id timer id
 *
 * \param [in] dir counting direction of the timer, up or down
 *
 */
__STATIC_INLINE void hw_timer_set_direction(HW_TIMER_ID id, HW_TIMER_DIR dir)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_COUNT_DOWN_EN, dir);
}

/**
 * \brief Set timer mode
 *
 * \param [in] id timer id
 * \param [in] mode timer mode: '1' One shot, '0' counter
 *
 */
__STATIC_INLINE void hw_timer_set_mode(HW_TIMER_ID id, HW_TIMER_MODE mode)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_ONESHOT_MODE_EN, mode);
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_EDGE_DET_CNT_EN, mode >> 1);
}

/**
 * \brief Return timer mode
 *
 * \param [in] id timer id
 *
 */
__STATIC_INLINE HW_TIMER_MODE hw_timer_get_mode(HW_TIMER_ID id)
{
        if (HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_EN) == 0 &&
            HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_EDGE_DET_CNT_EN) == 1){
                return HW_TIMER_MODE_EDGE_DETECTION;
        }
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_ONESHOT_MODE_EN);
}

/**
 * \brief Get the tick count of the timer
 *
 * \param [in] id timer id
 *
 * \return current value of the timer ticks
 *
 * \sa hw_timer_get_prescaler_val
 *
 */
__STATIC_FORCEINLINE uint32_t hw_timer_get_count(HW_TIMER_ID id)
{

        return TBA(id)->TIMER_TIMER_VAL_REG;
}

/**
 * \brief Get the current phase of the one shot mode
 *
 * \param [in] id timer id
 *
 * \return current phase of the one shot mode
 *
 */
__STATIC_INLINE HW_TIMER_ONESHOT hw_timer_get_oneshot_phase(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_ONESHOT_PHASE);
}

/**
 * \brief Get the current state of Event input 1 (IN1)
 *
 * \param [in] id timer id
 *
 * \return current logic level of IN1
 *
 */
__STATIC_INLINE bool hw_timer_get_gpio1_state(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_IN1_STATE);
}

/**
 * \brief Get the time capture event GPIOs pending events.
 *
 * This function can be used to read the pending status of the capture
 * time event GPIOs (GPIO1 to GPIO4)
 *
 * \return event bitmap of GPIOs. When "1" GPIO event is pending.
 * \parblock
 *         Bit:      |   3   |  2    |  1    |   0   |
 *                   +-------+-------+-------+-------+
 *         Event     | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                   +-------+-------+-------+-------+
 * \endparblock
 *
 * \note Only valid for Timer4
 *
 */
__STATIC_INLINE uint8_t hw_timer_get_gpio_event_pending(void)
{
        return ((TIMER4->TIMER4_STATUS_REG &
                (TIMER4_TIMER4_STATUS_REG_TIM_GPIO1_EVENT_PENDING_Msk |
                 TIMER4_TIMER4_STATUS_REG_TIM_GPIO2_EVENT_PENDING_Msk |
                 TIMER4_TIMER4_STATUS_REG_TIM_GPIO3_EVENT_PENDING_Msk |
                 TIMER4_TIMER4_STATUS_REG_TIM_GPIO4_EVENT_PENDING_Msk)) >> 4);
}

/**
 * \brief Get the current state of Event input 2 (IN2)
 *
 * \param [in] id timer id
 *
 * \return current logic level of IN2
 *
 */
__STATIC_INLINE bool hw_timer_get_gpio2_state(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_IN2_STATE);
}

/**
 * \brief Get the current state of Event input 3 (IN3)
 *
 * Only applicable for TIMER & TIMER4
 *
 * \param [in] id timer id
 *
 * \return current logic level of IN3
 *
 */
__STATIC_INLINE bool hw_timer_get_gpio3_state(HW_TIMER_ID id)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_IN3_STATE);
}

/**
 * \brief Get the current state of Event input 4 (IN4)
 *
 * \param [in] id timer id
 *
 * Only applicable for TIMER & TIMER4
 *
 * \return current logic level of IN4
 *
 */
__STATIC_INLINE bool hw_timer_get_gpio4_state(HW_TIMER_ID id)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_IN4_STATE);
}

/**
 * \brief Get the current prescaler counter value
 *
 * This is value of internal counter used for prescaling. It can be used to have finer granularity
 * when reading timer value.
 *
 * For reading current setting of prescaler, see hw_timer_get_prescaler().
 *
 * \param [in] id timer id
 *
 * \return current prescaler counter value
 *
 * \sa hw_timer_get_count
 * \sa hw_timer_get_prescaler
 *
 */
__STATIC_INLINE uint16_t hw_timer_get_prescaler_val(HW_TIMER_ID id)
{
        return TBA(id)->TIMER_PRESCALER_VAL_REG;
}

/**
 * \brief Register an interrupt handler.
 *
 * \param [in] id timer id
 * \param [in] handler function pointer to handler to call when an interrupt occurs
 *
 */
__RETAINED_CODE void hw_timer_register_int(const HW_TIMER_ID id, hw_timer_handler_cb handler);

/**
 * \brief Unregister an interrupt handler
 *
 * \param [in] id timer id
 *
 */
__RETAINED_HOT_CODE void hw_timer_unregister_int(const HW_TIMER_ID id);

/**
 * \brief Register an interrupt handler for GPIO triggered Timer Capture interrupt.
 *
 * \param [in] handler function pointer to handler to call when an interrupt occurs
 * \param [in] gpio_mask bitmask of capture time event GPIOs. Set "1" to enable capture interrupt on GPIOx event
 * \parblock
 *         Bit:      |   3   |  2    |  1    |   0   |
 *                   +-------+-------+-------+-------+
 *         IRQ_EN    | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                   +-------+-------+-------+-------+
 * \endparblock
 *
 */
void hw_timer_register_capture_int(hw_timer_capture_handler_cb handler, uint8_t gpio_mask);

/**
 * \brief Unregister an interrupt handler for GPIO triggered Timer Capture interrupt
 *
 */
void hw_timer_unregister_capture_int(void);

/**
 * \brief Enable the timer
 *
 * \param [in] id timer id
 *
 * \note Assuming the timer clock is enabled, which is done during timer initialisation (hw_timer_init),
 *       the timer will start running immediately after the execution of this function.
 *
 */
__STATIC_INLINE void hw_timer_enable(HW_TIMER_ID id)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_EN, 1);
}

/**
 * \brief Disable the timer
 *
 * \param [in] id timer id
 *
 * \note This function will disable the timer and timer clock
 *
 */
__STATIC_INLINE void hw_timer_disable(HW_TIMER_ID id)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_EN, 0);
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_CLK_EN, 0);

}

/**
 * \brief Enable the timer clock
 *
 * \param [in] id timer id
 *
 */
__STATIC_INLINE void hw_timer_enable_clk(HW_TIMER_ID id)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_CLK_EN, 1);
}

/**
 * \brief Disable the timer clock
 *
 * \param [in] id timer id
 *
 */
__STATIC_INLINE void hw_timer_disable_clk(HW_TIMER_ID id)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_CLK_EN, 0);
}

/**
 * \brief Timer PWM configuration
 *
 * Shortcut to call appropriate configuration function.
 *
 * \param [in] id timer id
 * \param [in] cfg configuration
 *
 * \note PWM will not be enabled if either frequency or duty cycle are 0
 * \note PD COM should be enabled in order to configure PWM output PIN
 *
 */
void hw_timer_configure_pwm(HW_TIMER_ID id, const timer_config_pwm *cfg);

/**
 * \brief Set PWM frequency prescaler
 *
 * Actual PWM frequency is \p pwm_freq = \p timer_freq / (\p value + 1)
 *
 * \param [in] id timer id
 * \param [in] value PWM frequency defined as above
 *
 * \sa hw_timer_set_prescaler
 *
 */
__STATIC_INLINE void hw_timer_set_pwm_freq(HW_TIMER_ID id, uint32_t value)
{
        ASSERT_WARNING(TIMER_MAX_PWM_FREQ_VAL >= value);

        HW_TIMER_REG_SETF(id, TIMER_PWM_CTRL_REG, TIM_PWM_FREQ, value);
        while (HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_PWM_BUSY));
}

/**
 * \brief Set PWM duty cycle
 *
 * Actualy PWM duty cycle is \p pwm_dc = \p value / (\p pwm_freq + 1)
 *
 * \param [in] id timer id
 * \param [in] value PWM duty cycle defined as above
 *
 * \sa hw_timer_set_pwm_freq
 *
 */
__STATIC_INLINE void hw_timer_set_pwm_duty_cycle(HW_TIMER_ID id, uint32_t value)
{
        ASSERT_WARNING(TIMER_MAX_PWM_DC_VAL >= value);
        HW_TIMER_REG_SETF(id, TIMER_PWM_CTRL_REG, TIM_PWM_DC, value);
        while (HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_PWM_BUSY));
}

/**
 * \brief Get PWM frequency
 *
 * Actual PWM frequency is \p pwm_freq = \p timer_freq / (\p retval + 1)
 *
 * \param [in] id timer id
 *
 * \return PWM frequency as defined above
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_pwm_freq(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_PWM_CTRL_REG, TIM_PWM_FREQ);
}

/**
 * \brief Get PWM duty cycle
 *
 * Actualy PWM duty cycle is \p pwm_dc = \p retval / (\p pwm_freq + 1)
 *
 * \param [in] id timer id
 *
 * \return PWM duty cycle as defined above
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_pwm_duty_cycle(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_PWM_CTRL_REG, TIM_PWM_DC);
}

/**
 * \brief Clear capture time GPIO event.
 *
 * \param [in] mask bitmask of capture time event GPIOs. Set "1" to clear event
 * \parblock
 *         Bit:          |   3   |  2    |  1    |   0   |
 *                       +-------+-------+-------+-------+
 *         event GPIO    | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                       +-------+-------+-------+-------+
 * \endparblock
 *
 * \note Only valid for Timer4
 *
 */
__STATIC_INLINE void hw_timer_clear_gpio_event(uint8_t mask)
{
        TIMER4->TIMER4_CLEAR_GPIO_EVENT_REG = mask;
}

/**
 * \brief Clear timer interrupt.
 *
 * Writing any value clears the interrupt
 *
 * \param [in] id timer id
 *
 */
__STATIC_INLINE void hw_timer_clear_interrupt(const HW_TIMER_ID id)
{
        if (id == HW_TIMER) {
                TIMER->TIMER_CLEAR_IRQ_REG = 0;
        } else if (id == HW_TIMER2) {
                TIMER2->TIMER2_CLEAR_IRQ_REG = 0;
        } else if (id == HW_TIMER3) {
                TIMER3->TIMER3_CLEAR_IRQ_REG = 0;
        } else if (id == HW_TIMER4) {
                TIMER4->TIMER4_CLEAR_IRQ_REG = 0;
        } else if (id == HW_TIMER5) {
                TIMER5->TIMER5_CLEAR_IRQ_REG = 0;
        } else if (id == HW_TIMER6) {
                TIMER6->TIMER6_CLEAR_IRQ_REG = 0;
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }
}

/**
 * \brief Set a GPIO input for the pulse counter
 *
 * \param [in] id timer id
 * \param [in] gpio GPIO input
 *
 */
__STATIC_INLINE void hw_timer_set_pulse_counter_gpio(HW_TIMER_ID id, HW_TIMER_GPIO gpio)
{
        HW_TIMER_REG_SETF(id, TIMER_PULSE_CNT_CTRL_REG, PULSE_CNT_GPIO_SEL, gpio);
}

/**
 * \brief Get the GPIO input for the pulse counter
 *
 * \param [in] id timer id
 *
 * \return The GPIO input
 *
 */
__STATIC_INLINE HW_TIMER_GPIO hw_timer_get_pulse_counter_gpio(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_PULSE_CNT_CTRL_REG, PULSE_CNT_GPIO_SEL);
}

/**
 * \brief Select after how many pulses an interrupt is fired for the pulse counter
 *
 * \param [in] id timer id
 * \param [in] threshold number of pulses required to trigger the interrupt
 *
 */
__STATIC_INLINE void hw_timer_set_pulse_counter_threshold(HW_TIMER_ID id, uint32_t threshold)
{
        HW_TIMER_REG_SETF(id, TIMER_PULSE_CNT_CTRL_REG, PULSE_CNT_THRESHOLD, threshold);
}

/**
 * \brief Get the pulse interrupt threshold
 *
 * \param [in] id timer id
 *
 * \return The counter threshold
 *
 */
__STATIC_INLINE uint32_t hw_timer_get_pulse_counter_threshold(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_PULSE_CNT_CTRL_REG, PULSE_CNT_THRESHOLD);
}

/**
 * \brief Get the status bit of the IRQ pulse counter.
 *
 * When the pulse counter reaches the preset threshold value, this bit is 1.
 *
 * \param [in] id       timer id
 *
 * \return True if threshold is reached, otherwise false
 */
__STATIC_INLINE bool hw_timer_get_pulse_threshold_reached(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_IRQ_PULSE_STATUS);
}

/**
 * \brief Clear pulse interrupt.
 *
 * \param [in] id timer id
 *
 */
__STATIC_INLINE void hw_timer_clear_pulse_interrupt(HW_TIMER_ID id)
{
        HW_TIMER_REG_SETF(id, TIMER_CLEAR_IRQ_PULSE_REG, TIM_CLEAR_PULSE_IRQ, 1);
}

/**
 * \brief Set timer oneshot trigger source.
 *
 * Only applicable for TIMER & TIMER4
 *
 * 00: Select external GPIO as the trigger for one shot
 * 01: Select a register write as the trigger of one shot
 * 10: Either of the two triggers one shot
 * 11: None of the two triggers one shot
 *
 * \param [in] id       timer id
 * \param [in] trigger  timer oneshot trigger source
 *
 */
__STATIC_INLINE void hw_timer_set_oneshot_trigger(HW_TIMER_ID id, HW_TIMER_ONESHOT_TRIGGER_MODE trigger)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_ONESHOT_TRIGGER, trigger);
}

/**
 * \brief Set timer oneshot automated switch to counter mode.
 *
 * Applicable only for TIMER & TIMER4
 *
 * false: No automated switch from OneShot to Counter mode
 * true : Automated switch from OneShot to Counter mode and start counting down.
 *          In case no start value has been programmed, an interrupt should be
 *          immediately issued towards the CPU.
 *
 * \param [in] id           timer id
 * \param [in] auto_switch  enable/disable automated switch
 *
 */
__STATIC_INLINE void hw_timer_set_oneshot_auto_switch(HW_TIMER_ID id, bool auto_switch)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_ONESHOT_SWITCH, auto_switch);
}

/**
 * \brief Check if automated switch to counter mode is set for timer
 *
 * Applicable only for TIMER & TIMER4
 *
 * \param [in] id           timer id
 *
 * \return True if automatic switch is enabled, otherwise false
 *
 */
__STATIC_INLINE bool hw_timer_get_oneshot_auto_switch(HW_TIMER_ID id)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_ONESHOT_SWITCH);
}

/**
 * \brief Trigger oneshot
 *
 * Trigger the TIMER oneshot using the register write method.
 * The TIMER oneshot must be configured for register write triggering, or
 * both register write and GPIO triggering.
 *
 * Only applicable for TIMER & TIMER4
 *
 * \param [in] id timer id
 */
__STATIC_INLINE void hw_timer_trigger_oneshot_reg_write(HW_TIMER_ID id)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        HW_TIMER_REG_SETF(id, TIMER_ONESHOT_TRIGGER_REG, TIM_ONESHOT_TRIGGER_SW, 1);
}

/**
 * \brief Select the timers that will produce synchronized PWMs
 *
 *  \param [in] mask_id bitmask of timers with synchronized PWMs. "1" means synchronized PWM
 * \parblock
 *         Bit:      |   5    |   4    |   3    |   2    |   1    |    0   |
 *                   +--------+--------+--------+--------+--------+--------+
 *                   | TIMER6 |        |        | TIMER3 | TIMER2 | TIMER  |
 *                   +--------+--------+--------+--------+--------+--------+
 * \endparblock
 *
 */
__STATIC_INLINE void hw_timer_configure_pwm_sync(uint8_t mask_id)
{
        REG_SETF(TIMER, TIMER_PWM_SYNC_REG, TIMER_SYNC,  (mask_id & PWM_SYNC_TIMER?  1 : 0));
        REG_SETF(TIMER, TIMER_PWM_SYNC_REG, TIMER2_SYNC, (mask_id & PWM_SYNC_TIMER2? 1 : 0));
        REG_SETF(TIMER, TIMER_PWM_SYNC_REG, TIMER3_SYNC, (mask_id & PWM_SYNC_TIMER3? 1 : 0));
        REG_SETF(TIMER, TIMER_PWM_SYNC_REG, TIMER6_SYNC, (mask_id & PWM_SYNC_TIMER6? 1 : 0));
}

/**
 * \brief Start PWM of the timers selected for synchronized PWM generation.
 *
 */
__STATIC_INLINE void hw_timer_pwm_sync_start(void)
{
        HW_TIMER_REG_SETF(TIMER, TIMER_PWM_SYNC_REG, PWM_START, 1);
}

/**
 * \brief Stop PWM of the timers selected for synchronized PWM generation.
 *
 */
__STATIC_INLINE void hw_timer_pwm_sync_stop(void)
{
        HW_TIMER_REG_SETF(TIMER, TIMER_PWM_SYNC_REG, PWM_START, 0);
}

/**
 * \brief Check if PWM of the timers selected for synchronized PWM generation is started
 *
 * \return True if PWM is started, otherwise false
 */
__STATIC_INLINE bool hw_timer_is_pwm_sync_started(void)
{
        return HW_TIMER_REG_GETF(TIMER, TIMER_PWM_SYNC_REG, PWM_START);
}

/**
 * \brief Enable PWM start for the timers selected for synchronized PWM generation.
 *
 * \param [in] enable Enable/Disable start synchronization of the selected timers.
 */
__STATIC_INLINE void hw_timer_pwm_sync_enable(bool enable)
{
        HW_TIMER_REG_SETF(TIMER, TIMER_PWM_SYNC_REG, SYNC_ENABLE, enable);
}

/**
 * \brief Check if PWM start for the timers selected for synchronized PWM generation is enabled
 *
 * \return True if PWM start is enabled, otherwise false
 */
__STATIC_INLINE bool hw_timer_is_pwm_sync_enabled(void)
{
        return HW_TIMER_REG_GETF(TIMER, TIMER_PWM_SYNC_REG, SYNC_ENABLE);
}

/**
 * \brief Select if edge detection should be triggered by falling edges
 *
 * True:  the counter is triggered on a falling edge
 * False: the counter is triggered on a rising edge
 *
 * \param [in] id       timer id
 * \param [in] enable   enable detection on falling edge
 */
__STATIC_INLINE void hw_timer_set_edge_detection_count_on_falling(HW_TIMER_ID id, bool enable)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_EDGE_DET_CNT_FALL_EN, enable);
}

/**
 * \brief Check if edge detection is triggered by falling edges
 *
 * \return True if set to detect falling edge, otherwise false
 */
__STATIC_INLINE bool hw_timer_get_edge_detection_count_on_falling(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_EDGE_DET_CNT_FALL_EN);
}

/**
 * \brief Enable edge detection counter
 *
 * \param [in] id       timer id
 * \param [in] enable   Enable/Disable edge detection counter
 */
__STATIC_INLINE void hw_timer_enable_edge_detection_counter(HW_TIMER_ID id, bool enable)
{
        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_EDGE_DET_CNT_EN, enable);
}

/**
 * \brief Check if edge detection counter is enabled
 *
 * \param [in] id       timer id
 *
 * \return True if edge detection counter is enabled, otherwise false
 */
__STATIC_INLINE bool hw_timer_is_edge_detection_counter_enabled(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_EDGE_DET_CNT_EN);
}

/**
 * \brief Check if the timer clock has been switched to divn clock
 *
 * \param [in] id       timer id
 *
 * \return True if the timer clock is switched to DIVN, otherwise false
 */
__STATIC_INLINE bool hw_timer_is_switched_to_divn(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_SWITCHED_TO_DIVN_CLK);
}

/**
 * \brief Check if the timer PWM is busy
 *
 * Check if the timer is busy with synchronizing PWM_FREQ_REG and PWM_DC_REG.
 * Do not write a new value to these registers when this bit is high.
 *
 * \param [in] id       timer id
 *
 * \return True if the timer PWM is busy, otherwise false
 */
__STATIC_INLINE bool hw_timer_is_pwm_busy(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_PWM_BUSY);
}

/**
 * \brief Check if the timer is busy
 *
 * Check if the timer is busy with synchronizing PRESCALER_REG, RELOAD_REG and SHOTWIDTH_REG.
 * Do not write a new value to these registers when this bit is high.
 *
 * \param [in] id       timer id
 *
 * \return True if the timer is busy, otherwise false
 */
__STATIC_INLINE bool hw_timer_is_timer_busy(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_TIMER_BUSY);
}

/**
 * \brief Check if an interrupt has occurred
 *
 * \param [in] id       timer id
 *
 * \return True if an interrupt has occurred, otherwise false
 */
__STATIC_INLINE bool hw_timer_get_interrupt_status(HW_TIMER_ID id)
{
        return HW_TIMER_REG_GETF(id, TIMER_STATUS_REG, TIM_IRQ_STATUS);
}

/**
 * \brief Enable single capture event mode
 *
 * When enabled, only the first event on GPIO1 will be captured
 *
 * Only applicable for TIMER & TIMER4
 *
 * \param [in] id timer id.
 * \param [in] enable   Enable/Disable single capture event
 *
 */
__STATIC_INLINE void hw_timer_set_single_event_capture(HW_TIMER_ID id, bool enable)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        HW_TIMER_REG_SETF(id, TIMER_CTRL_REG, TIM_SINGLE_EVENT_CAPTURE, enable);
}

/**
 * \brief Return the value of single capture event mode
 *
 * Only applicable for TIMER & TIMER4
 *
 * \return True if single capture event mode is enabled, otherwise false
 *
 */
__STATIC_INLINE bool hw_timer_get_single_event_capture(HW_TIMER_ID id)
{
        ASSERT_WARNING(id == HW_TIMER || id == HW_TIMER4);

        return HW_TIMER_REG_GETF(id, TIMER_CTRL_REG, TIM_SINGLE_EVENT_CAPTURE);
}


#endif /* dg_configUSE_HW_TIMER */


#endif /* HW_TIMER_H_ */
/**
 * \}
 * \}
 */
