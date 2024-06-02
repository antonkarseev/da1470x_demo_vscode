/**
 * \addtogroup PLA_DRI_PER_ANALOG
 *
 * \{
 *
 * \addtogroup HW_SDADC SDADC Driver
 *
 * \brief Sigma Delta ADC
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_sdadc.h
 *
 * @brief Definition of API for the SDADC Low Level Driver.
 *
 * Copyright (C) 2018-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef HW_SDADC_H
#define HW_SDADC_H


#if dg_configUSE_HW_SDADC

#ifndef HW_SDADC_DMA_SUPPORT
#define HW_SDADC_DMA_SUPPORT                    ( 1 )
#endif

#if HW_SDADC_DMA_SUPPORT && !dg_configUSE_HW_DMA
#error "SDADC DMA support requires DMA hardware to be enabled. " \
       "Please revisit the application custom configuration."
#endif

#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"
#if HW_SDADC_DMA_SUPPORT
#include "hw_dma.h"

/**
 * \brief Cut-down to necessary DMA configuration
 */
typedef struct {
        HW_DMA_CHANNEL       channel;                   /**< DMA Channel Number to be used */
        HW_DMA_PRIO          prio;                      /**< Channel priority from 0 to 7 */
        uint32_t             dest;                      /**< Destination address */
        dma_size_t           len;                       /**< Number of DMA transfers */
        hw_dma_transfer_cb   cb;                        /**< Function to call after irq_nr_of_trans transfers */
        void                 *ud;                       /**< Data to pass to Callback */
} sdadc_dma_cfg;
#endif

/*=============================================================================================*/
/* Macro, type and data-structure definitions                                                  */
/*=============================================================================================*/

/**
 * \addtogroup SDADC_DATA SDADC Data Types
 *
 * \brief Enumeration, structure, type and macro definitions
 *
 * \{
 */


/**
 * \brief Sample mode
 *
 */
typedef enum {
        HW_SDADC_RESULT_SAMPLE_EXTENSION       = 0,     /**< Sample extension, SDADC_RESULT_REG = {sample[15:2], sample[2], sample[2]} */
        HW_SDADC_RESULT_SAMPLE_TRUNCATION      = 1,     /**< Sample truncation, SDADC_RESULT_REG = {0x00, sample[15:8]} */
        HW_SDADC_RESULT_SAMPLE_NORMAL          = 2,     /**< Normal mode (default), SDADC_RESULT_REG = sample[15:0] */
        HW_SDADC_RESULT_MODE_INVALID           = 3,     /**< N.A. */
} HW_SDADC_RESULT_MODE;

/**
 * \brief PGA gain selection
 *
 */
typedef enum {
        HW_SDADC_PGA_GAIN_MINUS_12dB = 0,               /**< 0 : -12 dB (default) */
        HW_SDADC_PGA_GAIN_MINUS_6dB = 1,                /**< 1 : -6 dB */
        HW_SDADC_PGA_GAIN_MINUS_0dB = 2,                /**< 2 : 0 dB */
        HW_SDADC_PGA_GAIN_6dB = 3,                      /**< 3 : 6 dB */
        HW_SDADC_PGA_GAIN_12dB = 4,                     /**< 4 : 12 dB */
        HW_SDADC_PGA_GAIN_18dB = 5,                     /**< 5 : 18 dB */
        HW_SDADC_PGA_GAIN_24dB = 6,                     /**< 6 : 24 dB */
        HW_SDADC_PGA_GAIN_30dB = 7,                     /**< 7 : 30 dB */
} HW_SDADC_PGA_GAIN;

/**
 * \brief PGA mode selection
 *
 */
typedef enum {
        HW_SDADC_PGA_MODE_DIFF = 0,                     /**< 0 : Differential mode (default) */
        HW_SDADC_PGA_MODE_SE_N = 1,                     /**< 1 : Use N-branch as single ended mode */
        HW_SDADC_PGA_MODE_DIFF2 = 2,                    /**< 2 : Differential mode */
        HW_SDADC_PGA_MODE_SE_P = 3,                     /**< 3 : Use P-branch as single ended mode */
} HW_SDADC_PGA_MODE;


/**
 * \brief PGA bias configuration
 *
 */
typedef enum {
        HW_SDADC_PGA_BIAS_40 = 0,                       /**< 0 :0.40 x Ibias */
        HW_SDADC_PGA_BIAS_44 = 1,                       /**< 1 :0.44 x Ibias */
        HW_SDADC_PGA_BIAS_50 = 2,                       /**< 2 :0.50 x Ibias */
        HW_SDADC_PGA_BIAS_57 = 3,                       /**< 3 :0.57 x Ibias */
        HW_SDADC_PGA_BIAS_66 = 4,                       /**< 4 :0.66 x Ibias (default) */
        HW_SDADC_PGA_BIAS_80 = 5,                       /**< 5 :0.80 x Ibias */
        HW_SDADC_PGA_BIAS_100 = 6,                      /**< 6 :1.00 x Ibias */
        HW_SDADC_PGA_BIAS_133 = 7,                      /**< 7 :1.33 x Ibias */
} HW_SDADC_PGA_BIAS;

/**
 * \brief PGA enabled branch(es)
 *
 */
typedef enum {
        HW_SDADC_PGA_ENABLE_NONE = 0,                   /**< 00 : both branches of PGA disabled */
        HW_SDADC_PGA_ENABLE_POSITIVE = 1,               /**< 01 : Positive branch of PGA enabled, Negative branch disabled */
        HW_SDADC_PGA_ENABLE_NEGATIVE = 2,               /**< 10 : Positive branch of PGA disabled, Negative branch enabled */
        HW_SDADC_PGA_ENABLE_BOTH = 3,                   /**< 11 : Both branches of PGA enabled */
} HW_SDADC_PGA_EN;

/**
 * \brief SDADC interrupt handler
 *
 */
typedef void (*hw_sdadc_interrupt_cb)(void);

/**
 * \brief SDADC configuration
 *
 */
typedef struct {
#if HW_SDADC_DMA_SUPPORT
        sdadc_dma_cfg          *dma_setup;              /**< DMA configuration - NULL to disable */
#endif
        bool                   mask_int;                /**< Enable/Disable (mask) SDADC interrupt */
        HW_SDADC_RESULT_MODE   result_mode;             /**< Sample mode for the ADC result */
        HW_SDADC_PGA_GAIN      pga_gain;                /**< PGA gain selection */
        HW_SDADC_PGA_MODE      pga_mode;                /**< PGA mode selection (differential/positive/negative) */
        HW_SDADC_PGA_BIAS      pga_bias;                /**< PGA bias selection */
        HW_SDADC_PGA_EN        pga_en;                  /**< PGA branch enabling */
} sdadc_config;

/**
 * \}
 */

/*
 * Necessary forward declaration
 */
static bool hw_sdadc_in_progress(void);

/*=============================================================================================*/
/* Configuring the SDADC                                                                       */
/*=============================================================================================*/

/**
 * \addtogroup SDADC_CONFIGURATION Configuration options for the SDADC
 *
 * \brief Access to specific sdadc_config structure members and other essential configuration
 *
 * \{
 */

/**
 * \brief Enable SDADC interrupt
 *
 */
__STATIC_INLINE void hw_sdadc_enable_interrupt(void)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MINT, 1);
}

/**
 * \brief Disable SDADC interrupt
 *
 */
__STATIC_INLINE void hw_sdadc_disable_interrupt(void)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MINT, 0);
}

/**
 * \brief Get the status of the SDADC maskable interrupt (MINT) to the CPU
 *
 * \return SDADC maskable interrupt (MINT) status
 *
 */
__STATIC_INLINE bool hw_sdadc_is_interrupt_enabled(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_MINT);
}

/**
 * \brief Enable/Disable DMA functionality
 *
 * \param [in] enabled When true, DMA functionality is enabled
 *
 */
__STATIC_INLINE void hw_sdadc_set_dma_functionality(bool enabled)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_DMA_EN, !!enabled);
}

/**
 * \brief Get DMA functionality state
 *
 * \return DMA functionality state
 *
 */
__STATIC_INLINE bool hw_sdadc_get_dma_functionality(void)
{
        return (bool) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_DMA_EN);
}

/**
 * \brief Get the result register value.
 *
 * \return result value
 *
 */
__STATIC_INLINE uint16_t hw_sdadc_read_result_register(void)
{
        return REG_GETF(SDADC, SDADC_RESULT_REG, SDADC_VAL);
}


/**
 * \brief Enable the audio filter, this bit needs to be set before the SDADC_START bit is set to '1'.
 *
 * \sa hw_sdadc_start
 *
 */
__STATIC_INLINE void hw_sdadc_audio_filter_enable(void)
{
        ASSERT_WARNING(hw_sdadc_in_progress() == false);
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_AUDIO_FILTER_EN, 1);
}

/**
 * \brief  Disable audio filter, ADC is forced into reset.
 *         When setting to '0' while SDADC_START = '1',
 *         the last sample is completed first before disabling.
 */
__STATIC_INLINE void hw_sdadc_audio_filter_disable(void)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_AUDIO_FILTER_EN, 0);
}

/**
 * \brief Set store-mode for the converted samples
 *
 * \param [in] mode sample mode
 */
__STATIC_INLINE void hw_sdadc_set_result_mode(HW_SDADC_RESULT_MODE mode)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_RESULT_MODE, mode);
}

/**
 * \brief Get current result mode
 *
 * \return current store-mode for the converted samples
 *
 */
__STATIC_INLINE HW_SDADC_RESULT_MODE hw_sdadc_get_result_mode(void)
{
        return (HW_SDADC_RESULT_MODE) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_RESULT_MODE);
}

/**
 * \}
 */


/*=============================================================================================*/
/* Configuring the PGA in SDADC                                                                */
/*=============================================================================================*/

/**
 * \addtogroup SDADC_PGA Programmable Gain Amplifier
 *
 * \brief Controlling the PGA in SDADC
 *
 * \{
 */

/**
 * \brief Set the PGA gain
 *
 * \param [in] gain gain of the PGA
 */
__STATIC_INLINE void hw_sdadc_pga_set_gain(HW_SDADC_PGA_GAIN gain)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_GAIN, gain);
}

/**
 * \brief Get the PGA gain status
 *
 * \return PGA current gain
 */
__STATIC_INLINE HW_SDADC_PGA_GAIN hw_sdadc_pga_get_gain(void)
{
        return (HW_SDADC_PGA_GAIN) REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_GAIN);
}

/**
 * \brief Set the PGA branch mode. Use PGA in single ended/differential mode
 *
 * \param [in] mode mode of the PGA positive and negative branches
 *
 */
__STATIC_INLINE void hw_sdadc_pga_set_mode(HW_SDADC_PGA_MODE mode)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_MODE, mode);
}

/**
 * \brief Get the PGA branch mode status
 *
 * \return PGA current mode of the positive and negative branches
 */
__STATIC_INLINE HW_SDADC_PGA_MODE hw_sdadc_pga_get_mode(void)
{
        return (HW_SDADC_PGA_MODE) REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_MODE);
}

/**
 * \brief Mute the PGA
 *
 */
__STATIC_INLINE void hw_sdadc_pga_mute(void)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_MUTE, 1);
}

/**
 * \brief Un-mute the PGA
 *
 */
__STATIC_INLINE void hw_sdadc_pga_unmute(void)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_MUTE, 0);
}

/**
 * \brief Get the PGA mute status
 *
 * \return PGA is in mute or unmute state
 */
__STATIC_INLINE bool hw_sdadc_pga_is_mute(void)
{
        return REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_MUTE);
}

/**
 * \brief Set the PGA bias
 *
 * \param [in] bias PGA bias control value
 */
__STATIC_INLINE void hw_sdadc_pga_set_bias(HW_SDADC_PGA_BIAS bias)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_BIAS, bias);
}

/**
 * \brief Get the PGA bias status
 *
 * \return PGA current bias
 */
__STATIC_INLINE HW_SDADC_PGA_BIAS hw_sdadc_pga_get_bias(void)
{
        return (HW_SDADC_PGA_BIAS) REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_BIAS);
}


/**
 * \brief Short the PGA input channels
 *
 */
__STATIC_INLINE void hw_sdadc_pga_short_inputs(void)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_SHORTIN, 1);
}

/**
 * \brief Disconnect the short-circuited PGA input channels
 *
 */
__STATIC_INLINE void hw_sdadc_pga_unshort_inputs(void)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_SHORTIN, 0);
}

/**
 * \brief Get the PGA short-input status
 *
 * \return true if input channels are shorted, false otherwise
 */
__STATIC_INLINE bool hw_sdadc_pga_inputs_are_shorted(void)
{
        return REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_SHORTIN);
}

/**
 * \brief Select the enabled PGA input channel(s)
 *
 * \param [in] channels selection of channel to enable
 */
__STATIC_INLINE void hw_sdadc_pga_select_enabled_channels(HW_SDADC_PGA_EN channels)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_EN, channels);
}

/**
 * \brief Get the PGA enabled input-channel status
 *
 * \return PGA current gain
 */
__STATIC_INLINE HW_SDADC_PGA_GAIN hw_sdadc_pga_enabled_channels_status(void)
{
        return (HW_SDADC_PGA_GAIN) REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_EN);
}

/**
 * \brief Set the audio filter register
 *
 * Constant CIC offset
 *
 * \param [in] val
 *
 */
__STATIC_INLINE void hw_sdadc_set_cic_offset(uint32_t val)
{
        ASSERT_WARNING(hw_sdadc_in_progress() == false);
        REG_SETF(SDADC, SDADC_AUDIO_FILT_REG, SDADC_CIC_OFFSET, val);
}

/**
 * \brief Get the audio filter register status
 *
 * Constant CIC offset
 *
 * \return cic offset
 *
 */
__STATIC_INLINE uint32_t hw_sdadc_get_cic_offset(void)
{
        return REG_GETF(SDADC, SDADC_AUDIO_FILT_REG, SDADC_CIC_OFFSET);
}

/**
 * \}
 */

/*=============================================================================================*/
/* Basic functionality of the SDADC                                                            */
/*=============================================================================================*/

/**
 * \addtogroup SDADC_BASIC Basic SDADC Functionality
 *
 * \brief Initialization, configuration, measurement and voltage conversion functions
 *
 * \{
 */

/**
 * \brief Initialize SDADC
 *
 * Sets the SDADC control register to default values and then calls
 * the configuration function. It also disables and clears pending SDADC interrupts.
 *
 * \p cfg can be NULL - no configuration is performed in such case.
 *
 * \param [in] cfg configuration
 *
 * \sa hw_sdadc_configure
 *
 */
void hw_sdadc_init(const sdadc_config *cfg);

/**
 * \brief De-initialize SDADC
 *
 * Sets the SDADC control register to default values.
 * It also disables and clears pending SDADC interrupts.
 *
 */
void hw_sdadc_deinit(void);

/**
 * \brief Configure SDADC
 *
 * Shortcut to call appropriate configuration function. If \p cfg is NULL, this function does
 * nothing.
 *
 * \param [in] cfg configuration
 *
 */
void hw_sdadc_configure(const sdadc_config *cfg);

/**
 * \brief Reset SDADC to its default values without disabling the LDO.
 *
 */
void hw_sdadc_reset(void);

/**
 * \brief Register interrupt handler
 *
 * Interrupt is enabled after calling this function. Application is responsible for clearing
 * interrupt using hw_sdadc_clear_interrupt(). If no callback is specified interrupt is cleared by
 * driver.
 *
 * \param [in] cb callback fired on interrupt
 *
 * \sa hw_sdadc_clear_interrupt
 *
 */
void hw_sdadc_register_interrupt(hw_sdadc_interrupt_cb cb);

/**
 * \brief Unregister interrupt handler
 *
 * Interrupt is disabled after calling this function.
 *
 */
void hw_sdadc_unregister_interrupt(void);

/**
 * \brief Clear interrupt
 *
 * Application should call this in interrupt handler to clear interrupt.
 *
 * \sa hw_sdadc_register_interrupt
 *
 */
__STATIC_INLINE void hw_sdadc_clear_interrupt(void)
{
        REG_SETF(SDADC, SDADC_CLEAR_INT_REG, SDADC_CLR_INT, 1);
}

/**
 * \brief Enable SDADC
 *
 * This function enables the SDADC. LDO, bias currents and modulator are enabled.
 * To start a conversion, the application should call hw_sdadc_start().
 *
 * \sa hw_sdadc_start
 *
 */
__STATIC_INLINE void hw_sdadc_enable(void)
{
        /* Ensure that the LDO can be powered-up */
        ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_CTRL_REG, DCDC_V18P_EN));
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_EN, 1);
        while (0 == REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_LDO_OK));       // Wait for LDO OK
}

/**
 * \brief Disable SDADC
 *
 * Application should wait for conversion to be completed before disabling SDADC. In case of
 * continuous mode, application should disable continuous mode and then wait for conversion to be
 * completed in order to have SDADC in defined state.
 *
 * \sa hw_sdadc_in_progress
 * \sa hw_sdadc_set_continuous
 *
 */
__STATIC_INLINE void hw_sdadc_disable(void)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_EN, 0);
}

/**
 * \brief Get the enable status of the SDADC
 *
 * \return SDADC enable status
 *
 */
__STATIC_INLINE bool hw_sdadc_is_enabled(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_EN);
}

/**
 * \brief Start the ADC engine in continuous conversion mode
 *
 */
__STATIC_INLINE void hw_sdadc_start(void)
{
        hw_sdadc_audio_filter_enable();
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_START, 1);
}

/**
 * \brief Check if conversion is in progress
 *
 * \return conversion state
 *
 */
__STATIC_INLINE bool hw_sdadc_in_progress(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_START);
}

/**
 * \brief Put ADC to idle state, stopping continuous conversions
 *
 */
__STATIC_INLINE void hw_sdadc_stop(void)
{
        hw_sdadc_audio_filter_disable();
        while (hw_sdadc_in_progress());
}

/**
 * \}
 */

#endif /* dg_configUSE_HW_SDADC */


#endif /* HW_SDADC_H_ */
/**
 * \}
 * \}
 */
