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
 * @file oqspi_gigadevice_quad.h
 *
 * @brief This file contains macros and functions used by the OQSPIC flash drivers of the quad
 *        Gigadevice flash memories.
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _OQSPI_GIGADEVICE_QUAD_H_
#define _OQSPI_GIGADEVICE_QUAD_H_

#include "oqspi_common.h"

#define OQSPI_GIGADEVICE_QUAD_MANUFACTURER_ID                   (0xC8)
#define OQSPI_GIGADEVICE_QUAD_GD25LQ_LE_TYPE                    (0x60)

#define OQSPI_GIGADEVICE_QUAD_READ_STATUS_REG2_OPCODE           (0x35)

#define OQSPI_GIGADEVICE_QUAD_SUSPEND_OPCODE                    (0x75)
#define OQSPI_GIGADEVICE_QUAD_RESUME_OPCODE                     (0x7A)

#define OQSPI_GIGADEVICE_QUAD_STATUS_REG2_ERASE_SUSPEND_BIT     (7)
#define OQSPI_GIGADEVICE_QUAD_STATUS_REG2_ERASE_SUSPEND_MASK    (1 << OQSPI_GIGADEVICE_QUAD_STATUS_REG2_ERASE_SUSPEND_BIT)

#define OQSPI_GIGADEVICE_QUAD_STATUS_REG2_PROGRAM_SUSPEND_BIT   (2)
#define OQSPI_GIGADEVICE_QUAD_STATUS_REG2_PROGRAM_SUSPEND_MASK  (1 << OQSPI_GIGADEVICE_QUAD_STATUS_REG2_PROGRAM_SUSPEND_BIT)

#define OQSPI_GIGADEVICE_QUAD_STATUS_REG2_SUSPEND_MASK          (OQSPI_GIGADEVICE_QUAD_STATUS_REG2_ERASE_SUSPEND_MASK | \
                                                                 OQSPI_GIGADEVICE_QUAD_STATUS_REG2_PROGRAM_SUSPEND_MASK)

#define OQSPI_GIGADEVICE_QUAD_STATUS_REG2_QUAD_ENABLE_BIT       (1)
#define OQSPI_GIGADEVICE_QUAD_STATUS_REG2_QUAD_ENABLE_MASK      (1 << OQSPI_GIGADEVICE_QUAD_STATUS_REG2_QUAD_ENABLE_BIT)

__UNUSED __RETAINED_CODE static uint8_t oqspi_gigadevice_quad_read_register(uint8_t opcode,  uint8_t mask)
{
        __DBG_OQSPI_VOLATILE__ uint8_t reg_val;

        ASSERT_ERROR((opcode == OQSPI_READ_STATUS_REG_OPCODE) ||
                     (opcode == OQSPI_GIGADEVICE_QUAD_READ_STATUS_REG2_OPCODE));

        hw_oqspi_cs_enable();
        hw_oqspi_write8(opcode);
        reg_val = hw_oqspi_read8();
        hw_oqspi_cs_disable();

        return (reg_val & mask);
}

__UNUSED __RETAINED_CODE static void oqspi_gigadevice_quad_write_register(uint8_t opcode, uint8_t value)
{
        ASSERT_ERROR((opcode == OQSPI_WRITE_STATUS_REG_OPCODE));

        hw_oqspi_cs_enable();
        hw_oqspi_write8(opcode);
        hw_oqspi_write8(value);
        hw_oqspi_cs_disable();
}

__UNUSED __RETAINED_CODE static void oqspi_gigadevice_quad_write_status_reg1_2(uint16_t value)
{
        hw_oqspi_cs_enable();
        hw_oqspi_write8(OQSPI_WRITE_STATUS_REG_OPCODE);
        hw_oqspi_write16(value);
        hw_oqspi_cs_disable();
}

__UNUSED __RETAINED_CODE static uint8_t oqspi_gigadevice_quad_read_status_reg(HW_OQSPI_BUS_MODE bus_mode)
{
        return oqspi_gigadevice_quad_read_register(OQSPI_READ_STATUS_REG_OPCODE, 0xFF);
}

__UNUSED __RETAINED_CODE static void oqspi_gigadevice_quad_write_status_reg(HW_OQSPI_BUS_MODE bus_mode, uint8_t value)
{
        oqspi_gigadevice_quad_write_register(OQSPI_WRITE_STATUS_REG_OPCODE, value);
}


__UNUSED __RETAINED_CODE static uint8_t oqspi_gigadevice_quad_get_dummy_bytes(sys_clk_t sys_clk)
{
        UNUSED_ARG(sys_clk);

        return 2;
}

__UNUSED __RETAINED_CODE static void oqspi_gigadevice_quad_sys_clock_cfg(sys_clk_t sys_clk)
{
        UNUSED_ARG(sys_clk);
}

__UNUSED __RETAINED_CODE static bool oqspi_gigadevice_quad_is_suspended(HW_OQSPI_BUS_MODE bus_mode)
{
        UNUSED_ARG(bus_mode);
        __DBG_OQSPI_VOLATILE__ uint8_t is_suspended;

        is_suspended = oqspi_gigadevice_quad_read_register(OQSPI_GIGADEVICE_QUAD_READ_STATUS_REG2_OPCODE,
                                                           OQSPI_GIGADEVICE_QUAD_STATUS_REG2_SUSPEND_MASK);
        return (is_suspended ? true : false);
}

__UNUSED __RETAINED_CODE static bool oqspi_gigadevice_quad_is_busy(HW_OQSPI_BUS_MODE bus_mode, HW_OQSPI_BUSY_LEVEL busy_level)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUSY_LEVEL is_busy;

        is_busy = (HW_OQSPI_BUSY_LEVEL ) (oqspi_gigadevice_quad_read_status_reg(bus_mode) & OQSPI_STATUS_REG_BUSY_MASK);

        return (is_busy == busy_level) ? true : false;
}

__UNUSED __RETAINED_CODE static void oqspi_gigadevice_quad_enable_quad_mode(void)
{
        __DBG_OQSPI_VOLATILE__ uint8_t status_reg2;

        status_reg2 = oqspi_gigadevice_quad_read_register(OQSPI_GIGADEVICE_QUAD_READ_STATUS_REG2_OPCODE, 0xFF);

        if (!(status_reg2 & OQSPI_GIGADEVICE_QUAD_STATUS_REG2_QUAD_ENABLE_MASK)) {
                __DBG_OQSPI_VOLATILE__ uint8_t status_reg1;
                __DBG_OQSPI_VOLATILE__ uint16_t status_reg;

                status_reg1 = oqspi_gigadevice_quad_read_register(OQSPI_READ_STATUS_REG_OPCODE, 0xFF);
                status_reg = ((status_reg1 | OQSPI_GIGADEVICE_QUAD_STATUS_REG2_QUAD_ENABLE_MASK) |
                              (status_reg2 << 8));

                oqspi_flash_write_enable(HW_OQSPI_BUS_MODE_SINGLE);
                oqspi_gigadevice_quad_write_status_reg1_2(status_reg);
                while (oqspi_gigadevice_quad_is_busy(HW_OQSPI_BUS_MODE_SINGLE, HW_OQSPI_BUSY_LEVEL_HIGH));
        }
}

#endif /* _OQSPI_GIGADEVICE_QUAD_H_ */
/**
 * \}
 * \}
 */
