/**
 * \addtogroup PLA_DRI_PER_AUDIO
 * \{
 * \addtogroup HW_VAD Voice Auto Detection
 * \{
 * \brief Voice auto-detection unit driver
 */

/**
 ****************************************************************************************
 *
 * @file hw_vad.h
 *
 * @brief Definition of API for the VAD Low Level Driver.
 *
 * Copyright (C) 2020-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 * VAD voice detection algorithm has 2 main parameters:
 * -  Vs: Voice Tracking sensitivity
 * -  Ns: Background Noise Tracking sensitivity
 *
 * For tuning recognition envelope for each usecase the following parameters are considered:
 * - Minimum Delay
 * - Minimum Event Duration
 *
 * The threshold for which a recognition event is detected is determined by parameter:
 * -  Ps: Power Level Sensitivity
 *
 * Noise Floor Information (NFI) is the ambient noise reference level used in the VAD.
 * An IRQ can be generated if a specified ambient noise threshold is surpassed:
 * - NFI Detection threshold.
 *
 * Usage:
 *
 * VAD is configured with the appropriate settings for the current usecase with
 *  hw_vad_init(vad_cfg) or hw_vad_configure(vad_cfg)
 *
 * VAD can operate in 3 modes:
 *  1.VAD_MODE_STANDBY
 *  2.VAD_MODE_ALWAYS_LISTENING
 *  3.VAD_MODE_SLEEP
 *
 * When the system goes to sleep and expects to be awaken by voice detection event as configured,
 * VAD is set in VAD_MODE_ALWAYS_LISTENING
 *
 * After the system is awake and no voice detection is needed VAD is set either in
 * VAD_MODE_STANDBY or
 * VAD_MODE_SLEEP if the system is expected to go to VAD sleep mode soon.
 *
 * VAD IRQ handler is called when an IRQ event is triggered by the VAD system.
 *
 * VAD handler function is assigned with hw_vad_register_interrupt(hw_vad_interrupt_cb cb)
 *
 * An IRQ can also be triggered if ambient noise information (NFI) is beyond a specified threshold (VAD_NFI_DET)
 * This can be used by the application for properly configuring the detection parameters or the ADC sensitivity.
 *
 * NFI threshold is set by:
 * hw_vad_set_nfi_threshold()
 *
 * If NFI pins are implemented we can get the NFI of the VAD algorithm with:
 * hw_vad_get_nfi_threshold()
 * NFI can be useful for setting the proper VAD configuration parameters, or other uses such as properly configuring the ADC
 * or other processing blocks such as a hot word detection algorithm.
 *
 *
 *
 ****************************************************************************************
 */

#ifndef HW_VAD_H_
#define HW_VAD_H_

#if dg_configUSE_HW_VAD

#include <sdk_defs.h>


/**
 * \brief VAD clock selection
 *
 */
typedef enum {
        HW_VAD_MCLK_RCLP32K = 0,      /**< VAD clock is RCLP normalized at 32Khz*/
        HW_VAD_MCLK_XTAL32K = 1       /**< VAD clock is XTAL */
} HW_VAD_CLK;

/**
 * \brief VAD mclk clock div
 *
 */
typedef enum {
        HW_VAD_MCLK_DIV_1 = 0,        /**< MCLK is input clock divided by 1 */
        HW_VAD_MCLK_DIV_2 = 1,        /**< MCLK is input clock divided by 2 */
        HW_VAD_MCLK_DIV_4 = 2,        /**< MCLK is input clock divided by 4 */
        HW_VAD_MCLK_DIV_8 = 3,        /**< MCLK is input clock divided by 8 */
        HW_VAD_MCLK_DIV_16 = 4,       /**< MCLK is input clock divided by 16 */
        HW_VAD_MCLK_DIV_24 = 5,       /**< MCLK is input clock divided by 24 */
        HW_VAD_MCLK_DIV_48 = 6        /**< MCLK is input clock divided by 48 */
} HW_VAD_MCLK_DIV;

/**
 * \brief VAD IRQ mode
 *
 */
typedef enum {
        HW_VAD_IRQ_MODE_HIGH = 0,       /**< The generated IRQ is a high level. */
        HW_VAD_IRQ_MODE_PULSE = 1       /**< The generated IRQ is a pulse, whose duration is 8 internal clock cycles */
} HW_VAD_IRQ_MODE;

/**
 * \brief VAD  mode
 *
 * 1.VAD_MODE_STANDBY  :
 *      sb=1
 *
 *          VAD is turned off. No voice event can be detected. IRQ=0.
 *
 * 2.VAD_MODE_SLEEP :
 *      sb=0
 *      sleep=1
 *
 *          VAD analog part is powered on. No voice event can be detected. IRQ=0.
 *          This mode allows a fast transition to Always Listening mode.
 *          It is recommended to set VAD in Sleep mode during the recording of AIP/AIN audio
 *          inputs on ADC path.
 *
 * 3.VAD_MODE_ALWAYS_LISTENING:
 *      sb=0
 *      sleep=0
 *
 *          When entering Always Listening mode, IRQ is set to ’0’. A voice event can be detected.
 *          When a voice event is detected, IRQ is set to ’1’.
 */
typedef enum {
        HW_VAD_MODE_STANDBY = 0,            /**< VAD standby mode. */
        HW_VAD_MODE_SLEEP = 1,              /**< VAD sleep mode. */
        HW_VAD_MODE_ALWAYS_LISTENING = 2    /**< VAD always listening (detection on) mode. */
} HW_VAD_MODE;

/**
 * \brief VAD voice track sensitivity
 *
 * Voice Tracking parameter: This parameter allows to
 * set the adaptation speed of the system depending on
 * the voice input. When the setting of this parameter
 * is low, the high-frequency sensitivity of the VAD increases,
 * some phoneme can be detected easily but high-frequency
 * ambient noise can be considered as voice. When the setting
 * of this parameter is high, the high-frequency sensitivity of
 * the VAD decreases, high frequency ambient noise is filtered
 * but some phoneme can be lost.
 *
 */
typedef enum {
        HW_VAD_VOICE_SENS_FAST = 0,     /**< */
        HW_VAD_VOICE_SENS_DEFAULT = 1,  /**< Default voice track sensitivity */
        HW_VAD_VOICE_SENS_SLOW = 2,     /**< */
        HW_VAD_VOICE_SENS_SLOWER = 3    /**< */
} HW_VAD_VOICE_SENS;

/**
 * \brief VAD noise track sensitivity
 *
 * Background Noise Tracking parameter: This parameter
 * allows to set the speed of the system adaptation to
 * the ambient noise. This parameter gives the flexibility
 * to adapt the VAD to the application environment.
 *
 */
typedef enum {
        HW_VAD_NOISE_SENS_FAST_4 = 0,   /**< */
        HW_VAD_NOISE_SENS_FAST_3 = 1,   /**< */
        HW_VAD_NOISE_SENS_FAST_2 = 2,   /**< */
        HW_VAD_NOISE_SENS_FAST_1 = 3,   /**< */
        HW_VAD_NOISE_SENS_DEFAULT = 4,  /**< Default noise sensitivity */
        HW_VAD_NOISE_SENS_SLOW_1 = 5,   /**< */
        HW_VAD_NOISE_SENS_SLOW_2 = 6,   /**< */
        HW_VAD_NOISE_SENS_SLOW_3 = 7,   /**< */
} HW_VAD_NOISE_SENS;

/**
 * \brief VAD power level sensitivity
 *
 * Power Level Sensitivity: Ratio between ambient noise
 * and voice level to be detected. When the setting of this
 * parameter is low, the VAD sensitivity increases, leading
 * to higher VDV and possibly higher NDV. When
 * the setting of this parameter is high, the VAD sensitivity
 * decreases, leading to lower NDV and possibly
 * lower VDV.
 *
 */
typedef enum {
        HW_VAD_PWR_LVL_SENS_2_DB = 0,   /**< */
        HW_VAD_PWR_LVL_SENS_3_DB = 1,   /**< */
        HW_VAD_PWR_LVL_SENS_4_DB = 2,   /**< */
        HW_VAD_PWR_LVL_SENS_5_DB = 3,   /**< */
        HW_VAD_PWR_LVL_SENS_6_DB = 4,   /**< 6db Default power level sensitivity */
        HW_VAD_PWR_LVL_SENS_8_DB = 5,   /**< */
        HW_VAD_PWR_LVL_SENS_10_DB = 6,  /**< */
        HW_VAD_PWR_LVL_SENS_16_DB = 7,  /**< */
} HW_VAD_PWR_LVL_SENS;


/**
 * \brief VAD Minimum delay
 *
 * Minimum Delay: This parameter allows to set the
 * minimum time before a detection when switching to
 * Always listening mode. This delay is defined as a
 * number of clock cycle, divided from MCLK depending
 * on MCLK_DIV setting.
 */
typedef enum {
        HW_VAD_MIN_DELAY_768_CYCLES = 0,    /**< */
        HW_VAD_MIN_DELAY_1536_CYCLES = 1,   /**< Default MIN delay */
        HW_VAD_MIN_DELAY_3584_CYCLES = 2,   /**< */
        HW_VAD_MIN_DELAY_9632_CYCLES = 3    /**< */
} HW_VAD_MIN_DELAY;


/**
 * \brief VAD Minimum event duration
 *
 * Minimum Event Duration: This parameter allows to
 * set the Minimum vocal signal duration that can be
 * detected by the system. When the setting of this parameter
 * is low, the detection latency decreases but the
 * high-frequency ambient noise can be considered as
 * voice. When the setting of this parameter is high,
 * the high-frequency ambient noise is filtered but the
 * detection latency increase. This delay is defined as a
 * number of clock cycle, divided from MCLK depending
 * on MCLK_DIV setting.
 *
 *
 */
typedef enum {
        HW_VAD_MIN_EVENT_1_CYCLE = 0,       /**< */
        HW_VAD_MIN_EVENT_16_CYCLES = 1,     /**< */
        HW_VAD_MIN_EVENT_32_CYCLES = 2,     /**< 32 cycles Default min event duration. */
        HW_VAD_MIN_EVENT_64_CYCLES = 3,     /**< */
        HW_VAD_MIN_EVENT_128_CYCLES = 4,    /**< */
        HW_VAD_MIN_EVENT_256_CYCLES = 5,    /**< */
        HW_VAD_MIN_EVENT_512_CYCLES = 6,    /**< */
        HW_VAD_MIN_EVENT_1024_CYCLES = 7,   /**< */
} HW_VAD_MIN_EVENT;

/**
 * \brief VAD interrupt handler
 *
 */
typedef void (*hw_vad_interrupt_cb)(void);

/**
 * \brief VAD configuration
 *
 */
typedef struct {
        HW_VAD_CLK              mclk;               /**< Main clock selection */
        HW_VAD_MCLK_DIV         mclk_div;           /**< Main clock divisor */
        HW_VAD_IRQ_MODE         irq_mode;           /**< Interrupt mode */
        HW_VAD_VOICE_SENS       voice_sens;         /**< Voice track sensitivity */
        HW_VAD_NOISE_SENS       noise_sens;         /**< Noise track sensitivity */
        HW_VAD_PWR_LVL_SENS     power_sens;         /**< Power level sensitivity */
        HW_VAD_MIN_DELAY        min_delay;          /**< Minimum delay before a detection when switching to always listening mode */
        HW_VAD_MIN_EVENT        min_event;          /**< Minimum event duration */
        uint8_t                 nfi_threshold;      /**< NFI detection threshold above which an IRQ is sent */
} hw_vad_config_t;


typedef void (*hw_vad_interrupt_cb)(void);

/**
 * \brief configure VAD
 *
 * If \p cfg is NULL, this function does
 * nothing.

 * \param [in] cfg configuration
 *
 * \warning it is recommended to call this function while VAD is in Stand-by or Sleep mode,
 * because it changes MCLK divisor and sensitivity settings
 */
void hw_vad_configure(const hw_vad_config_t *cfg);


/**
 * \brief get VAD configuration
 *
 * If \p cfg is NULL, this function does
 * nothing.

 * \param [out] cfg a pointer to VAD configuration
 *
 */
void hw_vad_get_config(hw_vad_config_t* cfg);

/**
 * \brief Reset VAD to its default values.
 *
 */
void hw_vad_reset(void);

/**
 * \brief Register interrupt handler
 *
 * Interrupt is enabled after calling this function.
 *
 * \param [in] cb callback fired on interrupt
 *
 * \warning when VAD interrupt is triggered VAD mode changes to SLEEP. It is application
 * responsibility to change VAD mode to the desired one.
 *
 * \warning if VAD handler has already been called, VAD mode will have been changed to standby.
 * It is application responsibility to set VAD in correct mode after calling this function.
 */
void hw_vad_register_interrupt(hw_vad_interrupt_cb cb);

/**
 * \brief Unregister interrupt handler
 *
 * Interrupt is disabled after calling this function.
 *
 */
void hw_vad_unregister_interrupt(void);

/**
 * \brief set VAD mode
 *
 * Set VAD to HW_VAD_MODE.
 *
 */
void hw_vad_set_mode(HW_VAD_MODE mode);

/**
 * \brief get VAD mode
 *
 * \return VAD mode
 *
 */
HW_VAD_MODE hw_vad_get_mode(void);

/**
 * \brief Configure voice tracking sensitivity
 *
 * Voice Tracking parameter: This parameter allows to
 * set the adaptation speed of the system depending on
 * the voice input. When the setting of this parameter
 * is low, the high-frequency sensitivity of the VAD increases,
 * some phoneme can be detected easily but high-frequency ambient
 * noise can be considered as voice. When the setting of this
 * parameter is high, the high-frequency sensitivity of the VAD
 * decreases, high frequency ambient noise is filtered but some
 * phoneme can be lost.
 *
 * \warning It is recommended to change the sensitivity settings in Stand-by mode or in Sleep mode.
 */
__STATIC_INLINE void hw_vad_set_voice_track_sens(HW_VAD_VOICE_SENS sensitivity)
{
        ASSERT_WARNING(hw_vad_get_mode() != HW_VAD_MODE_ALWAYS_LISTENING);
        REG_SETF(VAD, VAD_CTRL0_REG, VAD_VTRACK, sensitivity);
}

/**
 * \brief Configure background noise sensitivity
 *
 * Background Noise Tracking parameter: This parameter
 * allows to set the speed of the system adaptation to
 * the ambient noise. This parameter gives the flexibility
 * to adapt the VAD to the application environment.
 *
 * \warning It is recommended to change the sensitivity settings in Stand-by mode or in Sleep mode.
 */
__STATIC_INLINE void hw_vad_set_bg_noise_sens(HW_VAD_NOISE_SENS sensitivity)
{
        ASSERT_WARNING(hw_vad_get_mode() != HW_VAD_MODE_ALWAYS_LISTENING);
        REG_SETF(VAD, VAD_CTRL0_REG, VAD_NTRACK, sensitivity);
}

/**
 * \brief Configure power level sensitivity
 *
 * Power Level Sensitivity: Ratio between ambient noise
 * and voice level to be detected. When the setting of this
 * parameter is low, the VAD sensitivity increases, leading
 * to higher VDV and possibly higher NDV. When
 * the setting of this parameter is high, the VA
 *
 * \warning It is recommended to change the sensitivity settings in Stand-by mode or in Sleep mode.
 */
__STATIC_INLINE void hw_vad_set_pwr_lvl_sens(HW_VAD_PWR_LVL_SENS sensitivity)
{
        ASSERT_WARNING(hw_vad_get_mode() != HW_VAD_MODE_ALWAYS_LISTENING);
        REG_SETF(VAD, VAD_CTRL0_REG, VAD_PWR_LVL_SNSTVTY, sensitivity);
}

/**
 * \brief Configure minimum delay (before detection in listening mode)
 *
 * Minimum Delay: This parameter allows to set the
 * minimum time before a detection when switching to
 * Always listening mode. This delay is defined as a
 * number of clock cycle, divided from MCLK depending
 * on MCLK_DIV setting.
 *
 *
 */
__STATIC_INLINE void hw_vad_set_min_delay(HW_VAD_MIN_DELAY delay)
{
        REG_SETF(VAD, VAD_CTRL1_REG, VAD_MINDELAY, delay);
}

/**
 * \brief Configure minimum event duration (MIN vocal signal duration).
 *
 * Minimum Event Duration: This parameter allows to
 * set the Minimum vocal signal duration that can be
 * detected by the system. When the setting of this parameter
 * is low, the detection latency decreases but the
 * high-frequency ambient noise can be considered as voice.
 * When the setting of this parameter is high, the high-frequency
 * ambient noise is filtered but the detection latency increase.
 * This delay is defined as a number of clock cycle, divided from
 * MCLK depending on MCLK_DIV setting.
 *
 */
__STATIC_INLINE void hw_vad_set_min_evt_duration(HW_VAD_MIN_EVENT duration)
{
        REG_SETF(VAD, VAD_CTRL1_REG, VAD_MINEVENT, duration);
}

/**
 * \brief Configure noise floor information.
 *
 * NFI Detection: This parameter defines the NFI
 * threshold above which an IRQ is sent. Refer to NFI
 * description.
 *
 * The Noise Floor Information (NFI) is the ambient noise reference level used in the VAD. The NFI
 * represents the average during typically 100 ms of the peak output noise level, given in dBVp on
 * the audio bandwidth [20Hz - 20kHz] after a [100Hz - 6kHz] first order filtering.
 *
 */
__STATIC_INLINE void hw_vad_set_nfi_threshold(uint8_t threshold)
{
        REG_SETF(VAD, VAD_CTRL2_REG, VAD_NFI_DET, threshold);
}

/**
 * \brief Get noise floor information
 *
 * VAD NFI output (5bits range)
 *
 */
__STATIC_INLINE uint8_t hw_vad_get_nfi_threshold(void)
{
        return REG_GETF(VAD, VAD_CTRL2_REG, VAD_NFI_DET);
}

/**
 * \brief Configure master clock divisor.
 *
 * \warning It is recommended to change the MCLK_DIV setting in Stand-by mode.
 */
__STATIC_INLINE void hw_vad_set_clock_div(HW_VAD_MCLK_DIV division)
{
        ASSERT_WARNING(hw_vad_get_mode() != HW_VAD_MODE_ALWAYS_LISTENING);
        REG_SETF(VAD, VAD_CTRL3_REG, VAD_MCLK_DIV, division);
}

/**
 * \brief Configure IRQ generation level/pulse.
 *
 */
__STATIC_INLINE void hw_vad_set_irq_mode(HW_VAD_IRQ_MODE mode)
{
        REG_SETF(VAD, VAD_CTRL4_REG, VAD_IRQ_MODE, mode);
}
#endif /* dg_configUSE_HW_VAD */
#endif /* HW_VAD_H_ */

/**
 * \}
 * \}
 */
