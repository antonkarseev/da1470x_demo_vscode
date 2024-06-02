/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup PLA_MEMORY
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file oqspi_common.h
 *
 * @brief OQSPI flash driver common definitions
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _OQSPI_COMMON_H_
#define _OQSPI_COMMON_H_

#include <stdbool.h>
#include "hw_oqspi.h"

#if __DBG_OQSPI_ENABLED
#define __DBG_OQSPI_VOLATILE__           volatile
#pragma message "oqspi_automode.{h c} debugging mode enabled"
#else
#define __DBG_OQSPI_VOLATILE__
#endif

#define OQSPI_WRITE_STATUS_REG_OPCODE           (0x01)
#define OQSPI_WRITE_DISABLE_OPCODE              (0x04)
#define OQSPI_READ_STATUS_REG_OPCODE            (0x05)
#define OQSPI_WRITE_ENABLE_OPCODE               (0x06)
#define OQSPI_RESET_EN_OPCODE                   (0x66)
#define OQSPI_RESET_OPCODE                      (0x99)
#define OQSPI_READ3B_OPCODE                     (0x03)
#define OQSPI_FAST_READ_QUAD_OPCODE             (0xEB)
#define OQSPI_BLOCK_ERASE_OPCODE                (0x52)
#define OQSPI_CHIP_ERASE_OPCODE                 (0xC7)
#define OQSPI_SECTOR_ERASE_OPCODE               (0x20)
#define OQSPI_PAGE_PROGRAM_QPI_OPCODE           (0x02)
#define OQSPI_PAGE_PROGRAM_QUAD_OPCODE          (0x32)
#define OQSPI_READ_JEDEC_ID_OPCODE              (0x9F)
#define OQSPI_EXIT_CONTINUOUS_MODE_BYTE         (0xFF)
#define OQSPI_EXIT_CONTINUOUS_MODE_WORD         (0xFFFFFFFF)

#define OQSPI_RELEASE_POWER_DOWN_OPCODE         (0xAB)
#define OQSPI_ENTER_POWER_DOWN_OPCODE           (0xB9)

#define OQSPI_ENTER_QPI_OPCODE                  (0x38)
#define OQSPI_EXIT_QPI_OPCODE                   (0xFF)

/* Erase/Write in progress */
#define OQSPI_STATUS_REG_BUSY_BIT               (0)
#define OQSPI_STATUS_REG_BUSY_MASK              (1 << OQSPI_STATUS_REG_BUSY_BIT)

/* WE Latch bit */
#define OQSPI_STATUS_REG_WEL_BIT                (1)
#define OQSPI_STATUS_REG_WEL_MASK               (1 << OQSPI_STATUS_REG_WEL_BIT)

#define OQSPI_MEMORY_SIZE_1Mbit                 (1024 * 1024)
#define OQSPI_MEMORY_SIZE_2Mbits                (2 * OQSPI_MEMORY_SIZE_1Mbit)
#define OQSPI_MEMORY_SIZE_4Mbits                (4 * OQSPI_MEMORY_SIZE_1Mbit)
#define OQSPI_MEMORY_SIZE_8Mbits                (8 * OQSPI_MEMORY_SIZE_1Mbit)
#define OQSPI_MEMORY_SIZE_16Mbits               (16 * OQSPI_MEMORY_SIZE_1Mbit)
#define OQSPI_MEMORY_SIZE_32Mbits               (32 * OQSPI_MEMORY_SIZE_1Mbit)
#define OQSPI_MEMORY_SIZE_64Mbits               (64 * OQSPI_MEMORY_SIZE_1Mbit)
#define OQSPI_MEMORY_SIZE_128Mbits              (128 * OQSPI_MEMORY_SIZE_1Mbit)
#define OQSPI_MEMORY_SIZE_256Mbits              (256 * OQSPI_MEMORY_SIZE_1Mbit)
#define OQSPI_MEMORY_SIZE_512Mbits              (512 * OQSPI_MEMORY_SIZE_1Mbit)
#define OQSPI_MEMORY_SIZE_1Gbit                 (1024 * OQSPI_MEMORY_SIZE_1Mbit)

/*
 * Some Octa FLash memories (e.g. Macronix) use dual byte opcode where the second byte equals to the
 * inverted value of the first one. The same memories use single byte opcode when configured in single
 * bus mode. The macro USE_DUAL_BYTE_OPCODE() checks if the dual byte must be used. If this is the case,
 * the CONVERT_OPCODE_TO_DUAL_BYTE() has to be used to convert the single byte opcode to dual byte opcode.
 */
#define USE_DUAL_BYTE_OPCODE(opcode_len, bus_mode)      ((opcode_len == HW_OQSPI_OPCODE_LEN_2_BYTES) && \
                                                         (bus_mode == HW_OQSPI_BUS_MODE_OCTA))
#define CONVERT_OPCODE_TO_DUAL_BYTE(opcode)             ((((uint16_t) (opcode) << 8) & 0xFF00) | (~(opcode) & 0x00FF))

#if (dg_configUSE_SEGGER_FLASH_LOADER == 1) && (dg_configOQSPI_FLASH_AUTODETECT == 0)
#define PRODUCT_HEADER_STRUCT(_N_) \
__PACKED_STRUCT { \
        uint32_t busrtcmdA; \
        uint32_t busrtcmdB; \
        uint32_t ctrlmode; \
        uint16_t flash_config_section; \
        uint16_t flash_config_length; \
        uint8_t config_seq[_N_]; \
        uint16_t crc; \
}
#endif

typedef void (* oqspi_initialize_cb_t) (HW_OQSPI_BUS_MODE bus_mode, sys_clk_t sys_clk);
typedef void (* oqspi_sys_clk_cfg_cb_t) (sys_clk_t sys_clk);
typedef bool (* oqspi_exit_opi_qpi_cb_t) (void);
typedef uint8_t (* oqspi_get_dummy_bytes_cb_t) (sys_clk_t sys_clk);
typedef bool (* oqspi_is_suspended_cb_t) (HW_OQSPI_BUS_MODE bus_mode);
typedef bool (* oqspi_is_busy_cb_t) (HW_OQSPI_BUS_MODE bus_mode, HW_OQSPI_BUSY_LEVEL busy_level);
typedef uint8_t (* oqspi_read_status_reg_cb_t) (HW_OQSPI_BUS_MODE bus_mode);
typedef void (* oqspi_write_status_reg_cb_t) (HW_OQSPI_BUS_MODE bus_mode, uint8_t value);

/**
 * \brief       JEDEC ID struct
 */
typedef struct {
        uint8_t         manufacturer_id;        /**< JEDEC manufacturer ID */
        uint8_t         type;                   /**< JEDEC device type */
        uint8_t         density;                /**< JEDEC device density */
        uint8_t         density_mask;           /**< JEDEC device density mask. Used to mask
                                                     the device density reading, if necessary.
                                                     Otherwise must be set to 0xFF */
} jedec_id_t;

/**
 * \brief       OQSPI memory callbacks struct
 */
typedef struct {
        oqspi_initialize_cb_t           initialize_cb;          /**< Device initialization callback function */
        oqspi_sys_clk_cfg_cb_t          sys_clk_cfg_cb;         /**< Device system clock configuration callback
                                                                     function */
        oqspi_exit_opi_qpi_cb_t         exit_opi_qpi_cb;        /**< Callback function that exits the device
                                                                     from OPI/QPI mode (for Quad/Octa flashes
                                                                     respectively) */
        oqspi_get_dummy_bytes_cb_t      get_dummy_bytes_cb;    /**<  Callback function that returns the
                                                                     number of dummy bytes */
        oqspi_is_suspended_cb_t         is_suspended_cb;        /**< Callback function for checking if the
                                                                     device is in erase/program suspend state */
        oqspi_is_busy_cb_t              is_busy_cb;             /**< Callback function for checking if the device is busy */
        oqspi_read_status_reg_cb_t      read_status_reg_cb;     /**< Read status register callback function */
        oqspi_write_status_reg_cb_t     write_status_reg_cb;    /**< Write status register callback function */
} oqspi_callback_t;

/**
 * \brief       OQSPI memory delays
 */
typedef struct {
        uint16_t                        reset_usec;             /**< Reset delay (usec) */
        uint16_t                        power_down_usec;        /**< The minimum required delay (usec)
                                                                     to enter power down mode after
                                                                     sending the corresponding command */
        uint16_t                        release_power_down_usec;/**< The minimum required delay (usec)
                                                                     to release from power down mode
                                                                     after sending the corresponding
                                                                     command */
        uint16_t                        power_up_usec;          /**< Power Up delay (usec) */
} oqspi_delay_t;

/**
 * \brief       OQSPI memory configuration structure
 *
 * This struct is used to define a driver for a specific OQSPI memory.
 */
typedef struct {
        jedec_id_t                                      jedec;                          /**< JEDEC ID structure */
        uint32_t                                        size_mbits;                     /**< Memory size in Mbits */
        HW_OQSPI_ADDR_SIZE                              address_size : 1;               /**< Device address size (24bits or 32-bit) */
        HW_OQSPI_CLK_MODE                               clk_mode : 1;                   /**< Clock Mode */
        HW_OQSPI_OPCODE_LEN                             opcode_len : 1;                 /**< Opcode length of the command phase */
        hw_oqspi_read_instr_config_t                    read_instr_cfg;                 /**< Read instruction configuration struct */
        hw_oqspi_erase_instr_config_t                   erase_instr_cfg;                /**< Erase instruction configuration struct */
        hw_oqspi_read_status_instr_config_t             read_status_instr_cfg;          /**< Read status register instruction configuration struct */
        hw_oqspi_write_enable_instr_config_t            write_enable_instr_cfg;         /**< Write enable instruction configuration struct */
        hw_oqspi_page_program_instr_config_t            page_program_instr_cfg;         /**< Page program instruction configuration struct */
        hw_oqspi_suspend_resume_instr_config_t          suspend_resume_instr_cfg;       /**< Program and erase suspend/resume
                                                                                             instruction configuration struct */
        hw_oqspi_exit_continuous_mode_instr_config_t    exit_continuous_mode_instr_cfg; /**< Exit from continuous mode of operation
                                                                                             instruction configuration struct */
        oqspi_delay_t                                   delay;                          /**< OQSPI memory delays struct */
        oqspi_callback_t                                callback;                       /**< Callbacks struct */
        bool                                            resume_before_writing_regs;     /**< Resume the flash memory before writing the status
                                                                                             register or any other configuration registers.
                                                                                             Some flash memories reject these commands while
                                                                                             being in erase suspend mode, thus a flash erase
                                                                                             resume command must be issued in advance. This setting
                                                                                             is in scope only when the background flash operations
                                                                                             are enabled. Check the manufacturer datasheet and
                                                                                             set this flag accordingly. If the implementation of
                                                                                             the sys_clk_cfg_cb() doesn't make use of the
                                                                                             aforementioned commands this flag must be false */
} oqspi_flash_config_t;

#endif /* _OQSPI_COMMON_H_ */

/**
 * \}
 * \}
 */
