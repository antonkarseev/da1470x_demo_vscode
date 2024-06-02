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
 * @file oqspi_at25ql641.h
 *
 * @brief OQSPI flash driver for Renesas AT25QL641
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _OQSPI_AT25QL641_H_
#define _OQSPI_AT25QL641_H_

#include "hw_clk.h"
#include "hw_oqspi.h"
#include "oqspi_common.h"
#include "oqspi_adesto_quad.h"

#define OQSPI_AT25QL641_DENSITY (0x17)

/* Flash power up/down timings in usec */

#define AT25QL641_READ_CS_IDLE_DELAY_NS       70   // tCSH
#define AT25QL641_ERASE_CS_IDLE_DELAY_NS      100   // tCSH
#define AT25QL641_SUSPEND_LATENCY_US          30    // tSUS
#define AT25QL641_RESUME_LATENCY_US           3     // tRES
#define AT25QL641_RES_SUS_LATENCY_US          64     // tSUS  100 ns
#define AT25QL641_RESET_DELAY_US              30    // tRST/tSWRST
#define AT25QL641_POWER_DOWN_DELAY_US         3     // tDP/tEDPD
#define AT25QL641_RELEASE_POWER_DOWN_DELAY_US 3     // tRES1/tRDPD
#define AT25QL641_POWER_UP_DELAY_US           10000 // tVSL/tVCSL  tPUV?

__RETAINED_CODE static void oqspi_at25ql641_initialize(HW_OQSPI_BUS_MODE bus_mode, sys_clk_t sys_clk);

static const oqspi_flash_config_t oqspi_at25ql641_cfg = {
    .jedec.manufacturer_id = OQSPI_ADESTO_QUAD_MANUFACTURER_ID,
    .jedec.type            = OQSPI_ADESTO_QUAD_AD25QLXXX_TYPE,
    .jedec.density         = OQSPI_AT25QL641_DENSITY,
    .jedec.density_mask    = 0xFF,

    .size_mbits   = OQSPI_MEMORY_SIZE_128Mbits,
    .address_size = HW_OQSPI_ADDR_SIZE_24,
    .clk_mode     = HW_OQSPI_CLK_MODE_HIGH,
    .opcode_len   = HW_OQSPI_OPCODE_LEN_1_BYTE,

    .read_instr_cfg.opcode_bus_mode     = HW_OQSPI_BUS_MODE_SINGLE,
    .read_instr_cfg.addr_bus_mode       = HW_OQSPI_BUS_MODE_QUAD,
    .read_instr_cfg.extra_byte_bus_mode = HW_OQSPI_BUS_MODE_QUAD,
    .read_instr_cfg.dummy_bus_mode      = HW_OQSPI_BUS_MODE_QUAD,
    .read_instr_cfg.data_bus_mode       = HW_OQSPI_BUS_MODE_QUAD,
    .read_instr_cfg.continuous_mode     = HW_OQSPI_CONTINUOUS_MODE_ENABLE,
    .read_instr_cfg.extra_byte_cfg      = HW_OQSPI_EXTRA_BYTE_ENABLE,
    .read_instr_cfg.extra_byte_half_cfg = HW_OQSPI_EXTRA_BYTE_HALF_DISABLE,
    .read_instr_cfg.opcode              = OQSPI_FAST_READ_QUAD_OPCODE,
    .read_instr_cfg.extra_byte_value    = 0xA0,
    .read_instr_cfg.cs_idle_delay_nsec  = AT25QL641_READ_CS_IDLE_DELAY_NS,

    .erase_instr_cfg.opcode_bus_mode    = HW_OQSPI_BUS_MODE_SINGLE,
    .erase_instr_cfg.addr_bus_mode      = HW_OQSPI_BUS_MODE_SINGLE,
    .erase_instr_cfg.hclk_cycles        = 0,
    .erase_instr_cfg.opcode             = OQSPI_SECTOR_ERASE_OPCODE,
    .erase_instr_cfg.cs_idle_delay_nsec = AT25QL641_ERASE_CS_IDLE_DELAY_NS,

    .read_status_instr_cfg.opcode_bus_mode  = HW_OQSPI_BUS_MODE_SINGLE,
    .read_status_instr_cfg.receive_bus_mode = HW_OQSPI_BUS_MODE_SINGLE,
    .read_status_instr_cfg.dummy_bus_mode   = HW_OQSPI_BUS_MODE_SINGLE,
    .read_status_instr_cfg.dummy_value      = HW_OQSPI_READ_STATUS_DUMMY_VAL_UNCHANGED,
    .read_status_instr_cfg.busy_level       = HW_OQSPI_BUSY_LEVEL_HIGH,
    .read_status_instr_cfg.busy_pos         = OQSPI_STATUS_REG_BUSY_BIT,
    .read_status_instr_cfg.dummy_bytes      = 0,
    .read_status_instr_cfg.opcode           = OQSPI_READ_STATUS_REG_OPCODE,
    .read_status_instr_cfg.delay_nsec       = 200,

    .write_enable_instr_cfg.opcode_bus_mode = HW_OQSPI_BUS_MODE_SINGLE,
    .write_enable_instr_cfg.opcode          = OQSPI_WRITE_ENABLE_OPCODE,

    .page_program_instr_cfg.opcode_bus_mode = HW_OQSPI_BUS_MODE_SINGLE,
    .page_program_instr_cfg.addr_bus_mode   = HW_OQSPI_BUS_MODE_SINGLE,
    .page_program_instr_cfg.data_bus_mode   = HW_OQSPI_BUS_MODE_QUAD,
    .page_program_instr_cfg.opcode          = OQSPI_PAGE_PROGRAM_QUAD_OPCODE,

    .suspend_resume_instr_cfg.suspend_bus_mode     = HW_OQSPI_BUS_MODE_SINGLE,
    .suspend_resume_instr_cfg.resume_bus_mode      = HW_OQSPI_BUS_MODE_SINGLE,
    .suspend_resume_instr_cfg.suspend_opcode       = OQSPI_ADESTO_QUAD_SUSPEND_OPCODE,
    .suspend_resume_instr_cfg.resume_opcode        = OQSPI_ADESTO_QUAD_RESUME_OPCODE,
    .suspend_resume_instr_cfg.suspend_latency_usec = AT25QL641_SUSPEND_LATENCY_US,
    .suspend_resume_instr_cfg.resume_latency_usec  = AT25QL641_RESUME_LATENCY_US,
    .suspend_resume_instr_cfg.res_sus_latency_usec = AT25QL641_RES_SUS_LATENCY_US,

    .exit_continuous_mode_instr_cfg.opcode_bus_mode     = HW_OQSPI_BUS_MODE_QUAD,
    .exit_continuous_mode_instr_cfg.sequence_len        = 4,
    .exit_continuous_mode_instr_cfg.disable_second_half = 0,
    .exit_continuous_mode_instr_cfg.opcode              = OQSPI_EXIT_QPI_OPCODE,

    .delay.reset_usec              = AT25QL641_RESET_DELAY_US,
    .delay.power_down_usec         = AT25QL641_POWER_DOWN_DELAY_US,
    .delay.release_power_down_usec = AT25QL641_RELEASE_POWER_DOWN_DELAY_US,
    .delay.power_up_usec           = AT25QL641_POWER_UP_DELAY_US,

    .callback.initialize_cb       = oqspi_at25ql641_initialize,
    .callback.sys_clk_cfg_cb      = oqspi_adesto_quad_sys_clock_cfg,
    .callback.exit_opi_qpi_cb     = oqspi_exit_qpi,
    .callback.get_dummy_bytes_cb  = oqspi_adesto_quad_get_dummy_bytes,
    .callback.is_suspended_cb     = oqspi_adesto_quad_is_suspended,
    .callback.is_busy_cb          = oqspi_adesto_quad_is_busy,
    .callback.read_status_reg_cb  = oqspi_adesto_quad_read_status_reg,
    .callback.write_status_reg_cb = oqspi_adesto_quad_write_status_reg,

    .resume_before_writing_regs = false,
};

__RETAINED_CODE static void oqspi_at25ql641_initialize(HW_OQSPI_BUS_MODE bus_mode, sys_clk_t sys_clk) {
    UNUSED_ARG(sys_clk);
    ASSERT_ERROR(bus_mode == HW_OQSPI_BUS_MODE_SINGLE || bus_mode == HW_OQSPI_BUS_MODE_QUAD);

    if(bus_mode == HW_OQSPI_BUS_MODE_SINGLE) {
        oqspi_adesto_quad_enable_quad_mode();
    }
}

#if(dg_configUSE_SEGGER_FLASH_LOADER == 1) && (dg_configOQSPI_FLASH_AUTODETECT == 0)
__attribute__((used, __section__("__product_header_primary__"))) static const PRODUCT_HEADER_STRUCT(3) ph_primary = {
    .busrtcmdA            = 0xA8A000EB,
    .busrtcmdB            = 0x00040616,
    .ctrlmode             = 0xf0018103,
    .flash_config_section = 0x11AA,
    .flash_config_length  = 0x0003,
    .config_seq           = {0x31, 0x02, 0x07},
    .crc                  = 0x7E59};

__attribute__((used, __section__("__product_header_backup__"))) static const PRODUCT_HEADER_STRUCT(3) ph_backup = {
    .busrtcmdA            = 0xA8A000EB,
    .busrtcmdB            = 0x00040616,
    .ctrlmode             = 0xf0018103,
    .flash_config_section = 0x11AA,
    .flash_config_length  = 0x0003,
    .config_seq           = {0x31, 0x02, 0x07},
    .crc                  = 0x7E59};
#endif /* dg_configUSE_SEGGER_FLASH_LOADER == 1 */

#endif /* _OQSPI_AT25QL641_H_ */
/**
 * \}
 * \}
 */
