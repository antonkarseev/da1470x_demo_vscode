/**
 * \addtogroup PLA_DRI_PER_AUDIO
 * \{
 * \addtogroup HW_SRC Audio Processing Unit - Sample Rate Converter
 * \{
 * \brief Audio Sample Rate Converter
 */

/**
 ****************************************************************************************
 *
 * @file hw_src.h
 *
 * @brief Definition of the API for the Audio Unit SRC Low Level Driver.
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_SRC_H_
#define HW_SRC_H_

#if dg_configUSE_HW_SRC
#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"

#define BASE_TYPE       SRC1_Type
/**
 * \brief SRC id
 *
 */
#define HW_SRC1         ((void *)SRC1_BASE)
#define HW_SRC2         ((void *)SRC2_BASE)

/* SRC Base Address */
#define SRCBA(id)      ((BASE_TYPE *)id)

typedef void * HW_SRC_ID;

#define MUX_REG     SRC1_MUX_REG

#define CTRL_REG     SRC1_CTRL_REG
/**
 * \brief Get the value of a field of a SRC register.
 *
 * \param [in] id       identifies SRC1, SRC2
 * \param [in] base     identifies APU, SRC1, SRC2 bases
 * \param [in] reg      is the register to access
 * \param [in] field    is the register field to read
 *
 * \return the value of the register field
 *
 */
#define HW_SRC_REG_GETF(id, base, reg, field)                                                   \
        ((SRCBA(id)->SRC1_##reg & (base##_SRC1_##reg##_##field##_Msk)) >> (base##_SRC1_##reg##_##field##_Pos))

/**
 * \brief Set the value of a field of a SRC register.
 *
 * \param [in] id       identifies SRC1, SRC2
 * \param [in] base     identifies APU, SRC1, SRC2 bases
 * \param [in] reg      is the register to access
 * \param [in] field    is the register field to write
 * \param [in] val      is the value to write
 *
 */
#define HW_SRC_REG_SETF(id, base, reg, field, val)                                                \
        SRCBA(id)->SRC1_##reg = ((SRCBA(id)->SRC1_##reg & ~(base##_SRC1_##reg##_##field##_Msk)) |                    \
        ((base##_SRC1_##reg##_##field##_Msk) &  ((val) << (base##_SRC1_##reg##_##field##_Pos))))

/**
 * \brief Set SRC register field value.
 *
 * Sets a register field value (aimed to be used with local variables).
 * e.g.
 * \code
 * uint32_t val;
 *
 * val = SRCBA(config->id)->CTRL_REG;
 * HW_SRC_REG_SET_FIELD(SRC1, SRC1_CTRL_REG, SRC_IN_DS, val, iir_setting);
 * SRCBA(config->id)->CTRL_REG = val;
 * ...
 * \endcode
 */
#define  HW_SRC_REG_SET_FIELD(base, reg, field, var, val)                                       \
        var = (((var & ~(base##_SRC1_##reg##_##field##_Msk))) |                                 \
                (((val) << (base##_SRC1_##reg##_##field##_Pos)) &                               \
                (base##_SRC1_##reg##_##field##_Msk)))

/**
 * \brief Access register field value.
 *
 * Returns a register field value (aimed to be used with local variables).
 * e.g.
 * \code
 * uint16_t tmp;
 * int val;
 *
 * tmp = SRC1->SRC1_MUX_REG;
 * val = HW_SRC_REG_GET_FIELD(SRC1, SRC1_MUX_REG, PDM1_MUX_IN, tmp);
 * ...
 * \endcode
 */
#define HW_SRC_REG_GET_FIELD(base, reg, field, var)                                             \
        ((var & (base##_SRC1_##reg##_##field##_Msk)) >>                                         \
                (base##_SRC1_##reg##_##field##_Pos))
/**
 * \brief Clear register field value.
 *
 * Clears a register field value (aimed to be used with local variables).
 * e.g.
 * \code
 * uint16_t tmp;
 *
 * tmp = SRC1->SRC1_MUX_REG;
 * HW_SRC_REG_CLR_FIELD(SRC1, SRC1_MUX_REG, PDM1_MUX_IN, tmp);
 * SRC1->SRC1_MUX_REG = tmp;
 * ...
 * \endcode
 */
#define HW_SRC_REG_CLR_FIELD(base, reg, field, var)                                             \
        var &= ~(base##_SRC1_##reg##_##field##_Msk)

/**
 * \brief Set a bit of a SRC register.
 *
 * \param [in] id       identifies SRC1, SRC2
 * \param [in] base     identifies APU, SRC1, SRC2 bases
 * \param [in] reg      is the register to access
 * \param [in] field    is the register field to write
 */
#define HW_SRC_REG_SET_BIT(id, base, reg, field)                                                \
        do {                                                                                    \
                SRCBA(id)->SRC1_##reg |= (1 << (base##_SRC1_##reg##_##field##_Pos));            \
         } while (0)


/**
 * \brief Clear a bit of a SRC register.
 *
 * \param [in] id       identifies SRC1, SRC2
 * \param [in] base     identifies APU, SRC1, SRC2 bases
 * \param [in] reg      is the register to access
 * \param [in] field    is the register field to write
 */
#define HW_SRC_REG_CLR_BIT(id, base, reg, field)                                                \
        do {                                                                                    \
                SRCBA(id)->SRC1_##reg &= ~(base##_SRC1_##reg##_##field##_Msk);                  \
         } while (0)

/**
 * \brief Input/Output direction
 */
typedef enum {
        HW_SRC_IN,  /**< SRC input */
        HW_SRC_OUT, /**< SRC output */
} HW_SRC_DIRECTION;

/**
 * \brief Flow status
 */
typedef enum {
        HW_SRC_FLOW_OK = 0,         /**< No flow errors */
        HW_SRC_FLOW_OVER,           /**< Overflow errors */
        HW_SRC_FLOW_UNDER,          /**< Underflow errors */
        HW_SRC_FLOW_OVER_UNDER,     /**< Both overflow and underflow errors */
} HW_SRC_FLOW_STATUS;

/**
 * \brief Input/Output selection
 */
typedef enum {
        HW_SRC_PCM = 1,         /**< PCM interface */
        HW_SRC_PDM,         /**< PDM interface */
        HW_SRC_REGS,        /**< SRC registers */
# if dg_configUSE_HW_SDADC
        HW_SRC_SDADC,
# endif /* dg_configUSE_HW_SDADC */
        HW_SRC_SELECTION_SIZE
} HW_SRC_SELECTION;

/**
 * \brief SRCx input multiplexer
 *
 */
typedef enum {
        HW_SRC_INPUT_MUX_OFF,              /**< PCM input is off */
        HW_SRC_INPUT_MUX_PCM_OUT_REG,      /**< PCM input set to PCM_OUT_REG */
        HW_SRC_INPUT_MUX_SRCx_IN_REG,      /**< PCM input set to SRCx_OUT_REG */
        HW_SRC_INPUT_MUX_SDADC_OUT,        /**< PCM input set to SDADC */
        HW_SRCx_MUX_IN_SIZE
} HW_SRCx_MUX_IN;

/**
 * \brief PDM input multiplexer
 *
 */
typedef enum {
        HW_PDM_INPUT_MUX_SRCx_MUX_IN,        /**< PDM input is SRCx_MUX_IN */
        HW_PDM_INPUT_MUX_PDM_INPUT,          /**< PDM input is PDM */
} HW_PDM1_MUX_IN;

/**
 * \brief SRC configuration structure definition
 */
typedef struct {
        HW_SRC_ID       id;              /**< identifies SRC */
        uint16_t        src_clk;         /**< SRC clock in kHz with allowed values (in kHz):
                                               128,  160,  200,  250,  256,  320,  400,  500,  640,   800,
                                               1000, 1280, 1600, 2000, 3200, 4000, 6400, 8000, 16000, 32000 */
        uint32_t        in_sample_rate;  /**< Input sampling rate in Hz with allowed values:
                                               0, 8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000 */
        uint32_t        out_sample_rate; /**< Input sampling rate in Hz with allowed values:
                                               0, 8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000 */
        HW_SRC_SELECTION  data_input;    /**< The SRC input */
} hw_src_config_t;

/* *************************************************************************
 *
 *                       ENABLE-DISABLE FUNCTIONS
 *
 * ************************************************************************* */

/**
 * \brief Enable SRC
 *
 * \param [in] id identifies SRC1, SRC2
 */
__STATIC_INLINE void hw_src_enable(HW_SRC_ID id)
{


        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        /*
         * The under/overflows that occur due to the reconfiguration can be
         * ignored, so we disable under/overflow notifications until
         * (SRC_IN_OK == 1 && SRC_OUT_OK == 1).
         */
        HW_SRC_REG_SET_BIT(id, SRC1, CTRL_REG, SRC_IN_FLOWCLR);
        HW_SRC_REG_SET_BIT(id, SRC1, CTRL_REG, SRC_OUT_FLOWCLR);

        HW_SRC_REG_SET_BIT(id, SRC1, CTRL_REG, SRC_EN);

        while (HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_IN_OK) == 0 &&
                HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_OUT_OK) == 0);

        HW_SRC_REG_CLR_BIT(id, SRC1, CTRL_REG, SRC_IN_FLOWCLR);
        HW_SRC_REG_CLR_BIT(id, SRC1, CTRL_REG, SRC_OUT_FLOWCLR);
}

/**
 * \brief Disable SRC
 *
 * \param [in] id identifies SRC1, SRC2
 */
__STATIC_INLINE void hw_src_disable(HW_SRC_ID id)
{

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        HW_SRC_REG_CLR_BIT(id, SRC1, CTRL_REG, SRC_EN);
}

/**
 * \brief Check if SRC is enabled
 *
 * \param [in] id       identifies SRC1, SRC2
 * \return
 *                     \retval True if it is enabled
 *                     \retval False if it is disabled
 *
 */
__STATIC_INLINE bool hw_src_is_enabled(HW_SRC_ID id)
{

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        return (HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_EN));
}

/**
 * \brief Enable SRC FIFO. FIFO is used to store samples from/to SRC
 *
 * \param [in] id       identifies SRC1, SRC2
 * \param[in] direction The SRC FIFO direction.
 *                       HW_SRC_IN  - FIFO is used to store samples from memory to SRC
 *                       HW_SRC_OUT - FIFO is used to store samples from SRC to memory
 */
__STATIC_INLINE void hw_src_enable_fifo(HW_SRC_ID id, HW_SRC_DIRECTION direction)
{

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        switch (direction) {
        case HW_SRC_IN:
                HW_SRC_REG_CLR_BIT(id, SRC1, CTRL_REG, SRC_FIFO_DIRECTION);
                break;
        case HW_SRC_OUT:
                HW_SRC_REG_SET_BIT(id, SRC1, CTRL_REG, SRC_FIFO_DIRECTION);
                break;
        default:
                ASSERT_WARNING(0);
        }
        HW_SRC_REG_SET_BIT(id, SRC1, CTRL_REG, SRC_FIFO_ENABLE);
}

/**
 * \brief Disable SRC FIFO. On each SRC request, one sample is serviced.
 *
 * \param [in] id identifies SRC1, SRC2
 */
__STATIC_INLINE void hw_src_disable_fifo(HW_SRC_ID id)
{

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        HW_SRC_REG_CLR_BIT(id, SRC1, CTRL_REG, SRC_FIFO_ENABLE);
}

/**
 * \brief Check if SRC FIFO is enabled. FIFO is used to store samples from/to SRC
 *
 * \param [in]   id identifies SRC1, SRC2
 * \return
 *              \retval True if it is enabled
 *              \retval False if it is disabled
 */
__STATIC_INLINE bool hw_src_is_fifo_enabled(HW_SRC_ID id)
{

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        return(HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_FIFO_ENABLE));
}

/* *************************************************************************
 *
 *                              SET FUNCTIONS
 *
 * ************************************************************************* */

/**
 * \brief Set Automatic Conversion mode
 *
 * \param [in] id               identifies SRC1, SRC2
 * \param[in] direction         Input/Output direction of data flow allowed values:
 *                                       HW_SRC_IN, HW_SRC_OUT
 */
__STATIC_INLINE void hw_src_set_automode(HW_SRC_ID id, HW_SRC_DIRECTION direction)
{

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        if (direction == HW_SRC_IN) {
                HW_SRC_REG_SET_BIT(id, SRC1, CTRL_REG, SRC_IN_AMODE);
        } else {
                HW_SRC_REG_SET_BIT(id, SRC1, CTRL_REG, SRC_OUT_AMODE);
        }
}

/**
 * \brief Clear Automatic Conversion mode. Use manual mode
 *
 * \param [in] id               identifies SRC1, SRC2
 * \param[in] direction         Input/Output direction of data flow allowed values:
 *                                       HW_SRC_IN, HW_SRC_OUT
 */
__STATIC_INLINE void hw_src_set_manual_mode(HW_SRC_ID id, HW_SRC_DIRECTION direction)
{

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        if (direction == HW_SRC_IN) {
                HW_SRC_REG_CLR_BIT(id, SRC1, CTRL_REG, SRC_IN_AMODE);
        } else {
                HW_SRC_REG_CLR_BIT(id, SRC1, CTRL_REG, SRC_OUT_AMODE);
        }
}

/**
 * \brief Select the SRC input
 *
 * \note call this function once SRC interface initialization is done
 *
 * \param[in] input          the SRC input
 * \param[in, out] config    the configuration structure of SRC
 */
__STATIC_INLINE void hw_src_select_input(HW_SRC_SELECTION input, hw_src_config_t *config)
{
        ASSERT_WARNING(input < HW_SRC_SELECTION_SIZE);

        config->data_input = input;

        ASSERT_WARNING(config->id == HW_SRC1 || config->id == HW_SRC2);
        uint32_t address = SRCBA(config->id)->MUX_REG;

        switch (config->data_input) {
        case HW_SRC_PDM:
                HW_SRC_REG_SET_FIELD(SRC1, MUX_REG, PDM1_MUX_IN, address, HW_PDM_INPUT_MUX_PDM_INPUT);
                HW_SRC_REG_CLR_FIELD(SRC1, MUX_REG, SRC1_MUX_IN, address);
                break;
        case HW_SRC_PCM:
                HW_SRC_REG_SET_FIELD(SRC1, MUX_REG, SRC1_MUX_IN, address, HW_SRC_INPUT_MUX_PCM_OUT_REG);
                HW_SRC_REG_CLR_FIELD(SRC1, MUX_REG, PDM1_MUX_IN, address); // HW_PDM_INPUT_MUX_SRCx_MUX_IN
                break;
        case HW_SRC_REGS:
                HW_SRC_REG_SET_FIELD(SRC1, MUX_REG, SRC1_MUX_IN, address, HW_SRC_INPUT_MUX_SRCx_IN_REG);
                HW_SRC_REG_CLR_FIELD(SRC1, MUX_REG, PDM1_MUX_IN, address);// HW_PDM_INPUT_MUX_SRCx_MUX_IN
                break;
# if dg_configUSE_HW_SDADC
        case HW_SRC_SDADC:
                HW_SRC_REG_SET_FIELD(SRC1, MUX_REG, SRC1_MUX_IN, address, HW_SRC_INPUT_MUX_SDADC_OUT);
                HW_SRC_REG_CLR_FIELD(SRC1, MUX_REG, PDM1_MUX_IN, address);// HW_PDM_INPUT_MUX_SRCx_MUX_IN
                break;
# endif /* dg_configUSE_HW_SDADC */
        default:
                ASSERT_WARNING(0);
        }

        SRCBA(config->id)->MUX_REG = address;
}

/**
 * \brief Write data to an input SRC register
 *
 * \param [in] id       identifies SRC1, SRC2
 * \param[in] stream    The input stream (1 or 2)
 * \param[in] value     The data to be written
 */
__STATIC_INLINE void hw_src_write_input(HW_SRC_ID id, uint8_t stream, uint32_t value)
{

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        switch (stream) {
        case 1:
                HW_SRC_REG_SETF(id, SRC1, IN1_REG, SRC_IN, value);
                break;
        case 2:
                HW_SRC_REG_SETF(id, SRC1, IN2_REG, SRC_IN, value);
                break;
        default:
                ASSERT_WARNING(0);
        }
}

/* *************************************************************************
 *
 *                              GET FUNCTIONS
 *
 * ************************************************************************* */

/**
 * \brief Get the mode
 *
 * \param [in] id            identifies SRC1, SRC2
 * \param[in] direction      Input/Output direction of data flow allowed values:
 *                                       HW_SRC_IN, HW_SRC_OUT
 * \return mode
 *                            \retval 0 for manual mode
 *                            \retval 1 for automatic mode
 */
__STATIC_INLINE bool hw_src_is_auto_mode(HW_SRC_ID id, HW_SRC_DIRECTION direction)
{

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        if (direction == HW_SRC_IN) {
                return HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_IN_AMODE);
        } else {
                return HW_SRC_REG_GETF(id, SRC1, CTRL_REG, SRC_OUT_AMODE);
        }
}

/**
 * \brief Read data from an output SRC register
 *
 * \param [in] id       identifies SRC1, SRC2
 * \param[in] stream    The output stream (1 or 2)
 *
 * \return              The data read
 */
__STATIC_INLINE uint32_t hw_src_read_output(HW_SRC_ID id, uint8_t stream)
{

        ASSERT_WARNING(id == HW_SRC1 || id == HW_SRC2);

        switch (stream) {
        case 1:
                return HW_SRC_REG_GETF(id, SRC1, OUT1_REG, SRC_OUT);
        case 2:
                return HW_SRC_REG_GETF(id, SRC1, OUT2_REG, SRC_OUT);
        default:
                ASSERT_WARNING(0);
                return 0;
        }
}

/**
 * \brief Check if SRC flow errors have occurred and clear the indication
 *
 * \param [in] id               identifies SRC1, SRC2
 * \param[in] direction         Input/Output direction
 *
 * \return                      The flow status
 */
HW_SRC_FLOW_STATUS hw_src_get_flow_status(HW_SRC_ID id, HW_SRC_DIRECTION direction);

/**
 * \brief Initialize the SRC
 *
 * Configure the SRC sampling frequencies, the input down-sampler and output up-sampler IIR filters,
 * the conversion modes, the divider of the internally generated clock and enable the clock
 *
 * \note call hw_src_enable() once SRC interface initialization is done
 *
 * \param[in]   id          identifies SRC1, SRC2
 * \param[out]  config      configuration structure of SRC
 */
void hw_src_init(HW_SRC_ID id, hw_src_config_t *config);
#endif /* dg_configUSE_HW_SRC */
#endif /* HW_SRC_H_ */
/**
 * \}
 * \}
 */
