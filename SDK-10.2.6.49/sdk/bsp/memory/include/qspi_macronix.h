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
 * @file qspi_macronix.h
 *
 * @brief QSPI flash driver for Macronix flashes - common code
 *
 * Copyright (C) 2016-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _QSPI_MACRONIX_H_
#define _QSPI_MACRONIX_H_

#define MACRONIX_ID                             0xC2

#include "qspi_common.h"

#define MACRONIX_PERFORMANCE_MODE               1       // bootrom does not support Macronix performance mode

#define MX_QUAD_IO_PAGE_PROGRAM                 0x38
#define MX_QUAD_IO_PAGE_PROGRAM_4B              0x3E

#define MX_ERASE_PROGRAM_SUSPEND                0xB0
#define MX_ERASE_PROGRAM_RESUME                 0x30

#define MX_READ_SECURITY_REGISTER               0x2B
#define MX_READ_CONFIG_REGISTER                 0x15

#define MX_STATUS_QE_BIT                        6 // Quad Enable
#define MX_STATUS_QE_MASK                       (1 << MX_STATUS_QE_BIT)

#define MX_STATUS_SRWD_BIT                      7 // Status register write disable
#define MX_STATUS_SRWD_MASK                     (1 << MX_STATUS_SRWD_BIT)

/* Suspend */
#define MX_SECURITY_ESB_BIT                     3 // Erase suspend bit
#define MX_SECURITY_ESB_MASK                    (1 << MX_SECURITY_ESB_BIT)

#define MX_SECURITY_PSB_BIT                     2 // Program suspend bit
#define MX_SECURITY_PSB_MASK                    (1 << MX_SECURITY_PSB_BIT)

#define MX_CONFIG_DC_BIT                        6 // dummy cycle offset
#define MX_CONFIG_DC_MASK                       (0x3 << MX_CONFIG_DC_BIT)

#define MX_CONFIG_ODS_BIT                       0 // Output driver strength
#define MX_CONFIG_ODS_MASK                      (0x7 << MX_CONFIG_ODS_BIT)

#define MX_CONFIG2_HIGH_PERFORMANCE_BIT         1 // Program suspend bit
#define MX_CONFIG2_HIGH_PERFORMANCE_MASK        (1 << MX_CONFIG2_HIGH_PERFORMANCE_BIT)

// Device type using command 0x9F
#define MX25L_SERIES                            0x20

typedef enum {
        FLASH_MX25U_ODS_146OHM = 0,
        FLASH_MX25U_ODS_76OHM  = 1,
        FLASH_MX25U_ODS_52OHM  = 2,
        FLASH_MX25U_ODS_41OHM  = 3,
        FLASH_MX25U_ODS_34OHM  = 4,
        FLASH_MX25U_ODS_30OHM  = 5,
        FLASH_MX25U_ODS_26OHM  = 6,
        FLASH_MX25U_ODS_24OHM  = 7,
} FLASH_MX25U_ODS;

typedef enum {
        FLASH_MX25L_MX66U_ODS_90OHM = 1,
        FLASH_MX25L_MX66U_ODS_60OHM = 2,
        FLASH_MX25L_MX66U_ODS_45OHM = 3,
        FLASH_MX25L_MX66U_ODS_20OHM = 5,
        FLASH_MX25L_MX66U_ODS_15OHM = 6,
        FLASH_MX25L_MX66U_ODS_30OHM = 7,
} FLASH_MX25L_MX66U_ODS;

typedef enum {
        FLASH_MX_CONFIG_REG_DC_6 = 0x00,
        FLASH_MX_CONFIG_REG_DC_4 = 0x01,
        FLASH_MX_CONFIG_REG_DC_8 = 0x02,
        FLASH_MX_CONFIG_REG_DC_10 = 0x03,
} FLASH_MX_CONFIG_REG_DC;

// This bit will be cleared when the Status register is read
__RETAINED_RW static uint8_t flash_mx_status_reg = MX_STATUS_SRWD_MASK;
__RETAINED static uint8_t flash_mx_conf_reg;

__STATIC_FORCEINLINE uint8_t flash_mx_read_config_register(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t config_reg;
        uint8_t cmd[] = { MX_READ_CONFIG_REGISTER };

        flash_transact(id, cmd, 1, &config_reg, 1);

        return config_reg;
}

__STATIC_FORCEINLINE void flash_mx_write_config_register(HW_QSPIC_ID id, uint8_t config_reg)
{
        uint8_t cmd[]= { CMD_WRITE_STATUS_REGISTER, flash_mx_status_reg, config_reg };

        flash_write_enable(id);
        flash_write(id, cmd, 3);
        while (flash_is_busy(id));
}

__STATIC_FORCEINLINE __UNUSED uint8_t flash_mx_read_security_register(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t security_reg;
        uint8_t cmd[] = { MX_READ_SECURITY_REGISTER };

        flash_transact(id, cmd, 1, &security_reg, 1);

        return security_reg;
}

__STATIC_FORCEINLINE void flash_mx_enable_quad_mode(HW_QSPIC_ID id)
{
        flash_mx_status_reg = flash_read_status_register(id);

        if (!(flash_mx_status_reg & MX_STATUS_QE_MASK)) {
                flash_write_enable(id);
                flash_write_status_register(id, flash_mx_status_reg | MX_STATUS_QE_MASK);
        }
}

__STATIC_FORCEINLINE __UNUSED void flash_mx_set_output_driver_strength(HW_QSPIC_ID id, uint8_t ods_value)
{
        ASSERT_ERROR((ods_value & ~MX_CONFIG_ODS_MASK) == 0);

        flash_mx_status_reg = flash_read_status_register(id);
        flash_mx_conf_reg = flash_mx_read_config_register(id);
        flash_mx_conf_reg = (flash_mx_conf_reg & ~MX_CONFIG_ODS_MASK) | (ods_value << MX_CONFIG_ODS_BIT);

        flash_mx_write_config_register(id, flash_mx_conf_reg);
}

__STATIC_FORCEINLINE void flash_mx_set_dummy_bytes(HW_QSPIC_ID id, uint8_t dummy_bytes)
{
        uint8_t config_reg_dc;

        // flash_mx_status_reg must be read from device and SRWD must be 0
        ASSERT_ERROR(flash_mx_status_reg != MX_STATUS_SRWD_MASK);

        switch (dummy_bytes) {
        case 1:
                config_reg_dc = (uint8_t) FLASH_MX_CONFIG_REG_DC_4;
                break;
        case 2:
                config_reg_dc = (uint8_t) FLASH_MX_CONFIG_REG_DC_6;
                break;
        case 3:
                config_reg_dc = (uint8_t) FLASH_MX_CONFIG_REG_DC_8;
                break;
        case 4:
                config_reg_dc = (uint8_t) FLASH_MX_CONFIG_REG_DC_10;
                break;
        default:
                ASSERT_ERROR(0);
                return;
        }

        config_reg_dc = config_reg_dc << MX_CONFIG_DC_BIT;
        flash_mx_conf_reg =  flash_mx_read_config_register(id);

        if ((flash_mx_conf_reg & MX_CONFIG_DC_MASK) != config_reg_dc) {
                flash_mx_conf_reg = ((flash_mx_conf_reg & ~MX_CONFIG_DC_MASK) | config_reg_dc);
                flash_mx_write_config_register(id, flash_mx_conf_reg);
        }
}

__STATIC_FORCEINLINE void flash_mx_set_high_performance(HW_QSPIC_ID id)
{
        uint8_t cmd[] = { MX_READ_CONFIG_REGISTER };
        uint8_t conf_reg[2];
        uint8_t new_value;
        uint8_t status;

        status = flash_read_status_register(id);
        flash_transact(id, cmd, 1, conf_reg, 2);

        new_value = conf_reg[1] | MX_CONFIG2_HIGH_PERFORMANCE_MASK;
        uint8_t wr_cmd[] = {CMD_WRITE_STATUS_REGISTER, status, conf_reg[0], new_value };

        flash_write_enable(id);
        flash_write(id, wr_cmd, 4);

        while (flash_is_busy(id));
}

__RETAINED_CODE static bool flash_mx_is_suspended(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t security_reg;

        security_reg = flash_mx_read_security_register(id);
        return (security_reg & (MX_SECURITY_ESB_MASK | MX_SECURITY_PSB_MASK)) != 0;
}

#endif /* _QSPI_MACRONIX_H_ */
/**
 * \}
 * \}
 */
