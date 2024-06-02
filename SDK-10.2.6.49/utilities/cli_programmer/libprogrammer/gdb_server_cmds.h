/**
 ****************************************************************************************
 *
 * @file gdb_server_cmds.h
 *
 * @brief GDB Server interface header file
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 * \addtogroup UTILITIES
 * \{
 * \addtogroup PROGRAMMER
 * \{
 * \addtogroup LIBRARY
 * \{
 */

#ifndef GDB_SERVER_CMDS_H_
#define GDB_SERVER_CMDS_H_

/**
 * Maximal read and write chunk sizes for GDB Server interface
 */
#define GDB_SERVER_READ_CHUNK_SIZE      (0xC000)
#define GDB_SERVER_WRITE_CHUNK_SIZE     (0x2000)

/**
 * \brief Initialize GDB Server
 *
 * \note 0 is returned if initialization was done properly, but GDB Server's PID is unavailable
 *
 * \param [in] gdb_server_conf GDB Server configuration
 *
 * \returns PID of used GDB Server instance or 0 on success, negative value with error code on failure
 *
 */
int gdb_server_initialization(const prog_gdb_server_config_t *gdb_server_conf);

/**
 * \brief Set GDB Server bootloader code for firmware update
 *
 *
 * Binary data specified in this command will be sent to device.
 *
 * \param [in] code binary bootloader data
 * \param [in] size code size
 *
 */
void gdb_server_set_boot_loader_code(uint8_t *code, size_t size);

/**
 * \brief Get GDB Server bootloader code for firmware update
 *
 * \param [out] code binary bootloader data
 * \param [out] size code size
 *
 */
void gdb_server_get_boot_loader_code(uint8_t **code, size_t *size);

/**
 * \brief Write device memory
 *
 * This function writes device RAM with specified data.
 *
 * \param [in] buf binary data to send to device
 * \param [in] size size of buf
 * \param [in] addr RAM address to write data to
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_write(const uint8_t *buf, size_t size, uint32_t addr);

/**
 * \brief Read device memory without bootloader
 *
 * This function reads device memory as seen by device CPU. It doesn't require to have bootloader
 * running.
 *
 * \param [out] buf buffer for data
 * \param [in] size size of buf in bytes
 * \param [in] addr RAM address to read from
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_direct_read(uint8_t *buf, size_t size, uint32_t addr);

/**
 * \brief Read device memory
 *
 * This function reads device memory as seen by device CPU.
 *
 * \param [out] buf buffer for data
 * \param [in] size size of buf in bytes
 * \param [in] addr RAM address to read from
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_read(uint8_t *buf, size_t size, uint32_t addr);

/**
 * \brief Copy memory to QSPI flash or eFLASH
 *
 * This function programs QSPI flash memory with data already present in device RAM.
 *
 * \note For using eFLASH the address must be OR-ed with 0x80000000 mask in \p dst_address.
 *
 * \param [in] src_address address in device RAM
 * \param [in] size size of memory to copy
 * \param [in] dst_address offset in flash to write to
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_copy_to_qspi(uint32_t src_address, size_t size, uint32_t dst_address);

/**
 * \brief Write data to QSPI FLASH or eFLASH
 *
 * This function writes the specified data to the device QSPI FLASH memory.
 *
 * \note For using eFLASH the address must be OR-ed with 0x80000000 mask in \p addr.
 *
 * \param [in] buf binary data to send to device
 * \param [in] size size of buf
 * \param [in] addr offset in flash to write to
 * \param [in] verify true for performing QSPI writing verification
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_direct_write_to_qspi(const uint8_t *buf, size_t size, uint32_t addr, bool verify);

/**
 * \brief Boot arbitrary binary.
 *
 * This function boots the device with a provided application binary.
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_run(uint32_t address);

/**
 * \brief Erase QSPI flash or eFLASH region
 *
 * This function erases flash at specified offset.
 *
 * \note For using eFLASH the address must be OR-ed with 0x80000000 mask in \p address.
 *
 * \param [in] address address in flash to start erase
 * \param [in] size size of memory to erase
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_erase_qspi(uint32_t address, size_t size);

/**
 * \brief Chip erase QSPI flash
 *
 * This function erases whole flash memory.
 *
 * \param [in] address Start address of the QSPI flash memory to be erased
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_chip_erase_qspi(uint32_t address);

/**
 * \brief Write data to OTP
 *
 * \param [in] address address of 64-bit cell in OTP
 * \param [in] buf words to be written
 * \param [in] len number of words in buffer
 *
 * \return 0 on success, error code on failure
 *
 */
int gdb_server_cmd_write_otp(uint32_t address, const uint32_t *buf, uint32_t len);

/**
 * \brief Read data from OTP
 *
 * \param [in] address address of 64-bit cell in OTP
 * \param [out] buf buffer for read words
 * \param [in] len number of words to be read
 *
 * \return 0 on success, error code on failure
 *
 */
int gdb_server_cmd_read_otp(uint32_t address, uint32_t *buf, uint32_t len);

/**
 * \brief Read data from QSPI or eFLASH
 *
 * \note For using eFLASH the address must be OR-ed with 0x80000000 mask in \p address.
 *
 * \param [in] address offset in flash
 * \param [out] buf buffer for data
 * \param [in] len length of buffer
 *
 * \return 0 on success, error code on failure
 *
 */
int gdb_server_cmd_read_qspi(uint32_t address, uint8_t *buf, uint32_t len);

/**
 * \brief Check emptiness of QSPI flash
 *
 * If specified flash region is empty, parameter \p ret_number is a number of checked bytes
 * (positive value). Otherwise if specified flash region contains values different than 0xFF,
 * parameter \p ret_number is a nonpositive value. This value is a number of first non 0xFF byte in
 * checked region multiplied by -1.
 *
 * \note For using eFLASH the address must be OR-ed with 0x80000000 mask in \p start_address.
 *
 * \param [in]  size number of bytes to check
 * \param [in]  start_address start address in QSPI flash
 * \param [out] ret_number number of checked bytes, or number of first non 0xFF byte multiplied by -1
 *
 * \return 0 on success, error code on failure
 *
 */
int gdb_server_cmd_is_empty_qspi(unsigned int size, unsigned int start_address, int *ret_number);

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
int gdb_server_cmd_get_qspi_state(uint8_t id, flash_dev_info_t *qspi_flash_info);

/**
 * \brief Read partition table
 *
 * \param [out] buf buffer to store the contents of the partition table
 * \param [out] len the size of buffer in bytes
 *
 * \return 0 on success, error code on failure
 *
 */
int gdb_server_cmd_read_partition_table(uint8_t **buf, uint32_t *len);

/**
 * \brief Read data from partition
 *
 * \param [in] id partition id
 * \param [in] address address in partition
 * \param [out] buf buffer for data
 * \param [in] len length of buffer
 *
 * \return 0 on success, error code on failure
 *
 */
int gdb_server_cmd_read_partition(nvms_partition_id_t id, uint32_t address, uint8_t *buf,
                                                                                uint32_t len);

/**
 * \brief Write NVMS partition with device memory
 *
 * This function writes NVMS partition with specified data.
 *
 * \param [in] id partition id
 * \param [in] dst_address destination address to write data to
 * \param [in] src_address RAM source address
 * \param [in] size number of bytes to write
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_write_partition(nvms_partition_id_t id, uint32_t dst_address,
                                                                uint32_t src_address, size_t size);

/**
 * \brief Boot arbitrary application binary
 *
 * \param [in] executable_code pointer to executable binary buffer
 * \param [in] executable_code_size size of executable binary buffer
 *
 * \return 0 on success, error code on failure
 *
 */
int gdb_server_cmd_boot(uint8_t *executable_code, size_t executable_code_size);

/**
 * \brief Force upload uartboot to device
 *
 * \return 0 on success, error code on failure
 *
 */
int gdb_server_cmd_upload_bootloader(void);

/**
 * \brief Mass erase eFLASH
 *
 * This function erases whole eFLASH memory.
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_mass_erase_eflash(void);

/**
 * \brief Copy memory to OQSPI flash
 *
 * This function programs OQSPI flash memory with data already present in device RAM.
 *
 * \param [in] src_address address in device RAM
 * \param [in] size size of memory to copy
 * \param [in] dst_address offset in flash to write to
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_copy_to_oqspi(uint32_t src_address, size_t size, uint32_t dst_address);

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
int gdb_server_cmd_direct_write_to_oqspi(const uint8_t *buf, size_t size, uint32_t addr, bool verify);

/**
 * \brief Erase OQSPI flash region
 *
 * This function erases flash at specified offset.
 *
 * \param [in] address address in flash to start erase
 * \param [in] size size of memory to erase
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_erase_oqspi(uint32_t address, size_t size);

/**
 * \brief Chip erase OQSPI flash
 *
 * This function erases whole flash memory.
 *
 * \param [in] address Start address of the OQSPI flash memory to be erased
 *
 * \returns 0 on success, negative value with error code on failure
 *
 */
int gdb_server_cmd_chip_erase_oqspi(uint32_t address);

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
int gdb_server_cmd_read_oqspi(uint32_t address, uint8_t *buf, uint32_t len);

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
int gdb_server_cmd_is_empty_oqspi(unsigned int size, unsigned int start_address, int *ret_number);

/**
 * \brief Read OQSPI flash info
 *
 * Reads manufacturer ID as well as device type and density of OQSPI flash.
 *
 * \param [out] oqspi_flash_info pointer to the struct where OQSPI flash info is stored
 *
 * \return 0 on success, error code on failure
 */
int gdb_server_cmd_get_oqspi_state(flash_dev_info_t *oqspi_flash_info);

/**
 * \brief Get product information
 *
 * \param [out] buf buffer to store the contents of the product information
 * \param [out] len the size of buffer in bytes
 *
 * \return 0 on success, error code on failure
 *
 */
int gdb_server_cmd_get_product_info(uint8_t **buf, uint32_t *len);

/**
 * \brief Close GDB Server interface
 *
 * Function stops GDB Server instance with given pid.
 *
 * \note This function doesn't close opened socket, gdb_server_disconnect() function should be used
 * before calling this function.
 * \note If no_kill_mode was set to NO_KILL_ALL or NO_KILL_DISCONNECT during interface
 * initialization then GDB Server process will not be killed.
 *
 * \param [in] pid PID of the selected GDB Server
 *
 * \sa gdb_server_disconnect
 *
 */
void gdb_server_close(int pid);

/**
* \brief Invalidate stub
*
* This function ensures that the stub will be reloaded next time.
*
*/
void gdb_invalidate_stub(void);

/**
 * \brief Connect to GDB Server
 *
 * \note After usage of this function the gdb_server_disconnect function should be used for cleanup.
 *
 * \param [in] host_name host name
 * \param [in] port port number
 *
 * \return 0 on success, error code on failure
 *
 */
int gdb_server_connect(const char *host_name, int port);

/**
 * \brief Disconnect with GDB Server instance
 *
 * Function closes socket which is used for communication with GDB Server.
 *
 * \note If socket was not opened earlier, then function will do nothing.
 *
 */
void gdb_server_disconnect();

/**
 * \brief Get GDB Server instances array
 *
 * \note Each valid element has PID value > 0. First element with PID < 0 means end of array.
 * \note Array is dynamic allocated - must be freed after use.
 *
 * \param [in] gdb_server_cmd GDB Server run command
 *
 * \return array with GDB Server instances on success, NULL otherwise
 *
 */
prog_gdb_server_info_t *gdb_server_get_instances(const char *gdb_server_cmd);

/**
 * \brief Verify connection with device
 *
 * \note Use this option only if `check_bootloader` configuration field is set to true.
 *
 * \return CONN_ESTABLISHED if bootloader check is enabled and uartboot is uploaded to the device
 *         CONN_ALLOWED if uartboot can be uploaded
 *         CONN_ERROR if uartboot can not be uploaded (not connected to gdbserver)
 *
 */
connection_status_t gdb_server_verify_connection(void);

#endif /* GDB_SERVER_CMDS_H_ */

/**
 * \}
 * \}
 * \}
 */
