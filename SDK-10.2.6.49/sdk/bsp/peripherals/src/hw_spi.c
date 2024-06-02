/**
 ****************************************************************************************
 *
 * @file hw_spi.c
 *
 * @brief Implementation of the SPI Low Level Driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_SPI

#include <stdint.h>
#include "hw_spi.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

typedef enum {
        HW_SPI_STATUS_OVFL_ERR     = -2,   /**< SPI Status Slave Overflow Error */
        HW_SPI_STATUS_CFG_ERR      = -1,   /**< SPI Status Config Error */
        HW_SPI_STATUS_ERR_OK       = 0,    /**< SPI Status No Error */
} HW_SPI_STATUS;

typedef enum {
        HW_SPI_TRANSFER_READ      = 1,
        HW_SPI_TRANSFER_WRITE     = 2,
        HW_SPI_TRANSFER_READWRITE = 3,
} HW_SPI_TRANSFER;


        #define HW_SPI_WRITE_DUMMY_WORD         (0x00000000)

typedef struct
{
        SPI_Pad            cs_pad;
        hw_spi_tx_callback rx_cb;
        hw_spi_tx_callback tx_cb;
        void               *cb_data;

        const uint8_t      *tx_buffer;
        uint16_t           tx_len;
        uint16_t           tx_words_rem;

        uint8_t            *rx_buffer;
        uint16_t           rx_len;
        uint16_t           rx_words_rem;

        HW_SPI_TRANSFER    transfer_mode;
        HW_SPI_CS_MODE     cs_mode;
        HW_SPI_MODE        smn_role;

        HW_SPI_FIFO_TL     rx_tl;
        HW_SPI_FIFO_TL     tx_tl;
#if (HW_SPI_DMA_SUPPORT == 1)
        uint8_t            use_dma;
        DMA_setup          tx_dma;
        DMA_setup          rx_dma;
#endif /* HW_SPI_DMA_SUPPORT */
} SPI_Data;

/* Non-cached, non-retained global. */
static volatile uint32_t hw_spi_read_buf_dummy;

static uint32_t trash_buf;

#ifdef HW_SPI3
        /* SPI data are not retained. The user must ensure that they are updated after exiting sleep. */
        static SPI_Data spi_data[3];

        #define SPI_INT(id)  ((id) == HW_SPI1 ? (SPI_IRQn) : ((id) == HW_SPI2 ? (SPI2_IRQn) : (SPI3_IRQn)))
        #define SPIIX(id)    ((id) == HW_SPI1 ? 0 : ((id) == HW_SPI2 ? 1 : 2))
        #define SPIDATA(id)  (&spi_data[SPIIX(id)])
#endif

#ifndef HW_SPI3
#ifdef HW_SPI2
        /* SPI data are not retained. The user must ensure that they are updated after exiting sleep. */
        static SPI_Data spi_data[2];

        #define SPI_INT(id)  ((id) == HW_SPI1 ? (SPI_IRQn) : (SPI2_IRQn))
        #define SPIIX(id)    ((id) == HW_SPI1 ? 0 : 1)
        #define SPIDATA(id)  (&spi_data[SPIIX(id)])
#else
        /* SPI data are not retained. The user must ensure that they are updated after exiting sleep. */
        static SPI_Data spi_data;

        #define SPI_INT(id)  (SPI_IRQn)
        #define SPIIX(id)    (0)
        #define SPIDATA(id)  (&spi_data)
#endif
#endif

#if (HW_SPI_DMA_SUPPORT == 1)
#define HW_SPI_DEFAULT_DMA_RX_PRIO      (HW_DMA_PRIO_2)
#define HW_SPI_DEFAULT_DMA_TX_PRIO      (HW_DMA_PRIO_2)
#endif /* HW_SPI_DMA_SUPPORT */

//==================== Configuration functions =================================

void hw_spi_set_cs_pad(HW_SPI_ID id, const SPI_Pad *cs_pad)
{
        SPI_Data *spid = SPIDATA(id);

        spid->cs_pad.port = cs_pad->port;
        spid->cs_pad.pin = cs_pad->pin;
}

void hw_spi_init_clk_reg(const HW_SPI_ID id, bool select_divn)
{
        if (id == HW_SPI1) {
                ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, SNC_IS_UP) == 1);

                if (select_divn) {
                        CRG_SNC->RESET_CLK_SNC_REG = CRG_SNC_RESET_CLK_SNC_REG_SPI_CLK_SEL_Msk;
                } else {
                        CRG_SNC->SET_CLK_SNC_REG = CRG_SNC_SET_CLK_SNC_REG_SPI_CLK_SEL_Msk;
                }
                CRG_SNC->SET_CLK_SNC_REG = CRG_SNC_SET_CLK_SNC_REG_SPI_ENABLE_Msk;
        } else if (id == HW_SPI2) {
                ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, SNC_IS_UP) == 1);

                if (select_divn) {
                        CRG_SNC->RESET_CLK_SNC_REG = CRG_SNC_RESET_CLK_SNC_REG_SPI2_CLK_SEL_Msk;
                } else {
                        CRG_SNC->SET_CLK_SNC_REG = CRG_SNC_SET_CLK_SNC_REG_SPI2_CLK_SEL_Msk;
                }
                CRG_SNC->SET_CLK_SNC_REG = CRG_SNC_SET_CLK_SNC_REG_SPI2_ENABLE_Msk;
        } else if (id == HW_SPI3) {
                if (select_divn) {
                        CRG_SYS->RESET_CLK_SYS_REG = CRG_SYS_RESET_CLK_SYS_REG_SPI3_CLK_SEL_Msk;
                } else {
                        CRG_SYS->SET_CLK_SYS_REG = CRG_SYS_SET_CLK_SYS_REG_SPI3_CLK_SEL_Msk;
                }
                CRG_SYS->SET_CLK_SYS_REG = CRG_SYS_SET_CLK_SYS_REG_SPI3_ENABLE_Msk;
        } else {
                ASSERT_WARNING(0);
        }
}

void hw_spi_deinit_clk_reg(const HW_SPI_ID id)
{
        if (id == HW_SPI1) {
                ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, SNC_IS_UP) == 1);
                CRG_SNC->RESET_CLK_SNC_REG = CRG_SNC_RESET_CLK_SNC_REG_SPI_ENABLE_Msk;
        } else if (id == HW_SPI2) {
                ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, SNC_IS_UP) == 1);
                CRG_SNC->RESET_CLK_SNC_REG = CRG_SNC_RESET_CLK_SNC_REG_SPI2_ENABLE_Msk;
        } else if (id == HW_SPI3) {
                CRG_SYS->RESET_CLK_SYS_REG = CRG_SYS_RESET_CLK_SYS_REG_SPI3_ENABLE_Msk;
        } else {
                ASSERT_WARNING(0);
        }
}

void hw_spi_init(HW_SPI_ID id, const spi_config *cfg)
{
        SPI_Data *spid = SPIDATA(id);

        // Enable Clock for SPI
        hw_spi_init_clk_reg(id, cfg->select_divn);
        // Disable SPI / Reset FIFO in SPI Control Register
        hw_spi_set_ctrl_reg_fifo_reset(id, true);
        // Set SPI Word length
        hw_spi_set_config_reg_word_len(id, cfg->word_mode);
        // Set SPI Mode (CPOL, CPHA)
        hw_spi_set_config_reg_spi_mode(id, cfg->cpol_cpha_mode);
        // Set SPI Master/Slave mode
        hw_spi_set_config_reg_slave_en(id, cfg->smn_role);
        spid->smn_role = cfg->smn_role;
        // Set SPI RX FIFO threshold level
#if (HW_SPI_DMA_SUPPORT == 1)
        spid->use_dma = cfg->use_dma;
        if (spid->use_dma) {
                ASSERT_WARNING(cfg->rx_tl == HW_SPI_FIFO_LEVEL0);
        }
#endif
        ASSERT_WARNING(cfg->rx_tl < hw_spi_get_fifo_depth_in_bytes(id));
        spid->rx_tl = cfg->rx_tl;
        // Set SPI TX FIFO threshold level
        spid->tx_tl = cfg->tx_tl;

        // Clear Tx, Rx and DMA enable paths in Control Register
        hw_spi_set_ctrl_reg_clear_enable(id);
        // Enable Tx and/or Rx paths in Control Register
        hw_spi_set_fifo_mode(id, cfg->fifo_mode);
        // Set CS mode
        hw_spi_set_cs_config_reg_mode(id, HW_SPI_CS_NONE);
        spid->cs_mode = cfg->spi_cs;
        // Set swap bytes
        hw_spi_set_ctrl_reg_swap_bytes(id, cfg->swap_bytes);

        if (hw_spi_is_slave(id)) {
                hw_spi_set_ctrl_reg_capture_next_edge(id, HW_SPI_MASTER_EDGE_CAPTURE_CURRENT);
                ASSERT_WARNING(spid->cs_mode == HW_SPI_CS_0);
        } else {
                // SPI_CAPTURE_AT_NEXT_EDGE: always set
                hw_spi_set_ctrl_reg_capture_next_edge(id, HW_SPI_MASTER_EDGE_CAPTURE_NEXT);
                // Set SPI master clock speed
                hw_spi_set_clock_reg_clk_div(id, cfg->xtal_freq);
                // Set SPI CS pad
                spid->cs_pad.port = cfg->cs_pad.port;   // GPIO CS is set high at APP layer
                spid->cs_pad.pin = cfg->cs_pad.pin;
        }

        // enable SPI block (if needed)
        hw_spi_enable(id, cfg->disabled ? 0 : 1);
#if (HW_SPI_DMA_SUPPORT == 1)
        if (spid->use_dma) {
                hw_spi_configure_dma_channels(id, cfg->rx_dma_channel,  &cfg->dma_prio);
        }
#endif /* HW_SPI_DMA_SUPPORT */
        // Disable FIFO reset
        hw_spi_set_ctrl_reg_fifo_reset(id, false);
}

//=========================== CS handling function =============================

void hw_spi_set_cs_low(HW_SPI_ID id)
{
        SPI_Data *spid = SPIDATA(id);

        hw_spi_set_cs_config_reg_mode(id, spid->cs_mode);

        if (spid->cs_mode == HW_SPI_CS_GPIO) {
                hw_gpio_set_inactive(spid->cs_pad.port, spid->cs_pad.pin);    // push CS low
        }
}

void hw_spi_set_cs_high(HW_SPI_ID id)
{
        SPI_Data *spid = SPIDATA(id);

        hw_spi_set_cs_config_reg_mode(id, HW_SPI_CS_NONE);

        if (spid->cs_mode == HW_SPI_CS_GPIO) {
                hw_gpio_set_active(spid->cs_pad.port, spid->cs_pad.pin);    // push CS high
        }
}


//=========================== FIFO control functions ===========================

void hw_spi_set_fifo_mode(HW_SPI_ID id, HW_SPI_FIFO mode)
{
        switch (mode) {
        case HW_SPI_FIFO_RX_TX:
                hw_spi_set_ctrl_reg_rx_en(id, true);         // Enable RX path
                hw_spi_set_ctrl_reg_tx_en(id, true);         // Enable TX path
                break;
        case HW_SPI_FIFO_RX_ONLY:
                hw_spi_set_ctrl_reg_rx_en(id, true);         // Enable RX path
                hw_spi_set_ctrl_reg_tx_en(id, false);        // Disable TX path
                break;
        case HW_SPI_FIFO_TX_ONLY:
                hw_spi_set_ctrl_reg_rx_en(id, false);        // Disable RX path
                hw_spi_set_ctrl_reg_tx_en(id, true);         // Enable TX path
                break;
        case HW_SPI_FIFO_NONE:
                hw_spi_set_ctrl_reg_rx_en(id, false);        // Disable RX path
                hw_spi_set_ctrl_reg_tx_en(id, false);        // Disable TX path
                break;
        default:
                ASSERT_ERROR(0);
        }
}

HW_SPI_FIFO hw_spi_get_fifo_mode(HW_SPI_ID id)
{
        // Get the SPI FIFO mode from the secondary SPI control register
        uint8_t mode = hw_spi_get_ctrl_reg_tx_en(id)<<1 | hw_spi_get_ctrl_reg_rx_en(id);

        switch (mode) {
        case 1:
                return HW_SPI_FIFO_RX_ONLY;
        case 2:
                return HW_SPI_FIFO_TX_ONLY;
        case 3:
                return HW_SPI_FIFO_RX_TX;
        }
        return HW_SPI_FIFO_NONE;
}

HW_SPI_FIFO hw_spi_change_fifo_mode(HW_SPI_ID id, HW_SPI_FIFO mode)
{
        HW_SPI_FIFO old_mode = hw_spi_get_fifo_mode(id);

        if (old_mode != mode) {
                if ( !hw_spi_is_slave(id) &&
                     ((old_mode == HW_SPI_FIFO_RX_ONLY) || (old_mode == HW_SPI_FIFO_TX_ONLY)) ) {
                        hw_spi_wait_while_busy(id);
                }
                hw_spi_set_fifo_mode(id, mode);
        }

        return old_mode;
}

//=========================== DMA control functions ============================

#if (HW_SPI_DMA_SUPPORT == 1)

static void hw_spi_rx_dma_callback(void *user_data, dma_size_t len)
{
        SPI_Data *spid = user_data;
        hw_spi_tx_callback cb = spid->rx_cb;

        spid->rx_cb = NULL;
        spid->rx_words_rem = 0;
        if (cb) {
                cb(spid->cb_data, len * (spid->rx_dma.bus_width == HW_DMA_BW_BYTE ? 1 : spid->rx_dma.bus_width));
        }
}

static void hw_spi_tx_dma_callback(void *user_data, dma_size_t len)
{
        SPI_Data *spid = user_data;
        hw_spi_tx_callback cb = spid->tx_cb;

        spid->tx_cb = NULL;
        spid->tx_words_rem = 0;
        if (spid->smn_role == HW_SPI_MODE_SLAVE) {
                len++;
        }
        if (cb) {
                cb(spid->cb_data, len * (spid->tx_dma.bus_width == HW_DMA_BW_BYTE ? 1 : spid->tx_dma.bus_width));
        }
}

void hw_spi_configure_dma_channels(HW_SPI_ID id, int8_t channel, const hw_spi_dma_prio_t *prio)
{
        HW_DMA_PRIO rx_priority = HW_SPI_DEFAULT_DMA_RX_PRIO;
        HW_DMA_PRIO tx_priority = HW_SPI_DEFAULT_DMA_TX_PRIO;

        if (prio && prio->use_prio) {
                rx_priority = prio->rx_prio;
                tx_priority = prio->tx_prio;
        }

        SPI_Data *spid = SPIDATA(id);
        uint16_t wordsize = hw_spi_get_memory_word_size(id);

        bool channel_is_valid = (channel < 0 ||
                channel == HW_DMA_CHANNEL_0 ||
                channel == HW_DMA_CHANNEL_2 ||
                channel == HW_DMA_CHANNEL_4 ||
                channel == HW_DMA_CHANNEL_6);
        /* Make sure the channel is valid or -1 (no DMA) */
        ASSERT_ERROR(channel_is_valid);
        if (channel < 0 || (wordsize > 4 || wordsize == 3)) {
                spid->use_dma = 0;
                spid->rx_dma.channel_number = 0;
                spid->tx_dma.channel_number = 0;
        } else {
                spid->use_dma = 1;

                spid->rx_dma.channel_number = channel;
                spid->rx_dma.bus_width = (wordsize == 1 ? HW_DMA_BW_BYTE : (wordsize == 2 ? HW_DMA_BW_HALFWORD : HW_DMA_BW_WORD));
                spid->rx_dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
                spid->rx_dma.dma_req_mux = SPIIX(id) == 0 ? HW_DMA_TRIG_SPI_RXTX :
                                          (SPIIX(id) == 1 ? HW_DMA_TRIG_SPI2_RXTX : HW_DMA_TRIG_SPI3_RXTX);
                spid->rx_dma.irq_nr_of_trans = 0;
                spid->rx_dma.a_inc = HW_DMA_AINC_FALSE;
                spid->rx_dma.b_inc = HW_DMA_BINC_TRUE; // Change during transmission
                spid->rx_dma.circular = HW_DMA_MODE_NORMAL;
                spid->rx_dma.dma_prio = rx_priority;
                spid->rx_dma.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE; /* Not used by the HW in this case */
                spid->rx_dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
                spid->rx_dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
                spid->rx_dma.burst_mode = HW_DMA_BURST_MODE_DISABLED;

                spid->rx_dma.src_address = (uint32_t)&SBA(id)->SPI_FIFO_READ_REG;
                spid->rx_dma.dest_address = 0;  // Change during transmission
                spid->rx_dma.length = 0;        // Change during transmission
                spid->rx_dma.callback = hw_spi_rx_dma_callback;
                spid->rx_dma.user_data = spid;

                spid->tx_dma.channel_number = channel + 1;
                spid->tx_dma.bus_width = (wordsize == 1 ? HW_DMA_BW_BYTE : (wordsize == 2 ? HW_DMA_BW_HALFWORD : HW_DMA_BW_WORD));
                spid->tx_dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
                spid->tx_dma.dma_req_mux = SPIIX(id) == 0 ? HW_DMA_TRIG_SPI_RXTX :
                                          (SPIIX(id) == 1 ? HW_DMA_TRIG_SPI2_RXTX : HW_DMA_TRIG_SPI3_RXTX);
                spid->tx_dma.irq_nr_of_trans = 0;
                spid->tx_dma.a_inc = HW_DMA_AINC_TRUE;
                spid->tx_dma.b_inc = HW_DMA_BINC_FALSE;
                spid->tx_dma.circular = HW_DMA_MODE_NORMAL;
                spid->tx_dma.dma_prio = tx_priority;
                spid->tx_dma.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE; /* Not used by the HW in this case */
                spid->tx_dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
                spid->tx_dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
                spid->tx_dma.burst_mode = HW_DMA_BURST_MODE_DISABLED;
                spid->tx_dma.src_address = 0; // Change during transmission
                spid->tx_dma.dest_address = (uint32_t)&SBA(id)->SPI_FIFO_WRITE_REG;
                spid->tx_dma.length = 0;     // Change during transmission
                spid->tx_dma.callback = hw_spi_tx_dma_callback;
                spid->tx_dma.user_data = spid;
        }
}
#endif /* HW_SPI_DMA_SUPPORT */

//===================== Read/Write functions ===================================
uint16_t hw_spi_writeread(HW_SPI_ID id, uint16_t val)
{
        uint16_t v;
        while (hw_spi_is_tx_fifo_full(id) == true);
        hw_spi_fifo_write16(id, val);
        while (hw_spi_get_fifo_status_reg_rx_empty(id) == true);
        v = hw_spi_fifo_read16(id);
        return v;
}

uint32_t hw_spi_writeread32(HW_SPI_ID id, uint32_t val)
{
        uint32_t v;
        while (hw_spi_is_tx_fifo_full(id) == true);
        hw_spi_fifo_write32(id, val);
        while (hw_spi_get_fifo_status_reg_rx_empty(id) == true);
        v = hw_spi_fifo_read32(id);
        return v;
}

__STATIC_INLINE void hw_spi_read_word(HW_SPI_ID id, uint8_t *buf, uint32_t wordsize)
{
        switch (wordsize) {
        case 1:
                *buf = hw_spi_fifo_read8(id);
                break;
        case 2:
                *(uint16_t *) buf = hw_spi_fifo_read16(id);
                break;
        case 3:
                ASSERT_WARNING(wordsize == 3);
                break;
        case 4:
                *(uint32_t *) buf = hw_spi_fifo_read32(id);
                break;
        }
}


__STATIC_INLINE void hw_spi_write_word(HW_SPI_ID id, const uint8_t *buf, uint32_t wordsize)
{
        switch (wordsize) {
        case 1:
                hw_spi_fifo_write8(id, *buf);
                break;
        case 2:
                hw_spi_fifo_write16(id, *(uint16_t *) buf);
                break;
        case 3:
                ASSERT_WARNING(wordsize == 3);
                break;
        case 4:
                hw_spi_fifo_write32(id, *(uint32_t *) buf);
                break;
        }
}

static uint16_t hw_spi_transfer_write(HW_SPI_ID id)
{
        SPI_Data *spid = SPIDATA(id);
        uint32_t wordsize = hw_spi_get_memory_word_size(id);    // wordsize in bytes = 1,2 or 4
        const uint8_t *txbuf = spid->tx_buffer;
        uint16_t tx_words_rem;

        // Write output FIFO
        tx_words_rem = spid->tx_words_rem;
        if (spid->tx_cb == NULL) {
                while (tx_words_rem) {
                        while (hw_spi_is_tx_fifo_full(id) == true);
                        hw_spi_write_word(id, txbuf, wordsize);
                        txbuf += wordsize;
                        tx_words_rem--;
                }
                if (hw_spi_is_slave(id)) {
                        // Wait pending data in TX fifo
                        while (hw_spi_get_fifo_status_reg_tx_fifo_level(id) != 0);
                        // Wait until transaction is finished and SPI is not busy
                        while (hw_spi_get_fifo_status_reg_transaction_active(id) == true);
                }
        } else {
                /*
                 * TX_EMPTY IRQ triggers the Master SPI handler
                 */

                // Write TX_FIFO until full
                if (tx_words_rem) {
                        uint8_t avail_tx_fifo_lvl_words = (hw_spi_get_fifo_depth_in_bytes(id) - hw_spi_get_fifo_status_reg_tx_fifo_level(id)) / wordsize;
                        uint32_t wr_words = (tx_words_rem < avail_tx_fifo_lvl_words) ? tx_words_rem : avail_tx_fifo_lvl_words;

                        for (uint32_t i = 0; i < wr_words; i++) {
                                hw_spi_write_word(id, txbuf, wordsize);
                                txbuf += wordsize;
                        }
                        tx_words_rem -= wr_words;
                }
        }
        spid->tx_words_rem = tx_words_rem;
        spid->tx_buffer = txbuf;
        return tx_words_rem;
}

/* Non-cached, non-retained global. */
static uint16_t hw_spi_transfer_read(HW_SPI_ID id)
{
        SPI_Data *spid = SPIDATA(id);
        uint32_t wordsize = hw_spi_get_memory_word_size(id);

        uint16_t rx_words_rem = spid->rx_words_rem;
        uint16_t tx_words_rem = spid->tx_words_rem;
        uint8_t *rxbuf = spid->rx_buffer;

        if (hw_spi_get_fifo_status_reg_rx_fifo_overflow(id)) {
                ASSERT_WARNING(0);
                hw_spi_set_ctrl_reg_fifo_reset(id, true);
                spid->rx_words_rem = 0;
                spid->rx_len = 0;
                return 0;
        }

        if (spid->rx_cb == NULL) {
                if (hw_spi_is_slave(id)) {
                        while (rx_words_rem) {
                                do {
                                        if (hw_spi_get_fifo_status_reg_rx_fifo_overflow(id)) {
                                                hw_spi_set_ctrl_reg_fifo_reset(id, true);
                                                rx_words_rem = 0;
                                                break;
                                        }
                                } while (hw_spi_get_fifo_status_reg_rx_empty(id) == true);

                                hw_spi_read_word(id, rxbuf, wordsize);
                                rxbuf += wordsize;
                                rx_words_rem--;
                        }
                } else {
                        while (rx_words_rem) {
                                // Wait until TX-FIFO is not full, then dummy write
                                while (hw_spi_is_tx_fifo_full(id) == true);
                                hw_spi_set_fifo_write_reg(id, 0);

                                // Wait while RX FIFO is empty, then read
                                while (hw_spi_get_fifo_status_reg_rx_empty(id) == true);
                                hw_spi_read_word(id, rxbuf, wordsize);

                                rxbuf += wordsize;
                                rx_words_rem--;
                        }
                }
        } else {
                /*
                 * TX_EMPTY IRQ is required to trigger the Master SPI handler for the first time only
                 * RX_FULL IRQ will trigger the SPI handler for the first time and from now on for both Master and Slave
                 */

                // Read RX_FIFO until empty, if there are any bytes in the FIFO
                uint32_t rd_words = hw_spi_get_fifo_status_reg_rx_fifo_level(id) / wordsize;

                ASSERT_WARNING(rx_words_rem >= rd_words);

                for (uint32_t i = 0; i < rd_words; i++) {
                        hw_spi_read_word(id, rxbuf, wordsize);
                        rxbuf += wordsize;
                }
                rx_words_rem -= rd_words;

                if (hw_spi_is_slave(id)) {
                        // Update RX_TL, if required (depends on number of the last expected bytes)
                        if (rx_words_rem && (rx_words_rem < spid->rx_tl / wordsize)) {
                                hw_spi_set_fifo_config_reg_rx_tl(id, rx_words_rem * wordsize - 1);
                        }
                } else {
                        // Disable TX_EMPTY IRQ, wait for RX_FULL IRQ from now on
                        hw_spi_set_irq_mask_reg_tx_empty_en(id, HW_SPI_MINT_DISABLE);

                        /*
                         * The master writes DUMMY words in order to read new words.
                         * In order to avoid RX overflow at the master, a basic flow control mechanism is implemented.
                         * Specifically, the master writes to TX_FIFO only after reading the expected bytes from RX_FIFO,
                         * i.e. tx_words_rem == rx_words_rem.
                         */
                        if (tx_words_rem && (tx_words_rem == rx_words_rem) ) {
                                uint8_t avail_tx_fifo_lvl_words = (hw_spi_get_fifo_depth_in_bytes(id) - hw_spi_get_fifo_status_reg_tx_fifo_level(id)) / wordsize;
                                uint32_t wr_words = (tx_words_rem < avail_tx_fifo_lvl_words) ? tx_words_rem : avail_tx_fifo_lvl_words;

                                // Update RX_TL, if required (depends on number of the last expected bytes)
                                if (wr_words && (wr_words < spid->rx_tl / wordsize)) {
                                        hw_spi_set_fifo_config_reg_rx_tl(id, wr_words * wordsize - 1);
                                }

                                for (uint32_t i = 0; i < wr_words; i++) {
                                        hw_spi_set_fifo_write_reg(id, HW_SPI_WRITE_DUMMY_WORD);
                                }
                                tx_words_rem -= wr_words;
                        }
                }
        }
        spid->rx_words_rem = rx_words_rem;
        spid->tx_words_rem = tx_words_rem;
        spid->rx_buffer = rxbuf;
        return rx_words_rem;
}

static HW_SPI_STATUS hw_spi_txbuffer_force_write(HW_SPI_ID id, const uint8_t *out_buf, uint16_t wsz)
{
        switch (wsz) {
        case 1:
                hw_spi_set_txbuffer_force_reg(id, *out_buf);
                break;
        case 2:
                hw_spi_set_txbuffer_force_reg(id, *(uint16_t*)out_buf);
                break;
        case 4:
                hw_spi_set_txbuffer_force_reg(id, *(uint32_t*)out_buf);
                break;
        default:
                return HW_SPI_STATUS_CFG_ERR;
                break;
        }
        return HW_SPI_STATUS_ERR_OK;
}

static uint16_t hw_spi_transfer(HW_SPI_ID id)
{
        SPI_Data *spid = SPIDATA(id);

        uint32_t wordsize = hw_spi_get_memory_word_size(id);
        uint32_t txbuf_inc = spid->tx_buffer != NULL ? hw_spi_get_memory_word_size(id) : 0;
        uint32_t rxbuf_inc = spid->rx_buffer != NULL ? hw_spi_get_memory_word_size(id) : 0;

        uint8_t *rxbuf = spid->rx_buffer != NULL ? spid->rx_buffer : (uint8_t *)&trash_buf;
        const uint8_t *txbuf = spid->tx_buffer != NULL ? spid->tx_buffer : (uint8_t *)&trash_buf;

        uint16_t rx_words_rem = spid->rx_words_rem;
        uint16_t tx_words_rem = spid->tx_words_rem;

        if (hw_spi_get_fifo_status_reg_rx_fifo_overflow(id)) {
                ASSERT_WARNING(0);
                hw_spi_set_ctrl_reg_fifo_reset(id, true);
                spid->rx_words_rem = 0;
                spid->rx_len = 0;
                return 0;
        }

        if (spid->rx_cb == NULL) {
                if (hw_spi_is_slave(id)) {
                        // Write TX FIFO until it is full. Don't wait for SPI Master clock.
                        while (tx_words_rem && hw_spi_is_tx_fifo_full(id) == false) {
                                hw_spi_write_word(id, txbuf, wordsize);
                                txbuf += txbuf_inc;
                                tx_words_rem--;
                        }

                        // Write rest of the data to FIFO, when SPI Master starts reading
                        while (rx_words_rem) {
                                while (hw_spi_get_fifo_status_reg_rx_empty(id) == true);
                                // Read data
                                hw_spi_read_word(id, rxbuf, wordsize);
                                rxbuf += rxbuf_inc;
                                rx_words_rem--;

                                // If there are remaining tx data, write them in Tx FIFO, if there is space.
                                while (tx_words_rem && hw_spi_is_tx_fifo_full(id) == false) {
                                        hw_spi_write_word(id, txbuf, wordsize);
                                        txbuf += txbuf_inc;
                                        tx_words_rem--;
                                }
                        }
                } else {
                        while (rx_words_rem) {
                                // Wait until TX-FIFO is not full
                                while (hw_spi_is_tx_fifo_full(id) == true);

                                // Write
                                hw_spi_write_word(id, txbuf, wordsize);
                                txbuf += txbuf_inc;
                                tx_words_rem--;

                                // Wait while RX FIFO is empty
                                while (hw_spi_get_fifo_status_reg_rx_empty(id) == true);

                                hw_spi_read_word(id, rxbuf, wordsize);

                                rxbuf += rxbuf_inc;
                                rx_words_rem--;
                        }
                }
        } else {
                /*
                 * TX_EMPTY IRQ is required to trigger the SPI handler for the first time only
                 * RX_FULL IRQ will trigger the SPI handler from now on
                 */
                hw_spi_set_irq_mask_reg_tx_empty_en(id, HW_SPI_MINT_DISABLE);

                // Read RX_FIFO until empty, if there are any bytes in the FIFO
                uint32_t rd_words = hw_spi_get_fifo_status_reg_rx_fifo_level(id) / wordsize;

                ASSERT_WARNING(rx_words_rem >= rd_words);

                for (uint32_t i = 0; i < rd_words; i++) {
                        hw_spi_read_word(id, rxbuf, wordsize);
                        rxbuf += rxbuf_inc;
                }
                rx_words_rem -= rd_words;

                // Update RX_TL, if required (depends on number of the last expected bytes)
                if (rx_words_rem && (rx_words_rem < spid->rx_tl / wordsize)) {
                        hw_spi_set_fifo_config_reg_rx_tl(id, rx_words_rem * wordsize - 1);
                }

                // Write TX_FIFO until FULL
                if (tx_words_rem) {
                        uint8_t avail_tx_fifo_lvl_words = (hw_spi_get_fifo_depth_in_bytes(id) - hw_spi_get_fifo_status_reg_tx_fifo_level(id)) / wordsize;
                        uint32_t wr_words = (tx_words_rem < avail_tx_fifo_lvl_words) ? tx_words_rem : avail_tx_fifo_lvl_words;

                        for (uint32_t i = 0; i < wr_words; i++) {
                                hw_spi_write_word(id, txbuf, wordsize);
                                txbuf += txbuf_inc;
                        }
                        tx_words_rem -= wr_words;
                }
        }
        spid->rx_words_rem = rx_words_rem;
        spid->rx_buffer = rxbuf;
        spid->tx_words_rem = tx_words_rem;
        spid->tx_buffer = txbuf;
        return rx_words_rem;
}

static void hw_spi_write_first_word(HW_SPI_ID id)
{
        SPI_Data *spid = SPIDATA(id);

        if (hw_spi_is_slave(id)) {
                uint16_t wordsize = hw_spi_get_memory_word_size(id);

                if (spid->tx_buffer == NULL) {
                        hw_spi_txbuffer_force_write(id, (uint8_t *)&trash_buf, wordsize);
                } else {
                        hw_spi_txbuffer_force_write(id, spid->tx_buffer, wordsize);
                }

                spid->tx_buffer += wordsize;
                spid->tx_words_rem--;
        }
}

void hw_spi_writeread_buf(HW_SPI_ID id, const uint8_t *out_buf, uint8_t *in_buf, uint16_t len,
                                                             hw_spi_tx_callback cb, void *user_data)
{
        SPI_Data *spid = SPIDATA(id);
        uint16_t wordsize = hw_spi_get_memory_word_size(id);    // wordsize in bytes = 1,2 or 4

        /* Check alignment */
        ASSERT_WARNING( (in_buf != NULL) || (out_buf != NULL) );
        if (in_buf != NULL) {
                ASSERT_WARNING(((uintptr_t) in_buf) % wordsize == 0);
        }
        if (out_buf != NULL) {
                ASSERT_WARNING(((uintptr_t) out_buf) % wordsize == 0);
        }
        ASSERT_WARNING( (len > 0) && (len % wordsize == 0) );

        spid->rx_cb = cb;
        spid->cb_data = user_data;

        spid->tx_buffer = out_buf;
        spid->tx_len = len;
        spid->tx_words_rem = len / wordsize;
        spid->rx_buffer = in_buf;
        spid->rx_len = len;
        spid->rx_words_rem = len / wordsize;
        spid->transfer_mode = HW_SPI_TRANSFER_READWRITE;

        // Clear Tx, Rx and DMA enable paths in Control Register
        hw_spi_set_ctrl_reg_clear_enable(id);
        // Enable TX path
        hw_spi_set_fifo_config_reg_tx_tl(id, spid->tx_tl);
        hw_spi_set_ctrl_reg_tx_en(id, true);
        // Enable RX path
        ASSERT_WARNING(spid->rx_len > spid->rx_tl);
        hw_spi_set_fifo_config_reg_rx_tl(id, spid->rx_tl);
        hw_spi_set_ctrl_reg_rx_en(id, true);

        if (cb == NULL) {
#if (HW_SPI_DMA_SUPPORT == 1)
                ASSERT_WARNING(spid->use_dma == 0);
#endif /* HW_SPI_DMA_SUPPORT */
                // Enable SPI
                hw_spi_set_ctrl_reg_spi_en(id, true);
                // If slave write first word
                hw_spi_write_first_word(id);
                hw_spi_transfer(id);
#if (HW_SPI_DMA_SUPPORT == 1)
        } else if (spid->use_dma) {
                // Enable SPI
                hw_spi_set_ctrl_reg_spi_en(id, true);
                // If slave write first word
                hw_spi_write_first_word(id);

                spid->rx_dma.length = len / wordsize;
                if (in_buf != NULL) {
                        spid->rx_dma.dest_address = (uint32_t)in_buf;
                        spid->rx_dma.b_inc = HW_DMA_BINC_TRUE;
                }
                else {
                        spid->rx_dma.dest_address = (uint32_t)&trash_buf;
                        spid->rx_dma.b_inc = HW_DMA_BINC_FALSE;
                }

                if (spid->tx_words_rem > 0) {
                        spid->tx_dma.length = spid->tx_words_rem;
                        if (out_buf != NULL) {
                                spid->tx_dma.src_address = (uint32_t)spid->tx_buffer;
                                spid->tx_dma.a_inc = HW_DMA_AINC_TRUE;
                        } else {
                                spid->tx_dma.src_address = (uint32_t)&trash_buf;
                                spid->tx_dma.a_inc = HW_DMA_AINC_FALSE;
                        }
                        spid->tx_dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
                }

                // DMA requested
                hw_dma_channel_initialization(&spid->rx_dma);
                if (spid->tx_words_rem > 0) {
                        hw_dma_channel_initialization(&spid->tx_dma);
                }
                GLOBAL_INT_DISABLE();
                hw_dma_channel_enable(spid->rx_dma.channel_number, HW_DMA_STATE_ENABLED);
                if (spid->tx_words_rem > 0) {
                        hw_dma_channel_enable(spid->tx_dma.channel_number, HW_DMA_STATE_ENABLED);
                }
                GLOBAL_INT_RESTORE();

                // Enable SPI DMA Rx Path
                hw_spi_set_ctrl_reg_dma_rx_en(id, true);
                if (spid->tx_words_rem > 0) {
                        // Enable SPI DMA Tx Path
                        hw_spi_set_ctrl_reg_dma_tx_en(id, true);
                }
#endif /* HW_SPI_DMA_SUPPORT */
        } else {
                // Interrupt driven
                NVIC_DisableIRQ(SPI_INT(id));
                // Enable SPI
                hw_spi_set_ctrl_reg_spi_en(id, true);
                // If slave write first word
                hw_spi_write_first_word(id);
                hw_spi_enable_interrupt(id);
                NVIC_EnableIRQ(SPI_INT(id));
                // TX_EMPTY IRQ triggers SPI handler immediately because TX_FIFO_LVL=0 <= TX_TL
                // and hw_spi_transfer() is called
        }
}

void hw_spi_write_buf(HW_SPI_ID id, const uint8_t *out_buf, uint16_t len,
                                                             hw_spi_tx_callback cb, void *user_data)
{
        SPI_Data *spid = SPIDATA(id);
        uint16_t wordsize = hw_spi_get_memory_word_size(id);    // wordsize in bytes = 1,2 or 4

        /* Check alignment */
        ASSERT_WARNING( (out_buf != NULL) && (((uintptr_t) out_buf) % wordsize == 0) );
        ASSERT_WARNING( (len > 0) && (len % wordsize == 0) );

        spid->tx_cb = cb;
        spid->cb_data = user_data;

        spid->tx_buffer = out_buf;
        spid->tx_len = len;
        spid->tx_words_rem = len / wordsize;
        spid->rx_len = 0;
        spid->rx_words_rem = 0;

        spid->transfer_mode = HW_SPI_TRANSFER_WRITE;

        // Clear Tx, Rx and DMA enable paths in Control Register
        hw_spi_set_ctrl_reg_clear_enable(id);
        // Enable TX path
        hw_spi_set_fifo_config_reg_tx_tl(id, spid->tx_tl);
        hw_spi_set_ctrl_reg_tx_en(id, true);

        if (cb == NULL) {
#if (HW_SPI_DMA_SUPPORT == 1)
                ASSERT_WARNING(spid->use_dma == 0);
#endif /* HW_SPI_DMA_SUPPORT */
                // Enable SPI
                hw_spi_set_ctrl_reg_spi_en(id, true);
                // If slave write first word
                hw_spi_write_first_word(id);
                hw_spi_transfer_write(id);
#if (HW_SPI_DMA_SUPPORT == 1)
        } else if ( (spid->use_dma) && !((spid->tx_words_rem == 1) && hw_spi_is_slave(id)) ) {
                // Enable SPI
                hw_spi_set_ctrl_reg_spi_en(id, true);
                // If slave write first word
                hw_spi_write_first_word(id);

                spid->tx_dma.src_address = (uint32_t)spid->tx_buffer;
                spid->tx_dma.length = spid->tx_words_rem;
                spid->tx_dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
                spid->tx_dma.a_inc = HW_DMA_AINC_TRUE;
                // DMA requested
                hw_dma_channel_initialization(&spid->tx_dma);

                GLOBAL_INT_DISABLE();
                hw_dma_channel_enable(spid->tx_dma.channel_number, HW_DMA_STATE_ENABLED);
                GLOBAL_INT_RESTORE();
                // Enable SPI DMA Tx Path (SPI_TX_EN should be enabled first)
                hw_spi_set_ctrl_reg_dma_tx_en(id, true);
#endif /* HW_SPI_DMA_SUPPORT */
        } else {
                // Interrupt driven
                NVIC_DisableIRQ(SPI_INT(id));
                // Enable SPI
                hw_spi_set_ctrl_reg_spi_en(id, true);
                // If slave write first word
                hw_spi_write_first_word(id);
                hw_spi_enable_interrupt(id);
                NVIC_EnableIRQ(SPI_INT(id));
                // TX_EMPTY IRQ triggers SPI handler immediately because TX_FIFO_LVL=0 <= TX_TL
                // and hw_spi_transfer_write() is called
        }
}

void hw_spi_read_buf(HW_SPI_ID id, uint8_t *in_buf, uint16_t len,
                                                             hw_spi_tx_callback cb, void *user_data)
{
        hw_spi_read_buf_dummy = 0xFFFFFFFF;
        SPI_Data *spid = SPIDATA(id);
        uint16_t wordsize = hw_spi_get_memory_word_size(id);    // wordsize in bytes = 1,2 or 4

        /* Check alignment */
        ASSERT_WARNING(((uintptr_t) in_buf) % wordsize == 0);
        ASSERT_WARNING(len % wordsize == 0);

        spid->rx_cb = cb;
        spid->cb_data = user_data;

        spid->tx_len = len;
        spid->tx_words_rem = len / wordsize;
        spid->rx_buffer = in_buf;
        spid->rx_len = len;
        spid->rx_words_rem = len / wordsize;
        spid->transfer_mode = HW_SPI_TRANSFER_READ;

        // Clear Tx, Rx and DMA enable paths in Control Register
        hw_spi_set_ctrl_reg_clear_enable(id);

        if (!hw_spi_is_slave(id)) {
                // Enable TX path
                // The master has to write dummy data, thus giving a clock to slave and read data from slave
                // The master TX_EMPTY IRQ triggers the SPI handler for the first time only
                hw_spi_set_fifo_config_reg_tx_tl(id, spid->tx_tl);
                hw_spi_set_ctrl_reg_tx_en(id, true);
        }
        // Enable RX path
        // The slave RX_FULL IRQ triggers the SPI handler, so the RX_TL should be set correctly
        ASSERT_WARNING(spid->rx_len > spid->rx_tl);
        hw_spi_set_fifo_config_reg_rx_tl(id, spid->rx_tl);
        hw_spi_set_ctrl_reg_rx_en(id, true);

        if (cb == NULL) {
#if (HW_SPI_DMA_SUPPORT == 1)
                ASSERT_WARNING(spid->use_dma == 0);
#endif /* HW_SPI_DMA_SUPPORT */
                // Enable SPI
                hw_spi_set_ctrl_reg_spi_en(id, true);
                hw_spi_transfer_read(id);
#if (HW_SPI_DMA_SUPPORT == 1)
        } else if (spid->use_dma) {
                spid->rx_dma.dest_address = (uint32_t)in_buf;
                spid->rx_dma.length = len / wordsize;
                spid->rx_dma.b_inc = HW_DMA_BINC_TRUE;
                // DMA requested
                hw_dma_channel_initialization(&spid->rx_dma);
                if (!hw_spi_is_slave(id)) {
                        spid->tx_dma.src_address = (uint32_t) &hw_spi_read_buf_dummy;
                        spid->tx_dma.length = len / wordsize;
                        /*
                         * We don't use HW_DMA_INIT_AX_BX_BY because it will lock the bus until
                         * the DMA transaction is finished, which might cause bus starvation to
                         * other peripherals.
                         */
                        spid->tx_dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
                        spid->tx_dma.a_inc = HW_DMA_AINC_FALSE;
                        hw_dma_channel_initialization(&spid->tx_dma);
                        GLOBAL_INT_DISABLE();
                        hw_dma_channel_enable(spid->rx_dma.channel_number, HW_DMA_STATE_ENABLED);
                        hw_dma_channel_enable(spid->tx_dma.channel_number, HW_DMA_STATE_ENABLED);
                        GLOBAL_INT_RESTORE();

                        // Enable SPI DMA Rx Path
                        hw_spi_set_ctrl_reg_dma_rx_en(id, true);
                        // Enable SPI DMA Tx Path
                        hw_spi_set_ctrl_reg_dma_tx_en(id, true);
                        // Enable SPI
                        hw_spi_set_ctrl_reg_spi_en(id, true);
                } else {
                        hw_dma_channel_enable(spid->rx_dma.channel_number, HW_DMA_STATE_ENABLED);
                        // Enable SPI DMA Rx Path
                        hw_spi_set_ctrl_reg_dma_rx_en(id, true);
                        // Enable SPI
                        hw_spi_set_ctrl_reg_spi_en(id, true);
                }
#endif /* HW_SPI_DMA_SUPPORT */
        } else {
                // Interrupt driven
                NVIC_DisableIRQ(SPI_INT(id));
                // Enable SPI
                hw_spi_set_ctrl_reg_spi_en(id, true);
                hw_spi_enable_interrupt(id);
                NVIC_EnableIRQ(SPI_INT(id));
                // Master: TX_EMPTY IRQ triggers SPI handler immediately because TX_FIFO_LVL=0 <= TX_TL
                //         and hw_spi_transfer_read() is called
                // Slave: RX_FULL IRQ triggers the SPI handler when RX_FIFO_LVL >= RX_TL+1
                //         and hw_spi_transfer_read() is called
        }
}

void hw_spi_deinit(HW_SPI_ID id)
{
        SPI_Data *spid = SPIDATA(id);

        GLOBAL_INT_DISABLE();

        hw_spi_disable_interrupt(id);
        // Disable SPI / Reset FIFO in SPI Control Register
        hw_spi_set_ctrl_reg_fifo_reset(id, true);
        // Disable TX path
        hw_spi_set_ctrl_reg_tx_en(id, false);
        // Disable RX path
        hw_spi_set_ctrl_reg_rx_en(id, false);
        hw_spi_enable(id, 0);

        NVIC_DisableIRQ(SPI_INT(id));
        NVIC_ClearPendingIRQ(SPI_INT(id));

        // Disable Clock for SPI
        hw_spi_deinit_clk_reg(id);
#if (HW_SPI_DMA_SUPPORT == 1)
        if (spid->use_dma) {
                hw_dma_channel_stop(spid->rx_dma.channel_number);
                hw_dma_channel_stop(spid->tx_dma.channel_number);
        }
#endif /* HW_SPI_DMA_SUPPORT */
        spid->tx_cb = NULL;
        spid->rx_cb = NULL;

        GLOBAL_INT_RESTORE();
}

bool hw_spi_is_occupied(const HW_SPI_ID id)
{
        SPI_Data *spid = SPIDATA(id);

        /* check if the all the data are written to SPI FIFO */
        if ((spid->rx_cb != NULL) || (spid->tx_cb != NULL)) {
                return true;
        }
        return false;
}

//=========================== Interrupt handling ===============================
static void SPI_Interrupt_Handler(HW_SPI_ID id)
{
        SPI_Data *spid = SPIDATA(id);
        uint16_t *words_rem;
        uint16_t *len;
        hw_spi_tx_callback *pcb;

        switch (spid->transfer_mode) {
        case HW_SPI_TRANSFER_READ:
                hw_spi_transfer_read(id);
                words_rem = &spid->rx_words_rem;
                len = &spid->rx_len;
                pcb = &spid->rx_cb;
                break;
        case HW_SPI_TRANSFER_WRITE:
                hw_spi_transfer_write(id);
                words_rem = &spid->tx_words_rem;
                len = &spid->tx_len;
                pcb = &spid->tx_cb;
                break;
        default:
                hw_spi_transfer(id);
                words_rem = &spid->rx_words_rem;
                len = &spid->rx_len;
                pcb = &spid->rx_cb;
                break;
        }
        // Fire callback when done
        if (!(*words_rem)) {
                hw_spi_tx_callback cb = *pcb;
                *pcb = NULL;
                hw_spi_disable_interrupt(id);
                if (cb) {
                        cb(spid->cb_data, *len);
                }
        }
}

/**
 * \brief SPI1 Interrupt Handler
 *
 */
void SPI_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        SPI_Interrupt_Handler(HW_SPI1);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#ifdef HW_SPI2
/**
 * \brief SPI2 Interrupt Handler
 *
 */
void SPI2_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        SPI_Interrupt_Handler(HW_SPI2);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
#endif

#ifdef HW_SPI3
/**
 * \brief SPI3 Interrupt Handler
 *
 */
void SPI3_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        SPI_Interrupt_Handler(HW_SPI3);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
#endif

#endif /* dg_configUSE_HW_SPI */


