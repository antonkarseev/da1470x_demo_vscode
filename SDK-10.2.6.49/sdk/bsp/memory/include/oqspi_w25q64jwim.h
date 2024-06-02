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
 * @file oqspi_w25q64jwim.h
 *
 * @brief OQSPI flash driver for Winbond W25Q64JWIM
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _OQSPI_W25Q64JWIM_H_
#define _OQSPI_W25Q64JWIM_H_

#include "hw_clk.h"
#include "hw_oqspi.h"
#include "oqspi_common.h"
#include "oqspi_winbond_quad.h"

#define OQSPI_W25Q64JWIM_DENSITY                                (0x17)

__RETAINED_CODE static void oqspi_w25q64jwim_initialize(HW_OQSPI_BUS_MODE bus_mode, sys_clk_t sys_clk);

static const oqspi_flash_config_t oqspi_w25q64jwim_cfg = {
        .jedec.manufacturer_id                                  = OQSPI_WINBOND_QUAD_MANUFACTURER_ID,
        .jedec.type                                             = OQSPI_WINBOND_QUAD_W25QXXXJWXM_TYPE,
        .jedec.density                                          = OQSPI_W25Q64JWIM_DENSITY,
        .jedec.density_mask                                     = 0xFF,

        .size_mbits                                             = OQSPI_MEMORY_SIZE_64Mbits,
        .address_size                                           = HW_OQSPI_ADDR_SIZE_24,
        .clk_mode                                               = HW_OQSPI_CLK_MODE_HIGH,
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
        .read_instr_cfg.extra_byte_value                        = 0xA0,
        .read_instr_cfg.cs_idle_delay_nsec                      = 10,   // tSHSL1

        .erase_instr_cfg.opcode_bus_mode                        = HW_OQSPI_BUS_MODE_SINGLE,
        .erase_instr_cfg.addr_bus_mode                          = HW_OQSPI_BUS_MODE_SINGLE,
        .erase_instr_cfg.hclk_cycles                            = 0,
        .erase_instr_cfg.opcode                                 = OQSPI_SECTOR_ERASE_OPCODE,
        .erase_instr_cfg.cs_idle_delay_nsec                     = 50,   // tSHSL2

        .read_status_instr_cfg.opcode_bus_mode                  = HW_OQSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.receive_bus_mode                 = HW_OQSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.dummy_bus_mode                   = HW_OQSPI_BUS_MODE_SINGLE,
        .read_status_instr_cfg.dummy_value                      = HW_OQSPI_READ_STATUS_DUMMY_VAL_UNCHANGED,
        .read_status_instr_cfg.busy_level                       = HW_OQSPI_BUSY_LEVEL_HIGH,
        .read_status_instr_cfg.busy_pos                         = OQSPI_STATUS_REG_BUSY_BIT,
        .read_status_instr_cfg.dummy_bytes                      = 0,
        .read_status_instr_cfg.opcode                           = OQSPI_READ_STATUS_REG_OPCODE,
        .read_status_instr_cfg.delay_nsec                       = 200,

        .write_enable_instr_cfg.opcode_bus_mode                 = HW_OQSPI_BUS_MODE_SINGLE,
        .write_enable_instr_cfg.opcode                          = OQSPI_WRITE_ENABLE_OPCODE,

        .page_program_instr_cfg.opcode_bus_mode                 = HW_OQSPI_BUS_MODE_SINGLE,
        .page_program_instr_cfg.addr_bus_mode                   = HW_OQSPI_BUS_MODE_SINGLE,
        .page_program_instr_cfg.data_bus_mode                   = HW_OQSPI_BUS_MODE_QUAD,
        .page_program_instr_cfg.opcode                          = OQSPI_PAGE_PROGRAM_QUAD_OPCODE,

        .suspend_resume_instr_cfg.suspend_bus_mode              = HW_OQSPI_BUS_MODE_SINGLE,
        .suspend_resume_instr_cfg.resume_bus_mode               = HW_OQSPI_BUS_MODE_SINGLE,
        .suspend_resume_instr_cfg.suspend_opcode                = OQSPI_WINBOND_QUAD_SUSPEND_OPCODE,
        .suspend_resume_instr_cfg.resume_opcode                 = OQSPI_WINBOND_QUAD_RESUME_OPCODE,
        .suspend_resume_instr_cfg.suspend_latency_usec          = 20,   // tSUS
        .suspend_resume_instr_cfg.resume_latency_usec           = 1,    // 200nsec
        .suspend_resume_instr_cfg.res_sus_latency_usec          = 20,   // tSUS

        .exit_continuous_mode_instr_cfg.opcode_bus_mode         = HW_OQSPI_BUS_MODE_QUAD,
        .exit_continuous_mode_instr_cfg.sequence_len            = 4,
        .exit_continuous_mode_instr_cfg.disable_second_half     = 0,
        .exit_continuous_mode_instr_cfg.opcode                  = 0xFF,

        .delay.reset_usec                                       = 12000, // tRST
        .delay.power_down_usec                                  = 3,     // tDP
        .delay.release_power_down_usec                          = 30,    // tRES1
        .delay.power_up_usec                                    = 20,    // tVSL

        .callback.initialize_cb                                 = oqspi_w25q64jwim_initialize,
        .callback.sys_clk_cfg_cb                                = oqspi_winbond_quad_sys_clock_cfg,
        .callback.exit_opi_qpi_cb                               = oqspi_exit_qpi,
        .callback.get_dummy_bytes_cb                            = oqspi_winbond_quad_get_dummy_bytes,
        .callback.is_suspended_cb                               = oqspi_winbond_quad_is_suspended,
        .callback.is_busy_cb                                    = oqspi_winbond_quad_is_busy,
        .callback.read_status_reg_cb                            = oqspi_winbond_quad_read_status_reg,
        .callback.write_status_reg_cb                           = oqspi_winbond_quad_write_status_reg,

        .resume_before_writing_regs                             = false,
};

__RETAINED_CODE static void oqspi_w25q64jwim_initialize(HW_OQSPI_BUS_MODE bus_mode, sys_clk_t sys_clk)
{
        UNUSED_ARG(sys_clk);
        ASSERT_ERROR(bus_mode == HW_OQSPI_BUS_MODE_SINGLE || bus_mode == HW_OQSPI_BUS_MODE_QUAD);

#if OQSPI_WINBOND_QUAD_UNLOCK_PROTECTION
        oqspi_winbond_quad_unlock_protection();
#endif

        if (bus_mode == HW_OQSPI_BUS_MODE_SINGLE) {
                oqspi_winbond_quad_enable_quad_mode();
        }
}

#if (dg_configUSE_SEGGER_FLASH_LOADER == 1) && (dg_configOQSPI_FLASH_AUTODETECT == 0)
__attribute__((used, __section__("__product_header_primary__")))
static const PRODUCT_HEADER_STRUCT(3) ph_primary = {
        .busrtcmdA = 0xA8A000EB,
        .busrtcmdB = 0x00000616,
        .ctrlmode = 0xF8018F83,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x0003,
        .config_seq = { 0x02, 0x31, 0x02 },
        .crc = 0x7E59
};

__attribute__((used, __section__("__product_header_backup__")))
static const PRODUCT_HEADER_STRUCT(3) ph_backup = {
        .busrtcmdA = 0xA8A000EB,
        .busrtcmdB = 0x00000616,
        .ctrlmode = 0xF8018F83,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x0003,
        .config_seq = { 0x02, 0x31, 0x02 },
        .crc = 0x7E59
};
#endif /* dg_configUSE_SEGGER_FLASH_LOADER == 1 */

#endif /* _OQSPI_W25Q64JWIM_H_ */
/**
 * \}
 * \}
 */
