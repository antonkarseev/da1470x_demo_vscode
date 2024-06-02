/**
 ****************************************************************************************
 *
 * @file hw_oqspi.c
 *
 * @brief Implementation of the OQSPI Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if (dg_configUSE_HW_OQSPI)

#include <stdint.h>
#include "hw_oqspi.h"

__RETAINED_CODE void hw_oqspi_init(const hw_oqspi_config_t *cfg)
{
        if (cfg) {
                ASSERT_WARNING(IS_HW_OQSPI_ADDR_SIZE(cfg->address_size));
                ASSERT_WARNING(IS_HW_OQSPI_CLK_DIV(cfg->clk_div));
                ASSERT_WARNING(IS_HW_OQSPI_CLK_MODE(cfg->clock_mode));
                ASSERT_WARNING(IS_HW_OQSPI_DRIVE_CURRENT(cfg->drive_current));
                ASSERT_WARNING(IS_HW_OQSPI_OPCODE_LEN(cfg->opcode_len));
                ASSERT_WARNING(IS_HW_OQSPI_READ_PIPE(cfg->read_pipe));
                ASSERT_WARNING(IS_HW_OQSPI_READ_PIPE_DELAY(cfg->read_pipe_delay));
                ASSERT_WARNING(IS_HW_OQSPI_SAMPLING_EDGE(cfg->sampling_edge));
                ASSERT_WARNING(IS_HW_OQSPI_SLEW_RATE(cfg->slew_rate));
                ASSERT_WARNING(IS_HW_OQSPI_BURST_LEN_LIMIT(cfg->auto_mode_cfg.burst_len_limit));
                ASSERT_WARNING(IS_HW_OQSPI_FULL_BUFFER_MODE(cfg->auto_mode_cfg.full_buffer_mode));
                ASSERT_WARNING(IS_HW_OQSPI_DIR_CHANGE_MODE(cfg->manual_mode_cfg.dir_change_mode));
                ASSERT_WARNING(IS_HW_OQSPI_DUMMY_MODE(cfg->manual_mode_cfg.dummy_mode));
                ASSERT_WARNING(IS_HW_OQSPI_HREADY_MODE(cfg->manual_mode_cfg.hready_mode));
                ASSERT_WARNING(IS_HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE(cfg->manual_mode_cfg.mapped_addr_rd_acc_response));

                uint32_t ctrlmode_reg = OQSPIF->OQSPIF_CTRLMODE_REG;
                uint32_t gp_reg = OQSPIF->OQSPIF_GP_REG;

                hw_oqspi_set_div(cfg->clk_div);
                hw_oqspi_clock_enable();

                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_USE_32BA, ctrlmode_reg, cfg->address_size);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_CMD_X2_EN, ctrlmode_reg, cfg->opcode_len);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_CLK_MD, ctrlmode_reg, cfg->clock_mode);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_RXD_NEG, ctrlmode_reg, cfg->sampling_edge);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_RPIPE_EN, ctrlmode_reg, cfg->read_pipe);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_PCLK_MD, ctrlmode_reg, cfg->read_pipe_delay);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_HRDY_MD, ctrlmode_reg, cfg->manual_mode_cfg.hready_mode);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_DMY_MD, ctrlmode_reg, cfg->manual_mode_cfg.dummy_mode);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_MAN_DIRCHG_MD, ctrlmode_reg, cfg->manual_mode_cfg.dir_change_mode);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_RD_ERR_EN, ctrlmode_reg, cfg->manual_mode_cfg.mapped_addr_rd_acc_response);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_BUF_LIM_EN, ctrlmode_reg, cfg->auto_mode_cfg.full_buffer_mode);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_INC_LIM_EN, ctrlmode_reg, cfg->auto_mode_cfg.burst_len_limit);

                REG_SET_FIELD(OQSPIF, OQSPIF_GP_REG, OSPIC_PADS_DRV, gp_reg, cfg->drive_current);
                REG_SET_FIELD(OQSPIF, OQSPIF_GP_REG, OSPIC_PADS_SLEW , gp_reg, cfg->slew_rate);

                OQSPIF->OQSPIF_CTRLMODE_REG = ctrlmode_reg;
                OQSPIF->OQSPIF_GP_REG = gp_reg;
        }
}

__RETAINED_CODE void hw_oqspi_erase_block(uint32_t addr)
{
        if (hw_oqspi_get_access_mode() != HW_OQSPI_ACCESS_MODE_AUTO) {
                hw_oqspi_set_access_mode(HW_OQSPI_ACCESS_MODE_AUTO);
        }

        // Wait for previous erase to end
        while (hw_oqspi_get_erase_status() != HW_OQSPI_ERASE_STATUS_NO) {
        }

        uint32_t block_sector = addr >> 12;
        switch (hw_oqspi_get_address_size()) {
        case HW_OQSPI_ADDR_SIZE_24:
                ASSERT_WARNING(addr <= 0x00FFFFFF);
                // OQSPIF_ERASECTRL_REG bits 23-12 determine the block/sector address bits (23-12)
                block_sector <<= 8;
                break;
        case HW_OQSPI_ADDR_SIZE_32:
                ASSERT_WARNING(addr < (MEMORY_OQSPIC_S_END - MEMORY_OQSPIC_S_BASE));
                // OQSPIF_ERASECTRL_REG bits 23-4 determine the block/sector address bits (31-12)
                break;
        default:
                ASSERT_WARNING(0);
        }

        hw_oqspi_set_erase_address(block_sector);
        hw_oqspi_trigger_erase();
}

static uint32_t buf_to_word(const uint8 *data)
{
        ASSERT_WARNING(data);
        if ((uint32_t)data & 0x3) {
                return SWAP32(__UNALIGNED_UINT32_READ(data));
        } else {
                return SWAP32(*(uint32_t *)data);
        }
}

__RETAINED_CODE void hw_oqspi_set_aes_ctr_nonce(const uint8_t *nonce)
{
        ASSERT_WARNING(nonce);

        REG_SETF(OQSPIF, OQSPIF_CTR_NONCE_0_3_REG, OSPIC_CTR_NONCE_0_3, buf_to_word(nonce));
        REG_SETF(OQSPIF, OQSPIF_CTR_NONCE_4_7_REG, OSPIC_CTR_NONCE_4_7, buf_to_word((uint8_t *) (nonce + 4)));
}

__RETAINED_CODE void hw_oqspi_set_aes_ctr_key(const uint8_t *key)
{
        ASSERT_WARNING(key);

        REG_SETF(OQSPIF, OQSPIF_CTR_KEY_0_3_REG, OSPIC_CTR_KEY_0_3, buf_to_word(key));
        REG_SETF(OQSPIF, OQSPIF_CTR_KEY_4_7_REG, OSPIC_CTR_KEY_4_7, buf_to_word((uint8_t *) (key + 4)));
        REG_SETF(OQSPIF, OQSPIF_CTR_KEY_8_11_REG, OSPIC_CTR_KEY_8_11, buf_to_word((uint8_t *) (key + 8)));
        REG_SETF(OQSPIF, OQSPIF_CTR_KEY_12_15_REG, OSPIC_CTR_KEY_12_15, buf_to_word((uint8_t *) (key + 12)));
        REG_SETF(OQSPIF, OQSPIF_CTR_KEY_16_19_REG, OSPIC_CTR_KEY_16_19, buf_to_word((uint8_t *) (key + 16)));
        REG_SETF(OQSPIF, OQSPIF_CTR_KEY_20_23_REG, OSPIC_CTR_KEY_20_23, buf_to_word((uint8_t *) (key + 20)));
        REG_SETF(OQSPIF, OQSPIF_CTR_KEY_24_27_REG, OSPIC_CTR_KEY_24_27, buf_to_word((uint8_t *) (key + 24)));
        REG_SETF(OQSPIF, OQSPIF_CTR_KEY_28_31_REG, OSPIC_CTR_KEY_28_31, buf_to_word((uint8_t *) (key + 28)));
}

__RETAINED_CODE void hw_oqspi_aes_ctr_init(const hw_oqspi_aes_ctr_config_t *cfg)
{
        hw_oqspi_set_aes_ctr_nonce(cfg->nonce);
        hw_oqspi_set_aes_ctr_key(cfg->key);
        hw_oqspi_set_aes_ctr_addr_range(cfg->start_addr, cfg->end_addr);
}

#endif /* dg_configUSE_HW_OQSPI */
