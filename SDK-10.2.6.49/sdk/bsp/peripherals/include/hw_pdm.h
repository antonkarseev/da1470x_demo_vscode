/**
 * \addtogroup PLA_DRI_PER_AUDIO
 * \{
 * \addtogroup HW_PDM_AUDIO PDM Audio Interface Driver
 * \{
 * \brief PDM LLD provides a serial audio connection for 1 stereo or 2 mono input devices
 *  or outputs devices.
 */

/**
 ****************************************************************************************
 *
 * @file hw_pdm.h
 *
 * @brief Definition of API for the PDM Low Level Driver.
 *
 * Copyright (C) 2019-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_PDM_H_
#define HW_PDM_H_


#if dg_configUSE_HW_PDM

#include "sdk_defs.h"
#include "hw_src.h"

/**
 * \brief Get the mask of a field of a PDM register of CRG.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 * \note The stripped register name should be provided, e.g.:
 *        - to get CRG_PER_PDM_DIV_REG_PDM_DIV_Ms, use
 *              HW_PDM_CRG_REG_FIELD_MASK(DIV, PDM_DIV)
 *
 */
#define HW_PDM_CRG_REG_FIELD_MASK(reg, field)   REG_MSK(CRG_AUD, PDM_##reg##_REG, field)
/**
 * \brief Get the bit position of a field of a PDM register of CRG.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 * \note The stripped register name should be provided, e.g.:
 *        - to get CRG_PER_PDM_DIV_REG_PDM_DIV_Pos, use
 *              HW_PDM_CRG_REG_FIELD_POS(DIV, PDM_DIV)
 *
 */
#define HW_PDM_CRG_REG_FIELD_POS(reg, field)    REG_POS(CRG_AUD, PDM_##reg##_REG, field)

/**
 * \brief Get the value of a field of a PDM register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to read
 *
 * \return the value of the register field
 *
 */
#define HW_PDM_CRG_REG_GETF(reg, field)         REG_GETF(CRG_AUD, PDM_##reg##_REG, field)

/**
 * \brief Set the value of a field of a PDM register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] val is the value to write
 *
 */
#define HW_PDM_CRG_REG_SETF(reg, field, val)    REG_SETF(CRG_AUD, PDM_##reg##_REG, field, val)

/**
 * \brief Set a bit of a PDM register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 */
#define HW_PDM_CRG_REG_SET_BIT(reg, field)      REG_SET_BIT(CRG_AUD, PDM_##reg##_REG, field)

/**
 * \brief Clear a bit of a PDM register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 */
#define HW_PDM_CRG_REG_CLR_BIT(reg, field)      REG_CLR_BIT(CRG_AUD, PDM_##reg##_REG, field)

/**
 * \brief Get the value of a field of a PDM register.
 *
 * \param [in] id       identifies APU, SRC1, SRC2
 * \param [in] reg      is the register to access
 * \param [in] field    is the register field to read
 *
 * \return the value of the register field
 *
 */
#define HW_PDM_SRC_REG_GETF(id, reg, field)         HW_SRC_REG_GETF(id, SRC1, reg##_REG, field)

/**
 * \brief Set the value of a field of a PDM register.
 *
 * \param [in] id       identifies APU, SRC1, SRC2
 * \param [in] reg      is the register to access
 * \param [in] field    is the register field to write
 * \param [in] val      is the value to write
 *
 */
#define HW_PDM_SRC_REG_SETF(id, reg, field, val)    HW_SRC_REG_SETF(id, SRC1, reg##_REG, field, val)

/**
 * \brief Set a bit of a PDM register.
 *
 * \param [in] id       identifies APU, SRC1, SRC2
 * \param [in] reg      is the register to access
 * \param [in] field    is the register field to write
 */
#define HW_PDM_SRC_REG_SET_BIT(id, reg, field)      HW_SRC_REG_SET_BIT(id, SRC1, reg##_REG, field)

/**
 * \brief Clear a bit of a PDM register.
 *
 * \param [in] id       identifies APU, SRC1, SRC2
 * \param [in] reg      is the register to access
 * \param [in] field    is the register field to write
 */
#define HW_PDM_SRC_REG_CLR_BIT(id, reg, field)      HW_SRC_REG_CLR_BIT(id, SRC1, reg##_REG, field)

/**
 * \brief PDM data direction
 *
 */
typedef enum {
        PDM_DIRECTION_INPUT,
        PDM_DIRECTION_OUTPUT
} HW_PDM_DATA_DIRECTION;

/**
 * \brief PDM Master/Slave mode
 *
 */
typedef enum {
        HW_PDM_SLAVE_MODE = 0, /**< PDM Interface in slave mode */
        HW_PDM_MASTER_MODE     /**< PDM Interface in master mode */
} HW_PDM_MODE;

/**
 * \brief PDM input delay
 *
 */
typedef enum {
        HW_PDM_DI_NO_DELAY = 0, /**< no PDM input delay */
        HW_PDM_DI_4_NS_DELAY,   /**< 4ns PDM input delay */
        HW_PDM_DI_8_NS_DELAY,   /**< 8ns PDM input delay */
        HW_PDM_DI_12_NS_DELAY   /**< 12ns PDM input delay */
} HW_PDM_DI_DELAY;

/**
 * \brief PDM output delay
 *
 */
typedef enum {
        HW_PDM_DO_NO_DELAY = 0, /**< no delay */
        HW_PDM_DO_8_NS_DELAY,   /**< 8ns PDM output delay */
        HW_PDM_DO_12_NS_DELAY,  /**< 12ns PDM output delay */
        HW_PDM_DO_16_NS_DELAY   /**< 16ns PDM output delay */
} HW_PDM_DO_DELAY;

/**
 * \brief PDM output channel configuration
 *
 */
typedef enum {
        HW_PDM_CHANNEL_NONE = 0,  /**< No PDM output - no output */
        HW_PDM_CHANNEL_R,         /**< Right channel only PDM output (falling edge of PDM_CLK) */
        HW_PDM_CHANNEL_L,         /**< Left channel only PDM output (rising edge of PDM_CLK) */
        HW_PDM_CHANNEL_LR         /**< Left and Right channel PDM output */
} HW_PDM_CHANNEL_CONFIG;

/**
 * \brief PDM output multiplexer
 *
 */
typedef enum {
        HW_PDM_MUX_OUT_SRC1,          /**< PDM output for PDM_MUX_OUT from SRC1 */
        HW_PDM_MUX_OUT_SRC2,          /**< PDM output for PDM_MUX_OUT from SRC2 */
} HW_PDM_MUX_OUT;

/**
 \brief PDM interface mode configuration
 */
typedef struct {
        /* PDM mode configuration is placed in mode specific structures */
        HW_PDM_MODE             config_mode;            /**< PDM master/slave mode */
        HW_PDM_DI_DELAY         in_delay;               /**< PDM input delay */
        HW_PDM_DO_DELAY         out_delay;              /**< PDM output delay */
        HW_PDM_CHANNEL_CONFIG   output_channel;         /**< PDM output channel */
        bool                    swap_channel;           /**< PDM  swap channels this parameter is applicable only when
                                                              2 channel are used */
        uint32_t                clk_frequency;          /**< PDM_CLK frequency 62.5 kHz - 4 MHz. It should be noted
                                                              that the audio quality degrades when the oversampling
                                                              ratio is less than 64. For an 8 kHz sample rate
                                                              the minimum recommended PDM clock rate is
                                                              64 x 8 kHz = 512 kHz. */
        HW_PDM_DATA_DIRECTION   data_direction;        /**< PDM data direction **/
} hw_pdm_config_t;

/**
 * \brief Get input delay in PDM interface
 *
 * \param [in] id       identifies SRC1, SRC2
 *
 * \return Additional delay (in ns) from the PDM data input pad to
 *            the PDM interface. Available delay values are:
 * - no delay,
 * - 4 ns delay,
 * - 8 ns delay,
 * - 12 ns delay.
 */
__STATIC_INLINE HW_PDM_DI_DELAY hw_pdm_get_input_delay(HW_SRC_ID id)
{
        return(HW_PDM_SRC_REG_GETF(id, CTRL, SRC_PDM_DI_DEL));
}

/**
 * \brief Get output delay in PDM interface
 *
 * \return additional delay (in ns) from the PDM interface to the PDM data outpu pad.
 *  Available delay values are:
 * - no delay,
 * - 8 ns delay,
 * - 12 ns delay,
 * - 16 ns delay.
 */
__STATIC_INLINE HW_PDM_DO_DELAY hw_pdm_get_output_delay(HW_SRC_ID id)
{
        return(HW_PDM_SRC_REG_GETF(id, CTRL, SRC_PDM_DO_DEL));
}

/**
 * \brief Get PDM output channel configuration
 *
 * \param [in] id       identifies SRC1, SRC2
 *
 * \return channel_conf Output configuration of the PDM output interface.
 * Available values for PDM output configuration are:
 * \parblock
 *      ::HW_PDM_CHANNEL_NONE<br>
 *      There is no data on the PDM output interface.
 *
 *      ::HW_PDM_CHANNEL_R<br>
 *      Data stream at the output of the PDM interface is available only on the right channel.
 *
 *      ::HW_PDM_CHANNEL_L<br>
 *      Data stream at the output of the PDM interface is available only on the left channel.
 *
 *      ::HW_PDM_CHANNEL_LR<br>
 *      Data stream at the output of the PDM interface is available on both left and right channel.
 * \endparblock
 */
__STATIC_INLINE HW_PDM_CHANNEL_CONFIG hw_pdm_get_output_channel_config(HW_SRC_ID id)
{
        return(HW_PDM_SRC_REG_GETF(id, CTRL, SRC_PDM_MODE));
}

/**
 * \brief Get PDM Master/Slave mode
 *
 * \return The PDM mode, HW_PDM_SLAVE_MODE or HW_PDM_MASTER_MODE
 *
 */
__STATIC_INLINE HW_PDM_MODE hw_pdm_get_mode(void)
{

        return(HW_PDM_CRG_REG_GETF(DIV, PDM_MASTER_MODE));
}

/**
 * \brief Get PDM status. Supported only for Master mode
 *
 * \return  PDM status
 *
 */
__STATIC_INLINE bool hw_pdm_get_status(void)
{
        if (hw_pdm_get_mode() == HW_PDM_SLAVE_MODE) {
                return true;
        }

        return(HW_PDM_CRG_REG_GETF(DIV, CLK_PDM_EN));
}

/**
 * \brief Get PDM clock divider.
 *
 * \return  PDM clock divider
 *
 */
__STATIC_INLINE uint8_t hw_pdm_get_clk_div(void) {

        return(HW_PDM_CRG_REG_GETF(DIV, PDM_DIV));
}

/**
 * \brief Get the status of swap of the channels on the PDM input source
 *
 * \param [in] id       identifies SRC1, SRC2
 *
 * \return true when input PDM channels are swapped otherwise false
 *
 */
__STATIC_INLINE bool hw_pdm_get_in_channel_swap(HW_SRC_ID id)
{
        return(HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_PDM_IN_INV));
}

/**
 * \brief Get the status of swap of the channels on the PDM output source
 *
 * \return true when output PDM channels are swapped otherwise false
 *
 */
__STATIC_INLINE bool hw_pdm_get_out_channel_swap(HW_SRC_ID id)
{
        return(HW_PDM_SRC_REG_GETF(id, CTRL, SRC_PDM_OUT_INV));
}

/**
 * \brief Enable PDM block system clock source used only for Master mode
 *
 * Enable the PDM clock source.
 * PDM_DIV must be set before or together with CLK_PDM_EN.
 */
__STATIC_INLINE void hw_pdm_enable(void)
{
        HW_PDM_CRG_REG_SET_BIT(DIV, CLK_PDM_EN);
}

/**
 * \brief Disable PDM block system clock source
 *
 * Disable the PDM clock source.
 */
__STATIC_INLINE void hw_pdm_disable(void)
{
        HW_PDM_CRG_REG_CLR_BIT(DIV, CLK_PDM_EN);
}

/**
 * \brief Set input delay in PDM interface
 *
 * \param [in] id       identifies SRC1, SRC2
 * \param[in] delay Additional delay (in ns) from the PDM data input pad to
 *            the PDM interface. Available delay values are:
 * - no delay,
 * - 4 ns delay,
 * - 8 ns delay,
 * - 12 ns delay.
 */
__STATIC_INLINE void hw_pdm_set_input_delay(HW_SRC_ID id, HW_PDM_DI_DELAY delay)
{
        HW_PDM_SRC_REG_SETF(id, CTRL, SRC_PDM_DI_DEL, delay);
}

/**
 * \brief Set output delay in PDM interface
 *
 * \param [in] id       identifies APU, SRC1, SRC2
 * \param[in] delay     additional delay (in ns) from the PDM interface to the PDM data outpu pad.
 *  Available delay values are:
 * - no delay,
 * - 8 ns delay,
 * - 12 ns delay,
 * - 16 ns delay.
 */
__STATIC_INLINE void hw_pdm_set_output_delay(HW_SRC_ID id, HW_PDM_DO_DELAY delay)
{
        HW_PDM_SRC_REG_SETF(id, CTRL, SRC_PDM_DO_DEL, delay);
}

/**
 * \brief Set PDM output channel configuration applicable only for SRC1, APU
 *
 * \param [in] id          identifies SRC1, SRC2
 * \param[in] channel_conf Output configuration of the PDM output interface.
 * Available values for PDM output configuration are:
 * \parblock
 *      ::HW_PDM_CHANNEL_NONE<br>
 *      There is no data on the PDM output interface.
 *
 *      ::HW_PDM_CHANNEL_R<br>
 *      Data stream at the output of the PDM interface is available only on the right channel.
 *
 *      ::HW_PDM_CHANNEL_L<br>
 *      Data stream at the output of the PDM interface is available only on the left channel.
 *
 *      ::HW_PDM_CHANNEL_LR<br>
 *      Data stream at the output of the PDM interface is available on both left and right channel.
 * \endparblock
 */
__STATIC_INLINE void hw_pdm_set_output_channel_config(HW_SRC_ID id, HW_PDM_CHANNEL_CONFIG channel_conf)
{
        HW_PDM_SRC_REG_SETF(id, CTRL, SRC_PDM_MODE, channel_conf);
}

/**
 * \brief Set PDM Master/Slave mode
 *
 * \param[in] mode The PDM mode, HW_PDM_SLAVE_MODE or HW_PDM_MASTER_MODE
 *
 */
__STATIC_INLINE void hw_pdm_set_mode(HW_PDM_MODE mode)
{
        HW_PDM_CRG_REG_SETF(DIV, PDM_MASTER_MODE, mode);
}

/**
 * \brief Swap left and right channel on the PDM input source
 *
 * \param [in] id       identifies SRC1, SRC2
 * \param[in] swap      true when input PDM channels are swapped otherwise false
 *
 */
__STATIC_INLINE void hw_pdm_set_in_channel_swap(HW_SRC_ID id, bool swap)
{
        if (swap == true) {
                HW_PDM_SRC_REG_SET_BIT(id, CTRL, SRC_PDM_IN_INV);
        } else {
                HW_PDM_SRC_REG_CLR_BIT(id, CTRL, SRC_PDM_IN_INV);
        }
}

/**
 * \brief Swap left and right channel on the PDM output source
 *
 * \param [in] id       identifies APU, SRC1, SRC2
 * \param[in] swap      true when output PDM channels are swapped otherwise false
 *
 */
__STATIC_INLINE void hw_pdm_set_out_channel_swap(HW_SRC_ID id, bool swap)
{
        if (swap == true) {
                HW_PDM_SRC_REG_SET_BIT(id, CTRL, SRC_PDM_OUT_INV);
        } else {
                HW_PDM_SRC_REG_CLR_BIT(id, CTRL, SRC_PDM_OUT_INV);
        }
}

/**
 * \brief Set output for the PDM_MUX_OUT multiplexer
 *
 * \param[in] output The output for PCM:
 *      \retval HW_PDM_OUTPUT_MUX_SRC1           = PDM output for PDM_MUX_OUT from SRC1
 *      \retval HW_PDM_OUTPUT_MUX_SRC2           = PDM output for PDM_MUX_OUT from SRC2
 */
__STATIC_INLINE void hw_pdm_set_pdm_output_mux(HW_PDM_MUX_OUT output)
{
        // Set PDM_MUX_OUT field in SRC2 MUX register
        REG_SETF(SRC2, SRC2_MUX_REG, PDM_MUX_OUT, output);
}

/**
 * \brief Get output for the PDM_MUX_OUT multiplexer
 *
 * \return  The output for PCM:
 *      \retval HW_PDM_OUTPUT_MUX_SRC1           = PDM output for PDM_MUX_OUT from SRC1
 *      \retval HW_PDM_OUTPUT_MUX_SRC2           = PDM output for PDM_MUX_OUT from SRC2
 */
__STATIC_INLINE HW_PDM_MUX_OUT hw_pdm_get_pdm_output_mux(void)
{
        // Get HW_PDM_MUX_OUT field in SRC2 MUX register
        return(REG_GETF(SRC2, SRC2_MUX_REG, PDM_MUX_OUT));
}

/**
 * \brief Initialize PDM clock
 *
 * \param[in] frequency requested frequency in the range 125490...4000000 (Hz) of PDM clock
 *              for default clock DIVN.
 *
 * \return achieved frequency (Hz) of PDM clock
 */
uint32_t hw_pdm_clk_init(uint32_t frequency);

/**
 * \brief Initialize PDM interface
 *
 * call hw_pdm_enable() once PDM interface initialization is done
 *
 * \param [in] id       identifies SRC1, SRC2
 * \param[in] config configuration of PDM interface
 */
void hw_pdm_init(HW_SRC_ID id, hw_pdm_config_t *config);
#endif /* dg_configUSE_HW_PDM */
#endif /* HW_PDM_H_ */

/**
 * \}
 * \}
 */
