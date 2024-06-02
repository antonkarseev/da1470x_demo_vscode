/**
 ****************************************************************************************
 *
 * @file sys_audio_mgr.c
 *
 * @brief System Audio manager
 *
 * Copyright (C) 2019-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_SYS_AUDIO_MGR

#include <sdk_defs.h>
#ifdef OS_PRESENT
#include "osal.h"
#include "resmgmt.h"
#endif
#include "hw_src.h"
#include "hw_sys.h"
#include "sys_power_mgr.h"
#include "sys_audio_mgr.h"

/* SRC Defaults */
#define DEFAULT_SRC_CLK         32000000

/* PCM Defaults */
#define DEFAULT_PCM_OUTPUT_MODE         HW_PCM_DO_OUTPUT_PUSH_PULL
#define DEFAULT_PCM_CYCLE_PER_BIT       HW_PCM_ONE_CYCLE_PER_BIT
#define DEFAULT_PCM_FSC_DELAY           HW_PCM_FSC_STARTS_SYNCH_TO_MSB_BIT

#define BIT_DEPTH_MAX            32                     /* the max number of bit depth */
#define CHANNEL_NUM_MAX          2                      /* the max number of audio channels */

#define AUDIO    PCM1

#define SYS_AUDIO_DEFAULT_DMA_LEFT_PRIO         (HW_DMA_PRIO_2)
#define SYS_AUDIO_DEFAULT_DMA_RIGHT_PRIO        (HW_DMA_PRIO_2)

typedef enum {
        DIRECTION_INPUT,                                /* Audio interface's data direction input */
        DIRECTION_OUTPUT,                               /* Audio interface's data direction output */
} SYS_AUDIO_MGR_DIRECTION;

typedef enum {
        DMA_DIR_MEM_TO_PERIPH,                          /* DMA direction from memory to peripheral */
        DMA_DIR_PERIPH_TO_MEM,                          /* DMA direction from peripheral to memory */
} SYS_AUDIO_MGR_DMA_DIR;

typedef struct {
        HW_DMA_CHANNEL dma_channel_number;              /* DMA channel number */
        sys_audio_mgr_buffer_ready_cb cb;               /* DMA callback */
        sys_audio_path_t *path;                         /* Audio data path where DMA is used */
        sys_audio_mgr_buffer_data_block_t buff_block;   /* DMA buffer data block */
        uint8_t bit_depth;                              /* Audio bit depth */
        void *app_ud;                                   /* Application user data */
        bool circular;                                  /* Enable Circular DMA */
} dma_user_data_t;

/*
 * SRC static configuration
 */
typedef struct {
        RES_ID resource_id;                             /* Resource's ID */
        HW_SRC_ID hw_src_id;                            /* SRC's ID */
} src_static_cfg_t;

static const src_static_cfg_t src_static_cfg[] = {
        {RES_ID_SRC1, HW_SRC1},
        {RES_ID_SRC2, HW_SRC2}
};

static uint8_t nof_paths = 0;
static sys_audio_path_t sys_audio_path;
static hw_src_config_t src_config[MAX_NO_PATHS];
static  dma_user_data_t dma_user_data[CHANNEL_NUM_MAX * MAX_NO_PATHS * 2];
static bool single_dev_type_out[SIZE_OF_AUDIO];
static bool single_dev_type_in[SIZE_OF_AUDIO];
static bool audio_path_idx_status[MAX_NO_PATHS];
static bool pcm_loopback = false;
static sleep_mode_t pm_mode = pm_mode_active;
# if dg_configUSE_HW_SDADC
static const uint32_t sdadc_sample_rate = 16000;        // For SDADC, sample rate is constant at 16KHz
# endif /* dg_configUSE_HW_SDADC */

static uint32_t get_sampling_rate(sys_audio_device_t *dev)
{
        uint32_t ret = 0;

        switch (dev->device_type) {
        case AUDIO_PDM:
                break;
        case AUDIO_PCM:
                ret = dev->pcm_param.sample_rate;
                break;
        case AUDIO_MEMORY:
                ret = dev->memory_param.sample_rate;
                break;
# if dg_configUSE_HW_SDADC
        case AUDIO_SDADC:
                ret = sdadc_sample_rate;
                break;
# endif /* dg_configUSE_HW_SDADC */
        default :
                ASSERT_ERROR(0);
                break;
        }

        return ret;
}

static bool is_src_conversion_required(sys_audio_device_t *dev_in,  sys_audio_device_t *dev_out, SYS_AUDIO_MGR_SRC_USE src)
{
# if dg_configUSE_HW_SDADC
        if ((dev_in->device_type == AUDIO_SDADC) && (dev_out->device_type != AUDIO_MEMORY)) {
                return true;
        }
# endif /* dg_configUSE_HW_SDADC */

        if ((dev_in->device_type == AUDIO_PDM) || (dev_out->device_type == AUDIO_PDM)) {
                return true;
        }

        /* Return false when SRC_AUTO or NO_SRC is selected for the same sampling rate */
        uint32_t sample_rate_in = get_sampling_rate(dev_in);
        uint32_t sample_rate_out = get_sampling_rate(dev_out);

        if (sample_rate_in == sample_rate_out) {
                if (src == SRC_AUTO || src == NO_SRC) {
                        return false;
                } else {
                        return true;
                }
        }

        return true;
}

#ifdef OS_PRESENT
__STATIC_INLINE resource_mask_t dma_resource_mask(HW_DMA_CHANNEL num)
{
        const resource_mask_t res_mask[] = {
                RES_MASK(RES_ID_DMA_CH0), RES_MASK(RES_ID_DMA_CH1),
                RES_MASK(RES_ID_DMA_CH2), RES_MASK(RES_ID_DMA_CH3),
                RES_MASK(RES_ID_DMA_CH4), RES_MASK(RES_ID_DMA_CH5),
                RES_MASK(RES_ID_DMA_CH6), RES_MASK(RES_ID_DMA_CH7)
        };

        return res_mask[num];
}

static void dma_resource_mng(bool acquire, sys_audio_device_t *dev_id)
{
        uint8_t idx = 0;

        while (idx < CHANNEL_NUM_MAX) {

                if (dev_id->memory_param.dma_channel[idx] != HW_DMA_CHANNEL_INVALID) {

                        if (acquire) {
                                resource_acquire(dma_resource_mask(dev_id->memory_param.dma_channel[idx]), RES_WAIT_FOREVER);

                        } else {
                                resource_release(dma_resource_mask(dev_id->memory_param.dma_channel[idx]));
                        }
                }
                idx++;
        }
}

static void src_resource_mng_implicitly(bool acquire, uint8_t idx)
{
        uint8_t src_id = 0;
        for (src_id = 0; src_id < ARRAY_LENGTH(src_static_cfg); src_id++) {
                if (acquire) {
                        /* Acquire SRC */
                        if (resource_acquire(RES_MASK(src_static_cfg[src_id].resource_id), 0) != 0) {
                                src_config[idx].id = src_static_cfg[src_id].hw_src_id;
                                break;
                        } else {
                                src_config[idx].id = 0;
                        }
                } else {
                        /* Release SRC */
                        if (src_config[idx].id == src_static_cfg[src_id].hw_src_id) {
                                resource_release(RES_MASK(src_static_cfg[src_id].resource_id));
                                src_config[idx].id = 0;
                                break;
                        }
                }
        }

        if (acquire) {
                ASSERT_ERROR(src_config[idx].id != 0);
        }
}
#else /* NO OS */
#define dma_resource_mng(acquire, dev_id)
#define src_resource_mng_implicitly(acquire, idx)
#endif /* OS_PRESENT */

static void dma_transfer_cb(void *user_data, dma_size_t len)
{
        dma_user_data_t *_dma_user_data = (dma_user_data_t *)user_data;
        uint8_t bus_width = 1;

        /* Calculate index for the range DMA may now be recording to or playing back */
        if (_dma_user_data->bit_depth > 16) {
                bus_width = 4;
        } else if (_dma_user_data->bit_depth > 8) {
                bus_width = 2;
        }

        /* Calculate index for the range DMA may now be recording to or playing back in bytes*/
        uint32_t next_buff_len_pos = _dma_user_data->buff_block.buff_len_pos + _dma_user_data->buff_block.buff_len_cb;

        if (_dma_user_data->circular && next_buff_len_pos >= _dma_user_data->buff_block.buff_len_total) {
                next_buff_len_pos -= _dma_user_data->buff_block.buff_len_total;
        }

        _dma_user_data->buff_block.buff_len_pos = next_buff_len_pos;

        if (hw_dma_is_channel_active(_dma_user_data->dma_channel_number)) {

                uint32_t num_of_transfers = ((next_buff_len_pos +
                        _dma_user_data->buff_block.buff_len_cb) / bus_width) - 1;

                if (_dma_user_data->circular && num_of_transfers >= _dma_user_data->buff_block.buff_len_total / bus_width) {
                        num_of_transfers -= _dma_user_data->buff_block.buff_len_total / bus_width;
                }

                if (num_of_transfers > UINT16_MAX) {
                        num_of_transfers &= UINT16_MAX;
                }

                hw_dma_channel_update_int_ix(_dma_user_data->dma_channel_number, num_of_transfers);
        } else if (next_buff_len_pos < _dma_user_data->buff_block.buff_len_total) {

                uint32_t _len = (_dma_user_data->buff_block.buff_len_total - next_buff_len_pos) /
                        bus_width;
                uint16_t num_of_transfers = _dma_user_data->buff_block.buff_len_cb / bus_width - 1;
                uint32_t address = _dma_user_data->buff_block.address + next_buff_len_pos;
                if (_len > UINT16_MAX + 1) {
                        _len = UINT16_MAX + 1;
                }

                if (num_of_transfers > UINT16_MAX) {
                        num_of_transfers = UINT16_MAX;
                }

                if (_dma_user_data->dma_channel_number % 2 == 0) {
                        hw_dma_channel_update_destination(_dma_user_data->dma_channel_number,
                                (void *)address,
                                _len,
                                dma_transfer_cb);
                } else {
                        hw_dma_channel_update_source(_dma_user_data->dma_channel_number,
                                (void *)address,
                                _len,
                                dma_transfer_cb);
                }

                hw_dma_channel_update_int_ix(_dma_user_data->dma_channel_number, num_of_transfers);
                hw_dma_channel_enable(_dma_user_data->dma_channel_number, HW_DMA_STATE_ENABLED);
        }

        /* We might not have a call-back - if using two audio paths (e.g. PDM->RAM and RAM->PCM)
         * only one path's call-back is required to do any post-capture/pre-playback processing.
         *
         * If doing processing in IRQ context, the app call-back must maintain a read/write index,
         * and use buff_block.buff_len_cb both to update it and specify how much data to process.
         * It should reset it's index to 0 when >= buff_block.buff_len_total.
         *
         * If passing responsibility to a task with notify, the task should maintain a read/write index
         * and compare buff_block.buff_len_pos to calculate quantity to process. This way, multiple
         * IRQs and delayed notification handling will lead to it processing the available range of
         * data, not just a single chunk. It must cope with buff_len_pos having wrapped through 0.
         */
        if (_dma_user_data->cb != NULL) {
                _dma_user_data->cb(&(_dma_user_data->buff_block), _dma_user_data->app_ud);
        }
}

static void initialize_dma_reg(uint8_t path_num, uint8_t max_path_num, sys_audio_memory_specific_t *param, SYS_AUDIO_MGR_DMA_DIR dir)
{
        /* Setup generic left/right channel data parameter */

        /* The value of bit depth must not exceed the size of 32 bits
         * and an integer multiplier of bytes*/
        ASSERT_ERROR((param->bit_depth != 0) && (param->bit_depth % 8 == 0) && (param->bit_depth <= BIT_DEPTH_MAX));
        ASSERT_ERROR(param->cb_buffer_len);
        ASSERT_ERROR(param->total_buffer_len);
        ASSERT_ERROR(param->cb_buffer_len <= param->total_buffer_len);

        DMA_setup channel_setup;
        uint32_t offset = 0;

        channel_setup.circular = param->circular ?  HW_DMA_MODE_CIRCULAR : HW_DMA_MODE_NORMAL;

        if (param->bit_depth > 16) {
                channel_setup.bus_width = HW_DMA_BW_WORD;
        } else if (param->bit_depth > 8) {
                channel_setup.bus_width = HW_DMA_BW_HALFWORD;
                offset = 2;
        } else {
                channel_setup.bus_width = HW_DMA_BW_BYTE;
                offset = 3;
        }

        channel_setup.length = param->total_buffer_len >> (channel_setup.bus_width/2);
        channel_setup.irq_nr_of_trans = param->cb_buffer_len >> (channel_setup.bus_width/2);

        if (channel_setup.length > UINT16_MAX + 1) {
                channel_setup.length = UINT16_MAX + 1;
        }

        channel_setup.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
        channel_setup.dreq_mode = HW_DMA_DREQ_TRIGGERED;
        channel_setup.burst_mode = HW_DMA_BURST_MODE_DISABLED;
        channel_setup.a_inc = (dir == DMA_DIR_MEM_TO_PERIPH) ? HW_DMA_AINC_TRUE : HW_DMA_AINC_FALSE;
        channel_setup.b_inc = (dir == DMA_DIR_MEM_TO_PERIPH) ? HW_DMA_BINC_FALSE : HW_DMA_BINC_TRUE;
        channel_setup.callback = dma_transfer_cb;
        channel_setup.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE;
        channel_setup.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
# if dg_configUSE_HW_SDADC

        audio_path_t* const paths = sys_audio_path.audio_path;

# endif /* dg_configUSE_HW_SDADC */
        /*
         * Select DMA_REQ_MUX_REG if SRCx is used
         */
        if (src_config[path_num].id != 0) {
                if (src_config[path_num].id == HW_SRC1) {
                        channel_setup.dma_req_mux = HW_DMA_TRIG_SRC_RXTX;
                } else if (src_config[path_num].id == HW_SRC2) {
                        channel_setup.dma_req_mux = HW_DMA_TRIG_SRC2_RXTX;
                }
        } else {
# if dg_configUSE_HW_SDADC
                if (paths[path_num].dev_in->device_type == AUDIO_SDADC) {
                        channel_setup.dma_req_mux = HW_DMA_TRIG_GP_ADC_APP_ADC;
                }
                else
# endif /* dg_configUSE_HW_SDADC */
                {
                        /*
                         * Select DMA_REQ_MUX_REG if PCM is used
                         */
                        channel_setup.dma_req_mux = HW_DMA_TRIG_PCM_RXTX;
                }
        }

        /* support up to 2 channel */
        for (uint8_t ch = 0; ch < CHANNEL_NUM_MAX; ch++) {

                if (param->dma_channel[ch] != HW_DMA_CHANNEL_INVALID) {

                        channel_setup.channel_number = param->dma_channel[ch];
                        channel_setup.dma_prio = (param->dma_prio.use_prio ? param->dma_prio.prio[ch] :
                                (ch == 0 ? SYS_AUDIO_DEFAULT_DMA_LEFT_PRIO : SYS_AUDIO_DEFAULT_DMA_RIGHT_PRIO));

                        /* Decide data in/out dir */
                        if (dir == DMA_DIR_MEM_TO_PERIPH) {
                                //Odd channels are only applicable for DMA_DIR_MEM_TO_PERIPH
                                ASSERT_ERROR((channel_setup.channel_number & 0x1) == 1);
                                channel_setup.src_address = param->buff_addr[ch];
                                if (src_config[path_num].id != 0) {
                                        channel_setup.dest_address = (ch == 0) ?
                                                ((uint32_t)&(SRCBA(src_config[path_num].id)->SRC1_IN1_REG) + offset) :
                                                ((uint32_t)&(SRCBA(src_config[path_num].id)->SRC1_IN2_REG) + offset);
                                } else {

                                        channel_setup.dest_address = (ch == 0) ?
                                                ((uint32_t)&(AUDIO->PCM1_OUT1_REG) + offset) :
                                                ((uint32_t)&(AUDIO->PCM1_OUT2_REG) + offset);
                                }
                        } else {
                                channel_setup.dest_address = param->buff_addr[ch];
                                if (src_config[path_num].id != 0) {

                                        //Even channels are only applicable for DMA_DIR_PERIPH_TO_MEM
                                        ASSERT_ERROR((channel_setup.channel_number & 0x1) == 0);
                                        channel_setup.src_address = (ch == 0) ?
                                                ((uint32_t)&(SRCBA(src_config[path_num].id)->SRC1_OUT1_REG) + offset) :
                                                ((uint32_t)&(SRCBA(src_config[path_num].id)->SRC1_OUT2_REG) + offset);
                                } else {
# if dg_configUSE_HW_SDADC
                                        if (paths[path_num].dev_in->device_type == AUDIO_SDADC) {
                                                /* Odd channels are only applicable for SDADC */
                                                ASSERT_ERROR((channel_setup.channel_number & 0x1) == 1);

                                                channel_setup.src_address = (uint32_t)&(SDADC->SDADC_RESULT_REG);
                                        }
                                        else
# endif /* dg_configUSE_HW_SDADC */
                                        {
                                                //Even channels are only applicable for DMA_DIR_PERIPH_TO_MEM
                                                ASSERT_ERROR((channel_setup.channel_number & 0x1) == 0);
                                                channel_setup.src_address = (ch == 0) ?
                                                        ((uint32_t)&(AUDIO->PCM1_IN1_REG) + offset) :
                                                        ((uint32_t)&(AUDIO->PCM1_IN2_REG) + offset);
                                        }
                                }
                        }

                        uint8_t i = ch | (dir << 1);

                        i+= 4 * (path_num);
                        dma_user_data[i].dma_channel_number = param->dma_channel[ch];
                        dma_user_data[i].buff_block.buff_len_total = param->total_buffer_len;
                        dma_user_data[i].buff_block.buff_len_pos = 0;
                        dma_user_data[i].buff_block.buff_len_cb =  param->cb_buffer_len;
                        dma_user_data[i].buff_block.address = param->buff_addr[ch];
                        dma_user_data[i].cb = param->cb;
                        dma_user_data[i].path = &sys_audio_path;
                        dma_user_data[i].app_ud = param->app_ud;
                        dma_user_data[i].buff_block.channel_num = ch;
                        dma_user_data[i].buff_block.stereo = param->stereo;
                        dma_user_data[i].bit_depth = param->bit_depth;
                        dma_user_data[i].circular = param->circular;

                        channel_setup.user_data = &dma_user_data[i];

                        hw_dma_channel_initialization(&channel_setup);
                }
        }
}

#if dg_configUSE_HW_SDADC
static void initialize_sdadc_reg(uint8_t idx, sys_audio_sdadc_specific_t *param)
{
        sdadc_config adc_config = {
                .dma_setup = NULL,
                .result_mode = HW_SDADC_RESULT_SAMPLE_EXTENSION,//In SDADC_RESULT_REG the 16bits output, the ENOB is 13 hence the 3 LSBs are considered to be noise and should be discarded.
                .pga_en = HW_SDADC_PGA_ENABLE_BOTH,
                .pga_bias = HW_SDADC_PGA_BIAS_66,
                .pga_gain = param->pga_gain,
                .pga_mode = param->pga_mode,
        };

        hw_sdadc_init(&adc_config);

        audio_path_t * const path = &sys_audio_path.audio_path[idx];

        /* Enable DMA for SDADC -> DMA-> MEMORY without using SRC */
        if (path->dev_out->device_type == AUDIO_MEMORY && src_config[idx].id == 0) {
                hw_sdadc_set_dma_functionality(true);
        }

        hw_sdadc_enable();
}
#endif /* dg_configUSE_HW_SDADC */

static void initialize_pdm_reg(uint8_t idx, sys_audio_pdm_specific_t *param, SYS_AUDIO_MGR_DIRECTION dir)
{
        hw_pdm_config_t config = {0};
        audio_path_t* const paths = sys_audio_path.audio_path;

        /* Over-sampling ratio should be at least 64 times the sampling rate to
         * avoid degradation of the audio quality */
        sys_audio_device_t *other_dev = ((dir == DIRECTION_INPUT) ? paths[idx].dev_out : paths[idx].dev_in);

        if (other_dev->device_type == AUDIO_MEMORY) {
                ASSERT_ERROR(param->clk_frequency >= (other_dev->memory_param.sample_rate * 64));
        }

        if (other_dev->device_type == AUDIO_PCM) {
                if (other_dev->pcm_param.sample_rate < 48000) {
                        /* Maximum SRC bandwidth is 24KHz */
                        ASSERT_ERROR(param->clk_frequency >= (other_dev->pcm_param.sample_rate * 64));
                } else {
                        ASSERT_ERROR(param->clk_frequency >= (48000 * 64));
                }
        }

        config.clk_frequency = param->clk_frequency;
        config.config_mode = param->mode;
        config.in_delay = param->in_delay;
        config.out_delay = param->out_delay;
        config.output_channel = param->channel;
        config.data_direction = dir;

        hw_pdm_clk_init(config.clk_frequency);
        hw_pdm_init(src_config[idx].id,  &config);
}

static void validate_pcm_cfg(sys_audio_pcm_specific_t *param)
{
        switch (param->format) {
        case PCM_MODE:
                break;
        case I2S_MODE:
                ASSERT_ERROR(param->total_channel_num == 2);
                ASSERT_ERROR(param->channel_delay == 0);
                break;
        case TDM_MODE:
                ASSERT_ERROR(param->total_channel_num == 2);
                break;
        case IOM2_MODE:
                ASSERT_ERROR(param->channel_delay == 0);
                break;
        default:
                ASSERT_ERROR(0);
                break;
        }
}

static void initialize_pcm_reg(sys_audio_pcm_specific_t *param)
{
        validate_pcm_cfg(param);

        hw_pcm_config_t config = {0};
        hw_pcm_clk_cfg_t pcm_clk = {
                .bit_depth = param->bit_depth,
                .ch_delay = param->channel_delay,
                .chs = param->total_channel_num,
                .clock = param->clock,
                .cycle_per_bit = param->cycle_per_bit,
                .sample_rate = param->sample_rate/1000,
                .slot = 1,
                .div = param->clk_generation,
                .fsc_div = 0
        };

        uint8_t fsc_length = param->fsc_length;

        config.gpio_output_mode = param->output_mode;

        if (param->mode == MODE_SLAVE) {
                config.pcm_mode = HW_PCM_MODE_SLAVE;
        } else {
                config.pcm_mode = HW_PCM_MODE_MASTER;
        }

        /* Channel delays are added as follow:
         * - fsc_edge = 0 which means offset is set
         *      after rising edge (1 time) (case of PCM mode)
         *  - fsc_edge = 1 which means offset is set
         *      after rising and falling edge (two times) (case of TDM mode)
         *  So slot is used to count how many times channel delay (offset) is added
         */
        if (param->format == TDM_MODE) {
                pcm_clk.slot = 2;
        }

        HW_PCM_ERROR_CODE pcm_init_ret = hw_pcm_init_clk(&pcm_clk);

        ASSERT_ERROR(pcm_init_ret == HW_PCM_ERROR_NO_ERROR);

        switch (param->format) {
        case PCM_MODE:
                config.config_mode = HW_PCM_CONFIG_GENERIC_PCM_MODE;
                config.pcm_param.channel_delay = param->channel_delay;
                config.pcm_param.fsc_polarity = param->inverted_fsc_polarity;
                config.pcm_param.clock_polarity = param->inverted_clk_polarity;
                config.pcm_param.fsc_delay = param->fsc_delay;
                config.pcm_param.fsc_div = pcm_clk.fsc_div;
                config.pcm_param.fsc_length = fsc_length;
                break;
        case I2S_MODE:
                fsc_length = param->bit_depth / 8;

                config.config_mode = HW_PCM_CONFIG_I2S_MODE;
                config.i2s_param.fsc_length = fsc_length;
                config.i2s_param.fsc_div = pcm_clk.fsc_div;
                config.i2s_param.fsc_polarity = param->inverted_fsc_polarity;
                break;
        case TDM_MODE:
                fsc_length = (param->bit_depth / 8) + param->channel_delay;

                config.config_mode = HW_PCM_CONFIG_TDM_MODE;
                config.tdm_param.fsc_polarity = param->inverted_fsc_polarity;
                config.tdm_param.channel_delay = param->channel_delay;
                config.tdm_param.fsc_length = fsc_length;
                config.tdm_param.fsc_div = pcm_clk.fsc_div;
                break;
        case IOM2_MODE:
                config.config_mode = HW_PCM_CONFIG_IOM_MODE;
                config.iom_param.fsc_div = pcm_clk.fsc_div;
                config.iom_param.fsc_polarity = param->inverted_fsc_polarity;
                break;
        default:
                ASSERT_ERROR(0);
                break;
        }

        /* The FSC length must be smaller or equal to the bit_depth plus the channel offset,
         *  which is fsc_div
         */
        if (fsc_length > 0) {

                ASSERT_ERROR((fsc_length <= 8)  && fsc_length * 8 <= pcm_clk.fsc_div - 8);

                if (param->cycle_per_bit) {
                        ASSERT_ERROR(pcm_clk.fsc_div > CHANNEL_NUM_MAX * fsc_length);
                }
        }

        hw_pcm_init(&config);
}

static void initialize_reg(uint8_t idx, uint8_t path_num, sys_audio_device_t *dev_id, SYS_AUDIO_MGR_DIRECTION dir)
{
        switch (dev_id->device_type) {
        case AUDIO_PDM:
                initialize_pdm_reg(idx, &(dev_id->pdm_param), dir);
                break;
        case AUDIO_PCM:
                initialize_pcm_reg(&(dev_id->pcm_param));
                break;
        case AUDIO_MEMORY:
                dma_resource_mng(true, dev_id);
                initialize_dma_reg(idx, path_num, &(dev_id->memory_param), dir);
                break;
# if dg_configUSE_HW_SDADC
        case AUDIO_SDADC:
                initialize_sdadc_reg(idx, &(dev_id->sdadc_param));
                break;
# endif /* dg_configUSE_HW_SDADC */
        default :
                ASSERT_ERROR(0);
                break;
        }
}

static void assert_src_pcm_mode(sys_audio_device_t *dev)
{
        /* In case of PCM/IOM MODE as input/output device bit depth should be equal to
         * 32 bits to be processed by SRC for 2 channels (left and right) (32 bits for each register) */
        if (dev->device_type == AUDIO_PCM ) {
                if (dev->pcm_param.format == PCM_MODE || dev->pcm_param.format == IOM2_MODE) {
                        if (dev->pcm_param.total_channel_num == CHANNEL_NUM_MAX) {
                                ASSERT_ERROR(dev->pcm_param.bit_depth == 32);
                        }
                }
        }
}

__STATIC_INLINE HW_SRC_SELECTION get_audio_lld_device(SYS_AUDIO_MGR_DEVICE dev)
{
        switch (dev) {
        case AUDIO_PCM:
                return HW_SRC_PCM;
        case AUDIO_PDM:
                return HW_SRC_PDM;
        case AUDIO_MEMORY:
                return HW_SRC_REGS;
# if dg_configUSE_HW_SDADC
        case AUDIO_SDADC:
                return HW_SRC_SDADC;
# endif /* dg_configUSE_HW_SDADC */
        case AUDIO_INVALID:
        case SIZE_OF_AUDIO:
        default:
                return HW_SRC_SELECTION_SIZE;
        }
}

static void initialize_src_reg(sys_audio_device_t *dev_in, sys_audio_device_t *dev_out, hw_src_config_t* src_cfg)
{
        assert_src_pcm_mode(dev_in);
        assert_src_pcm_mode(dev_out);

        /* Initialize src */

        /* Select the input */
        hw_src_select_input(get_audio_lld_device(dev_in->device_type), src_cfg);

        /* Set src clk at kHz divide Hz by 1000 */
        src_cfg->src_clk = DEFAULT_SRC_CLK/1000;

        src_cfg->in_sample_rate = 0;
        src_cfg->out_sample_rate = 0;

        /* Interfaces with sample rate (PCM/MEMORY)
         * initialize the fsc and iir setting in src
         */
        if (dev_in->device_type != AUDIO_PDM) {
                src_cfg->in_sample_rate = get_sampling_rate(dev_in);
        }

        if (dev_out->device_type != AUDIO_PDM) {
                src_cfg->out_sample_rate = get_sampling_rate(dev_out);
        }

        hw_src_init(src_cfg->id, src_cfg);

        /* Only in case that interface is memory it is used the manual mode */
        if (dev_in->device_type != AUDIO_MEMORY) {
                hw_src_set_automode(src_cfg->id, HW_SRC_IN);
        } else {
                hw_src_set_manual_mode(src_cfg->id, HW_SRC_IN);
        }

        if (dev_out->device_type != AUDIO_MEMORY) {
                hw_src_set_automode(src_cfg->id, HW_SRC_OUT);
        } else {
                hw_src_set_manual_mode(src_cfg->id, HW_SRC_OUT);
        }

        /* Enable the SRC FIFO and set direction. FIFO cannot be enabled in stereo mode */
        if (dev_in->device_type == AUDIO_MEMORY &&
            dev_in->memory_param.stereo == false) {
                hw_src_enable_fifo(src_cfg->id, HW_SRC_IN);
        } else if (dev_out->device_type == AUDIO_MEMORY &&
                   dev_out->memory_param.stereo == false) {
                hw_src_enable_fifo(src_cfg->id, HW_SRC_OUT);
        } else {
                hw_src_disable_fifo(src_cfg->id);
        }
}

static void sys_pcm_loopback_interrupt_cb(void)
{
        hw_pcm_output_write(HW_PCM_OUTPUT_REG_1, hw_pcm_input_read(HW_PCM_INPUT_REG_1));
        hw_pcm_output_write(HW_PCM_OUTPUT_REG_2, hw_pcm_input_read(HW_PCM_INPUT_REG_2));
}

static void select_output(sys_audio_device_t *dev_in,  sys_audio_device_t *dev_out, hw_src_config_t* src_cfg)
{
        switch (dev_out->device_type) {
        case AUDIO_PCM:
                ASSERT_ERROR(hw_pcm_get_pcm_input_mux() == HW_PCM_INPUT_MUX_OFF);
                if (src_cfg && src_cfg->id != 0) {

                        switch ((uint32_t)src_cfg->id) {
                        case (uint32_t)HW_SRC1:
                                hw_pcm_set_pcm_input_mux(HW_PCM_INPUT_MUX_SRC1_OUT);
                        break;
                        case (uint32_t)HW_SRC2:
                                hw_pcm_set_pcm_input_mux(HW_PCM_INPUT_MUX_SRC2_OUT);
                        break;
                        default:
                                ASSERT_ERROR(0);
                                break;
                        }
                } else {
                        hw_pcm_set_pcm_input_mux(HW_PCM_INPUT_MUX_PCM_OUT_REG);

                        if (dev_in->device_type == AUDIO_PCM) {
                                hw_pcm_register_interrupt(sys_pcm_loopback_interrupt_cb);
                        }
                }
                break;
        case AUDIO_PDM:
                /* The use of SRC is not checked as the use of PDM interface needs SRC by default */
                switch ((uint32_t) src_cfg->id) {
                case (uint32_t)HW_SRC1:
                        hw_pdm_set_pdm_output_mux(HW_PDM_MUX_OUT_SRC1);
                break;
                case (uint32_t)HW_SRC2:
                        hw_pdm_set_pdm_output_mux(HW_PDM_MUX_OUT_SRC2);
                break;
                default:
                        ASSERT_ERROR(0);
                        break;
                }
                hw_pdm_set_output_channel_config(src_cfg->id, dev_out->pdm_param.channel);
                break;
                case AUDIO_MEMORY:
                        if (src_cfg && src_cfg->id != 0) {
                                hw_pdm_set_output_channel_config(src_cfg->id, HW_PDM_CHANNEL_NONE);
                        }
                        break;
                default:
                        ASSERT_WARNING(0);
                        break;
        }
}

static void assert_audio_mem_channels_consistency(sys_audio_device_t *dev_in,  sys_audio_device_t *dev_out)
{
        sys_audio_device_t *pcm_dev = NULL;
        sys_audio_device_t *mem_dev = NULL;

        if (dev_in->device_type == AUDIO_PCM && dev_out->device_type == AUDIO_MEMORY) {
                pcm_dev = dev_in;
                mem_dev = dev_out;
        } else if (dev_in->device_type == AUDIO_MEMORY && dev_out->device_type == AUDIO_PCM) {
                pcm_dev = dev_out;
                mem_dev = dev_in;
        } else {
                return;
        }

        switch (pcm_dev->pcm_param.format) {
        case PCM_MODE:
        case IOM2_MODE:
                /* Mono mode */
                if ((pcm_dev->pcm_param.total_channel_num == 1)) {
                        ASSERT_ERROR(mem_dev->memory_param.stereo == false);
                } else {
                        /* Stereo mode*/
                        ASSERT_ERROR(mem_dev->memory_param.stereo == true);
                }
                break;
        case I2S_MODE:
        case TDM_MODE:
                ASSERT_ERROR(mem_dev->memory_param.stereo == true);
                break;
        default:
                break;
        }
}

static void check_cfgs(sys_audio_device_t *dev1, sys_audio_device_t *dev2)
{
        switch (dev1->device_type) {
        case AUDIO_PCM:
                ASSERT_ERROR(dev1->pcm_param.bit_depth == dev2->pcm_param.bit_depth);
                ASSERT_ERROR(dev1->pcm_param.sample_rate == dev2->pcm_param.sample_rate);
                ASSERT_ERROR(dev1->pcm_param.total_channel_num == dev2->pcm_param.total_channel_num);
                ASSERT_ERROR(dev1->pcm_param.channel_delay == dev2->pcm_param.channel_delay);
                ASSERT_ERROR(dev1->pcm_param.clk_generation == dev2->pcm_param.clk_generation);
                ASSERT_ERROR(dev1->pcm_param.clock == dev2->pcm_param.clock);
                ASSERT_ERROR(dev1->pcm_param.cycle_per_bit == dev2->pcm_param.cycle_per_bit);
                ASSERT_ERROR(dev1->pcm_param.format == dev2->pcm_param.format);
                ASSERT_ERROR(dev1->pcm_param.fsc_delay == dev2->pcm_param.fsc_delay);
                ASSERT_ERROR(dev1->pcm_param.fsc_length == dev2->pcm_param.fsc_length);
                ASSERT_ERROR(dev1->pcm_param.inverted_clk_polarity == dev2->pcm_param.inverted_clk_polarity);
                ASSERT_ERROR(dev1->pcm_param.inverted_fsc_polarity == dev2->pcm_param.inverted_fsc_polarity);
                ASSERT_ERROR(dev1->pcm_param.mode == dev2->pcm_param.mode);
                ASSERT_ERROR(dev1->pcm_param.output_mode == dev2->pcm_param.output_mode);
                break;
        case AUDIO_PDM:
                ASSERT_ERROR(dev1->pdm_param.channel == dev2->pdm_param.channel);
                ASSERT_ERROR(dev1->pdm_param.clk_frequency == dev2->pdm_param.clk_frequency);
                ASSERT_ERROR(dev1->pdm_param.in_delay == dev2->pdm_param.in_delay);
                ASSERT_ERROR(dev1->pdm_param.mode == dev2->pdm_param.mode);
                ASSERT_ERROR(dev1->pdm_param.out_delay == dev2->pdm_param.out_delay);
                ASSERT_ERROR(dev1->pdm_param.swap_channel == dev2->pdm_param.swap_channel);
                break;
        default:
                ASSERT_ERROR(0);
                break;
        }
}

static void assert_same_cfg(sys_audio_device_t *dev, uint8_t path_idx)
{
        audio_path_t * path = sys_audio_path.audio_path;
        /* Find the cfg of the device in all data paths */
        for (uint8_t i = 0; i < MAX_NO_PATHS; i++) {
                if (path[i].dev_in->device_type == dev->device_type) {
                        check_cfgs(path[i].dev_in, dev);
                        break;
                }

                if (path[path_idx].dev_out->device_type == dev->device_type) {
                        check_cfgs(path[i].dev_out, dev);
                        break;
                }
        }
}

__STATIC_INLINE bool validate_path(sys_audio_device_t *dev_in,  sys_audio_device_t *dev_out)
{
        return (dev_in != NULL && dev_out != NULL &&
                dev_in->device_type != AUDIO_INVALID &&
                dev_out->device_type != AUDIO_INVALID);
}

/*
 * \brief Start audio device input or output - helper function
 *
 * \param [in] dev input or output device
 * \param [in] nof_paths number of used paths
 * \param [in] idx path id
 * \param [in] dir device direction as input or output
 *
 * \return
 *         \retval true in case of closing audio manager with success
 *         \retval false in case of closing audio manager failed
 *
 */
static bool start_device(sys_audio_device_t *dev, uint8_t nof_paths, uint8_t idx, SYS_AUDIO_MGR_DIRECTION dir)
{
        bool ret = true;

        switch (dev->device_type) {
        case AUDIO_PDM:
                if (dev->pdm_param.mode == MODE_MASTER) {
                        hw_pdm_enable();
                        ret = hw_pdm_get_status();
                }
                break;
        case AUDIO_PCM:
                hw_pcm_enable();
                ret = hw_pcm_is_enabled();
                break;
        case AUDIO_MEMORY:
                initialize_dma_reg(idx, nof_paths, &(dev->memory_param), dir);
                for (uint8_t i = 0; i < CHANNEL_NUM_MAX; i++) {
                        if (dev->memory_param.dma_channel[i] != HW_DMA_CHANNEL_INVALID) {
                                hw_dma_channel_enable(dev->memory_param.dma_channel[i],
                                        HW_DMA_STATE_ENABLED);
                                ret = hw_dma_is_channel_active(dev->memory_param.dma_channel[i]);

                                if (ret == false) {
                                        break;
                                }
                        }
                }
                break;
# if dg_configUSE_HW_SDADC
        case AUDIO_SDADC:
                hw_sdadc_start();
                break;
# endif /* dg_configUSE_HW_SDADC */
        default:
                ret = false;
                break;
        }

        return(ret);
}

static bool stop_device(sys_audio_device_t *dev)
{
        bool ret = true;

        switch (dev->device_type) {
        case AUDIO_PDM:
                hw_pdm_disable();

                if (dev->pdm_param.mode == MODE_MASTER) {
                        ret = !hw_pdm_get_status();
                }
                break;
        case AUDIO_PCM:
                hw_pcm_disable();
                ret = !hw_pcm_is_enabled();
                break;
        case AUDIO_MEMORY:
                for (uint8_t i = 0; i < CHANNEL_NUM_MAX; i++) {
                        if (dev->memory_param.dma_channel[i] != HW_DMA_CHANNEL_INVALID) {
                                hw_dma_channel_enable(dev->memory_param.dma_channel[i],
                                        HW_DMA_STATE_DISABLED);
                                ret = !hw_dma_is_channel_active(dev->memory_param.dma_channel[i]);

                                if (ret == false) {
                                        break;
                                }
                        }
                }
                break;
# if dg_configUSE_HW_SDADC
        case AUDIO_SDADC:
                hw_sdadc_stop();
                break;
# endif /* dg_configUSE_HW_SDADC */
        default:
                ret = false;
                break;
        }

        return ret;
}

__STATIC_INLINE void close_device(sys_audio_device_t *dev)
{
        if (dev->device_type == AUDIO_MEMORY) {
                dma_resource_mng(false, dev);
        }

        OS_FREE(dev);
}

static bool deep_copy_paths(sys_audio_device_t *udev_in, sys_audio_device_t *udev_out, uint8_t idx)
{
        sys_audio_device_t *dev = NULL;
        sys_audio_device_t *udev = NULL;

        audio_path_t* const paths = sys_audio_path.audio_path;

#  ifdef OS_PRESENT
        if (2 * sizeof(sys_audio_device_t) > OS_GET_FREE_HEAP_SIZE()) {
                paths[idx].dev_in = NULL;
                paths[idx].dev_out = NULL;
                return false;
        }
#  endif

        paths[idx].dev_in = OS_MALLOC(sizeof(sys_audio_device_t));

        if (!paths[idx].dev_in) {
                return false;
        }

        paths[idx].dev_out = OS_MALLOC(sizeof(sys_audio_device_t));

        if (!paths[idx].dev_out) {
                OS_FREE(paths[idx].dev_in);
                return false;
        }
        for (uint8_t i = 0; i < 2; i++) {

                if (i == 0) {
                        dev = paths[idx].dev_in;
                        udev = udev_in;
                } else {
                        dev = paths[idx].dev_out;
                        udev = udev_out;
                }

                *dev = *udev;
        }

        return true;
}

bool sys_audio_mgr_start(uint8_t idx)
{
        if (idx >= MAX_NO_PATHS) {
                return false;
        }

        audio_path_t* const path = &sys_audio_path.audio_path[idx];

        if (validate_path(path->dev_in, path->dev_out) == false) {
                return false;
        }

        if (!start_device(path->dev_in, nof_paths, idx, DIRECTION_INPUT)) {
                return false;
        }

        if (!start_device(path->dev_out, nof_paths, idx, DIRECTION_OUTPUT)) {
                stop_device(path->dev_in);
                return false;
        }

        if (src_config[idx].id != 0) {

                if (!hw_src_is_enabled(src_config[idx].id)) {
                        hw_src_enable(src_config[idx].id);
                }
        }
        return true;
}

bool sys_audio_mgr_stop(uint8_t idx)
{
        bool ret = false;

        if (idx >= MAX_NO_PATHS) {
                return (ret);
        }

        audio_path_t* const path = &sys_audio_path.audio_path[idx];

        if (validate_path(path->dev_in, path->dev_out) == false) {
                return (ret);
        }

        if (src_config[idx].id != 0) {

                if (hw_src_is_enabled(src_config[idx].id)) {
                        hw_src_disable(src_config[idx].id);
                }
        }

        ret = stop_device(path->dev_in);

        if (ret) {
                ret = stop_device(path->dev_out);
        }

        return(ret);
}

#ifdef OS_PRESENT
static void src_resource_mng_explicitly(bool acquire, uint8_t idx, uint8_t  src_id)
{
        if (acquire) {
                /* Acquire SRC */
                if (resource_acquire(RES_MASK(src_static_cfg[src_id].resource_id), 0) != 0) {
                        src_config[idx].id = src_static_cfg[src_id].hw_src_id;
                } else {
                        src_config[idx].id = 0;
                }
        } else {
                /* Release SRC */
                if (src_config[idx].id == src_static_cfg[src_id].hw_src_id) {
                        resource_release(RES_MASK(src_static_cfg[src_id].resource_id));
                }
        }

        if (acquire) {
                ASSERT_ERROR(src_config[idx].id != 0);
        }


}
#else /* NO OS */
#define src_resource_mng_explicitly(acquire, idx, src_id)
#endif /* OS_PRESENT */

static void validate_single_path(sys_audio_device_t *dev_in,  sys_audio_device_t *dev_out, uint8_t path_idx)
{
        /* Check current path's input and output devices */

        /* Check validity of input and output device of the current path */
        ASSERT_ERROR(validate_path(dev_in, dev_out));

        if (dev_in->device_type == dev_out->device_type) {
                /* Check current use of PDM */
                ASSERT_ERROR(dev_in->device_type != AUDIO_PDM);

                /* Check current use of PCM */
                if (dev_in->device_type == AUDIO_PCM) {
                        check_cfgs(dev_in, dev_out);
                }
        }

        /* Check previous use of devices */
        /*
         * According to Audio Unit Block Diagram each interface (PCM/PDM),
         * can be input for both SRCs but only one can be as output except from memory.
         * For that case, a table that matches
         * each device_type used in output for each data path implemented as mask.
         */

        /* PDM may only be used once, either as input or as an output device for each data path */

        if (dev_in->device_type == AUDIO_PDM) {
                ASSERT_ERROR(!single_dev_type_out[AUDIO_PDM]);
        }

        if (dev_out->device_type == AUDIO_PDM) {
                ASSERT_ERROR(!single_dev_type_in[AUDIO_PDM]);
        }

        /* Each  device type can be used multiple times as input devices
         * but with the same configuration (except from memory)
         */
        if (single_dev_type_in[dev_in->device_type] && dev_in->device_type != AUDIO_MEMORY) {
                /* Find the cfg of the device in all data paths */
                assert_same_cfg(dev_in, path_idx);
        }

        /* Each  device type (except from memory) should only be used once as output device */
        if (single_dev_type_out[dev_out->device_type] && dev_out->device_type != AUDIO_MEMORY) {
                /* Find the cfg of the device in all data paths */
                assert_same_cfg(dev_out, path_idx);
        }

        /* Check that the audio channels correspond to the number of memory channels */
        assert_audio_mem_channels_consistency(dev_in, dev_out);
}

static uint8_t path_index_acquire(void)
{
        uint8_t path_idx;

        for (path_idx = 0; path_idx < MAX_NO_PATHS; path_idx++) {
                if (audio_path_idx_status[path_idx] == false) {
                        audio_path_idx_status[path_idx] = true;
                        break;
                }
        }

        ASSERT_ERROR(path_idx < MAX_NO_PATHS);

        return path_idx;
}

static bool validate_use_of_src(sys_audio_device_t *dev_in,  sys_audio_device_t *dev_out, SYS_AUDIO_MGR_SRC_USE src)
{
        sys_audio_device_t* dev[2] = {dev_in, dev_out};

        /* Validation check of the use of SRC and interfaces */
        if (src == NO_SRC) {

                for (uint8_t i = 0; i < 2; i++) {

                        switch (dev[i]->device_type) {
# if dg_configUSE_HW_SDADC
                        case AUDIO_SDADC:
# endif
                                /* PDM as input/output and SDADC (if applicable) require SRC */
                        case AUDIO_PDM:
                                return false;
                        default:
                                return true;
                        }
                }
        }
        return true;
}

uint8_t sys_audio_mgr_open_path(sys_audio_device_t *dev_in,  sys_audio_device_t *dev_out, SYS_AUDIO_MGR_SRC_USE src)
{
        uint8_t path_idx = path_index_acquire();

        /* Input validation */
        validate_single_path(dev_in, dev_out, path_idx);

        /* Deep copy of system path cfg */
        ASSERT_ERROR(deep_copy_paths(dev_in, dev_out, path_idx));

        /* Init src configuration */
        memset(&src_config[path_idx], 0, sizeof(hw_src_config_t));

#ifdef OS_PRESENT
        /* Disable interrupt in case of PCM1 input to PCM1 output */
        if (dev_in->device_type == AUDIO_PCM && dev_out->device_type == AUDIO_PCM) {
                pcm_loopback = true;
                /* Set sleep mode to active */
                pm_mode = pm_mode_active;
        }

# if dg_configUSE_HW_SDADC
        if (dev_in->device_type == AUDIO_SDADC) {
                /* Set sleep mode to active */
                pm_mode = pm_mode_active;
        } else
# endif /* dg_configUSE_HW_SDADC */
        {
                /* Set sleep mode to idle */
                pm_mode = pm_mode_idle;
        }

        pm_sleep_mode_request(pm_mode);
#endif /* OS_PRESENT */

        /* Open audio power domain */
        hw_sys_pd_audio_enable();

        ASSERT_ERROR(validate_use_of_src(dev_in, dev_out, src));

        if (is_src_conversion_required(dev_in, dev_out, src)) {

                switch (src) {
                case SRC_1:
                        /* Acquire SRC1 and define src_config for one path*/
                        src_resource_mng_explicitly(true, path_idx, 0);
                        break;
                case SRC_2:
                        /* Acquire SRC2 and define src_config for one path*/
                        src_resource_mng_explicitly(true, path_idx, 1);
                        break;
                case SRC_AUTO:
                        /* Acquire SRC and define src_config for one path with an automated way*/
                        src_resource_mng_implicitly(true, path_idx);
                        break;
                default:
                        ASSERT_ERROR(0);
                }
                /* SRC configuration */
                initialize_src_reg(dev_in, dev_out, &src_config[path_idx]);
        } else {
                //Case of NO_SRC
                src_config[path_idx].id = 0;

# if dg_configUSE_HW_SDADC
                if (dev_in->device_type == AUDIO_SDADC && dev_out->device_type == AUDIO_MEMORY) {
                        /* In case of SDADC->DMA->MEMORY, only bit_depth = 16 is supported as SDADC filter delivers a 16bits word */
                        ASSERT_ERROR(dev_out->memory_param.bit_depth == 16);
                }
# endif /* dg_configUSE_HW_SDADC */
                /* Data path with input and output Audio Memory is supported only with the use of SRC */
                if (dev_in->device_type == dev_out->device_type) {
                        ASSERT_ERROR(dev_in->device_type != AUDIO_MEMORY);
                }
        }

        /* Select the output */
        select_output(dev_in, dev_out, &src_config[path_idx]);

        /* Input/Ouput interface configuration for each data path */
        /* Check if the device input (except memory) is already used in previous paths,
         * if not then initialize the device
         * */
        if (dev_in->device_type == AUDIO_MEMORY || !single_dev_type_in[dev_in->device_type]) {
                initialize_reg(path_idx, nof_paths, dev_in, DIRECTION_INPUT);
        }
        if (dev_out->device_type == AUDIO_MEMORY || !single_dev_type_out[dev_out->device_type]) {
                initialize_reg(path_idx, nof_paths, dev_out, DIRECTION_OUTPUT);
        }

        single_dev_type_in[dev_in->device_type] = true;
        single_dev_type_out[dev_out->device_type] = true;

        return path_idx;
}

static bool is_any_path_active(void)
{
        /* Check if there is any src active */
        for (uint8_t path_idx = 0; path_idx < MAX_NO_PATHS; path_idx++) {
                if (src_config[path_idx].id) {
                        return true;
                }
        }
        /* Check if there is any device active except from memory */
        for (SYS_AUDIO_MGR_DEVICE dev = AUDIO_PCM; dev < SIZE_OF_AUDIO; dev++) {
                if (dev != AUDIO_MEMORY && single_dev_type_in[dev]) {
                        return true;
                }
        }

        return false;
}

void sys_audio_mgr_close_path(uint8_t index)
{
        if (index >= MAX_NO_PATHS) {
                return;
        }

        audio_path_t* const path = &sys_audio_path.audio_path[index];

        if (validate_path(path->dev_in, path->dev_out) == false) {
                return;
        }

        /* Disable interrupt in case of PCM1 input to PCM1 output */
        if (pcm_loopback) {
                hw_pcm_unregister_interrupt();
                pcm_loopback = false;
        }

        close_device(path->dev_out);

        if (path->dev_out->device_type == AUDIO_PCM) {
                hw_pcm_set_pcm_input_mux(HW_PCM_INPUT_MUX_OFF);
        }

        if (src_config[index].id != 0) {
                src_resource_mng_implicitly(false, index);
        }

        close_device(path->dev_in);

        /* Remove used input device */
        single_dev_type_in[path->dev_in->device_type] = false;
        /* Remove used output device */
        single_dev_type_out[path->dev_out->device_type] = false;

        if (is_any_path_active() == false) {
#ifdef OS_PRESENT
                pm_sleep_mode_release(pm_mode);
                hw_sys_pd_audio_disable();
#endif /* OS_PRESENT */
                /* Initialize device types */
                memset(single_dev_type_out, 0, SIZE_OF_AUDIO);
                memset(single_dev_type_in, 0, SIZE_OF_AUDIO);
        }

        /* Remove used audio data path */
        audio_path_idx_status[index] = false;
}
# endif /* dg_configSYS_AUDIO_MGR */
