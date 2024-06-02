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
 * @file oqspi_winbond_quad.h
 *
 * @brief This header contains macros and functions used by the OQSPIC flash drivers of the quad
 *        Winbond flash memories.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _OQSPI_WINBOND_QUAD_H_
#define _OQSPI_WINBOND_QUAD_H_

#include "oqspi_common.h"

#ifndef OQSPI_WINBOND_QUAD_UNLOCK_PROTECTION
#define OQSPI_WINBOND_QUAD_UNLOCK_PROTECTION                    (0)
#endif

#define OQSPI_WINBOND_QUAD_MANUFACTURER_ID                      (0xEF)
#define OQSPI_WINBOND_QUAD_W25QXXXJWXM_TYPE                     (0x80)
#define OQSPI_WINBOND_QUAD_W25QXXXJWXQ_TYPE                     (0x60)

#define OQSPI_WINBOND_QUAD_READ_STATUS_REG2_OPCODE              (0x35)
#define OQSPI_WINBOND_QUAD_READ_STATUS_REG3_OPCODE              (0x15)
#define OQSPI_WINBOND_QUAD_WRITE_STATUS_REG2_OPCODE             (0x31)
#define OQSPI_WINBOND_QUAD_WRITE_STATUS_REG3_OPCODE             (0x11)

#define OQSPI_WINBOND_QUAD_ENTER_4B_ADDR_MODE_OPCODE            (0xB7)
#define OQSPI_WINBOND_QUAD_EXIT_4B_ADDR_MODE_OPCODE             (0xE9)

#define OQSPI_WINBOND_QUAD_SUSPEND_OPCODE                       (0x75)
#define OQSPI_WINBOND_QUAD_RESUME_OPCODE                        (0x7A)

#define OQSPI_WINBOND_QUAD_STATUS_REG_BP0_BIT                   (2)     // Block Protection Bit 0
#define OQSPI_WINBOND_QUAD_STATUS_REG_BP1_BIT                   (3)     // Block Protection Bit 1
#define OQSPI_WINBOND_QUAD_STATUS_REG_BP2_BIT                   (4)     // Block Protection Bit 2
#define OQSPI_WINBOND_QUAD_STATUS_REG_TB_BIT                    (5)     // Top/Bottom Protection Bit
#define OQSPI_WINBOND_QUAD_STATUS_REG_SEC_BIT                   (6)     // Sector/Block Protection Bit
#define OQSPI_WINBOND_QUAD_STATUS_REG_SRP_BIT                   (7)     // Status Register Protection Bit 0

#define OQSPI_WINBOND_QUAD_STATUS_REG_PROTECTION_MASK           ((1 << OQSPI_WINBOND_QUAD_STATUS_REG_BP0_BIT) | \
                                                                 (1 << OQSPI_WINBOND_QUAD_STATUS_REG_BP1_BIT) | \
                                                                 (1 << OQSPI_WINBOND_QUAD_STATUS_REG_BP2_BIT) | \
                                                                 (1 << OQSPI_WINBOND_QUAD_STATUS_REG_TB_BIT) | \
                                                                 (1 << OQSPI_WINBOND_QUAD_STATUS_REG_SEC_BIT) | \
                                                                 (1 << OQSPI_WINBOND_QUAD_STATUS_REG_SRP_BIT))

#define OQSPI_WINBOND_QUAD_STATUS_REG2_SRL_BIT                  (0)     // Status Register2 Protection Bit 1
#define OQSPI_WINBOND_QUAD_STATUS_REG2_CMP_BIT                  (6)     // Complement Protect Bit 1

#define OQSPI_WINBOND_QUAD_STATUS_REG2_PROTECTION_MASK          ((1 << OQSPI_WINBOND_QUAD_STATUS_REG2_SRL_BIT) | \
                                                                 (1 << OQSPI_WINBOND_QUAD_STATUS_REG2_CMP_BIT))

#define OQSPI_WINBOND_QUAD_STATUS_REG2_SUSPEND_BIT              (7)
#define OQSPI_WINBOND_QUAD_STATUS_REG2_SUSPEND_MASK             (1 << OQSPI_WINBOND_QUAD_STATUS_REG2_SUSPEND_BIT)

#define OQSPI_WINBOND_QUAD_STATUS_REG2_QUAD_ENABLE_BIT          (1)
#define OQSPI_WINBOND_QUAD_STATUS_REG2_QUAD_ENABLE_MASK         (1 << OQSPI_WINBOND_QUAD_STATUS_REG2_QUAD_ENABLE_BIT)

#define OQSPI_WINBOND_QUAD_STATUS_REG3_ADDR_MODE_BIT            (0)
#define OQSPI_WINBOND_QUAD_STATUS_REG3_ADDR_MODE_MASK           (1 << OQSPI_WINBOND_QUAD_STATUS_REG3_ADDR_MODE_BIT)

#define OQSPI_WINBOND_QUAD_STATUS_REG3_DRV_STRENGTH_BITS        (5)
#define OQSPI_WINBOND_QUAD_STATUS_REG3_DRV_STRENGTH_MASK        (3 << OQSPI_WINBOND_QUAD_STATUS_REG3_DRV_STRENGTH_BITS)

__UNUSED __RETAINED_CODE static uint8_t oqspi_winbond_quad_read_register(uint8_t opcode,  uint8_t mask)
{
        __DBG_OQSPI_VOLATILE__ uint8_t reg_val;

        ASSERT_ERROR((opcode == OQSPI_READ_STATUS_REG_OPCODE) ||
                     (opcode == OQSPI_WINBOND_QUAD_READ_STATUS_REG2_OPCODE) ||
                     (opcode == OQSPI_WINBOND_QUAD_READ_STATUS_REG3_OPCODE));

        hw_oqspi_cs_enable();
        hw_oqspi_write8(opcode);
        reg_val = hw_oqspi_read8();
        hw_oqspi_cs_disable();

        return (reg_val & mask);
}

__UNUSED __RETAINED_CODE static void oqspi_winbond_quad_write_register(uint8_t opcode, uint8_t value)
{
        ASSERT_ERROR((opcode == OQSPI_WRITE_STATUS_REG_OPCODE) ||
                     (opcode == OQSPI_WINBOND_QUAD_WRITE_STATUS_REG2_OPCODE) ||
                     (opcode == OQSPI_WINBOND_QUAD_WRITE_STATUS_REG3_OPCODE));

        hw_oqspi_cs_enable();
        hw_oqspi_write8(opcode);
        hw_oqspi_write8(value);
        hw_oqspi_cs_disable();
}

__UNUSED __RETAINED_CODE static uint8_t oqspi_winbond_quad_read_status_reg(HW_OQSPI_BUS_MODE bus_mode)
{
        return oqspi_winbond_quad_read_register(OQSPI_READ_STATUS_REG_OPCODE, 0xFF);
}

__UNUSED __RETAINED_CODE static void oqspi_winbond_quad_write_status_reg(HW_OQSPI_BUS_MODE bus_mode, uint8_t value)
{
        oqspi_winbond_quad_write_register(OQSPI_WRITE_STATUS_REG_OPCODE, value);
}

__UNUSED __RETAINED_CODE static uint8_t oqspi_winbond_quad_get_dummy_bytes(sys_clk_t sys_clk)
{
        UNUSED_ARG(sys_clk);

        return 2;
}

__UNUSED __RETAINED_CODE static void oqspi_winbond_quad_sys_clock_cfg(sys_clk_t sys_clk)
{
        UNUSED_ARG(sys_clk);
}

__UNUSED __RETAINED_CODE static bool oqspi_winbond_quad_is_suspended(HW_OQSPI_BUS_MODE bus_mode)
{
        UNUSED_ARG(bus_mode);
        __DBG_OQSPI_VOLATILE__ uint8_t is_suspended;

        is_suspended = oqspi_winbond_quad_read_register(OQSPI_WINBOND_QUAD_READ_STATUS_REG2_OPCODE,
                                                        OQSPI_WINBOND_QUAD_STATUS_REG2_SUSPEND_MASK);
        return (is_suspended ? true : false);
}

__UNUSED __RETAINED_CODE static bool oqspi_winbond_quad_is_busy(HW_OQSPI_BUS_MODE bus_mode, HW_OQSPI_BUSY_LEVEL busy_level)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUSY_LEVEL is_busy;

        is_busy = (HW_OQSPI_BUSY_LEVEL ) (oqspi_winbond_quad_read_status_reg(bus_mode) & OQSPI_STATUS_REG_BUSY_MASK);

        return (is_busy == busy_level) ? true : false;
}

__UNUSED __RETAINED_CODE static void oqspi_winbond_quad_enable_quad_mode(void)
{
        __DBG_OQSPI_VOLATILE__ uint8_t quad_mode;

        quad_mode = oqspi_winbond_quad_read_register(OQSPI_WINBOND_QUAD_READ_STATUS_REG2_OPCODE, 0xFF);

        if (!(quad_mode & OQSPI_WINBOND_QUAD_STATUS_REG2_QUAD_ENABLE_MASK)) {
                oqspi_flash_write_enable(HW_OQSPI_BUS_MODE_SINGLE);
                oqspi_winbond_quad_write_register(OQSPI_WINBOND_QUAD_WRITE_STATUS_REG2_OPCODE,
                                                  (quad_mode | OQSPI_WINBOND_QUAD_STATUS_REG2_QUAD_ENABLE_MASK));
                while (oqspi_winbond_quad_is_busy(HW_OQSPI_BUS_MODE_SINGLE, HW_OQSPI_BUSY_LEVEL_HIGH));
        }
}

__UNUSED __UNUSED __RETAINED_CODE static void oqspi_winbond_quad_set_max_drive_strength(void)
{
        __DBG_OQSPI_VOLATILE__ uint8_t drive_strength;

        drive_strength = oqspi_winbond_quad_read_register(OQSPI_WINBOND_QUAD_READ_STATUS_REG3_OPCODE, 0xFF);

        if ((drive_strength & OQSPI_WINBOND_QUAD_STATUS_REG3_DRV_STRENGTH_MASK) != 0) {
                oqspi_flash_write_enable(HW_OQSPI_BUS_MODE_SINGLE);
                oqspi_winbond_quad_write_register(OQSPI_WINBOND_QUAD_WRITE_STATUS_REG3_OPCODE,
                                                  (drive_strength & (~OQSPI_WINBOND_QUAD_STATUS_REG3_DRV_STRENGTH_MASK)));

                while (oqspi_winbond_quad_is_busy(HW_OQSPI_BUS_MODE_SINGLE, HW_OQSPI_BUSY_LEVEL_HIGH));
        }
}

__UNUSED __RETAINED_CODE static void oqspi_winbond_quad_enter_addr_mode_4b(void)
{
        __DBG_OQSPI_VOLATILE__ uint8_t addr_mode_4b;

        do {
                hw_oqspi_cs_enable();
                hw_oqspi_write8(OQSPI_WINBOND_QUAD_ENTER_4B_ADDR_MODE_OPCODE);
                hw_oqspi_cs_disable();

                addr_mode_4b = oqspi_winbond_quad_read_register(OQSPI_WINBOND_QUAD_READ_STATUS_REG3_OPCODE,
                                                                OQSPI_WINBOND_QUAD_STATUS_REG3_ADDR_MODE_MASK);
        } while (!addr_mode_4b);
}

#if OQSPI_WINBOND_QUAD_UNLOCK_PROTECTION
__RETAINED_CODE static void oqspi_winbond_quad_unlock_protection(void)
{
        __DBG_OQSPI_VOLATILE__ uint8_t status_reg;
        __DBG_OQSPI_VOLATILE__ uint8_t status_reg2;
        __DBG_OQSPI_VOLATILE__ uint8_t verify;

        status_reg = oqspi_winbond_quad_read_status_reg(HW_OQSPI_BUS_MODE_SINGLE);
        status_reg2 = oqspi_winbond_quad_read_register(OQSPI_WINBOND_QUAD_READ_STATUS_REG2_OPCODE, 0xFF);

        // Clear Protection Bits [SRP TB BP3 BP2 BP1 BP0]
        if (status_reg & OQSPI_WINBOND_QUAD_STATUS_REG_PROTECTION_MASK) {
                status_reg &= ~OQSPI_WINBOND_QUAD_STATUS_REG_PROTECTION_MASK;
                oqspi_flash_write_enable(HW_OQSPI_BUS_MODE_SINGLE);
                oqspi_winbond_quad_write_status_reg(HW_OQSPI_BUS_MODE_SINGLE, status_reg);
                while (oqspi_winbond_quad_is_busy(HW_OQSPI_BUS_MODE_SINGLE, HW_OQSPI_BUSY_LEVEL_HIGH));
                verify = oqspi_winbond_quad_read_status_reg(HW_OQSPI_BUS_MODE_SINGLE);
                ASSERT_WARNING(status_reg == verify);
        }

        if (status_reg2 & OQSPI_WINBOND_QUAD_STATUS_REG2_PROTECTION_MASK) {
                status_reg2 &= ~OQSPI_WINBOND_QUAD_STATUS_REG2_PROTECTION_MASK;
                oqspi_flash_write_enable(HW_OQSPI_BUS_MODE_SINGLE);
                oqspi_winbond_quad_write_register(OQSPI_WINBOND_QUAD_WRITE_STATUS_REG2_OPCODE, status_reg2);
                while (oqspi_winbond_quad_is_busy(HW_OQSPI_BUS_MODE_SINGLE, HW_OQSPI_BUSY_LEVEL_HIGH));
                verify = oqspi_winbond_quad_read_register(OQSPI_WINBOND_QUAD_READ_STATUS_REG2_OPCODE, 0xFF);
                ASSERT_WARNING(status_reg2 == verify);
        }
}
#endif

#endif /* _OQSPI_WINBOND_QUAD_H_ */
/**
 * \}
 * \}
 */
