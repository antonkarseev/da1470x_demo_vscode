/**
 ****************************************************************************************
 *
 * @file hw_i3c.c
 *
 * @brief Implementation of the I3C Low Level Driver.
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_I3C

#include <stdint.h>
#include <string.h>
#include "hw_i3c.h"
#include "hw_pd.h"
#if (HW_I3C_DMA_SUPPORT == 1)
#include "hw_dma.h"
#endif

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

/**
 * \brief I3C transfer type
 *
 */
typedef enum {
        I3C_TRANSFER_READ,                   /**< I3C read transfer operation      */
        I3C_TRANSFER_WRITE,                  /**< I3C write transfer operation     */
        I3C_TRANSFER_SDA_WRITE,              /**< I3C SDA write transfer operation */
} I3C_TRANSFER;

/**
 * \brief Short Data Argument max size
 *
 */
#define SDA_MAX_SIZE                                   (3)

/**
 * \brief Return value with one bit set.
 *
 * \param[in] x Position of the bit to set
 *
 */
#define  BIT(x)                  (1UL << (x))

// Command compose macros
#define COMMAND_PORT_CMD(x)                     (((x) << 7) & GENMASK(14, 7))
#define COMMAND_PORT_DEV_COUNT(x)               (((x) << 21) & GENMASK(25, 21))
#define COMMAND_PORT_DEV_INDEX(x)               (((x) << 16) & GENMASK(20, 16))
#define COMMAND_PORT_SPEED(x)                   (((x) << 21) & GENMASK(23, 21))
#define COMMAND_PORT_TID(x)                     (((x) << 3) & GENMASK(6, 3))
#define COMMAND_PORT_ARG_DATA_LEN(x)            (((x) << 16) & GENMASK(31, 16))
#define COMMAND_PORT_SDA_DATA_BYTE_3(x)         (((x) & GENMASK(7, 0)) << 24)
#define COMMAND_PORT_SDA_DATA_BYTE_2(x)         (((x) & GENMASK(7, 0)) << 16)
#define COMMAND_PORT_SDA_DATA_BYTE_1(x)         (((x) & GENMASK(7, 0)) << 8)

/**
 * \brief Command Attribute definitions of the Command Type
 *
 */
enum {
        I3C_COMMAND_ATTR_CMD,                        /**< I3C Transfer Command           */
        I3C_COMMAND_ATTR_ARG,                        /**< I3C Transfer Argument          */
        I3C_COMMAND_ATTR_SDAP,                       /**< I3C Short Data Argument        */
        I3C_COMMAND_ATTR_ADDR_ASSGN_CMD,             /**< I3C Address Assignment Command */
};

/**
 * \brief Byte Strobe for valid data bytes of Short Data Argument
 *
 */
enum {
        I3C_COMMAND_PORT_SDA_BYTE_STRB_1 = BIT(3),  /**< Data Byte 1 Valid Qualifier    */
        I3C_COMMAND_PORT_SDA_BYTE_STRB_2 = BIT(4),  /**< Data Byte 2 Valid Qualifier    */
        I3C_COMMAND_PORT_SDA_BYTE_STRB_3 = BIT(5),  /**< Data Byte 3 Valid Qualifier    */
};

/**
 * \brief Transfer Command parameters
 *
 */
enum {
        I3C_COMMAND_PORT_TOC           = BIT(30),   /**< Termination On Completion field   */
        I3C_COMMAND_PORT_READ_TRANSFER = BIT(28),   /**< Read and Write  field             */
        I3C_COMMAND_PORT_SDAP          = BIT(27),   /**< Short Data Argument Present field */
        I3C_COMMAND_PORT_ROC           = BIT(26),   /**< Response On Completion field      */
        I3C_COMMAND_PORT_CP            = BIT(15),   /**< Command Present field             */
};

// Device address table macros
#define DEV_ADDR_TABLE_LEGACY_I2C_DEVICE        BIT(31)
#define DEV_ADDR_TABLE_DEV_DYNAMIC_ADDR(x)      (((x) << 16) & GENMASK(23, 16))
#define DEV_ADDR_TABLE_DEV_STATIC_ADDR(x)       ((x) & GENMASK(6, 0))

/**
 * In Band Interrupts macros
 *
 *      31     30.....16   15...9        8        7......0
 * +---------+-----------+----------------------+----------+
 * | IBI_STS | Reserved  | IBI_ID | IBI_RNW_BIT | Reserved |
 * +---------+-----------+----------------------+----------+
 *
 * Bit [7-0]   : Reserved
 * Bit [8]     : IBI_RNW_BIT
 * Bit [15-9]  : IBI_ID
 * Bit [30-16] : Reserved
 * Bit [31]    : IBI_STS
 */
#define IBI_PORT_RNW_BIT(x)                     (((x) & BIT(8)) >> 8)
#define IBI_PORT_ID(x)                          (((x) & GENMASK(15, 9)) >> 9)
#define IBI_PORT_STS(x)                         (((x) & BIT(31)) >> 31)

#if (HW_I3C_DMA_SUPPORT == 1)
#define HW_I3C_DEFAULT_DMA_RX_PRIO      (HW_DMA_PRIO_2)
#define HW_I3C_DEFAULT_DMA_TX_PRIO      (HW_DMA_PRIO_2)
#endif

typedef struct
{
        const uint8_t      *tx_buffer;
        void               *tx_user_data;
        uint16_t           tx_len;
        uint16_t           tx_num;

        uint8_t            *rx_buffer;
        void               *rx_user_data;
        uint16_t           rx_len;
        uint16_t           rx_num;

#if (HW_I3C_DMA_SUPPORT == 1)
        uint8_t            use_dma;
        DMA_setup          tx_dma;
        DMA_setup          rx_dma;
#endif /* HW_I3C_DMA_SUPPORT */

        hw_i3c_xfer_callback xfer_cb;
        I3C_TRANSFER       transfer_mode;

        hw_i3c_ibi_sir_hj_config ibi_sir_hj_config;
        hw_i3c_interrupt_callback   intr_cb;
        i3c_private_transfer_config transfer_cfg;
} i3c_data;

// I3C data are not retained. The user must ensure that they are updated after exiting sleep.
static i3c_data i3c_env;

// Handler used for non blocking read/write operation
static void hw_i3c_intr_handler(uint32_t mask);

// Reply for transfer completion
static void hw_i3c_xfer_reply(bool success);

#if (HW_I3C_DMA_SUPPORT == 1)
// Callback for DMA transfer completion
static void hw_i3c_xfer_dma_callback(void *user_data, dma_size_t len);
// Set I3C DMA channels
static void hw_i3c_set_dma_channels(HW_I3C_DMA_CHANNEL_PAIR dma_channel_pair, const hw_i3c_dma_prio_t *prio);
#endif

//==================== Configuration functions =================================

void hw_i3c_init_clk_reg(bool select_divn)
{
        ASSERT_WARNING(hw_pd_check_snc_status());

        if (select_divn) {
                REG_SET_BIT(CRG_SNC, RESET_CLK_SNC_REG, I3C_CLK_SEL);
        } else {
                REG_SET_BIT(CRG_SNC, SET_CLK_SNC_REG, I3C_CLK_SEL);
        }
        REG_SET_BIT(CRG_SNC, SET_CLK_SNC_REG, I3C_ENABLE);
}

void hw_i3c_deinit_clk_reg(void)
{
        ASSERT_WARNING(hw_pd_check_snc_status());
        REG_SET_BIT(CRG_SNC, RESET_CLK_SNC_REG, I3C_ENABLE);
}

bool hw_i3c_is_clk_enabled(void)
{
        return REG_GETF(CRG_SNC, CLK_SNC_REG, I3C_ENABLE);
}

/**
 * \brief Configure I3C controller SCL timing parameters
 *
 * \param [in] cfg configuration
 *
 */
static void hw_i3c_scl_timing_config(const i3c_scl_config *cfg)
{
        if (!cfg) {
                return;
        }

        // SCL I2C Fast Mode Timing Register
        REG_SETF(I3C, I3C_SCL_I2C_FM_TIMING_REG, I2C_FM_HCNT, cfg->i2c_fm_hcnt);
        REG_SETF(I3C, I3C_SCL_I2C_FM_TIMING_REG, I2C_FM_LCNT, cfg->i2c_fm_lcnt);

        // SCL I2C Fast Mode Plus Timing Register
        REG_SETF(I3C, I3C_SCL_I2C_FMP_TIMING_REG, I2C_FMP_HCNT, cfg->i2c_fm_plus_hcnt);
        REG_SETF(I3C, I3C_SCL_I2C_FMP_TIMING_REG , I2C_FMP_LCNT, cfg->i2c_fm_plus_lcnt);

        // SCL I3C Push Pull Timing Register
        REG_SETF(I3C, I3C_SCL_I3C_PP_TIMING_REG, I3C_PP_HCNT, cfg->i3c_pp_hcnt);
        REG_SETF(I3C, I3C_SCL_I3C_PP_TIMING_REG, I3C_PP_LCNT, cfg->i3c_pp_lcnt);

        // SCL I3C Open Drain Timing Register
        REG_SETF(I3C, I3C_SCL_I3C_OD_TIMING_REG, I3C_OD_HCNT, cfg->i3c_od_hcnt);
        REG_SETF(I3C, I3C_SCL_I3C_OD_TIMING_REG, I3C_OD_LCNT, cfg->i3c_od_lcnt);

        // SCL Extended Low Count Timing Register
        REG_SETF(I3C, I3C_SCL_EXT_LCNT_TIMING_REG, I3C_EXT_LCNT_1, cfg->i3c_sdr1_ext_lcnt);
        REG_SETF(I3C, I3C_SCL_EXT_LCNT_TIMING_REG, I3C_EXT_LCNT_2, cfg->i3c_sdr2_ext_lcnt);
        REG_SETF(I3C, I3C_SCL_EXT_LCNT_TIMING_REG, I3C_EXT_LCNT_3, cfg->i3c_sdr3_ext_lcnt);
        REG_SETF(I3C, I3C_SCL_EXT_LCNT_TIMING_REG, I3C_EXT_LCNT_4, cfg->i3c_sdr4_ext_lcnt);
}

/**
 * \brief Configure I3C Device Address Table(DAT) for slave devices
 *
 * \param [in] cfg configuration
 *
 */
static void hw_i3c_dat_config(const i3c_dat_config *cfg)
{
        if (!cfg) {
                return;
        }

        for (uint8_t i = 0; i < HW_I3C_SLAVE_DEV_MAX; i++) {
                hw_i3c_set_slave_device_address(cfg[i].static_address, cfg[i].dynamic_address, cfg[i].slave_type, i);
        }
}

/**
 * \brief Configure I3C IBI environment
 *
 * \param [in] cfg configuration
 *
 */
static void hw_i3c_ibi_env_config(const hw_i3c_ibi_sir_hj_config *cfg)
{
        if ((!cfg) || (!cfg->ibi_sir_hj_cb)) {
                return;
        }

        i3c_env.ibi_sir_hj_config.ibi_sir_hj_cb = cfg->ibi_sir_hj_cb;
}

/**
 * \brief Enable requested interrupt events and register interrupt callback
 *
 * \param [in] irq_sources requested interrupt sources
 * \param [in] cb callback function
 *
 */
static void hw_i3c_enable_irq_sources_and_register_cb(uint32_t irq_sources, hw_i3c_interrupt_callback cb)
{
        NVIC_DisableIRQ(I3C_IRQn);
        NVIC_ClearPendingIRQ(I3C_IRQn);

        // Enable required events
        I3C->I3C_INTR_STATUS_EN_REG = irq_sources;

        // Unmask required events
        I3C->I3C_INTR_SIGNAL_EN_REG = irq_sources;

        // Register interrupt callback
        hw_i3c_register_interrupt_callback(cb);

        NVIC_EnableIRQ(I3C_IRQn);
}

/**
 * \brief Enable requested interrupt events
 *
 * \param [in] irq_sources requested interrupt sources
 *
 */
static void hw_i3c_enable_irq_sources(uint32_t irq_sources)
{
        NVIC_DisableIRQ(I3C_IRQn);

        I3C->I3C_INTR_STATUS_EN_REG = irq_sources;

        I3C->I3C_INTR_SIGNAL_EN_REG = irq_sources;

        NVIC_EnableIRQ(I3C_IRQn);
}

HW_I3C_ERROR hw_i3c_init(const i3c_config *cfg)
{
        if (!cfg) {
                return HW_I3C_ERROR_INVALID_PARAMETER;
        }

        // Enable Clock for I3C
        hw_i3c_init_clk_reg(cfg->select_divn);

        // Reset I3C controller
        hw_i3c_software_reset();

        // I3C Hot-Join control
        hw_i3c_set_hot_join_accept(cfg->hot_join_accept);

        // Include I3C Broadcast Address (0x7E) for private transfers
        hw_i3c_set_include_bcast_addr(cfg->iba);

        // Configure SCL timings for I3C and I2C mode
        hw_i3c_scl_timing_config(&cfg->i3c_scl_cfg);

        // Configure Device Address Table
        hw_i3c_dat_config(&cfg->i3c_dat_cfg[0]);

#if (HW_I3C_DMA_SUPPORT == 1)
        // Enable the DMA Handshaking
        i3c_env.use_dma = cfg->use_dma;
        if (i3c_env.use_dma) {
                // Configure I3C DMA channels
                hw_i3c_set_dma_channels(cfg->dma_channel_pair, &cfg->dma_prio);

                // Enable the DMA Handshaking
                hw_i3c_set_dma_enable(true);
        }
#endif

        hw_i3c_ibi_env_config(&cfg->i3c_ibi_sir_hj_cfg);

        // Set IBI Status Queue threshold
        hw_i3c_set_ibi_status_queue_threshold(HW_I3C_IBI_STATUS_QUEUE_TL_1);

        // Enable IBI SIR Rejection(NACK and send directed auto disable CCC) for all I3C slave devices
        I3C->I3C_IBI_SIR_REQ_REJECT_REG = 0xFFFFFFFF;

        hw_i3c_enable_irq_sources_and_register_cb(HW_I3C_INT_IBI_THLD_STS, hw_i3c_intr_handler);

        // Enable I3C controller
        hw_i3c_enable_controller();

        return HW_I3C_ERROR_NONE;
}

void hw_i3c_deinit(void)
{
        // Disable interrupts
        I3C->I3C_INTR_SIGNAL_EN_REG = 0;
        I3C->I3C_INTR_STATUS_EN_REG = 0;

        hw_i3c_software_reset();

        NVIC_DisableIRQ(I3C_IRQn);
        NVIC_ClearPendingIRQ(I3C_IRQn);

        // Disable Clock for I3C
        hw_i3c_deinit_clk_reg();
}

#if (HW_I3C_DMA_SUPPORT == 1)
//=========================== DMA control functions ============================

static void hw_i3c_xfer_dma_callback(void *user_data, dma_size_t len)
{
        // Fire user callback immediately if response on completion is not required or RESTART condition
        // or response already received
        if ((i3c_env.transfer_cfg.response_on_completion == false) ||
                (i3c_env.transfer_cfg.termination_on_completion == HW_I3C_TRANSFER_TOC_RESTART) ||
                (i3c_env.transfer_cfg.cmd_response.valid))       {
                hw_i3c_xfer_reply(true);
        }
}

/**
 * \brief Set up both DMA channels for I3C
 *
 * \param [in] dma_channel_pair DMA channel-pair
 * \param [in] prio DMA priority per channel
 *
 */
static void hw_i3c_set_dma_channels(HW_I3C_DMA_CHANNEL_PAIR dma_channel_pair, const hw_i3c_dma_prio_t *prio)
{
        HW_DMA_PRIO rx_priority = HW_I3C_DEFAULT_DMA_RX_PRIO;
        HW_DMA_PRIO tx_priority = HW_I3C_DEFAULT_DMA_TX_PRIO;

        if (prio && prio->use_prio) {
                rx_priority = prio->rx_prio;
                tx_priority = prio->tx_prio;
        }

        // Configure RX DMA Channel for I3C
        i3c_env.rx_dma.channel_number = dma_channel_pair;
        i3c_env.rx_dma.bus_width = HW_DMA_BW_WORD;
        i3c_env.rx_dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
        i3c_env.rx_dma.dma_req_mux = HW_DMA_TRIG_I3C_RXTX;
        i3c_env.rx_dma.irq_nr_of_trans = 0;
        i3c_env.rx_dma.a_inc = HW_DMA_AINC_FALSE;
        i3c_env.rx_dma.b_inc = HW_DMA_BINC_TRUE;
        i3c_env.rx_dma.circular = HW_DMA_MODE_NORMAL;
        i3c_env.rx_dma.dma_prio = rx_priority;
        i3c_env.rx_dma.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE;
        i3c_env.rx_dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
        i3c_env.rx_dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
        i3c_env.rx_dma.src_address = (uint32_t)&I3C->I3C_RX_TX_DATA_PORT_REG;
        i3c_env.rx_dma.dest_address = 0;
        i3c_env.rx_dma.length = 0;
        i3c_env.rx_dma.callback = hw_i3c_xfer_dma_callback;
        i3c_env.rx_dma.user_data = i3c_env.rx_user_data;

        // Configure TX DMA Channel for I3C
        i3c_env.tx_dma.channel_number = dma_channel_pair + 1;
        i3c_env.tx_dma.bus_width = HW_DMA_BW_WORD;
        i3c_env.tx_dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
        i3c_env.tx_dma.dma_req_mux = HW_DMA_TRIG_I3C_RXTX;
        i3c_env.tx_dma.irq_nr_of_trans = 0;
        i3c_env.tx_dma.a_inc = HW_DMA_AINC_TRUE;
        i3c_env.tx_dma.b_inc = HW_DMA_BINC_FALSE;
        i3c_env.tx_dma.circular = HW_DMA_MODE_NORMAL;
        i3c_env.tx_dma.dma_prio = tx_priority;
        i3c_env.tx_dma.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE;
        i3c_env.tx_dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
        i3c_env.tx_dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
        i3c_env.tx_dma.src_address = 0;
        i3c_env.tx_dma.dest_address = (uint32_t)&I3C->I3C_RX_TX_DATA_PORT_REG;
        i3c_env.tx_dma.length = 0;
        i3c_env.tx_dma.callback = hw_i3c_xfer_dma_callback;
        i3c_env.tx_dma.user_data = i3c_env.tx_user_data;
}
#endif /* HW_I3C_DMA_SUPPORT */

/**
 * @brief Add odd parity for the 7-bit Dynamic addresses
 *
 * \param [in] dynamic_address 7-bit Dynamic addresses
 *
 * \return The Dynamic addresses with odd parity in the MSB
 *
 * \details
 *             bit |    7   |  6 ................. 0  |
 *             def | Parity |  7-bit Dynamic address  |
 *
 */
static uint8_t hw_i3c_add_parity(uint8_t dynamic_address)
{
        uint8_t p_bit;

        p_bit = (dynamic_address & 0x0F) ^ (dynamic_address >> 4);
        p_bit = (p_bit & 0x03) ^ (p_bit >> 2);
        p_bit = (p_bit & 0x01) ^ (p_bit >> 1);

        if (p_bit & 1) {
                return dynamic_address;
        } else {
                return dynamic_address | (0x01 << 7);
        }
}

void hw_i3c_set_slave_device_address(uint8_t static_address, uint8_t dynamic_address, HW_I3C_SLAVE_DEVICE slave_type, HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION slave_dev_loc)
{
        volatile uint32_t *i3c_dev_addr_table_loc_x_reg;
        uint32_t dev_addr_table_val;

        // Look up I3C_DEV_ADDR_TABLE_LOCX_REG address
        i3c_dev_addr_table_loc_x_reg = REG_GET_ADDR_INDEXED(I3C, I3C_DEV_ADDR_TABLE_LOC1_REG, 4, slave_dev_loc);
        dev_addr_table_val = *i3c_dev_addr_table_loc_x_reg;

        // Set static address and dynamic address
        dev_addr_table_val &= ~(I3C_I3C_DEV_ADDR_TABLE_LOC1_REG_DEV_STATIC_ADDR_Msk | I3C_I3C_DEV_ADDR_TABLE_LOC1_REG_DEV_DYNAMIC_ADDR_Msk);
        dev_addr_table_val |= I3C_I3C_DEV_ADDR_TABLE_LOC1_REG_DEV_STATIC_ADDR_Msk & DEV_ADDR_TABLE_DEV_STATIC_ADDR(static_address);
        dev_addr_table_val |= I3C_I3C_DEV_ADDR_TABLE_LOC1_REG_DEV_DYNAMIC_ADDR_Msk & DEV_ADDR_TABLE_DEV_DYNAMIC_ADDR(hw_i3c_add_parity(dynamic_address));

        // Set slave type
        if (slave_type == HW_I3C_SLAVE_DEVICE_LEGACY_I2C) {
                dev_addr_table_val |=  DEV_ADDR_TABLE_LEGACY_I2C_DEVICE;
        } else {
                dev_addr_table_val &=  ~DEV_ADDR_TABLE_LEGACY_I2C_DEVICE;
        }

        // Set I3C_DEV_ADDR_TABLE_LOCX_REG with provided settings
        *i3c_dev_addr_table_loc_x_reg = dev_addr_table_val;
}

void hw_i3c_set_slave_interrupt_request_rejection_enable(HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION slave_dev_loc, bool i3c_sir_rejection_ctrl)
{
        uint32_t val = 1;
        uint8_t dynamic_address = REG_GETF_INDEXED(I3C, I3C_DEV_ADDR_TABLE_LOC1_REG, DEV_DYNAMIC_ADDR, 4, slave_dev_loc);

        // Calculate pos based on mod32{dynamic_address[4:0]+dynamic_address[6:5]} equation
        uint32_t pos = ((dynamic_address & 0x1F) + ((dynamic_address >> 5) & 0x03)) % 32;

        val <<= pos;

        if (i3c_sir_rejection_ctrl) {
                I3C->I3C_IBI_SIR_REQ_REJECT_REG |= val;
        } else {
                I3C->I3C_IBI_SIR_REQ_REJECT_REG &= ~val;
        }
}

//===================== Read/Write functions ===================================

/**
 * \brief Write transfer argument into COMMAND QUEUE
 *
 * It is the caller's responsibility to ensure that there is free space in CMD QUEUE before
 * calling this function - hw_i3c_get_cmd_queue_empty_entries() can be used for this
 * purpose.
 *
 * \param [in] len data length in bytes
 *
 * \sa hw_i3c_get_cmd_queue_empty_entries
 */
static void hw_i3c_send_transfer_argument(uint16_t len)
{
        uint32_t transfer_argument;

        // Prepare I3C Transfer Argument
        transfer_argument = COMMAND_PORT_ARG_DATA_LEN(len) | I3C_COMMAND_ATTR_ARG;

        // Program I3C Transfer Argument
        hw_i3c_enqueue_command(transfer_argument);
}

/**
 * \brief Write transfer command into COMMAND QUEUE
 *
 * It is the caller's responsibility to ensure that there is free space in COMMAND QUEUE before
 * calling this function - hw_i3c_get_cmd_queue_empty_entries() can be used for this
 * purpose.
 *
 * \param [in] i3c_transfer_cfg I3C transfer configuration
 *
 * \sa hw_i3c_get_cmd_queue_empty_entries
 */
static void hw_i3c_send_transfer_command(const i3c_private_transfer_config *i3c_transfer_cfg)
{
        uint32_t transfer_command = 0;

        // Prepare I3C Transfer Command
        if (i3c_env.transfer_mode == I3C_TRANSFER_SDA_WRITE) {
                transfer_command |= I3C_COMMAND_PORT_SDAP;
        } else if (i3c_env.transfer_mode == I3C_TRANSFER_READ) {
                transfer_command |= I3C_COMMAND_PORT_READ_TRANSFER;
        }

        transfer_command |= I3C_COMMAND_ATTR_CMD |
                COMMAND_PORT_DEV_INDEX(i3c_transfer_cfg->slave_dev_idx) |
                COMMAND_PORT_SPEED(i3c_transfer_cfg->i3c_tranfer_speed) |
                COMMAND_PORT_TID(i3c_transfer_cfg->i3c_tid);

        // Add condition (STOP or RESTART) for transfer completion
        if (i3c_transfer_cfg->termination_on_completion == HW_I3C_TRANSFER_TOC_STOP) {
                transfer_command |= I3C_COMMAND_PORT_TOC;
        }

        // Generate response for the command
        if (i3c_transfer_cfg->response_on_completion) {
                transfer_command |= I3C_COMMAND_PORT_ROC;
        }

        // Program I3C Transfer Command
        hw_i3c_enqueue_command(transfer_command);
}

/**
 * \brief Recover I3C controller from error
 *
 * \param [in] error_response  The error response from RESP Queue
 *
 */
static void hw_i3c_recover_from_error(uint32_t error_response)
{
        hw_i3c_reset_cmd_queue();
        if (HW_I3C_RESPONSE_PORT_ERR_STATUS(error_response) == HW_I3C_RESPONSE_ERROR_TRANSF_ABORT) {
                // Reset all FIFOs and Queues
                hw_i3c_reset_ibi_queue();
                hw_i3c_reset_resp_queue();
                hw_i3c_reset_tx_fifo();
                hw_i3c_reset_rx_fifo();
        } else {
                if (i3c_env.transfer_mode == I3C_TRANSFER_WRITE) {
                        hw_i3c_reset_tx_fifo();
                } else if (i3c_env.transfer_mode == I3C_TRANSFER_READ) {
                        hw_i3c_reset_rx_fifo();
                }
        }

        // Resume controller from HALT state
        hw_i3c_controller_resume();
}

/**
 * \brief Write Short Data Argument into COMMAND QUEUE
 *
 * It is the caller's responsibility to ensure that there is free space in COMMAND QUEUE before
 * calling this function - hw_i3c_get_cmd_queue_empty_entries() can be used for this
 * purpose.
 *
 * \param [in] out_buf data to send
 * \param [in] len data length in bytes
 *
 * \details The max buffer length for SDA commands is 3 bytes.
 *
 * \sa hw_i3c_get_cmd_queue_empty_entries
 */
static void hw_i3c_send_sda_command(const uint8_t *out_buf, uint16_t len)
{
        uint32_t sda_command = 0;

        // Prepare I3C Short Data Argument
        sda_command |= I3C_COMMAND_ATTR_SDAP;

        switch (len) {
        case 3:
                sda_command |= I3C_COMMAND_PORT_SDA_BYTE_STRB_3;
                sda_command |= COMMAND_PORT_SDA_DATA_BYTE_3(out_buf[2]);
                /* no break */
        case 2:
                sda_command |= I3C_COMMAND_PORT_SDA_BYTE_STRB_2;
                sda_command |= COMMAND_PORT_SDA_DATA_BYTE_2(out_buf[1]);
                /* no break */
        case 1:
                sda_command |= I3C_COMMAND_PORT_SDA_BYTE_STRB_1;
                sda_command |= COMMAND_PORT_SDA_DATA_BYTE_1(out_buf[0]);
                break;
        default:
                // Max Buffer length 3 bytes for SDA command
                ASSERT_ERROR(0);
        }

        // Program I3C Short Data Argument
        hw_i3c_enqueue_command(sda_command);
}

HW_I3C_ERROR hw_i3c_set_ccc(i3c_ccc_transfer_config *i3c_ccc_cfg)
{
        // Check input parameter
        if (!i3c_ccc_cfg) {
                return HW_I3C_ERROR_INVALID_PARAMETER;
        }

        uint32_t ccc_command = 0;

        if ((i3c_ccc_cfg->i3c_ccc_data_len > 0) && (i3c_ccc_cfg->i3c_ccc_data_len <= SDA_MAX_SIZE)) {

                // Wait for an empty location in CMD-QUEUE
                while (hw_i3c_get_cmd_queue_empty_entries() == 0);

                // Send Short Data Argument
                hw_i3c_send_sda_command(i3c_ccc_cfg->i3c_ccc_data , i3c_ccc_cfg->i3c_ccc_data_len);

                // Prepare I3C Transfer(CCC) Command
                ccc_command |= I3C_COMMAND_PORT_SDAP;

        } else if (i3c_ccc_cfg->i3c_ccc_data_len > SDA_MAX_SIZE) {
        }

        // Prepare I3C Transfer(CCC) Command
        if (i3c_ccc_cfg->i3c_ccc_command_id == HW_I3C_CCC_ID_B_ENTDAA) {
                ccc_command |= I3C_COMMAND_ATTR_ADDR_ASSGN_CMD;

                ccc_command |= COMMAND_PORT_CMD(i3c_ccc_cfg->i3c_ccc_command_id) |
                        COMMAND_PORT_DEV_INDEX(i3c_ccc_cfg->slave_dev_idx) |
                        COMMAND_PORT_TID(i3c_ccc_cfg->i3c_tid) |
                        COMMAND_PORT_DEV_COUNT(i3c_ccc_cfg->i3c_dev_count);
        } else {
                ccc_command |= I3C_COMMAND_ATTR_CMD;

                ccc_command |= I3C_COMMAND_PORT_CP | COMMAND_PORT_CMD(i3c_ccc_cfg->i3c_ccc_command_id) |
                        COMMAND_PORT_DEV_INDEX(i3c_ccc_cfg->slave_dev_idx) |
                        COMMAND_PORT_TID(i3c_ccc_cfg->i3c_tid);
        }


        // Add condition (STOP or RESTART) for transfer completion
        if (i3c_ccc_cfg->termination_on_completion == HW_I3C_TRANSFER_TOC_STOP) {
                ccc_command |= I3C_COMMAND_PORT_TOC;
        }

        // Generate response for the command
        if (i3c_ccc_cfg->response_on_completion) {
                ccc_command |= I3C_COMMAND_PORT_ROC;
        }

        // Wait for an empty entry in CMD-QUEUE
        while (hw_i3c_get_cmd_queue_empty_entries() == 0);

        // Program I3C Transfer Command
        hw_i3c_enqueue_command(ccc_command);

        // Handle the case of an early response due to error on the bus or RESTART condition
        if (hw_i3c_get_resp_queue_level()) {

                // Get command response
                i3c_ccc_cfg->cmd_response.response = hw_i3c_dequeue_response();
                i3c_ccc_cfg->cmd_response.valid = true;

                if (HW_I3C_RESPONSE_PORT_ERR_STATUS(i3c_ccc_cfg->cmd_response.response) != HW_I3C_RESPONSE_ERROR_NO_ERROR) {
                        hw_i3c_recover_from_error(i3c_ccc_cfg->cmd_response.response);
                        return HW_I3C_ERROR_RESPONSE;
                } else {
                        // Response is from previous transfer without error
                        if (HW_I3C_RESPONSE_PORT_TID(i3c_ccc_cfg->cmd_response.response) != i3c_ccc_cfg->i3c_tid) {
                                i3c_ccc_cfg->cmd_response.valid = false;
                        }
                }
        }

        // Response status is required and transfer terminates with STOP condition
        if ((i3c_ccc_cfg->response_on_completion) && (i3c_ccc_cfg->termination_on_completion == HW_I3C_TRANSFER_TOC_STOP) &&
                (i3c_ccc_cfg->cmd_response.valid == false)) {

                do {
                        // Waiting for response
                        while (hw_i3c_get_resp_queue_level() == 0);

                        // Get command response
                        i3c_ccc_cfg->cmd_response.response = hw_i3c_dequeue_response();
                        i3c_ccc_cfg->cmd_response.valid = true;

                        if (HW_I3C_RESPONSE_PORT_ERR_STATUS(i3c_ccc_cfg->cmd_response.response) != HW_I3C_RESPONSE_ERROR_NO_ERROR) {
                                hw_i3c_recover_from_error(i3c_ccc_cfg->cmd_response.response);
                                return HW_I3C_ERROR_RESPONSE;
                        } else {
                                // Response is from previous transfer without error
                                if (HW_I3C_RESPONSE_PORT_TID(i3c_ccc_cfg->cmd_response.response) != i3c_ccc_cfg->i3c_tid) {
                                        i3c_ccc_cfg->cmd_response.valid = false;
                                }
                        }

                // Get response for current transfer
                } while (HW_I3C_RESPONSE_PORT_TID(i3c_ccc_cfg->cmd_response.response) != i3c_ccc_cfg->i3c_tid);
        }
        return HW_I3C_ERROR_NONE;
}

/**
 * \brief Configure I3C private transfer environment
 *
 * \param [in] i3c_transfer_cfg I3C transfer configuration
 * \param [in] transfer_mode transfer mode
 *
 * \return HW_I3C_ERROR_NONE if no error occurred, else error code.
 *
 */
static HW_I3C_ERROR hw_i3c_private_xfer_env_config(const i3c_private_transfer_config *i3c_transfer_cfg, I3C_TRANSFER transfer_mode)
{
        if (!i3c_transfer_cfg) {
                return HW_I3C_ERROR_INVALID_PARAMETER;
        }

        // Copy private transfer parameters to environment
        memcpy(&i3c_env.transfer_cfg, i3c_transfer_cfg, sizeof(i3c_private_transfer_config));
        i3c_env.transfer_mode = transfer_mode;

        return HW_I3C_ERROR_NONE;
}

/**
 * \brief Write SDA buffer to I3C
 *
 * It initiates an I3C transmission, no data is received (Write-only mode for up to 3 bytes).
 * If no callback is provided this function waits for the transfer to finish.
 * If a callback is provided, the function sets up the transfer in interrupt mode
 * and ends immediately.
 * If i3c_transfer_cfg->response_on_completion is false, this function ends immediately
 * without waiting for response.
 * In callback mode data pointed by out_buf should not be touched till callback is called.
 *
 * \param [in,out] i3c_transfer_cfg I3C private transfer configuration and response when response
 *  on completion is required
 * \param [in] out_buf data to send
 * \param [in] len data length in bytes
 * \param [in] cb callback to call after transfer is finished, if no callback is provided
 *             the transfer will be initiated in blocking mode
 * \param [in] user_data parameter for callback
 *
 * \return HW_I3C_ERROR_NONE if no error occurred, else error code
 *
 * \details In blocking mode the response status of a transfer can be retrieved at the end of the
 * transmission using i3c_transfer_cfg->cmd_response parameter. In callback mode the response status
 * is returned by callback function. The following macro definitions could be used to parse response
 * in order to check error status, transaction ID and remaining data length if the transfer terminated
 * early.
 *
 * \sa HW_I3C_RESPONSE_PORT_DATA_LEN, HW_I3C_RESPONSE_PORT_ERR_STATUS, HW_I3C_RESPONSE_PORT_TID
 *
*/
static HW_I3C_ERROR hw_i3c_private_write_sda_buffer(i3c_private_transfer_config *i3c_transfer_cfg, const uint8_t *out_buf, uint16_t len, hw_i3c_xfer_callback cb, void *user_data)
{
        // Initialize transfer response
        i3c_transfer_cfg->cmd_response.response = 0;
        i3c_transfer_cfg->cmd_response.valid = false;

        if (hw_i3c_private_xfer_env_config(i3c_transfer_cfg, I3C_TRANSFER_SDA_WRITE) != HW_I3C_ERROR_NONE) {
                return HW_I3C_ERROR_INVALID_PARAMETER;
        }

        if (cb == NULL) {

                // Wait for an empty entry in CMD-QUEUE
                while (hw_i3c_get_cmd_queue_empty_entries() == 0);

                // Send Short Data Argument
                hw_i3c_send_sda_command(out_buf, len);

                // Wait for an empty entry in CMD-QUEUE
                while (hw_i3c_get_cmd_queue_empty_entries() == 0);

                // Send Transfer Command
                hw_i3c_send_transfer_command(&i3c_env.transfer_cfg);

                // Handle the case of an early response due to error on the bus or RESTART condition
                if (hw_i3c_get_resp_queue_level()) {

                        // Get command response
                        i3c_transfer_cfg->cmd_response.response = hw_i3c_dequeue_response();
                        i3c_transfer_cfg->cmd_response.valid = true;

                        if (HW_I3C_RESPONSE_PORT_ERR_STATUS(i3c_transfer_cfg->cmd_response.response) != HW_I3C_RESPONSE_ERROR_NO_ERROR) {
                                hw_i3c_recover_from_error(i3c_transfer_cfg->cmd_response.response);
                                return HW_I3C_ERROR_RESPONSE;
                        } else {
                                // Response is from previous transfer without error
                                if (HW_I3C_RESPONSE_PORT_TID(i3c_transfer_cfg->cmd_response.response) != i3c_env.transfer_cfg.i3c_tid) {
                                        i3c_transfer_cfg->cmd_response.valid = false;
                                }
                        }
                }

                // Response status is required and transfer terminates with STOP condition
                if ((i3c_env.transfer_cfg.response_on_completion) && (i3c_env.transfer_cfg.termination_on_completion == HW_I3C_TRANSFER_TOC_STOP) &&
                        (i3c_transfer_cfg->cmd_response.valid == false)) {

                        do {
                                // Waiting for response
                                while (hw_i3c_get_resp_queue_level() == 0);

                                // Get command response
                                i3c_transfer_cfg->cmd_response.response = hw_i3c_dequeue_response();
                                i3c_transfer_cfg->cmd_response.valid = true;

                                if (HW_I3C_RESPONSE_PORT_ERR_STATUS(i3c_transfer_cfg->cmd_response.response) != HW_I3C_RESPONSE_ERROR_NO_ERROR) {
                                        hw_i3c_recover_from_error(i3c_transfer_cfg->cmd_response.response);
                                        return HW_I3C_ERROR_RESPONSE;
                                } else {
                                        // Response is from previous transfer without error
                                        if (HW_I3C_RESPONSE_PORT_TID(i3c_transfer_cfg->cmd_response.response) != i3c_env.transfer_cfg.i3c_tid) {
                                                i3c_transfer_cfg->cmd_response.valid = false;
                                        }
                                }

                        // Get response for current transfer
                        } while (HW_I3C_RESPONSE_PORT_TID(i3c_transfer_cfg->cmd_response.response) != i3c_env.transfer_cfg.i3c_tid);
                }
       } else {
               i3c_env.tx_buffer = out_buf;
               i3c_env.tx_len = len;
               i3c_env.xfer_cb = cb;
               i3c_env.tx_user_data = user_data;

               // Set Command Queue empty threshold
               hw_i3c_set_cmd_empty_queue_threshold(HW_I3C_CMD_EMPTY_QUEUE_TL_0);

               // Set Response buffer threshold
               hw_i3c_set_resp_queue_threshold(HW_I3C_RESP_QUEUE_TL_1);

               uint32_t irq_sources = HW_I3C_INT_CMD_QUEUE_READY_STS | HW_I3C_INT_RESP_READY_STS | HW_I3C_INT_TRANSFER_ERR_STS | HW_I3C_INT_TRANSFER_ABORT_STS;

               hw_i3c_enable_irq_sources(irq_sources);
       }
       return HW_I3C_ERROR_NONE;
}

/**
 *
 * \brief Writes data to TX FIFO using I3C_RX_TX_DATA_PORT_REG Register
 *
 * \param [in] buf data to write to TX Buffer
 * \param [in] word_num word number
 * \param [in] len total length
 *
 * \details The transmit data should always be packed as 4-byte aligned data
 * words and written to the Transmit Data Port register. If the Command length
 * is not aligned to 4-bytes, then the additional bytes are ignored.
 *
 * \return The number of bytes written to the TX PORT
 *
 */
static uint8_t hw_i3c_write_word_tx_port(const uint8_t *buf, uint16_t word_num, uint16_t len)
{
        uint32_t tx_data = 0;
        uint8_t word_offset;

        for (word_offset = 0; word_offset< 4 && (word_offset + word_num) < len; word_offset++) {
                tx_data |= (buf[word_offset + word_num] << (word_offset * 8));
        }

        hw_i3c_write_tx_port(tx_data);

        return word_offset;
}

/**
 * \brief Write array of bytes to I3C
 *
 * It initiates an I3C transmission, no data is received (Write-only mode).
 * If no callback is provided this function waits for the transfer to finish.
 * If a callback is provided, the function sets up the transfer in interrupt mode
 * and ends immediately.
 * If i3c_transfer_cfg->response_on_completion is false, this function ends immediately
 * without waiting for response.
 * In callback mode data pointed by out_buf and i3c_transfer_cfg should not be touched till
 * callback is called.
 *
 * \param [in,out] i3c_transfer_cfg I3C transfer configuration and response when response
 *  on completion is required in blocking mode
 * \param [in] out_buf data to send
 * \param [in] len data length in bytes
 * \param [in] cb callback to call after transfer is finished, if no callback is provided
 *             the transfer will be initiated in blocking mode
 * \param [in] user_data parameter for callback
 *
 * \return HW_I3C_ERROR_NONE if no error occurred, else error code
 *
 * \details The response status of a transfer can be retrieved at the end of the transmission
 * using i3c_transfer_cfg->cmd_response parameter. The following macro definitions could be used
 * to parse response in order to check error status, transaction ID and remaining data length if
 * the transfer terminated early.
 *
 * \sa HW_I3C_RESPONSE_PORT_DATA_LEN, HW_I3C_RESPONSE_PORT_ERR_STATUS, HW_I3C_RESPONSE_PORT_TID
 *
 * \warning If a callback is provided, the function registers an internal interrupt handler, which
 *          overrides any previously installed handler.
 *
 * \warning In DMA mode the supplied buffer address must be word-aligned.
 *
*/
static HW_I3C_ERROR hw_i3c_private_write_buffer(i3c_private_transfer_config *i3c_transfer_cfg, const uint8_t *out_buf, uint16_t len, hw_i3c_xfer_callback cb, void *user_data)
{
#if (HW_I3C_DMA_SUPPORT == 1)
        if (i3c_env.use_dma) {
                // Check buffer alignment
                if (((uint32_t) out_buf & 3) != 0) {
                        return HW_I3C_ERROR_INVALID_PARAMETER;
                }
        }
#endif
        // Initialize transfer response
        i3c_transfer_cfg->cmd_response.response = 0;
        i3c_transfer_cfg->cmd_response.valid = false;

        if (hw_i3c_private_xfer_env_config(i3c_transfer_cfg, I3C_TRANSFER_WRITE) != HW_I3C_ERROR_NONE) {
                return HW_I3C_ERROR_INVALID_PARAMETER;
        }

        if (cb == NULL) {
                uint32_t i;

                // Wait for an empty entry in CMD-QUEUE
                while (hw_i3c_get_cmd_queue_empty_entries() == 0);

                // Send Transfer Argument
                hw_i3c_send_transfer_argument(len);

                // Wait for an empty entry in CMD-QUEUE
                while (hw_i3c_get_cmd_queue_empty_entries() == 0);

                // Send Transfer Command
                hw_i3c_send_transfer_command(&i3c_env.transfer_cfg);

                for (i = 0; i < len; i += 4) {

                        while ((hw_i3c_get_tx_buffer_empty_locations() == 0) || hw_i3c_get_resp_queue_level()) {

                                // Handle the case of an early response due to error on the bus or RESTART condition
                                if (hw_i3c_get_resp_queue_level()) {

                                        // Get command response
                                        i3c_transfer_cfg->cmd_response.response = hw_i3c_dequeue_response();
                                        i3c_transfer_cfg->cmd_response.valid = true;

                                        if (HW_I3C_RESPONSE_PORT_ERR_STATUS(i3c_transfer_cfg->cmd_response.response) != HW_I3C_RESPONSE_ERROR_NO_ERROR) {
                                                hw_i3c_recover_from_error(i3c_transfer_cfg->cmd_response.response);
                                                return HW_I3C_ERROR_RESPONSE;
                                        } else {
                                                // Response is from previous transfer without error
                                                if (HW_I3C_RESPONSE_PORT_TID(i3c_transfer_cfg->cmd_response.response) != i3c_transfer_cfg->i3c_tid) {
                                                        i3c_transfer_cfg->cmd_response.valid = false;
                                                }
                                        }
                                }
                        }
                        hw_i3c_write_word_tx_port(out_buf, i, len);
                }

                // Response status is required and transfer terminates with STOP condition
                if ((i3c_transfer_cfg->response_on_completion) && (i3c_transfer_cfg->termination_on_completion == HW_I3C_TRANSFER_TOC_STOP) &&
                        (i3c_transfer_cfg->cmd_response.valid == false)) {

                        do {
                                // Waiting for response
                                while (hw_i3c_get_resp_queue_level() == 0);

                                // Get command response
                                i3c_transfer_cfg->cmd_response.response = hw_i3c_dequeue_response();
                                i3c_transfer_cfg->cmd_response.valid = true;

                                if (HW_I3C_RESPONSE_PORT_ERR_STATUS(i3c_transfer_cfg->cmd_response.response) != HW_I3C_RESPONSE_ERROR_NO_ERROR) {
                                        hw_i3c_recover_from_error(i3c_transfer_cfg->cmd_response.response);
                                        return HW_I3C_ERROR_RESPONSE;
                                } else {
                                        // Response is from previous transfer without error
                                        if (HW_I3C_RESPONSE_PORT_TID(i3c_transfer_cfg->cmd_response.response) != i3c_env.transfer_cfg.i3c_tid) {
                                                i3c_transfer_cfg->cmd_response.valid = false;
                                        }
                                }

                        // Get response for current transfer
                        } while (HW_I3C_RESPONSE_PORT_TID(i3c_transfer_cfg->cmd_response.response) != i3c_env.transfer_cfg.i3c_tid);
                }
#if (HW_I3C_DMA_SUPPORT == 1)
        } else if (i3c_env.use_dma) {
                i3c_env.tx_buffer = out_buf;
                i3c_env.tx_len = len;
                i3c_env.tx_num = 0;
                i3c_env.xfer_cb = cb;
                i3c_env.tx_user_data = user_data;

                // Set Command Queue empty threshold
                hw_i3c_set_cmd_empty_queue_threshold(HW_I3C_CMD_EMPTY_QUEUE_TL_0);

                // Set Response buffer threshold
                hw_i3c_set_resp_queue_threshold(HW_I3C_RESP_QUEUE_TL_1);

                // Configure TX DMA Channel and TX FIFO threshold level for I3C
                if (len < 16) {
                        i3c_env.tx_dma.burst_mode = HW_DMA_BURST_MODE_DISABLED;
                        hw_i3c_set_tx_empty_buffer_threshold(HW_I3C_TX_FIFO_EMPTY_TL_1);
                } else if (len < 32) {
                        i3c_env.tx_dma.burst_mode = HW_DMA_BURST_MODE_4x;
                        hw_i3c_set_tx_empty_buffer_threshold(HW_I3C_TX_FIFO_EMPTY_TL_4);
                } else {
                        i3c_env.tx_dma.burst_mode = HW_DMA_BURST_MODE_8x;
                        hw_i3c_set_tx_empty_buffer_threshold(HW_I3C_TX_FIFO_EMPTY_TL_8);
                }

                i3c_env.tx_dma.src_address = (uint32_t)out_buf;
                i3c_env.tx_dma.length = (len + 3) >> 2;

                // Initialize TX DMA Channel for I3C
                hw_dma_channel_initialization(&i3c_env.tx_dma);

                uint32_t irq_sources = HW_I3C_INT_CMD_QUEUE_READY_STS | HW_I3C_INT_RESP_READY_STS | HW_I3C_INT_TRANSFER_ERR_STS | HW_I3C_INT_TRANSFER_ABORT_STS;

                hw_i3c_enable_irq_sources(irq_sources);

                // Enable TX DMA
                hw_dma_channel_enable(i3c_env.tx_dma.channel_number, HW_DMA_STATE_ENABLED);
#endif /* HW_I3C_DMA_SUPPORT */
        } else {
                i3c_env.tx_buffer = out_buf;
                i3c_env.tx_len = len;
                i3c_env.tx_num = 0;
                i3c_env.xfer_cb = cb;
                i3c_env.tx_user_data = user_data;

                // Set Command Queue empty threshold
                hw_i3c_set_cmd_empty_queue_threshold(HW_I3C_CMD_EMPTY_QUEUE_TL_0);

                // Set Response buffer threshold
                hw_i3c_set_resp_queue_threshold(HW_I3C_RESP_QUEUE_TL_1);

                uint32_t irq_sources = HW_I3C_INT_CMD_QUEUE_READY_STS | HW_I3C_INT_TX_THLD_STS | HW_I3C_INT_RESP_READY_STS | HW_I3C_INT_TRANSFER_ERR_STS | HW_I3C_INT_TRANSFER_ABORT_STS;

                hw_i3c_enable_irq_sources(irq_sources);
        }
        return HW_I3C_ERROR_NONE;
}

static void hw_i3c_xfer_reply(bool success)
{
        // Disable all events except IBI threshold event
        hw_i3c_enable_irq_sources(HW_I3C_INT_IBI_THLD_STS);

        // Fire user callback
        if (i3c_env.xfer_cb) {
                hw_i3c_xfer_callback cb = i3c_env.xfer_cb;
                hw_i3c_reset_xfer_cb();
                if ((i3c_env.transfer_mode == I3C_TRANSFER_WRITE) || (i3c_env.transfer_mode == I3C_TRANSFER_SDA_WRITE)) {
                        cb(i3c_env.tx_user_data, success, &i3c_env.transfer_cfg.cmd_response);
                } else if (i3c_env.transfer_mode == I3C_TRANSFER_READ) {
                        cb(i3c_env.rx_user_data, success, &i3c_env.transfer_cfg.cmd_response);
                }
        }
}

HW_I3C_ERROR hw_i3c_private_write_buf(i3c_private_transfer_config *i3c_transfer_cfg, const uint8_t *out_buf, uint16_t len, hw_i3c_xfer_callback cb, void *user_data)
{
        // Check input parameters
        if ((!out_buf) || (len < 1)) {
                return HW_I3C_ERROR_INVALID_PARAMETER;
        }

        HW_I3C_ERROR error_code;

        if (len <= SDA_MAX_SIZE) {
                error_code = hw_i3c_private_write_sda_buffer(i3c_transfer_cfg, out_buf, len, cb, user_data);
        } else {
                error_code = hw_i3c_private_write_buffer(i3c_transfer_cfg, out_buf, len, cb, user_data);
        }
        return error_code;
}

/**
*
* \brief Read data from RX FIFO using I3C_RX_TX_DATA_PORT_REG Register
*
* \param [out] buf data read from RX Buffer
* \param [in] word_num
* \param [in] len
*
* \details The Receive data is always packed in 4-byte aligned data words
* and stored in the RX-Data Buffer. If the command length is not aligned
* to the 4-bytes, then the additional data bytes have to be ignored.
*
* \return The number of bytes read from RX PORT
*
*/
static uint8_t hw_i3c_read_word_rx_port(uint8_t *buf, uint16_t word_num, uint16_t len)
{
        uint32_t rx_data = 0;
        uint8_t word_offset;

        rx_data = hw_i3c_read_rx_port();

        for (word_offset = 0; word_offset < 4 && (word_offset + word_num) < len; word_offset++) {
                buf[word_offset + word_num] = rx_data >> (word_offset * 8);
        }
        return word_offset;
}

HW_I3C_ERROR hw_i3c_private_read_buf(i3c_private_transfer_config *i3c_transfer_cfg, uint8_t *in_buf, uint16_t len, hw_i3c_xfer_callback cb, void *user_data)
{
        // Check input parameters
        if ((!in_buf) || (len < 1)) {
                return HW_I3C_ERROR_INVALID_PARAMETER;
        }

#if (HW_I3C_DMA_SUPPORT == 1)
        if (i3c_env.use_dma) {
                // Check transfer length and buffer alignment
                if (((len & 3) != 0) || ((((uint32_t) in_buf & 3) != 0))) {
                        return HW_I3C_ERROR_INVALID_PARAMETER;
                }
        }
#endif
        // Initialize transfer response
        i3c_transfer_cfg->cmd_response.response = 0;
        i3c_transfer_cfg->cmd_response.valid = false;

        if (hw_i3c_private_xfer_env_config(i3c_transfer_cfg, I3C_TRANSFER_READ) != HW_I3C_ERROR_NONE) {
                return HW_I3C_ERROR_INVALID_PARAMETER;
        }

        if (cb == NULL) {
                uint32_t i;

                // Wait for an empty entry in CMD-QUEUE
                while (hw_i3c_get_cmd_queue_empty_entries() == 0);

                // Send Transfer Argument
                hw_i3c_send_transfer_argument(len);

                // Wait for an empty entry in CMD-QUEUE
                while (hw_i3c_get_cmd_queue_empty_entries() == 0);

                // Send Transfer Command
                hw_i3c_send_transfer_command(&i3c_env.transfer_cfg);

                for (i = 0; i < len; i += 4) {

                        while ((hw_i3c_get_rx_buffer_level() == 0) || hw_i3c_get_resp_queue_level()) {

                                // Handle the case of an early response due to error on the bus or RESTART condition
                                if (hw_i3c_get_resp_queue_level()) {

                                        // Get command response
                                        i3c_transfer_cfg->cmd_response.response = hw_i3c_dequeue_response();
                                        i3c_transfer_cfg->cmd_response.valid = true;

                                        if (HW_I3C_RESPONSE_PORT_ERR_STATUS(i3c_transfer_cfg->cmd_response.response) != HW_I3C_RESPONSE_ERROR_NO_ERROR) {
                                                hw_i3c_recover_from_error(i3c_transfer_cfg->cmd_response.response);
                                                return HW_I3C_ERROR_RESPONSE;
                                        } else {
                                                // Response is from previous transfer without error
                                                if (HW_I3C_RESPONSE_PORT_TID(i3c_transfer_cfg->cmd_response.response) != i3c_transfer_cfg->i3c_tid) {
                                                        i3c_transfer_cfg->cmd_response.valid = false;
                                                }
                                        }
                                }
                        };
                        hw_i3c_read_word_rx_port(in_buf, i, len);
                }

                // Response status is required and transfer terminates with STOP condition
                if ((i3c_transfer_cfg->response_on_completion) && (i3c_transfer_cfg->termination_on_completion == HW_I3C_TRANSFER_TOC_STOP) &&
                        (i3c_transfer_cfg->cmd_response.valid == false)) {

                        do {
                                // Waiting for response
                                while (hw_i3c_get_resp_queue_level() == 0);

                                // Get command response
                                i3c_transfer_cfg->cmd_response.response = hw_i3c_dequeue_response();
                                i3c_transfer_cfg->cmd_response.valid = true;

                                if (HW_I3C_RESPONSE_PORT_ERR_STATUS(i3c_transfer_cfg->cmd_response.response) != HW_I3C_RESPONSE_ERROR_NO_ERROR) {
                                        hw_i3c_recover_from_error(i3c_transfer_cfg->cmd_response.response);
                                        return HW_I3C_ERROR_RESPONSE;
                                } else {
                                        // Response is from previous transfer without error
                                        if (HW_I3C_RESPONSE_PORT_TID(i3c_transfer_cfg->cmd_response.response) != i3c_transfer_cfg->i3c_tid) {
                                                i3c_transfer_cfg->cmd_response.valid = false;
                                        }
                                }

                        // Get response for current transfer
                        } while (HW_I3C_RESPONSE_PORT_TID(i3c_transfer_cfg->cmd_response.response) != i3c_transfer_cfg->i3c_tid);
                }
#if (HW_I3C_DMA_SUPPORT == 1)
        } else if (i3c_env.use_dma) {
                i3c_env.rx_buffer = in_buf;
                i3c_env.rx_len = len;
                i3c_env.rx_num = 0;
                i3c_env.xfer_cb = cb;
                i3c_env.rx_user_data = user_data;

                // Set Command Queue empty threshold
                hw_i3c_set_cmd_empty_queue_threshold(HW_I3C_CMD_EMPTY_QUEUE_TL_0);

                // Set Response buffer threshold
                hw_i3c_set_resp_queue_threshold(HW_I3C_RESP_QUEUE_TL_1);

                // Configure RX DMA Channel and RX FIFO threshold level for I3C
                if (len % 32 == 0) {
                        i3c_env.rx_dma.burst_mode = HW_DMA_BURST_MODE_8x;
                        hw_i3c_set_rx_buffer_threshold(HW_I3C_RX_FIFO_USED_TL_8);
                } else if (len % 16 == 0) {
                        i3c_env.rx_dma.burst_mode = HW_DMA_BURST_MODE_4x;
                        hw_i3c_set_rx_buffer_threshold(HW_I3C_RX_FIFO_USED_TL_4);
                } else {
                        i3c_env.rx_dma.burst_mode = HW_DMA_BURST_MODE_DISABLED;
                        hw_i3c_set_rx_buffer_threshold(HW_I3C_RX_FIFO_USED_TL_1);
                }

                i3c_env.rx_dma.dest_address = (uint32_t)in_buf;
                i3c_env.rx_dma.length = (len + 3) >> 2;

                // Initialize RX DMA Channel for I3C
                hw_dma_channel_initialization(&i3c_env.rx_dma);

                uint32_t irq_sources = HW_I3C_INT_CMD_QUEUE_READY_STS | HW_I3C_INT_RESP_READY_STS | HW_I3C_INT_TRANSFER_ERR_STS | HW_I3C_INT_TRANSFER_ABORT_STS;

                hw_i3c_enable_irq_sources(irq_sources);

                // Enable RX DMA
                hw_dma_channel_enable(i3c_env.rx_dma.channel_number, HW_DMA_STATE_ENABLED);
#endif /* HW_I3C_DMA_SUPPORT */
        } else {
                i3c_env.rx_buffer = in_buf;
                i3c_env.rx_len = len;
                i3c_env.rx_num = 0;
                i3c_env.xfer_cb = cb;
                i3c_env.rx_user_data = user_data;

                // Set Command Queue empty threshold
                hw_i3c_set_cmd_empty_queue_threshold(HW_I3C_CMD_EMPTY_QUEUE_TL_0);

                // Set Response buffer threshold
                hw_i3c_set_resp_queue_threshold(HW_I3C_RESP_QUEUE_TL_1);

                // Set RX FIFO threshold level
                hw_i3c_set_rx_buffer_threshold(HW_I3C_RX_FIFO_USED_TL_1);

                uint32_t irq_sources = HW_I3C_INT_CMD_QUEUE_READY_STS | HW_I3C_INT_RX_THLD_STS | HW_I3C_INT_RESP_READY_STS | HW_I3C_INT_TRANSFER_ERR_STS | HW_I3C_INT_TRANSFER_ABORT_STS;

                hw_i3c_enable_irq_sources(irq_sources);
        }
        return HW_I3C_ERROR_NONE;
}

bool hw_i3c_is_occupied(void)
{
        if (i3c_env.xfer_cb != NULL) {
                return true;
        }
        return false;
}

void hw_i3c_reset_xfer_cb(void)
{
        i3c_env.xfer_cb = NULL;
}

static void hw_i3c_intr_handler(uint32_t mask)
{
        if ((mask & HW_I3C_INT_TRANSFER_ERR_STS) || (mask & HW_I3C_INT_TRANSFER_ABORT_STS)) {

                if (mask & HW_I3C_INT_TRANSFER_ERR_STS) {
                        // Ack transfer error interrupt
                        HW_I3C_REG_SET_BIT(I3C_INTR_STATUS_REG, TRANSFER_ERR_STS);
                }

                if (mask & HW_I3C_INT_TRANSFER_ABORT_STS) {
                        // Ack transfer abort interrupt
                        HW_I3C_REG_SET_BIT(I3C_INTR_STATUS_REG, TRANSFER_ABORT_STS);
                }
                goto get_response;
        }
        if (mask & HW_I3C_INT_IBI_THLD_STS) {
                // Get IBI status from IBI QUEUE
                uint32_t ibi_queue_status_regval = hw_i3c_dequeue_ibi();

                i3c_ibi_sir_hj_request ibi_sir_hj_request;
                ibi_sir_hj_request.ibi_status = IBI_PORT_STS(ibi_queue_status_regval) ? HW_I3C_IBI_STATUS_NACK : HW_I3C_IBI_STATUS_ACK;
                ibi_sir_hj_request.ibi_rnw_bit = IBI_PORT_RNW_BIT(ibi_queue_status_regval) ? HW_I3C_IBI_RNW_BIT_READ : HW_I3C_IBI_RNW_BIT_WRITE;
                ibi_sir_hj_request.ibi_id = IBI_PORT_ID(ibi_queue_status_regval);
                ibi_sir_hj_request.ibi_type = (ibi_sir_hj_request.ibi_id == HW_I3C_HOT_JOIN_ID)
                        && (ibi_sir_hj_request.ibi_rnw_bit == HW_I3C_IBI_RNW_BIT_WRITE) ? HW_I3C_IBI_TYPE_HJ : HW_I3C_IBI_TYPE_SIR;

                // Fire user callback
                if (i3c_env.ibi_sir_hj_config.ibi_sir_hj_cb) {
                        i3c_env.ibi_sir_hj_config.ibi_sir_hj_cb(ibi_sir_hj_request);
                }
        }
        if (mask & HW_I3C_INT_CMD_QUEUE_READY_STS) {

                HW_I3C_REG_CLR_BIT(I3C_INTR_SIGNAL_EN_REG, CMD_QUEUE_READY_SIGNAL_EN);
                HW_I3C_REG_CLR_BIT(I3C_INTR_STATUS_EN_REG, CMD_QUEUE_READY_STS_EN);

                if (i3c_env.transfer_mode == I3C_TRANSFER_SDA_WRITE) {
                        // Send Short Data Argument
                        hw_i3c_send_sda_command(i3c_env.tx_buffer, i3c_env.tx_len);
                } else if (i3c_env.transfer_mode == I3C_TRANSFER_WRITE) {
                        // Send Transfer Argument
                        hw_i3c_send_transfer_argument(i3c_env.tx_len);
                } else if (i3c_env.transfer_mode == I3C_TRANSFER_READ) {
                        // Send Transfer Argument
                        hw_i3c_send_transfer_argument(i3c_env.rx_len);
                }

                // Send Transfer Command
                hw_i3c_send_transfer_command(&i3c_env.transfer_cfg);

                if (i3c_env.transfer_mode == I3C_TRANSFER_SDA_WRITE) {
                        // Fire user callback immediately if response on completion is not required or RESTART condition
                        if ((i3c_env.transfer_cfg.response_on_completion == false) ||
                                (i3c_env.transfer_cfg.termination_on_completion == HW_I3C_TRANSFER_TOC_RESTART)) {
                                hw_i3c_xfer_reply(true);
                        }
                }
        }
        if (mask & HW_I3C_INT_TX_THLD_STS) {

                while (i3c_env.tx_num < i3c_env.tx_len && hw_i3c_get_tx_buffer_empty_locations()) {

                        i3c_env.tx_num += hw_i3c_write_word_tx_port(i3c_env.tx_buffer, i3c_env.tx_num, i3c_env.tx_len);
                }

                if (i3c_env.tx_num == i3c_env.tx_len) {
                        HW_I3C_REG_CLR_BIT(I3C_INTR_SIGNAL_EN_REG, TX_THLD_SIGNAL_EN);
                        HW_I3C_REG_CLR_BIT(I3C_INTR_STATUS_EN_REG, TX_THLD_STS_EN);
                }

                // Fire user callback when response on completion is not required or RESTART condition enabled
                // and all data were written into TX FIFO
                if ((i3c_env.tx_num == i3c_env.tx_len) && ((i3c_env.transfer_cfg.termination_on_completion == HW_I3C_TRANSFER_TOC_RESTART) ||
                        (i3c_env.transfer_cfg.response_on_completion == false))) {
                        hw_i3c_xfer_reply(true);
                }
        }
        if (mask & HW_I3C_INT_RX_THLD_STS) {

                uint32_t i, length;

                length = hw_i3c_get_rx_buffer_level() * 4;

                // Get received data from RX FIFO
                for (i = 0; i < length; i += 4) {
                        i3c_env.rx_num += hw_i3c_read_word_rx_port(i3c_env.rx_buffer, i3c_env.rx_num, i3c_env.rx_len);
                }

                // Fire user callback when response on completion is not required or RESTART condition enabled
                // and all data were read from RX FIFO
                if ((i3c_env.rx_num == i3c_env.rx_len) && ((i3c_env.transfer_cfg.termination_on_completion == HW_I3C_TRANSFER_TOC_RESTART) ||
                        (i3c_env.transfer_cfg.response_on_completion == false))) {
                        hw_i3c_xfer_reply(true);
                }
        }
get_response:
        if (mask & HW_I3C_INT_RESP_READY_STS) {

                // Get command response
                i3c_env.transfer_cfg.cmd_response.response = hw_i3c_dequeue_response();
                i3c_env.transfer_cfg.cmd_response.valid = true;

                if (HW_I3C_RESPONSE_PORT_ERR_STATUS(i3c_env.transfer_cfg.cmd_response.response) != HW_I3C_RESPONSE_ERROR_NO_ERROR) {

                        hw_i3c_recover_from_error(i3c_env.transfer_cfg.cmd_response.response);

#if (HW_I3C_DMA_SUPPORT == 1)
                        if (i3c_env.use_dma) {
                                if (i3c_env.transfer_mode == I3C_TRANSFER_WRITE) {
                                        hw_dma_channel_enable(i3c_env.tx_dma.channel_number, HW_DMA_STATE_DISABLED);

                                } else if (i3c_env.transfer_mode == I3C_TRANSFER_READ) {
                                        hw_dma_channel_enable(i3c_env.rx_dma.channel_number, HW_DMA_STATE_DISABLED);
                                }
                        }
#endif
                        // Fire user callback
                        hw_i3c_xfer_reply(false);
                        return;
                }

                // Response is from previous transfer without error
                if (HW_I3C_RESPONSE_PORT_TID(i3c_env.transfer_cfg.cmd_response.response) != i3c_env.transfer_cfg.i3c_tid) {
                        i3c_env.transfer_cfg.cmd_response.valid = false;
                } else {
#if (HW_I3C_DMA_SUPPORT == 1)
                        if (i3c_env.use_dma == 0) {
#endif
                                if (i3c_env.transfer_mode == I3C_TRANSFER_READ) {
                                        if ((i3c_env.rx_num < i3c_env.rx_len) &&
                                                (HW_I3C_RESPONSE_PORT_ERR_STATUS(i3c_env.transfer_cfg.cmd_response.response)) == HW_I3C_RESPONSE_ERROR_NO_ERROR) {

                                                uint32_t i, length;

                                                length = HW_I3C_RESPONSE_PORT_DATA_LEN(i3c_env.transfer_cfg.cmd_response.response) - i3c_env.rx_num;

                                                // Get received data from RX FIFO
                                                for (i = 0; i < length; i += 4) {
                                                        i3c_env.rx_num += hw_i3c_read_word_rx_port(i3c_env.rx_buffer, i3c_env.rx_num, i3c_env.rx_len);
                                                }
                                        }
                                }

                                // Fire user callback
                                hw_i3c_xfer_reply(true);
#if (HW_I3C_DMA_SUPPORT == 1)
                        } else {
                                if (i3c_env.transfer_mode == I3C_TRANSFER_WRITE) {
                                        if (hw_dma_is_channel_active(i3c_env.tx_dma.channel_number) == false) {
                                                hw_i3c_xfer_reply(true);
                                        }
                                } else if (i3c_env.transfer_mode == I3C_TRANSFER_READ) {
                                        if (hw_dma_is_channel_active(i3c_env.rx_dma.channel_number) == false) {
                                                hw_i3c_xfer_reply(true);
                                        }
                                } else if (i3c_env.transfer_mode == I3C_TRANSFER_SDA_WRITE) {
                                        hw_i3c_xfer_reply(true);
                                }
                        }
#endif
                }
        }
}

//=========================== Interrupt handling ===============================
void hw_i3c_register_interrupt_callback(hw_i3c_interrupt_callback cb)
{
        i3c_env.intr_cb = cb;
}

/**
 * \brief I3C Interrupt Handler
 *
 */
void I3C_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        // Read interrupt state
        uint32_t mask = I3C->I3C_INTR_STATUS_REG;

        mask &= I3C->I3C_INTR_STATUS_EN_REG;

        if (i3c_env.intr_cb) {
                i3c_env.intr_cb(mask);
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_I3C */

