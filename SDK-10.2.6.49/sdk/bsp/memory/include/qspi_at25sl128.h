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
 * @file qspi_at25sl128.h
 *
 * @brief Driver for flash ADESTO AT25SL128
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef _QSPI_ADESTO_AT25SL128_H_
#define _QSPI_ADESTO_AT25SL128_H_

#include "sdk_defs.h"
#include "qspi_common.h"
#include "hw_qspi.h"

/************************************************* JEDEC ID INFO *************************************************/

/**
 * \brief The Flash type JEDEC ID
 *
 * This is the first byte returned by the 0x9F command
 */
#define ADESTO_ID                                       0x1F

/**
 * \brief The Flash type JEDEC ID
 *
 * This is the second byte returned by the 0x9F command
 */
#define AT25SL_TYPE                                     0x42

/**
 * \brief The Flash density JEDEC ID
 *
 * This is the third byte returned by the 0x9F command
 */
#define AT25SL128_SIZE                                  0x18

/**************************************************** TIMINGS ****************************************************/

/* Flash power up/down timings in usec */
#define AT25SL128_POWER_DOWN_DELAY_US                   3
#define AT25SL128_RELEASE_POWER_DOWN_DELAY_US           3
#define AT25SL128_POWER_UP_DELAY_US                     10000
#define AT25SL128_READ_CS_IDLE_DELAY_NS                 100
#define AT25SL128_ERASE_CS_IDLE_DELAY_NS                100

# if (NSEC_TO_CLK_CYCLES(AT25SL128_READ_CS_IDLE_DELAY_NS, dg_configRCHS_96M_FREQ) > HW_QSPI_READ_CS_IDLE_CYCLES_MAX)
# pragma message "The CS idle delay cannot be achieved due to hardware limitations, thus it will be limited to the max permissible value."
# endif

/************************************************ Opcodes SECTION ************************************************/

/* Status Register Opcodes */
#define AT25SL128_READ_STATUS_REGISTER_2_OPCODE         0x35
#define AT25SL128_WRITE_STATUS_REGISTER_2_OPCODE        0x31

/* Suspend/Resume Opcodes */
#define AT25SL128_ERASE_PROGRAM_SUSPEND_OPCODE          0x75
#define AT25SL128_ERASE_PROGRAM_RESUME_OPCODE           0x7A

/* Quad Read Opcode */
#define AT25SL128_FAST_READ_QUAD_OPCODE                 0xEB

/* Quad Page Program */
#define AT25SL128_QUAD_PAGE_PROGRAM_OPCODE              0x33

/********************************************** DRIVER GENERIC INFO **********************************************/

/* Quad Enable Bit Position - Status Register 2 */
#define AT25SL128_SR_2_QE_POS                           (1)
#define AT25SL128_SR_2_QE_MASK                          (1 << AT25SL128_SR_2_QE_POS)

/* Erase/Program Suspend Bit Position - Status Register 2 */
#define AT25SL128_SR_2_ESUS_POS                         (7)
#define AT25SL128_SR_2_ESUS_MASK                        (1 << AT25SL128_SR_2_ESUS_POS)

__RETAINED_CODE static bool flash_at25sl128_is_suspended(HW_QSPIC_ID id);
__RETAINED_CODE static void flash_at25sl128_initialize(HW_QSPIC_ID id);
__RETAINED_CODE static void flash_at25sl128_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk);
__RETAINED_CODE static uint8_t flash_at25sl128_get_dummy_bytes(__UNUSED HW_QSPIC_ID id, __UNUSED sys_clk_t sys_clk);


/**
 * \brief This struct configures the system for the specific flash
 *
 * \note This struct MUST be const for this to work. Therefore, assignments must
 *       not change (must be read-only)
 */
static const qspi_flash_config_t flash_at25sl128_config = {
        /* JEDEC Bytes 9Fh */
        .manufacturer_id                  = ADESTO_ID,
        .device_type                      = AT25SL_TYPE,
        .device_density                   = AT25SL128_SIZE,

        /* Flash Info */
        .memory_size                      = MEMORY_SIZE_128Mb,
        .address_size                     = HW_QSPI_ADDR_SIZE_24,
        .is_ram                           = false,
        .qpi_mode                         = false,

        /* Callbacks */
        .is_suspended                     = flash_at25sl128_is_suspended,
        .initialize                       = flash_at25sl128_initialize,
        .sys_clk_cfg                      = flash_at25sl128_sys_clock_cfg,
        .get_dummy_bytes                  = flash_at25sl128_get_dummy_bytes,

        /* Read */
        .fast_read_opcode                 = AT25SL128_FAST_READ_QUAD_OPCODE,
        .send_once                        = 1,
        .extra_byte                       = 0xA0,
        .break_seq_size                   = HW_QSPI_BREAK_SEQ_SIZE_1B,

        /* Page Program */
        .page_program_opcode              = AT25SL128_QUAD_PAGE_PROGRAM_OPCODE,
        .quad_page_program_address        = true,

        /* Sector Erase */
        .erase_opcode                     = CMD_SECTOR_ERASE,
        .read_erase_progress_opcode       = CMD_READ_STATUS_REGISTER,
        .erase_in_progress_bit            = FLASH_STATUS_BUSY_BIT,
        .erase_in_progress_bit_high_level = true,

        /* Program/Erase Suspend/Resume */
        .erase_suspend_opcode             = AT25SL128_ERASE_PROGRAM_SUSPEND_OPCODE,
        .erase_resume_opcode              = AT25SL128_ERASE_PROGRAM_RESUME_OPCODE,

        /* Timings */
        .power_down_delay                 = AT25SL128_POWER_DOWN_DELAY_US,
        .release_power_down_delay         = AT25SL128_RELEASE_POWER_DOWN_DELAY_US,
        .power_up_delay                   = AT25SL128_POWER_UP_DELAY_US,
        .suspend_delay_us                 = 30,
        .resume_delay_us                  = 1, /* = 1usec > 200nsec = maximum resume delay according to memory's datasheet */
        .reset_delay_us                   = 30,
        .read_cs_idle_delay_ns            = AT25SL128_READ_CS_IDLE_DELAY_NS,
        .erase_cs_idle_delay_ns           = AT25SL128_ERASE_CS_IDLE_DELAY_NS,
};

__RETAINED_CODE static uint8_t flash_at25sl128_read_status_register_2(HW_QSPIC_ID id)
{
        uint8_t status;
        uint8_t cmd[] = { AT25SL128_READ_STATUS_REGISTER_2_OPCODE };

        flash_transact(id, cmd, 1, &status, 1);

        return status;
}

__RETAINED_CODE static void flash_at25sl128_write_status_register_2(HW_QSPIC_ID id, uint8_t value)
{
        uint8_t cmd[2] = { AT25SL128_WRITE_STATUS_REGISTER_2_OPCODE, value };

        flash_write(id, cmd, 2);

        /* Wait for the Flash to process the command */
        while (flash_is_busy(id));
}

__RETAINED_CODE static void flash_at25sl128_enable_quad_mode(HW_QSPIC_ID id)
{
        uint8_t status;

        status = flash_at25sl128_read_status_register_2(id);
        if (!(status & AT25SL128_SR_2_QE_MASK)) {
                flash_write_enable(id);
                flash_at25sl128_write_status_register_2(id, status | AT25SL128_SR_2_QE_MASK);
        }
}

__RETAINED_CODE static uint8_t flash_at25sl128_get_dummy_bytes(__UNUSED HW_QSPIC_ID id, __UNUSED sys_clk_t sys_clk)
{
        return 2;
}

__RETAINED_CODE static bool flash_at25sl128_is_suspended(HW_QSPIC_ID id)
{
        uint8_t status;

        status = flash_at25sl128_read_status_register_2(id);
        return ((status & AT25SL128_SR_2_ESUS_MASK) != 0);
}

__RETAINED_CODE static void flash_at25sl128_initialize(HW_QSPIC_ID id)
{
        /* Set QE Bit if it is not set */
        flash_at25sl128_enable_quad_mode(id);
}

__RETAINED_CODE static void flash_at25sl128_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk)
{

}

#endif /* _QSPI_ADESTO_AT25SL128_H_ */
/**
 * \}
 * \}
 */
