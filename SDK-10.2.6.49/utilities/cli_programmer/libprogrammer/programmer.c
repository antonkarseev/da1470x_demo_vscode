/**
 ****************************************************************************************
 *
 * @file programmer.c
 *
 * @brief Bootloader API.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <uartboot_types.h>
#include <suota.h>

#include <programmer.h>

#include "gdb_server_cmds.h"
#include "protocol_cmds.h"
#include "serial.h"

/*
 * this is 'magic' address which can be used in some commands to indicate some kind of temporary
 * storage, i.e. command needs to store some data but does not care where as long as it can be
 * accessed later
 */
#define ADDRESS_TMP                     (0xFFFFFFFF)
#define VIRTUAL_BUF_ADDRESS             (0x80000000)

/*
 * 680 specific chip registers
 */
static const prog_chip_regs_t chip_680_regs = {
        /* .sys_ctrl_reg = */                   0x50000012,
        /* .chip_id1_reg = */                   0x50003200,
        /* .chip_id2_reg = */                   0x50003201,
        /* .chip_id3_reg = */                   0x50003202,
        /* .chip_revision_reg = */              0x50003204,
        /* .chip_test1_reg = */                 0x5000320A,
        /* .otp_start_address = */              0x7F80000,
        /* .otp_size = */                       0x10000,
        /* .otp_header_chip_id = */             0x7F8EA20,
        /* .otp_header_pos_pack_info = */       0x7F8EA00,
        /* .memory_sysram_base = */             0x7FC0000,
        /* .memory_sysram_end = */              0x7FE4000,
        /* .memory_qspif_base = */              0x7F80000,
        /* .memory_qspif_end = */               0xA0000000,
        /* .magic_value1_reg = */               0x7FD0000,
        /* .magic_value2_reg = */               0x7FD0004,
        /* .magic_value3_reg = */               0x7FD0008,
        /* .magic_value4_reg = */               0x7FD000C,
        /* .swd_reset_reg = */                  0x400C3050,
        /* .virtual_buf_mask = */               0xFFFC0000,
        /* .sys_ctrl_reg_val = */               0x00AB,
};

/*
 * 690 specific chip registers
 */
static const prog_chip_regs_t chip_690_regs = {
        /* .sys_ctrl_reg = */                   0x50000024,
        /* .chip_id1_reg = */                   0x50040200,
        /* .chip_id2_reg = */                   0x50040204,
        /* .chip_id3_reg = */                   0x50040208,
        /* .chip_revision_reg = */              0x50040214,
        /* .chip_test1_reg = */                 0x500402F8,
        /* .otp_start_address = */              0x10080000,
        /* .otp_size = */                       0x1000,
        /* .otp_header_chip_id = */             0x0,
        /* .otp_header_pos_pack_info = */       0x0,
        /* .memory_sysram_base = */             0x20000000,
        /* .memory_sysram_end = */              0x20080000,
        /* .memory_qspif_base = */              0x16000000,
        /* .memory_qspif_end = */               0x18000000,
        /* .magic_value1_reg = */               0x20010000,
        /* .magic_value2_reg = */               0x20010004,
        /* .magic_value3_reg = */               0x20010008,
        /* .magic_value4_reg = */               0x2001000C,
        /* .swd_reset_reg = */                  0x100C0050,
        /* .virtual_buf_mask = */               0xFFF80000,
        /* .sys_ctrl_reg_val = */               0x00C3,
};

/*
 * 700 specific chip registers
 */
static const prog_chip_regs_t chip_700_regs = {
        /* .sys_ctrl_reg = */                   0x50000024,
        /* .chip_id1_reg = */                   0x50040000,
        /* .chip_id2_reg = */                   0x50040004,
        /* .chip_id3_reg = */                   0x50040008,
        /* .chip_revision_reg = */              0x50040014,
        /* .chip_test1_reg = */                 0x500400F8,
        /* .otp_start_address = */              0x10080000,
        /* .otp_size = */                       0x1000,
        /* .otp_header_chip_id = */             0x0,
        /* .otp_header_pos_pack_info = */       0x0,
        /* .memory_sysram_base = */             0x20000000,
        /* .memory_sysram_end = */              0x20180000,
        /* .memory_qspif_base = */              0x18000000,
        /* .memory_qspif_end = */               0x20000000,
        /* .magic_value1_reg = */               0x0F001000,
        /* .magic_value2_reg = */               0x0F001004,
        /* .magic_value3_reg = */               0x0F001008,
        /* .magic_value4_reg = */               0x0F00100C,
        /* .swd_reset_reg = */                  0x100C0050,
        /* .virtual_buf_mask = */               0xFFF00000,
        /* .sys_ctrl_reg_val = */               0x00C5,
};

/*
 * 680 specific memory sizes
 */
static const prog_memory_sizes_t chip_680_mem_sizes = {
        /* .ram_size = */                       0x400 * 144,    /* 144 kB */
        /* .otp_size = */                       0x400 * 64,     /* 64 kB */
        /* .qspi_size = */                      0x100000 * 32,  /* 32 MB */
        /* .eflash_size = */                    0,              /* N/A */
        /* .oqspi_size = */                     0,              /* N/A */
};

/*
 * 690 specific memory sizes
 */
static const prog_memory_sizes_t chip_690_mem_sizes = {
        /* .ram_size = */                       0x400 * 512,    /* 512 kB */
        /* .otp_size = */                       0x400 * 4,      /* 4 kB */
        /* .qspi_size = */                      0x100000 * 32,  /* 32 MB */
        /* .eflash_size = */                    0,              /* N/A */
        /* .oqspi_size = */                     0,              /* N/A */
};

/*
 * 700 specific memory sizes
 */
static const prog_memory_sizes_t chip_700_mem_sizes = {
        /* .ram_size = */                       0x400 * 1536,   /* 1536 kB */
        /* .otp_size = */                       0x400 * 4,      /* 4 kB */
        /* .qspi_size = */                      0x400000 * 32,  /* 128 MB */
        /* .eflash_size = */                    0,              /* N/A */
        /* .oqspi_size = */                     0x400000 * 32,  /* 128 MB */
};

/*
 * Address of chip id in OTP header.
 */
#define OTP_HEADER_CHIP_ID      0x7F8EA20
#define OTP_HEADER_CHIP_ID_LEN  0x8

/*
 * Address of position/package in OTP header.
 */
#define OTP_HEADER_POS_PACK_INFO        0x7F8EA00
#define OTP_HEADER_POS_PACK_INFO_LEN    0x8

/*
 * Erase sector mask
 */
#define FLASH_ERASE_MASK (0x0FFF)

/*
 * Pointer will hold memory for bootloader code
 *
 */
static uint8_t *boot_loader = NULL;

/*
 * Variables will hold the mode in which the dll operates
 *
 */
static bool gdb_gui_mode = false;
static bool block_otp_write = false;

/*
 * This array will store chip revision set by the user
 */
static char prog_chip_rev[CHIP_REV_STRLEN] = CHIP_REV_680BB;

/*
 *
 * buffers for gui mode
 */
#define STDOUT_BUF_SIZE 128
#define STDERR_BUF_SIZE 128
static char stdout_msg[STDOUT_BUF_SIZE];
static char stderr_msg[STDERR_BUF_SIZE];
static char copy_stdout_msg[STDOUT_BUF_SIZE];
static char copy_stderr_msg[STDERR_BUF_SIZE];

static unsigned int prog_intial_baudrate;
static unsigned int uartTimeoutInMs = 5000;
char *target_reset_cmd;

typedef struct {
        /**
         * \brief Close the target interface
         *
         * \note In the GDB Server interface case this is a selected process ID. In the serial
         * interface case this could be anything (currently not used).
         *
         * \param [in] data interface dependent data
         *
         */
        void (*close)(int data);

        /**
         * \brief Verify board connection status
         *
         * \return CONN_ESTABLISHED if uartboot is uploaded to the device
         *         CONN_ALLOWED if uartboot can be uploaded
         *         CONN_ERROR if uartboot can not be uploaded
         */
        connection_status_t (*verify_connection)(void);

        /**
         * \brief Set the bootloader code which updates the firmware
         *
         * Binary data specified in this command will be sent to the device.
         *
         * \param [in] code binary data to send to first stage boot loader
         * \param [in] size code size
         *
         */
        void (*set_boot_loader_code)(uint8_t *code, size_t size);

        /**
         * \brief Get the bootloader code which updates the firmware
         *
         * \param [out] code binary data that will be sent to first stage boot loader
         * \param [out] size code size
         *
         */
        void (*get_boot_loader_code)(uint8_t **code, size_t * size);

        /**
         * \brief Read the device memory
         *
         * This function reads the device memory as seen by the device CPU.
         *
         * \param [out] buf buffer for data
         * \param [in]  size size of buf in bytes
         * \param [in]  addr device CPU address to read from
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_read)(uint8_t *buf, size_t size, uint32_t addr);

        /**
         * \brief Write the device memory
         *
         * This function writes the specified data to the device RAM.
         *
         * \param [in] buf binary data to send to device
         * \param [in] size size of buf
         * \param [in] addr device CPU address at device to write data to
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_write)(const uint8_t *buf, size_t size, uint32_t addr);

        /**
         * \brief Read QSPI flash info
         *
         * Reads manufacturer ID as well as device type and density of QSPI flash.
         *
         * \param [in]  id QSPI controller ID
         * \param [out] qspi_flash_info pointer to the struct where QSPI flash info is stored
         *
         * \return 0 on success, error code on failure
         */
        int (*cmd_get_qspi_state)(uint8_t id, flash_dev_info_t *qspi_flash_info);

        /**
         * \brief Erase the QSPI flash region
         *
         * This function erases the flash at a specified offset.
         *
         * \param [in] address address in flash to start erase
         * \param [in] size size of memory to erase
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_erase_qspi)(uint32_t address, size_t size);

        /**
         * \brief Apply chip erase on QSPI
         *
         * This function completely erases the QSPI flash memory starting at \p address.
         *
         * \param [in] address Start address of the QSPI flash memory to be erased
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_chip_erase_qspi)(uint32_t address);

        /**
         * \brief Read data from QSPI
         *
         * \param [in] address offset in flash
         * \param [out] buf buffer for data
         * \param [in] len length of buffer
         *
         * \return 0 on success, error code on failure
         *
         */
        int (*cmd_read_qspi)(uint32_t address, uint8_t *buf, uint32_t len);

        /**
         * \brief Check emptiness of QSPI flash
         *
         * If specified flash region is empty, parameter \p ret_number is a number of checked bytes
         * (positive value). Otherwise if specified flash region contains values different than 0xFF,
         * parameter \p ret_number is a nonpositive value. This value is a number of first non 0xFF byte in
         * checked region multiplied by -1.
         *
         * \param [in]  size number of bytes to check
         * \param [in]  start_address start address in QSPI flash
         * \param [out] ret_number number of checked bytes, or number of first non 0xFF byte multiplied by -1
         *
         * \return 0 on success, error code on failure
         *
         */
        int (*cmd_is_empty_qspi)(unsigned int size, unsigned int start_address, int *ret_number);

        /**
         * \brief Read the partition table
         *
         * \param [out] buf buffer to store the contents of the partition table
         * \param [out] len the size of buffer in bytes
         *
         * \return 0 on success, error code on failure
         *
         */
        int (*cmd_read_partition_table)(uint8_t **buf, uint32_t *len);

        /**
         * \brief Read data from a specific partition
         *
         * \param [in] id partition id
         * \param [in] address offset in flash
         * \param [out] buf buffer for data
         * \param [in] len length of buffer
         *
         * \return 0 on success, error code on failure
         *
         */
        int (*cmd_read_partition)(nvms_partition_id_t id, uint32_t address, uint8_t *buf,
                                                                                uint32_t len);

        /**
         * \brief Write memory to NVMS partition
         *
         * This function programs NVMS partition memory with data already present in the device RAM.
         *
         * \param [in] id partition id
         * \param [in] dst_address offset in partition to write to
         * \param [in] src_address address in the device RAM
         * \param [in] size size of memory to copy
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_write_partition)(nvms_partition_id_t id, uint32_t dst_address,
                                                                uint32_t src_address, size_t size);

        /**
         * \brief Copy memory to QSPI flash
         *
         * This function programs QSPI flash memory with data already present in the device RAM.
         *
         * \param [in] src_address address in the device RAM
         * \param [in] size size of memory to copy
         * \param [in] dst_address offset in flash to write to
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_copy_to_qspi)(uint32_t src_address, size_t size, uint32_t dst_address);

        /**
         * \brief Write data to QSPI FLASH
         *
         * This function writes the specified data to the device QSPI FLASH memory.
         *
         * \param [in] buf binary data to send to device
         * \param [in] size size of buf
         * \param [in] addr offset in flash to write to
         * \param [in] verify true for performing QSPI writing verification
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_direct_write_to_qspi)(const uint8_t *buf, size_t size, uint32_t addr, bool verify);

        /**
         * \brief Read data from OTP
         *
         * \param [in] address address of the starting cell in the OTP
         * \param [out] buf buffer for read words
         * \param [in] len number of words to be read
         *
         * \return 0 on success, error code on failure
         *
         */
        int (*cmd_read_otp)(uint32_t address, uint32_t *buf, uint32_t len);

        /**
         * \brief Write data to OTP
         *
         * \param [in] address address of the starting cell in the OTP
         * \param [in] buf words to be written
         * \param [in] len number of words in buffer
         *
         * \return 0 on success, error code on failure
         *
         */
        int (*cmd_write_otp)(uint32_t address, const uint32_t *buf, uint32_t len);

        /**
         * \brief Execute code on the device
         *
         * This function starts execution of the code on the device.
         *
         * \param [in] address address in device RAM to execute
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_run)(uint32_t address);

        /**
         * \brief Boot arbitrary binary.
         *
         * This function boots the device with a provided application binary.
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_boot) (uint8_t *executable_code, size_t executable_code_size);

        /**
         * \brief Force upload bootloader.
         *
         * This function forces uploads second stage bootloader.
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_upload_bootloader)(void);

        /**
         * \brief Apply mass erase on eFLASH
         *
         * This function erases the whole eFLASH memory.
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_mass_erase_eflash)(void);

        /**
         * \brief Read OQSPI flash info
         *
         * Reads manufacturer ID as well as device type and density of OQSPI flash.
         *
         * \param [out] oqspi_flash_info pointer to the struct where OQSPI flash info is stored
         *
         * \return 0 on success, error code on failure
         */
        int (*cmd_get_oqspi_state)(flash_dev_info_t *oqspi_flash_info);

        /**
         * \brief Erase the OQSPI flash region
         *
         * This function erases the flash at a specified offset.
         *
         * \param [in] address address in flash to start erase
         * \param [in] size size of memory to erase
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_erase_oqspi)(uint32_t address, size_t size);

        /**
         * \brief Apply chip erase on OQSPI
         *
         * This function completely erases the OQSPI flash memory starting at \p address.
         *
         * \param [in] address Start address of the OQSPI flash memory to be erased
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_chip_erase_oqspi)(uint32_t address);

        /**
         * \brief Read data from OQSPI
         *
         * \param [in] address offset in flash
         * \param [out] buf buffer for data
         * \param [in] len length of buffer
         *
         * \return 0 on success, error code on failure
         *
         */
        int (*cmd_read_oqspi)(uint32_t address, uint8_t *buf, uint32_t len);

        /**
         * \brief Check emptiness of OQSPI flash
         *
         * If specified flash region is empty, parameter \p ret_number is a number of checked bytes
         * (positive value). Otherwise if specified flash region contains values different than 0xFF,
         * parameter \p ret_number is a non-positive value. This value is a number of first non 0xFF byte in
         * checked region multiplied by -1.
         *
         * \param [in]  size number of bytes to check
         * \param [in]  start_address start address in OQSPI flash
         * \param [out] ret_number number of checked bytes, or number of first non 0xFF byte multiplied by -1
         *
         * \return 0 on success, error code on failure
         *
         */
        int (*cmd_is_empty_oqspi)(unsigned int size, unsigned int start_address, int *ret_number);

        /**
         * \brief Copy memory to OQSPI flash
         *
         * This function programs OQSPI flash memory with data already present in the device RAM.
         *
         * \param [in] src_address address in the device RAM
         * \param [in] size size of memory to copy
         * \param [in] dst_address offset in flash to write to
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_copy_to_oqspi)(uint32_t src_address, size_t size, uint32_t dst_address);

        /**
         * \brief Write data to OQSPI FLASH
         *
         * This function writes the specified data to the device OQSPI FLASH memory.
         *
         * \param [in] buf binary data to send to device
         * \param [in] size size of buf
         * \param [in] addr offset in flash to write to
         * \param [in] verify true for performing OQSPI writing verification
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_direct_write_to_oqspi)(const uint8_t *buf, size_t size, uint32_t addr, bool verify);

        /**
         * \brief Get product information.
         *
         * This function returns device classification and production information.
         *
         * \param [out] buf buffer to store the device classification and production information
         * \param [out] len the size of buffer in bytes
         *
         * \returns 0 on success, negative value with error code on failure
         *
         */
        int (*cmd_get_product_info)(uint8_t **buf, uint32_t *len);

        /**
         * \brief Maximal size of chunk which could be used for read commands
         */
        size_t read_chunk_size;

        /**
         * \brief Maximal size of chunk which could be used for write commands
         */
        size_t write_chunk_size;
} target_interface_t;

static const target_interface_t target_serial = {
        /* .close_interface = */                prog_serial_close,
        /* .verify_connection = */              protocol_verify_connection,
        /* .set_boot_loader_code = */           set_boot_loader_code,
        /* .get_boot_loader_code = */           get_boot_loader_code,
        /* .cmd_read = */                       protocol_cmd_read,
        /* .cmd_write = */                      protocol_cmd_write,
        /* .cmd_get_qspi_state */               protocol_cmd_get_qspi_state,
        /* .cmd_erase_qspi = */                 protocol_cmd_erase_qspi,
        /* .cmd_chip_erase_qspi = */            protocol_cmd_chip_erase_qspi,
        /* .cmd_read_qspi = */                  protocol_cmd_read_qspi,
        /* .cmd_is_empty_qspi = */              protocol_cmd_is_empty_qspi,
        /* .cmd_read_partition_table = */       protocol_cmd_read_partition_table,
        /* .cmd_read_partition = */             protocol_cmd_read_partition,
        /* .cmd_write_partition = */            protocol_cmd_write_partition,
        /* .cmd_copy_to_qspi = */               protocol_cmd_copy_to_qspi,
        /* .cmd_direct_write_to_qspi = */       protocol_cmd_direct_write_to_qspi,
        /* .cmd_read_otp = */                   protocol_cmd_read_otp,
        /* .cmd_write_otp = */                  protocol_cmd_write_otp,
        /* .cmd_run = */                        protocol_cmd_run,
        /* .cmd_boot = */                       protocol_cmd_boot,
        /* .cmd_upload_bootloader = */          protocol_cmd_upload_bootloader,
        /* .cmd_mass_erase_eflash = */          protocol_cmd_mass_erase_eflash,
        /* .cmd_get_oqspi_state */              protocol_cmd_get_oqspi_state,
        /* .cmd_erase_oqspi = */                protocol_cmd_erase_oqspi,
        /* .cmd_chip_erase_oqspi = */           protocol_cmd_chip_erase_oqspi,
        /* .cmd_read_oqspi = */                 protocol_cmd_read_oqspi,
        /* .cmd_is_empty_oqspi = */             protocol_cmd_is_empty_oqspi,
        /* .cmd_copy_to_oqspi = */              protocol_cmd_copy_to_oqspi,
        /* .cmd_direct_write_to_oqspi = */      protocol_cmd_direct_write_to_oqspi,
        /* .cmd_get_product_info = */           protocol_cmd_get_product_info,
        /* .read_chunk_size = */                PROTOCOL_READ_CHUNK_SIZE,
        /* .write_chunk_size = */               PROTOCOL_WRITE_CHUNK_SIZE,
};

static const target_interface_t target_gdb_server = {
        /* .close_interface = */                gdb_server_close,
        /* .verify_connection = */              gdb_server_verify_connection,
        /* .set_boot_loader_code = */           gdb_server_set_boot_loader_code,
        /* .get_boot_loader_code = */           gdb_server_get_boot_loader_code,
        /* .cmd_read = */                       gdb_server_cmd_read,
        /* .cmd_write = */                      gdb_server_cmd_write,
        /* .cmd_get_qspi_state */               gdb_server_cmd_get_qspi_state,
        /* .cmd_erase_qspi = */                 gdb_server_cmd_erase_qspi,
        /* .cmd_chip_erase_qspi = */            gdb_server_cmd_chip_erase_qspi,
        /* .cmd_read_qspi = */                  gdb_server_cmd_read_qspi,
        /* .cmd_is_empty_qspi = */              gdb_server_cmd_is_empty_qspi,
        /* .cmd_read_partition_table = */       gdb_server_cmd_read_partition_table,
        /* .cmd_read_partition = */             gdb_server_cmd_read_partition,
        /* .cmd_write_partition = */            gdb_server_cmd_write_partition,
        /* .cmd_copy_to_qspi = */               gdb_server_cmd_copy_to_qspi,
        /* .cmd_direct_write_to_qspi = */       gdb_server_cmd_direct_write_to_qspi,
        /* .cmd_read_otp = */                   gdb_server_cmd_read_otp,
        /* .cmd_write_otp = */                  gdb_server_cmd_write_otp,
        /* .cmd_run = */                        gdb_server_cmd_run,
        /* .cmd_boot = */                       gdb_server_cmd_boot,
        /* .cmd_upload_bootloader = */          gdb_server_cmd_upload_bootloader,
        /* .cmd_mass_erase_eflash = */          gdb_server_cmd_mass_erase_eflash,
        /* .cmd_get_oqspi_state */              gdb_server_cmd_get_oqspi_state,
        /* .cmd_erase_oqspi = */                gdb_server_cmd_erase_oqspi,
        /* .cmd_chip_erase_oqspi = */           gdb_server_cmd_chip_erase_oqspi,
        /* .cmd_read_oqspi = */                 gdb_server_cmd_read_oqspi,
        /* .cmd_is_empty_oqspi = */             gdb_server_cmd_is_empty_oqspi,
        /* .cmd_copy_to_oqspi = */              gdb_server_cmd_copy_to_oqspi,
        /* .cmd_direct_write_to_oqspi = */      gdb_server_cmd_direct_write_to_oqspi,
        /* .cmd_get_product_info = */           gdb_server_cmd_get_product_info,
        /* .read_chunk_size = */                GDB_SERVER_READ_CHUNK_SIZE,
        /* .write_chunk_size = */               GDB_SERVER_WRITE_CHUNK_SIZE,
};

/* Selected target interface is now serial port protocol */
static const target_interface_t *target = &target_serial;

void prog_set_initial_baudrate(unsigned int initial_baudrate)
{
        prog_intial_baudrate = initial_baudrate;
}

unsigned int prog_get_initial_baudrate(void)
{
        return prog_intial_baudrate;
}

int prog_serial_open(const char *port, int baudrate)
{
        int ret;

        ret = serial_open(port, baudrate);
        if (!ret) {
                return ERR_FILE_OPEN;
        }
        target = &target_serial;

        return 0;
}

void prog_serial_close(int data)
{
        (void) data;

        serial_close();
}

void prog_gdb_close(int pid)
{
        gdb_server_close(pid);
}

int prog_set_uart_boot_loader(uint8_t *buf, size_t size)
{
        boot_loader = realloc(boot_loader, size);

        if (boot_loader == NULL) {
                return ERR_ALLOC_FAILED;
        }

        memcpy(boot_loader, buf, size);
        target->set_boot_loader_code(boot_loader, size);

        return 0;
}

int prog_set_uart_boot_loader_from_file(const char *file_name)
{
        FILE *f;
        struct stat st;
        int err = 0;

        if (file_name == NULL || stat(file_name, &st) < 0) {
                return ERR_FILE_OPEN;
        }

        f = fopen(file_name, "rb");
        if (f == NULL) {
                return ERR_FILE_OPEN;
        }
        if(boot_loader) {
                boot_loader = realloc(boot_loader, st.st_size);
        } else {
                boot_loader = malloc(st.st_size);
        }

        if (boot_loader == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        if (st.st_size != fread(boot_loader, 1, st.st_size, f)) {
                err = ERR_FILE_READ;
                goto end;
        }

        target->set_boot_loader_code(boot_loader, st.st_size);
end:
        if (f != NULL) {
                fclose(f);
        }

        return err;
}

static void print_log(char buf_msg[], const char *msg, va_list *args)
{
        if (gdb_gui_mode) {
                vsprintf(buf_msg, msg, *args);
        } else {
                FILE *to = (buf_msg == stderr_msg) ? stderr : stdout;

                vfprintf(to, msg, *args);
                fflush(to);
        }
}

void prog_print_log(const char *msg, ...)
{
        va_list args;
        va_start(args, msg);

        print_log(stdout_msg, msg, &args);
        va_end(args);
}

void prog_print_err(const char *msg, ...)
{
        va_list args;
        va_start(args, msg);

        print_log(stderr_msg, msg, &args);
        va_end(args);
}

int prog_write_to_ram(uint32_t ram_address, const uint8_t *buf, uint32_t size)
{
        const uint8_t MAX_RETRY_COUNT = 10;
        int err = 0;
        uint32_t offset = 0;
        uint8_t retry_cnt = 0;

        while (offset < size) {
                uint32_t chunk_size = size - offset;

                if (retry_cnt > MAX_RETRY_COUNT) {
                        prog_print_err(stderr_msg, "Write to RAM failed. Abort.\r\n");
                        return err;
                }

                if (chunk_size > target->write_chunk_size) {
                        chunk_size = target->write_chunk_size;
                }

                prog_print_log("Writing to address: 0x%08x offset: 0x%08x chunk size: 0x%08x\n",
                                                                ram_address, offset, chunk_size);

                err = target->cmd_write(buf + offset, chunk_size, ram_address + offset);
                if (err != 0) {
                        prog_print_log("Writing to RAM address 0x%x failed (%d). Retrying ...\n",
                                                                        ram_address + offset, err);
                        retry_cnt++;
                        continue;
                }

                retry_cnt = 0;
                offset += chunk_size;
        }

        return err;
}

int prog_write_file_to_ram(uint32_t ram_address, const char *file_name, uint32_t size)
{
        int err = 0;
        uint8_t *buf = NULL;
        FILE *f = NULL;

        f = fopen(file_name, "rb");
        if (f == NULL) {
                err = ERR_FILE_OPEN;
                goto end;
        }

        buf = (uint8_t *) malloc(size);
        if (buf == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        if (fread(buf, 1, size, f) != size) {
                err = ERR_FILE_READ;
                goto end;
        }
        err = prog_write_to_ram(ram_address, buf, size);
end:
        if (f) {
                fclose(f);
        }
        if (buf != NULL) {
                free(buf);
        }
        return err;
}

int prog_write_to_qspi(uint32_t flash_address, const uint8_t *buf, uint32_t size)
{
        int err = 0;
        uint32_t offset = 0;
        uint8_t retry_cnt = 0;

        while (offset < size) {
                uint32_t chunk_size = size - offset;

                if (retry_cnt > 10) {
                        err = ERR_PROG_QSPI_WRITE;
                        prog_print_err("Write to qspi failed. Abort. \n");
                        goto done;
                }

                if (chunk_size > target->write_chunk_size) {
                        chunk_size = target->write_chunk_size;
                }
                /*
                 * Modify chunk size if write would not start at the beginning of sector.
                 */
                if (((flash_address + offset) & FLASH_ERASE_MASK) + chunk_size >
                                                                        target->write_chunk_size) {
                        chunk_size = target->write_chunk_size - ((flash_address + offset) &
                                                                                FLASH_ERASE_MASK);
                }

                prog_print_log("Writing to address: 0x%08x offset: 0x%08x chunk size: 0x%08x\n",
                                                                flash_address, offset, chunk_size);

                err = target->cmd_direct_write_to_qspi(buf + offset, chunk_size,
                                                                (flash_address + offset), true);
                if (err != 0) {
                        prog_print_log("Verify writing to qspi address 0x%x failed. Retrying ...\n",
                                                                        flash_address + offset);
                        retry_cnt++;
                        continue;
                }
                retry_cnt = 0;
                offset += chunk_size;
        }

done:
        return err;
}

int prog_write_file_to_qspi(uint32_t flash_address, const char *file_name, uint32_t size)
{
        int err = 0;
        uint8_t *buf = NULL;
        FILE *f = NULL;
        bool flash_binary = false;

        f = fopen(file_name, "rb");
        if (f == NULL) {
                err = ERR_FILE_OPEN;
                goto end;
        }

        buf = (uint8_t *) malloc(size);
        if (buf == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        if (fread(buf, 1, size, f) != size) {
                err = ERR_FILE_READ;
                goto end;
        }

        if (flash_address == 0 && !memcmp(buf, "qQ", 2)) {
                memset(buf, 0xFF, 2);
                flash_binary = true;
        }

        err = prog_write_to_qspi(flash_address, buf, size);
end:
        if (f) {
                fclose(f);
        }

        if (buf != NULL) {
                free(buf);
        }

        if (!err && flash_binary) {
                /*
                 * Writing "qQ" on old 0xFF values does not trigger flash erasing in uartboot, so
                 * the rest of bytes in this sector will not be erased.
                 */
                err = prog_write_to_qspi(0, (const uint8_t *) "qQ", 2);
        }

        return err;
}

int prog_erase_qspi(uint32_t flashAddress, uint32_t size)
{
        return target->cmd_erase_qspi(flashAddress, size);
}

int prog_read_memory(uint32_t mem_address, uint8_t *buf, uint32_t size)
{
        const uint8_t MAX_RETRY_COUNT = 10;
        int err = 0;
        uint32_t offset = 0;
        uint8_t retry_cnt = 0;

        while (offset < size) {
                uint32_t chunk_size = size - offset;

                if (retry_cnt > MAX_RETRY_COUNT) {
                        prog_print_err(stderr_msg, "Reading from RAM failed. Abort.\r\n");
                        return err;
                }

                if (chunk_size > target->read_chunk_size) {
                        chunk_size = target->read_chunk_size;
                }

                prog_print_log("Reading from address: 0x%08x offset: 0x%08x chunk size: 0x%08x\n",
                                                                mem_address, offset, chunk_size);

                err = target->cmd_read(buf + offset, chunk_size, mem_address + offset);
                if (err != 0) {
                        prog_print_log("Reading from RAM address 0x%x failed (%d). Retrying ...\n",
                                                                        mem_address + offset, err);
                        retry_cnt++;
                        continue;
                }

                retry_cnt = 0;
                offset += chunk_size;
        }

        return err;
}

int prog_read_memory_to_file(uint32_t mem_address, const char *file_name, uint32_t size)
{
        int err = 0;
        uint8_t *buf = NULL;
        FILE *f = NULL;

        f = fopen(file_name, "wb");
        if (f == NULL) {
                err = ERR_FILE_OPEN;
                goto end;
        }

        buf = (uint8_t*) malloc(size);
        if (buf == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }
        err = prog_read_memory(mem_address, buf, size);
        if (err < 0) {
                goto end;
        }
        fwrite(buf, 1, size, f);
end:
        if (f) {
                fclose(f);
        }
        if (buf != NULL) {
                free(buf);
        }
        return err;
}

int prog_copy_to_qspi(uint32_t mem_address, uint32_t flash_address, uint32_t size)
{
        return target->cmd_copy_to_qspi(mem_address, size, flash_address);
}

int prog_chip_erase_qspi(void)
{
        const char *chip_rev;

        prog_get_chip_rev(&chip_rev);

        if (strncmp(prog_chip_rev, CHIP_REV_700AB, CHIP_REV_STRLEN) == 0) {
                // NOTE: 0x8000000UL --> virtual base address of QSPI controller 1 in DA1470x
                return target->cmd_chip_erase_qspi(0x8000000UL);
        } else {
                // NOTE: 0x00000000UL --> virtual base address of QSPI controller 1 in DA1468x/DA1469x
                return target->cmd_chip_erase_qspi(0x00000000UL);
        }
}

int prog_chip_erase_qspi_by_addr(uint32_t flashAddress)
{
        return target->cmd_chip_erase_qspi(flashAddress);
}

int prog_write_file_to_otp(uint32_t otp_address, const char *file_name, uint32_t size)
{
        int err = 0;
        uint8_t *buf = NULL;
        FILE *f = NULL;
        uint32_t write_size;
        int i;

        if (strncmp(prog_chip_rev, CHIP_REV_690AB, CHIP_REV_STRLEN) == 0) {
                if (size > chip_690_regs.otp_size) {
                        return ERR_PROG_INVALID_ARGUMENT;
                }
        } else if (strncmp(prog_chip_rev, CHIP_REV_700AB, CHIP_REV_STRLEN) == 0) {
                if (size > chip_700_regs.otp_size) {
                        return ERR_PROG_INVALID_ARGUMENT;
                }
        } else {
                if (size > chip_680_regs.otp_size) {
                        return ERR_PROG_INVALID_ARGUMENT;
                }
        }

        f = fopen(file_name, "rb");
        if (!f) {
                return ERR_FILE_OPEN;
        }

        write_size = (size % 4) ? (size + 4 - (size % 4)) : size;

        buf = (uint8_t *) malloc(write_size);
        if (buf == NULL) {
                fclose(f);
                return ERR_ALLOC_FAILED;
        }

        /* Set last bytes to zero (if exists) - this is faster than memset on large block of memory */
        for (i = size; i < write_size; i++) {
                buf[i] = 0;
        }

        if (fread(buf, 1, size, f) != size) {
                fclose(f);
                free(buf);
                return ERR_FILE_READ;
        }

        fclose(f);

        /* Length of data is a number of 32-bits words, so it must be divided by 4 */
        err = prog_write_otp(otp_address, (const uint32_t *) buf, (write_size >> 2));

        free(buf);

        return err;
}

int write_otp_64(uint32_t address, const uint32_t *buf, uint32_t len)
{

        unsigned int i;
        int err = 0;
        uint32_t *read_buf = NULL;
        bool otp_same = true;
        bool otp_addr_not_empty = false;

        uint32_t length = len;

        if (len%2) {
                /*Not 64 bit aligned*/
                read_buf = (uint32_t*) malloc((len + 1) * sizeof(uint32_t));
                length += 1;
        }
        else {
                read_buf = (uint32_t*) malloc(len * sizeof(uint32_t));
        }

        if (read_buf == NULL) {
                 return ERR_ALLOC_FAILED;
        }

        if ((err = target->cmd_read_otp(address, read_buf, length)) != 0 ) {
                free(read_buf);
                return  err;
        }

        for (i = 0; i < len -1 ; i += 2) {
                if (block_otp_write && (buf[i] == 0) && (buf[i + 1]) == 0) {
                        continue;
                } else  {
                        if (buf[i] != read_buf[i] ||
                                buf[i + 1] != read_buf[i + 1]) {
                                otp_same = false;

                                if (read_buf[i] || read_buf[i + 1]) {
                                        otp_addr_not_empty = true;
                                        break;
                                }
                        }
                        else {
                                break;
                        }
                }
        }

        if (i == len - 1) { /* len is odd no 64 bits alignment */
                /*buf[len -1] must be compared against the programmed one*/
                if (read_buf[len - 1] != buf[len -1]) {
                        otp_same = false;
                }

                if (read_buf[len-1] || read_buf[len]) {
                        /* when we try to write without be 64 bits aligned we must check
                         * the next 32 bit from the end if are empty too*/
                        otp_addr_not_empty = true;
                }
        }

        if (otp_same) {
                prog_print_err("Otp address 0x%x has same data...\n",
                        (chip_680_regs.otp_start_address + (address * 8 + i * 4)));
                free(read_buf);
                return ERR_PROG_OTP_SAME;

        }

        if (otp_addr_not_empty) {
                prog_print_err("Otp address 0x%x not empty...\n",
                        (chip_680_regs.otp_start_address + (address * 8 + i * 4)));
                free(read_buf);
                return  ERR_PROG_OTP_NOT_EMPTY;
        }



        /*Write to OTP address*/
        if ((err = target->cmd_write_otp(address, buf, len)) != 0 ) {
                free(read_buf);
                return  err;
        }

        /*Read back and verify*/
        if ((err = target->cmd_read_otp(address, read_buf, len)) != 0 ) {
                free(read_buf);
                return  err;
        }

        if (block_otp_write) {
                /* Now make 64 bit entries in read_buf zero, where buf also contains zero's */
                for (i = 0; i < len -1; i += 2) {
                        if ((buf[i] == 0) && (buf[i + 1]) == 0) {// corresponding places in buf not zero
                                read_buf[i] = 0;
                                read_buf[i + 1] = 0;
                        }

                        if ((i == len - 1)) { /* check the last not aligned 32 bit part */
                                if (buf[len-1] == 0) {
                                        read_buf[len-1] = 0;
                                }
                        }

                }
        }

        err = memcmp(read_buf, buf, len * sizeof(*buf));

        if (err != 0) {
                err = ERR_PROG_OTP_VERIFY;
                prog_print_err("Verify writing to otp address 0x%x failed ...\n",
                        (chip_680_regs.otp_start_address + (address * 8 + i * 4)));
                free(read_buf);
                return  err;
        }
        return err;
}

int write_otp_32(uint32_t address, const uint32_t *buf, uint32_t len)
{
        unsigned int i;
        int err = 0;
        uint32_t *read_buf = NULL;
        bool otp_same = true;
        bool otp_addr_not_empty = false;
        uint32_t otp_start_address;

        if (strncmp(prog_chip_rev, CHIP_REV_690AB, CHIP_REV_STRLEN) == 0) {
                otp_start_address = chip_690_regs.otp_start_address;
        } else if (strncmp(prog_chip_rev, CHIP_REV_700AB, CHIP_REV_STRLEN) == 0) {
                otp_start_address = chip_700_regs.otp_start_address;
        } else {
                return ERR_PROG_UNKNOW_CHIP;
        }

        read_buf = (uint32_t*) malloc(len * sizeof(uint32_t));


        if (read_buf == NULL) {
              return ERR_ALLOC_FAILED;
        }

        if ((err = target->cmd_read_otp(address, read_buf, len)) != 0 ) {
                free(read_buf);
                return  err;
        }


        for (i = 0; i < len; i++) {
                if (block_otp_write && (buf[i] == 0xFFFFFFFF)) {
                        continue;
                }
                else {
                        if (buf[i] != read_buf[i]) {
                                otp_same = false;
                                if ( read_buf[i] != 0xFFFFFFFF) {
                                        otp_addr_not_empty = true;
                                        break;
                                }

                        } else {
                                break;
                        }
                }
        }


        if (otp_same) {
                prog_print_err("Otp address 0x%x has same data...\n",
                        (otp_start_address + (address * 4 + i * 4)));
                free(read_buf);
                return ERR_PROG_OTP_SAME;

        }

        if (otp_addr_not_empty) {
                prog_print_err("Otp address 0x%x not empty...\n",
                        (otp_start_address + (address * 4 + i * 4)));
                free(read_buf);
                return  ERR_PROG_OTP_NOT_EMPTY;
        }

        /*Write to OTP address*/
        if ((err = target->cmd_write_otp(address, buf, len)) != 0 ) {
                free(read_buf);
                return  err;
        }

        /*Read back and verify*/
        if ((err = target->cmd_read_otp(address, read_buf, len)) != 0 ) {
                free(read_buf);
                return  err;
        }

        if (block_otp_write) {
                /* Now make 64 bit entries in read_buf zero, where buf also contains zero's */
                for (i = 0; i < len; i++) {
                        if (buf[i] == 0xFFFFFFFF) {// corresponding places in buf not zero
                                read_buf[i] = 0xFFFFFFFF;
                        }
                }
        }


        err = memcmp(read_buf, buf, len * sizeof(*buf));

        if (err != 0) {
                err = ERR_PROG_OTP_VERIFY;
                prog_print_err("Verify writing to otp address 0x%x failed ...\n",
                        (otp_start_address + (address * 4 + i * 4)));
                free(read_buf);
                return  err;
        }

        return err;
}

int prog_write_otp(uint32_t address, const uint32_t *buf, uint32_t len)
{
        if ((strncmp(prog_chip_rev, CHIP_REV_690AB, CHIP_REV_STRLEN) == 0) ||
            (strncmp(prog_chip_rev, CHIP_REV_700AB, CHIP_REV_STRLEN) == 0)) {
                return write_otp_32(address, buf, len);
        } else {
                return write_otp_64(address, buf, len);
        }
}

int prog_read_otp(uint32_t address, uint32_t *buf, uint32_t len)
{
        return target->cmd_read_otp(address, buf, len);
}

int prog_write_tcs(uint32_t *address, const uint32_t *buf, uint32_t len)
{
        int err = 0;
        unsigned int i;
        uint32_t *read_buf;

        *address = 0;   //addr of 64bit words
        /*Find empty section in OTP*/
        //Allocate buffer
        read_buf = calloc(TCS_WORD_SIZE, sizeof(*read_buf));
        if (read_buf == NULL) {
                err = ERR_ALLOC_FAILED;
                goto done;
        }
        //Read TCS
        err = target->cmd_read_otp(TCS_ADDR, read_buf, TCS_WORD_SIZE);
        if (err != 0) {
                err = ERR_PROG_OTP_READ;
                prog_print_err("Read from OTP failed...\n");
                goto done;
        }

        //Check for empty TCS section
        while (*address <= ( TCS_WORD_SIZE - len) >> 1) {
                i = 0;
                while (i < len >> 1 && !((uint64_t*) read_buf)[*address + i]) {
                        i++;
                }
                if (i < len >> 1) {
                        *address += i + 1;
                } else {
                        break;
                }
        }

        if (*address > ( TCS_WORD_SIZE - len) >> 1) {
                err = ERR_PROG_OTP_NOT_EMPTY;
                prog_print_err("not enough empty space in TCS\n");
                goto done;
        }
        *address += TCS_ADDR;

        /*Write OTP*/
        err = target->cmd_write_otp(*address, buf, len);
done:
        if (read_buf) {
                free(read_buf);
        }
        return err;
}

int prog_read_qspi(uint32_t address, uint8_t *buf, uint32_t len)
{
        uint32_t offset = 0;
        int err = 0;

        while (err == 0 && offset < len) {
                uint32_t chunk_size = len - offset;
                if (chunk_size > target->read_chunk_size) {
                        chunk_size = target->read_chunk_size;
                }
                err = target->cmd_read_qspi(address + offset, buf + offset, chunk_size);
                offset += chunk_size;
        }
        return err;
}

int prog_read_qspi_to_file(uint32_t address, const char *fname, uint32_t len)
{
        int err = 0;
        uint8_t *buf = NULL;
        FILE *f = NULL;

        f = fopen(fname, "wb");
        if (f == NULL) {
                err = ERR_FILE_OPEN;
                goto end;
        }

        buf = (uint8_t*) malloc(len);
        if (buf == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        err = prog_read_qspi(address, buf, len);
        if (err < 0) {
                goto end;
        }

        fwrite(buf, 1, len, f);

end:
        if (f) {
                fclose(f);
        }

        free(buf);

        return err;
}

int prog_is_empty_qspi(unsigned int size, unsigned int start_address, int *ret_number)
{
        return target->cmd_is_empty_qspi(size, start_address, ret_number);
}

int prog_read_partition_table(uint8_t **buf, uint32_t *len)
{
        return target->cmd_read_partition_table(buf, len);
}

int prog_read_partition(nvms_partition_id_t id, uint32_t address, uint8_t *buf, uint32_t len)
{
        uint32_t offset = 0;
        int err = 0;

        while (err == 0 && offset < len) {
                uint32_t chunk_size = len - offset;
                if (chunk_size > target->read_chunk_size) {
                        chunk_size = target->read_chunk_size;
                }
                err = target->cmd_read_partition(id, address + offset, buf + offset, chunk_size);
                offset += chunk_size;
        }

        return err;
}

int prog_read_patrition_to_file(nvms_partition_id_t id, uint32_t address, const char *fname,
                                                                                uint32_t len)
{
        int err = 0;
        uint8_t *buf = NULL;
        FILE *f = NULL;

        f = fopen(fname, "wb");
        if (f == NULL) {
                err = ERR_FILE_OPEN;
                goto end;
        }

        buf = (uint8_t*) malloc(len);
        if (buf == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        err = prog_read_partition(id, address, buf, len);
        if (err < 0) {
                goto end;
        }

        if (fwrite(buf, 1, len, f) != len) {
                err = ERR_FILE_WRITE;
        }

end:
        if (f) {
                fclose(f);
        }

        free(buf);

        return err;
}

int prog_write_partition(nvms_partition_id_t id, uint32_t part_address, const uint8_t *buf,
                                                                                uint32_t size)
{
        int err = 0;
        uint32_t offset = 0;
        uint8_t retry_cnt = 0;
        /*
         * Reading and writing is used in this function - chunk size must be adjusted to the smaller
         * value.
         */
        uint32_t max_chunk_size = target->write_chunk_size < target->read_chunk_size ?
                                                target->write_chunk_size : target->read_chunk_size;

        while (offset < size) {
                uint32_t chunk_size = size - offset;
                uint8_t read_buf[max_chunk_size];

                if (retry_cnt > 10) {
                        err = ERR_PROG_QSPI_WRITE;
                        prog_print_err("Write to partition failed. Abort.\n");
                        break;
                }

                if (chunk_size > max_chunk_size) {
                        chunk_size = max_chunk_size;
                }
                /*
                 * Modify chunk size if write would not start at the beginning of sector.
                 */
                if (((part_address + offset) & FLASH_ERASE_MASK) + chunk_size >
                                                                        target->write_chunk_size) {
                        chunk_size = target->write_chunk_size - ((part_address + offset) &
                                                                                FLASH_ERASE_MASK);
                }

                err = target->cmd_write(buf + offset, chunk_size, ADDRESS_TMP);
                if (err != 0) {
                        break;
                }

                prog_print_log("Writing to address: 0x%08x offset: 0x%08x "
                                        "chunk size: 0x%08x\n", part_address, offset, chunk_size);

                err = target->cmd_write_partition(id, part_address + offset, ADDRESS_TMP,
                                                                                chunk_size);
                if (err != 0) {
                        break;
                }

                /*Verify write*/
                err = prog_read_partition(id, part_address + offset, read_buf, chunk_size);
                if (err != 0) {
                        break;
                }

                err = memcmp(read_buf, buf + offset, chunk_size);
                if (err != 0) {
                        prog_print_log("Verify writing to qspi address 0x%x failed."
                                                        "Retrying ...\n", part_address + offset);
                        retry_cnt++;
                        continue;
                }
                retry_cnt = 0;
                offset += chunk_size;
        }

        return err;
}

int prog_write_file_to_partition(nvms_partition_id_t id, uint32_t part_address,
                                                        const char *file_name, uint32_t size)
{
        int err = 0;
        uint8_t *buf = NULL;
        FILE *f = NULL;

        f = fopen(file_name, "rb");
        if (f == NULL) {
                err = ERR_FILE_OPEN;
                goto end;
        }

        buf = (uint8_t *) malloc(size);
        if (buf == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        if (fread(buf, 1, size, f) != size) {
                err = ERR_FILE_READ;
                goto end;
        }

        err = prog_write_partition(id, part_address, buf, size);
end:
        if (f) {
                fclose(f);
        }

        if (buf != NULL) {
                free(buf);
        }

        return err;
}

int prog_boot(uint8_t *executable_code, size_t executable_code_size)
{
        const char *chip_rev;
        prog_get_chip_rev(&chip_rev);

        /* Max bootloader size which can be sent to RAM (at 0 address) */
        uint32_t max_bootloader_size;

        if ( strncmp(prog_chip_rev, CHIP_REV_690AB, CHIP_REV_STRLEN) == 0 ) {
                max_bootloader_size = 0x1FFFF;
        } else if ( strncmp(prog_chip_rev, CHIP_REV_700AB, CHIP_REV_STRLEN) == 0 ) {
                max_bootloader_size = 0x120000;
        } else {
                max_bootloader_size = 0x10000;
        }

        if (executable_code_size == 0) {
                return ERR_FILE_EMPTY;
        }

        if (target->cmd_boot == protocol_cmd_boot) {
                if (executable_code_size > max_bootloader_size) {
                        prog_print_log("Too big image file.\n");
                        return ERR_FILE_TOO_BIG;
                }
        }

        return target->cmd_boot(executable_code, executable_code_size);
}

int prog_run(uint8_t *executable_code, size_t executable_code_size)
{
        int err;

        if (executable_code_size == 0) {
                return ERR_FILE_EMPTY;
        }

        prog_print_log("Sending executable to device...\n");

        err = prog_write_to_ram(VIRTUAL_BUF_ADDRESS, executable_code, executable_code_size);
        if (err < 0) {
               return err;
        }

        prog_print_log("Starting executable...\n");

        return target->cmd_run(VIRTUAL_BUF_ADDRESS);
}

int prog_upload_bootloader(void)
{
        return target->cmd_upload_bootloader();
}

int prog_mass_erase_eflash(void)
{
        return ERR_CMD_UNSUPPORTED;
}

int prog_write_to_oqspi(uint32_t flash_address, const uint8_t *buf, uint32_t size)
{
        int err = 0;
        uint32_t offset = 0;
        uint8_t retry_cnt = 0;

        while (offset < size) {
                uint32_t chunk_size = size - offset;

                if (retry_cnt > 10) {
                        err = ERR_PROG_OQSPI_WRITE;
                        prog_print_err("Write to oqspi failed. Abort. \n");
                        goto done;
                }

                if (chunk_size > target->write_chunk_size) {
                        chunk_size = target->write_chunk_size;
                }
                /*
                 * Modify chunk size if write would not start at the beginning of sector.
                 */
                if (((flash_address + offset) & FLASH_ERASE_MASK) + chunk_size >
                                                                        target->write_chunk_size) {
                        chunk_size = target->write_chunk_size - ((flash_address + offset) &
                                                                                FLASH_ERASE_MASK);
                }

                prog_print_log("Writing to address: 0x%08x offset: 0x%08x chunk size: 0x%08x\n",
                                                                flash_address, offset, chunk_size);

                err = target->cmd_direct_write_to_oqspi(buf + offset, chunk_size,
                                                                (flash_address + offset), true);
                if (err != 0) {
                        prog_print_log("Verify writing to oqspi address 0x%x failed. Retrying ...\n",
                                                                        flash_address + offset);
                        retry_cnt++;
                        continue;
                }
                retry_cnt = 0;
                offset += chunk_size;
        }

done:
        return err;
}

int prog_write_file_to_oqspi(uint32_t flash_address, const char *file_name, uint32_t size)
{
        int err = 0;
        uint8_t *buf = NULL;
        FILE *f = NULL;
        bool flash_binary = false;

        f = fopen(file_name, "rb");
        if (f == NULL) {
                err = ERR_FILE_OPEN;
                goto end;
        }

        buf = (uint8_t *) malloc(size);
        if (buf == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        if (fread(buf, 1, size, f) != size) {
                err = ERR_FILE_READ;
                goto end;
        }

        if (flash_address == 0 && !memcmp(buf, "qQ", 2)) {
                memset(buf, 0xFF, 2);
                flash_binary = true;
        }

        err = prog_write_to_oqspi(flash_address, buf, size);
end:
        if (f) {
                fclose(f);
        }

        if (buf != NULL) {
                free(buf);
        }

        if (!err && flash_binary) {
                /*
                 * Writing "qQ" on old 0xFF values does not trigger flash erasing in uartboot, so
                 * the rest of bytes in this sector will not be erased.
                 */
                err = prog_write_to_oqspi(0, (const uint8_t *) "qQ", 2);
        }

        return err;
}

int prog_erase_oqspi(uint32_t flashAddress, uint32_t size)
{
        return target->cmd_erase_oqspi(flashAddress, size);
}

int prog_is_empty_oqspi(unsigned int size, unsigned int start_address, int *ret_number)
{
        return target->cmd_is_empty_oqspi(size, start_address, ret_number);
}

int prog_copy_to_oqspi(uint32_t mem_address, uint32_t flash_address, uint32_t size)
{
        return target->cmd_copy_to_oqspi(mem_address, size, flash_address);
}

int prog_chip_erase_oqspi(void)
{
        const char *chip_rev;

        prog_get_chip_rev(&chip_rev);

        if (strncmp(prog_chip_rev, CHIP_REV_700AB, CHIP_REV_STRLEN) == 0) {
                // NOTE: 0 --> virtual base address of OQSPI controller in DA1470x
                return target->cmd_chip_erase_oqspi(0x00000000UL);
        } else {
                return ERR_CMD_UNSUPPORTED;
        }
}

int prog_chip_erase_oqspi_by_addr(uint32_t flashAddress)
{
        return target->cmd_chip_erase_oqspi(flashAddress);
}

int prog_read_oqspi(uint32_t address, uint8_t *buf, uint32_t len)
{
        uint32_t offset = 0;
        int err = 0;

        while (err == 0 && offset < len) {
                uint32_t chunk_size = len - offset;
                if (chunk_size > target->read_chunk_size) {
                        chunk_size = target->read_chunk_size;
                }
                err = target->cmd_read_oqspi(address + offset, buf + offset, chunk_size);
                offset += chunk_size;
        }
        return err;
}

int prog_read_oqspi_to_file(uint32_t address, const char *fname, uint32_t len)
{
        int err = 0;
        uint8_t *buf = NULL;
        FILE *f = NULL;

        f = fopen(fname, "wb");
        if (f == NULL) {
                err = ERR_FILE_OPEN;
                goto end;
        }

        buf = (uint8_t*) malloc(len);
        if (buf == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        err = prog_read_oqspi(address, buf, len);
        if (err < 0) {
                goto end;
        }

        fwrite(buf, 1, len, f);

end:
        if (f) {
                fclose(f);
        }

        free(buf);

        return err;
}

const char* prog_get_err_message(int err_int)
{
        switch (err_int) {
        case ERR_FAILED:
                return "general error";
        case ERR_ALLOC_FAILED:
                return "memory allocation failed";
        case ERR_FILE_OPEN:
                return "file cannot be opened";
        case ERR_FILE_READ:
                return "file cannot be read";
        case ERR_FILE_PATCH:
                return "secondary boot loader cannot be patched";
        case ERR_FILE_WRITE:
                return "file cannot be written";
        case ERR_FILE_CLOSE:
                return "file cannot be closed";
        case ERR_FILE_TOO_BIG:
                return "file is too big";
        case ERR_FILE_EMPTY:
                return "file is empty";
        case ERR_PROT_NO_RESPONSE:
                return "timeout waiting for response";
        case ERR_PROT_CMD_REJECTED:
                return "NAK received when waiting for ACK";
        case ERR_PROT_INVALID_RESPONSE:
                return "invalid data received when waiting for ACK";
        case ERR_PROT_CRC_MISMATCH:
                return "CRC16 mismatch";
        case ERR_PROT_CHECKSUM_MISMATCH:
                return "checksum mismatch while uploading 2nd stage bootloader";
        case ERR_PROT_BOOT_LOADER_REJECTED:
                return "2nd stage bootloader rejected";
        case ERR_PROT_UNKNOWN_RESPONSE:
                return "invalid announcement message received";
        case ERR_PROT_TRANSMISSION_ERROR:
                return "failed to transmit data";
        case ERR_PROT_COMMAND_ERROR:
                return "error executing command";
        case ERR_PROT_UNSUPPORTED_VERSION:
                return "unsupported bootloader version";
        case ERR_GDB_SERVER_SOCKET:
                return "communication with GDB Server socket failed";
        case ERR_GDB_SERVER_CRC_MISMATCH:
                return "checksum mismatch";
        case ERR_GDB_SERVER_CMD_REJECTED:
                return "NAK received when waiting for ACK";
        case ERR_GDB_SERVER_INVALID_RESPONSE:
                return "invalid data received from GDB Server";
        case ERR_GDB_SERVER_OUT_OF_MEMORY:
                return "could not allocate memory to open GDB erver";
        case ERR_PROG_OTP_SAME:
                return "Data written to OTP match data to be written";
        case ERR_PROG_QSPI_IMAGE_FORMAT:
                return "invalid image format";
        case ERR_PROG_UNKNOW_CHIP:
                return "can't read chip revision";
        case ERR_PROG_NO_PARTITON:
                return "required partition not found";
        case ERR_PROG_UNKNOWN_PRODUCT_ID:
                return "Unknown product id";
        case ERR_PROG_INSUFICIENT_BUFFER:
                return "Insufficient memory buffer";
        case ERR_PROG_INVALID_ARGUMENT:
                return "Invalid argument";
        case ERR_CMD_UNSUPPORTED:
                return "Command unsupported by target";

        case MSG_FROM_STDOUT:
                strcpy(copy_stdout_msg, stdout_msg);
                stdout_msg[0] = 0;
                return (const char*) copy_stdout_msg;
        case MSG_FROM_STDERR:
                strcpy(copy_stderr_msg, stderr_msg);
                stderr_msg[0] = 0;
                return (const char*) copy_stderr_msg;

        default:
                return "unknown error";

        }

}

static void prog_uartboot_patch_write_value(uint8_t * code, int offset, unsigned int value)
{
        uint8_t * pos;
        int i;
        pos = code + offset;
        /* Write value ensuring little-endianess. */
        for (i = 0; i < 4; i++) {
                *pos++ = (uint8_t) value & 0xff;
                value >>= 8;
        }
}

int prog_uartboot_patch_config(const prog_uartboot_config_t * uartboot_config)
{
        uint8_t * code;
        size_t size;
        get_boot_loader_code(&code, &size);
        if (code == NULL) {
                return ERR_FILE_PATCH;
        }
        /* Check size. */
        if (size < PROGRAMMER_PATCH_OFFSET_MAX + 4) {
                return ERR_FILE_PATCH;
        }
        if (uartboot_config->baudrate_patch) {
                prog_uartboot_patch_write_value(code, PROGRAMMER_PATCH_OFFSET_BAUDRATE,
                        uartboot_config->baudrate);
        }
        if (uartboot_config->tx_port_patch) {
                prog_uartboot_patch_write_value(code, PROGRAMMER_PATCH_OFFSET_TX_PORT,
                        uartboot_config->tx_port);
        }
        if (uartboot_config->tx_pin_patch) {
                prog_uartboot_patch_write_value(code, PROGRAMMER_PATCH_OFFSET_TX_PIN,
                        uartboot_config->tx_pin);
        }
        if (uartboot_config->rx_port_patch) {
                prog_uartboot_patch_write_value(code, PROGRAMMER_PATCH_OFFSET_RX_PORT,
                        uartboot_config->rx_port);
        }
        if (uartboot_config->rx_pin_patch) {
                prog_uartboot_patch_write_value(code, PROGRAMMER_PATCH_OFFSET_RX_PIN,
                        uartboot_config->rx_pin);
        }
        return 0;
}

void prog_close_interface(int data)
{
        if (target) {
                target->close(data);
        }
}

int DLLEXPORT prog_gdb_open(const prog_gdb_server_config_t *gdb_server_conf)
{
        target = &target_gdb_server;

        return gdb_server_initialization(gdb_server_conf);
}

int DLLEXPORT prog_gdb_mode(int mode)
{
        int rw = 0;
        if (mode & 0xfffffff8) { // no valid bits addressed
                rw = ERR_FAILED;
        } else {
                if (mode & GDB_MODE_GUI) {
                        gdb_gui_mode = true;
                        stdout_msg[0] = 0;      // empty last known stdout msg buffer
                        stderr_msg[0] = 0;      // empty last known stderr msg buffer
                        if (mode & GDB_MODE_INVALIDATE_STUB) {  // Invalidate downloaded stub
                                gdb_invalidate_stub();
                        }
                } else {
                        gdb_gui_mode = false;
                }
                if (mode & GDB_MODE_BLOCK_WRITE_OTP) {
                        block_otp_write = true;
                } else {
                        block_otp_write = false;
                }
        }
        return rw;
}

void prog_set_uart_timeout(unsigned int timeoutInMs)
{
        uartTimeoutInMs = timeoutInMs;
}

unsigned int get_uart_timeout(void)
{
        return uartTimeoutInMs;
}

static int fill_chip_id_regs(uint32_t id_regs[5])
{
        const prog_chip_regs_t *chip_regs;
        int err;

        err = prog_get_chip_regs(prog_chip_rev, &chip_regs);
        if (err) {
                return err;
        }

        id_regs[0] = chip_regs->chip_id1_reg;
        id_regs[1] = chip_regs->chip_id2_reg;
        id_regs[2] = chip_regs->chip_id3_reg;
        id_regs[3] = chip_regs->chip_revision_reg;
        id_regs[4] = chip_regs->chip_test1_reg;

        return 0;
}

int prog_read_chip_info(chip_info_t *chip_info)
{
        uint32_t id_regs[5];
        uint8_t id[5] = { 0 };
        uint32_t chip_otp_id[OTP_HEADER_CHIP_ID_LEN >> 2] = { 0 };
        uint32_t chip_package[OTP_HEADER_POS_PACK_INFO_LEN >> 2] = { 0 };
        int i;
        int err = 0;

        err = fill_chip_id_regs(id_regs);
        if (err) {
                return err;
        }

        /* Read chip revision */
        for (i = 0; err == 0 && i < sizeof(id); ++i) {
                err = prog_read_memory(id_regs[i], &id[i], 1);
        }
        if (err) {
                return err;
        }

        /* Copy info to output buffer */
        /* Chip revision */
        chip_info->chip_rev[0] = id[0];
        chip_info->chip_rev[1] = id[1];
        chip_info->chip_rev[2] = id[2];
        chip_info->chip_rev[3] = id[3];
        /* Increase revision number to ASCII sign */
        chip_info->chip_rev[4] = id[4] < 'A' ? id[4] + 'A' : id[4];
        chip_info->chip_rev[5] = '\0';

        if (strcmp(chip_info->chip_rev, "252AB") == 0) {
                /* Override values for 69x -- D2522AB */
                strncpy(chip_info->chip_rev, CHIP_REV_690AB, CHIP_REV_STRLEN);
                strncpy(chip_info->chip_otp_id, "00000000", CHIP_OTP_ID_STRLEN);
                strncpy(chip_info->chip_package, "      ", CHIP_PACKAGE_LEN);
                strncpy(chip_info->chip_id, CHIP_ID_D2522AB, CHIP_ID_STRLEN);
        } else if (strcmp(chip_info->chip_rev, "308AA") == 0) {
                /* Override values for 69x -- D3080AA */
                strncpy(chip_info->chip_rev, CHIP_REV_690AB, CHIP_REV_STRLEN);
                strncpy(chip_info->chip_otp_id, "00000000", CHIP_OTP_ID_STRLEN);
                strncpy(chip_info->chip_package, "      ", CHIP_PACKAGE_LEN);
                strncpy(chip_info->chip_id, CHIP_ID_D3080AA, CHIP_ID_STRLEN);
        } else if (strcmp(chip_info->chip_rev, "279AA") == 0 || strcmp(chip_info->chip_rev, "279AB") == 0) {
                uint32_t hw_config_reg_addr = 0x500000B8UL;
                uint8_t  hw_config_reg_val;
                err = prog_read_memory(hw_config_reg_addr, &hw_config_reg_val, sizeof(hw_config_reg_val));
                if (err) {
                        return err;
                }
                if ('A' == chip_info->chip_rev[4]) {
                        strncpy(chip_info->chip_rev, CHIP_REV_700AA, CHIP_REV_STRLEN);
                } else if ('B' == chip_info->chip_rev[4]) {
                        strncpy(chip_info->chip_rev, CHIP_REV_700AB, CHIP_REV_STRLEN);
                }
                if ((hw_config_reg_val & 0x8) == 0x8) {
                        /* JEITA charger disabled */
                        strncpy(chip_info->chip_otp_id, "DA14701", CHIP_OTP_ID_STRLEN);
                } else if ((hw_config_reg_val & 0x1) == 0x1) {
                        /* External PSRAM with data cache disabled */
                        strncpy(chip_info->chip_otp_id, "DA14705", CHIP_OTP_ID_STRLEN);
                } else if ((hw_config_reg_val & 0x6) == 0x6) {
                        /* SDIO/eMMC as well as MIPI DSI disabled */
                        strncpy(chip_info->chip_otp_id, "DA14706", CHIP_OTP_ID_STRLEN);
                } else {
                        /* All features available */
                        strncpy(chip_info->chip_otp_id, "DA14708", CHIP_OTP_ID_STRLEN);
                }
                strncpy(chip_info->chip_package, "      ", CHIP_PACKAGE_LEN);
        } else {
                /* Read chip id as stored in OTP */
                err = target->cmd_read_otp(((OTP_HEADER_CHIP_ID & ~chip_680_regs.otp_start_address) >> 3), chip_otp_id,
                OTP_HEADER_CHIP_ID_LEN >> 2);
                if (err) {
                        return err;
                }

                /* Read package info*/
                err = target->cmd_read_otp(((OTP_HEADER_POS_PACK_INFO & ~chip_680_regs.otp_start_address) >> 3),
                        chip_package, OTP_HEADER_POS_PACK_INFO_LEN >> 2);
                if (err) {
                        return err;
                }



                /* Chip otp id*/
                for (i = 0; i < OTP_HEADER_CHIP_ID_LEN; i++) {
                        chip_info->chip_otp_id[i] = *(((char*) chip_otp_id) + i);
                }
                chip_info->chip_otp_id[i] = '\0';

                /* Chip package info */
                if (*(((char*) chip_package) + 3) == 0x00) {
                        strncpy(chip_info->chip_package, "WLCSP ", CHIP_PACKAGE_LEN);
                }
                else if (*(((char*) chip_package) + 3) == 0x55) {
                        strncpy(chip_info->chip_package, "aQFN60", CHIP_PACKAGE_LEN);
                }
        }

        return err;
}

int prog_read_flash_info(flash_info_t *flash_info)
{
        int err = 0;

        err = target->cmd_get_qspi_state(flash_info->qspic_id, &flash_info->qspi_flash_info);
        if (err) {
                return err;
        }

        err = target->cmd_get_oqspi_state(&flash_info->oqspi_flash_info);

        return err;
}

int prog_get_product_info(uint8_t **buf, uint32_t *len)
{
        return target->cmd_get_product_info(buf, len);
}

int prog_gdb_direct_read(uint32_t mem_address, uint8_t *buf, uint32_t size)
{
        if (target != &target_gdb_server) {
                return ERR_FAILED;
        }

        return gdb_server_cmd_direct_read(buf, size, mem_address);
}

int prog_gdb_read_chip_rev(char *chip_rev)
{
        uint32_t id_regs[5];
        uint8_t id[5] = { 0 };
        int i;
        int err = 0;

        if (target != &target_gdb_server) {
                return ERR_FAILED;
        }

        err = fill_chip_id_regs(id_regs);
        if (err) {
                return err;
        }

        /* Read chip revision */
        for (i = 0; err == 0 && i < sizeof(id); ++i) {
                err = prog_gdb_direct_read(id_regs[i], &id[i], 1);
        }

        if (err) {
                return err;
        }

        /* Copy info to output buffer */
        /* Chip revision */
        chip_rev[0] = id[0];
        chip_rev[1] = id[1];
        chip_rev[2] = id[2];
        chip_rev[3] = id[3];
        /* Increase revision number to ASCII sign */
        chip_rev[4] = id[4] < 'A' ? id[4] + 'A' : id[4];
        chip_rev[5] = '\0';

        if (strcmp(chip_rev, "252AB") == 0) {
                strncpy(chip_rev, CHIP_REV_690AB, CHIP_REV_STRLEN);
        } else if (strcmp(chip_rev, "279AA") == 0) {
                strncpy(chip_rev, CHIP_REV_700AA, CHIP_REV_STRLEN);
        } else if (strcmp(chip_rev, "279AB") == 0) {
                strncpy(chip_rev, CHIP_REV_700AB, CHIP_REV_STRLEN);
        }

        return err;
}

int prog_map_product_id_to_chip_rev(const char *product_id, char *chip_rev)
{
        if ((strcmp(product_id, "DA14681-01") == 0) || (strcmp(product_id, "DA14680-01") == 0)) {
                if (chip_rev) {
                        strncpy(chip_rev, CHIP_REV_680AH, CHIP_REV_STRLEN);
                }
                return 0;
        }
        if ( (strcmp(product_id, "DA14682-00") == 0) || (strcmp(product_id, "DA14683-00") == 0) ||
             (strcmp(product_id, "DA15000-00") == 0) || (strcmp(product_id, "DA15001-00") == 0) ||
             (strcmp(product_id, "DA15100-00") == 0) || (strcmp(product_id, "DA15101-00") == 0) ) {
                if (chip_rev) {
                        strncpy(chip_rev, CHIP_REV_680BB, CHIP_REV_STRLEN);
                }
                return 0;
        }
        if ((strcmp(product_id, "DA1469x-00") == 0)) {
                if (chip_rev) {
                        strncpy(chip_rev, CHIP_REV_690AB, CHIP_REV_STRLEN);
                }
                return 0;
        }
        if ((strcmp(product_id, "DA1470x-00") == 0)) {
                if (chip_rev) {
                        strncpy(chip_rev, CHIP_REV_700AB, CHIP_REV_STRLEN);
                }
                return 0;
        }
        return ERR_PROG_UNKNOWN_PRODUCT_ID;
}

void prog_get_chip_rev(const char **chip_rev)
{
        *chip_rev = prog_chip_rev;
}

int prog_set_chip_rev(const char *chip_rev)
{
        int status;

        status = prog_chip_rev_valid(chip_rev);
        if (status) {
                return status;
        }

        strncpy(prog_chip_rev, chip_rev, CHIP_REV_STRLEN);
        prog_chip_rev[CHIP_REV_STRLEN - 1] = '\0';

        return 0;
}

int prog_chip_rev_valid(const char *chip_rev)
{
        if (!chip_rev) {
                return ERR_PROG_INVALID_ARGUMENT;
        }

        if ((strcmp(chip_rev, CHIP_REV_680AH) == 0) || (strcmp(chip_rev, CHIP_REV_680BB) == 0) ||
                (strcmp(chip_rev, CHIP_REV_690AB) == 0) ||
                (strcmp(chip_rev, CHIP_REV_700AB) == 0)) {
                return 0;
        }

        return ERR_PROG_UNKNOW_CHIP;
}

int prog_get_chip_regs(const char *chip_rev, const prog_chip_regs_t **regs)
{
        if (!chip_rev) {
                return ERR_PROG_INVALID_ARGUMENT;
        }

        if ((strcmp(chip_rev, CHIP_REV_680AH) == 0) || (strcmp(chip_rev, CHIP_REV_680BB) == 0)) {
                *regs = &chip_680_regs;

                return 0;
        }

        if (strcmp(chip_rev, CHIP_REV_690AB) == 0) {
                *regs = &chip_690_regs;

                return 0;
        }

        if (strcmp(chip_rev, CHIP_REV_700AB) == 0) {
                *regs = &chip_700_regs;

                return 0;
        }

        return ERR_PROG_UNKNOW_CHIP;
}

int prog_get_memory_sizes(const char *chip_rev, const prog_memory_sizes_t **sizes)
{
        if (!chip_rev) {
                return ERR_PROG_INVALID_ARGUMENT;
        }

        if ((strcmp(chip_rev, CHIP_REV_680AH) == 0) || (strcmp(chip_rev, CHIP_REV_680BB) == 0)) {
                *sizes = &chip_680_mem_sizes;

                return 0;
        }

        if (strcmp(chip_rev, CHIP_REV_690AB) == 0) {
                *sizes = &chip_690_mem_sizes;

                return 0;
        }

        if (strcmp(chip_rev, CHIP_REV_700AB) == 0) {
                *sizes = &chip_700_mem_sizes;

                return 0;
        }

        return ERR_PROG_UNKNOW_CHIP;
}

enum endianess {
        USE_LITTLE_ENDIANESS,
        USE_BIG_ENDIANESS
};

static inline void store32(void *buf, uint32_t val, enum endianess e)
{
        uint8_t *b = buf;

        if (USE_LITTLE_ENDIANESS == e) {
                b[0] = val & 0xff;
                val >>= 8;
                b[1] = val & 0xff;
                val >>= 8;
                b[2] = val & 0xff;
                val >>= 8;
                b[3] = val & 0xff;
        } else {
                b[3] = val & 0xff;
                val >>= 8;
                b[2] = val & 0xff;
                val >>= 8;
                b[1] = val & 0xff;
                val >>= 8;
                b[0] = val & 0xff;
        }
}

int prog_fill_image_header(uint8_t *buf, int image_size, const char *chip_rev,
                                                        image_type_t type, image_mode_t mode)
{
        struct qspi_image_header image_header = {
                .magic = { 0 },
                .length = { 0 }
        };
        int header_size = -1;

        image_header.magic[2] = 0x00;

        if (IMG_QSPI == type) {
                image_header.magic[0] = 'q';
                image_header.magic[1] = 'Q';
                if (IMG_CACHED == mode) {
                        store32(&image_header.length, 0x80000000UL | (image_size - 8),
                                                                                USE_BIG_ENDIANESS);
                } else {
                        store32(&image_header.length, image_size, USE_BIG_ENDIANESS);
                }
                header_size = IMAGE_HEADER_SIZE;
                memcpy(buf, &image_header, header_size);
        } else if (IMG_QSPI_S == type) {
                image_header.magic[0] = 'p';
                image_header.magic[1] = 'P';
                store32(&image_header.length, image_size, USE_BIG_ENDIANESS);
                header_size = IMAGE_HEADER_SIZE;
                memcpy(buf, &image_header, header_size);
        } else if (IMG_OTP == type) {
                uint32_t len;

                /* convert to 8-byte multiple */
                len = (image_size + 7) & ~7;
                /* convert to counting 32-bit words */
                len >>= 2;

                if (IMG_CACHED == mode) {
                        store32(buf, 0x80000000UL | len, USE_LITTLE_ENDIANESS);
                } else {
                        store32(buf, len, USE_LITTLE_ENDIANESS);
                }
                header_size = IMAGE_HEADER_SIZE >> 1;
        }

        return header_size;
}

int prog_make_image(uint8_t *binary, int binary_size, const char *chip_rev,
                                                image_type_t type, image_mode_t mode,
                                                uint8_t *buf, int buf_size, int *required_size)
{
        int header_size;
        int size;
        uint32_t stack_pointer;
        uint32_t reset_handler;
        uint32_t *int_buf = (uint32_t *) binary;
        /* Size of memory block that is in RAM even when FLASH is mapped at address 0 */
        int ram_at_0_size = 0x200;
        const prog_chip_regs_t *regs;
        int status;
        if (strcmp(chip_rev, CHIP_REV_680AH) == 0) {
                ram_at_0_size = 0x100;
        }
        uint8_t header_buffer[IMAGE_HEADER_SIZE];

        if (NULL == binary || (NULL == buf && buf_size > 0) || buf_size < 0) {
                return ERR_PROG_INVALID_ARGUMENT;
        }

        header_size = prog_fill_image_header(header_buffer, binary_size, chip_rev,
                type, mode);

        if (header_size < 0) {
                return header_size;
        }

        status = prog_get_chip_regs(chip_rev, &regs);
        if (status) {
                return status;
        }

        stack_pointer = int_buf[0];
        reset_handler = int_buf[1];
        /*
         * Sanity check for image
         *
         * Stack pointer has to be in range of (0x100, 0x24000) or (0x7FC0000, 0x7FE4000)
         * and reset handler has to be in range of (0x100, 0x24000) or (0x7F80000, 0xA0000000)
         * Otherwise the image is invalid.
         */
        if (!((stack_pointer > 0x100 && stack_pointer <
                                        (regs->memory_sysram_end - regs->memory_sysram_base)) ||
                                        (stack_pointer > regs->memory_sysram_base &&
                                        stack_pointer < regs->memory_sysram_end))) {
                return ERR_PROG_QSPI_IMAGE_FORMAT;
        }

        if (!((reset_handler > 0x100 && reset_handler <
                                (regs->memory_sysram_end - regs->memory_sysram_base)) ||
                                (reset_handler > regs->memory_sysram_base && reset_handler <
                                (regs->memory_sysram_end - regs->memory_sysram_base)) ||
                                (reset_handler > regs->memory_qspif_base &&
                                reset_handler < regs->memory_qspif_end))) {
                return ERR_PROG_QSPI_IMAGE_FORMAT;
        }

        /* Compute required output buffer size */
        if (IMG_QSPI == type && IMG_CACHED == mode) {
                /* For cached images size does not change */
                size = binary_size;
        } else {
                /* For other images, header is added extra, add some padding for OTP */
                size = header_size + binary_size;
                if (IMG_OTP == type) {
                        size += ((binary_size + 7) & ~7) - binary_size;
                }
        }

        if (required_size) {
                *required_size = size;
        }

        if (size > buf_size || !buf) {
                return ERR_PROG_INSUFICIENT_BUFFER;
        }

        /* Image can be made in place, make sure that memory is moved correctly */
        if (IMG_QSPI == type && IMG_CACHED == mode) {
                /* Buffer must contain at least RAM block */
                if (buf_size < ram_at_0_size) {
                        return ERR_PROG_INSUFICIENT_BUFFER;
                }

                /*
                 * Cached binary, most of the image does not need to be moved if input
                 * and output buffer is same.
                 */
                if (binary != buf) {
                        memcpy(buf + ram_at_0_size, binary + ram_at_0_size,
                                                                binary_size - ram_at_0_size);
                }
                /* Move vector table a bit, 8  bytes are lost */
                memmove(buf + header_size, binary, ram_at_0_size - header_size);
        } else {
                /* Fill padding with 0 for OTP images */
                memset(buf + binary_size, 0, size - binary_size);
                /* Copy/move binary to make space for header */
                memmove(buf + header_size, binary, binary_size);
        }

        /* Add header at the beginning */
        memcpy(buf, header_buffer, header_size);

        return size;
}

static const uint32_t crc32_tab[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
        0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
        0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
        0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
        0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
        0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
        0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
        0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
        0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
        0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t len)
{
        while (len--) {
                crc = crc32_tab[(crc ^ *data++) & 0xff] ^ (crc >> 8);
        }
        return crc;
}

static void fill_suota_header(suota_1_1_image_header_t *img_header, const uint8_t *buf, int size,
                                        const char *version, time_t time_stamp, uint16_t flags)
{
        img_header->signature[0] = SUOTA_1_1_IMAGE_HEADER_SIGNATURE_B1;
        img_header->signature[1] = SUOTA_1_1_IMAGE_HEADER_SIGNATURE_B2;
        img_header->flags = flags;
        img_header->code_size = size;
        img_header->timestamp = time_stamp;
        img_header->exec_location = sizeof(img_header);
        strncpy((char *) img_header->version, version, sizeof(img_header->version));
        img_header->version[sizeof(img_header->version) - 1] = '\0';
        img_header->crc = crc32_update(~0, buf, size) ^ ~0;
}

int prog_write_qspi_suota_image(uint8_t *buf, int size, const char *version,
                                                                time_t time_stamp, uint16_t flags)
{
        cmd_partition_table_t *part_table;
        cmd_partition_entry_t *entry;
        cmd_partition_entry_t *part_table_end;
        uint32_t part_table_len;
        suota_1_1_image_header_t img_header;
        int exec_partition_start = -1;
        int image_header_start = -1;
        int ret;

        /*
         * Read partition table to see if NVMS_IMAGE_HEADER_PART and NVMS_FW_EXEC_PART
         * partitions are present.
         */
        ret = prog_read_partition_table((uint8_t **) &part_table, &part_table_len);
        if (ret) {
                goto end;
        }

        /*
         * Find start of those two partitions.
         */
        part_table_end = (cmd_partition_entry_t *) (((uint8_t *) part_table) + part_table_len);
        entry = &part_table->entry;

        while (entry < part_table_end) {
                if (entry->type == NVMS_FW_EXEC_PART) {
                        exec_partition_start = entry->start_address;
                } else if (entry->type == NVMS_IMAGE_HEADER_PART) {
                        image_header_start = entry->start_address;
                }
                entry = (cmd_partition_entry_t *) (((uint8_t *) entry) + sizeof(*entry)
                                                                        + entry->name.len);
        }
        free(part_table);

        if (image_header_start < 0 || exec_partition_start < 0) {
                ret = ERR_PROG_NO_PARTITON;
                goto end;
        }

        /*
         * Prepare SUOTA image header with user supplied data.
         */
        fill_suota_header(&img_header, buf, size, version, time_stamp, flags);

        /*
         * Write image header to NVMS_IMAGE_HEADER_PART partition then binary
         * data to NVMS_FW_EXEC_PART
         */
        ret = prog_write_to_qspi(image_header_start, (uint8_t *) &img_header, sizeof(img_header));
        if (ret) {
                goto end;
        }

        ret = prog_write_to_qspi(exec_partition_start, buf, size);

end:
        return ret;
}

void prog_set_target_reset_cmd(const char *trc)
{
        target_reset_cmd = strdup(trc);
}

int prog_gdb_connect(const char *host_name, int port)
{
        target = &target_gdb_server;

        return gdb_server_connect(host_name, port);
}

void prog_gdb_disconnect()
{
        gdb_server_disconnect();
}

prog_gdb_server_info_t *prog_get_gdb_instances(const char *gdb_server_cmd)
{
        return gdb_server_get_instances(gdb_server_cmd);
}

void prog_get_version(uint32_t *major, uint32_t *minor)
{
        *major = LIB_PROG_VERSION_MAJOR;
        *minor = LIB_PROG_VERSION_MINOR;
}

connection_status_t prog_verify_connection(void)
{
        return target->verify_connection();
}
