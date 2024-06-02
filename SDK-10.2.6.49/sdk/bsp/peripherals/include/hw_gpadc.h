/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_GPADC GPADC GPADC Driver
 * \{
 * \brief General Purpose ADC
 */

/**
 ****************************************************************************************
 *
 * @file hw_gpadc.h
 *
 * @brief Definition of API for the GPADC Low Level Driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef HW_GPADC_H
#define HW_GPADC_H


#if dg_configUSE_HW_GPADC

#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"

/***************************************************************************
 *********    Macro, type and data-structure definitions     ***************
 ***************************************************************************/

/**
 * \addtogroup GPADC_DATA GPADC Data Types
 *
 * \brief Enumeration, structure, type and macro definitions
 *
 * \{
 */

typedef void * HW_GPADC_ID;

/**
 * \brief ADC interrupt handler
 *
 */
typedef void (*hw_gpadc_interrupt_cb)(void);

/**
 * \brief ADC callback for read function
 *
 */
typedef void (*hw_gpadc_read_cb)(void *user_data, uint32_t conv_to_go);

/**
 * \brief The 16 LSBits of the 32-bit result register
 *
 */
#define HW_GPADC_RESULT_NOB                     (16U)

/**
 * \brief The default Effective Number Of Bits with no averaging (zero oversampling)
 *
 */
#define HW_GPADC_DEFAULT_ENOB                   (10U)

/**
 * \brief The excessive bits in the result register with no averaging (zero oversampling)
 *
 */
#define HW_GPADC_UNUSED_BITS                    (HW_GPADC_RESULT_NOB - HW_GPADC_DEFAULT_ENOB)

/**
 * \brief A help macro to define a mid-scale measurement:
 *        0 mVolt  in Differential mode
 *        Vref/2 mVolt in Single-ended mode
 */
#define HW_GPADC_MID_SCALE_ADC                  (1 << (HW_GPADC_RESULT_NOB - 1))

/**
 * \brief Reset value for the GP_ADC_OFFP and GP_ADC_OFFN registers.
 *        The given default 0x200 is suitable for Common Mode Level = VREF/2.
 *        It should be adjusted according to paragraph Common Mode Adjustment in the DataSheet.
 */
#define HW_GPADC_OFFSET_RESET                   0x200

/**
 * \def HW_GPADC_DMA_SUPPORT
 *
 * \brief DMA support for GPADC
 *
 */
#define HW_GPADC_DMA_SUPPORT                    dg_configGPADC_DMA_SUPPORT

/**
 * \brief ADC input mode
 *
 */
typedef enum {
        HW_GPADC_INPUT_MODE_DIFFERENTIAL = 0,   /**< differential mode (default) */
        HW_GPADC_INPUT_MODE_SINGLE_ENDED = 1    /**< single ended mode */
} HW_GPADC_INPUT_MODE;

/**
 * \brief Temperature sensor mask
 */
#define HW_GPADC_DIFF_TEMPSENS_Msk                      0x03

/**
 * \brief On-chip temperature sensors
 *
 */
typedef enum {
        /* Sensor selection for GP_ADC_DIFF_TEMP_EN = 0 follows this line */
        HW_GPADC_CHARGER_TEMPSENS_GND     = 0,  /**< Ground (no sensor) */
        HW_GPADC_CHARGER_TEMPSENS_Z       = 1,  /**< Z from charger */
        HW_GPADC_CHARGER_TEMPSENS_VNTC    = 2,  /**< V(ntc) from charger */
        HW_GPADC_CHARGER_TEMPSENS_VTEMP   = 3,  /**< V(temp) from charger */
        /* Sensor selection for GP_ADC_DIFF_TEMP_EN = 1 follows this line */
        HW_GPADC_NO_TEMP_SENSOR           = 4,  /**< No on-chip temperature sensor selected (default) */
        HW_GPADC_TEMP_SENSOR_NEAR_RADIO   = 5,  /**< Diode temperature sensor near radio */
        HW_GPADC_TEMP_SENSOR_NEAR_CHARGER = 6,  /**< Diode temperature sensor near charger */
        HW_GPADC_TEMP_SENSOR_NEAR_BANDGAP = 7,  /**< Diode temperature sensor near bandgap */
        HW_GPADC_TEMP_SENSOR_DIE_TEMP     = 8,  /**< DIE_TEMP sensor - Not effective if assigned to gpadc_config.temp_sensor */
        HW_GPADC_TEMPSENSOR_MAX
} HW_GPADC_TEMP_SENSORS;

/**
 * \brief GPADC oversampling
 *
 * In this mode multiple successive conversions will be executed and the results are added together
 * to increase the effective number of bits
 *
 */
typedef enum {
        HW_GPADC_OVERSAMPLING_1_SAMPLE          = 0,    /**< 1 sample is taken or 2 in case chopping is enabled */
        HW_GPADC_OVERSAMPLING_2_SAMPLES         = 1,    /**< 2 samples are taken */
        HW_GPADC_OVERSAMPLING_4_SAMPLES         = 2,    /**< 4 samples are taken */
        HW_GPADC_OVERSAMPLING_8_SAMPLES         = 3,    /**< 8 samples are taken */
        HW_GPADC_OVERSAMPLING_16_SAMPLES        = 4,    /**< 16 samples are taken */
        HW_GPADC_OVERSAMPLING_32_SAMPLES        = 5,    /**< 32 samples are taken */
        HW_GPADC_OVERSAMPLING_64_SAMPLES        = 6,    /**< 64 samples are taken */
        HW_GPADC_OVERSAMPLING_128_SAMPLES       = 7     /**< 128 samples are taken */
} HW_GPADC_OVERSAMPLING;

#if HW_GPADC_DMA_SUPPORT
#include "hw_dma.h"

/**
 * \brief DMA configuration.
 *        This is a cut down set of the \ref DMA_setup structure,
 *        offering the configurable DMA parameters applicable to GPADC.
 *
 * \note The DMA functionality delivers the content of the GP_ADC_RESULT_REG with the help of
 *       \ref hw_gpadc_read, hence post-processing of the delivered data is necessary.
 *
 * \sa gpadc_config
 */
typedef struct {
        HW_DMA_CHANNEL       channel;                   /**< DMA Channel Number to be used (must be even number) */
        HW_DMA_PRIO          prio;                      /**< Channel priority from 0 to 7 */
        bool                 circular;                  /**< Select normal or circular operation for DMA */
        uint16               irq_nr_of_trans;           /**< Number of transfers before IRQ generation */
} gpadc_dma_cfg;
#endif /* HW_GPADC_DMA_SUPPORT */

/**
 * \}
 */

#include "hw_gpadc_v2.h"

/***************************************************************************
 ****************      GP_ADC configuration functions    *******************
 ***************************************************************************/

/**
 * \addtogroup GPADC_CONFIG_FUNCTIONS Configuration functions
 *
 * \brief Access to specific gpadc_config structure members and other essential configuration tweaks
 *
 * \{
 *
 */

/**
 * \brief Set continuous mode
 *
 * With continuous mode enabled ADC will automatically restart conversion once completed. It's still
 * required to start 1st conversion using hw_gpadc_start(). Interval between subsequent conversions
 * can be adjusted using hw_gpadc_set_interval().
 *
 * \param [in] enabled continuous mode state
 *
 * \sa hw_gpadc_start
 * \sa hw_gpadc_set_interval
 */
__STATIC_INLINE void hw_gpadc_set_continuous(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_CONT, !!enabled);
}

/**
 * \brief Get continuous mode state
 *
 * \return continuous mode state
 *
 */
__STATIC_INLINE bool hw_gpadc_get_continuous(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_CONT);
}

/**
 * \brief Set input mode
 *
 * \param [in] mode input mode
 *
 */
__STATIC_INLINE void hw_gpadc_set_input_mode(HW_GPADC_INPUT_MODE mode)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SE, mode);
}

/**
 * \brief Get the current input mode
 *
 * return input mode
 *
 */
__STATIC_INLINE HW_GPADC_INPUT_MODE hw_gpadc_get_input_mode(void)
{
        return (HW_GPADC_INPUT_MODE) REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SE);
}

/**
 * \brief Set oversampling
 *
 * With oversampling enabled multiple successive conversions will be executed and results are added
 * together to increase effective number of bits in result.
 *
 * Number of samples taken is 2<sup>\p n_samples</sup>. Valid values for \p n_samples are 0-7 thus
 * at most 128 samples can be taken. In this case, 17bits of result are generated with the least
 * significant bit being discarded.
 *
 * \param [in] n_samples number of samples to be taken
 *
 */
__STATIC_INLINE void hw_gpadc_set_oversampling(HW_GPADC_OVERSAMPLING n_samples)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_CONV_NRS, n_samples);
}

/**
 * \brief Get the current oversampling
 *
 * \return number of samples to be taken
 *
 * \sa hw_gpadc_set_oversampling
 *
 */
__STATIC_INLINE HW_GPADC_OVERSAMPLING hw_gpadc_get_oversampling(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_CONV_NRS);
}

/**
 * \brief Set input mute state
 *
 * Once enabled, samples are taken at mid-scale to determine internal offset and/or notice of the
 * ADC with regards to VDD_REF.
 *
 * \param [in] enabled mute state
 *
 * \sa hw_gpadc_offset_calibrate
 *
 */
__STATIC_INLINE void hw_gpadc_set_mute(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_MUTE, !!enabled);
}

/**
 * \brief Get the current input mute state
 *
 * \return mute state
 *
 * \sa hw_gpadc_set_mute
 *
 */
__STATIC_INLINE bool hw_gpadc_get_mute(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_MUTE);
}

/**
 * \brief Set input and output sign change
 *
 * Once enabled, sign of ADC input and output is changed.
 *
 * \param [in] enabled sign change state
 *
 */
__STATIC_INLINE void hw_gpadc_set_sign_change(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SIGN, !!enabled);
}

/**
 * \brief Get the current input and output sign change
 *
 * \return sign change state
 *
 */
__STATIC_INLINE bool hw_gpadc_get_sign_change(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SIGN);
}

/**
 * \brief Set state of on-chip temperature sensors
 *
 * Once enabled, the diode temperature sensors can be selected.
 *
 * \param [in] enabled on-chip temperature sensors
 *
 * \sa hw_gpadc_set_temp_sensor
 */
__STATIC_INLINE void hw_gpadc_set_diff_temp_sensors(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIFF_TEMP_EN, !!enabled);
}

/**
 * \brief Get the current state of on-chip temperature sensors
 *
 * \return on-chip temperature sensors state
 *
 * \sa hw_gpadc_set_diff_temp_sensors
 */
__STATIC_INLINE bool hw_gpadc_get_diff_temp_sensors(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIFF_TEMP_EN);
}

/**
 * \brief Selects on-chip temperature sensor
 *
 * \param [in] sensor on-chip temperature sensor
 *
 * \note When temperature sensors are enabled (GP_ADC_DIFF_TEMP_EN=1),
 * then: 0 = GND, 1 = sensor near radio, 2 = sensor near charger, 3 = sensor near bandgap.
 * When temperature sensors are disabled (GP_ADC_DIFF_TEMP_EN=0),
 * then: 0 = GND, 1 = Z, 2 = V(ntc) from charger, 3 = V(temp) from charger.
 *
 * \note Users are advised NOT to use this API function, unless they know exactly what they are doing.
 * In the general case, setting the gpadc->temp_sensor and calling hw_gpadc_init() or hw_gpadc_configure() is enough.
 *
 * \sa hw_gpadc_set_diff_temp_sensors
 * \sa hw_gpadc_configure
 * \note Not to be used for absolute temperature measurements
 *
 */
__STATIC_INLINE void hw_gpadc_select_diff_temp_sensor(HW_GPADC_TEMP_SENSORS sensor)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIFF_TEMP_SEL, sensor & HW_GPADC_DIFF_TEMPSENS_Msk);
}

/**
 * \brief Reads on-chip temperature sensor selection
 *
 * \return on-chip temperature sensor
 *
 * \note When temperature sensors are enabled (GP_ADC_DIFF_TEMP_EN=1),
 * then: 0 = GND, 1 = sensor near radio, 2 = sensor near charger, 3 = sensor near bandgap.
 * When temperature sensors are disabled (GP_ADC_DIFF_TEMP_EN=0),
 * then: 0 = GND, 1 = Z, 2 = V(ntc) from charger, 3 = V(temp) from charger.
 *
 * \sa hw_gpadc_set_temp_sensor
 * \note Not to be used for absolute temperature measurements
 */
__STATIC_INLINE HW_GPADC_TEMP_SENSORS hw_gpadc_get_temp_sensor(void)
{
        return (REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIFF_TEMP_SEL) & HW_GPADC_DIFF_TEMPSENS_Msk);
}

/**
 * \brief Set chopping state
 *
 * Once enabled, two samples with opposite polarity are taken to cancel offset.
 *
 * \param [in] enabled chopping state
 *
 */
__STATIC_INLINE void hw_gpadc_set_chopping(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_CHOP, !!enabled);
}

/**
 * \brief Get the current chopping state
 *
 * \return chopping state
 *
 */
__STATIC_INLINE bool hw_gpadc_get_chopping(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_CHOP);
}


/**
 * \brief Set masked interrupt
 *
 * \param [in] enabled masked interrupt
 *
 */
__STATIC_INLINE void hw_gpadc_set_mint(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_MINT, !!enabled);
}

/**
 * \brief Get masked interrupt state
 *
 * \return masked interrupt enabled or disabled
 *
 */
__STATIC_INLINE bool hw_gpadc_get_mint(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_MINT);
}

/**
 * \brief Set state of constant 20uA load current on ADC LDO output
 *
 * Constant 20uA load current on LDO output can be enabled so that the current will not drop to 0.
 *
 * \param [in] enabled load current state
 *
 */
__STATIC_INLINE void hw_gpadc_set_ldo_constant_current(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_I20U, !!enabled);
}

/**
 * \brief Get the current state of constant 20uA load current on ADC LDO output
 *
 * \return load current current state
 *
 * \sa hw_gpadc_set_ldo_constant_current
 *
 */
__STATIC_INLINE bool hw_gpadc_get_ldo_constant_current(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_I20U);
}

/**
 * \brief Set interval between conversions in continuous mode
 *
 * Interval time is \p mult x 1.024ms. Valid values are 0-255.
 *
 * \param [in] mult multiplier
 *
 * \sa hw_gpadc_set_continuous
 *
 */
__STATIC_INLINE void hw_gpadc_set_interval(uint8_t mult)
{
        REG_SETF(GPADC, GP_ADC_CTRL3_REG, GP_ADC_INTERVAL, mult);
}

/**
 * \brief Get the current interval between conversions in continuous mode
 *
 * \return multiplier (interval = multiplier x 1024 ms)
 *
 * \sa hw_gpadc_set_interval
 *
 */
__STATIC_INLINE uint8_t hw_gpadc_get_interval(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL3_REG, GP_ADC_INTERVAL);
}

/**
 * \brief Set DMA functionality
 *
 * \param [in] enabled  True to enable DMA functionality, false to disable it
 *
 */
__STATIC_INLINE void hw_gpadc_set_dma_functionality(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DMA_EN, !!enabled);
}

/**
 * \brief Get current state of DMA functionality
 *
 * \return DMA functionality state
 *
 */
__STATIC_INLINE bool hw_gpadc_get_dma_functionality(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DMA_EN);
}

/**
 * \}
 */

/***************************************************************************
 ****************      GP_ADC calibration functions      *******************
 ***************************************************************************/

/**
 * \addtogroup GPADC_CALIBR_FUNCTIONS Calibration functions
 *
 * \brief Gain and offset calibration
 *
 * \{
 */

/**
 * \brief Set offset adjustment for positive ADC array.
 *        This register offers a coarse offset calibration, whereas a finer calibration
 *        occurs in the hw_gpadc_apply_correction, using the device trim values
 *        (positive/negative offsets, gain correction) stored in the Configuration Script
 *
 * \param [in] offset offset value
 *
 * \sa hw_gpadc_offset_calibrate
 * \sa hw_gpadc_apply_correction
 * \sa hw_gpadc_set_negative_offset
 *
 */
__STATIC_INLINE void hw_gpadc_set_offset_positive(uint16_t offset)
{
        GPADC->GP_ADC_OFFP_REG = offset & REG_MSK(GPADC, GP_ADC_OFFP_REG, GP_ADC_OFFP);
}

/**
 * \brief Get the current offset adjustment for positive ADC array
 *
 * \return offset value
 *
 * \sa hw_gpadc_set_offset_positive
 *
 */
__STATIC_INLINE uint16_t hw_gpadc_get_offset_positive(void)
{
        return GPADC->GP_ADC_OFFP_REG & REG_MSK(GPADC, GP_ADC_OFFP_REG, GP_ADC_OFFP);
}

/**
 * \brief Set offset adjustment for negative ADC array.
 *        This register offers a coarse offset calibration, whereas a finer calibration
 *        occurs in the hw_gpadc_apply_correction, using the device trim values
 *        (positive/negative offsets, gain correction) stored in the Configuration Script
 *
 * \param [in] offset offset value
 *
 * \sa hw_gpadc_offset_calibrate
 * \sa hw_gpadc_apply_correction
 * \sa hw_gpadc_set_positive_offset
 *
 */
__STATIC_INLINE void hw_gpadc_set_offset_negative(uint16_t offset)
{
        GPADC->GP_ADC_OFFN_REG = offset & REG_MSK(GPADC, GP_ADC_OFFN_REG, GP_ADC_OFFN);
}


/**
 * \brief Get the current offset adjustment for negative ADC array
 *
 * \return offset value
 *
 * \sa hw_gpadc_set_offset_negative
 *
 */
__STATIC_INLINE uint16_t hw_gpadc_get_offset_negative(void)
{
        return GPADC->GP_ADC_OFFN_REG & REG_MSK(GPADC, GP_ADC_OFFN_REG, GP_ADC_OFFN);
}

/**
 * \brief Store Single Ended ADC Gain Error
 *
 * \param [in] single ADC Single Ended Gain Error
 *
 */
void hw_gpadc_store_se_gain_error(int16_t single);

/**
 * \brief Store Differential ADC Gain Error
 *
 * \param [in] diff ADC Differential Gain Error
 *
 */
void hw_gpadc_store_diff_gain_error(int16_t diff);

/**
 * \brief Store Single Ended ADC Offset Error
 *
 * \param [in] single ADC Single Ended Offset Error
 *
 */
void hw_gpadc_store_se_offset_error(int16_t single);

/**
 * \brief Store Differential ADC Offset Error
 *
 * \param [in] diff ADC Differential Offset Error
 *
 */
void hw_gpadc_store_diff_offset_error(int16_t diff);

/**
 * \brief Calculate Single Ended ADC Gain Error from two points
 *
 * \param [in] low  measurement at the low end of the full scale
 * \param [in] high measurement at the high end of the full scale
 *
 * \return Single Ended ADC Gain Error
 *
 * \note Valid return range (-2048, 2048)
 *
 */
__STATIC_INLINE int16_t hw_gpadc_calculate_single_ended_gain_error(int16_t low, int16_t high)
{
        return ((high - low + ((uint16_t)(high - low) >> 2 )) - UINT16_MAX);
}

/**
 * \brief Calculate Single Ended ADC Offset Error from two points
 *
 * \param [in] low  measurement at the low end of the full scale
 * \param [in] high measurement at the high end of the full scale
 *
 * \return Single Ended ADC Offset Error
 *
 * \note Valid return range (-512, 512)
 *
 */
__STATIC_INLINE int16_t hw_gpadc_calculate_single_ended_offset_error(int16_t low, int16_t high)
{
        return ((int16_t)((9 * low) - high) >> 3);
}

/**
 * \brief Calculate Differential ADC Gain Error from two points
 *
 * \param [in] low  measurement at the low end of the full scale
 * \param [in] high measurement at the high end of the full scale
 *
 * \return Differential ADC Gain Error
 *
 * \note Valid return range (-2048, 2048)
 *
 */
__STATIC_INLINE int16_t hw_gpadc_calculate_differential_gain_error(int16_t low, int16_t high)
{
        return (((high - low) + ((uint16_t)(high - low) >> 2)) - UINT16_MAX);
}

/**
 * \brief Calculate Differential ADC Offset Error from two points
 *
 * \param [in] low  measurement at the low end of the full scale
 * \param [in] high measurement at the high end of the full scale
 *
 * \return Differential ADC Offset Error
 *
 * \note Valid return range (-512, 512)
 *
 */
__STATIC_INLINE int16_t hw_gpadc_calculate_differential_offset_error(int16_t low, int16_t high)
{
        return (low + high) >> 1;
}

/**
 * \brief Check the availability of ADC Gain Error
 *
 * \return ADC Gain Error availability
 *
 */
bool hw_gpadc_pre_check_for_gain_error(void);

/**
 * \brief Get single ended ADC Gain Error
 *
 * \return ADC Gain Error
 *
 */
int16_t hw_gpadc_get_single_ended_gain_error(void);

/** \brief Perform coarse ADC offset calibration
 *
 * A coarse offset calibration is performed at runtime.
 * Dependent on the current \sa HW_GPADC_INPUT_MODE.
 * The calibration must be done with specific configuration settings of the ADC.
 * The driver configuration is saved and re-applied after calibration.
 * The outcome is saved in the OFFP and OFFN registers.
 * Re-calibrate if the input mode is changed OR these OFFP/OFFN registers are reset to their defaults.
 */
void hw_gpadc_offset_calibrate(void);

/**
 * \}
 */

/***************************************************************************
 ****************    Basic functionality of the GPADC    *******************
 ***************************************************************************/

/**
 * \addtogroup GPADC_FUNCTIONS Basic GPADC Functionality
 *
 * \brief Initialization, configuration, measurement and voltage conversion functions
 *
 * \{
 */

/**
 * \brief Initialize ADC
 *
 * \p cfg can be NULL - no configuration is performed in such case.
 *
 * \param [in] cfg configuration
 * \param [in] enable enable the ADC LDO after the configuration is applied
 *
 */
void hw_gpadc_init(const gpadc_config *cfg, bool enable);

/**
 * \brief Configure ADC
 *
 * Shortcut to call appropriate configuration function. \p cfg must be valid.
 *
 * \param [in] cfg configuration
 *
 */
void hw_gpadc_configure(const gpadc_config *cfg);

/**
 * \brief Register interrupt handler
 *
 * Additionally, interrupt is enabled after calling this function.
 * Any pending interrupt is cleared before callback registration.
 *
 * \param [in] cb callback to be fired on interrupt
 *
 * \sa hw_gpadc_clear_interrupt
 *
 */
void hw_gpadc_register_interrupt(hw_gpadc_interrupt_cb cb);

/**
 * \brief Unregister interrupt handler
 *
 * Additionally, interrupt is disabled after calling this function.
 * Any pending interrupt is cleared.
 *
 * \sa hw_gpadc_clear_interrupt
 *
 */
void hw_gpadc_unregister_interrupt(void);

/**
 * \brief Unregister interrupt handler without clearing the ADC engine interrupt.
 *        Only the NVIC interrupt is disabled and cleared.
 *
 * \sa hw_gpadc_unregister_interrupt
 * \sa hw_gpadc_clear_interrupt
 *
 */
void hw_gpadc_unregister_interrupt_no_clear(void);

/**
 * \brief Clear interrupt
 *
 * In case an interrupt handler has been registered, the application does not have to call
 * this function in the interrupt handler to clear the interrupt.
 * This is handled by the GPADC driver module.
 *
 * \sa hw_gpadc_register_interrupt
 *
 */
__STATIC_INLINE void hw_gpadc_clear_interrupt(void)
{
        GPADC->GP_ADC_CLEAR_INT_REG = 1;
}

/**
 * \brief Enable ADC
 *
 * Sampling is started after calling this function, to start conversion application should call
 * hw_gpadc_start().
 *
 * \sa hw_gpadc_start
 *
 */
__STATIC_INLINE void hw_gpadc_enable(void)
{
        /* Ensure that the LDO can be powered-up */
        ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, SNC_IS_UP));
        ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V14_EN));
        REG_SET_BIT(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN);
        while (0 == REG_GETF(CRG_TOP, ANA_STATUS_REG, FLAG_ADC_LDO_OK));        // Wait for LDO OK
}

/**
 * \brief Disable ADC
 *
 * Application should wait for conversion to be completed before disabling ADC. In case of
 * continuous mode, application should disable continuous mode and then wait for conversion to be
 * completed in order to have ADC in defined state.
 *
 * \sa hw_gpadc_in_progress
 * \sa hw_gpadc_set_continuous
 *
 */
__STATIC_INLINE void hw_gpadc_disable(void)
{
        REG_CLR_BIT(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN);
}

/**
 * \brief ADC enabled state
 *
 * \return true if ADC is enabled, false if disabled
 *
 */
__STATIC_INLINE bool hw_gpadc_is_enabled(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN);
}

/**
 * \brief Start conversion
 *
 * Application should not call this function while conversion is still in progress.
 *
 * \sa hw_gpadc_in_progress
 *
 */
__STATIC_INLINE void hw_gpadc_start(void)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_START, 1);
}

/**
 * \brief Check if conversion is in progress
 *
 * \return conversion state
 *
 */
__STATIC_INLINE bool hw_gpadc_in_progress(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_START);
}

/**
 * \brief Get raw ADC value.
 *
 * \sa HW_GPADC_RESULT_MODE
 *
 * \return raw ADC value.
 *
 * \note Neither correction nor conversion takes place.
 */
__STATIC_INLINE uint16_t hw_gpadc_get_raw_value(void)
{
        return GPADC->GP_ADC_RESULT_REG;
}

/**
 * \brief Get conversion result value with gain compensation and over sampling
 *
 * Invalid bits are discarded from result, i.e. oversampling is taken into account when calculating
 * value.
 *
 * \return conversion result value
 *
 * \sa hw_gpadc_get_raw_value
 * \sa hw_gpadc_set_oversampling
 * \sa hw_gpadc_apply_correction
 *
 */
uint16_t hw_gpadc_get_value(void);

/**
 * \brief Start the ADC conversion engine, providing a measurement
 *
 * \sa hw_gpadc_start
 * \sa hw_gpadc_in_progress
 *
 * \note The function polls the ADC engine waiting for the measurement to be ready.
 */
__STATIC_INLINE void hw_gpadc_adc_measure(void)
{
        ASSERT_ERROR(hw_gpadc_get_continuous() == false);
        hw_gpadc_start();
        while (hw_gpadc_in_progress());
}

/**
 * \brief Generic read function
 *        Follows \sa hw_gpadc_init() or \sa hw_gpadc_configure().
 *        According to the \sa gpadc_config passed in the above mentioned functions, the generic read function
 *        starts the ADC engine, delivers the requested conversions and stops the ADC engine when finished.
 *        If a callback is set by the user, the function operates in interrupt mode, otherwise in blocking mode.
 *        The results are always in raw format, which means they need post-processing to be converted to something valuable.
 *        To ensure all conversions are in place, the caller may poll for the falling of the GP_ADC_START bit via \sa hw_gpadc_in_progress.
 *        \sa hw_gpadc_get_value
 *        \sa hw_gpadc_convert_to_millivolt
 *        \sa hw_gpadc_apply_correction
 *
 * \param [in] nof_conv  number of conversions to be delivered. Must be non-zero
 * \param [out] out_buf  buffer to place the conversion results, NULL is allowed making the user responsible for fetching the converted results from the GP_ADC_RESULT_REG
 * \param [in] cb        user callback to execute when conversions are over, NULL for polling mode which blocks until conversions are over
 * \param [in] user_data parameter for callback
 *
 * \return true if conversions have started, false otherwise
 *
 * \note Interrupt mode can operate without an output buffer but never without a user callback.
 *
 * \note DMA mode can operate without a callback but never without an output buffer.
 *
 * \note If in \sa gpadc_config the \sa dma_setup section is valid, the converted results are transfered through DMA to the requested buffer.
 *       In this case the ADC interrupt in M33 is bypassed, unless there is deliberate extra handling by the user outside this function.
 *       At any given point, calling \sa hw_gpadc_abort_read will abandon the converting process, executing the user callback passed as argument.
 *
 */
bool hw_gpadc_read(uint32_t nof_conv, uint16_t *out_buf, hw_gpadc_read_cb cb, void *user_data);

/**
 * \brief Stop conversions
 *
 * Application can call this function to abort an ongoing read operation.
 * It is applicable only in when the ADC operates either in interrupt or DMA mode.
 *
 * \sa hw_gpadc_read
 *
 */
void hw_gpadc_abort_read(void);

/**
 * \brief Apply a fine trimming algorithm to the conversion result
 *
 * \param [in] cfg      configuration parameters for the conversion, NULL to use the current ADC settings
 * \param [in] raw      the raw result of the ADC conversion
 *
 * \return 16-bit left-aligned corrected value
 *
 */
uint16_t hw_gpadc_apply_correction(const gpadc_config *cfg, uint16_t raw);

/**
 * \brief Convert a GPADC raw measurement to voltage in mVolt.
 *        If no configuration is given, the current state of the GPADC
 *        control registers is used for the conversion parameters.
 *
 * \param [in] cfg      configuration parameters for the conversion, NULL to use the current ADC settings
 * \param [in] raw      the raw result of the ADC conversion
 *
 * \return voltage (mVolt)
 *
 * \sa hw_gpadc_apply_correction
 *
 */
int16_t hw_gpadc_convert_to_millivolt(const gpadc_config *cfg, uint16_t raw);

/**
 * \}
 */

/***************************************************************************
 ******************    TEMPERATURE SENSOR declarations   *******************
 ***************************************************************************/

/**
 * \addtogroup GPADC_TEMPSENS_FUNCTIONS Temperature sensor functions
 *
 * \brief Functions for temperature measurements and sensor calibration.
 *        The granularity of all temperature values is given in the level of Celsius degrees.
 * \{
 */

/**
 * \brief Calibration Data - (Temperature, 16-bit ADC value) pair
 */
typedef struct {
        int16_t temp;   /**< Temperature */
        uint16_t adc;   /**< ADC measurement in 16-bit resolution */
} hw_gpadc_calibration_point_t;

/**
 * \}
 */

#endif /* dg_configUSE_HW_GPADC */


#endif /* HW_GPADC_H */

/**
 * \}
 * \}
 */
