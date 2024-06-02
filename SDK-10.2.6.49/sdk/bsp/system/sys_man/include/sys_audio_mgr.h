/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_AUDIO_MANAGER Audio Manager Service
 *
 * \brief Audio Manager Service
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_audio_mgr.h
 *
 * @brief Audio manager API
 *
 * Copyright (C) 2019-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_AUDIO_MGR_H_
#define SYS_AUDIO_MGR_H_


#if dg_configUSE_SYS_AUDIO_MGR

#include <stdbool.h>
#include <string.h>
#include "hw_dma.h"
#include "hw_pdm.h"
#include "hw_pcm.h"

# if dg_configUSE_HW_SDADC
#include "hw_sdadc.h"
# endif /* dg_configUSE_HW_SDADC */

#define MAX_NO_PATHS         4          // the max number of concurrently supported audio data paths according to device family

/**
 * \brief Audio unit manager input/output source/sink kind.
 */
typedef enum {
        AUDIO_INVALID = 0,     /**< Invalid device */
        AUDIO_PCM,             /**< In/out PCM */
        AUDIO_PDM,             /**< In/out PDM */
        AUDIO_MEMORY,          /**< In/out memory (DMA) data */
# if dg_configUSE_HW_SDADC
        AUDIO_SDADC,           /**< In/out SDADC */
# endif /* dg_configUSE_HW_SDADC */
        SIZE_OF_AUDIO              /**< The size of enum */
} SYS_AUDIO_MGR_DEVICE;

/**
 \brief Interface mode.
 */
typedef enum {
        MODE_SLAVE = 0,        /**< Interface in slave mode, i.e. clocked externally. */
        MODE_MASTER = 1,       /**< Interface in master mode, i.e. it provides the clock signal. */
} SYS_AUDIO_MGR_MODE;

/**
 * \brief PCM formats
 */
typedef enum {
        PCM_MODE = 0,           /**< General case of PCM mode */
        I2S_MODE,               /**< I2S mode */
        IOM2_MODE,              /**< IOM2 mode */
        TDM_MODE                /**< TDM mode */
} SYS_AUDIO_MGR_PCM_FORMATS;

/**
 * \brief Use of SRC
 */
typedef enum {
        NO_SRC,                  /**< Not use of SRC if applicable */
        SRC_1,                   /**< Use of HW_SRC1 */
        SRC_2,                   /**< Use of HW_SRC2 */
        SRC_AUTO                 /**< Use of SRC automated */
}SYS_AUDIO_MGR_SRC_USE;

/**
 * \brief PDM specific configuration
 */
typedef struct {
        SYS_AUDIO_MGR_MODE mode;         /**< The mode of the interface. Master or slave. */
        uint32_t clk_frequency;          /**< PDM_CLK frequency 62.5 kHz - 4 MHz. It should be noted
                                              that the audio quality degrades when the oversampling
                                              ratio is less than 64. For an 8 kHz sample rate
                                              the minimum recommended PDM clock rate is
                                              64 x 8 kHz = 512 kHz. */

        HW_PDM_CHANNEL_CONFIG channel;   /**< Programmable Left/Right channel for output only */
        HW_PDM_DI_DELAY in_delay;        /**< PDM input delay */
        HW_PDM_DO_DELAY out_delay;       /**< PDM output delay */
        bool swap_channel;               /**< PDM  swap channel this parameter is important only when
                                              2 channel are on the PDM bus*/
} sys_audio_pdm_specific_t;


/**
 * \brief PCM specific configuration
 */
typedef struct {
        SYS_AUDIO_MGR_MODE mode;                          /**< The mode of the interface. Master or slave. */
        SYS_AUDIO_MGR_PCM_FORMATS format;                 /**< Interface PCM formats from SYS_AUDIO_MGR_PCM_FORMATS.
                                                                All modes are supported */
        HW_PCM_CLOCK clock;                               /**< Interface clock - DIVN=32MHz, DIV1=sys_clk */
        uint32_t sample_rate;                             /**< The sample rate of the sample rate converter (Hz).
                                                                The SRC controller is implementing
                                                                an up-to 192 kHz synchronous interface
                                                                to external audio devices */
        uint8_t channel_delay;                            /**< Channel delay in range 0-3 */
        uint8_t total_channel_num;                        /**< The total channel number,
                                                                that corresponds to the number of 32 bit  PCM IN/OUT registers.
                                                                In case of I2S, TDM, total channel number is 2 for the Left and Right audio channels*/
        HW_PCM_DO_OUTPUT_MODE output_mode;                /**< PCM DO output mode.
                                                                HW_PCM_DO_OUTPUT_PUSH_PULL is supported */
        uint8_t bit_depth;                                /**< The number of bits per channel.
                                                                 16, 24, 32 bits per channels are supported */
        HW_PCM_CLK_GENERATION clk_generation;             /**< This is used to enable the
                                                                fractional or the integer only feature of
                                                                the pcm. */
        HW_PCM_FSC_DELAY fsc_delay;                       /**< PCM FSC starts one cycle before
                                                                MSB bit otherwise at the same time
                                                                as MSB bit. Only used for PCM_MODE
                                                                other PCM formats are set appropriately */
        HW_PCM_FSC_POLARITY inverted_fsc_polarity;        /**< The polarity of the fsc signal
                                                                can be inverted with this field. Only used for PCM_MODE, IOM2_MODE
                                                                other PCM formats are set appropriately */
        HW_PCM_CLK_POLARITY inverted_clk_polarity;        /**< The polarity of the clk signal can be inverted with this field.
                                                                Only used for PCM_MODE, IOM2_MODE
                                                                other PCM formats are set appropriately */
        HW_PCM_CYCLE_PER_BIT cycle_per_bit;               /**< PCM clock cycles per bit. Only used for PCM_MODE, I2S_MODE,
                                                                TDM_MODE for IOM2_MODE is set appropriately */
        uint8_t fsc_length;                               /**< PCM FSC length (in number of bytes).
                                                                Only used for PCM_MODE, other PCM formats
                                                                calculate this automatically*/
} sys_audio_pcm_specific_t;

/**
 * \brief DMA configuration for callback function
 */
typedef struct {
        uint32_t address;         /**< source/destination address */
        uint32_t buff_len_total;  /**< The total buffer size in bytes, comprising one/multiple equal size chunks */
        uint32_t buff_len_cb;     /**< Size in bytes of a buffer chunk filled/consumed before each call-back */
        uint32_t buff_len_pos;    /**< At call-back, the start of current chunk being recorded/played */
        uint8_t  channel_num;     /**< For DMA IRQ to determine when all channels are processed and
                                       issue a single application IRQ to process all buffers together */
        bool     stereo;          /**< Tells DMA IRQ if it actually needs to wait for another channel */
} sys_audio_mgr_buffer_data_block_t;

/**
 * \brief Asynchronous callback function. Execute when new audio data available.
 *
 * \param[in] buff_data_block pointer to the audio buffer data block
 * \param[in] app_ud Application user data
 *
 */
typedef void (*sys_audio_mgr_buffer_ready_cb)(sys_audio_mgr_buffer_data_block_t *buff_data_block, void *app_ud);

typedef struct {
        bool            use_prio;       /**< Use DMA priority */
        HW_DMA_PRIO     prio[2];        /**< DMA priority. tab[0] -left channel, tab[1] - right channel */
} sys_audio_dma_prio_t;

/**
 * \brief MEMORY specific configuration
 */
typedef struct {
        HW_DMA_CHANNEL                dma_channel[2];                 /**< DMA channel. tab[0] - left channel, tab[1] - right channel */
        uint32_t                      buff_addr[2];                   /**< Data input or output address. tab[0] - left channel, tab[1] - right channel */
        uint32_t                      total_buffer_len;               /**< Total buffer length in bytes available for each audio channel */
        uint32_t                      cb_buffer_len;                  /**< Number of bytes dma required to execute callback routine */
        sys_audio_mgr_buffer_ready_cb cb;                             /**< Data buffer package ready callback */
        void*                         app_ud;                         /**< Application user data that will be passed to callback */
        uint32_t                      sample_rate;                    /**< The sample rate of the sample rate converter (Hz).
                                                                           The PCM controller is implementing an up-to 192 kHz
                                                                           synchronous interface to external audio devices */
        bool                          stereo;                         /**< Selects dual channel operation */
        uint8_t                       bit_depth;                      /**< The number of bits per channel at a sample */
        bool                          circular;                       /**< Use circular buffer */
        sys_audio_dma_prio_t          dma_prio;                       /**< DMA channel priority */
} sys_audio_memory_specific_t;

# if dg_configUSE_HW_SDADC
/**
 * \brief SDADC specific configuration
 */
typedef struct {
        HW_SDADC_PGA_GAIN pga_gain;       /**< PGA gain selection */
        HW_SDADC_PGA_MODE pga_mode;       /**< PGA mode selection */
} sys_audio_sdadc_specific_t;
# endif /* dg_configUSE_HW_SDADC */

/**
 * \brief Input/Output audio device specific configuration
 */
typedef struct {
        SYS_AUDIO_MGR_DEVICE device_type; /**< The kind of data device to be used as input or output. */

        union {
                sys_audio_pdm_specific_t pdm_param;       /**< PDM device configuration */
                sys_audio_pcm_specific_t pcm_param;       /**< PCM device configuration */
                sys_audio_memory_specific_t memory_param; /**< Memory configuration */
# if dg_configUSE_HW_SDADC
                sys_audio_sdadc_specific_t sdadc_param;   /**< SDADC configuration */
# endif /* dg_configUSE_HW_SDADC */
        };
} sys_audio_device_t;

/**
 * \brief Audio path configuration
 */
typedef struct {
        sys_audio_device_t *dev_in;     /**< Input device */
        sys_audio_device_t *dev_out;    /**< Output device */
} audio_path_t;

/**
 * \brief System audio path configuration
 */
typedef struct {
        audio_path_t audio_path[MAX_NO_PATHS]; /**< Audio data paths*/
} sys_audio_path_t;

/**
 * \brief Audio start function for a path that was previously configured.
 *
 * \sa sys_audio_mgr_open
 *
 * \param[in] idx  path id
 *
 * \return
 *           \retval true in case of starting audio path with success
 *           \retval false in case of starting audio path failed
 */
bool sys_audio_mgr_start(uint8_t idx);

/**
 * \brief Audio stop function.
 *
 * \param[in] idx  path id
 *
 * \return
 *           \retval true in case of stopping devices with success
 *           \retval false in case of stopping devices audio manager failed
 *
 * \sa sys_audio_mgr_start()
 */
bool sys_audio_mgr_stop(uint8_t idx);

/**
 * \brief Open and start device input to output path - initialize devices properly
 *
 * \param[in] dev_in     data structure of input audio device
 * \param[in] dev_out    data structure of output audio device
 * \param[in] src        Use of src
 *
 * \note devs should be pointers to global variables
 *
 * \note this function should be followed by sys_audio_mgr_close_path() to be able to be called again
 *
 * \return      the newly acquired path index
 */
uint8_t sys_audio_mgr_open_path(sys_audio_device_t *dev_in,  sys_audio_device_t *dev_out, SYS_AUDIO_MGR_SRC_USE src);

/**
 * \brief Stop selected audio devices, close audio path and release resources.
 *        To call this function sys_audio_mgr_open_path() must be called previously
 *
 * \param[in] idx     index to a particular data path
 *
 * \sa sys_audio_mgr_open_path()
 */
void sys_audio_mgr_close_path(uint8_t idx);

/**
 * \brief Open device input to output paths - initialize devices properly
 *
 * \param[in] devs     data structure of input audio devices
 *
 * \note devs should be pointers to global variables
 *
 * \note this function should be followed by sys_audio_mgr_close() to be able to be called again
 * \deprecated This function is deprecated. User shall call sys_audio_mgr_open_path() instead.
 */
DEPRECATED_MSG("API no longer supported, use sys_audio_mgr_open_path() instead.")
__STATIC_INLINE void sys_audio_mgr_open(sys_audio_path_t *devs)
{
        for (uint8_t idx = 0; idx < MAX_NO_PATHS; idx++) {
                if (devs->audio_path[idx].dev_in != NULL &&
                    devs->audio_path[idx].dev_out != NULL &&
                    devs->audio_path[idx].dev_in->device_type != AUDIO_INVALID &&
                    devs->audio_path[idx].dev_out->device_type != AUDIO_INVALID) {
                        sys_audio_mgr_open_path(devs->audio_path[idx].dev_in,
                                                  devs->audio_path[idx].dev_out, SRC_AUTO);
                }
        }
}

/**
 * \brief Close audio paths and release resources.
 *        To call this function sys_audio_mgr_open() must be called previously
 *
 * \sa sys_audio_mgr_open()
 * \deprecated This function is deprecated. User shall call sys_audio_mgr_close_path() instead.
 */
DEPRECATED_MSG("API no longer supported, use sys_audio_mgr_close_path() instead.")
__STATIC_INLINE void sys_audio_mgr_close(void)
{
        for (uint8_t idx = 0; idx < MAX_NO_PATHS; idx++) {
                sys_audio_mgr_close_path(idx);
        }
}

#  endif /* dg_configUSE_SYS_AUDIO_MGR */
#endif /* SYS_AUDIO_MGR_H_ */

/**
 * \}
 * \}
 */
