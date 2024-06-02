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
 * @file oqspi_macronix_octa.h
 *
 * @brief This header contains macros and functions used by the OQSPIC flash drivers of the octa
 *        Macronix flash memories.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _OQSPI_MACRONIX_OCTA_H_
#define _OQSPI_MACRONIX_OCTA_H_

#include "oqspi_common.h"

#define OQSPI_MACRONIX_OCTA_MANUFACTURER_ID             (0xC2)
#define OQSPI_MACRONIX_OCTA_MX66L_TYPE                  (0x85)
#define OQSPI_MACRONIX_OCTA_MX66U_TYPE                  (0x80)
#define OQSPI_MACRONIX_OCTA_MX25U_TYPE                  (0x80)

#define OQSPI_MACRONIX_OCTA_8READ_OPCODE                (0xEC)
#define OQSPI_MACRONIX_OCTA_PAGE_PROGRAM_OPCODE         (0x12)
#define OQSPI_MACRONIX_OCTA_SECTOR_ERASE_OPCODE         (0x21)
#define OQSPI_MACRONIX_OCTA_BLOCK_ERASE_OPCODE          (0xDC)
#define OQSPI_MACRONIX_OCTA_CHIP_ERASE_OPCODE           (0x60)

#define OQSPI_MACRONIX_OCTA_SUSPEND_OPCODE              (0xB0)
#define OQSPI_MACRONIX_OCTA_RESUME_OPCODE               (0x30)

#define OQSPI_MACRONIX_OCTA_READ_CFG_REG2_OPCODE        (0x71)
#define OQSPI_MACRONIX_OCTA_WRITE_CFG_REG2_OPCODE       (0x72)

#define OQSPI_MACRONIX_OCTA_READ_SECURITY_REG_OPCODE    (0x2B)
#define OQSPI_MACRONIX_OCTA_WRITE_SECURITY_REG_OPCODE   (0x2F)

#define OQSPI_MACRONIX_OCTA_CFG_REG2_BUS_MODE_ADDR      (0x00000000)
#define OQSPI_MACRONIX_OCTA_CFG_REG2_DUMMY_BYTES_ADDR   (0x00000300)

#define OQSPI_MACRONIX_OCTA_BUS_MODE_MASK               (0x03)
#define OQSPI_MACRONIX_OCTA_DUMMY_BYTES_MASK            (0x07)

#define OQSPI_MACRONIX_OCTA_BUS_MODE_SPI                (0x00)
#define OQSPI_MACRONIX_OCTA_BUS_MODE_STR_OPI            (0x01)

/* Program Suspend Bit/Mask */
#define OQSPI_MACRONIX_OCTA_SECURITY_PSB_BIT            (0x02)
#define OQSPI_MACRONIX_OCTA_SECURITY_PSB_MASK           (1 << OQSPI_MACRONIX_OCTA_SECURITY_PSB_BIT)

/* Erase Suspend Bit/Mask */
#define OQSPI_MACRONIX_OCTA_SECURITY_ESB_BIT            (0x03)
#define OQSPI_MACRONIX_OCTA_SECURITY_ESB_MASK           (1 << OQSPI_MACRONIX_OCTA_SECURITY_ESB_BIT)

#define OQSPI_MACRONIX_OCTA_SUSPENDED_MASK              (OQSPI_MACRONIX_OCTA_SECURITY_PSB_MASK | \
                                                        OQSPI_MACRONIX_OCTA_SECURITY_ESB_MASK)
/*
 * Dummy bytes configuration
 *
 * DC           Dummy bytes      Octa I/O STR (MHz)
 * -------------------------------------------------------
 * 000(Default) 20                      133
 * 001          18                      133
 * 010          16                      133
 * 011          14                      133
 * 100          12                      104
 * 101          10                      104
 * 110          8                       84
 * 111          6                       66
 */
typedef enum {
        OQSPI_MX_DUMMY_BYTES_20 = 0x00,
        OQSPI_MX_DUMMY_BYTES_18 = 0x01,
        OQSPI_MX_DUMMY_BYTES_16 = 0x02,
        OQSPI_MX_DUMMY_BYTES_14 = 0x03,
        OQSPI_MX_DUMMY_BYTES_12 = 0x04,
        OQSPI_MX_DUMMY_BYTES_10 = 0x05,
        OQSPI_MX_DUMMY_BYTES_8  = 0x06,
        OQSPI_MX_DUMMY_BYTES_6  = 0x07,
        OQSPI_MX_DUMMY_BYTES_INVALID  = 0xFF,
} OQSPI_MX_DUMMY_BYTES;


__UNUSED __RETAINED_CODE static uint8_t oqspi_macronix_octa_read_register(uint8_t opcode, uint32_t address,
                                                            uint8_t mask, HW_OQSPI_BUS_MODE bus_mode)
{
        __DBG_OQSPI_VOLATILE__ uint8_t reg_val;

        ASSERT_ERROR((opcode == OQSPI_READ_STATUS_REG_OPCODE && address == 0x00000000) ||
                     (opcode == OQSPI_MACRONIX_OCTA_READ_CFG_REG2_OPCODE) ||
                     (opcode == OQSPI_MACRONIX_OCTA_READ_SECURITY_REG_OPCODE  && address == 0x00000000));

        hw_oqspi_cs_enable();

        switch (bus_mode) {
        case HW_OQSPI_BUS_MODE_SINGLE:
                hw_oqspi_write8(opcode);
                if (opcode == OQSPI_MACRONIX_OCTA_READ_CFG_REG2_OPCODE) {
                        hw_oqspi_write32(address);
                }
                break;
        case HW_OQSPI_BUS_MODE_OCTA:
                hw_oqspi_write16(CONVERT_OPCODE_TO_DUAL_BYTE(opcode));
                hw_oqspi_write32(address);
                hw_oqspi_write16(0x0000);
                hw_oqspi_write8(0x00);
                hw_oqspi_dummy8();
                break;
        default:
                ASSERT_ERROR(0);
        }

        reg_val = hw_oqspi_read8();
        hw_oqspi_cs_disable();

        return (reg_val & mask);
}

__UNUSED __RETAINED_CODE static void oqspi_macronix_octa_write_register(uint8_t opcode, uint32_t address,
                                                          uint8_t value, HW_OQSPI_BUS_MODE bus_mode)
{
        ASSERT_ERROR((opcode == OQSPI_WRITE_STATUS_REG_OPCODE && address == 0x00000000) ||
                     (opcode == OQSPI_MACRONIX_OCTA_WRITE_CFG_REG2_OPCODE));

        hw_oqspi_cs_enable();

        switch (bus_mode) {
        case HW_OQSPI_BUS_MODE_SINGLE:
                hw_oqspi_write8(opcode);
                break;
        case HW_OQSPI_BUS_MODE_OCTA:
                hw_oqspi_write16(CONVERT_OPCODE_TO_DUAL_BYTE(opcode));
                break;
        default:
                ASSERT_ERROR(0);
        }

        hw_oqspi_write32(address);
        hw_oqspi_write8(value);
        hw_oqspi_cs_disable();
}

__UNUSED __RETAINED_CODE static uint8_t oqspi_macronix_octa_read_status_reg(HW_OQSPI_BUS_MODE bus_mode)
{
        __DBG_OQSPI_VOLATILE__ uint8_t status_reg;

        status_reg = oqspi_macronix_octa_read_register(OQSPI_READ_STATUS_REG_OPCODE, 0x00, 0xFF, bus_mode);

        return status_reg;
}

__UNUSED __RETAINED_CODE static void oqspi_macronix_octa_write_status_reg(HW_OQSPI_BUS_MODE bus_mode, uint8_t value)
{
        oqspi_macronix_octa_write_register(OQSPI_WRITE_STATUS_REG_OPCODE, 0, value, bus_mode);
}

__UNUSED __RETAINED_CODE static uint8_t oqspi_macronix_octa_get_dummy_bytes(sys_clk_t sys_clk)
{
        switch (sys_clk) {
        case sysclk_RCHS_32:
        case sysclk_RCHS_64:
        case sysclk_XTAL32M:
                return 6;
        case sysclk_RCHS_96:
                return 10;
        // When the PLL160 is used as system clock the OQSPIC switches to clock divider 2 thus
        // the OQSPIC clock frequency is 80 MHz.
        case sysclk_PLL160:
                return 8;
        default:
                ASSERT_WARNING(0);
                return 0;
        }
}

__UNUSED __RETAINED_CODE static bool oqspi_macronix_octa_set_dummy_bytes(uint8_t dummy_bytes, HW_OQSPI_BUS_MODE bus_mode)
{
        __DBG_OQSPI_VOLATILE__ uint8_t read_dummy_bytes = 0xFF;
        __DBG_OQSPI_VOLATILE__ OQSPI_MX_DUMMY_BYTES mx_dummy_bytes = OQSPI_MX_DUMMY_BYTES_INVALID;

        switch (dummy_bytes) {
        case 20:
                mx_dummy_bytes = OQSPI_MX_DUMMY_BYTES_20;
                break;
        case 18:
                mx_dummy_bytes = OQSPI_MX_DUMMY_BYTES_18;
                break;
        case 16:
                mx_dummy_bytes = OQSPI_MX_DUMMY_BYTES_16;
                break;
        case 14:
                mx_dummy_bytes = OQSPI_MX_DUMMY_BYTES_14;
                break;
        case 12:
                mx_dummy_bytes = OQSPI_MX_DUMMY_BYTES_12;
                break;
        case 10:
                mx_dummy_bytes = OQSPI_MX_DUMMY_BYTES_10;
                break;
        case 8:
                mx_dummy_bytes = OQSPI_MX_DUMMY_BYTES_8;
                break;
        case 6:
                mx_dummy_bytes = OQSPI_MX_DUMMY_BYTES_6;
                break;
        default:
                return false;
        }

        oqspi_flash_write_enable(bus_mode);
        oqspi_macronix_octa_write_register(OQSPI_MACRONIX_OCTA_WRITE_CFG_REG2_OPCODE,
                                      OQSPI_MACRONIX_OCTA_CFG_REG2_DUMMY_BYTES_ADDR,
                                      mx_dummy_bytes, bus_mode);

        read_dummy_bytes = oqspi_macronix_octa_read_register(OQSPI_MACRONIX_OCTA_READ_CFG_REG2_OPCODE,
                                                        OQSPI_MACRONIX_OCTA_CFG_REG2_DUMMY_BYTES_ADDR,
                                                        OQSPI_MACRONIX_OCTA_DUMMY_BYTES_MASK, bus_mode);

        return (read_dummy_bytes == (uint8_t) mx_dummy_bytes);
}

__UNUSED __RETAINED_CODE static void oqspi_macronix_octa_sys_clock_cfg(sys_clk_t sys_clk)
{
        uint8_t dummy_bytes = oqspi_macronix_octa_get_dummy_bytes(sys_clk);

        oqspi_enter_manual_access_mode();
        ASSERT_ERROR(oqspi_macronix_octa_set_dummy_bytes(dummy_bytes, HW_OQSPI_BUS_MODE_OCTA));
        oqspi_automode_int_enter_auto_access_mode();

        hw_oqspi_set_dummy_bytes(dummy_bytes);
}

__UNUSED __RETAINED_CODE static bool oqspi_macronix_octa_enter_opi(void)
{
        __DBG_OQSPI_VOLATILE__ uint8_t opi_mode = 0x00;

        oqspi_flash_write_enable(HW_OQSPI_BUS_MODE_SINGLE);

        // Switch the device to OPI mode before switching the controller to Octa Bus mode
        oqspi_macronix_octa_write_register(OQSPI_MACRONIX_OCTA_WRITE_CFG_REG2_OPCODE,
                                      OQSPI_MACRONIX_OCTA_CFG_REG2_BUS_MODE_ADDR,
                                      OQSPI_MACRONIX_OCTA_BUS_MODE_STR_OPI,
                                      HW_OQSPI_BUS_MODE_SINGLE);

        oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_OCTA, true);

        // Confirm that the device has switched to OPI mode
        opi_mode = oqspi_macronix_octa_read_register(OQSPI_MACRONIX_OCTA_READ_CFG_REG2_OPCODE,
                                                OQSPI_MACRONIX_OCTA_CFG_REG2_BUS_MODE_ADDR,
                                                OQSPI_MACRONIX_OCTA_BUS_MODE_STR_OPI,
                                                HW_OQSPI_BUS_MODE_OCTA);
        return (opi_mode == OQSPI_MACRONIX_OCTA_BUS_MODE_STR_OPI);
}

__UNUSED __RETAINED_CODE static bool oqspi_macronix_octa_exit_opi(void)
{
        __DBG_OQSPI_VOLATILE__ uint8_t bus_mode = 0xFF;

        oqspi_flash_write_enable(HW_OQSPI_BUS_MODE_OCTA);

        // Disable OPI mode before switching the OQSPIC to single SPI mode
        oqspi_macronix_octa_write_register(OQSPI_MACRONIX_OCTA_WRITE_CFG_REG2_OPCODE,
                                      OQSPI_MACRONIX_OCTA_CFG_REG2_BUS_MODE_ADDR,
                                      OQSPI_MACRONIX_OCTA_BUS_MODE_SPI,
                                      HW_OQSPI_BUS_MODE_OCTA);

        oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_SINGLE, true);

        // Confirm that the device has switched to Single SPI mode
        bus_mode = oqspi_macronix_octa_read_register(OQSPI_MACRONIX_OCTA_READ_CFG_REG2_OPCODE,
                                                OQSPI_MACRONIX_OCTA_CFG_REG2_BUS_MODE_ADDR,
                                                OQSPI_MACRONIX_OCTA_BUS_MODE_MASK,
                                                HW_OQSPI_BUS_MODE_SINGLE);

        return (bus_mode == OQSPI_MACRONIX_OCTA_BUS_MODE_SPI);
}

__UNUSED __RETAINED_CODE static bool oqspi_macronix_octa_is_suspended(HW_OQSPI_BUS_MODE bus_mode)
{
        __DBG_OQSPI_VOLATILE__ uint8_t is_suspended;

        is_suspended = oqspi_macronix_octa_read_register(OQSPI_MACRONIX_OCTA_READ_SECURITY_REG_OPCODE, 0x00000000,
                                                    OQSPI_MACRONIX_OCTA_SUSPENDED_MASK, bus_mode);
        return (is_suspended ? true : false);
}

__UNUSED __RETAINED_CODE static bool oqspi_macronix_octa_is_busy(HW_OQSPI_BUS_MODE bus_mode, HW_OQSPI_BUSY_LEVEL busy_level)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUSY_LEVEL is_busy;

        is_busy = (HW_OQSPI_BUSY_LEVEL ) oqspi_macronix_octa_read_register(OQSPI_READ_STATUS_REG_OPCODE, 0x00000000,
                                                         OQSPI_STATUS_REG_BUSY_MASK, bus_mode);

        return ((is_busy == busy_level) ? true : false);
}

#endif /* _OQSPI_MACRONIX_OCTA_H_ */
/**
 * \}
 * \}
 */
