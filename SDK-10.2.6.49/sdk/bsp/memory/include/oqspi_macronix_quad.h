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
 * @file oqspi_macronix_quad.h
 *
 * @brief This header contains macros and functions used by the OQSPIC flash drivers of the quad
 *        Macronix flash memories.
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _OQSPI_MACRONIX_QUAD_H_
#define _OQSPI_MACRONIX_QUAD_H_

#include "oqspi_common.h"

#define OQSPI_MACRONIX_QUAD_MANUFACTURER_ID                     (0xC2)
#define OQSPI_MACRONIX_QUAD_MX25U_TYPE                          (0x25)
#define OQSPI_MACRONIX_QUAD_MX25R_TYPE                          (0x28)

#define OQSPI_MACRONIX_QUAD_READ_CONFIG_REG_OPCODE              (0x15)
#define OQSPI_MACRONIX_QUAD_READ_SECURITY_REG_OPCODE            (0x2B)

#define OQSPI_MACRONIX_QUAD_PAGE_PROGRAM_4IO_OPCODE             (0x38)

#define OQSPI_MACRONIX_QUAD_SUSPEND_OPCODE                      (0xB0)
#define OQSPI_MACRONIX_QUAD_RESUME_OPCODE                       (0x30)

#define OQSPI_MACRONIX_QUAD_STATUS_REG_QUAD_ENABLE_BIT          (6)
#define OQSPI_MACRONIX_QUAD_STATUS_REG_QUAD_ENABLE_MASK         (1 << OQSPI_MACRONIX_QUAD_STATUS_REG_QUAD_ENABLE_BIT)

#define OQSPI_MACRONIX_QUAD_STATUS_REG3_ADDR_MODE_BIT           (0)
#define OQSPI_MACRONIX_QUAD_STATUS_REG3_ADDR_MODE_MASK          (1 << OQSPI_MACRONIX_QUAD_STATUS_REG3_ADDR_MODE_BIT)

#define OQSPI_MACRONIX_QUAD_STATUS_REG3_DRV_STRENGTH_BITS       (5)
#define OQSPI_MACRONIX_QUAD_STATUS_REG3_DRV_STRENGTH_MASK       (3 << OQSPI_MACRONIX_QUAD_STATUS_REG3_DRV_STRENGTH_BITS)

#define OQSPI_MACRONIX_QUAD_SECURITY_REG_ERASE_SUSPEND_BIT      (3)
#define OQSPI_MACRONIX_QUAD_SECURITY_REG_ERASE_SUSPEND_MASK     (1 << OQSPI_MACRONIX_QUAD_SECURITY_REG_ERASE_SUSPEND_BIT)

#define OQSPI_MACRONIX_QUAD_SECURITY_REG_PROGRAM_SUSPEND_BIT    (2)
#define OQSPI_MACRONIX_QUAD_SECURITY_REG_PROGRAM_SUSPEND_MASK   (1 << OQSPI_MACRONIX_QUAD_SECURITY_REG_PROGRAM_SUSPEND_BIT)

#define OQSPI_MACRONIX_QUAD_SECURITY_REG_SUSPEND_MASK           (OQSPI_MACRONIX_QUAD_SECURITY_REG_ERASE_SUSPEND_MASK | \
                                                                 OQSPI_MACRONIX_QUAD_SECURITY_REG_PROGRAM_SUSPEND_MASK)

__UNUSED __RETAINED_CODE static uint8_t oqspi_macronix_quad_read_register(uint8_t opcode,  uint8_t mask)
{
        __DBG_OQSPI_VOLATILE__ uint8_t reg_val;

        ASSERT_WARNING((opcode == OQSPI_READ_STATUS_REG_OPCODE) ||
                       (opcode == OQSPI_MACRONIX_QUAD_READ_CONFIG_REG_OPCODE) ||
                       (opcode == OQSPI_MACRONIX_QUAD_READ_SECURITY_REG_OPCODE));

        hw_oqspi_cs_enable();
        hw_oqspi_write8(opcode);
        reg_val = hw_oqspi_read8();
        hw_oqspi_cs_disable();

        return (reg_val & mask);
}

// This function is used by macronix flash memory families where the status register and the
// configuration register are modified simultaneously.
__UNUSED __RETAINED_CODE static void oqspi_macronix_quad_write_status_and_config_reg(uint8_t status_reg, uint8_t config_reg)
{
        // The hw_oqspi_write16() swaps the MSB with LSB, therefore the status_reg and config_reg
        // are swapped in regs too.
        __DBG_OQSPI_VOLATILE__ uint16_t regs = ((((uint16_t) (status_reg)) << 8) & 0xFF00) | config_reg;

        hw_oqspi_cs_enable();
        hw_oqspi_write8(OQSPI_WRITE_STATUS_REG_OPCODE);
        hw_oqspi_write16(regs);
        hw_oqspi_cs_disable();
}

__UNUSED __RETAINED_CODE static uint8_t oqspi_macronix_quad_read_status_reg(HW_OQSPI_BUS_MODE bus_mode)
{
        UNUSED_ARG(bus_mode);

        return oqspi_macronix_quad_read_register(OQSPI_READ_STATUS_REG_OPCODE, 0xFF);
}

__UNUSED __RETAINED_CODE static void oqspi_macronix_quad_write_status_reg(HW_OQSPI_BUS_MODE bus_mode, uint8_t status_reg)
{
        __DBG_OQSPI_VOLATILE__ uint8_t config_reg;

        config_reg = oqspi_macronix_quad_read_register(OQSPI_MACRONIX_QUAD_READ_CONFIG_REG_OPCODE, 0xFF);
        oqspi_macronix_quad_write_status_and_config_reg(status_reg, config_reg);
}

__UNUSED __RETAINED_CODE static bool oqspi_macronix_quad_is_suspended(HW_OQSPI_BUS_MODE bus_mode)
{
        __DBG_OQSPI_VOLATILE__ uint8_t is_suspended;

        UNUSED_ARG(bus_mode);

        is_suspended = oqspi_macronix_quad_read_register(OQSPI_MACRONIX_QUAD_READ_SECURITY_REG_OPCODE,
                                                         OQSPI_MACRONIX_QUAD_SECURITY_REG_SUSPEND_MASK);
        return (is_suspended ? true : false);
}

__UNUSED __RETAINED_CODE static bool oqspi_macronix_quad_is_busy(HW_OQSPI_BUS_MODE bus_mode, HW_OQSPI_BUSY_LEVEL busy_level)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUSY_LEVEL is_busy;

        is_busy = (HW_OQSPI_BUSY_LEVEL ) (oqspi_macronix_quad_read_status_reg(bus_mode) & OQSPI_STATUS_REG_BUSY_MASK);

        return (is_busy == busy_level) ? true : false;
}

#endif /* _OQSPI_MACRONIX_QUAD_H_ */
/**
 * \}
 * \}
 */
