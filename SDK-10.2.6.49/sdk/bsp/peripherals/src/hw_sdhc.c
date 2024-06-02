/**
 ****************************************************************************************
 *
 * @file hw_sdhc.c
 *
 * @brief Implementation of the SD Host Controller Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include "hw_sdhc.h"
#if (dg_configUSE_HW_EMMC == 1) || (__HW_SDHC_USE_HW_EMMC_ONLY == 0)

#include "hw_clk.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

/**
 * \brief Pointer to the context data of the registered eMMC LLD
 *
 * Index 0 = eMMC
 * Index 1 = INVALID
 *
 */
static hw_sdhc_context_data_t *context_p[2];

/***************************************************************************************************
 *
 * Declaration of private functions
 *
 **************************************************************************************************/
/**
 * \brief Wait for a timeout while the internal clock is not stable
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_sdhc_is_internal_clk_stable(HW_SDHC_ID id);

/**
 * \brief Set IP frequency/bus speed
 *
 * The system clock if divided by the requested frequency SHOULD give
 * a valid integer clock divider value: 1/16=>0, 1/1=>1, 1/2=>2, 1/4=>4, 1/8=>8
 * Otherwise, an error value is returned
 *
 * \param [in] id               SDHC controller instance
 * \param [in] freq             Frequency in Hz
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_sdhc_set_frequency_sequence(HW_SDHC_ID id, uint32_t freq);

/**
 * \brief Reset context state to IDLE and reset driver events
 *
 * \param [in] id               SDHC controller instance
 * \param [in] reset_lines      if true then reset CMD and DAT lines as well
 */
static void hw_sdhc_reset_evt_handler(HW_SDHC_ID id, bool reset_lines);

/**
 * \brief Handler of command event
 *
 * \param [in] id               SDHC controller instance
 * \param [in] events           events occurred at transaction
 */
static void hw_sdhc_cmd_evt_handler(HW_SDHC_ID id, uint32_t events);

/**
 * \brief Wait for CMD_COMPLETE event until timeout
 *
 * \param [in] id               SDHC controller instance
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_sdhc_wait_cmd_complete(HW_SDHC_ID id);

/**
 * \brief Read ERROR_INT_STAT_R and return the first error code starting from LSbit
 *
 * \param [in] error_int_stat   Value of ERROR_INT_STAT_R
 *
 * \return an error id. If no error is found then return HW_SDHC_STATUS_SUCCESS
 */
static HW_SDHC_STATUS hw_sdhc_get_error_interrupt_code(uint16_t error_int_stat);

/**
 * \brief Check card status for errors and return the first error code starting from LSbit
 *
 * \param [in] card_status      Card status
 *
 * \return an error id. If no error is found then return HW_SDHC_STATUS_SUCCESS
 */
static HW_SDHC_STATUS hw_sdhc_get_card_status_error_code(uint32_t card_status);

#if (dg_configUSE_HW_EMMC == 1)
/**
 * \brief Program CID or CSD register using CMD26 or CMD27, respectively
 *
 * \param [in] id               SDHC controller instance
 * \param [in] buf              pointer to buffer 16 bytes long
 * \param [in] tout_ms          data transfer timeout in msec
 * \param [in] cid_csd          Select which register to program CID or CSD
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_sdhc_emmc_program_cid_csd(HW_SDHC_ID id, const uint8_t *buf, uint32_t tout_ms, HW_SDHC_PROGRAM_CID_CSD cid_csd);

/**
 * \brief Sends ERASE_GROUP_START (CMD35) or ERASE_GROUP_END (CMD36)
 *
 * \param [in] id               SDHC controller instance
 * \param [in] data_addr        start/end data address to be erased
 * \param [in] cmd_index        command index, either CMD35 or CMD36
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_sdhc_CMD35_CMD36(HW_SDHC_ID id, uint32_t data_addr, uint32_t cmd_index);

/**
 * \brief CMD42 data transfer in blocking non-dma mode
 *
 * \param [in] id               SDHC controller instance
 * \param [in] config           data transfer configuration
 *
 * \return HW_SDHC_STATUS_SUCCESS if OK. Otherwise, an error id.
 */
static HW_SDHC_STATUS hw_sdhc_data_xfer_CMD42(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config);
#endif

/***************************************************************************************************
 *
 * Implementation of API - Public functions
 *
 **************************************************************************************************/

HW_SDHC_STATUS hw_sdhc_register_context(const HW_SDHC_ID id, hw_sdhc_context_data_t *context)
{
        if (!context) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (HW_SDHC_STATE_FREE != context->state) {
                return HW_SDHC_STATUS_ERROR_STATE_NOT_FREE;
        }

        HW_SDHC_DATA(id) = context;

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_set_active_interrupts_mask(HW_SDHC_ID id, uint16_t normal_int_mask, uint16_t error_int_mask)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        hw_sdhc_set_normal_int_stat_en_r(id, normal_int_mask);
        hw_sdhc_set_error_int_stat_en_r(id, error_int_mask);

        context->normal_int_stat_mask = normal_int_mask;

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_unregister_context(const HW_SDHC_ID id)
{
        HW_SDHC_DATA(id) = NULL;

        return HW_SDHC_STATUS_SUCCESS;
}

bool hw_sdhc_assert_bus_speed(HW_SDHC_ID id, uint32_t bus_speed)
{
        if ( (bus_speed == 0) || (bus_speed > HW_SDHC_1MHZ * hw_sdhc_get_capabilities1_r_base_clk_freq(id)) ) {
                return false;
        }
        return true;
}

bool hw_sdhc_assert_clk_div(HW_SDHC_ID id, uint8_t clk_div)
{
        if (!(clk_div == 0 || clk_div == 8 || clk_div == 4 || clk_div == 2 || clk_div == 1)) {
                return false;
        }
        return true;
}

bool hw_sdhc_assert_bus_width_and_speed_mode(const HW_SDHC_ID id, HW_SDHC_BUS_WIDTH bus_width, uint8_t speed_mode)
{
        if (HW_EMMCC == id) {
                if (bus_width == HW_SDHC_BUS_WIDTH_1_BIT && speed_mode == HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_HS_DDR) {
                        return false;
                }
        } else {
                if (bus_width == HW_SDHC_BUS_WIDTH_1_BIT && speed_mode == HW_SDHC_UHS_BUS_SPEED_MODE_SEL_DDR50) {
                        return false;
                }
        }
        return true;
}

bool hw_sdhc_assert_bus_speed_and_speed_mode(const HW_SDHC_ID id, uint32_t bus_speed, uint8_t speed_mode)
{
        if (HW_EMMCC == id) {
                switch (speed_mode) {
                case HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_LEGACY:
                        if (bus_speed > HW_SDHC_EMMC_BUS_SPEED_LEGACY_MAX) {
                                return false;
                        }
                        break;
                case HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_HS_SDR:
                        if (bus_speed > HW_SDHC_EMMC_BUS_SPEED_HS_SDR_MAX) {
                                return false;
                        }
                        break;
#if (HW_SDHC_SUPPORT_DDR == 1)
                case HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_HS_DDR:
                        if (bus_speed > HW_SDHC_EMMC_BUS_SPEED_HS_DDR_MAX) {
                                return false;
                        }
                        break;
#endif
                default:
                        ASSERT_WARNING(0);
                        return false;
                }
        } else {
                switch (speed_mode) {
                case HW_SDHC_UHS_BUS_SPEED_MODE_SEL_SDR12:
                        if (bus_speed > HW_SDHC_UHS_BUS_SPEED_SDR12_MAX) {
                                return false;
                        }
                        break;
                case HW_SDHC_UHS_BUS_SPEED_MODE_SEL_SDR25:
                        if (bus_speed > HW_SDHC_UHS_BUS_SPEED_SDR25_MAX) {
                                return false;
                        }
                        break;
#if (HW_SDHC_SUPPORT_DDR == 1)
                case HW_SDHC_UHS_BUS_SPEED_MODE_SEL_DDR50:
                        if (bus_speed > HW_SDHC_UHS_BUS_SPEED_DDR50_MAX) {
                                return false;
                        }
                        break;
#endif
                default:
                        ASSERT_WARNING(0);
                        return false;
                }
        }
        return true;
}

bool hw_sdhc_is_busy(HW_SDHC_ID id)
{
        return hw_sdhc_get_pstate_cmd_inhibit(id) || hw_sdhc_get_pstate_cmd_inhibit_dat(id);
}

HW_SDHC_STATUS hw_sdhc_wait_while_card_is_busy(HW_SDHC_ID id, uint32_t tout_ms)
{
        uint32_t cnt = 0;

        if (0 == tout_ms) {
                tout_ms = 1;
        }

        while (!(hw_sdhc_get_pstate_dat_3_0(id) & 0x01) && (cnt++ <= tout_ms)) {
                hw_clk_delay_usec(1000);
        }

        if (!(hw_sdhc_get_pstate_dat_3_0(id) & 0x01)) {
                return HW_SDHC_STATUS_ERROR_TIMEOUT;
        }

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_send_command(HW_SDHC_ID id, const hw_sdhc_cmd_config_t *cmd_config, uint32_t *response)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        if (!cmd_config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        // Wait command line is not inhibited
        if (HW_SDHC_STATUS_SUCCESS != hw_sdhc_wait_cmd_line_not_inhibited(id)) {
                return HW_SDHC_STATUS_ERROR_TIMEOUT_CMD_LINE;
        }

        if (cmd_config->data_present && (cmd_config->cmd_type != HW_SDHC_CMD_TYPE_ABORT)) {
                // Wait data line is not inhibited
                if (HW_SDHC_STATUS_SUCCESS != hw_sdhc_wait_data_line_not_inhibited(id)) {
                        return HW_SDHC_STATUS_ERROR_TIMEOUT_DATA_LINE;
                }
        }

        NVIC_DisableIRQ(HW_SDHC_INT(id));

        hw_sdhc_set_normal_int_signal_en_r_cmd_complete_signal_en(id, true);

        context->read_resp = cmd_config->read_resp;
        context->response = response;
        context->resp_type = cmd_config->resp_type;
        context->state = HW_SDHC_STATE_WAIT_CMD_COMPLETE;

        context->data_xfer_cmd = cmd_config->data_present;

        context->cmd_events = 0;

        NVIC_EnableIRQ(HW_SDHC_INT(id));

        hw_sdhc_set_argument_r(id, cmd_config->cmd_arg);

        /*
         * The command is sent when the cmd_idx is written to the EMMC_CMD_R_REG register.
         * Therefore, all fields could be set separately and set cmd_idx last to send the command.
         */
        hw_sdhc_set_cmd_r(id,
                (cmd_config->resp_type << EMMC_EMMC_CMD_R_REG_RESP_TYPE_SELECT_Pos) |
                (cmd_config->sub_cmd_flag << EMMC_EMMC_CMD_R_REG_SUB_CMD_FLAG_Pos) |
                (cmd_config->crc_check_en << EMMC_EMMC_CMD_R_REG_CMD_CRC_CHK_ENABLE_Pos) |
                (cmd_config->idx_check_en << EMMC_EMMC_CMD_R_REG_CMD_IDX_CHK_ENABLE_Pos) |
                (cmd_config->data_present << EMMC_EMMC_CMD_R_REG_DATA_PRESENT_SEL_Pos) |
                (cmd_config->cmd_type << EMMC_EMMC_CMD_R_REG_CMD_TYPE_Pos) |
                (cmd_config->cmd_index << EMMC_EMMC_CMD_R_REG_CMD_INDEX_Pos)
        );

        if (!cmd_config->wait_cmd_complete) {
                return HW_SDHC_STATUS_SUCCESS;
        }

        if (cmd_config->cmd_complete_delay) {
                hw_clk_delay_usec(cmd_config->cmd_complete_delay);
        }

        HW_SDHC_STATUS ret = hw_sdhc_wait_cmd_complete_event(id);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        if (cmd_config->wait_for_busy) {
                ret = hw_sdhc_wait_while_card_is_busy(id, cmd_config->busy_tout_ms);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
        }

        if (cmd_config->read_resp && cmd_config->check_errors) {
                ret = hw_sdhc_get_card_status_error_code(context->card_status);
        }

        return ret;
}

HW_SDHC_STATUS hw_sdhc_wait_cmd_line_not_inhibited(HW_SDHC_ID id)
{
        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                if (!hw_sdhc_get_pstate_cmd_inhibit(id)) {
                        return HW_SDHC_STATUS_SUCCESS;
                }
                hw_clk_delay_usec(HW_SDHC_TOUT_CMD_INHIBIT_MS);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT_CMD_LINE;
}

HW_SDHC_STATUS hw_sdhc_wait_data_line_not_inhibited(HW_SDHC_ID id)
{
        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                if (!hw_sdhc_get_pstate_cmd_inhibit_dat(id)) {
                        return HW_SDHC_STATUS_SUCCESS;
                }
                hw_clk_delay_usec(HW_SDHC_TOUT_CMD_INHIBIT_MS);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT_DATA_LINE;
}

HW_SDHC_STATUS hw_sdhc_timeout_setting(HW_SDHC_ID id, uint32_t tout)
{
        uint32_t tout_clk_freq = hw_sdhc_get_capabilities1_r_tout_clk_freq(id);
        uint32_t tout_cnt;

        if ( !((tout >= HW_SDHC_TOUT_CNT_MIN / tout_clk_freq) &&
                        (tout <= HW_SDHC_TOUT_CNT_MAX / tout_clk_freq)) ) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        // Calculate tout_cnt that gives a "floor" of tout
        uint32_t temp_cnt = (tout) * tout_clk_freq;
        tout_cnt = 8*sizeof(temp_cnt) - __CLZ(temp_cnt) - 1;

        if ( temp_cnt > (1<<tout_cnt) ) {
                tout_cnt++;
        }

        hw_sdhc_set_tout_ctrl_r_tout_cnt(id, (uint8_t)tout_cnt - HW_SDHC_TOUT_CNT_OFFSET);

        return HW_SDHC_STATUS_SUCCESS;
}

static HW_SDHC_STATUS hw_sdhc_is_internal_clk_stable(HW_SDHC_ID id)
{
        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                if (hw_sdhc_get_clk_ctrl_r_internal_clk_stable(id)) {
                        return HW_SDHC_STATUS_SUCCESS;
                }
                hw_clk_delay_usec(HW_SDHC_TOUT_INTERNAL_CLK_STABLE_MS);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_internal_clk_enable(HW_SDHC_ID id)
{
        hw_sdhc_set_clk_ctrl_r_internal_clk_en(id, true);

        if (HW_SDHC_STATUS_SUCCESS == hw_sdhc_is_internal_clk_stable(id)) {
                // This step does not affect Host Controllers which do not support PLL Enable
                hw_sdhc_set_clk_ctrl_r_pll_enable(id, true);
                if (HW_SDHC_STATUS_SUCCESS == hw_sdhc_is_internal_clk_stable(id)) {
                        return HW_SDHC_STATUS_SUCCESS;
                }
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_set_and_wait_sw_rst_dat(HW_SDHC_ID id)
{
        hw_sdhc_set_sw_rst_r_sw_rst_dat(id, true);
        while (hw_sdhc_get_sw_rst_r_sw_rst_dat(id)) {
                // Wait reset to complete
                __NOP();
        }
        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_set_and_wait_sw_rst_cmd(HW_SDHC_ID id)
{
        hw_sdhc_set_sw_rst_r_sw_rst_cmd(id, true);
        while (hw_sdhc_get_sw_rst_r_sw_rst_cmd(id)) {
                // Wait reset to complete
                __NOP();
        }
        return HW_SDHC_STATUS_SUCCESS;
}

void hw_sdhc_wait_power_ramp_up(HW_SDHC_ID id, uint32_t bus_speed)
{
        // Wait for voltage ramp up time
        hw_clk_delay_usec(HW_SDHC_DELAY_VOLTAGE_RAMP_UP_US);

        // Provide >= 74 clocks before SD CMD
        hw_clk_delay_usec((74 * HW_SDHC_1MHZ) / bus_speed);
}

static HW_SDHC_STATUS hw_sdhc_set_frequency_sequence(HW_SDHC_ID id, uint32_t freq)
{
        if (!hw_sdhc_assert_bus_speed(id, freq)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        uint32_t sysclk_freq = hw_clk_get_sysclk_freq();
        uint32_t clk_div = sysclk_freq / freq;
        clk_div = (clk_div == 16) ? 0 : clk_div;

        if ((sysclk_freq % freq) || !hw_sdhc_assert_clk_div(id, clk_div)) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (HW_SDHC_STATUS_SUCCESS != hw_sdhc_stop_sd_clock(id)) {
                return HW_SDHC_STATUS_ERROR_TIMEOUT_STOP_SD_CLK;
        }

        hw_sdhc_set_clk_ctrl_r_pll_enable(id, false);

        // Preset values are are not enabled
        if (hw_sdhc_get_host_ctrl2_r_preset_val_enable(id)) {
                ASSERT_WARNING(0);
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        // Select clk div: 0=1/16, 1=1/1, 2=1/2, 4=1/4, 8=1/8
        if (id == HW_EMMCC) {
                REG_SETF(CRG_CTRL, CLK_PDCTRL_REG, EMMC_CLK_DIV, clk_div);
        }

        hw_sdhc_set_clk_ctrl_r_pll_enable(id, true);

        if (HW_SDHC_STATUS_SUCCESS == hw_sdhc_is_internal_clk_stable(id)) {
                hw_sdhc_set_clk_ctrl_r_sd_clk_en(id, true);

                // SW Reset to avoid the effect of any glitch on sampling clock
                hw_sdhc_set_and_wait_sw_rst_dat(id);
                hw_sdhc_set_and_wait_sw_rst_cmd(id);
                return HW_SDHC_STATUS_SUCCESS;
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_set_frequency(HW_SDHC_ID id, uint32_t frequency)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        if (hw_sdhc_is_busy(id)) {
                return HW_SDHC_STATUS_ERROR_OPERATION_IN_PROGRESS;
        }

        if ((context->bus_speed != frequency) || !hw_sdhc_get_clk_ctrl_r_sd_clk_en(id)) {
                // "frequency" is already set and running
                HW_SDHC_STATUS ret = hw_sdhc_set_frequency_sequence(id, frequency);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
        }
        context->bus_speed = frequency;

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_stop_sd_clock(HW_SDHC_ID id)
{
        for (uint32_t i = 0; i < HW_SDHC_TOUT_CMD_INHIBIT_MS; i++) {
                if (!hw_sdhc_get_pstate_cmd_inhibit_dat(id) && !hw_sdhc_get_pstate_cmd_inhibit(id)) {
                        hw_sdhc_set_clk_ctrl_r_sd_clk_en(id, false);
                        return HW_SDHC_STATUS_SUCCESS;
                }
                hw_clk_delay_usec(HW_SDHC_DELAY_1MS);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT_STOP_SD_CLK;
}

void hw_sdhc_set_bus_width_at_host(HW_SDHC_ID id, HW_SDHC_BUS_WIDTH bus_width)
{
        switch (bus_width) {
        case HW_SDHC_BUS_WIDTH_1_BIT:
                hw_sdhc_set_host_ctrl1_r_ext_dat_xfer(id, HW_SDHC_EXT_DAT_XFER_DEFAULT);
                hw_sdhc_set_host_ctrl1_r_dat_xfer_width(id, HW_SDHC_DAT_XFER_WIDTH_1BIT);
                break;
        case HW_SDHC_BUS_WIDTH_4_BIT:
#if (HW_SDHC_SUPPORT_DDR == 1)
        case HW_SDHC_BUS_WIDTH_4_BIT_DDR:
#endif
                hw_sdhc_set_host_ctrl1_r_ext_dat_xfer(id, HW_SDHC_EXT_DAT_XFER_DEFAULT);
                hw_sdhc_set_host_ctrl1_r_dat_xfer_width(id, HW_SDHC_DAT_XFER_WIDTH_4BIT);
                break;
        case HW_SDHC_BUS_WIDTH_8_BIT:
#if (HW_SDHC_SUPPORT_DDR == 1)
        case HW_SDHC_BUS_WIDTH_8_BIT_DDR:
#endif
                hw_sdhc_set_host_ctrl1_r_dat_xfer_width(id, HW_SDHC_DAT_XFER_WIDTH_1BIT);
                hw_sdhc_set_host_ctrl1_r_ext_dat_xfer(id, HW_SDHC_EXT_DAT_XFER_8BIT);
                break;
        default:
                ASSERT_WARNING(0);
        }
}

static HW_SDHC_STATUS hw_sdhc_wait_cmd_complete(HW_SDHC_ID id)
{
        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                if (hw_sdhc_get_normal_int_stat_r_cmd_complete(id)) {
                        return HW_SDHC_STATUS_SUCCESS;
                }
                hw_clk_delay_usec(HW_SDHC_TOUT_CMD_COMPLETE_MS);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

static HW_SDHC_STATUS hw_sdhc_get_error_interrupt_code(uint16_t error_int_stat)
{
        if (!error_int_stat) {
                return HW_SDHC_STATUS_SUCCESS;
        }

        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_CMD_TOUT_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_CMD_TOUT;
        }
        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_CMD_CRC_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_CMD_CRC;
        }
        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_CMD_END_BIT_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_CMD_END_BIT;
        }
        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_CMD_IDX_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_CMD_IDX;
        }
        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_DATA_TOUT_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_DATA_TOUT;
        }
        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_DATA_CRC_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_DATA_CRC;
        }
        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_DATA_END_BIT_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_DATA_END_BIT;
        }
        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_CUR_LMT_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_CUR_LMT;
        }
        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_AUTO_CMD_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_AUTO_CMD;
        }
        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_ADMA_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_ADMA_ERR;
        }
        if (error_int_stat & EMMC_EMMC_ERROR_INT_STAT_R_REG_RESP_ERR_Msk) {
                return HW_SDHC_STATUS_ERROR_RESP_ERR;
        }
        return HW_SDHC_STATUS_ERROR_INT_STAT_R;
}

static HW_SDHC_STATUS hw_sdhc_get_card_status_error_code(uint32_t card_status)
{
        if (card_status & HW_SDHC_CARD_STATUS_ERRORS_MASK) {
                if (card_status & HW_SDHC_CARD_STATUS_SWITCH_ERROR) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_SWITCH;
                }
                if (card_status & HW_SDHC_CARD_STATUS_ERASE_RESET) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_ERASE_RESET;
                }
                if (card_status & HW_SDHC_CARD_STATUS_WP_ERASE_SKIP) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_WP_ERASE_SKIP;
                }
                if (card_status & HW_SDHC_CARD_STATUS_CID_CSD_OVERWRITE) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_CID_CSD_OVRWR;
                }
                if (card_status & HW_SDHC_CARD_STATUS_ERROR) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_GEN_ERROR;
                }
                if (card_status & HW_SDHC_CARD_STATUS_CC_ERROR) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_CC;
                }
                if (card_status & HW_SDHC_CARD_STATUS_CARD_ECC_FAILED) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_ECC;
                }
                if (card_status & HW_SDHC_CARD_STATUS_ILLEGAL_COMMAND) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_ILLEGAL_CMD;
                }
                if (card_status & HW_SDHC_CARD_STATUS_COM_CRC_ERROR) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_COM_CRC;
                }
                if (card_status & HW_SDHC_CARD_STATUS_LOCK_UNLOCK_FAILED) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_LOCK_UNLOCK_FAIL;
                }
                if (card_status & HW_SDHC_CARD_STATUS_CARD_IS_LOCKED) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_CARD_IS_LOCKED;
                }
                if (card_status & HW_SDHC_CARD_STATUS_WP_VIOLATION) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_WP_VIOLATION;
                }
                if (card_status & HW_SDHC_CARD_STATUS_ERASE_PARAM) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_ERASE_PARAM;
                }
                if (card_status & HW_SDHC_CARD_STATUS_ERASE_SEQ_ERROR) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_ERASE_SEQ;
                }
                if (card_status & HW_SDHC_CARD_STATUS_BLOCK_LEN_ERROR) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_BLOCK_LEN;
                }
                if (card_status & HW_SDHC_CARD_STATUS_ADDRESS_MISALIGN) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_ADDRESS_MISALIGN;
                }
                if (card_status & HW_SDHC_CARD_STATUS_OUT_OF_RANGE) {
                        return HW_SDHC_STATUS_ERROR_CARD_STATUS_ADDR_OUT_OF_RANGE;
                }
        }
        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_wait_cmd_complete_event(const HW_SDHC_ID id)
{
        volatile hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                if ( context->cmd_events & context->normal_int_stat_mask) {
                        if (context->cmd_events & HW_SDHC_EVENT_DMA_INTERRUPT) {
                                return HW_SDHC_STATUS_ERROR_PAGE_BOUNDARY;
                        }

                        if ( context->cmd_events & HW_SDHC_EVENT_ERR_INTERRUPT ) {
                                HW_SDHC_STATUS ret = hw_sdhc_get_error_interrupt_code(context->error_int_stat);
                                ASSERT_ERROR((ret != HW_SDHC_STATUS_SUCCESS) && (ret != HW_SDHC_STATUS_ERROR_INT_STAT_R));
                                return ret;
                        }

                        if ( context->cmd_events & HW_SDHC_EVENT_CMD_COMPLETE ) {
                                return HW_SDHC_STATUS_SUCCESS;
                        }
                }

                hw_clk_delay_usec(HW_SDHC_TOUT_CMD_COMPLETE_MS);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_data_xfer_init(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config)
{
        HW_SDHC_STATUS ret;
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        if (!config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (config->dma_en) {
                if ((HW_SDHC_DMA_SEL_SDMA == config->dma_type) && !hw_sdhc_get_capabilities1_r_sdma_support(id)) {
                        return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
                }

                if ((HW_SDHC_DMA_SEL_ADMA2 == config->dma_type) && !hw_sdhc_get_capabilities1_r_adma2_support(id)) {
                        return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
                }
        }

#if (dg_configUSE_HW_EMMC == 1)
        if (config->set_blk_len || config->bus_testing) {
                if (!((context->bus_width == HW_SDHC_BUS_WIDTH_1_BIT) ||
                      (context->bus_width == HW_SDHC_BUS_WIDTH_4_BIT) ||
                      (context->bus_width == HW_SDHC_BUS_WIDTH_8_BIT))) {
                        // In DDR mode, CMD16 and bus testing are illegal operations
                        return HW_SDHC_STATUS_ERROR;
                }
        }
#endif
        // Initialize transfer related registers...
        ret = hw_sdhc_set_xfer_registers(id, config);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Could also set XFER_MODE_R at once...
        hw_sdhc_set_xfer_mode_r_data_xfer_dir(id, config->xfer_dir);
        hw_sdhc_set_xfer_mode_r_multi_blk_sel(id, config->block_cnt != 1);


        hw_sdhc_set_xfer_mode_r_block_count_enable(id, config->block_cnt > 1);

        if (HW_EMMCC == id) {
                hw_sdhc_set_xfer_mode_r_auto_cmd_enable(id, config->auto_command);
        } else {
                hw_sdhc_set_xfer_mode_r_auto_cmd_enable(id, false);
        }
        hw_sdhc_set_xfer_mode_r_dma_en_emmc(id, config->dma_en);

        if (hw_sdhc_get_xfer_mode_r_resp_err_chk_enable(id)) {
                hw_sdhc_set_xfer_mode_r_resp_int_disable(id, true);
                if (hw_sdhc_get_emmc_ctrl_r_card_is_emmc(id)) {
                        hw_sdhc_set_xfer_mode_r_resp_type(id, HW_SDHC_RESP_TYPE_R1_MEMORY);
                }
        }

        hw_sdhc_timeout_setting(id, config->tout_cnt_time);

#if (dg_configUSE_HW_EMMC == 1)
        if (config->set_blk_len && !config->bus_testing) {
                ret = hw_sdhc_set_blocklen_CMD16(id, config->block_size);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
        }

        if (config->block_cnt > 1) {
                // If this is not called, transaction is open-ended/infinite and should stop with CMD12/CMD25
                ret = hw_sdhc_set_block_count_CMD23(id, config->emmc_reliable_write_en, config->block_cnt);
        }
#endif
        return ret;
}

HW_SDHC_STATUS hw_sdhc_set_xfer_registers(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        if (!config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (config->dma_en) {
                switch (config->dma_type) {
                case HW_SDHC_DMA_SEL_SDMA:
                        if ( (HW_SDHC_PAGE_BDARY_BYTES_4K << config->page_bdary) < (config->block_cnt * config->block_size) ) {
                                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
                        }

                        hw_sdhc_set_blocksize_r(id, 0);
                        hw_sdhc_set_xfer_mode_r(id, 0);
                        hw_sdhc_set_sdmasa_r(id, 0);

                        hw_sdhc_set_host_ctrl1_r_dma_sel(id, config->dma_type);

                        hw_sdhc_set_adma_sa_low_r(id, (uint32_t)config->data);
                        hw_sdhc_set_sdmasa_r(id, config->block_cnt);

                        if (config->use_32bit_counter) {
                                // SDMASA_R will be used as 32-bit block counter
                                hw_sdhc_set_blockcount_r(id, 0);
                        } else {
                                hw_sdhc_set_blockcount_r(id, config->block_cnt);
                        }

                        hw_sdhc_set_blocksize_r_sdma_buf_bdary(id, config->page_bdary);
                        hw_sdhc_set_blocksize_r_xfer_block_size(id, config->block_size);

                        // Save dma context
                        context->dma_en = true;
                        context->dma_type = config->dma_type;
                        break;
                case HW_SDHC_DMA_SEL_ADMA2:
                        {
                                _Static_assert(sizeof(hw_sdhc_adma_descriptor_table_t) == 2*sizeof(uint32_t), "Invalid size of hw_sdhc_adma_descriptor_table_t!");

                                static hw_sdhc_adma_descriptor_table_t adma_desc_tab[HW_SDHC_ADMA2_MAX_DESC_TABLE_LINES];

                                // Create ADMA2 descriptor table... Current implementation: simple case with one line only!
                                uint32_t len = config->block_size * config->block_cnt;

                                adma_desc_tab[0].attr_n_len.valid = 1;   // this is a valid/active line in Desc. Table
                                adma_desc_tab[0].attr_n_len.end = 1;     // Define just one line in Desc. Table
                                /*
                                 * Generates DMA_INTERRUPT when this line xfer is complete
                                 * Since current implementation has only one line in the descriptor table,
                                 * there is no need to activate this attribute
                                 */
                                adma_desc_tab[0].attr_n_len.intr = 0;
                                adma_desc_tab[0].attr_n_len.act = HW_SDHC_ADMA2_ACT_TRAN<<1;

                                if (config->adma2_len_mode == HW_SDHC_ADMA2_LEN_MODE_16BIT) {
                                        if (len > HW_SDHC_ADMA2_MAX_DATA_LEN_MODE_16BIT_BYTES) {
                                                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
                                        }

                                        if (len == HW_SDHC_ADMA2_MAX_DATA_LEN_MODE_16BIT_BYTES) {
                                                len = 0;
                                        }
                                        adma_desc_tab[0].attr_n_len.len_lower = len;
                                        adma_desc_tab[0].attr_n_len.len_upper = 0;
                                } else {
                                        if (len > HW_SDHC_ADMA2_MAX_DATA_LEN_MODE_26BIT_BYTES) {
                                                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
                                        }

                                        if (len == HW_SDHC_ADMA2_MAX_DATA_LEN_MODE_26BIT_BYTES) {
                                                len = 0;
                                        }
                                        adma_desc_tab[0].attr_n_len.len_lower = len & 0xFFFF;
                                        adma_desc_tab[0].attr_n_len.len_upper = (len >> 16) & 0x3FF;
                                }
                                adma_desc_tab[0].addr = (uint32_t)config->data;      // Address of data in sys mem

                                // Set registers...
                                hw_sdhc_set_blocksize_r(id, 0);
                                hw_sdhc_set_xfer_mode_r(id, 0);
                                hw_sdhc_set_sdmasa_r(id, 0);

                                hw_sdhc_set_host_ctrl1_r_dma_sel(id, config->dma_type);
                                hw_sdhc_set_host_ctrl2_r_adma2_len_mode(id, config->adma2_len_mode);
                                hw_sdhc_set_adma_sa_low_r(id, (uint32)&adma_desc_tab[0]);

                                hw_sdhc_set_blockcount_r(id, config->block_cnt);
                                hw_sdhc_set_blocksize_r_xfer_block_size(id, config->block_size);

                                // Save dma context
                                context->dma_en = true;
                                context->dma_type = config->dma_type;
                                break;
                        }
                default:
                        ASSERT_WARNING(0);
                        return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
                }
        } else {
                context->dma_en = false;

                hw_sdhc_set_blocksize_r_xfer_block_size(id, config->block_size);
                hw_sdhc_set_blockcount_r(id, config->block_cnt);
                hw_sdhc_set_sdmasa_r(id, config->block_cnt);
        }

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_data_xfer_send_cmd(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        if (!config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (1 == config->block_cnt) {
#if (dg_configUSE_HW_EMMC == 1)
                if (config->bus_testing) {
                        // Bus testing with 1, 4, or 8 Bytes
                        if (config->xfer_dir == HW_SDHC_DATA_XFER_DIR_READ) {
                                cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD14;
                        } else {
                                cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD19;
                        }
                } else
#endif
                {
                        if (config->xfer_dir == HW_SDHC_DATA_XFER_DIR_READ) {
                                cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD17;
                        } else {
                                cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD24;
                        }
                }
        } else {
                if (config->xfer_dir == HW_SDHC_DATA_XFER_DIR_READ) {
                        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD18;
                } else {
                        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD25;
                }
        }
        cmd_config.cmd_arg = config->address;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = true;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_data_xfer_start_non_dma_blocking(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config)
{
        HW_SDHC_STATUS ret;

        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        if (!config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        context->data = (uint32_t *)config->data;

        if (config->xfer_dir == HW_SDHC_DATA_XFER_DIR_READ) {
                // Non-DMA blocking read...
                for (uint32_t blk_cnt = 0; blk_cnt < config->block_cnt; blk_cnt++) {
                        ret = hw_sdhc_wait_buf_rd_ready(id);
                        if (HW_SDHC_STATUS_SUCCESS != ret) {
                                return ret;
                        }
                        for (uint16_t blk_sz = 0; blk_sz < config->block_size; blk_sz += sizeof(uint32_t)) {
                                ret = hw_sdhc_wait_buf_rd_enable(id);
                                if (HW_SDHC_STATUS_SUCCESS != ret) {
                                        return ret;
                                }

                                *context->data = hw_sdhc_get_buf_dat_r(id);       // read 4 bytes
                                context->data++;
                        }
                }
        } else {
                // Non-DMA blocking write...
                for (uint32_t blk_cnt = 0; blk_cnt < config->block_cnt; blk_cnt++) {
                        ret = hw_sdhc_wait_buf_wr_ready(id);
                        if (HW_SDHC_STATUS_SUCCESS != ret) {
                                return ret;
                        }
                        for (uint16_t blk_sz = 0; blk_sz < config->block_size; blk_sz += sizeof(uint32_t)) {
                                ret = hw_sdhc_wait_buf_wr_enable(id);
                                if (HW_SDHC_STATUS_SUCCESS != ret) {
                                        return ret;
                                }

                                hw_sdhc_set_buf_dat_r(id, *context->data);       // write 4 bytes
                                context->data++;
                        }
                }
        }

        // Wait for transfer complete interrupt
        ret = hw_sdhc_wait_xfer_complete(id, config->xfer_tout_ms);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Wait command line is not inhibited
        ret = hw_sdhc_wait_cmd_line_not_inhibited(id);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Wait data line is not inhibited
        ret = hw_sdhc_wait_data_line_not_inhibited(id);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }
        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_data_xfer_start_non_dma_non_blocking(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        if (!config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        context->xfer_dir = config->xfer_dir;
        context->data = (uint32_t *)config->data;
        context->block_size = config->block_size;
        context->state = HW_SDHC_STATE_WAIT_DATA_XFER_COMPLETE;
        context->cmd_events = 0;

        NVIC_DisableIRQ(HW_SDHC_INT(id));

        hw_sdhc_set_normal_int_signal_en_r_xfer_complete_signal_en(id, true);
        if (context->xfer_dir == HW_SDHC_DATA_XFER_DIR_WRITE) {
                hw_sdhc_set_normal_int_signal_en_r_buf_wr_ready_signal_en(id, true);
        } else {
                hw_sdhc_set_normal_int_signal_en_r_buf_rd_ready_signal_en(id, true);
        }

        NVIC_EnableIRQ(HW_SDHC_INT(id));

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_data_xfer_start_dma_blocking(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        if (!config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        switch (config->dma_type) {
        case HW_SDHC_DMA_SEL_SDMA:
                for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                        if (hw_sdhc_get_normal_int_stat_r_xfer_complete(id)) {
                                // Clear interrupts
                                hw_sdhc_set_normal_int_stat_r_xfer_complete(id, true);
                                return HW_SDHC_STATUS_SUCCESS;
                        }

                        if (hw_sdhc_get_normal_int_stat_r_dma_interrupt(id)) {
                                context->error_int_stat = hw_sdhc_get_error_int_stat_r(id);

                                // Clear dma interrupt status bit
                                hw_sdhc_set_normal_int_stat_r_dma_interrupt(id, true);

                                return HW_SDHC_STATUS_ERROR_PAGE_BOUNDARY;
                        }

                        hw_clk_delay_usec(config->xfer_tout_ms);
                }
                break;
        case HW_SDHC_DMA_SEL_ADMA2:
                for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                        if (hw_sdhc_get_normal_int_stat_r_xfer_complete(id)) {
                                // Clear interrupts
                                hw_sdhc_set_normal_int_stat_r_xfer_complete(id, true);
                                return HW_SDHC_STATUS_SUCCESS;
                        }
                        if (hw_sdhc_get_error_int_stat_r_adma_err(id)) {
                                context->adma_error = hw_sdhc_get_adma_err_stat_r(id);
                                context->error_int_stat = hw_sdhc_get_error_int_stat_r(id);

                                return HW_SDHC_STATUS_ERROR_ADMA_ERR;
                        }
                        hw_clk_delay_usec(config->xfer_tout_ms);
                }
                break;
        default:
                ASSERT_WARNING(0);
                break;
        }

        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_data_xfer_start_dma_non_blocking(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        if (!config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        context->xfer_dir = config->xfer_dir;
        context->data = (uint32_t *)config->data;
        context->block_size = config->block_size;
        context->state = HW_SDHC_STATE_WAIT_DATA_XFER_COMPLETE;
        context->cmd_events = 0;

        switch (config->dma_type) {
        case HW_SDHC_DMA_SEL_SDMA:
                // Handle DMA_INTERRUPT due to page boundary violation inside the event handler
                // Page Boundary interrupts SHOULD be handled by user-app to set next data_addr
                NVIC_DisableIRQ(HW_SDHC_INT(id));

                hw_sdhc_set_normal_int_signal_en_r_xfer_complete_signal_en(id, true);
                hw_sdhc_set_normal_int_signal_en_r_dma_interrupt_signal_en(id, true);

                NVIC_EnableIRQ(HW_SDHC_INT(id));

                break;
        case HW_SDHC_DMA_SEL_ADMA2:
                NVIC_DisableIRQ(HW_SDHC_INT(id));

                hw_sdhc_set_normal_int_signal_en_r_xfer_complete_signal_en(id, true);
                hw_sdhc_set_error_int_signal_en_r_adma_err_en(id, true);

                NVIC_EnableIRQ(HW_SDHC_INT(id));

                break;
        default:
                ASSERT_WARNING(0);
                break;
        }

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_wait_xfer_complete_event(HW_SDHC_ID id, uint32_t tout)
{
        volatile hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                // Check if xfer complete bit is set when corresponding signal bit is set
                if ( context->cmd_events & HW_SDHC_EVENT_XFER_COMPLETE ) {
                        return HW_SDHC_STATUS_SUCCESS;
                }

                // Check if xfer complete bit is set when corresponding signal bit is not set
                if (hw_sdhc_get_normal_int_stat_r_xfer_complete(id)) {
                        hw_sdhc_clr_normal_int_stat(id);
                        return HW_SDHC_STATUS_SUCCESS;
                }

                hw_clk_delay_usec(tout);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_wait_buf_rd_ready(HW_SDHC_ID id)
{
        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                if (hw_sdhc_get_normal_int_stat_r_buf_rd_ready(id)) {
                        // Clear the interrupt
                        hw_sdhc_set_normal_int_stat_r_buf_rd_ready(id, true);
                        return HW_SDHC_STATUS_SUCCESS;
                }
                hw_clk_delay_usec(HW_SDHC_TOUT_BUF_RD_READY_MS);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_wait_buf_rd_enable(HW_SDHC_ID id)
{
        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                if (hw_sdhc_get_pstate_buf_rd_enable(id)) {
                        return HW_SDHC_STATUS_SUCCESS;
                }
                hw_clk_delay_usec(HW_SDHC_TOUT_BUF_RD_ENABLE_MS);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_wait_buf_wr_ready(HW_SDHC_ID id)
{
        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                if (hw_sdhc_get_normal_int_stat_r_buf_wr_ready(id)) {
                        // Clear the interrupt
                        hw_sdhc_set_normal_int_stat_r_buf_wr_ready(id, true);
                        return HW_SDHC_STATUS_SUCCESS;
                }
                hw_clk_delay_usec(HW_SDHC_TOUT_BUF_WR_READY_MS);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_wait_buf_wr_enable(HW_SDHC_ID id)
{
        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                if (hw_sdhc_get_pstate_buf_wr_enable(id)) {
                        return HW_SDHC_STATUS_SUCCESS;
                }
                hw_clk_delay_usec(HW_SDHC_TOUT_BUF_WR_ENABLE_MS);
        }
        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_wait_xfer_complete(HW_SDHC_ID id, uint32_t tout)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        HW_SDHC_STATUS ret = HW_SDHC_STATUS_ERROR_TIMEOUT;

        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                if (hw_sdhc_get_normal_int_stat_r_xfer_complete(id)) {
                        if (hw_sdhc_get_normal_int_stat_r_err_interrupt(id)) {
                                context->error_int_stat = hw_sdhc_get_error_int_stat_r(id);
                                ret = HW_SDHC_STATUS_ERROR;
                        } else {
                                ret = HW_SDHC_STATUS_SUCCESS;
                        }
                        // Clear the interrupts...
                        hw_sdhc_clr_error_int_stat(id);                 // Clears error at normal stat reg as well
                        hw_sdhc_clr_normal_int_stat(id);

                        return ret;
                }
                hw_clk_delay_usec(tout);
        }
        return ret;
}

static void hw_sdhc_reset_evt_handler(HW_SDHC_ID id, bool reset_lines)
{
        // Clear Signals...
        hw_sdhc_set_error_int_signal_en_r(id, 0);
        hw_sdhc_set_normal_int_signal_en_r(id, 0);

        // Clear interrupts
        hw_sdhc_clr_error_int_stat(id);
        hw_sdhc_clr_normal_int_stat(id);

        if (reset_lines) {
                // Reset CMD and DAT lines
                hw_sdhc_set_and_wait_sw_rst_dat(id);
                hw_sdhc_set_and_wait_sw_rst_cmd(id);
        }
}

void hw_sdhc_evt_complete(HW_SDHC_ID id, uint32_t events)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        // Save registers to context
        context->cmd_events = events;
        context->adma_error = hw_sdhc_get_adma_err_stat_r(id);
        context->error_int_stat = hw_sdhc_get_error_int_stat_r(id);

        hw_sdhc_reset_evt_handler(id, true);

        if (HW_SDHC_STATE_WAIT_DATA_XFER_COMPLETE == context->state) {
                /* Interrupt mode requires a callback */
                ASSERT_WARNING(context->cb);
                context->cb(events);
        }
        context->state = HW_SDHC_STATE_IDLE;
}

static void hw_sdhc_cmd_evt_handler(HW_SDHC_ID id, uint32_t events)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        context->state = HW_SDHC_STATE_IDLE;
        hw_sdhc_set_normal_int_signal_en_r_cmd_complete_signal_en(id, false);

        bool reset_flag = false;
        context->card_status = hw_sdhc_get_resp01_r(id);
        context->cmd_events = events;
        context->error_int_stat = hw_sdhc_get_error_int_stat_r(id);

        if (context->read_resp && context->response) {
                *context->response++ = context->card_status;
                if (HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_136 == context->resp_type) {
                        *context->response++ = hw_sdhc_get_resp23_r(id);
                        *context->response++ = hw_sdhc_get_resp45_r(id);
                        *context->response = hw_sdhc_get_resp67_r(id);
                } else if (context->card_status & HW_SDHC_CARD_STATUS_ERRORS_MASK) {
                        reset_flag = true;
                }
        }

        if (!reset_flag) {
                if (!context->data_xfer_cmd) {
                        reset_flag = !hw_sdhc_get_pstate_buf_rd_xfer_active(id) &&
                                     !hw_sdhc_get_pstate_buf_wr_xfer_active(id) &&
                                     !(context->cmd_events & EMMC_EMMC_NORMAL_INT_STAT_EN_R_REG_XFER_COMPLETE_STAT_EN_Msk);
                }
        }
        context->data_xfer_cmd = false;

        if (reset_flag) {
                hw_sdhc_reset_evt_handler(id, false);
        }
        return;
}

HW_SDHC_STATUS hw_sdhc_abort_xfer_sync(HW_SDHC_ID id, uint32_t tout_ms)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        // Stop data transfer
        context->cmd_events = 0;
        hw_sdhc_set_bgap_ctrl_r_stop_bg_req(id, true);

        HW_SDHC_STATUS ret = hw_sdhc_wait_xfer_complete_event(id, tout_ms);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        return hw_sdhc_abort_xfer_async(id, tout_ms);
}

HW_SDHC_STATUS hw_sdhc_abort_xfer_async(HW_SDHC_ID id, uint32_t tout_ms)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        return context->abort_impl(id, tout_ms);
}

HW_SDHC_STATUS hw_sdhc_error_recovery(HW_SDHC_ID id, uint32_t tout_ms)
{
        HW_SDHC_STATUS ret = HW_SDHC_STATUS_ERROR;

        // Keep current state of error interrupt signal register and disable it
        uint16 err_sig = hw_sdhc_get_error_int_signal_en_r(id);
        hw_sdhc_set_error_int_signal_en_r(id, 0);

        uint16_t err_int = hw_sdhc_get_error_int_stat_r(id);

        if (hw_sdhc_get_normal_int_stat_r_err_interrupt(id)) {
                // Check for CMD Line error: CMD_TOUT_ERR, CMD_CRC_ERR, CMD_END_BIT_ERR, CMD_IDX_ERR, AUTO_CMD_ERR
                if (err_int & (BIT0 | BIT1 | BIT2 | BIT3 | BIT8)) {
                        if (!(err_int & BIT8)) {
                                if (!(hw_sdhc_get_xfer_mode_r_resp_int_disable(id) || hw_sdhc_get_host_ctrl2_r_exec_tuning(id))) {
                                        if (!hw_sdhc_get_normal_int_stat_r_cmd_complete(id)) {
                                                ret = hw_sdhc_wait_cmd_complete(id);
                                                if (HW_SDHC_STATUS_SUCCESS != ret) {
                                                        return ret;
                                                }
                                        }
                                }

                        }
                        hw_sdhc_set_and_wait_sw_rst_cmd(id);
                }

                // Check for DAT Line error: DATA_TOUT_ERR, DATA_CRC_ERR, DATA_END_BIT_ERR, ADMA_ERR
                if (err_int & (BIT4 | BIT5 | BIT6 | BIT9)) {
                        hw_sdhc_set_and_wait_sw_rst_dat(id);
                }

                hw_sdhc_clr_error_int_stat(id);
        }

        ret = hw_sdhc_abort_xfer_sync(id, tout_ms);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Wait command line is not inhibited
        ret = hw_sdhc_wait_cmd_line_not_inhibited(id);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Wait data line is not inhibited
        ret = hw_sdhc_wait_data_line_not_inhibited(id);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        ret = HW_SDHC_STATUS_NON_RECOVERABLE_ERROR;
        err_int = hw_sdhc_get_error_int_stat_r(id);

        // Check for CMD Line error: CMD_TOUT_ERR, CMD_CRC_ERR, CMD_END_BIT_ERR, CMD_IDX_ERR, DATA_TOUT_ERR
        if (!(err_int & (BIT0 | BIT1 | BIT2 | BIT3 | BIT4))) {
                hw_clk_delay_usec(HW_SDHC_DELAY_ERROR_RECOVERY_WAIT_DAT_LINE_US);
                if (hw_sdhc_get_pstate_dat_3_0(id) == 0xF) {
                        // Instead of HW_SDHC_STATUS_RECOVERABLE_ERROR, return HW_SDHC_STATUS_SUCCESS
                        ret = HW_SDHC_STATUS_SUCCESS;
                }
        }

        // Restore previous state of error interrupt signal register and Enable it
        hw_sdhc_set_error_int_signal_en_r(id, err_sig);

        return ret;
}


void hw_sdhc_interrupt_handler(HW_SDHC_ID id)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        uint32_t events = (uint32_t)hw_sdhc_get_normal_int_stat(id);

        if (events & context->normal_int_stat_mask) {
                if (events &
                        (HW_SDHC_EVENT_CARD_INSERTION | HW_SDHC_EVENT_CARD_REMOVAL |
#if (__HW_SDHC_USE_HW_EMMC_ONLY == 1)
                         HW_SDHC_EVENT_CARD_INTERRUPT |
#endif
                         HW_SDHC_EVENT_INT_A | HW_SDHC_EVENT_INT_B | HW_SDHC_EVENT_INT_C |
                         HW_SDHC_EVENT_RE_TUNE_EVENT | HW_SDHC_EVENT_FX_EVENT | HW_SDHC_EVENT_CQE_EVENT)) {
                        // These events are not supported/applicable and should not be enabled, see HW_EMMC_ACTIVE_NORMAL_INTERRUPTS_MASK
                        ASSERT_ERROR(0);
                }

#if (__HW_SDHC_USE_HW_EMMC_ONLY == 0)
                if (events & HW_SDHC_EVENT_CARD_INTERRUPT) {
                }
#endif

                if (events & HW_SDHC_EVENT_BGAP_EVENT) {
                        // IGNORE: This event occurs with XFER_COMPLETE, so it is handled below
                }

                if (hw_sdhc_get_error_int_stat_r_adma_err(id)) {
                        // If ADMA descriptors are invalid, then this error bit is set
                        // after sending the command and before starting data transfer
                        hw_sdhc_evt_complete(id, events | HW_SDHC_EVENT_ADMA2_ERROR);
                        return;
                }

                if (events & HW_SDHC_EVENT_DMA_INTERRUPT) {
                        hw_sdhc_evt_complete(id, events);
                        return;
                }

                if (events & HW_SDHC_EVENT_ERR_INTERRUPT) {
                        hw_sdhc_evt_complete(id, events);
                        return;
                }

                if (events & HW_SDHC_EVENT_CMD_COMPLETE) {
                        if (HW_SDHC_STATE_WAIT_CMD_COMPLETE == context->state) {
                                hw_sdhc_cmd_evt_handler(id, events);
                                return;
                        }
                }

                /* Read next data block... */
                if (events & HW_SDHC_EVENT_BUF_RD_READY) {
                        hw_sdhc_set_normal_int_stat_r_buf_rd_ready(id, true);

                        for (uint16_t blk_sz = 0; blk_sz < context->block_size; blk_sz += sizeof(uint32_t)) {
                                if (HW_SDHC_STATUS_SUCCESS != hw_sdhc_wait_buf_rd_enable(id)) {
                                        hw_sdhc_evt_complete(id, events | HW_SDHC_EVENT_BUF_RD_ENABLE_TIMEOUT);
                                        return;
                                }
                                // Ready to read 4 bytes from data buf...
                                *context->data = hw_sdhc_get_buf_dat_r(id);
                                context->data++;
                        }
                }

                /* Write next data block... */
                if (events & HW_SDHC_EVENT_BUF_WR_READY) {
                        hw_sdhc_set_normal_int_stat_r_buf_wr_ready(id, true);

                        for (uint16_t blk_sz = 0; blk_sz < context->block_size; blk_sz += sizeof(uint32_t)) {
                                if (HW_SDHC_STATUS_SUCCESS != hw_sdhc_wait_buf_wr_enable(id)) {
                                        hw_sdhc_evt_complete(id, events | HW_SDHC_EVENT_BUF_WR_ENABLE_TIMEOUT);
                                        return;
                                }
                                // Ready to write 4 bytes to data buf...
                                hw_sdhc_set_buf_dat_r(id, *context->data);
                                context->data++;
                        }
                }

                if (events & HW_SDHC_EVENT_XFER_COMPLETE) {
                        hw_sdhc_evt_complete(id, events);
                        return;
                }
        } else {
                ASSERT_ERROR(0);
        }
}

HW_SDHC_STATUS hw_sdhc_go_idle_state_CMD0(HW_SDHC_ID id)
{
        HW_SDHC_STATUS ret;
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = 0;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_NO_RESP;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = false;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = false;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_ABORT;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD0;
        cmd_config.read_resp = false;

        cmd_config.wait_cmd_complete = false;
        cmd_config.check_errors = false;
        cmd_config.wait_for_busy = false;

        ret = hw_sdhc_send_command(id, &cmd_config, NULL);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        hw_clk_delay_usec(HW_SDHC_DELAY_AFTER_CMD0_USEC);
        return ret;
}

HW_SDHC_STATUS hw_sdhc_select_deselect_card_CMD7(HW_SDHC_ID id, uint16_t rca, bool wait_for_busy, uint32_t busy_tout_ms)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = rca << HW_SDHC_RCA_CMD_ARG_POS;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = false;                // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;                 // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD7;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = wait_for_busy;
        cmd_config.busy_tout_ms = busy_tout_ms;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_set_block_count_CMD23(HW_SDHC_ID id, bool reliable_wr, uint32_t blk_cnt)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = (blk_cnt & 0xFFFFUL) | (reliable_wr ? (1UL << 31UL) : 0UL);
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD23;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

#if (dg_configUSE_HW_EMMC == 1)
HW_SDHC_STATUS hw_sdhc_send_op_cond_CMD1(HW_SDHC_ID id, uint32_t *ocr, uint32_t cmd_arg)
{
        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        hw_sdhc_cmd_config_t cmd_config = { 0 };

        if (!ocr) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        cmd_config.cmd_arg = cmd_arg;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = false;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = false;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD1;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        for (uint32_t i = 0; i < HW_SDHC_DELAY_1MS; i++) {
                HW_SDHC_STATUS ret = hw_sdhc_send_command(id, &cmd_config, ocr);
                if ((HW_SDHC_STATUS_SUCCESS != ret) && (context->card_status & HW_SDHC_CMD1_OCR_BUSY_MASK)) {
                        return HW_SDHC_STATUS_SUCCESS;
                }

                hw_clk_delay_usec(HW_SDHC_TOUT_SEND_OP_COND_CMD1_MS);
        }

        return HW_SDHC_STATUS_ERROR_TIMEOUT;
}

HW_SDHC_STATUS hw_sdhc_all_send_cid_CMD2(HW_SDHC_ID id, uint32_t *cid)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        if (!cid) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        cmd_config.cmd_arg = 0;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_136;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = false;       // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD2;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = false;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, cid);
}

HW_SDHC_STATUS hw_sdhc_set_relative_address_CMD3(HW_SDHC_ID id, uint16_t rca)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        if (!rca) {
                /* Relative card address 0x0000 is reserved */
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        cmd_config.cmd_arg = rca << HW_SDHC_RCA_CMD_ARG_POS;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD3;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_set_dsr_CMD4(HW_SDHC_ID id, uint16_t dsr)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = (dsr << HW_SDHC_DSR_CMD_ARG_POS);
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = false;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = false;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD4;
        cmd_config.read_resp = false;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = false;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_emmc_sleep_awake_CMD5(HW_SDHC_ID id, uint32_t rca, bool sleep, uint32_t tout_ms)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = rca << HW_SDHC_RCA_CMD_ARG_POS | ((sleep) ? 1 : 0) << 15;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD5;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = true;
        cmd_config.busy_tout_ms = tout_ms;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_emmc_switch_CMD6(HW_SDHC_ID id, const hw_sdhc_switch_cmd6_config_t *config)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        if (NULL == config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        cmd_config.cmd_arg = config->cmd_arg.cmd_set << HW_SDHC_CMD6_ARG_CMD_SET_POS |
                             config->cmd_arg.value << HW_SDHC_CMD6_ARG_VALUE_POS |
                             config->cmd_arg.index << HW_SDHC_CMD6_ARG_INDEX_POS |
                             config->cmd_arg.access << HW_SDHC_CMD6_ARG_ACCESS_POS;

        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD6;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = true;
        cmd_config.busy_tout_ms = config->tout_ms;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_set_emmc_speed_mode_CMD6(HW_SDHC_ID id, HW_SDHC_HOST_CTRL2_R_EMMC_BUS_SPEED_MODE_SEL speed_mode, uint8_t hs_timing, uint32_t tout_ms)
{
        ASSERT_WARNING(speed_mode <= HW_SDHC_EMMC_BUS_SPEED_MODE_SEL_HS400);

        HW_SDHC_STATUS ret;
        hw_sdhc_switch_cmd6_config_t cmd6_config = { 0 };

        cmd6_config.tout_ms = tout_ms;
        cmd6_config.cmd_arg.cmd_set = 0;
        cmd6_config.cmd_arg.value = hs_timing;
        cmd6_config.cmd_arg.index = HW_SDHC_EMMC_EXT_CSD_HS_TIMING_IDX;
        cmd6_config.cmd_arg.access = HW_SDHC_CMD6_ACCESS_WRITE_BYTE;

        ret = hw_sdhc_emmc_switch_CMD6(id, &cmd6_config);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Change clock frequency
        // Change frequency afterwards, outside this function

        // Set speed mode at host...
        hw_sdhc_set_host_ctrl1_r_high_speed_en(id, hw_sdhc_get_capabilities1_r_high_speed_support(id));
        hw_sdhc_set_host_ctrl2_r_uhs_mode_sel(id, speed_mode);

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_set_emmc_data_bus_width_CMD6(HW_SDHC_ID id, HW_SDHC_BUS_WIDTH bus_width, uint32_t tout_ms)
{
#if (HW_SDHC_SUPPORT_DDR == 1)
        ASSERT_WARNING(bus_width <= HW_SDHC_BUS_WIDTH_8_BIT_DDR);
#else
        ASSERT_WARNING(bus_width <= HW_SDHC_BUS_WIDTH_8_BIT);
#endif

        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        HW_SDHC_STATUS ret;
        hw_sdhc_switch_cmd6_config_t cmd6_config = { 0 };

        cmd6_config.tout_ms = tout_ms;
        cmd6_config.cmd_arg.cmd_set = 0;
        cmd6_config.cmd_arg.value = bus_width;
        cmd6_config.cmd_arg.index = HW_SDHC_EMMC_EXT_CSD_BUS_WIDTH_IDX;
        cmd6_config.cmd_arg.access = HW_SDHC_CMD6_ACCESS_WRITE_BYTE;

        ret = hw_sdhc_emmc_switch_CMD6(id, &cmd6_config);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Set bus width at Host...
        hw_sdhc_set_bus_width_at_host(id, bus_width);
        context->bus_width = bus_width;

        return HW_SDHC_STATUS_SUCCESS;
}

HW_SDHC_STATUS hw_sdhc_emmc_send_ext_csd_CMD8(HW_SDHC_ID id, uint16_t rca, uint8_t *ext_csd)
{
        HW_SDHC_STATUS ret;

        hw_sdhc_cmd_config_t cmd_config = { 0 };
        hw_sdhc_data_transfer_config_t xfer_config;

        if (!ext_csd) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

//        xfer_config.address = 0;              // Read a register and not from Memory
        xfer_config.auto_command = false;
        xfer_config.block_cnt = 1;
        xfer_config.block_size = HW_SDHC_EXT_CSD_SIZE;
        xfer_config.data = ext_csd;
        xfer_config.tout_cnt_time = (1 << 27) / hw_sdhc_get_capabilities1_r_tout_clk_freq(id);
        xfer_config.dma_en = false;
        xfer_config.xfer_dir = HW_SDHC_DATA_XFER_DIR_READ;
        xfer_config.page_bdary = HW_SDHC_SDMA_BUF_BDARY_512KB;  // Not used in this case

        ret = hw_sdhc_data_xfer_init(id, &xfer_config);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                return ret;
        }

        cmd_config.cmd_arg = rca << HW_SDHC_RCA_CMD_ARG_POS;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = true;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD8;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        ret = hw_sdhc_send_command(id, &cmd_config, NULL);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                return ret;
        }

        return hw_sdhc_data_xfer_start_non_dma_blocking(id, &xfer_config);
}

HW_SDHC_STATUS hw_sdhc_send_csd_CMD9(HW_SDHC_ID id, uint16_t rca, uint32_t *csd)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        if (!csd) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        cmd_config.cmd_arg = rca << HW_SDHC_RCA_CMD_ARG_POS;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_136;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = false;       // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD9;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = false;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, csd);
}

HW_SDHC_STATUS hw_sdhc_send_cid_CMD10(HW_SDHC_ID id, uint16_t rca, uint32_t *cid)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        if (!cid) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        cmd_config.cmd_arg = rca << HW_SDHC_RCA_CMD_ARG_POS;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_136;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = false;       // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD10;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = false;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, cid);
}

HW_SDHC_STATUS hw_sdhc_stop_transmission_CMD12(HW_SDHC_ID id, uint16_t rca, bool hpi, uint32_t tout_ms)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = (rca << HW_SDHC_RCA_CMD_ARG_POS);
        if (hpi) {
                cmd_config.cmd_arg |= 1;
        }
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_ABORT;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD12;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = true;
        cmd_config.busy_tout_ms = tout_ms;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_send_status_CMD13(HW_SDHC_ID id, uint16_t rca, bool hpi, uint32_t *card_status)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        if (!card_status) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        cmd_config.cmd_arg = (rca << HW_SDHC_RCA_CMD_ARG_POS);
        if (hpi) {
                cmd_config.cmd_arg |= 1;
        }
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD13;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = false;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, card_status);
}

HW_SDHC_STATUS hw_sdhc_go_inactive_state_CMD15(HW_SDHC_ID id, uint16_t rca)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = (rca << HW_SDHC_RCA_CMD_ARG_POS);
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = false;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = false;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD15;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_set_blocklen_CMD16(HW_SDHC_ID id, uint32_t blk_len)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = blk_len;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD16;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_program_cid_CMD26(HW_SDHC_ID id, const uint8_t *buf, uint32_t tout_ms)
{
        return hw_sdhc_emmc_program_cid_csd(id, buf, tout_ms, HW_SDHC_PROGRAM_CID);
}

HW_SDHC_STATUS hw_sdhc_program_csd_CMD27(HW_SDHC_ID id, const uint8_t *buf, uint32_t tout_ms)
{
        return hw_sdhc_emmc_program_cid_csd(id, buf, tout_ms, HW_SDHC_PROGRAM_CSD);
}

static HW_SDHC_STATUS hw_sdhc_emmc_program_cid_csd(HW_SDHC_ID id, const uint8_t *buf, uint32_t tout_ms, HW_SDHC_PROGRAM_CID_CSD cid_csd)
{
        HW_SDHC_STATUS ret;

        hw_sdhc_cmd_config_t cmd_config = { 0 };
        hw_sdhc_data_transfer_config_t xfer_config;

        if (!buf) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        xfer_config.auto_command = false;
        xfer_config.block_cnt = 1;
        xfer_config.block_size = (cid_csd == HW_SDHC_PROGRAM_CID) ? HW_SDHC_CID_SIZE : HW_SDHC_CSD_SIZE;
        xfer_config.data = (uint8_t *)buf;
        xfer_config.tout_cnt_time = (1 << 27) / hw_sdhc_get_capabilities1_r_tout_clk_freq(id);
        xfer_config.xfer_tout_ms = tout_ms;
        xfer_config.dma_en = false;
        xfer_config.xfer_dir = HW_SDHC_DATA_XFER_DIR_WRITE;
        xfer_config.page_bdary = HW_SDHC_SDMA_BUF_BDARY_512KB;  // Not used in this case

        ret = hw_sdhc_data_xfer_init(id, &xfer_config);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                return ret;
        }

        cmd_config.cmd_arg = 0;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = true;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = (cid_csd == HW_SDHC_PROGRAM_CID) ? HW_SDHC_CMD_INDEX_CMD26 : HW_SDHC_CMD_INDEX_CMD27;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        ret = hw_sdhc_send_command(id, &cmd_config, NULL);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                return ret;
        }

        return hw_sdhc_data_xfer_start_non_dma_blocking(id, &xfer_config);
}

HW_SDHC_STATUS hw_sdhc_set_write_prot_CMD28(HW_SDHC_ID id, uint32_t data_addr, uint32_t tout_ms)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = data_addr;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD28;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = true;
        cmd_config.busy_tout_ms = tout_ms;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_clr_write_prot_CMD29(HW_SDHC_ID id, uint32_t data_addr, uint32_t tout_ms)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = data_addr;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD29;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = true;
        cmd_config.busy_tout_ms = tout_ms;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_send_write_prot_CMD30(HW_SDHC_ID id, uint32_t wp_addr, uint32_t *wp_status, uint32_t tout_ms)
{
        HW_SDHC_STATUS ret;

        hw_sdhc_cmd_config_t cmd_config = { 0 };
        hw_sdhc_data_transfer_config_t xfer_config;

        if (!wp_status) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        xfer_config.auto_command = false;
        xfer_config.block_cnt = 1;
        xfer_config.block_size = sizeof(uint32_t);      // Read 32 write protection bits
        xfer_config.data = (uint8_t *)wp_status;
        xfer_config.tout_cnt_time = (1 << 27) / hw_sdhc_get_capabilities1_r_tout_clk_freq(id);
        xfer_config.xfer_tout_ms = tout_ms;
        xfer_config.dma_en = false;
        xfer_config.xfer_dir = HW_SDHC_DATA_XFER_DIR_READ;
        xfer_config.page_bdary = HW_SDHC_SDMA_BUF_BDARY_512KB;  // Not used in this case

        ret = hw_sdhc_data_xfer_init(id, &xfer_config);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                return ret;
        }

        cmd_config.cmd_arg = wp_addr;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = true;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD30;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.cmd_complete_delay = 0;      // HW_SDHC_TOUT_CMD_COMPLETE_MS;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        ret = hw_sdhc_send_command(id, &cmd_config, NULL);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                hw_sdhc_reset_evt_handler(id, true);
                return ret;
        }
        return hw_sdhc_data_xfer_start_non_dma_blocking(id, &xfer_config);
}

HW_SDHC_STATUS hw_sdhc_send_write_prot_type_CMD31(HW_SDHC_ID id, uint32_t wp_addr, uint64_t *wp_type, uint32_t tout_ms)
{
        HW_SDHC_STATUS ret;

        hw_sdhc_cmd_config_t cmd_config = { 0 };
        hw_sdhc_data_transfer_config_t xfer_config;

        if (!wp_type) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        xfer_config.auto_command = false;
        xfer_config.block_cnt = 1;
        xfer_config.block_size = sizeof(uint64_t);      // Read 64 bits of groups write protection type
        xfer_config.data = (uint8_t *)wp_type;
        xfer_config.tout_cnt_time = (1 << 27) / hw_sdhc_get_capabilities1_r_tout_clk_freq(id);
        xfer_config.xfer_tout_ms = tout_ms;
        xfer_config.dma_en = false;
        xfer_config.xfer_dir = HW_SDHC_DATA_XFER_DIR_READ;
        xfer_config.page_bdary = HW_SDHC_SDMA_BUF_BDARY_512KB;  // Not used in this case

        ret = hw_sdhc_data_xfer_init(id, &xfer_config);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                return ret;
        }

        cmd_config.cmd_arg = wp_addr;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = true;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD31;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        ret = hw_sdhc_send_command(id, &cmd_config, NULL);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                hw_sdhc_reset_evt_handler(id, true);
                return ret;
        }
        return hw_sdhc_data_xfer_start_non_dma_blocking(id, &xfer_config);
}

static HW_SDHC_STATUS hw_sdhc_CMD35_CMD36(HW_SDHC_ID id, uint32_t data_addr, uint32_t cmd_index)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        cmd_config.cmd_arg = data_addr;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = cmd_index;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

HW_SDHC_STATUS hw_sdhc_erase_group_start_CMD35(HW_SDHC_ID id, uint32_t data_addr)
{
        return hw_sdhc_CMD35_CMD36(id, data_addr, HW_SDHC_CMD_INDEX_CMD35);
}

HW_SDHC_STATUS hw_sdhc_erase_group_end_CMD36(HW_SDHC_ID id, uint32_t data_addr)
{
        return hw_sdhc_CMD35_CMD36(id, data_addr, HW_SDHC_CMD_INDEX_CMD36);
}

HW_SDHC_STATUS hw_sdhc_erase_CMD38(HW_SDHC_ID id, HW_SDHC_CMD38_ARG arg, uint32_t tout_ms)
{
        hw_sdhc_cmd_config_t cmd_config = { 0 };

        if (arg != HW_SDHC_CMD38_ARG_ERASE &&
            arg != HW_SDHC_CMD38_ARG_TRIM &&
            arg != HW_SDHC_CMD38_ARG_SECURE_ERASE &&
            arg != HW_SDHC_CMD38_ARG_SECURE_TRIM_STEP_1 &&
            arg != HW_SDHC_CMD38_ARG_SECURE_TRIM_STEP_2 ) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        cmd_config.cmd_arg = arg;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = false;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD38;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = true;
        cmd_config.busy_tout_ms = tout_ms;

        return hw_sdhc_send_command(id, &cmd_config, NULL);
}

static HW_SDHC_STATUS hw_sdhc_data_xfer_CMD42(HW_SDHC_ID id, const hw_sdhc_data_transfer_config_t *config)
{
        HW_SDHC_STATUS ret;

        hw_sdhc_context_data_t *context = HW_SDHC_DATA(id);
        ASSERT_WARNING(context);

        if (!config) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        context->data = (uint32_t *)config->data;

        // Non-DMA blocking write...
        for (uint32_t blk_cnt = 0; blk_cnt < config->block_cnt; blk_cnt++) {
                ret = hw_sdhc_wait_buf_wr_ready(id);
                if (HW_SDHC_STATUS_SUCCESS != ret) {
                        return ret;
                }
                for (uint16_t blk_sz = 0; blk_sz < config->block_size; blk_sz += sizeof(uint32_t)) {
                        ret = hw_sdhc_wait_buf_wr_enable(id);
                        if (HW_SDHC_STATUS_SUCCESS != ret) {
                                return ret;
                        }

                        hw_sdhc_set_buf_dat_r(id, *context->data);       // write 4 bytes
                        context->data++;
                }
        }

        // Wait for transfer complete interrupt
        ret = hw_sdhc_wait_xfer_complete(id, config->xfer_tout_ms);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Wait command line is not inhibited
        ret = hw_sdhc_wait_cmd_line_not_inhibited(id);
        if (HW_SDHC_STATUS_SUCCESS != ret) {
                return ret;
        }

        // Wait data line is not inhibited
        return hw_sdhc_wait_data_line_not_inhibited(id);
}

HW_SDHC_STATUS hw_sdhc_lock_unlock_CMD42(HW_SDHC_ID id, uint8_t len, uint8_t *data, uint32_t tout_ms)
{
        HW_SDHC_STATUS ret;

        hw_sdhc_cmd_config_t cmd_config = { 0 };
        hw_sdhc_data_transfer_config_t xfer_config;

        if (!len || (len > HW_SDHC_CMD42_LEN_MAX) || !data) {
                return HW_SDHC_STATUS_ERROR_INVALID_PARAMETER;
        }

        xfer_config.auto_command = false;
        xfer_config.block_cnt = 1;
        xfer_config.block_size = len;
        xfer_config.data = data;
        xfer_config.tout_cnt_time = (1 << 27) / hw_sdhc_get_capabilities1_r_tout_clk_freq(id);
        xfer_config.xfer_tout_ms = tout_ms;
        xfer_config.dma_en = false;
        xfer_config.xfer_dir = HW_SDHC_DATA_XFER_DIR_WRITE;
        xfer_config.page_bdary = HW_SDHC_SDMA_BUF_BDARY_512KB;  // Not used in this case

        ret = hw_sdhc_data_xfer_init(id, &xfer_config);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                return ret;
        }

        cmd_config.cmd_arg = 0;
        cmd_config.resp_type = HW_SDHC_RESP_TYPE_SELECT_RESP_LEN_48;
        cmd_config.sub_cmd_flag = HW_SDHC_SUB_CMD_FLAG_MAIN;
        cmd_config.crc_check_en = true;        // Set to 0 for the command with no response, R3 and R4
        cmd_config.idx_check_en = true;        // Set to 0 for the command with no response, R2, R3 and R4
        cmd_config.data_present = true;
        cmd_config.cmd_type = HW_SDHC_CMD_TYPE_NORMAL;
        cmd_config.cmd_index = HW_SDHC_CMD_INDEX_CMD42;
        cmd_config.read_resp = true;

        cmd_config.wait_cmd_complete = true;
        cmd_config.cmd_complete_delay = 0;      //HW_SDHC_TOUT_CMD_COMPLETE_MS;
        cmd_config.check_errors = true;
        cmd_config.wait_for_busy = false;

        ret = hw_sdhc_send_command(id, &cmd_config, NULL);
        if (ret != HW_SDHC_STATUS_SUCCESS) {
                hw_sdhc_reset_evt_handler(id, true);
                return ret;
        }
        return hw_sdhc_data_xfer_CMD42(id, &xfer_config);
}

#endif


#endif /* (dg_configUSE_HW_EMMC == 1) || (dg_configUSE_HW_SDIO == 1) */
