
/**
 * \addtogroup PLA_DRI_PER_AUDIO
 * \{
 * \addtogroup HW_PCM_AUDIO PCM Audio Interface Driver
 * \{
 * \brief PCM interface
 */

/**
 ****************************************************************************************
 *
 * @file hw_pcm.h
 *
 * @brief Definition of API for the PCM interface Low Level Driver.
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_PCM_H_
#define HW_PCM_H_

#if dg_configUSE_HW_PCM

#include "sdk_defs.h"

/**
 * \brief Get the value of a field of a PCM register of CRG.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to read
 *
 * \return the value of the register field
 *
 */
#define HW_PCM_CRG_REG_GETF(reg, field)         REG_GETF(CRG_AUD, PCM_##reg##_REG, field)

/**
 * \brief Set the value of a field of a PCM register of CRG.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] val is the value to write
 *
 */
#define HW_PCM_CRG_REG_SETF(reg, field, val)    REG_SETF(CRG_AUD, PCM_##reg##_REG, field, val)

/**
 * \brief Set a bit of a PCM register of CRG.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 */
#define HW_PCM_CRG_REG_SET_BIT(reg, field)      REG_SET_BIT(CRG_AUD, PCM_##reg##_REG, field)

/**
 * \brief Clear a bit of a PCM register of CRG.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 */
#define HW_PCM_CRG_REG_CLR_BIT(reg, field)      REG_CLR_BIT(CRG_AUD, PCM_##reg##_REG, field)

/**
 * \brief Get the value of a field of a PCM register of SRC.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to read
 *
 * \return the value of the register field
 *
 */
#define HW_PCM_SRC_REG_GETF(reg, field)         REG_GETF(PCM1, PCM1_##reg##_REG, field)

/**
 * \brief Set the value of a field of a PCM register of SRC.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] val is the value to write
 *
 */
#define HW_PCM_SRC_REG_SETF(reg, field, val)    REG_SETF(PCM1, PCM1_##reg##_REG, field, val)

/**
 * \brief Set a bit of a PCM register of SRC.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 */
#define HW_PCM_SRC_REG_SET_BIT(reg, field)      REG_SET_BIT(PCM1, PCM1_##reg##_REG, field)

/**
 * \brief Clear a bit of a PCM register of SRC.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 */
#define HW_PCM_SRC_REG_CLR_BIT(reg, field)      REG_CLR_BIT(PCM1, PCM1_##reg##_REG, field)

/**
 * \brief PCM error code
 *
 */
typedef enum {
        HW_PCM_ERROR_NO_ERROR,               /**< No error during pcm init */
        HW_PCM_ERROR_INVALID_SAMPLE_RATE,    /**< Error related with not supported sampling rate */
        HW_PCM_ERROR_INVALID_BIT_DEPTH,      /**< Error related with not supported bits per samples */
        HW_PCM_ERROR_INVALID_CHANNELS,       /**< Error related with not supported number of channels */
        HW_PCM_ERROR_INVALID_CHANNEL_DELAYS, /**< Error related with not supported number of channel delays */
        HW_PCM_ERROR_INVALID_SLOTS,          /**< Error related with not supported number of slots */
        HW_PCM_ERROR_INVALID_FSC_DIV,        /**< Error related with not supported calculated fsc divider */
        HW_PCM_ERROR_NA_DIV,                 /**< Error related with not supported case of pcm divisors */
}HW_PCM_ERROR_CODE;

/**
 * \brief PCM master/slave mode
 *
 */
typedef enum {
        HW_PCM_MODE_SLAVE,                   /**< PCM interface in slave mode */
        HW_PCM_MODE_MASTER,                  /**< PCM interface in master mode */
} HW_PCM_MODE;

/**
 * \brief PCM system clock source
 *
 */
typedef enum {
        HW_PCM_CLK_DIVN,                      /**< PCM clk to be used is DIVN */
        HW_PCM_CLK_DIV1,                      /**< PCM clk to be used is DIV1*/
} HW_PCM_CLOCK;

/**
 * \brief PCM clock generation
 *
 */
typedef enum {
        HW_PCM_CLK_GEN_FRACTIONAL = 0,   /**< fractional option. Dividing the system clock by an
                                              integer and a fractional part*/
        HW_PCM_CLK_GEN_INTEGER_ONLY,     /**< integer only option. Approximate the sample rate
                                              by adding more clock pulses than required bits.
                                              These extra pulses are ignored */
} HW_PCM_CLK_GENERATION;

/**
 * \brief PCM clock cycles per bit
 *
 */
typedef enum {
        HW_PCM_ONE_CYCLE_PER_BIT = 0,   /**< one clock cycle per data bit */
        HW_PCM_TWO_CYCLE_PER_BIT,       /**< two clock cycles per data bit */
} HW_PCM_CYCLE_PER_BIT;

/**
 * \brief PCM DO output mode
 *
 */
typedef enum {
        HW_PCM_DO_OUTPUT_PUSH_PULL = 0,    /**< PCM DO push pull */
        HW_PCM_DO_OUTPUT_OPEN_DRAIN,       /**< PCM DO open drain */
} HW_PCM_DO_OUTPUT_MODE;

/**
 * \brief PCM FSC edge
 *
 */
typedef enum {
        HW_PCM_FSC_EDGE_RISING = 0,             /**< shift channels 1-8 after PCM_FSC edge */
        HW_PCM_FSC_EDGE_RISING_AND_FALLING,     /**< shift channels 1-4 after PCM_FSC edge
                                                    and channels 5-8 after opposite PCM_FSC edge */
} HW_PCM_FSC_EDGE;

/**
 * \brief PCM FSC delay
 *
 */
typedef enum {
        HW_PCM_FSC_STARTS_1_CYCLE_BEFORE_MSB_BIT = 0,   /**< PCM FSC starts one cycle before MSB bit */
        HW_PCM_FSC_STARTS_SYNCH_TO_MSB_BIT,             /**< PCM FSC starts at the same time as MSB bit */
} HW_PCM_FSC_DELAY;

/**
 * \brief PCM input register
 *
 */
typedef enum {
        HW_PCM_INPUT_REG_1,    /**< PCM input 1 */
        HW_PCM_INPUT_REG_2,    /**< PCM input 2 */
} HW_PCM_INPUT;

/**
 * \brief PCM output register
 *
 */
typedef enum {
        HW_PCM_OUTPUT_REG_1,  /**< PCM output 1 */
        HW_PCM_OUTPUT_REG_2,  /**< PCM output 2 */
} HW_PCM_OUTPUT;

/**
 * \brief PCM input multiplexer
 *
 */
typedef enum {
        HW_PCM_INPUT_MUX_OFF,                                     /**< PCM input is off */
        HW_PCM_INPUT_MUX_SRC1_OUT,                                /**< PCM input set to SRC1_OUT_REG */
        HW_PCM_INPUT_MUX_SRC_OUT = HW_PCM_INPUT_MUX_SRC1_OUT,     /**< deprecated name for SRC1_OUT_REG */
        HW_PCM_INPUT_MUX_PCM_OUT_REG,                             /**< PCM input set to PCM_OUT_REG */
        HW_PCM_INPUT_MUX_SRC2_OUT,                               /**< PCM input set to SRC2_OUT_REG */
        HW_PCM_INPUT_MUX_SIZE
} HW_PCM_INPUT_MUX;

/**
 * \brief PCM clock polarity
 *
 */
typedef enum {
        HW_PCM_CLK_POLARITY_NORMAL = 0,    /**< normal clock polarity */
        HW_PCM_CLK_POLARITY_INVERTED,      /**< inverted clock polarity */
} HW_PCM_CLK_POLARITY;

/**
 * \brief PCM FSC polarity
 *
 */
typedef enum {
        HW_PCM_FSC_POLARITY_NORMAL = 0,    /**< normal FSC polarity */
        HW_PCM_FSC_POLARITY_INVERTED,      /**< inverted FSC polarity */
} HW_PCM_FSC_POLARITY;

/**
 * \brief PCM configuration in PCM mode
 *
 */
typedef struct {
        HW_PCM_CYCLE_PER_BIT cycle_per_bit;   /**< 1 or 2 clock cycle per data bit */
        uint8_t channel_delay;                /**< channel delay in multiples of 8 bits  */
        HW_PCM_CLK_POLARITY clock_polarity;   /**< clock polarity, normal or inverted */
        HW_PCM_FSC_POLARITY fsc_polarity;     /**< FSC polarity, normal or inverted */
        uint16_t fsc_div;                     /**< FSC divider */
        HW_PCM_FSC_DELAY fsc_delay;           /**< PCM FSC starts one cycle before MSB bit,
                                                   otherwise at the same time as MSB bit */
        uint8_t fsc_length;                   /**< FSC length */
} hw_pcm_config_generic_pcm_t;

/**
 * \brief PCM configuration in I2S mode
 *
 */
typedef struct {
        HW_PCM_CYCLE_PER_BIT cycle_per_bit;   /**< 1 or 2 clock cycle per data bit.
                                                Only 1 clock cycle per data bit is used for i2s */
        uint8_t fsc_length;                   /**< FSC length */
        uint16_t fsc_div;                     /**< FSC divider */
        HW_PCM_FSC_POLARITY fsc_polarity;     /**< FSC polarity, normal or inverted */
} hw_pcm_config_i2s_mode_t;

/**
 * \brief PCM configuration in TDM mode
 *
 */
typedef struct {
        HW_PCM_CYCLE_PER_BIT cycle_per_bit;   /**< 1 or 2 clock cycle per data bit */
        HW_PCM_CLK_POLARITY clock_polarity;   /**< clock polarity, normal or inverted.
                                                   Only inverted clock polarity is used for tdm */
        uint8_t channel_delay;                /**< channel delay in multiples of 8 bits.
                                                   Slave 0-31, Master 1-3 */
        uint16_t fsc_div;                     /**< FSC divider */
        HW_PCM_FSC_POLARITY fsc_polarity;     /**< FSC polarity, normal or inverted */
        uint8_t fsc_length;                   /**< FSC length. Master 1 to 4, slave
                                                   waiting for edge.*/
} hw_pcm_config_tdm_mode_t;

/**
 * \brief PCM configuration in IOM2 mode
 *
 */
typedef struct {
        HW_PCM_CLK_POLARITY clock_polarity;   /**< clock polarity, normal or inverted.
                                                 Only normal clock polarity is used for iom */
        HW_PCM_FSC_POLARITY fsc_polarity;     /**< FSC polarity, normal or inverted */
        uint16_t fsc_div;                     /**< FSC divider */
} hw_pcm_config_iom_mode_t;

/**
 \brief PCM interface modes
 */
typedef enum {
        /* Generic PCM interface format configuration */
        HW_PCM_CONFIG_GENERIC_PCM_MODE,
        /* PCM-I2S configuration */
        HW_PCM_CONFIG_I2S_MODE,
        /* PCM-TDM configuration */
        HW_PCM_CONFIG_TDM_MODE,
        /* PCM-IOM configuration */
        HW_PCM_CONFIG_IOM_MODE
} HW_PCM_CONFIG_MODE;

/**
 \brief PCM interface mode configuration
 */
typedef struct {
        /* PCM mode configuration is placed in mode specific structures */
        HW_PCM_CONFIG_MODE config_mode;         /**< PCM format */
        HW_PCM_MODE pcm_mode;                   /**< master/slave mode */
        HW_PCM_DO_OUTPUT_MODE gpio_output_mode; /**< gpio pin output mode */

        union {
                hw_pcm_config_generic_pcm_t pcm_param;
                hw_pcm_config_i2s_mode_t    i2s_param;
                hw_pcm_config_tdm_mode_t    tdm_param;
                hw_pcm_config_iom_mode_t    iom_param;
        };
} hw_pcm_config_t;

/**
 \brief PCM interface clock configuration
 */
typedef struct {
        HW_PCM_CLOCK clock;                     /**< PCM clock source, either div1 or divN */
        uint8_t sample_rate;                    /**< sample rate in kHz */
        uint16_t bit_depth;                     /**< number of bit samples*/
        uint8_t chs;                            /**< audio channels */
        uint16_t ch_delay;                      /**< channel delay in multiples of 8 bit */
        HW_PCM_CYCLE_PER_BIT cycle_per_bit;     /**< 1 or 2 clock cycle per data bit */
        uint8_t slot;                           /**< the number of times channel delay (offset) is added */
        uint16_t fsc_div;                      /**< fsc divider calculated by (bits_depth * chs + channel_delay * 8 * slot ) * cycles_per_bits*/
        HW_PCM_CLK_GENERATION div;              /**< desired divisor type, fractional or integer only */
} hw_pcm_clk_cfg_t;

/**
 * \brief Enable the PCM interface clock source
 */
__STATIC_INLINE void hw_pcm_clk_enable(void)
{
        // Enable PCM interface clock source
        HW_PCM_CRG_REG_SET_BIT(DIV, CLK_PCM_EN);
}

/**
 * \brief Disable the PCM interface clock source
 */
__STATIC_INLINE void hw_pcm_clk_disable(void)
{
        // Disable PCM interface clock source
        HW_PCM_CRG_REG_CLR_BIT(DIV, CLK_PCM_EN);
}

/**
 * \brief Get the status of the PCM interface clock source
 *
 * \return
 *              \retval false if PCM interface clock source is disabled,
 *              \retval true otherwise
 */
__STATIC_INLINE bool hw_pcm_clk_is_enabled(void)
{
        return (HW_PCM_CRG_REG_GETF(DIV, CLK_PCM_EN));
}

/**
 * \brief Enable the PCM interface
 */
__STATIC_INLINE void hw_pcm_enable(void)
{
        // Enable PCM interface
        HW_PCM_SRC_REG_SET_BIT(CTRL, PCM_EN);
}

/**
 * \brief Disable the PCM interface
 */
__STATIC_INLINE void hw_pcm_disable(void)
{
        // Disable PCM interface
        HW_PCM_SRC_REG_CLR_BIT(CTRL, PCM_EN);
}

/**
 * \brief Get the status of the PCM interface
 *
 * \return
 *              \retval false if PCM interface is disabled,
 *              \retval true otherwise
 */
__STATIC_INLINE bool hw_pcm_is_enabled(void)
{
        return (HW_PCM_SRC_REG_GETF(CTRL, PCM_EN));
}

/**
 * \brief Get PCM channel delay
 *
 * \return  channel delay in multiples of 8 bits
 *
 */
__STATIC_INLINE uint8_t hw_pcm_get_channel_delay()
{
       return (HW_PCM_SRC_REG_GETF(CTRL, PCM_CH_DEL));
}

/**
 * \brief Get PCM FSC edge
 *
 * \return The FSC edge
 *              \retval HW_PCM_FSC_EDGE_RISING or
 *              \retval HW_PCM_FSC_EDGE_RISING_AND_FALLING
 */
__STATIC_INLINE bool hw_pcm_get_fsc_edge(void)
{
       return (HW_PCM_SRC_REG_GETF(CTRL, PCM_FSC_EDGE));
}

/**
 * \brief Get PCM FSC length
 *
 * \return  The FSC length in multiples of 8. if 0 then FSC length is equal to 1 data bit.
 */
__STATIC_INLINE uint8_t hw_pcm_get_fsc_length(void)
{
       return (HW_PCM_SRC_REG_GETF(CTRL, PCM_FSCLEN));
}

/**
 * \brief Get PCM FSC divider
 *
 * \return   The FSC divider.  Values must be in the range of 8..0x1000.
 *                If PCM_CLK_BIT=1, divider must always be even.
 */
__STATIC_INLINE uint16_t hw_pcm_get_fsc_div(void)
{
       return (HW_PCM_SRC_REG_GETF(CTRL, PCM_FSC_DIV));
}

/**
 * \brief Get PCM FSC delay
 *
 * \return    Start position of FSC is programmable. If delay is
 *            HW_PCM_FSC_STARTS_1_CYCLE_BEFORE_MSB_BIT then FSC starts one clock cycle before
 *            first bit of channel 0. If delay is HW_PCM_FSC_STARTS_SYNCH_TO_MSB_BIT then FSC
 *            starts synchronously to the first bit of channel 0.
 */
__STATIC_INLINE bool hw_pcm_get_fsc_delay(void)
{
       return (HW_PCM_SRC_REG_GETF(CTRL, PCM_FSCDEL));
}

/**
 * \brief Get PCM clock polarity
 *
 * \return  Polarity of PCM_CLK
 *              \retval HW_PCM_CLK_POLARITY_NORMAL or
 *              \retval HW_PCM_CLK_POLARITY_INVERTED
 *
 */
__STATIC_INLINE bool hw_pcm_get_clk_polarity(void)
{

       return (HW_PCM_SRC_REG_GETF(CTRL, PCM_CLKINV));
}

/**
 * \brief Get PCM FSC polarity
 *
 * \return The polarity of FSC
 *              \retval HW_PCM_FSC_POLARITY_NORMAL or
 *              \retval HW_PCM_FSC_POLARITY_INVERTED
 *
 */
__STATIC_INLINE bool hw_pcm_get_fsc_polarity(void)
{

       return (HW_PCM_SRC_REG_GETF(CTRL, PCM_FSCINV));
}

/**
 * \brief Get PCM clock cycles per data bit
 *
 * \return cycles The number of clock cycles per data bit:
 *              \retval HW_PCM_ONE_CYCLE_PER_BIT = One clock cycle per data bit
 *              \retval HW_PCM_TWO_CYCLE_PER_BIT = Two clock cycles per data bit
 */
__STATIC_INLINE bool hw_pcm_get_clk_per_bit(void)
{
       return (HW_PCM_SRC_REG_GETF(CTRL, PCM_CLK_BIT));
}

/**
 * \brief Get input for the PCM1_MUX_IN multiplexer
 *
 * \return The input for PCM is HW_PCM_INPUT_MUX enum value
 */
__STATIC_INLINE HW_PCM_INPUT_MUX hw_pcm_get_pcm_input_mux(void)
{
        // Set PCM1_MUX_IN field in SRC1 MUX register
        return(REG_GETF(SRC1, SRC1_MUX_REG, PCM1_MUX_IN));
}

/**
 * \brief Get PCM DO output mode
 *
 * \return output mode can be
 *         \retval HW_PCM_DO_OUTPUT_PUSH_PULL or
 *         \retval HW_PCM_DO_OUTPUT_OPEN_DRAIN
 *
 */
__STATIC_INLINE HW_PCM_DO_OUTPUT_MODE hw_pcm_get_output_mode(void)
{
        // Set PCM_PPOD field in PCM control register
        return(HW_PCM_SRC_REG_GETF(CTRL, PCM_PPOD));
}

/**
 * \brief Get PCM master/slave mode
 *
 * \return PCM mode
 *              \retval master or
 *              \retval slave
 *
 */
__STATIC_INLINE HW_PCM_MODE hw_pcm_get_mode(void)
{
        return(HW_PCM_SRC_REG_GETF(CTRL, PCM_MASTER));
}

/**
 * \brief Set PCM master/slave mode
 *
 * \param[in] mode PCM mode
 *              \retval master or
 *              \retval slave
 *
 */
__STATIC_INLINE void hw_pcm_set_mode(HW_PCM_MODE mode)
{
        HW_PCM_SRC_REG_SETF(CTRL, PCM_MASTER, mode); // set/reset PCM_MASTER bit
}


/**
 * \brief Initialize PCM clock registers
 *
 * \param[in, out] pcm_clk             PCM clock configuration
 *
 * \note The value of pcm_clk->fsc_div is calculated by the function; it should be used to configure PCM_FSC_DIV.
 *
 * \return  the error code
 *                              \retval HW_PCM_ERROR_NO_ERROR on success, else
 *                              \retval error
 *  \sa HW_PCM_ERROR_CODE
 */
HW_PCM_ERROR_CODE hw_pcm_init_clk(hw_pcm_clk_cfg_t *pcm_clk);

/**
 * \brief Set initialization of PCM interface
 *
 * call hw_pcm_enable() once PCM interface initialization is done
 *
 * \param[in] config configuration of PCM interface in specific mode (generic PCM, I2S, TDM, IOM)
 */
void hw_pcm_init(hw_pcm_config_t *config);

/**
 * \brief Set PCM channel delay
 *
 * \param[in] delay channel delay is the multiples (N) of 8 bits
 *  *         Values must be in the range of 0..3.
 */
__STATIC_INLINE void hw_pcm_set_channel_delay(uint8_t delay)
{
        // Delay has a maximum value of 31
        ASSERT_WARNING(delay <= 3);
        // Set PCM_CH_DEL field in PCM control register
        HW_PCM_SRC_REG_SETF(CTRL, PCM_CH_DEL, delay);
}

/**
 * \brief Set PCM FSC edge
 *
 * \param[in] edge The FSC edge, HW_PCM_FSC_EDGE_RISING or HW_PCM_FSC_EDGE_RISING_AND_FALLING
 */
__STATIC_INLINE void hw_pcm_set_fsc_edge(HW_PCM_FSC_EDGE edge)
{
        // Set PCM_FSC_EDGE field in PCM control register
        HW_PCM_SRC_REG_SETF(CTRL, PCM_FSC_EDGE, edge);
}

/**
 * \brief Set PCM FSC length
 *
 * \param[in] length The FSC length is the multiples (N) of 8.
 *            Values must be in the range of 0..8.
 *            if 0 then FSC length is equal to 1 data bit.
 */
__STATIC_INLINE void hw_pcm_set_fsc_length(uint8_t length)
{
        ASSERT_WARNING(length <= 8);

        // Set PCM_FSCLEN field in PCM control register
        HW_PCM_SRC_REG_SETF(CTRL, PCM_FSCLEN, length);
}

/**
 * \brief Set PCM FSC divider
 *
 * \param[in] div The FSC divider.  Values must be in the range of 8..0x1000.
 *                If PCM_CLK_BIT=1, divider must always be even.
 */
__STATIC_INLINE void hw_pcm_set_fsc_div(uint16_t div)
{
        ASSERT_WARNING(div >= 8 && div <= 0x1000);

        if (hw_pcm_get_clk_per_bit()) {
                ASSERT_ERROR((div % 2 == 0));
        }

        // Set PCM_FSC_DIV field in PCM control register
        HW_PCM_SRC_REG_SETF(CTRL, PCM_FSC_DIV, div - 1);
}

/**
 * \brief Set PCM FSC delay
 *
 * \param[in] delay Start position of FSC is programmable. If delay is
 *            HW_PCM_FSC_STARTS_1_CYCLE_BEFORE_MSB_BIT then FSC starts one clock cycle before
 *            first bit of channel 0. If delay is HW_PCM_FSC_STARTS_SYNCH_TO_MSB_BIT then FSC
 *            starts synchronously to the first bit of channel 0.
 */
__STATIC_INLINE void hw_pcm_set_fsc_delay(HW_PCM_FSC_DELAY delay)
{
        // Set PCM_FSCDEL field in PCM control register
        HW_PCM_SRC_REG_SETF(CTRL, PCM_FSCDEL, delay);
}

/**
 * \brief Set PCM clock polarity
 *
 * \param[in] pol Polarity of PCM_CLK,
 *              \retval HW_PCM_CLK_POLARITY_NORMAL or
 *              \retval  HW_PCM_CLK_POLARITY_INVERTED
 *
 */
__STATIC_INLINE void hw_pcm_set_clk_polarity(HW_PCM_CLK_POLARITY pol)
{
        // Set PCM_CLKINV field in PCM control register
        HW_PCM_SRC_REG_SETF(CTRL, PCM_CLKINV, pol);
}

/**
 * \brief Set input for the PCM1_MUX_IN multiplexer
 *
 * \param[in] input The input for PCM is HW_PCM_INPUT_MUX enum value
 */
__STATIC_INLINE void hw_pcm_set_pcm_input_mux(HW_PCM_INPUT_MUX input)
{
        // Set PCM1_MUX_IN field in APU MUX register
        REG_SETF(SRC1, SRC1_MUX_REG, PCM1_MUX_IN, input);
}

/**
 * \brief Set PCM FSC polarity
 *
 * \param[in] pol The polarity of FSC,
 *              \retval HW_PCM_FSC_POLARITY_NORMAL or
 *              \retval HW_PCM_FSC_POLARITY_INVERTED
 *
 */
__STATIC_INLINE void hw_pcm_set_fsc_polarity(HW_PCM_FSC_POLARITY pol)
{
        // Set PCM_FSCINV field in PCM control register
        HW_PCM_SRC_REG_SETF(CTRL, PCM_FSCINV, pol);
}

/**
 * \brief Set PCM clock cycles per data bit
 *
 * \param[in] cycles The number of clock cycles per data bit:
 *       \retval HW_PCM_ONE_CYCLE_PER_BIT = One clock cycle per data bit
 *       \retval HW_PCM_TWO_CYCLE_PER_BIT = Two clock cycles per data bit
 */
__STATIC_INLINE void hw_pcm_set_clk_per_bit(HW_PCM_CYCLE_PER_BIT cycles)
{
        // Set PCM_CLK_BIT field in PCM control register
        HW_PCM_SRC_REG_SETF(CTRL, PCM_CLK_BIT, cycles);
}

/**
 * \brief Set PCM DO output mode
 *
 * \param[in] mode      output mode can be
 *              \retval HW_PCM_DO_OUTPUT_PUSH_PULL or
 *              \retval HW_PCM_DO_OUTPUT_OPEN_DRAIN
 *
 */
__STATIC_INLINE void hw_pcm_set_output_mode(HW_PCM_DO_OUTPUT_MODE mode)
{
        // Set PCM_PPOD field in PCM control register
        HW_PCM_SRC_REG_SETF(CTRL, PCM_PPOD, mode);
}

/**
 * \brief Read PCM input (RX) register
 *
 * \param[in] input The input register, HW_PCM_INPUT_REG_1 or HW_PCM_INPUT_REG_2
 *
 * \return read data read from register
 *
 */
__STATIC_INLINE uint32_t hw_pcm_input_read(HW_PCM_INPUT input)
{
        switch (input) {
        case HW_PCM_INPUT_REG_1:
                return PCM1->PCM1_IN1_REG;
        case HW_PCM_INPUT_REG_2:
                return PCM1->PCM1_IN2_REG;
        default:
                ASSERT_WARNING(0);
        }
        return 0;
}

/**
 * \brief Write PCM output (TX) register
 *
 * \param[in] output The output register,
 *                      \retval HW_PCM_OUTPUT_REG_1 or
 *                      \retval HW_PCM_OUTPUT_REG_2
 * \param[in] data data to write to output register
 *
 */
__STATIC_INLINE void hw_pcm_output_write(HW_PCM_OUTPUT output, const uint32_t data)
{
        switch (output) {
        case HW_PCM_OUTPUT_REG_1:
                PCM1->PCM1_OUT1_REG = data;
                break;
        case HW_PCM_OUTPUT_REG_2:
                PCM1->PCM1_OUT2_REG = data;
                break;
        default:
                ASSERT_WARNING(0);
        }
}
/**
 * \brief Application-defined callback type for the PCM interrupt.
 *
 * \note The application defined callback should be declared as  __RETAINED_CODE.
 *
 */
typedef void (*hw_pcm_interrupt_cb_t)(void);

/**
 * \brief Register PCM interrupt handler
 *
 * A callback function is registered to be called when an interrupt is generated. Interrupt is
 * automatically enabled after calling this function. If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb Callback defined by the application.
 *
 */
void hw_pcm_register_interrupt(hw_pcm_interrupt_cb_t cb);

/**
 * \brief Unregister interrupt PCM handler
 *
 * Interrupt is automatically disabled after calling this function.
 *
 */
void hw_pcm_unregister_interrupt(void);

#endif /* dg_configUSE_HW_PCM */
#endif /* HW_PCM_H_ */
/**
 * \}
 * \}
 */

