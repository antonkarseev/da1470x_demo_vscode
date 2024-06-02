/**
 ****************************************************************************************
 *
 * @file hw_emmc.c
 *
 * @brief Implementation of the embedded Multi-Media Card (eMMC) Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configUSE_HW_EMMC == 1)

#include "hw_pd.h"
#include "hw_emmc.h"
#include "hw_clk.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#define HW_EMMC_CARD_RCA_DESELECT_VAL           (uint16_t)(0)     /* A zero 16-bit RCA value is used to de-select the card */
#define HW_EMMC_CARD_RCA_RESET_VAL              (uint16_t)(1)     /* A nonzero 16-bit value used for Relative Card Address (RCA) */

#define HW_EMMC_ABORT_TRANSFER_TRIES            (2)

#define HW_EMMC_HW_RESET_PULSE_MIN_US           (1)
#define HW_EMMC_HW_RESET_AFTER_DELAY_US         (500)

/**
 * \brief Driver context
 *
 */
static hw_sdhc_context_data_t sdhc_context;  /**< host controller context */
static hw_emmc_context_data_t emmc_context;  /**< eMMC card context */

/***************************************************************************************************
 *
 * Declaration of private functions
 *
 **************************************************************************************************/
/**
 * \brief eMMC Host Controller Setup Sequence
 *
 * \param [in] id               SDHC controller instance
 * \param [in] config           HC setup configuration

 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_hc_setup(HW_SDHC_ID id, const hw_sdhc_hc_setup_config_t *config);

/**
 * \brief eMMC Card Interface Setup Sequence
 *
 * \note eMMC is on-board (embedded), so detection is not required.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] frequency        HC and bus speed frequency in Hz

 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_card_interface_setup(HW_SDHC_ID id, uint32_t frequency);

/**
 * \brief Check that the card is in the expected state, send SEND_STATUS (CMD13)
 * \note This function should be called after CMD3 (SET_RELATIVE_ADDRESS)
 *
 * \param [in] id               SDHC controller instance
 * \param [in] state            expected card state
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_check_card_is_in_state(HW_SDHC_ID id, HW_SDHC_CARD_STATUS_CURRENT_STATE state);

/**
 * \brief Check whether the specified bits are set in the card status register
 *        The card status register is the response of SEND_STATUS (CMD13)
 *
 * \note This function should be called after CMD3 (SET_RELATIVE_ADDRESS)
 *
 * \param [in] id               SDHC controller instance
 * \param [in] status_mask      status mask specifies the bits that should be checked in status register
 *
 * \return HW_SDHC_STATUS_SUCCESS if the status_mask bits are not set.
 *         HW_SDHC_STATUS_ERROR_CARD_STATUS_ERRORS if the error bits in status_mask are set.
 *         HW_SDHC_STATUS_ERROR_CARD_STATUS_CARD_IS_LOCKED if the card is locked.
 */
static HW_SDHC_STATUS hw_emmc_check_card_status_register(HW_SDHC_ID id, uint32_t status_mask);

/**
 * \brief eMMC Card Initialization And Identification Sequence
 * \note After the end of this function the card is in Transfer state
 *
 * \param [in] id               SDHC controller instance
 * \param [in] bus_config       Data bus configuration
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_card_initializing_and_identifying(HW_SDHC_ID id, const hw_sdhc_bus_config_t bus_config);

/**
 * \brief Set bus voltage Vdd1
 *
 * \param [in] id               SDHC controller instance
 * \param [in] bus_vol_vdd1     Bus voltage for Vdd1
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_set_bus_vol_vdd1(HW_SDHC_ID id, uint8_t bus_vol_vdd1);

/**
 * \brief Check if the block size used in the data transaction is valid
 *
 * \note In order to check the block size, the CSD should be read. This can be done in higher layers.
 *
 * \param [in] id               SDHC controller instance
 * \param [in] xfer_dir         Data transfer direction i.e. read or write
 * \param [in] block_size       Block size, e.g 512 Bytes
 *
 * \return true if valid.
 */
static bool hw_emmc_is_block_size_valid(HW_SDHC_ID id, HW_SDHC_XFER_MODE_R_DATA_XFER_DIR xfer_dir, uint16_t block_size);

/**
 * \brief Reset context values of eMMC controller
 *
 * \param [in] id               SDHC controller instance
 */
static void hw_emmc_reset_context(HW_SDHC_ID id);

/**
 * \brief eMMC Data transfer abort implementation
 *
 * \param [in] id               SDHC controller instance
 * \param [in] tout_ms          timeout, in ms
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_abort_xfer_impl(HW_SDHC_ID id, uint32_t tout_ms);

/**
 * \brief Calculate CRC7 for one byte
 *
 * It's a 7 bit CRC with polynomial x**7 + x**3 + 1
 *
 * \param [in] crc              the CRC from previous step (0 for first step)
 * \param [in] data             the byte for CRC calculation
 *
 * \return new CRC7
 */
static uint8_t hw_emmc_CRC7_one(uint8_t crc, uint8_t data);

/**
 * \brief Calculate CRC7 value of the buffer
 *
 * \param [in] buf              pointer to buffer
 * \param [in] len              buffer length
 *
 * \return CRC7 of buffer
 */
static uint8_t hw_emmc_CRC7_buf(uint8_t *buf, uint8_t len);

/**
 * \brief Copy bytes of a buffer in reverse order
 *
 * \param [out] dst_buf         pointer to destination buffer
 * \param [in] src_buf          pointer to source buffer
 * \param [in] num              number of bytes to copy, should be a positive number
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_copy_bytes_in_reverse_order(uint8_t *dst_buf, const uint8_t *src_buf, uint32_t num);

/**
 * \brief Prepare the contents of the buffer to program the CID register, reverse byte order and add CRC7
 *
 * \param [out] cid_buf         pointer to buffer used to program CID register
 * \param [in] new_cid_val      pointer to the new value of the CID register
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_prepare_buffer_to_program_cid_register(uint8_t *cid_buf, const uint8_t *new_cid_val);

/**
 * \brief Prepare the contents of the buffer to program the CSD register, reverse byte order and add CRC7
 *
 * \param [out] csd_buf         pointer to buffer used to program CSD register
 * \param [in] new_csd_val      the new value of the CSD programmable byte
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_prepare_buffer_to_program_csd_register(uint8_t *csd_buf, uint8_t new_csd_val);

/**
 * \brief Send erase commands CMD35, 36 and 38
 *
 * \param [in] id               SDHC controller instance
 * \param [in] start_addr       start block address, valid values = 0..SEC_COUNT-1
 * \param [in] end_addr         end block address, valid values = 0..SEC_COUNT-1
 *                              start_addr cannot be greater than the end_addr
 *                              EXT_CSD[215:212] = SEC_COUNT = max sector count of the device
 * \param [in] tout_ms          operation timeout in msec
 * \param [in] arg              arg defines the specific erase operation: ERASE, SECURE ERASE, TRIM, SECURE TRIM
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_erase(HW_SDHC_ID id, uint32_t start_addr, uint32_t end_addr, uint32_t tout_ms, HW_SDHC_CMD38_ARG arg);

/**
 * \brief Send CMD42 and check specified bits in card status register
 *
 * \param [in] id               SDHC controller instance
 * \param [in] len              cmd42 data length
 * \param [in] data             pointer to cmd42 data bytes
 * \param [in] status_mask      check if these status bits are set in card status register
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_send_CMD42_and_check_status(HW_SDHC_ID id, uint8_t len, uint8_t *data, uint32_t status_mask);

/**
 * \brief Check card registers CSD and EXT_CSD regarding the supported version and features
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_check_card_versions(void);

/**
 * \brief Set card access data from registers CSD and EXT_CSD
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_emmc_set_access_data(void);

/***************************************************************************************************
 *
 * Implementation of API - Public functions
 *
 **************************************************************************************************/

HW_SDHC_STATUS hw_emmc_enable(HW_SDHC_ID id, const hw_sdhc_pdctrl_reg_config_t *config)
{
        if (HW_EMMCC != id || !config ||
            !hw_sdhc_assert_clk_div(id, config->clk_div)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (!hw_pd_check_ctrl_status()) {
                return HW_SDHC_STATUS_ERROR;
        }

        REG_SETF(CRG_CTRL, CLK_PDCTRL_REG, EMMC_INV_TX_CLK, config->inv_tx_clk);
        REG_SETF(CRG_CTRL, CLK_PDCTRL_REG, EMMC_INV_RX_CLK, config->inv_rx_clk);
        REG_SETF(CRG_CTRL, CLK_PDCTRL_REG, EMMC_CLK_DIV, config->clk_div);
        // Enable eMMC clock
        REG_SETF(CRG_CTRL, CLK_PDCTRL_REG, EMMC_ENABLE, 1);

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_emmc_hw_reset_card(HW_SDHC_ID id, uint32_t rst_pulse_us)
{
        if ((HW_EMMCC != id) || rst_pulse_us < HW_EMMC_HW_RESET_PULSE_MIN_US) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        hw_sdhc_set_emmc_ctrl_r_emmc_rst_n_oe(id, true);

        /* Wait RST_n pulse width of 1 us */
        hw_sdhc_set_emmc_ctrl_r_emmc_rst_n(id, false);
        hw_clk_delay_usec(rst_pulse_us);

        /* Wait RST_n to Command time (74 cycles of clock signal required
         * before issuing CMD1 or CMD0 with argument 0xFFFFFFFA)
         */
        hw_sdhc_set_emmc_ctrl_r_emmc_rst_n(id, true);
        hw_clk_delay_usec(HW_EMMC_HW_RESET_AFTER_DELAY_US);

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_emmc_init(HW_SDHC_ID id, const hw_sdhc_config_t *config, hw_sdhc_event_callback_t cb, const hw_emmc_context_data_t **ptr_emmc_context)
{
        HW_SDHC_STATUS ret;
        bool card_is_locked = false;

        if ((HW_EMMCC != id) || !config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (HW_SDHC_STATE_FREE != sdhc_context.state) {
                /* Driver is already in use! */
                return HW_SDHC_STATUS_ERROR_STATE_NOT_FREE;
        }

        hw_emmc_reset_context(id);
        /* Register eMMC driver context to SDHC before using it */
        ret = hw_sdhc_register_context(id, &sdhc_context);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        /* Set the masks with active and implemented interrupts, normal and error */
        hw_sdhc_set_active_interrupts_mask(id, HW_EMMC_ACTIVE_NORMAL_INTERRUPTS_MASK, HW_EMMC_ACTIVE_ERROR_INTERRUPTS_MASK);

        /*
         * Host Controller should be enabled before following assertions
         *
         * To be more specific, hw_sdhc_assert_bus_speed() calls hw_sdhc_get_capabilities1_r_base_clk_freq()
         */
        if (!hw_sdhc_assert_bus_width_and_speed_mode(id, config->bus_config.bus_width, config->bus_config.speed_mode) ||
            !hw_sdhc_assert_bus_speed(id, config->bus_config.bus_speed) ||
            !hw_sdhc_assert_bus_speed_and_speed_mode(id, config->bus_config.bus_speed, config->bus_config.speed_mode)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }


        /* Initialize eMMC driver context before using it */
        sdhc_context.state = HW_SDHC_STATE_IDLE;
        sdhc_context.cb = cb;

        /* Start initializations... */
        ret = hw_emmc_hc_setup(id, &config->hc_setup);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_card_interface_setup(id, hw_clk_get_sysclk_freq() / HW_SDHC_CLK_DIV_MAX); // Should be <= 400kHz, but works at 10MHz
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_card_initializing_and_identifying(id, config->bus_config);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_setup_data_bus(id, &config->bus_config);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_TRAN);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        if (hw_sdhc_get_resp01_r(id) & HW_SDHC_CARD_STATUS_CARD_IS_LOCKED) {
                card_is_locked = true;
        }

        ret = hw_sdhc_emmc_send_ext_csd_CMD8(id, emmc_context.rca, (uint8_t *)&emmc_context.ext_csd);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_check_card_versions();
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_set_access_data();
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        *ptr_emmc_context = &emmc_context;

        if (card_is_locked) {
                return HW_SDHC_STATUS_ERROR_CARD_STATUS_CARD_IS_LOCKED;
        }

        return ret;
}

static HW_SDHC_STATUS hw_emmc_check_card_versions(void)
{
        /* Check version */
        if (3 == emmc_context.csd.csd_structure) {
                if (emmc_context.ext_csd.csd_structure > 2) {
                        /* unrecognized EXT_CSD structure */
                        return HW_SDHC_STATUS_ERROR_CARD_REG_VAL_NOT_RECOGNIZED;
                }
        }

        if (emmc_context.ext_csd.ext_csd_rev > 8) {
                /* unrecognized EXT_CSD revision */
                return HW_SDHC_STATUS_ERROR_CARD_REG_VAL_NOT_RECOGNIZED;
        }

        if (emmc_context.ext_csd.ext_csd_rev >= 2) {
                /* Cards with density <= 2GB are byte addressed */
                if (emmc_context.ext_csd.sec_count <= ((uint32_t)2 * 1024 * 1024 * 1024) / 512) {
                        return HW_SDHC_STATUS_ERROR_CARD_REG_VAL_NOT_RECOGNIZED;
                }
        }

        if (emmc_context.ext_csd.ext_csd_rev >= 3) {
        }

        if (emmc_context.ext_csd.ext_csd_rev >= 4) {
        }

        if (emmc_context.ext_csd.ext_csd_rev >= 5) {
        }

        return HW_SDHC_STATUS_SUCCESS;
}

static HW_SDHC_STATUS hw_emmc_set_access_data(void)
{
        if ((emmc_context.ext_csd.s_a_timeout == 0) || (emmc_context.ext_csd.s_a_timeout > 0x17) ) {
                return HW_SDHC_STATUS_ERROR_CARD_REG_VAL_NOT_RECOGNIZED;
        }
        emmc_context.card_access_data.s_a_timeout_usec =
                ((100 * (1 << (emmc_context.ext_csd.s_a_timeout)) / 1000 ) + 1);        // round-up in next usec


        // TAAC time unit
        const uint32_t taac_ns[] = {
                1,              // 1ns
                10,             // 10ns
                100,            // 100ns
                1000,           // 1us
                10000,          // 10us
                100000,         // 100us
                1000000,        // 1ms
                10000000        // 10ms
        };

        // TAAC multiplier factor x 10
        const uint32_t taac_mult_x10[] = {
                0,              // RSVD
                10,             // 1.0 x 10
                12,             // 1.2 x 10
                13,             // 1.3 x 10
                15,             // 1.5 x 10
                20,             // 2.0 x 10
                25,             // 2.5 x 10
                30,             // 3.0 x 10
                35,             // 3.5 x 10
                40,             // 4.0 x 10
                45,             // 4.5 x 10
                50,             // 5.0 x 10
                55,             // 5.5 x 10
                60,             // 6.0 x 10
                70,             // 7.0 x 10
                80              // 8.0 x 10
        };

        const uint32_t hw_emmc_tout_access_mult = 10;

        // Read block delay = 10 x TAAC + 100 x NSAC / Fop ~= 10 x TAAC
        if (0 == (emmc_context.csd.taac >> 3)) {
                return HW_SDHC_STATUS_ERROR_CARD_REG_VAL_NOT_RECOGNIZED;
        }
        uint32_t nsac_ms = 1000 * (100 * emmc_context.csd.nsac) / emmc_context.card_access_data.bus_speed;
        uint32_t taac_ms = taac_ns[emmc_context.csd.taac & 0x07] * taac_mult_x10[emmc_context.csd.taac >> 3] / 1000000;
        uint32_t read_access_time_ms = taac_ms + nsac_ms;

        uint32_t write_block_time_ms = (1 << emmc_context.csd.r2w_factor) * read_access_time_ms;

        emmc_context.card_access_data.read_timeout_ms = hw_emmc_tout_access_mult * read_access_time_ms;
        emmc_context.card_access_data.write_timeout_ms = hw_emmc_tout_access_mult * write_block_time_ms;

        if (!emmc_context.ext_csd.sec_erase_mult ||
            !emmc_context.ext_csd.trim_mult ||
            !emmc_context.ext_csd.sec_trim_mult) {
                return HW_SDHC_STATUS_ERROR_CARD_REG_VAL_NOT_RECOGNIZED;
        }

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_emmc_deinit(HW_SDHC_ID id)
{
        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        HW_SDHC_STATUS ret = hw_emmc_stop_hc_clocks(id);

        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        sdhc_context.state = HW_SDHC_STATE_FREE;

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_emmc_disable(const HW_SDHC_ID id)
{
        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        // Disable eMMC clock
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_CTRL, CLK_PDCTRL_REG, EMMC_ENABLE, 0);
        GLOBAL_INT_RESTORE();

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_emmc_is_busy(HW_SDHC_ID id)
{
        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (hw_sdhc_is_busy(id)) {
                return HW_SDHC_STATUS_ERROR_OPERATION_IN_PROGRESS;
        }

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_emmc_data_xfer(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config)
{
        HW_SDHC_STATUS ret;

        if ( (HW_EMMCC != id) || (NULL == config) || (0 == config->block_cnt) || (0 == config->block_size) ||
             (config->address >= emmc_context.ext_csd.sec_count) ||
             (config->block_size > (HW_SDHC_DEFAULT_BLOCK_SIZE << hw_sdhc_get_capabilities1_r_max_blk_len(id))) ) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if ((config->block_size * config->block_cnt / HW_SDHC_DEFAULT_BLOCK_SIZE) > (emmc_context.ext_csd.sec_count - config->address)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (!config->bus_testing) {
                if ( !hw_emmc_is_block_size_valid(id, config->xfer_dir, config->block_size) ) {
                        return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
                }
        } else {
                if (!((config->block_size == 1) || (config->block_size == 4) || (config->block_size == 8))) {
                        return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
                }
        }

        /* Verify that no data xfer is active */
        if (hw_sdhc_is_busy(id)) {
                return HW_SDHC_STATUS_ERROR_OPERATION_IN_PROGRESS;
        }

        // InitDataTransfer: set related registers...
        ret = hw_sdhc_data_xfer_init(id, config);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Ready to send command...
        ret = hw_sdhc_data_xfer_send_cmd(id, config);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                // Reset CMD and DAT lines
                hw_sdhc_set_and_wait_sw_rst_dat(id);
                hw_sdhc_set_and_wait_sw_rst_cmd(id);
                return ret;
        }

        if (!config->dma_en) {
                if (!config->intr_en) {
                        ret = hw_sdhc_data_xfer_start_non_dma_blocking(id, config);
                } else {
                        ret = hw_sdhc_data_xfer_start_non_dma_non_blocking(id, config);
                }
        } else {
                if (!config->intr_en) {
                        ret = hw_sdhc_data_xfer_start_dma_blocking(id, config);
                } else {
                        ret = hw_sdhc_data_xfer_start_dma_non_blocking(id, config);
                }
        }

        if (HW_SDHC_STATUS_SUCCESS != ret) {
                // Reset CMD and DAT lines
                hw_sdhc_set_and_wait_sw_rst_dat(id);
                hw_sdhc_set_and_wait_sw_rst_cmd(id);
        }

        return ret;
}

HW_SDHC_STATUS hw_emmc_error_recovery(HW_SDHC_ID id, uint32_t tout_ms)
{
        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        return hw_sdhc_error_recovery(id, tout_ms);
}

HW_SDHC_STATUS hw_emmc_abort_xfer(HW_SDHC_ID id, HW_SDHC_ABORT_METHOD abort_method, uint32_t tout_ms)
{
        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (!hw_sdhc_is_busy(id)) {
                return HW_SDHC_STATUS_SUCCESS;
        }

        if (HW_SDHC_ABORT_METHOD_SYNC == abort_method) {
                return hw_sdhc_abort_xfer_sync(id, tout_ms);
        }

        if (hw_sdhc_get_pstate_cmd_inhibit(id)) {
                return HW_SDHC_STATUS_ERROR;
        }

        return hw_sdhc_abort_xfer_async(id, tout_ms);
}

static HW_SDHC_STATUS hw_emmc_abort_xfer_impl(HW_SDHC_ID id, uint32_t tout_ms)
{
        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        hw_sdhc_set_and_wait_sw_rst_dat(id);
        hw_sdhc_set_and_wait_sw_rst_cmd(id);

        for (uint32_t i = 0; i < HW_EMMC_ABORT_TRANSFER_TRIES; i++) {
                // If card is already in Transfer state, CMD12 is not accepted
                // Therefore, ignore return value from CMD12

                bool hpi = false;

                hw_sdhc_stop_transmission_CMD12(id, emmc_context.rca, hpi, tout_ms);

                HW_SDHC_STATUS ret = hw_emmc_check_card_is_in_state(id,
                                                HW_SDHC_CARD_STATUS_CURRENT_STATE_TRAN);
                if (HW_SDHC_STATUS_SUCCESS == ret) {
                        return HW_SDHC_STATUS_SUCCESS;
                }
        }

        return HW_SDHC_STATUS_ERROR;
}

HW_SDHC_STATUS hw_emmc_get_card_cid(HW_SDHC_ID id)
{
        HW_SDHC_STATUS ret;
        hw_sdhc_emmc_cid_t tmp_cid;

        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (HW_SDHC_STATUS_SUCCESS != hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_STBY)) {
                /* De-select the card so should not check the return value */
                hw_sdhc_select_deselect_card_CMD7(id, HW_EMMC_CARD_RCA_DESELECT_VAL, false, 0);

                ret = hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_STBY);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
        }

        ret = hw_sdhc_send_cid_CMD10(id, emmc_context.rca, (uint32_t *)&tmp_cid);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_sdhc_select_deselect_card_CMD7(id, emmc_context.rca, false, 0);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_TRAN);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Reset CMD and DAT lines
        hw_sdhc_set_and_wait_sw_rst_dat(id);
        hw_sdhc_set_and_wait_sw_rst_cmd(id);

        // The address of the local CID data is returned
        emmc_context.cid = tmp_cid;

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_emmc_program_cid(HW_SDHC_ID id, const hw_sdhc_emmc_cid_t *prg_cid)
{
        HW_SDHC_STATUS ret;

        if ((HW_EMMCC != id) || (NULL == prg_cid)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        uint8_t cid_buf[HW_SDHC_CID_SIZE] = { 0 };

        ret = hw_emmc_prepare_buffer_to_program_cid_register(cid_buf, (uint8_t *)prg_cid);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret =  hw_sdhc_program_cid_CMD26(id, cid_buf, emmc_context.card_access_data.write_timeout_ms);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        /*
         * Although the command PROGRAM_CID returns success, the card might not be programmed
         * Therefore, the card status register should be checked first, before updating the eMMC context
         */
        ret = hw_emmc_check_card_status_register(id, HW_SDHC_CARD_STATUS_CID_CSD_OVERWRITE | HW_SDHC_CARD_STATUS_CARD_IS_LOCKED);
        if (HW_SDHC_STATUS_SUCCESS == ret) {
                // Update emmc_context.cid
                emmc_context.cid = *prg_cid;
        }

        if (ret == HW_SDHC_STATUS_ERROR_CARD_STATUS_ERRORS) {
                return HW_SDHC_STATUS_ERROR_CARD_STATUS_CID_CSD_OVRWR;
        }
        return ret;
}

HW_SDHC_STATUS hw_emmc_get_card_csd(HW_SDHC_ID id)
{
        HW_SDHC_STATUS ret;
        hw_sdhc_emmc_csd_t tmp_csd;

        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (HW_SDHC_STATUS_SUCCESS != hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_STBY)) {
                /* De-select the card so should not check the return value */
                hw_sdhc_select_deselect_card_CMD7(id, HW_EMMC_CARD_RCA_DESELECT_VAL, false, 0);

                ret = hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_STBY);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
        }

        ret = hw_sdhc_send_csd_CMD9(id, emmc_context.rca, (uint32_t *)&tmp_csd);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_sdhc_select_deselect_card_CMD7(id, emmc_context.rca, false, 0);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_TRAN);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Reset CMD and DAT lines
        hw_sdhc_set_and_wait_sw_rst_dat(id);
        hw_sdhc_set_and_wait_sw_rst_cmd(id);

        emmc_context.csd = tmp_csd;

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_emmc_program_csd(HW_SDHC_ID id, const hw_emmc_prg_csd_t prg_csd)
{
        HW_SDHC_STATUS ret;

        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        _Static_assert(sizeof(hw_emmc_prg_csd_t) == sizeof(uint8_t), "Invalid size of hw_emmc_prg_csd_t!");

        if (prg_csd.prg_csd.perm_write_protect) {
                // If this bit is set at CSD register then the card is PERMANENTLY write protected
                ASSERT_WARNING(0);
        }

        uint8_t csd_buf[HW_SDHC_CSD_SIZE] = { 0 };
        ret = hw_emmc_prepare_buffer_to_program_csd_register(csd_buf, prg_csd.prg_csd_val);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret =  hw_sdhc_program_csd_CMD27(id, csd_buf, emmc_context.card_access_data.write_timeout_ms);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        /*
         * Although the command PROGRAM_CSD returns success, the card might not be programmed
         * Therefore, the card status register should be checked first, before updating the eMMC context
         */
        ret = hw_emmc_check_card_status_register(id, HW_SDHC_CARD_STATUS_CID_CSD_OVERWRITE | HW_SDHC_CARD_STATUS_CARD_IS_LOCKED);
        if (HW_SDHC_STATUS_SUCCESS == ret) {
                // Update programmable part of emmc_context.csd, which is the LSB
                uint8_t *p_csd = (uint8_t *)&emmc_context.csd;
                *p_csd = prg_csd.prg_csd_val;
        }

        return ret;
}

HW_SDHC_STATUS hw_emmc_get_card_ext_csd(HW_SDHC_ID id)
{
        HW_SDHC_STATUS ret;
        hw_sdhc_emmc_ext_csd_t tmp_ext_csd;

        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        _Static_assert( sizeof(hw_sdhc_emmc_ext_csd_t) == HW_SDHC_EXT_CSD_SIZE * sizeof(uint8_t), "Invalid size of hw_sdhc_emmc_ext_csd_t!");

        ret = hw_sdhc_emmc_send_ext_csd_CMD8(id, emmc_context.rca, (uint8_t *)&tmp_ext_csd);
        if (HW_SDHC_STATUS_SUCCESS == ret) {
                emmc_context.ext_csd = tmp_ext_csd;
        }

        return ret;
}

HW_SDHC_STATUS hw_emmc_get_card_status_register(HW_SDHC_ID id, uint32_t *status_reg)
{
        if ( (HW_EMMCC != id) || (NULL == status_reg) ) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        bool hpi = false;
        return hw_sdhc_send_status_CMD13(id, emmc_context.rca, hpi, status_reg);
}

/***************************************************************************************************
 *
 * Implementation of Private functions
 *
 **************************************************************************************************/

static HW_SDHC_STATUS hw_emmc_hc_setup(HW_SDHC_ID id, const hw_sdhc_hc_setup_config_t *config)
{
        HW_SDHC_STATUS ret;

        ret = hw_emmc_set_bus_vol_vdd1(id, config->bus_vol_vdd1);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        if (config->tout_cnt == HW_SDHC_TOUT_CNT_INVALID) {
                hw_sdhc_timeout_setting(id, config->tout);
        } else {
                hw_sdhc_set_tout_ctrl_r_tout_cnt(id, config->tout_cnt);
        }
        hw_sdhc_set_host_ctrl2_r_uhs2_if_enable(id, false);
        hw_sdhc_set_emmc_ctrl_r_card_is_emmc(id, true);

        ret = hw_sdhc_internal_clk_enable(id);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Set Version 4 Parameters
        hw_sdhc_set_host_ctrl2_r_host_ver4_enable(id, true);
        if (hw_sdhc_get_capabilities1_r_sys_addr_64_v4(id) == true) {
                // 32-bit addressing is supported
                hw_sdhc_set_host_ctrl2_r_addressing(id, true);
        }

        return HW_SDHC_STATUS_SUCCESS;
}

static HW_SDHC_STATUS hw_emmc_card_interface_setup(HW_SDHC_ID id, uint32_t frequency)
{
        HW_SDHC_STATUS ret;

        // Apply power to the bus
        hw_sdhc_set_host_ctrl2_r_uhs2_if_enable(id, false);
        hw_sdhc_set_pwr_ctrl_r_sd_bus_pwr_vdd1(id, true);
        hw_sdhc_set_host_ctrl2_r_uhs_mode_sel(id, HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_LEGACY);  // default value

        hw_sdhc_set_host_ctrl1_r_ext_dat_xfer(id, HW_SDHC_EXT_DAT_XFER_DEFAULT);
        hw_sdhc_set_bus_width_at_host(id, HW_SDHC_DAT_XFER_WIDTH_1BIT);

        ret = hw_sdhc_set_frequency(id, frequency);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        hw_sdhc_wait_power_ramp_up(id, frequency);

        return HW_SDHC_STATUS_SUCCESS;
}

static HW_SDHC_STATUS hw_emmc_check_card_is_in_state(HW_SDHC_ID id, HW_SDHC_CARD_STATUS_CURRENT_STATE state)
{
        HW_SDHC_STATUS ret;
        uint32_t response = 0;

        ret = hw_emmc_get_card_status_register(id, &response);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        if (state != ((response >> HW_SDHC_CARD_STATUS_CURRENT_STATE_POS) & HW_SDHC_CARD_STATUS_CURRENT_STATE_MASK)) {
                return HW_SDHC_STATUS_ERROR;
        }

        return HW_SDHC_STATUS_SUCCESS;
}

static HW_SDHC_STATUS hw_emmc_check_card_status_register(HW_SDHC_ID id, uint32_t status_mask)
{
        HW_SDHC_STATUS ret;
        uint32_t status_reg;

        ret = hw_emmc_get_card_status_register(id, &status_reg);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        if (status_reg & status_mask) {
                // CARD_IS_LOCKED is checked with highest priority
                if (status_reg & (status_mask & HW_SDHC_CARD_STATUS_CARD_IS_LOCKED)) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_CARD_IS_LOCKED;
                }

                return HW_SDHC_STATUS_ERROR_CARD_STATUS_ERRORS;
        }

        return HW_SDHC_STATUS_SUCCESS;
}

static HW_SDHC_STATUS hw_emmc_card_initializing_and_identifying(HW_SDHC_ID id, const hw_sdhc_bus_config_t bus_config)
{
        HW_SDHC_STATUS ret;

        ret = hw_sdhc_go_idle_state_CMD0(id);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }
        hw_sdhc_set_and_wait_sw_rst_dat(id);
        hw_sdhc_set_and_wait_sw_rst_cmd(id);

        // eMMC power-up
        uint32_t ocr_reg;
        ret = hw_sdhc_send_op_cond_CMD1(id, &ocr_reg, HW_SDHC_CMD1_VOLTAGE_WINDOW);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        if ((ocr_reg & ~HW_SDHC_CMD1_OCR_BUSY_MASK) != HW_SDHC_CMD1_VOLTAGE_WINDOW) {
                // Device is not compliant.
                // Power down the bus.
                return HW_SDHC_STATUS_ERROR_UNUSABLE_CARD;
        }

        /* Switch the bus to 1.8 V */
        hw_sdhc_set_host_ctrl2_r_signaling_en(id, true);


        _Static_assert( sizeof(hw_sdhc_emmc_cid_t) == (HW_SDHC_CID_SIZE - 1) * sizeof(uint8_t), "Invalid size of hw_sdhc_emmc_cid_t!");

        ret = hw_sdhc_all_send_cid_CMD2(id, (uint32_t *)&emmc_context.cid);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_sdhc_set_relative_address_CMD3(id, emmc_context.rca);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_STBY);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        _Static_assert( sizeof(hw_sdhc_emmc_csd_t) == (HW_SDHC_CSD_SIZE - 1) * sizeof(uint8_t), "Invalid size of hw_sdhc_emmc_csd_t!");

        ret = hw_sdhc_send_csd_CMD9(id, emmc_context.rca, (uint32_t *)&emmc_context.csd);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        if (bus_config.dsr_req && emmc_context.csd.dsr_imp) {
                ret = hw_sdhc_set_dsr_CMD4(id, bus_config.dsr);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
        }

        ret = hw_sdhc_select_deselect_card_CMD7(id, emmc_context.rca, false, 0);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        return hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_TRAN);
}

HW_SDHC_STATUS hw_emmc_sleep(HW_SDHC_ID id, uint32_t tout_ms)
{
        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (HW_SDHC_STATUS_SUCCESS != hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_STBY)) {
                /* De-select the card so should not check the return value */
                hw_sdhc_select_deselect_card_CMD7(id, HW_EMMC_CARD_RCA_DESELECT_VAL, false, 0);

                HW_SDHC_STATUS ret = hw_emmc_check_card_is_in_state(id,
                                                HW_SDHC_CARD_STATUS_CURRENT_STATE_STBY);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
        }

        // Reset CMD and DAT lines
        hw_sdhc_set_and_wait_sw_rst_dat(id);
        hw_sdhc_set_and_wait_sw_rst_cmd(id);

        if (!tout_ms) {
                // Use max timeout value
                tout_ms = emmc_context.card_access_data.s_a_timeout_usec / 1000;
        }

        return hw_sdhc_emmc_sleep_awake_CMD5(id, emmc_context.rca, true, tout_ms);
}

HW_SDHC_STATUS hw_emmc_awake(HW_SDHC_ID id, uint32_t tout_ms)
{
        HW_SDHC_STATUS ret;

        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (!tout_ms) {
                // Use max timeout value
                tout_ms = emmc_context.card_access_data.s_a_timeout_usec / 1000;
        }

        ret = hw_sdhc_emmc_sleep_awake_CMD5(id, emmc_context.rca, false, tout_ms);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        if (HW_SDHC_STATUS_SUCCESS == hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_STBY)) {
                ret = hw_sdhc_select_deselect_card_CMD7(id, emmc_context.rca, false, 0);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }

                ret = hw_emmc_check_card_is_in_state(id, HW_SDHC_CARD_STATUS_CURRENT_STATE_TRAN);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
        }

        // Reset CMD and DAT lines
        hw_sdhc_set_and_wait_sw_rst_dat(id);
        hw_sdhc_set_and_wait_sw_rst_cmd(id);

        return ret;
}

HW_SDHC_STATUS hw_emmc_start_hc_clocks(HW_SDHC_ID id)
{
        HW_SDHC_STATUS ret;

        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        ret = hw_sdhc_internal_clk_enable(id);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }
        hw_sdhc_set_clk_ctrl_r_sd_clk_en(id, true);

        // SW Reset to avoid the effect of any glitch on sampling clock
        hw_sdhc_set_and_wait_sw_rst_dat(id);
        hw_sdhc_set_and_wait_sw_rst_cmd(id);

        return ret;
}

HW_SDHC_STATUS hw_emmc_stop_hc_clocks(HW_SDHC_ID id)
{
        HW_SDHC_STATUS ret;

        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        // Reset CMD and DAT lines
        hw_sdhc_set_and_wait_sw_rst_dat(id);
        hw_sdhc_set_and_wait_sw_rst_cmd(id);

        ret = hw_sdhc_stop_sd_clock(id);

        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        hw_sdhc_set_clk_ctrl_r_pll_enable(id, false);
        hw_sdhc_set_clk_ctrl_r_internal_clk_en(id, false);

        return ret;
}

HW_SDHC_STATUS  hw_emmc_set_data_bus_width(HW_SDHC_ID id, HW_SDHC_BUS_WIDTH bus_width)
{
        HW_SDHC_STATUS ret;

        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        ret = hw_sdhc_set_emmc_data_bus_width_CMD6(id, bus_width, emmc_context.card_access_data.write_timeout_ms);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_check_card_status_register(id, HW_SDHC_CARD_STATUS_SWITCH_ERROR | HW_SDHC_CARD_STATUS_CARD_IS_LOCKED);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        emmc_context.ext_csd.bus_width = bus_width;
        return ret;
}

HW_SDHC_STATUS hw_emmc_set_speed_mode(HW_SDHC_ID id, uint8_t speed_mode)
{
        HW_SDHC_STATUS ret;

        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        uint8_t hs_timing = 0;

        if (emmc_context.csd.spec_ver == 0x04) {
                switch (speed_mode) {
                case HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_LEGACY:
                case HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_HS_SDR:
                        hs_timing = 1;
                        break;
                default:
                        return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
                }
        }

        ret = hw_sdhc_set_emmc_speed_mode_CMD6(id, speed_mode, hs_timing, emmc_context.card_access_data.write_timeout_ms);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = hw_emmc_check_card_status_register(id, HW_SDHC_CARD_STATUS_SWITCH_ERROR | HW_SDHC_CARD_STATUS_CARD_IS_LOCKED);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        emmc_context.ext_csd.hs_timing = hs_timing;
        return ret;
}


HW_SDHC_STATUS hw_emmc_setup_data_bus(HW_SDHC_ID id, const hw_sdhc_bus_config_t *bus_config)
{
        HW_SDHC_STATUS ret;

        if ((HW_EMMCC != id) || (NULL == bus_config)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        /*
         * NOTE: order of the following two calls is important!!!
         */
#if (HW_SDHC_SUPPORT_DDR == 1)
        if (HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_HS_DDR == bus_config->speed_mode)
        {
                // This implementation does not support DDR mode
                ASSERT_WARNING(0);

                /* Change HS_TIMING first when DDR mode */

                ret = hw_emmc_set_speed_mode(id, bus_config->speed_mode);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }

                ret = hw_emmc_set_data_bus_width(id, bus_config->bus_width);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
        } else
#endif
        {
                ret = hw_emmc_set_data_bus_width(id, bus_config->bus_width);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }

                ret = hw_emmc_set_speed_mode(id, bus_config->speed_mode);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
        }

        // Change CLK_PDCTRL_REG.EMMC_CLK_DIV...
        ret = hw_sdhc_set_frequency(id, bus_config->bus_speed);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }
        emmc_context.card_access_data.bus_speed = bus_config->bus_speed;

        hw_sdhc_set_host_ctrl2_r_drv_strength_sel(id, bus_config->drv_strength);
        return HW_SDHC_STATUS_SUCCESS;
}

static HW_SDHC_STATUS hw_emmc_erase(HW_SDHC_ID id, uint32_t start_addr, uint32_t end_addr, uint32_t tout_ms, HW_SDHC_CMD38_ARG arg)
{
        HW_SDHC_STATUS ret;

        if (!(emmc_context.csd.ccc & HW_EMMC_CARD_CMD_CLASS_5_ERASE)) {
                // The card does not support erase commands class
                return HW_SDHC_STATUS_ERROR;
        }

        if ((HW_EMMCC != id) || (start_addr > end_addr)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        // Erase addresses should be within the memory range
        if (( start_addr >= emmc_context.ext_csd.sec_count) || (end_addr >= emmc_context.ext_csd.sec_count)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        ret = hw_sdhc_erase_group_start_CMD35(id, start_addr);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                // Should never happen because of the above assertions/checks
                return ret;
        }

        ret = hw_sdhc_erase_group_end_CMD36(id, end_addr);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                // Should never happen because of the above assertions/checks
                return ret;
        }

        return hw_sdhc_erase_CMD38(id, arg, tout_ms);
}

uint32_t hw_emmc_get_erase_group_size(void)
{
        return (emmc_context.ext_csd.erase_group_def & 0x01) && emmc_context.ext_csd.hc_erase_grp_size ?
                1024 * emmc_context.ext_csd.hc_erase_grp_size :
                (emmc_context.csd.erase_grp_size + 1) * (emmc_context.csd.erase_grp_mult + 1);
}

uint32_t hw_emmc_get_wp_group_size(void)
{
        return (emmc_context.ext_csd.erase_group_def & 0x01) && emmc_context.ext_csd.hc_wp_grp_size ?
                1024 * emmc_context.ext_csd.hc_erase_grp_size * emmc_context.ext_csd.hc_wp_grp_size :
                (emmc_context.csd.wp_grp_size + 1) * (emmc_context.csd.erase_grp_size + 1) * (emmc_context.csd.erase_grp_mult + 1);
}

uint32_t hw_emmc_get_erase_timeout_ms(void)
{
        return (emmc_context.ext_csd.erase_group_def & 0x01) && emmc_context.ext_csd.erase_timeout_mult ?
                HW_EMMC_HC_TIMEOUT_ERASE_FACTOR_MS * emmc_context.ext_csd.erase_timeout_mult :
                emmc_context.card_access_data.write_timeout_ms;
}

uint32_t hw_emmc_get_sec_erase_timeout_ms(void)
{
        return (emmc_context.ext_csd.erase_group_def & 0x01) ?
                HW_EMMC_HC_TIMEOUT_ERASE_FACTOR_MS * emmc_context.ext_csd.erase_timeout_mult * emmc_context.ext_csd.sec_erase_mult :
                emmc_context.card_access_data.write_timeout_ms;
}

uint32_t hw_emmc_get_trim_timeout_ms(void)
{
        return HW_EMMC_HC_TIMEOUT_ERASE_FACTOR_MS * emmc_context.ext_csd.trim_mult;
}

uint32_t hw_emmc_get_sec_trim_timeout_ms(void)
{
        return HW_EMMC_HC_TIMEOUT_ERASE_FACTOR_MS * emmc_context.ext_csd.erase_timeout_mult * emmc_context.ext_csd.sec_trim_mult;
}

HW_SDHC_STATUS hw_emmc_erase_groups(HW_SDHC_ID id, uint32_t start_erase_group, uint32_t end_erase_group, uint32_t tout_ms)
{
        // start/end_addr is the first address of the start/end_erase_group, correspondingly
        uint32_t start_addr = start_erase_group * hw_emmc_get_erase_group_size();
        uint32_t end_addr = end_erase_group * hw_emmc_get_erase_group_size();
        uint32_t tout = (tout_ms) ? tout_ms : (end_erase_group - start_erase_group + 1) * hw_emmc_get_erase_timeout_ms();

        return hw_emmc_erase(id, start_addr, end_addr, tout, HW_SDHC_CMD38_ARG_ERASE);
}

HW_SDHC_STATUS hw_emmc_erase_groups_secure(HW_SDHC_ID id, uint32_t start_erase_group, uint32_t end_erase_group, uint32_t tout_ms)
{
        if (!(emmc_context.ext_csd.sec_feature_support & BIT0)) {
                return HW_SDHC_STATUS_ERROR;
        }

        // start/end_addr is the first address of the start/end_erase_group, correspondingly
        uint32_t start_addr = start_erase_group * hw_emmc_get_erase_group_size();
        uint32_t end_addr = end_erase_group * hw_emmc_get_erase_group_size();
        uint32_t tout = (tout_ms) ? tout_ms : (end_erase_group - start_erase_group + 1) * hw_emmc_get_sec_erase_timeout_ms();

        return hw_emmc_erase(id, start_addr, end_addr, tout, HW_SDHC_CMD38_ARG_SECURE_ERASE);
}

HW_SDHC_STATUS hw_emmc_trim_blocks(HW_SDHC_ID id, uint32_t start_addr, uint32_t end_addr, uint32_t tout_ms)
{
        if (!(emmc_context.ext_csd.sec_feature_support & BIT4)) {
                return HW_SDHC_STATUS_ERROR;
        }

        uint32_t start_erase_grp = start_addr / hw_emmc_get_erase_group_size();
        uint32_t end_erase_grp = end_addr / hw_emmc_get_erase_group_size();
        uint32_t tout = (tout_ms) ? tout_ms : (end_erase_grp - start_erase_grp + 1) * hw_emmc_get_trim_timeout_ms();

        return hw_emmc_erase(id, start_addr, end_addr, tout, HW_SDHC_CMD38_ARG_TRIM);
}

HW_SDHC_STATUS hw_emmc_trim_mark_blocks_secure(HW_SDHC_ID id, uint32_t start_addr, uint32_t end_addr, uint32_t tout_ms)
{
        if (!(emmc_context.ext_csd.sec_feature_support & (BIT0 | BIT4))) {
                return HW_SDHC_STATUS_ERROR;
        }

        uint32_t start_erase_grp = start_addr / hw_emmc_get_erase_group_size();
        uint32_t end_erase_grp = end_addr / hw_emmc_get_erase_group_size();
        uint32_t tout = (tout_ms) ? tout_ms : (end_erase_grp - start_erase_grp + 1) * hw_emmc_get_sec_trim_timeout_ms();

        return hw_emmc_erase(id, start_addr, end_addr, tout, HW_SDHC_CMD38_ARG_SECURE_TRIM_STEP_1);
}

HW_SDHC_STATUS hw_emmc_trim_blocks_secure(HW_SDHC_ID id, uint32_t tout_ms)
{
        if (!tout_ms) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (!(emmc_context.ext_csd.sec_feature_support & (BIT0 | BIT4))) {
                return HW_SDHC_STATUS_ERROR;
        }

        // Call hw_emmc_erase() with any addresses that are in range. They are ignored, anyway.
        return hw_emmc_erase(id, 0x00000000, 0x00000001, tout_ms, HW_SDHC_CMD38_ARG_SECURE_TRIM_STEP_2);
}

static HW_SDHC_STATUS hw_emmc_send_CMD42_and_check_status(HW_SDHC_ID id, uint8_t len, uint8_t *data, uint32_t status_mask)
{
        HW_SDHC_STATUS ret;

        ret = hw_sdhc_set_blocklen_CMD16(id, len);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        uint32_t tout = ((*data) == HW_SDHC_CMD42_CMD_ERASE) && (len == 1) ?
                                HW_SDHC_TOUT_FORCE_ERASE_MS : emmc_context.card_access_data.write_timeout_ms;

        ret = hw_sdhc_lock_unlock_CMD42(id, len, data, tout);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        return hw_emmc_check_card_status_register(id, status_mask);
}

HW_SDHC_STATUS hw_emmc_card_set_password(HW_SDHC_ID id, const uint8_t *pwd, uint8_t len, bool lock)
{
        HW_SDHC_STATUS ret;

        if ((HW_EMMCC != id) || !pwd || !len || (len > HW_SDHC_CMD42_PWD_LEN_MAX)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        uint8_t cmd42[2 + HW_SDHC_CMD42_PWD_LEN_MAX] = { 0 };

        cmd42[0] = lock ? HW_SDHC_CMD42_CMD_LOCK : 0;
        cmd42[0] |= HW_SDHC_CMD42_CMD_SET_PWD;

        cmd42[1] = len;
        for (uint8_t i = 0; i < len; i++) {
                cmd42[2 + i] = pwd[i];
        }

        ret = hw_emmc_send_CMD42_and_check_status(id, cmd42[1] + 2, cmd42, HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED | HW_SDHC_CARD_STATUS_CARD_IS_LOCKED);

        if (lock && (HW_SDHC_STATUS_ERROR_CARD_STATUS_CARD_IS_LOCKED == ret)) {
                return HW_SDHC_STATUS_SUCCESS;
        }

        return ret;
}

HW_SDHC_STATUS hw_emmc_card_clr_password(HW_SDHC_ID id, const uint8_t *pwd, uint8_t len)
{
        if ((HW_EMMCC != id) || !pwd || !len || (len > HW_SDHC_CMD42_PWD_LEN_MAX)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        uint8_t cmd42[2 + HW_SDHC_CMD42_PWD_LEN_MAX] = { 0 };

        cmd42[0] = HW_SDHC_CMD42_CMD_CLR_PWD;

        cmd42[1] = len;
        for (uint8_t i = 0; i < len; i++) {
                cmd42[2 + i] = pwd[i];
        }

        HW_SDHC_STATUS ret = hw_emmc_send_CMD42_and_check_status(id, cmd42[1] + 2, cmd42, HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED);
        if (ret == HW_SDHC_STATUS_ERROR_CARD_STATUS_ERRORS) {
                return HW_SDHC_STATUS_ERROR_CARD_STATUS_LOCK_UNLOCK_FAIL;
        }
        return ret;
}

HW_SDHC_STATUS hw_emmc_card_replace_password(HW_SDHC_ID id, const uint8_t *old_pwd, uint8_t old_len, const uint8_t *new_pwd, uint8_t new_len, bool lock)
{
        HW_SDHC_STATUS ret;

        if ((HW_EMMCC != id) || !old_pwd || !old_len || (old_len > HW_SDHC_CMD42_PWD_LEN_MAX) ||
                                !new_pwd || !new_len || (new_len > HW_SDHC_CMD42_PWD_LEN_MAX) ) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        uint8_t cmd42[HW_SDHC_CMD42_LEN_MAX] = { 0 };

        cmd42[0] = lock ? HW_SDHC_CMD42_CMD_LOCK : 0;
        cmd42[0] |= HW_SDHC_CMD42_CMD_SET_PWD;

        cmd42[1] = old_len + new_len;
        for (uint8_t i = 0; i < old_len; i++) {
                cmd42[2 + i] = old_pwd[i];
        }
        for (uint8_t i = 0; i < new_len; i++) {
                cmd42[2 + old_len + i] = new_pwd[i];
        }

        ret = hw_emmc_send_CMD42_and_check_status(id, cmd42[1] + 2, cmd42, HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED | HW_SDHC_CARD_STATUS_CARD_IS_LOCKED);

        if (lock && (HW_SDHC_STATUS_ERROR_CARD_STATUS_CARD_IS_LOCKED == ret)) {
                return HW_SDHC_STATUS_SUCCESS;
        }

        return ret;
}

HW_SDHC_STATUS hw_emmc_card_lock(HW_SDHC_ID id, const uint8_t *pwd, uint8_t len)
{
        HW_SDHC_STATUS ret;

        if ((HW_EMMCC != id) || !pwd || !len || (len > HW_SDHC_CMD42_PWD_LEN_MAX)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        uint8_t cmd42[2 + HW_SDHC_CMD42_PWD_LEN_MAX] = { 0 };

        cmd42[0] = HW_SDHC_CMD42_CMD_LOCK;

        cmd42[1] = len;
        for (uint8_t i = 0; i < len; i++) {
                cmd42[2 + i] = pwd[i];
        }

        ret = hw_emmc_send_CMD42_and_check_status(id, cmd42[1] + 2, cmd42, HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED | HW_SDHC_CARD_STATUS_CARD_IS_LOCKED);

        if (HW_SDHC_STATUS_ERROR_CARD_STATUS_CARD_IS_LOCKED == ret) {
                return HW_SDHC_STATUS_SUCCESS;
        }

        return HW_SDHC_STATUS_ERROR;
}

HW_SDHC_STATUS hw_emmc_card_unlock(HW_SDHC_ID id, const uint8_t *pwd, uint8_t len)
{
        if ((HW_EMMCC != id) || !pwd || !len || (len > HW_SDHC_CMD42_PWD_LEN_MAX)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        uint8_t cmd42[2 + HW_SDHC_CMD42_PWD_LEN_MAX] = { 0 };

        cmd42[0] = HW_SDHC_CMD42_CMD_UNLOCK;

        cmd42[1] = len;
        for (uint8_t i = 0; i < len; i++) {
                cmd42[2 + i] = pwd[i];
        }

        HW_SDHC_STATUS ret = hw_emmc_send_CMD42_and_check_status(id, cmd42[1] + 2, cmd42, HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED);
        if (ret == HW_SDHC_STATUS_ERROR_CARD_STATUS_ERRORS) {
                return HW_SDHC_STATUS_ERROR_CARD_STATUS_LOCK_UNLOCK_FAIL;
        }
        return ret;
}

HW_SDHC_STATUS hw_emmc_card_force_erase(HW_SDHC_ID id)
{
        if (HW_EMMCC != id) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        uint8_t cmd42 = HW_SDHC_CMD42_CMD_ERASE;

        return hw_emmc_send_CMD42_and_check_status(id, sizeof(cmd42), &cmd42, HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED | HW_SDHC_CARD_STATUS_CARD_IS_LOCKED);
}

static HW_SDHC_STATUS hw_emmc_set_bus_vol_vdd1(HW_SDHC_ID id, uint8_t bus_vol_vdd1)
{
        switch (bus_vol_vdd1) {
        case HW_SDHC_EMMC_BUS_VOL_VDD1_3V3:
                if (hw_sdhc_get_capabilities1_r_volt_33(id)) {
                        hw_sdhc_set_pwr_ctrl_r_sd_bus_vol_vdd1(id, bus_vol_vdd1);
                        return HW_SDHC_STATUS_SUCCESS;
                }
                break;
        case HW_SDHC_EMMC_BUS_VOL_VDD1_1V2:
                if (hw_sdhc_get_capabilities1_r_volt_30(id)) {
                        hw_sdhc_set_pwr_ctrl_r_sd_bus_vol_vdd1(id, bus_vol_vdd1);
                        return HW_SDHC_STATUS_SUCCESS;
                }
                break;
        case HW_SDHC_EMMC_BUS_VOL_VDD1_1V8:
                if (hw_sdhc_get_capabilities1_r_volt_18(id)) {
                        hw_sdhc_set_pwr_ctrl_r_sd_bus_vol_vdd1(id, bus_vol_vdd1);
                        return HW_SDHC_STATUS_SUCCESS;
                }
                break;
        default:
                // External voltage is supplied, so the above cases SHOULD not be selected.
                return HW_SDHC_STATUS_SUCCESS;
        }

        return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
}

static bool hw_emmc_is_block_size_valid(HW_SDHC_ID id, HW_SDHC_XFER_MODE_R_DATA_XFER_DIR xfer_dir, uint16_t block_size)
{
        if (emmc_context.csd.c_size == 0xFFF) {
                // Capacity >2GB
                if (HW_SDHC_DATA_XFER_DIR_READ == xfer_dir ) {
                        if (emmc_context.csd.read_bl_partial == 0) {
                                if ( (block_size != HW_SDHC_DEFAULT_BLOCK_SIZE) || (block_size != (1 << emmc_context.csd.read_bl_len)) ) {
                                        return false;
                                }
                        } else {
                                if ( (block_size < HW_SDHC_DEFAULT_BLOCK_SIZE) || (block_size > (1 << emmc_context.csd.read_bl_len)) ) {
                                        return false;
                                }
                        }
                } else {
                        if (emmc_context.csd.write_bl_partial == 0) {
                                if ( (block_size != HW_SDHC_DEFAULT_BLOCK_SIZE) || (block_size != (1 << emmc_context.csd.write_bl_len)) ) {
                                        return false;
                                }
                        } else {
                                if ( (block_size < HW_SDHC_DEFAULT_BLOCK_SIZE) || (block_size > (1 << emmc_context.csd.read_bl_len)) ) {
                                        return false;
                                }
                        }
                }
        } else {
                // Capacity <2GB is not implemented
                ASSERT_WARNING(0);
        }

        return true;
}

static uint8_t hw_emmc_CRC7_one(uint8_t crc, uint8_t data)
{
        const uint8_t gen = 0x89;       // Generator Polynomial = x**7 + x**3 + 1

        crc ^= data;

        for (uint8_t i = 0; i < 8; i++) {
                if (crc & 0x80) {
                        crc ^= gen;
                }
                crc <<= 1;
        }

        return crc;
}

static uint8_t hw_emmc_CRC7_buf(uint8_t *buf, uint8_t len)
{
        uint8_t crc = 0;

        while (len--) {
                crc = hw_emmc_CRC7_one(crc, *buf++);
        }

        return crc;
}

static HW_SDHC_STATUS hw_emmc_copy_bytes_in_reverse_order(uint8_t *dst_buf, const uint8_t *src_buf, uint32_t num)
{
        if (!dst_buf || !src_buf || !num) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        for (uint32_t i = 0; i < num; i++) {
                dst_buf[num - 1 - i] = *src_buf++;
        }

        return HW_SDHC_STATUS_SUCCESS;
}

static HW_SDHC_STATUS hw_emmc_prepare_buffer_to_program_cid_register(uint8_t *cid_buf, const uint8_t *new_cid_val)
{
        HW_SDHC_STATUS ret;

        if (!cid_buf || !new_cid_val) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        // Reverse bytes so that the MSB (cid_buf[0]) is first
        ret = hw_emmc_copy_bytes_in_reverse_order(cid_buf, new_cid_val, HW_SDHC_CID_SIZE - 1);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // The contents of the CID buffer should be first set and then calculate the CRC7
        cid_buf[HW_SDHC_CID_SIZE - 1] = hw_emmc_CRC7_buf(cid_buf, HW_SDHC_CID_SIZE - 1);

        // LSB: CID bit0 is always '1'
        cid_buf[HW_SDHC_CID_SIZE - 1] |= 0x01;

        return HW_SDHC_STATUS_SUCCESS;
}

static HW_SDHC_STATUS hw_emmc_prepare_buffer_to_program_csd_register(uint8_t *csd_buf, uint8_t new_csd_val)
{
        HW_SDHC_STATUS ret;

        if (!csd_buf) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        uint8_t *p_csd = (uint8_t *)&emmc_context.csd;

        // Skip the first byte in emmc_context.csd which is the programmable CSD byte
        p_csd++;

        // Reverse bytes so that the MSB (csd_buf[0]) is first
        ret = hw_emmc_copy_bytes_in_reverse_order(csd_buf, p_csd, HW_SDHC_CSD_SIZE - 2);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Set the programmable CSD byte
        csd_buf[HW_SDHC_CSD_SIZE - 2] = new_csd_val;

        // The contents of the CID buffer should be first set and then calculate the CRC7
        csd_buf[HW_SDHC_CSD_SIZE - 1] = hw_emmc_CRC7_buf(csd_buf, HW_SDHC_CSD_SIZE - 1);

        // LSB: CSD bit0 is always '1'
        csd_buf[HW_SDHC_CSD_SIZE - 1] |= 0x01;

        return HW_SDHC_STATUS_SUCCESS;
}

static void hw_emmc_reset_context(HW_SDHC_ID id)
{
        sdhc_context.state = HW_SDHC_STATE_FREE;

        sdhc_context.cmd_events = 0;
        sdhc_context.block_size = 0;
        sdhc_context.card_status = 0;
        sdhc_context.error_int_stat = 0;
        sdhc_context.bus_speed = 0;
        sdhc_context.bus_width = HW_SDHC_BUS_WIDTH_1_BIT;
        sdhc_context.data = NULL;
        sdhc_context.dma_en = false;
        sdhc_context.data_xfer_cmd = false;
        sdhc_context.cb = NULL;
        sdhc_context.abort_impl = hw_emmc_abort_xfer_impl;

        sdhc_context.normal_int_stat_mask = 0;

        hw_sdhc_unregister_context(id);

        emmc_context.rca = HW_EMMC_CARD_RCA_RESET_VAL;
        emmc_context.cid.mid = 0;
        emmc_context.csd.csd_structure = 0;
        emmc_context.ext_csd.csd_structure = 0;
        emmc_context.ext_csd.ext_csd_rev = 0;
}

/**
 * \brief eMMC Interrupt Handler
 *
 */
void EMMC_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        hw_sdhc_interrupt_handler(HW_EMMCC);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* (dg_configUSE_HW_EMMC == 1) */

