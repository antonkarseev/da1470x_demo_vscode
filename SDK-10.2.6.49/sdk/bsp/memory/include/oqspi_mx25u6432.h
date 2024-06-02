/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup PLA_MEMORY
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file oqspi_mx25u6432.h
 *
 * @brief OQSPI flash driver for Macronix MX25U6432
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _OQSPI_MX25U6432_H_
#define _OQSPI_MX25U6432_H_

#include "hw_clk.h"
#include "hw_oqspi.h"
#include "oqspi_common.h"
#include "oqspi_macronix_quad.h"

#define OQSPI_MX25U6432_DENSITY                         (0x37)

#define OQSPI_MX25U6432_DUMMY_BYTES_POS                 (6)
#define OQSPI_MX25U6432_DUMMY_BYTES_MASK                (0x3 << OQSPI_MX25U6432_DUMMY_BYTES_POS)

typedef enum {
        OQSPI_MX25U6432_DUMMY_BYTES_2 = 0x00,
        OQSPI_MX25U6432_DUMMY_BYTES_1 = 0x01,
        OQSPI_MX25U6432_DUMMY_BYTES_3 = 0x02,
        OQSPI_MX25U6432_DUMMY_BYTES_4 = 0x03,
        OQSPI_MX25U6432_DUMMY_BYTES_INVALID  = 0xFF,
} OQSPI_MX25U6432_DUMMY_BYTES;

__RETAINED_CODE static void oqspi_mx25u6432_initialize(HW_OQSPI_BUS_MODE bus_mode, sys_clk_t sys_clk);
__RETAINED_CODE static void oqspi_mx25u6432_sys_clock_cfg(sys_clk_t sys_clk);
__RETAINED_CODE static uint8_t oqspi_mx25u6432_get_dummy_bytes(sys_clk_t sys_clk);

static const oqspi_flash_config_t oqspi_mx25u6432_cfg = {
        .jedec.manufacturer_id                                  = OQSPI_MACRONIX_QUAD_MANUFACTURER_ID,
        .jedec.type                                             = OQSPI_MACRONIX_QUAD_MX25U_TYPE,
        .jedec.density                                          = OQSPI_MX25U6432_DENSITY,
        .jedec.density_mask                                     = 0xFF,

        .size_mbits                                             = OQSPI_MEMORY_SIZE_64Mbits,
        .address_size                                           = HW_OQSPI_ADDR_SIZE_24,
        .clk_mode                                               = HW_OQSPI_CLK_MODE_LOW,
        .opcode_len                                             = HW_OQSPI_OPCODE_LEN_1_BYTE,

        .read_instr_cfg.opcode_bus_mode                         = HW_OQSPI_BUS_MODE_SINGLE,
        .read_instr_cfg.addr_bus_mode                           = HW_OQSPI_BUS_MODE_QUAD,
        .read_instr_cfg.extra_byte_bus_mode                     = HW_OQSPI_BUS_MODE_QUAD,
        .read_instr_cfg.dummy_bus_mode                          = HW_OQSPI_BUS_MODE_QUAD,
        .read_instr_cfg.data_bus_mode                           = HW_OQSPI_BUS_MODE_QUAD,
        .read_instr_cfg.continuous_mode                         = HW_OQSPI_CONTINUOUS_MODE_ENABLE,
        .read_instr_cfg.extra_byte_cfg                          = HW_OQSPI_EXTRA_BYTE_ENABLE,
        .read_instr_cfg.extra_byte_half_cfg                     = HW_OQSPI_EXTRA_BYTE_HALF_DISABLE,
        .read_instr_cfg.opcode                                  = OQSPI_FAST_READ_QUAD_OPCODE,
        .read_instr_cfg.extra_byte_value                        = 0xA5,
        .read_instr_cfg.cs_idle_delay_nsec                      = 7,    // tSHSL (read)

        .erase_instr_cfg.opcode_bus_mode                        = HW_OQSPI_BUS_MODE_SINGLE,
        .erase_instr_cfg.addr_bus_mode                          = HW_OQSPI_BUS_MODE_SINGLE,
        .erase_instr_cfg.hclk_cycles                            = 0,
        .erase_instr_cfg.opcode                                 = OQSPI_SECTOR_ERASE_OPCODE,
        .erase_instr_cfg.cs_idle_delay_nsec                     = 30,   // tSHSL (erase)

        .read_status_instr_cfg.opcode_bus_mode                  = HW_OQSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.receive_bus_mode                 = HW_OQSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.dummy_bus_mode                   = HW_OQSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.dummy_value                      = HW_OQSPI_READ_STATUS_DUMMY_VAL_UNCHANGED,
        .read_status_instr_cfg.busy_level                       = HW_OQSPI_BUSY_LEVEL_HIGH,
        .read_status_instr_cfg.busy_pos                         = OQSPI_STATUS_REG_BUSY_BIT,
        .read_status_instr_cfg.dummy_bytes                      = 0,
        .read_status_instr_cfg.opcode                           = OQSPI_READ_STATUS_REG_OPCODE,
        .read_status_instr_cfg.delay_nsec                       = 0,

        .write_enable_instr_cfg.opcode_bus_mode                 = HW_OQSPI_BUS_MODE_SINGLE,
        .write_enable_instr_cfg.opcode                          = OQSPI_WRITE_ENABLE_OPCODE,

        .page_program_instr_cfg.opcode_bus_mode                 = HW_OQSPI_BUS_MODE_SINGLE,
        .page_program_instr_cfg.addr_bus_mode                   = HW_OQSPI_BUS_MODE_QUAD,
        .page_program_instr_cfg.data_bus_mode                   = HW_OQSPI_BUS_MODE_QUAD,
        .page_program_instr_cfg.opcode                          = OQSPI_MACRONIX_QUAD_PAGE_PROGRAM_4IO_OPCODE,

        .suspend_resume_instr_cfg.suspend_bus_mode              = HW_OQSPI_BUS_MODE_SINGLE,
        .suspend_resume_instr_cfg.resume_bus_mode               = HW_OQSPI_BUS_MODE_SINGLE,
        .suspend_resume_instr_cfg.suspend_opcode                = OQSPI_MACRONIX_QUAD_SUSPEND_OPCODE,
        .suspend_resume_instr_cfg.resume_opcode                 = OQSPI_MACRONIX_QUAD_RESUME_OPCODE,
        .suspend_resume_instr_cfg.suspend_latency_usec          = 25,   // tESL
        .suspend_resume_instr_cfg.resume_latency_usec           = 1,    // no latency
        .suspend_resume_instr_cfg.res_sus_latency_usec          = 100,  // tERS

        .exit_continuous_mode_instr_cfg.opcode_bus_mode         = HW_OQSPI_BUS_MODE_QUAD,
        .exit_continuous_mode_instr_cfg.sequence_len            = 4,
        .exit_continuous_mode_instr_cfg.disable_second_half     = 0,
        .exit_continuous_mode_instr_cfg.opcode                  = 0xFF,

        .delay.reset_usec                                       = 12000, // tREADY2
        .delay.power_down_usec                                  = 10,    // tDP
        .delay.release_power_down_usec                          = 30,    // tRDP
        .delay.power_up_usec                                    = 800,   // tVSL

        .callback.initialize_cb                                 = oqspi_mx25u6432_initialize,
        .callback.sys_clk_cfg_cb                                = oqspi_mx25u6432_sys_clock_cfg,
        .callback.exit_opi_qpi_cb                               = oqspi_exit_qpi,
        .callback.get_dummy_bytes_cb                            = oqspi_mx25u6432_get_dummy_bytes,
        .callback.is_suspended_cb                               = oqspi_macronix_quad_is_suspended,
        .callback.is_busy_cb                                    = oqspi_macronix_quad_is_busy,
        .callback.read_status_reg_cb                            = oqspi_macronix_quad_read_status_reg,
        .callback.write_status_reg_cb                           = oqspi_macronix_quad_write_status_reg,

        .resume_before_writing_regs                             = true,
};

__RETAINED_CODE static uint8_t oqspi_mx25u6432_get_dummy_bytes(sys_clk_t sys_clk)
{
        switch (sys_clk) {
        case sysclk_RCHS_32:
        case sysclk_RCHS_64:
        case sysclk_XTAL32M:
                return 1;
        case sysclk_RCHS_96:
                return 3;
        // When the PLL160 is used as system clock the OQSPIC switches to clock divider 2 thus
        // the OQSPIC clock frequency is 80 MHz.
        case sysclk_PLL160:
                return 2;
        default:
                ASSERT_WARNING(0);
                return 0;
        }
}

__RETAINED_CODE static bool oqspi_mx25u6432_set_dummy_bytes(uint8_t dummy_bytes)
{
        __DBG_OQSPI_VOLATILE__ uint8_t status_reg;
        __DBG_OQSPI_VOLATILE__ uint8_t config_reg;
        __DBG_OQSPI_VOLATILE__ uint8_t verify_config_reg;
        __DBG_OQSPI_VOLATILE__ OQSPI_MX25U6432_DUMMY_BYTES mx25u6432_dummy_bytes = OQSPI_MX25U6432_DUMMY_BYTES_INVALID;

        switch (dummy_bytes) {
        case 1:
                mx25u6432_dummy_bytes = OQSPI_MX25U6432_DUMMY_BYTES_1;
                break;
        case 2:
                mx25u6432_dummy_bytes = OQSPI_MX25U6432_DUMMY_BYTES_2;
                break;
        case 3:
                mx25u6432_dummy_bytes = OQSPI_MX25U6432_DUMMY_BYTES_3;
                break;
        case 4:
                mx25u6432_dummy_bytes = OQSPI_MX25U6432_DUMMY_BYTES_4;
                break;
        default:
                return false;
        }

        oqspi_flash_write_enable(HW_OQSPI_BUS_MODE_SINGLE);

        status_reg = oqspi_macronix_quad_read_register(OQSPI_READ_STATUS_REG_OPCODE, 0xFF);
        config_reg = oqspi_macronix_quad_read_register(OQSPI_MACRONIX_QUAD_READ_CONFIG_REG_OPCODE, 0xFF);

        config_reg = (config_reg & ~OQSPI_MX25U6432_DUMMY_BYTES_MASK) |
                     (mx25u6432_dummy_bytes << OQSPI_MX25U6432_DUMMY_BYTES_POS);

        oqspi_macronix_quad_write_status_and_config_reg(status_reg, config_reg);
        while (oqspi_flash_is_busy(HW_OQSPI_BUS_MODE_SINGLE));
        verify_config_reg = oqspi_macronix_quad_read_register(OQSPI_MACRONIX_QUAD_READ_CONFIG_REG_OPCODE, 0xFF);

        return (verify_config_reg == config_reg);
}

__RETAINED_CODE static void oqspi_mx25u6432_enable_quad_mode(void)
{
        __DBG_OQSPI_VOLATILE__ uint8_t status_reg;

        status_reg = oqspi_macronix_quad_read_register(OQSPI_READ_STATUS_REG_OPCODE, 0xFF);

        if (!(status_reg & OQSPI_MACRONIX_QUAD_STATUS_REG_QUAD_ENABLE_MASK)) {

                __DBG_OQSPI_VOLATILE__ uint8_t config_reg;

                config_reg = oqspi_macronix_quad_read_register(OQSPI_MACRONIX_QUAD_READ_CONFIG_REG_OPCODE, 0xFF);
                status_reg |= OQSPI_MACRONIX_QUAD_STATUS_REG_QUAD_ENABLE_MASK;

                oqspi_flash_write_enable(HW_OQSPI_BUS_MODE_SINGLE);
                oqspi_macronix_quad_write_status_and_config_reg(status_reg, config_reg);
                while (oqspi_macronix_quad_is_busy(HW_OQSPI_BUS_MODE_SINGLE, HW_OQSPI_BUSY_LEVEL_HIGH));

                status_reg = oqspi_macronix_quad_read_register(OQSPI_READ_STATUS_REG_OPCODE, 0xFF);
                ASSERT_WARNING(status_reg & OQSPI_MACRONIX_QUAD_STATUS_REG_QUAD_ENABLE_MASK);
        }
}

__RETAINED_CODE static void oqspi_mx25u6432_initialize(HW_OQSPI_BUS_MODE bus_mode, sys_clk_t sys_clk)
{
        __DBG_OQSPI_VOLATILE__ uint8_t dummy_bytes = oqspi_mx25u6432_get_dummy_bytes(sys_clk);

        ASSERT_WARNING(bus_mode == HW_OQSPI_BUS_MODE_SINGLE || bus_mode == HW_OQSPI_BUS_MODE_QUAD);
        ASSERT_WARNING(oqspi_mx25u6432_set_dummy_bytes(dummy_bytes));

        if (bus_mode == HW_OQSPI_BUS_MODE_SINGLE) {
                oqspi_mx25u6432_enable_quad_mode();
        }
}

__RETAINED_CODE static void oqspi_mx25u6432_sys_clock_cfg(sys_clk_t sys_clk)
{
        uint8_t dummy_bytes = oqspi_mx25u6432_get_dummy_bytes(sys_clk);

        oqspi_enter_manual_access_mode();
        ASSERT_WARNING(oqspi_mx25u6432_set_dummy_bytes(dummy_bytes));
        oqspi_automode_int_enter_auto_access_mode();

        hw_oqspi_set_dummy_bytes(dummy_bytes);
}

#if (dg_configUSE_SEGGER_FLASH_LOADER == 1) && (dg_configOQSPI_FLASH_AUTODETECT == 0)
__attribute__((used, __section__("__product_header_primary__")))
static const PRODUCT_HEADER_STRUCT(4) ph_primary = {
        .busrtcmdA = 0xA8A500EB,
        .busrtcmdB = 0x00000616,
        .ctrlmode = 0xF8018F83,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x0004,
        .config_seq = { 0x03, 0x01, 0x40, 0x07 },
        .crc = 0xA6D6
};

__attribute__((used, __section__("__product_header_backup__")))
static const PRODUCT_HEADER_STRUCT(4) ph_backup = {
        .busrtcmdA = 0xA8A500EB,
        .busrtcmdB = 0x00000616,
        .ctrlmode = 0xF8018F83,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x0004,
        .config_seq = { 0x03, 0x01, 0x40, 0x07 },
        .crc = 0xA6D6
};
#endif /* dg_configUSE_SEGGER_FLASH_LOADER == 1 */

#endif /* _OQSPI_MX25U6432_H_ */
/**
 * \}
 * \}
 */
