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
 * @file oqspi_xxx.h
 *
 * @brief OQSPI flash driver template
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef _OQSPI_XXX_
#define _OQSPI_XXX_

#include "hw_clk.h"
#include "hw_oqspi.h"
#include "oqspi_common.h"

#define OQSPI_XXX_MANUFACTURER_ID                       (0x00)
#define OQSPI_XXX_TYPE                                  (0x00)
#define OQSPI_XXX_DENSITY                               (0x00)

__RETAINED_CODE static void oqspi_xxx_initialize(HW_OQSPI_BUS_MODE bus_mode, sys_clk_t sys_clk);
__RETAINED_CODE static void oqspi_xxx_sys_clock_cfg(sys_clk_t sys_clk);
__RETAINED_CODE static void oqspi_xxx_exit_opi(void);
__RETAINED_CODE static void oqspi_xxx_get_dummy_bytes(sys_clk_t sys_clk);
__RETAINED_CODE static void oqspi_xxx_is_suspended(HW_OQSPI_BUS_MODE bus_mode);
__RETAINED_CODE static void oqspi_xxx_is_busy(HW_OQSPI_BUS_MODE bus_mode, HW_OQSPI_BUSY_LEVEL busy_level);
__RETAINED_CODE static void oqspi_xxx_read_status_reg(HW_OQSPI_BUS_MODE bus_mode);
__RETAINED_CODE static void oqspi_xxx_write_status_reg(HW_OQSPI_BUS_MODE bus_mode, uint8_t value);

static const oqspi_flash_config_t oqspi_xxx_cfg = {
        .jedec.manufacturer_id                                  = OQSPI_XXX_MANUFACTURER_ID,
        .jedec.type                                             = OQSPI_XXX_TYPE,
        .jedec.density                                          = OQSPI_XXX_DENSITY,
        .jedec.density_mask                                     = 0xFF,

        .size_mbits                                             = OQSPI_MEMORY_SIZE_1Gbit,
        .address_size                                           = HW_OQSPI_ADDR_SIZE_24,
        .clk_mode                                               = HW_OQSPI_CLK_MODE_LOW,
        .opcode_len                                             = HW_OQSPI_OPCODE_LEN_1_BYTE,

        .read_instr_cfg.opcode_bus_mode                         = HW_OQSPI_BUS_MODE_SINGLE,
        .read_instr_cfg.addr_bus_mode                           = HW_OQSPI_BUS_MODE_SINGLE,
        .read_instr_cfg.extra_byte_bus_mode                     = HW_OQSPI_BUS_MODE_SINGLE,
        .read_instr_cfg.dummy_bus_mode                          = HW_OQSPI_BUS_MODE_SINGLE,
        .read_instr_cfg.data_bus_mode                           = HW_OQSPI_BUS_MODE_SINGLE,
        .read_instr_cfg.continuous_mode                         = HW_OQSPI_CONTINUOUS_MODE_DISABLE,
        .read_instr_cfg.extra_byte_cfg                          = HW_OQSPI_EXTRA_BYTE_DISABLE,
        .read_instr_cfg.extra_byte_half_cfg                     = HW_OQSPI_EXTRA_BYTE_HALF_DISABLE,
        .read_instr_cfg.opcode                                  = 0x00, // Read opcode
        .read_instr_cfg.extra_byte_value                        = 0xFF,
        .read_instr_cfg.cs_idle_delay_nsec                      = 0,    // tSHSL (read)

        .erase_instr_cfg.opcode_bus_mode                        = HW_OQSPI_BUS_MODE_SINGLE,
        .erase_instr_cfg.addr_bus_mode                          = HW_OQSPI_BUS_MODE_SINGLE,
        .erase_instr_cfg.hclk_cycles                            = 0,
        .erase_instr_cfg.opcode                                 = 0x00,
        .erase_instr_cfg.cs_idle_delay_nsec                     = 0,   // tSHSL (erase)

        .read_status_instr_cfg.opcode_bus_mode                  = HW_OQSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.receive_bus_mode                 = HW_OQSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.dummy_bus_mode                   = HW_OQSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.dummy_value                      = HW_OQSPI_READ_STATUS_DUMMY_VAL_UNCHANGED,
        .read_status_instr_cfg.busy_level                       = HW_OQSPI_BUSY_LEVEL_HIGH,
        .read_status_instr_cfg.busy_pos                         = OQSPI_STATUS_REG_BUSY_BIT,
        .read_status_instr_cfg.dummy_bytes                      = 0,
        .read_status_instr_cfg.opcode                           = 0x00, // Read Status opcode
        .read_status_instr_cfg.delay_nsec                       = 0,

        .write_enable_instr_cfg.opcode_bus_mode                 = HW_OQSPI_BUS_MODE_SINGLE,
        .write_enable_instr_cfg.opcode                          = 0x00,

        .page_program_instr_cfg.opcode_bus_mode                 = HW_OQSPI_BUS_MODE_SINGLE,
        .page_program_instr_cfg.addr_bus_mode                   = HW_OQSPI_BUS_MODE_SINGLE,
        .page_program_instr_cfg.data_bus_mode                   = HW_OQSPI_BUS_MODE_SINGLE,
        .page_program_instr_cfg.opcode                          = 0x00, // Page Program opcode

        .suspend_resume_instr_cfg.suspend_bus_mode              = HW_OQSPI_BUS_MODE_SINGLE,
        .suspend_resume_instr_cfg.resume_bus_mode               = HW_OQSPI_BUS_MODE_SINGLE,
        .suspend_resume_instr_cfg.suspend_opcode                = 0x00, // Erase Suspend opcode
        .suspend_resume_instr_cfg.resume_opcode                 = 0x00, // Erase Resume opcode
        .suspend_resume_instr_cfg.suspend_latency_usec          = 0,    // tESL
        .suspend_resume_instr_cfg.resume_latency_usec           = 0,    // no latency
        .suspend_resume_instr_cfg.res_sus_latency_usec          = 0,    // tERS

        .exit_continuous_mode_instr_cfg.opcode_bus_mode         = HW_OQSPI_BUS_MODE_SINGLE,
        .exit_continuous_mode_instr_cfg.sequence_len            = 0,
        .exit_continuous_mode_instr_cfg.disable_second_half     = 0,
        .exit_continuous_mode_instr_cfg.opcode                  = 0xFF,

        .delay.reset_usec                                       = 0,    // tREADY2 (4KB Sector Erase operation)
        .delay.power_down_usec                                  = 0,    // tDP
        .delay.release_power_down_usec                          = 0,    // tRES1
        .delay.power_up_usec                                    = 0,    // tVSL

        .callback.initialize_cb                                 = oqspi_xxx_initialize,
        .callback.sys_clk_cfg_cb                                = oqspi_xxx_sys_clock_cfg,
        .callback.exit_opi_qpi_cb                               = oqspi_xxx_exit_opi,
        .callback.get_dummy_bytes_cb                            = oqspi_xxx_get_dummy_bytes,
        .callback.is_suspended_cb                               = oqspi_xxx_is_suspended,
        .callback.is_busy_cb                                    = oqspi_xxx_is_busy,
        .callback.read_status_reg_cb                            = oqspi_xxx_read_status_reg,
        .callback.write_status_reg_cb                           = oqspi_xxx_write_status_reg,
};

__RETAINED_CODE static void oqspi_xxx_initialize(HW_OQSPI_BUS_MODE bus_mode, sys_clk_t sys_clk)
{

}

__RETAINED_CODE static void oqspi_xxx_sys_clock_cfg(sys_clk_t sys_clk)
{

}

__RETAINED_CODE static void oqspi_xxx_exit_opi(void)
{

}

__RETAINED_CODE static void oqspi_xxx_get_dummy_bytes(sys_clk_t sys_clk)
{

}

__RETAINED_CODE static void oqspi_xxx_is_suspended(HW_OQSPI_BUS_MODE bus_mode)
{

}

__RETAINED_CODE static void oqspi_xxx_is_busy(HW_OQSPI_BUS_MODE bus_mode, HW_OQSPI_BUSY_LEVEL busy_level)
{

}

__RETAINED_CODE static void oqspi_xxx_read_status_reg(HW_OQSPI_BUS_MODE bus_mode)
{

}

__RETAINED_CODE static void oqspi_xxx_write_status_reg(HW_OQSPI_BUS_MODE bus_mode, uint8_t value)
{

}

#if (dg_configUSE_SEGGER_FLASH_LOADER == 1) && (dg_configOQSPI_FLASH_AUTODETECT == 0)
__attribute__((used, __section__("__product_header_primary__")))
static const PRODUCT_HEADER_STRUCT(7) ph_primary = {
        .busrtcmdA = 0x00000000,
        .busrtcmdB = 0x00000000,
        .ctrlmode = 0x00000000,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x0003,
        .config_seq = { 0x00, 0x00, 0x00 },
        .crc = 0x0000
};

__attribute__((used, __section__("__product_header_backup__")))
static const PRODUCT_HEADER_STRUCT(7) ph_backup = {
        .busrtcmdA = 0x00000000,
        .busrtcmdB = 0x00000000,
        .ctrlmode = 0x00000000,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x0003,
        .config_seq = { 0x00, 0x00, 0x00 },
        .crc = 0x0000
};
#endif /* dg_configUSE_SEGGER_FLASH_LOADER == 1 */

#endif /* _OQSPI_XXX_ */
/**
 * \}
 * \}
 */
