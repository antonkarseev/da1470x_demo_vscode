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
 * @file qspi_winbond.h
 *
 * @brief QSPI flash driver for Winbond flashes - common code
 *
 * Copyright (C) 2016-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _QSPI_WINBOND_H_
#define _QSPI_WINBOND_H_

#define WINBOND_ID                      0xEF

#include "qspi_common.h"

#define W25Q_ERASE_PROGRAM_SUSPEND      0x75
#define W25Q_ERASE_PROGRAM_RESUME       0x7A

#define W25Q_WRITE_STATUS_REGISTER2     0x31
#define W25Q_WRITE_STATUS_REGISTER3     0x11
#define W25Q_WRITE_ENABLE_NON_VOL       0x50
#define W25Q_READ_STATUS_REGISTER2      0x35
#define W25Q_READ_STATUS_REGISTER3      0x15
#define W25Q_BLOCK_ERASE_64K            0xD8
#define W25Q_FAST_READ_QPI              0x0B
#define W25Q_READ_DEVICE_ID_SINGLE      0x90    // Requires single mode for the command entry!
#define W25Q_READ_DEVICE_ID_DUAL        0x92    // Requires dual mode for the command entry!
#define W25Q_READ_DEVICE_ID_QUAD        0x94
#define W25Q_READ_UNIQUE_ID             0x4B    // Requires single mode for the command entry!
#define W25Q_READ_SFDP_REG              0x5A    // Requires single mode for the command entry!
#define W25Q_ERASE_SECURITY_REGS        0x44    // Requires single mode for the command entry!
#define W25Q_PROGR_SECURITY_REGS        0x42    // Requires single mode for the command entry!
#define W25Q_READ_SECURITY_REGS         0x48    // Requires single mode for the command entry!

#define W25Q_ENTER_4B_ADDR_MODE_CMD     0xB7
#define W25_EXIT_4B_ADDR_MODE_CMD       0xE9

/* Suspend */
#define W25Q_STATUS2_SUS_BIT            7
#define W25Q_STATUS2_SUS_MASK           (1 << W25Q_STATUS2_SUS_BIT)

/* QPI Enable */
#define W25Q_STATUS2_QE_BIT             1
#define W25Q_STATUS2_QE_MASK            (1 << W25Q_STATUS2_QE_BIT)

/* Address Mode (0: 24bits - 1: 32bits) */
#define W25Q_STATUS3_ADDR_MODE_BIT      0
#define W25Q_STATUS3_ADDR_MODE_MASK     (1 << W25Q_STATUS3_ADDR_MODE_BIT)

// Flash power up/down timings
#define W25Q_POWER_DOWN_DELAY_US          3
#define W25Q_RELEASE_POWER_DOWN_DELAY_US  3
#define W25Q_POWER_UP_DELAY_US            10

#if (dg_configFLASH_POWER_OFF == 1)
/**
 * \brief uCode for handling the QSPI FLASH activation from power off.
 */
        /*
         * Should work with all Winbond flashes -- verified with W25Q80EW.
         *
         * Delay 10usec
         * 0x01   // CMD_NBYTES = 0, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0xA0   // CMD_WT_CNT_LS = 160 --> 10000 / 62.5 = 160 = 10usec
         * 0x00   // CMD_WT_CNT_MS = 0
         * Exit from Fast Read mode
         * 0x09   // CMD_NBYTES = 1, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x00   // CMD_WT_CNT_LS = 0
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0xFF   // Enable Reset
         * (up to 16 words)
         */
        const uint32_t w25q_ucode_wakeup[] = {
                0x09000001 | (((uint16_t)(W25Q_POWER_UP_DELAY_US*1000/62.5) & 0xFFFF) << 8),
                0x00FF0000,
        };
#elif (dg_configFLASH_POWER_DOWN == 1)
/**
 * \brief uCode for handling the QSPI FLASH release from power-down.
 */
        /*
         * Should work with all Winbond flashes -- verified with W25Q80EW.
         *
         * 0x09   // CMD_NBYTES = 1, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x30   // CMD_WT_CNT_LS = 3000 / 62.5 = 48 // 3usec
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0xAB   // Release Power Down
         * (up to 16 words)
         */
        const uint32_t w25q_ucode_wakeup[] = {
                0xAB000009 | (((uint16_t)(W25Q_RELEASE_POWER_DOWN_DELAY_US*1000/62.5) & 0xFFFF) << 8),
        };
#else
/**
 * \brief uCode for handling the QSPI FLASH exit from the "Continuous Read Mode".
 */
        /*
         * Should work with all Winbond flashes -- verified with W25Q80EW.
         *
         * 0x25   // CMD_NBYTES = 4, CMD_TX_MD = 2 (Quad), CMD_VALID = 1
         * 0x00   // CMD_WT_CNT_LS = 0
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0x55   // Clocks 0-1 (A23-16)
         * 0x55   // Clocks 2-3 (A15-8)
         * 0x55   // Clocks 4-5 (A7-0)
         * 0x55   // Clocks 6-7 (M7-0) : M5-4 != '10' ==> Disable "Continuous Read Mode"
         * (up to 16 words)
         */
        const uint32_t w25q_ucode_wakeup[] = {
                0x55000025,
                0x00555555,
        };
#endif

static bool flash_w25q_is_suspended(HW_QSPIC_ID id);
static void flash_w25q_initialize(HW_QSPIC_ID id);

/**
 * \brief Enable volatile writes to Status Register bits
 * \details When this command is issued, any writes to any of the Status Registers of the Flash are
 *        done as volatile writes. This command is valid only when the Write Status Register command
 *        follows.
 *
 * \param[in] id QSPI controller id
 *
 * \note This function blocks until the Flash has processed the command.
 */
__STATIC_FORCEINLINE __UNUSED void flash_w25q_wre_volatile(HW_QSPIC_ID id)
{
        uint8_t cmd[] = { W25Q_WRITE_ENABLE_NON_VOL };

        flash_write(id, cmd, 1);

        /* Verify */
        while (flash_is_busy(id));
}

__STATIC_FORCEINLINE uint8_t flash_w25q_read_status_register_2(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { W25Q_READ_STATUS_REGISTER2 };

        flash_transact(id, cmd, 1, &status, 1);

        return status;
}

__STATIC_FORCEINLINE uint8_t flash_w25q_read_status_register_3(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { W25Q_READ_STATUS_REGISTER3 };

        flash_transact(id, cmd, 1, &status, 1);

        return status;
}

/**
 * \brief Write the Status Register 2 of the Flash
 *
 * \param[in] id QSPI controller id
 * \param[in] value The value to be written.
 *
 * \note This function blocks until the Flash has processed the command. No verification that the
 *        value has been actually written is done though. It is up to the caller to decide whether
 *        such verification is needed or not and execute it on its own.
 */
__STATIC_FORCEINLINE void flash_w25q_write_status_register_2(HW_QSPIC_ID id, uint8_t value)
{
        uint8_t cmd[] = { W25Q_WRITE_STATUS_REGISTER2, value };

        flash_write(id, cmd, 2);

        /* Wait for the Flash to process the command */
        while (flash_is_busy(id));
}

__STATIC_FORCEINLINE void flash_w25q_enable_quad_mode(HW_QSPIC_ID id)
{
        uint8_t status;

        status = flash_w25q_read_status_register_2(id);
        if (!(status & W25Q_STATUS2_QE_MASK)) {
                flash_write_enable(id);
                flash_w25q_write_status_register_2(id, status | W25Q_STATUS2_QE_MASK);
        }
}

__UNUSED __RETAINED_CODE static void qspi_winbond_enter_4b_addr_mode(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd = W25Q_ENTER_4B_ADDR_MODE_CMD;

        status = flash_w25q_read_status_register_3(id);

        if (!(status & W25Q_STATUS3_ADDR_MODE_MASK)) {
                flash_write(id, &cmd, 1);
        }

        while (flash_is_busy(id));
}

__RETAINED_CODE static bool flash_w25q_is_suspended(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;

        status = flash_w25q_read_status_register_2(id);
        return (status & W25Q_STATUS2_SUS_MASK) != 0;
}

__UNUSED __RETAINED_CODE static void flash_w25q_initialize(HW_QSPIC_ID id)
{
        flash_w25q_enable_quad_mode(id);
}


#endif /* _QSPI_WINBOND_H_ */
/**
 * \}
 * \}
 */
