/**
 ****************************************************************************************
 *
 * @file hw_gpadc_v2.h
 *
 * @brief Definition of API for the GPADC Low Level Driver
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef HW_GPADC_V2_H
#define HW_GPADC_V2_H


#if dg_configUSE_HW_GPADC

#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"

/***************************************************************************
 *********    Macro, type and data-structure definitions     ***************
 ***************************************************************************/

/**
 * \addtogroup GPADC_DATA
 *
 * \{
 */

/**
 * \brief Recommended sample time setting for accurate temperature measurements with DIE_TEMP
 *
 */
#define HW_GPADC_DIE_TEMP_SMPL_TIME             0x0F

/**
 * \brief Delay for enabling the ADC after enabling the LDO when ADC input is the temperature sensor
 *
 * HW_GPADC_TEMPSENS_INIT_DELAY * 4 * ADC_CLK period should be > 25usec
 *
 */
#define HW_GPADC_TEMPSENS_INIT_DELAY            0x68  /* 26 usec with a clock speed of (DivN_clk / 2) */

/**
 * \brief GPADC input voltages
 *
 */
typedef enum {
        HW_GPADC_INPUT_VOLTAGE_UP_TO_0V9 = 0,   /**< input voltages up to 0.9 V are allowed */
        HW_GPADC_INPUT_VOLTAGE_UP_TO_1V8 = 1,   /**< input voltages up to 1.8 V are allowed */
        HW_GPADC_INPUT_VOLTAGE_UP_TO_2V7 = 2,   /**< input voltages up to 2.7 V are allowed */
        HW_GPADC_INPUT_VOLTAGE_UP_TO_3V6 = 3,   /**< input voltages up to 3.6 V are allowed */
} HW_GPADC_MAX_INPUT_VOLTAGE;

/**
 * \brief GPADC Reference Voltage Level
 *
 */
#define HW_GPADC_VREF_MILLIVOLT                 (900)

/**
 * \brief Store delay
 *
 * \note Values 1-3 are reserved
 *
 */
typedef enum {
        HW_GPADC_STORE_DEL_0         = 0x0,     /**< Data is stored after handshake synchronization */
        HW_GPADC_STORE_DEL_5_CYCLES  = 0x4,     /**< Data is stored 5 ADC_CLK cycles after internal start trigger */
        HW_GPADC_STORE_DEL_6_CYCLES  = 0x5,     /**< Data is stored 6 ADC_CLK cycles after internal start trigger */
        HW_GPADC_STORE_DEL_7_CYCLES  = 0x6,     /**< Data is stored 7 ADC_CLK cycles after internal start trigger */
        HW_GPADC_STORE_DEL_8_CYCLES  = 0x7,     /**< Data is stored 8 ADC_CLK cycles after internal start trigger */
} HW_GPADC_STORE_DELAY;

/*
 *  ADC input to GPIO pin mapping
 */
typedef enum {
        HW_GPADC_INPUT_ADC0 = 0,
        HW_GPADC_INPUT_ADC1,
        HW_GPADC_INPUT_ADC2,
        HW_GPADC_INPUT_ADC3,
} HW_GPADC_GPIO_INPUT;

/**
 * \brief ADC input MUX1 selector
 *
 */
typedef enum {
        HW_GPADC_INPUT_MUX1_NONE = 0,                   /**< No rail selected */
        HW_GPADC_INPUT_MUX1_NC,                         /**< NC */
        HW_GPADC_INPUT_MUX1_RES1,                       /**< Reserved */
        HW_GPADC_INPUT_MUX1_I_SENSE_BUS,                /**< I_sense_bus */
        HW_GPADC_INPUT_MUX1_RES2,                       /**< Reserved */
        HW_GPADC_INPUT_MUX1_V30,                        /**< V30 */
        HW_GPADC_INPUT_MUX1_RES3,                       /**< Reserved */
        HW_GPADC_INPUT_MUX1_V18F,                       /**< V18F */
} HW_GPADC_INPUT_MUX1;

/**
 * \brief ADC input MUX2 selector
 *
 */
typedef enum {
        HW_GPADC_INPUT_MUX2_NONE = 0,                   /**< No rail selected */
        HW_GPADC_INPUT_MUX2_V12,                        /**< V12 */
        HW_GPADC_INPUT_MUX2_V18,                        /**< V18 */
        HW_GPADC_INPUT_MUX2_V14,                        /**< V14 */
        HW_GPADC_INPUT_MUX2_V18P,                       /**< V18P */
        HW_GPADC_INPUT_MUX2_VSYS,                       /**< VSYS monitor following a 0.186 scaler */
        HW_GPADC_INPUT_MUX2_VBUS,                       /**< VBUS monitor following a 0.164 scaler */
        HW_GPADC_INPUT_MUX2_VBAT,                       /**< VBAT monitor following a 0.189 scaler */
} HW_GPADC_INPUT_MUX2;

#define HW_GPADC_INP_Msk                                (0x07)

/**
 * \brief MUX1 Bit
 *        If this bit is set, then the positive input register field is MUX1 (0x04)
 *        and the MUX1 register field defines the input channel.
 */
#define HW_GPADC_INP_MUX1_Bit                           (0x08)

/**
 * \brief MUX2 bit
 *        If this bit is set, then the positive input register field is MUX2 (0x06)
 *        and the MUX2 register field defines the input channel.
 */
#define HW_GPADC_INP_MUX2_Bit                           (0x10)

/**
 * \brief ADC input - Positive side
 *
 */
typedef enum {
        HW_GPADC_INP_P0_5  = HW_GPADC_INPUT_ADC0,       /**< GPIO 0.5 */
        HW_GPADC_INP_P0_6  = HW_GPADC_INPUT_ADC1,       /**< GPIO 0.6 */
        HW_GPADC_INP_P0_27 = HW_GPADC_INPUT_ADC2,       /**< GPIO 0.27 */
        HW_GPADC_INP_P0_30 = HW_GPADC_INPUT_ADC3,       /**< GPIO 0.30 */
        HW_GPADC_INP_MUX1,                              /**< MUX1 */
        HW_GPADC_INP_DIFF_TEMP,                         /**< DIFF temp */
        HW_GPADC_INP_MUX2,                              /**< MUX2 */
        HW_GPADC_INP_DIE_TEMP,                          /**< DIE temp */
        HW_GPADC_INP_NC =          HW_GPADC_INP_MUX1_Bit | HW_GPADC_INPUT_MUX1_NC,
        HW_GPADC_INP_I_SENSE_BUS = HW_GPADC_INP_MUX1_Bit | HW_GPADC_INPUT_MUX1_I_SENSE_BUS,
        HW_GPADC_INP_V30 =         HW_GPADC_INP_MUX1_Bit | HW_GPADC_INPUT_MUX1_V30,
        HW_GPADC_INP_V18F =        HW_GPADC_INP_MUX1_Bit | HW_GPADC_INPUT_MUX1_V18F,
        HW_GPADC_INP_V12 =         HW_GPADC_INP_MUX2_Bit | HW_GPADC_INPUT_MUX2_V12,
        HW_GPADC_INP_V18 =         HW_GPADC_INP_MUX2_Bit | HW_GPADC_INPUT_MUX2_V18,
        HW_GPADC_INP_V14 =         HW_GPADC_INP_MUX2_Bit | HW_GPADC_INPUT_MUX2_V14,
        HW_GPADC_INP_V18P =        HW_GPADC_INP_MUX2_Bit | HW_GPADC_INPUT_MUX2_V18P,
        HW_GPADC_INP_VSYS =        HW_GPADC_INP_MUX2_Bit | HW_GPADC_INPUT_MUX2_VSYS,
        HW_GPADC_INP_VBUS =        HW_GPADC_INP_MUX2_Bit | HW_GPADC_INPUT_MUX2_VBUS,
        HW_GPADC_INP_VBAT =        HW_GPADC_INP_MUX2_Bit | HW_GPADC_INPUT_MUX2_VBAT,
} HW_GPADC_INPUT_POSITIVE;

/**
 * \brief ADC input - Negative side
 *
 */
typedef enum {
        HW_GPADC_INN_P0_5  = HW_GPADC_INPUT_ADC0,       /**< GPIO 0.5 */
        HW_GPADC_INN_P0_6  = HW_GPADC_INPUT_ADC1,       /**< GPIO 0.6 */
        HW_GPADC_INN_P0_27 = HW_GPADC_INPUT_ADC2,       /**< GPIO 0.27 */
        HW_GPADC_INN_P0_30 = HW_GPADC_INPUT_ADC3,       /**< GPIO 0.30 */
        /* All other combinations are reserved */
} HW_GPADC_INPUT_NEGATIVE;

/**
 * \brief Sample mode controlling the LSB's of the stored result
 *
 */
typedef enum {
        HW_GPADC_RESULT_EXTENDED        = 0,    /**< Sample extension, the result is aligned on the MSBs. The lowest calculated LSB is extended over the unused bits */
        HW_GPADC_RESULT_TRUNCATED       = 1,    /**< Sample truncation, the result is aligned on the 8 LSBs. Any additional accuracy isn't available */
        HW_GPADC_RESULT_NORMAL          = 2,    /**< Normal mode, the result is aligned on the MSBs. Any unused LSBs are kept zero */
        HW_GPADC_RESULT_MODE_INVALID    = 3,    /**< N.A. */
} HW_GPADC_RESULT_MODE;

/**
 * \brief ADC configuration
 *
 */
typedef struct {
        HW_GPADC_INPUT_MODE        input_mode;          /**< input mode */
        HW_GPADC_INPUT_POSITIVE    positive;            /**< positive channel */
        HW_GPADC_INPUT_NEGATIVE    negative;            /**< negative channel */
        HW_GPADC_TEMP_SENSORS      temp_sensor;         /**< DIFF temperature sensor selection */
        uint8_t                    sample_time;         /**< sample time, range: 0-15, time = (sample_time x 8) ADC_CLK cycles */
        bool                       continuous;          /**< continuous mode state */
        uint8_t                    interval;            /**< interval between conversions in continuous mode */
        HW_GPADC_MAX_INPUT_VOLTAGE input_attenuator;    /**< input attenuator regulates the maximum measured input voltage */
        bool                       chopping;            /**< chopping state */
        HW_GPADC_OVERSAMPLING      oversampling;        /**< oversampling rate */
        HW_GPADC_RESULT_MODE       result_mode;         /**< result mode */
#if HW_GPADC_DMA_SUPPORT
        gpadc_dma_cfg              *dma_setup;          /**< DMA configuration - NULL to disable */
#endif
} gpadc_config;

/**
 * \}
 */

/***************************************************************************
 ****************      GP_ADC configuration functions    *******************
 ***************************************************************************/

/**
 * \addtogroup GPADC_CONFIG_FUNCTIONS
 *
 * \{
 */

/**
 * \brief Set the delay required to enable the ADC_LDO.
 *        0: Not allowed
 *        1: 4x ADC_CLK period.
 *        n: n*4x ADC_CLK period.
 *
 * param [in] LDO enable delay
 *
 */
__STATIC_INLINE void hw_gpadc_set_ldo_delay(uint32_t delay)
{
        /* Zero delay is not allowed by the h/w specification */
        ASSERT_ERROR(delay != 0);
        REG_SETF(GPADC, GP_ADC_CTRL3_REG, GP_ADC_EN_DEL, delay);
}

/**
 * \brief Set STORE_DEL field
 *
 * 0: Data is stored after handshake synchronization
 * 1-3: Reserved
 * 4: Data is stored 5 ADC_CLK cycles after internal start trigger
 * 7: Data is stored 8 ADC_CLK cycles after internal start trigger
 *
 * \param [in] delay store delay setting
 *
 * \note The application should be very careful with this bitfield as it could easily
 * read outdated conversion data if the value is set too optimistic.
 * Setting it too pessimistic is only slowing down the conversion time.
 * The zero default value is strongly recommended to be used.
 */
 __STATIC_INLINE void hw_gpadc_set_store_delay(HW_GPADC_STORE_DELAY delay)
 {
         ASSERT_ERROR((delay == HW_GPADC_STORE_DEL_0) ||
                      ((delay >= HW_GPADC_STORE_DEL_5_CYCLES) && (delay <= HW_GPADC_STORE_DEL_8_CYCLES)));
         REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_STORE_DEL, delay);
 }

/**
 * \brief Set positive input channel
 *
 * \param [in] channel positive input channel
 *
 */
__STATIC_INLINE void hw_gpadc_set_positive(HW_GPADC_INPUT_POSITIVE channel)
{
        uint32_t mux_channel = channel & HW_GPADC_INP_Msk;

        if (channel & HW_GPADC_INP_MUX1_Bit) {
                ASSERT_WARNING(mux_channel != HW_GPADC_INPUT_MUX1_NONE);
                REG_SETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_MUX1, mux_channel);
                channel = HW_GPADC_INP_MUX1;
        } else if (channel & HW_GPADC_INP_MUX2_Bit) {
                ASSERT_WARNING(mux_channel != HW_GPADC_INPUT_MUX2_NONE);
                REG_SETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_MUX2, mux_channel);
                channel = HW_GPADC_INP_MUX2;
        }

        REG_SETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_P, channel);
}

/**
 * \brief Get the current positive input channel
 *
 * \return positive input channel
 *
 */
__STATIC_INLINE HW_GPADC_INPUT_POSITIVE hw_gpadc_get_positive(void)
{
        HW_GPADC_INPUT_POSITIVE channel = REG_GETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_P);
        if (channel == HW_GPADC_INP_MUX1) {
                return REG_GETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_MUX1) | HW_GPADC_INP_MUX1_Bit;
        } else if (channel == HW_GPADC_INP_MUX2) {
                return REG_GETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_MUX2) | HW_GPADC_INP_MUX2_Bit;
        }

        return REG_GETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_P);
}

/**
 * \brief Set negative input channel
 *
 * \param [in] channel negative input channel
 *
 */
__STATIC_INLINE void hw_gpadc_set_negative(HW_GPADC_INPUT_NEGATIVE channel)
{
        REG_SETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_N, channel);
}

/**
 * \brief Get the current negative input channel
 *
 * \return negative input channel
 *
 */
__STATIC_INLINE HW_GPADC_INPUT_NEGATIVE hw_gpadc_get_negative(void)
{
        return REG_GETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_N);
}

/**
 * \brief Set state of input attenuator
 *
 * Enabling the internal attenuator scales input voltage, increasing the effective input
 * scale from 0-1.2V to 0-3.6V in single ended mode or from -1.2-1.2V to -3.6-3.6V in differential
 * mode.
 *
 * \param [in] vmax attenuator state
 *
 */
__STATIC_INLINE void hw_gpadc_set_input_attenuator_state(HW_GPADC_MAX_INPUT_VOLTAGE vmax)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_ATTN, vmax);
}

/**
 * \brief Get the current state of input attenuator
 *
 * \return attenuator state
 *
 */
__STATIC_INLINE HW_GPADC_MAX_INPUT_VOLTAGE hw_gpadc_get_input_attenuator_state(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_ATTN);
}

/**
 * \brief Set sample time
 *
 * Sample time is \p mult x 8 clock cycles or 1 clock cycle when \p mult is 0. Valid values are
 * 0-15.
 *
 * \param [in] mult multiplier
 *
 */
__STATIC_INLINE void hw_gpadc_set_sample_time(uint8_t mult)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_SMPL_TIME, mult);
}

/**
 * \brief Get the current sample time.
 *        The sample time is calculated, based on this register field value.
 *
 * \return multiplier (sample time = multiplier x 8 x ADC_CLK)
 *
 * \sa hw_gpadc_set_sample_time
 */
__STATIC_INLINE uint8_t hw_gpadc_get_sample_time(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_SMPL_TIME);
}

/**
 * \brief Set DIE_TEMP_EN field
 *
 * Enables the die-temperature sensor. Output can be measured on GPADC input 4.
 *
 * \param [in] enabled enable/disable the die-temperature sensor
 *
 * \sa HW_GPADC_INPUT_POSITIVE
 */
__STATIC_INLINE void hw_gpadc_set_die_temp(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIE_TEMP_EN, !!enabled);
}

/**
 * \brief Get the current status of the die-temperature sensor. Output can be measured on GPADC input 4.
 *
 * \return current die-temperature sensor status
 *
 * \sa HW_GPADC_INPUT_POSITIVE
 *
 */
__STATIC_INLINE bool hw_gpadc_get_die_temp(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIE_TEMP_EN);
}

/**
 * \brief Set the mode of bandgap reference
 *
 * 0: GPADC LDO tracking bandgap reference (default)
 * 1: GPADC LDO hold sampled bandgap reference
 *
 * \param [in] enabled ldo bandgap reference mode
 *
 */
__STATIC_INLINE void hw_gpadc_set_ldo_hold(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_LDO_HOLD, !!enabled);
}

/**
 * \brief Get the current mode of bandgap reference
 *
 * \return current ldo bandgap reference mode
 *
 * \sa hw_gpadc_set_ldo_hold
 *
 */
__STATIC_INLINE bool hw_gpadc_get_ldo_hold(void)
{
        return (bool) REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_LDO_HOLD);
}

/**
 * \brief Set LDO_LEVEL field offset
 *
 * 0:   0 Offset (default)
 * 1:  +6mV
 * 2: +12mV
 * 3: +18mV
 * 4: -24mV
 * 5: -18mV
 * 6: -12mV
 * 7:  -6mV
 *
 * \param [in] val GPADC LDO level
 *
 */
__STATIC_INLINE void hw_gpadc_set_ldo_level(uint32_t val)
{
        REG_SETF(GPADC, GP_ADC_TRIM_REG, GP_ADC_LDO_LEVEL, val);
}

/*
 * \brief Get LDO_LEVEL field
 *
 * \return LDO level
 *
 * \sa hw_gpadc_set_ldo_level
 *
 */
__STATIC_INLINE uint32_t hw_gpadc_get_ldo_level(void)
{
        return REG_GETF(GPADC, GP_ADC_TRIM_REG, GP_ADC_LDO_LEVEL);
}

/**
 * \brief Set the result mode for the stored samples
 *
 * \param [in] mode result mode
 *
 */
__STATIC_INLINE void hw_gpadc_set_result_mode(HW_GPADC_RESULT_MODE mode)
{
        ASSERT_WARNING(mode < HW_GPADC_RESULT_MODE_INVALID);
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_RESULT_MODE, mode);
}

/**
 * \brief Get the result mode of the stored samples
 *
 * \return the current result mode
 *
 */
__STATIC_INLINE HW_GPADC_RESULT_MODE hw_gpadc_get_result_mode(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_RESULT_MODE);
}

/**
 * \}
 */

/***************************************************************************
 ****************    Basic functionality of the GPADC    *******************
 ***************************************************************************/

/**
 * \addtogroup GPADC_FUNCTIONS
 *
 * \{
 */

/**
 * \brief Get the measured voltage in mVolt
 *
 * \return voltage (mVolt)
 *
 * \sa hw_gpadc_get_value
 *
 */
int16_t hw_gpadc_get_voltage(void);

/**
 * \brief Calculate the compensation of the inherent scalers attached to internal channels
 *
 * Some internal channels have their own inherent hardware scalers.
 * This utility function helps converting the adc result to voltage.
 *
 * \param [in] channel input channel
 * \param [in] adc_val initial adc value
 *
 * \return compensated adc value
 */
__STATIC_INLINE uint32_t hw_gpadc_internal_scaler_compensate(HW_GPADC_INPUT_POSITIVE channel, uint16_t adc_val)
{
        if (channel == HW_GPADC_INP_VSYS) {
                /* Scaler 0.157 x VSYS */
                return (adc_val * 51) / 8;
        } else if (channel == HW_GPADC_INP_VBUS) {
                /* Scaler 0.164 x VBUS */
                return (adc_val * 214) / 35;
        } else if (channel == HW_GPADC_INP_VBAT) {
                /* Scaler 0.189 x VBAT */
                return (adc_val * 37) / 7;
        }
        return adc_val;
}

/**
 * \}
 */

/***************************************************************************
 ******************      TEMPERATURE SENSOR functions  *********************
 ***************************************************************************/

/**
 * \addtogroup GPADC_TEMPSENS_FUNCTIONS
 *
 * \{
 */

/**
 * \brief Convert a 16-bit, left-aligned, raw value to temperature.
 *        For accurate conversions using this function the ADC
 *        should operate in the following configuration:
 *
 *        Positive and negative offset registers = Default (0x200 uncalibrated),
 *        SampleTime = 0x02,
 *        Oversampling = 64 Samples,
 *        Chopping = Enabled and
 *        Attenuator = Disabled.
 *
 * \param [in] cfg      GPADC configuration, NULL to use the current ADC settings
 * \param [in] raw_val  digital GPADC value
 *
 * \return temperature in hundredths of Celsius degrees (ex. 2540 = 25.4 C)
 *
 * \sa gpadc_config
 * \sa hw_gpadc_set_offset_positive
 * \sa hw_gpadc_set_offset_negative
 */
int16_t hw_gpadc_convert_to_celsius_x100_util(const gpadc_config *cfg, uint16_t raw_val);

/**
 * \brief Convert a 16-bit, left-aligned, raw ADC digital value to temperature.
 *        The conversion uses the current GPADC configuration settings.
 *        For accurate conversions using this function the ADC
 *        should operate in the following configuration:
 *
 *        Positive and negative offset registers = Default (0x200 uncalibrated),
 *        SampleTime = 0x02,
 *        Oversampling = 64 Samples,
 *        Chopping = Enabled and
 *        Attenuator = Disabled.
 *
 * \param [in] adc_val  digital GPADC value
 *
 * \return temperature in hundredths of Celsius degrees (ex. 2540 = 25.4 C)
 *
 * \sa gpadc_config
 * \sa hw_gpadc_set_offset_positive
 * \sa hw_gpadc_set_offset_negative
 *
 * \deprecated This function is deprecated. User should call hw_gpadc_convert_to_celsius_x100_util() instead.
 *
 */
DEPRECATED_MSG("API no longer supported, use hw_gpadc_convert_to_celsius_x100_util() instead.")
__STATIC_INLINE int16_t hw_gpadc_convert_to_celsius_x100(uint16_t adc_val)
{
        return hw_gpadc_convert_to_celsius_x100_util(NULL, adc_val);
}

/**
 * \brief Convert a temperature value to raw GPADC value.
 *
 * \param [in] cfg         GPADC configuration, NULL to use the current ADC settings
 * \param [in] temperature temperature in hundredths of Celsius degrees (ex. 2540 = 25.4 C)
 *
 * \return 16-bit left-aligned ADC value (raw)
 */
uint16_t hw_gpadc_convert_celsius_x100_to_raw_val_util(const gpadc_config *cfg, int16_t temperature);

/**
 * \brief Convert a temperature value to raw GPADC value.
 *        The conversion uses the current GPADC configuration settings
 *
 * \param [in] temperature temperature in hundredths of Celsius degrees (ex. 2540 = 25.4 C)
 *
 * \return 16-bit left-aligned ADC value (raw)
 *
 * \deprecated This function is deprecated. User should call hw_gpadc_convert_celsius_x100_to_raw_val_util() instead. *
 */
DEPRECATED_MSG("API no longer supported, use hw_gpadc_convert_celsius_x100_to_raw_val_util() instead.")
__STATIC_INLINE uint16_t hw_gpadc_convert_celsius_x100_to_raw_val(int16_t temperature)
{
        return hw_gpadc_convert_celsius_x100_to_raw_val_util(NULL, temperature);
}

/**
 * \brief Store temperature calibration point at ambient temperature
 *
 * \param [in] raw_val ADC calibration value in 16-bit resolution
 * \param [in] temp    temperature in (Celsius degrees x 100)
 *
 */
void hw_gpadc_store_ambient_calibration_point(uint16_t raw_val, int16_t temp);

/**
 * \}
 */

#endif /* dg_configUSE_HW_GPADC */
#endif /* HW_GPADC_V2_H */
