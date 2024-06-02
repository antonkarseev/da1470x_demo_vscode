/**
 ****************************************************************************************
 *
 * @file qspi_automode.c
 *
 * @brief Access QSPI flash when running in auto mode
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sdk_defs.h"
#include "hw_clk.h"
#include "hw_pd.h"

#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)


#include "hw_qspi.h"
#include "qspi_automode.h"
#include "qspi_internal.h"

#define QSPI_READ_PIPE_DELAY_0V9             (2)
#define QSPI_READ_PIPE_DELAY_1V2             (7)

static bool flash_is_busy(HW_QSPIC_ID id);
static void flash_write_enable(HW_QSPIC_ID id);
static void flash_write_status_register(HW_QSPIC_ID id, uint8_t value);
static void flash_transact(HW_QSPIC_ID id, const uint8_t *wbuf, uint32_t wlen, uint8_t *rbuf, uint32_t rlen);
static void flash_write(HW_QSPIC_ID id, const uint8_t *wbuf, uint32_t wlen);
static uint8_t flash_read_status_register(HW_QSPIC_ID id);
static void init_hw_qspi(HW_QSPIC_ID id);

#if dg_configUSE_HW_QSPI2
static void psram_set_cs_active_max(HW_QSPIC_ID id, sys_clk_t sys_clk, uint32_t cs_active_time_max_us);
#endif


/*
 * Debug options
 */
#if __DBG_QSPI_ENABLED
#define __DBG_QSPI_VOLATILE__           volatile
#pragma message "Automode: Debugging is on!"
#else
#define __DBG_QSPI_VOLATILE__
#endif

/*
 * QSPI controller allows to execute code directly from QSPI flash.
 * When code is executing from flash there is no possibility to reprogram it.
 * To be able to modify flash memory while it is used for code execution it must me assured that
 * during the time needed for erase/write no code is running from flash.
 */

/*
 * Flash specific defines
 */


/*
 * Use QUAD mode for page write.
 *
 * Note: If the flash does not support QUAD mode or it is not connected for QUAD mode set it to 0
 * (single mode).
 */
#ifndef QUAD_MODE
#define QUAD_MODE                       1
#endif

#ifndef ERASE_IN_AUTOMODE
#define ERASE_IN_AUTOMODE               1
#endif

#ifndef FLASH_FORCE_24BIT_ADDRESSING
#define FLASH_FORCE_24BIT_ADDRESSING    0       // Force 24 bit addressing for devices > 128Mbits
#endif

        #define __HW_QSPI_INIT_POL        HW_QSPI_POL_LOW


/*
 * WARNING: The Autodetect mode will increase both the code and the used RetRAM size!!!!
 *          Use with extreme caution!!
 */
#if (dg_configUSE_HW_QSPI == 1) && (dg_configFLASH_AUTODETECT == 0)
        #if !defined(dg_configFLASH_CONFIG)
        #error Please define dg_configFLASH_CONFIG !!!
        #endif
        #endif

#if (dg_configUSE_HW_QSPI2 == 1) && (dg_configQSPIC2_DEV_AUTODETECT == 0)
        #if !defined(dg_configQSPIC2_DEV_CONFIG)
        #error Please define dg_configQSPIC2_DEV_CONFIG !!!
        #endif
#endif


#if (FLASH_AUTODETECT == 1)
# include dg_configQSPI_MEMORY_CONFIG_TABLE_HEADER
#else /* FLASH_AUTODETECT */
#       if ((dg_configUSE_HW_QSPI == 1) && (dg_configFLASH_AUTODETECT == 0))
#               ifndef dg_configFLASH_HEADER_FILE
#                      error Please define macro dg_configFLASH_HEADER_FILE to the header file name that contains the respective implementation
#               endif
#               include dg_configFLASH_HEADER_FILE
#       endif
#       if ((dg_configUSE_HW_QSPI2 == 1) && (dg_configQSPIC2_DEV_AUTODETECT == 0))
#               ifndef dg_configQSPIC2_DEV_HEADER_FILE
#                       error Please define macro dg_configQSPIC2_DEV_HEADER_FILE to the header file name that contains the respective implementation
#               endif
#              include dg_configQSPIC2_DEV_HEADER_FILE
#       endif
#endif /* FLASH_AUTODETECT */

#if FLASH_AUTODETECT
__RETAINED qspi_flash_config_t flash_config[QSPI_CONTROLLER_SUPPORT_NUM];
#endif

typedef struct {
        uint32_t ctrlmode_reg;
        uint32_t burstcmda_reg;
        uint32_t burstcmdb_reg;
        uint32_t erasecmda_reg;
        uint32_t erasecmdb_reg;
        uint32_t statuscmd_reg;
        uint32_t gp_reg;
        uint32_t awritecmd_reg;
        uint32_t memblen_reg;
} qspic_config_t;

__RETAINED static qspic_config_t qspic_config[QSPI_CONTROLLER_SUPPORT_NUM];
__RETAINED static bool qspi_is_device_present[QSPI_CONTROLLER_SUPPORT_NUM];


#if FLASH_AUTODETECT
#       if (dg_configUSE_HW_QSPI2 == 1)
#               define QSPI_GET_CONFIG_IDX(id) (id == HW_QSPIC ? 0 : 1)
#               define QSPI_GET_CONFIG_BASE_REG(idx) (idx == 0 ? HW_QSPIC : HW_QSPIC2)
#       else
#               define QSPI_GET_CONFIG_IDX(id) (0)
#               define QSPI_GET_CONFIG_BASE_REG(idx) (HW_QSPIC)
#       endif
#       define QSPI_GET_DEVICE_PARAM(idx, param) (flash_config[idx].param)
#else
#       if (dg_configUSE_HW_QSPI == 1) && (dg_configUSE_HW_QSPI2 == 0)
#               define QSPI_GET_CONFIG_IDX(id)       (0) // Value is not used. It is defined to suppress errors
#               define QSPI_GET_CONFIG_BASE_REG(idx) (HW_QSPIC)
#               define QSPI_GET_DEVICE_PARAM(idx, param) (dg_configFLASH_CONFIG.param)
#       elif (dg_configUSE_HW_QSPI == 0) && (dg_configUSE_HW_QSPI2 == 1)
#               define QSPI_GET_CONFIG_IDX(id)       (1) // Value is not used. It is defined to suppress errors
#               define QSPI_GET_CONFIG_BASE_REG(idx) (HW_QSPIC2)
#               define QSPI_GET_DEVICE_PARAM(idx, param) (dg_configQSPIC2_DEV_CONFIG.param)
#       elif (dg_configUSE_HW_QSPI == 1) && (dg_configUSE_HW_QSPI2 == 1)
#               define QSPI_GET_CONFIG_IDX(id) (id == HW_QSPIC ? 0 : 1)
#               define QSPI_GET_CONFIG_BASE_REG(idx) (idx == 0 ? HW_QSPIC : HW_QSPIC2)
#               define QSPI_GET_DEVICE_PARAM(idx, param) (idx == 0 ? \
                                dg_configFLASH_CONFIG.param : dg_configQSPIC2_DEV_CONFIG.param)
#       endif
#endif

/*
 * Function definitions
 */
/**
 * \brief Get the QSPI controller id from the address of data accessed
 *
 * \param[in] addr The address of data accessed. It may be anywhere in a page.
 * \param[in] size The number of bytes accessed.
 *
 * \return QSPI controller id
 */
__STATIC_FORCEINLINE HW_QSPIC_ID flash_get_addr_id(uint32_t addr, uint32_t size)
{
        ASSERT_WARNING(size > 0);
        ASSERT_WARNING(qspi_is_valid_addr(addr + size - 1));

#if dg_configUSE_HW_QSPI && dg_configUSE_HW_QSPI2
        if (addr >= QSPI_MEM2_VIRTUAL_BASE_ADDR) {
                return HW_QSPIC2;
        } else {
                return HW_QSPIC;
        }
#elif dg_configUSE_HW_QSPI
        return HW_QSPIC;
#elif dg_configUSE_HW_QSPI2
        return HW_QSPIC2;
#endif
}

__STATIC_FORCEINLINE uint32_t flash_get_zero_based_addr(uint32_t addr)
{
#if dg_configUSE_HW_QSPI && dg_configUSE_HW_QSPI2
        if (addr >= QSPI_MEM2_VIRTUAL_BASE_ADDR) {
                return addr - QSPI_MEM2_VIRTUAL_BASE_ADDR;
        } else {
                return addr - QSPI_MEM1_VIRTUAL_BASE_ADDR;
        }
#elif dg_configUSE_HW_QSPI
        return addr - QSPI_MEM1_VIRTUAL_BASE_ADDR;
#elif dg_configUSE_HW_QSPI2
        return addr - QSPI_MEM2_VIRTUAL_BASE_ADDR;
#endif
}


__RETAINED_CODE bool qspi_is_valid_addr(uint32_t addr)
{
        if (addr < QSPI_MEM1_VIRTUAL_BASE_ADDR) {
                return false;
        }

#if dg_configUSE_HW_QSPI2
        if (addr >= (QSPI_MEM2_VIRTUAL_BASE_ADDR + HW_QSPI_MAX_ADDR_SIZE)) {
                return false;
        }

        if (addr >= QSPI_MEM2_VIRTUAL_BASE_ADDR) {
                if (qspi_is_device_present[QSPI_GET_CONFIG_IDX(HW_QSPIC2)]) {
                        return (((addr - QSPI_MEM2_VIRTUAL_BASE_ADDR) * 8) <
                                QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(HW_QSPIC2),memory_size));
                } else {
                        return false;
                }
        }
#endif

        if (qspi_is_device_present[QSPI_GET_CONFIG_IDX(HW_QSPIC)] == false || addr >= (QSPI_MEM1_VIRTUAL_BASE_ADDR + HW_QSPI_MAX_ADDR_SIZE)) {
                return false;
        }

        return ((addr - QSPI_MEM1_VIRTUAL_BASE_ADDR) * 8) < QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(HW_QSPIC), memory_size);
}

uint32_t qspi_get_device_size(const HW_QSPIC_ID id)
{
        return (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), memory_size) / 8);
}

bool qspi_get_config(const HW_QSPIC_ID id, uint8_t *manufacturer_id,
                     uint8_t *device_type, uint8_t *density)
{
        if (!qspi_is_device_present[QSPI_GET_CONFIG_IDX(id)]) {
                return false;
        }

        *manufacturer_id = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), manufacturer_id);
        *device_type = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), device_type);
        *density = (uint8_t)QSPI_GET_DENSITY(QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), device_density));

        return true;
}


#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */


#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)

/**
 * \brief Set bus mode to single or QUAD mode.
 *
 * \param[in] id QSPI controller id
 * \param[in] mode Can be single (HW_QSPI_BUS_MODE_SINGLE) or quad (HW_QSPI_BUS_MODE_QUAD) mode.
 *
 * \note DUAL mode page program so is not supported by this function.
 */
__STATIC_FORCEINLINE void flash_set_bus_mode(HW_QSPIC_ID id, HW_QSPI_BUS_MODE mode)
{
#if dg_configUSE_HW_QSPI && dg_configUSE_HW_QSPI2
        ASSERT_WARNING((id == HW_QSPIC) || (id == HW_QSPIC2));
#elif dg_configUSE_HW_QSPI
        ASSERT_WARNING(id == HW_QSPIC);
#else
        ASSERT_WARNING(id == HW_QSPIC2);
#endif

        if (mode == HW_QSPI_BUS_MODE_SINGLE) {
                id->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_SET_SINGLE);
                id->QSPIC_CTRLMODE_REG |=
                        BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO2_OEN, 1) |
                        BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO2_DAT, 1) |
                        BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO3_OEN, 1) |
                        BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO3_DAT, 1);
        } else {
#if QUAD_MODE
                id->QSPIC_CTRLBUS_REG = REG_MSK(QSPIC, QSPIC_CTRLBUS_REG, QSPIC_SET_QUAD);
                id->QSPIC_CTRLMODE_REG &=
                        ~(BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO2_OEN, 1) |
                          BITS32(QSPIC, QSPIC_CTRLMODE_REG, QSPIC_IO3_OEN, 1));
#endif
        }
}

/**
 * \brief Set device in QPI mode
 *
 * \param[in] id QSPI controller id
 *
 */
__RETAINED_CODE static void flash_enter_qpi_mode(HW_QSPIC_ID id)
{
        if (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), qpi_mode)) {
                hw_qspi_cs_enable(id);
                hw_qspi_write8(id, QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), enter_qpi_opcode));
                hw_qspi_cs_disable(id);
                flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
        }
}

/**
 * \brief Set the mode of the QSPI controller (manual or auto)
 *
 * \param[in] id QSPI controller id
 * \param[in] automode True for auto and false for manual mode setting.
 */
__STATIC_FORCEINLINE void flash_set_automode(HW_QSPIC_ID id, bool automode)
{
        HW_QSPIC_REG_SETF(id, CTRLMODE, AUTO_MD, automode);
}

/**
 * \brief Write to the Flash the contents of a buffer
 *
 * \param[in] id QSPI controller id
 * \param[in] wbuf Pointer to the beginning of the buffer
 * \param[in] wlen The number of bytes to be written
 *
 * \note The data are transferred as bytes (8 bits wide). No optimization is done in trying to use
 *       faster access methods (i.e. transfer words instead of bytes whenever it is possible).
 */
__RETAINED_CODE static void flash_write(HW_QSPIC_ID id, const uint8_t *wbuf, uint32_t wlen)
{
        uint32_t i;

        hw_qspi_cs_enable(id);

        for (i = 0; i < wlen; ++i) {
                hw_qspi_write8(id, wbuf[i]);
        }

        hw_qspi_cs_disable(id);
}

/**
 * \brief Write an arbitrary number of bytes to the Flash and then read an arbitrary number of bytes
 *       from the Flash in one transaction
 *
 * \param[in] id QSPI controller id
 * \param[in] wbuf Pointer to the beginning of the buffer that contains the data to be written
 * \param[in] wlen The number of bytes to be written
 * \param[in] rbuf Pointer to the beginning of the buffer than the read data are stored
 * \param[in] rlen The number of bytes to be read
 *
 * \note The data are transferred as bytes (8 bits wide). No optimization is done in trying to use
 *       faster access methods (i.e. transfer words instead of bytes whenever it is possible).
 */
__RETAINED_CODE static void flash_transact(HW_QSPIC_ID id, const uint8_t *wbuf, uint32_t wlen,
                                                                        uint8_t *rbuf, uint32_t rlen)
{
        uint32_t i;

        hw_qspi_cs_enable(id);

        for (i = 0; i < wlen; ++i) {
                hw_qspi_write8(id, wbuf[i]);
        }

        for (i = 0; i < rlen; ++i) {
                rbuf[i] = hw_qspi_read8(id);
        }

        hw_qspi_cs_disable(id);
}

__RETAINED_CODE static bool flash_erase_program_in_progress(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd;

        cmd = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), read_erase_progress_opcode);

        flash_transact(id, &cmd, 1, &status, 1);

        return ((status & (1 << QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), erase_in_progress_bit))) != 0)
                        == QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), erase_in_progress_bit_high_level);
}

__RETAINED_CODE static bool flash_is_busy(HW_QSPIC_ID id)
{
        return (flash_read_status_register(id) & FLASH_STATUS_BUSY_MASK) != 0;
}

/**
 * \brief Exit from continuous mode.
 *
 * \param[in] id QSPI controller id
 */
__RETAINED_CODE static void flash_reset_continuous_mode(HW_QSPIC_ID id, HW_QSPI_BREAK_SEQ_SIZE break_seq_size)
{
#if (FLASH_AUTODETECT == 0)
        // All memories with 32bits address size require break sequence with 2bytes size
        ASSERT_WARNING(((QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), address_size) == HW_QSPI_ADDR_SIZE_32)
                         && (break_seq_size == HW_QSPI_BREAK_SEQ_SIZE_2B)) ||
                        (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), address_size) == HW_QSPI_ADDR_SIZE_24));
#endif

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, CMD_EXIT_CONTINUOUS_MODE);
        if (break_seq_size == HW_QSPI_BREAK_SEQ_SIZE_2B) {
                hw_qspi_write8(id, CMD_EXIT_CONTINUOUS_MODE);
        }
        hw_qspi_cs_disable(id);
}

/**
 * \brief Get Device ID when Flash is not in Power Down mode
 *
 * \param[in] id QSPI controller id
 * \return uint8_t The Device ID of the Flash
 *
 * \note The function blocks until the Flash executes the command.
 */
__RETAINED_CODE __UNUSED static uint8_t flash_get_device_id(HW_QSPIC_ID id)
{
        uint8_t device_id;

        hw_qspi_cs_enable(id);
        hw_qspi_write32(id, CMD_RELEASE_POWER_DOWN);
        device_id = hw_qspi_read8(id);
        hw_qspi_cs_disable(id);

        while (flash_is_busy(id));

        return device_id;
}

/**
 * \brief Set WEL (Write Enable Latch) bit of the Status Register of the Flash
 * \details The WEL bit must be set prior to every Page Program, Quad Page Program, Sector Erase,
 *       Block Erase, Chip Erase, Write Status Register and Erase/Program Security Registers
 *       instruction. In the case of Write Status Register command, any status bits will be written
 *       as non-volatile bits.
 *
 * \param[in] id QSPI controller id
 *
 * \note This function blocks until the Flash has processed the command and it will be repeated if,
 *       for any reason, the command was not successfully executed by the Flash.
 */
__RETAINED_CODE static void flash_write_enable(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { CMD_WRITE_ENABLE };

        do {
                flash_write(id, cmd, 1);
                /* Verify */
                do {
                        status = flash_read_status_register(id);
                } while (status & FLASH_STATUS_BUSY_MASK);
        } while (!(status & FLASH_STATUS_WEL_MASK));
}

/**
 * \brief Read the Status Register 1 of the Flash
 *
 * \param[in] id QSPI controller id
 *
 * \return uint8_t The value of the Status Register 1 of the Flash.
 */
__RETAINED_CODE static uint8_t flash_read_status_register(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { CMD_READ_STATUS_REGISTER };

        flash_transact(id, cmd, 1, &status, 1);

        return status;
}

/**
 * \brief Write the Status Register 1 of the Flash
 *
 * \param[in] id QSPI controller id
 * \param[in] value The value to be written.
 *
 * \note This function blocks until the Flash has processed the command. No verification that the
 *        value has been actually written is done though. It is up to the caller to decide whether
 *        such verification is needed or not and execute it on its own.
 */
__RETAINED_CODE __UNUSED static void flash_write_status_register(HW_QSPIC_ID id, uint8_t value)
{
        uint8_t cmd[2] = { CMD_WRITE_STATUS_REGISTER, value };

        flash_write(id, cmd, 2);

        /* Wait for the Flash to process the command */
        while (flash_is_busy(id));
}

/**
 * \brief Fast copy of a buffer to a FIFO
 * \details Implementation of a fast copy of the contents of a buffer to a FIFO in assembly. All
 *        addresses are word aligned.
 *
 * \param[in] start Pointer to the beginning of the buffer
 * \param[in] end Pointer to the end of the buffer
 * \param[in] Pointer to the FIFO
 *
 * \warning No validity checks are made! It is the responsibility of the caller to make sure that
 *        sane values are passed to this function.
 */
__STATIC_FORCEINLINE void fast_write_to_fifo32(uint32_t start, uint32_t end, uint32_t dest)
{
        asm volatile(   "copy:                                  \n"
                        "       ldmia %[start]!, {r3}           \n"
                        "       str r3, [%[dest]]               \n"
                        "       cmp %[start], %[end]            \n"
                        "       blt copy                        \n"
                        :
                        :                                                         /* output */
                        [start] "l" (start), [end] "r" (end), [dest] "l" (dest) : /* inputs (%0, %1, %2) */
                        "r3");                                              /* registers that are destroyed */
}

/**
 * \brief Write data (up to 1 page) to Flash
 *
 * \param[in] addr The address of the Flash where the data will be written. It may be anywhere in a
 *        page.
 * \param[in] buf Pointer to the beginning of the buffer that contains the data to be written.
 * \param[in] size The number of bytes to be written.
 *
 * \return The number of bytes written.
 *
 * \warning The boundary of the page where addr belongs to, will not be crossed! The caller should
 *        issue another flash_write_page() call in order to write the remaining data to the next
 *        page.
 */
__RETAINED_CODE static uint32_t flash_write_page(uint32_t addr, const uint8_t *buf, uint32_t size)
{
        uint32_t i = 0;

        uint32_t odd = ((uint32_t) buf) & 3;
        uint32_t size_aligned32;
        uint32_t tmp;
        HW_QSPIC_ID id = flash_get_addr_id(addr, size);
        uint8_t idx __UNUSED = QSPI_GET_CONFIG_IDX(id);
        addr = flash_get_zero_based_addr(addr);

        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_PAGE_PROG);

        flash_write_enable(id);

        /* Reduce max write size, that can reduce interrupt latency time */
        if (size > dg_configFLASH_MAX_WRITE_SIZE) {
                size = dg_configFLASH_MAX_WRITE_SIZE;
        }

        /* Make sure write will not cross page boundary */
        tmp = 256 - (addr & 0xFF);
        if (size > tmp) {
                size = tmp;
        }

        hw_qspi_cs_enable(id);

        if (QSPI_GET_DEVICE_PARAM(idx, qpi_mode)) { // QPI mode
                // Must already be in QUAD mode
                ASSERT_WARNING(QUAD_MODE == 1);

                if (QSPI_GET_DEVICE_PARAM(idx, address_size) == HW_QSPI_ADDR_SIZE_32) {
                        hw_qspi_write8(id, QSPI_GET_DEVICE_PARAM(idx, page_qpi_program_opcode));
                        hw_qspi_write32(id, __REV(addr));
                } else {
                       hw_qspi_write32(id, QSPI_GET_DEVICE_PARAM(idx,
                                          page_qpi_program_opcode) | (__REV(addr) & 0xFFFFFF00));
                }
        } else {
                if (QSPI_GET_DEVICE_PARAM(idx, address_size) == HW_QSPI_ADDR_SIZE_32) {
                        hw_qspi_write8(id, QSPI_GET_DEVICE_PARAM(idx, page_program_opcode));
#if QUAD_MODE
                        if (QSPI_GET_DEVICE_PARAM(idx, quad_page_program_address) == true) {
                                flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
                        }
#endif
                        hw_qspi_write32(id, __REV(addr));

#if QUAD_MODE
                        if (QSPI_GET_DEVICE_PARAM(idx, quad_page_program_address) == false) {
                                flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
                        }
#endif
                }
                else {
                        if (QSPI_GET_DEVICE_PARAM(idx, quad_page_program_address) == true) {
                                hw_qspi_write8(id, QSPI_GET_DEVICE_PARAM(idx, page_program_opcode));
#if QUAD_MODE
                                flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
#endif
                                hw_qspi_write8(id, (addr >> 16) & 0xFF);
                                hw_qspi_write16(id, (uint16_t)__REV16(addr));
                        } else {
                               hw_qspi_write32(id, QSPI_GET_DEVICE_PARAM(idx, page_program_opcode) |
                                                                       (__REV(addr) & 0xFFFFFF00));
#if QUAD_MODE
                               flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
#endif
                        }
                }
        }

        if (odd) {
                odd = 4 - odd;
                for (i = 0; i < odd && i < size; ++i) {
                        hw_qspi_write8(id, buf[i]);
                }
        }

        size_aligned32 = ((size - i) & ~0x3);

        if (size_aligned32) {
                fast_write_to_fifo32((uint32_t)(buf + i), (uint32_t)(buf + i + size_aligned32),
                        (uint32_t)&(id->QSPIC_WRITEDATA_REG));
                i += size_aligned32;
        }

        for (; i < size; i++) {
                hw_qspi_write8(id, buf[i]);
        }

        hw_qspi_cs_disable(id);

        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_PAGE_PROG);

#if QUAD_MODE
        if (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), qpi_mode) == false) { // QUAD mode
                flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);
        }
#endif

        return i;
}

#if !ERASE_IN_AUTOMODE
/**
 * \brief Erase a sector of the Flash
 *
 * \param[in] addr The address of the sector to be erased.
 *
 * \note This function blocks until the Flash has processed the command.
 */
__RETAINED_CODE __UNUSED static void flash_erase_sector(uint32_t addr)
{
        HW_QSPIC_ID id = flash_get_addr_id(addr, FLASH_SECTOR_SIZE);
        uint8_t idx __UNUSED = QSPI_GET_CONFIG_IDX(id);
        addr = flash_get_zero_based_addr(addr);

        flash_write_enable(id);

        if (QSPI_GET_DEVICE_PARAM(idx, address_size) == HW_QSPI_ADDR_SIZE_32) {
                hw_qspi_cs_enable(id);
                hw_qspi_write8(id, QSPI_GET_DEVICE_PARAM(idx, erase_opcode));
                hw_qspi_write32(id, __REV(addr));
                hw_qspi_cs_disable(id);
        }
        else {
                hw_qspi_cs_enable(id);
                hw_qspi_write32(id, QSPI_GET_DEVICE_PARAM(idx, erase_opcode) |
                                                        (__REV(addr) & 0xFFFFFF00));
                hw_qspi_cs_disable(id);
        }

        /* Wait for the Flash to process the command */
        while (flash_erase_program_in_progress(id));
}
#endif /* !ERASE_IN_AUTOMODE */

/**
 * \brief Check if the Flash can accept commands
 *
 * \param[in] id QSPI controller id
 * \return bool True if the Flash is not busy else false.
 *
 */
__RETAINED_CODE static bool flash_writable(HW_QSPIC_ID id)
{
        bool writable;

        /*
         * From now on QSPI may not be available, turn off interrupts.
         */
        GLOBAL_INT_DISABLE();

        /*
         * Turn on command entry mode.
         */
        qspi_int_activate_command_entry_mode(id);

        /*
         * Check if flash is ready.
         */
        writable = !(flash_is_busy(id));

        /*
         * Restore auto mode.
         */
        qspi_int_deactivate_command_entry_mode(id);

        /*
         * Let other code to be executed including QSPI one.
         */
        GLOBAL_INT_RESTORE();

        return writable;
}

__RETAINED_CODE void qspi_int_activate_command_entry_mode(HW_QSPIC_ID id)
{
        /*
         * Turn off auto mode to allow write.
         */
        flash_set_automode(id, false);

        /*
         * Switch to single mode for command entry.
         */
        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);

        /*
         * Exit continuous mode, after this the flash will interpret commands again.
         */
        if (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), send_once) != 0) {
                flash_reset_continuous_mode(id, QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), break_seq_size));
        }
}

__RETAINED_CODE void qspi_int_deactivate_command_entry_mode(HW_QSPIC_ID id)
{
        flash_enter_qpi_mode(id);

#if QUAD_MODE
        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
#endif
        flash_set_automode(id, true);
}


#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */


#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)

#if (ERASE_IN_AUTOMODE == 1)
/**
 * \brief Erase sector
 *
 * \details This function will execute a Flash sector erase operation. The operation will either be
 *        carried out immediately (dg_configDISABLE_BACKGROUND_FLASH_OPS is set to 1) or it will be
 *        deferred to be executed in the background when the system becomes idle (when
 *        dg_configDISABLE_BACKGROUND_FLASH_OPS is set to 0, default value). In the latter case, the
 *        caller will block until the registered erase operation is executed.
 *
 * \param[in] addr The address of the sector to be erased.
 */
__RETAINED_CODE static void qspi_erase_sector(uint32_t addr)
{
        HW_QSPIC_ID id = flash_get_addr_id(addr, FLASH_SECTOR_SIZE);

        uint32_t zero_base_addr = flash_get_zero_based_addr(addr);

        hw_qspi_erase_block(id, zero_base_addr);
}
#endif /* (ERASE_IN_AUTOMODE == 1)  */

/**
 * \brief Erase a sector of the Flash
 *
 * \details The time and the way that the operation will be carried out depends on the following
 *        settings:
 *        ERASE_IN_AUTOMODE = 0: the command is issued immediately in manual mode
 *        ERASE_IN_AUTOMODE = 1:
 *              dg_configDISABLE_BACKGROUND_FLASH_OPS = 0: the operation is executed manually in the
 *                      background when the system becomes idle
 *              dg_configDISABLE_BACKGROUND_FLASH_OPS = 1: the operation is executed automatically
 *                      by the QSPI controller.
 *
 * \param[in] addr The address of the sector to be erased.
 */
__RETAINED_CODE static void erase_sector(uint32_t addr)
{
        HW_QSPIC_ID id = flash_get_addr_id(addr, FLASH_SECTOR_SIZE);
#if ERASE_IN_AUTOMODE
        /*
         * Erase sector in automode
         */
        qspi_erase_sector(addr);

        /*
         * Wait for erase to finish
         */
        while (hw_qspi_get_erase_status(id) != HW_QSPI_ERS_NO) {
        }
#else
        /*
         * From now on QSPI may not be available, turn off interrupts.
         */
        GLOBAL_INT_DISABLE();

        /*
         * Turn off auto mode to allow write.
         */
        flash_set_automode(id, false);

        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);

        /*
         * Exit continuous mode, after this the flash will interpret commands again.
         */
        flash_reset_continuous_mode(id, QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), break_seq_size));

        flash_enter_qpi_mode(id);

        /*
         * Execute erase command.
         */
        flash_erase_sector(addr);

        /*
         * Restore auto mode.
         */
        qspi_int_deactivate_command_entry_mode(id);

        /*
         * Let other code to be executed including QSPI one.
         */
        GLOBAL_INT_RESTORE();
#endif
}

__RETAINED_CODE static uint32_t write_page(HW_QSPIC_ID id, uint32_t addr, const uint8_t *buf, uint32_t size)
{
        uint32_t written;

        /*
        * From now on QSPI may not be available, turn off interrupts.
        */
        GLOBAL_INT_DISABLE();

        /*
        * Turn on command entry mode.
        */
        qspi_int_activate_command_entry_mode(id);

        /*
        * Write data into the page of the Flash.
        */
        written = flash_write_page(addr, buf, size);

        /* Wait for the Flash to process the command */
        while (flash_erase_program_in_progress(id));

        /*
        * Restore auto mode.
        */
        qspi_int_deactivate_command_entry_mode(id);

        /*
        * Let other code to be executed including QSPI one.
        */
        GLOBAL_INT_RESTORE();

        return written;
}

uint32_t qspi_automode_write_flash_page(uint32_t addr, const uint8_t *buf, uint32_t size)
{
        ASSERT_WARNING(size > 0);

        uint32_t written;

        HW_QSPIC_ID id = flash_get_addr_id(addr, size);

        while (!flash_writable(id)) {
        }

        written = write_page(id, addr, buf, size);

        return written;
}

void qspi_automode_erase_flash_sector(uint32_t addr)
{
        HW_QSPIC_ID id = flash_get_addr_id(addr, FLASH_SECTOR_SIZE);

        while (!flash_writable(id)) {
        }

        erase_sector(addr);
}

void qspi_automode_erase_chip(void)
{
        for (int idx = 0;  idx < QSPI_CONTROLLER_SUPPORT_NUM; idx++) {
                qspi_automode_erase_chip_by_id(QSPI_GET_CONFIG_BASE_REG(idx));
        }
}

bool qspi_automode_erase_chip_by_id(HW_QSPIC_ID id)
{
        if (qspi_is_device_present[QSPI_GET_CONFIG_IDX(id)] == false) {
                return false;
        }

        if (qspi_is_ram_device(id) == true) {
                return false;
        }

        /* Proceed to erase the flash device */

        qspi_int_activate_command_entry_mode(id);

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, CMD_WRITE_ENABLE);
        hw_qspi_cs_disable(id);

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, CMD_CHIP_ERASE);
        hw_qspi_cs_disable(id);

        hw_qspi_cs_enable(id);
        hw_qspi_write8(id, CMD_READ_STATUS_REGISTER);
        while (hw_qspi_read8(id) & FLASH_STATUS_BUSY_MASK);
        hw_qspi_cs_disable(id);

        qspi_int_deactivate_command_entry_mode(id);

        return true;
}

uint32_t qspi_automode_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
        memcpy(buf, qspi_automode_addr(addr), len);
        return len;
}

const void *qspi_automode_addr(uint32_t addr)
{
#if (dg_configUSE_HW_QSPI == 1)
        HW_QSPIC_ID id = flash_get_addr_id(addr, 1);
#endif
        addr = flash_get_zero_based_addr(addr);

#if (dg_configUSE_HW_QSPI == 1)
        if (id == HW_QSPIC) {
                return (const void *) (MEMORY_QSPIC_BASE + addr);
        }
#endif /* dg_configUSE_HW_QSPI */

#if (dg_configUSE_HW_QSPI2 == 1)
        return (const void *) (MEMORY_QSPIC2_BASE + addr);
#endif /* dg_configUSE_HW_QSPI2 */

        ASSERT_WARNING(0);

        return (const void *) (MEMORY_QSPIC_BASE + addr);
}

void qspi_save_configuration(uint8_t idx)
{
        HW_QSPIC_ID id = QSPI_GET_CONFIG_BASE_REG(idx);

        qspic_config[idx].ctrlmode_reg  = id->QSPIC_CTRLMODE_REG;
        qspic_config[idx].burstcmda_reg = id->QSPIC_BURSTCMDA_REG;
        qspic_config[idx].burstcmdb_reg = id->QSPIC_BURSTCMDB_REG;
        qspic_config[idx].erasecmda_reg = id->QSPIC_ERASECMDA_REG;
        qspic_config[idx].erasecmdb_reg = id->QSPIC_ERASECMDB_REG;
        qspic_config[idx].statuscmd_reg = id->QSPIC_STATUSCMD_REG;
        qspic_config[idx].gp_reg        = id->QSPIC_GP_REG;
        qspic_config[idx].awritecmd_reg = id->QSPIC_AWRITECMD_REG;
        qspic_config[idx].memblen_reg   = id->QSPIC_MEMBLEN_REG;
}

__RETAINED_CODE void qspi_restore_configuration(uint8_t idx)
{
        HW_QSPIC_ID id = QSPI_GET_CONFIG_BASE_REG(idx);

        id->QSPIC_CTRLMODE_REG  = qspic_config[idx].ctrlmode_reg;
        id->QSPIC_BURSTCMDA_REG = qspic_config[idx].burstcmda_reg;
        id->QSPIC_BURSTCMDB_REG = qspic_config[idx].burstcmdb_reg;
        id->QSPIC_ERASECMDA_REG = qspic_config[idx].erasecmda_reg;
        id->QSPIC_ERASECMDB_REG = qspic_config[idx].erasecmdb_reg;
        id->QSPIC_STATUSCMD_REG = qspic_config[idx].statuscmd_reg;
        id->QSPIC_GP_REG        = qspic_config[idx].gp_reg;
        id->QSPIC_AWRITECMD_REG = qspic_config[idx].awritecmd_reg;
        id->QSPIC_MEMBLEN_REG   = qspic_config[idx].memblen_reg;
}

#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */

__RETAINED_CODE void qspi_automode_flash_power_up(void)
{
#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)

        /* Interrupts must be turned off since the flash goes in manual mode, and
         * code (e.g. for an ISR) cannot be fetched from flash during this time
         */
        GLOBAL_INT_DISABLE();

        uint8_t idx;


        for (idx = 0; idx < QSPI_CONTROLLER_SUPPORT_NUM; idx++) {
                HW_QSPIC_ID id = QSPI_GET_CONFIG_BASE_REG(idx);

                if (qspi_is_device_present[idx]) {
                        hw_qspi_clock_enable(id);
                        qspi_restore_configuration(idx);
                        if (QSPI_GET_DEVICE_PARAM(idx, is_ram)) {
                                qspi_int_deactivate_command_entry_mode(id);
                        } else {
                                if (dg_configFLASH_POWER_DOWN == 1) {
                                        hw_clk_delay_usec(QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id),
                                                power_down_delay));
                                        /*
                                         * Do not call qspi_int_activate_command_entry_mode().
                                         * This function will try to send break sequence to the
                                         * QSPI Flash which is in power-down mode.
                                         */
                                        flash_set_automode(id, false);
                                        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);

                                        hw_qspi_cs_enable(id);
                                        hw_qspi_write8(id, CMD_RELEASE_POWER_DOWN);
                                        hw_qspi_cs_disable(id);
                                        qspi_int_deactivate_command_entry_mode(id);
                                        hw_clk_delay_usec(QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id),
                                                release_power_down_delay));
                                } else if (hw_qspi_is_init_enabled(HW_QSPIC) == false) {
                                        /*
                                         * Flash is never initialized by the QSPI controller so
                                         * execute the initialization
                                         *
                                         * Note: If flash is initialized by the QSPI controller, it
                                         * will power up (and consume power) every time the system wakes up.
                                         * In case system wakes up by a master which does not use QSPI (e.g SNC),
                                         * power will be consumed for no reason.
                                         *
                                         */
                                        if (dg_configFLASH_POWER_OFF == 1) {
                                                hw_clk_delay_usec(QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id),
                                                        power_up_delay));

                                                qspi_int_activate_command_entry_mode(id);
                                                QSPI_GET_DEVICE_PARAM(idx, initialize)(id);
                                                qspi_int_deactivate_command_entry_mode(id);
                                        } else {
                                                qspi_int_deactivate_command_entry_mode(id);
                                        }
                                }
                        }
                }
        }

        /*
         * The flash is in auto mode again. Re-enable the interrupts
         */
        GLOBAL_INT_RESTORE();
#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */
}

__RETAINED_CODE void qspi_automode_flash_power_down(void)
{
#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)
        for (uint8_t idx = 0; idx < QSPI_CONTROLLER_SUPPORT_NUM; idx++) {
                HW_QSPIC_ID id = QSPI_GET_CONFIG_BASE_REG(idx);

                if (qspi_is_device_present[idx] &&
                    (QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), is_ram) == false)) {
                        /*
                         * Set QSPIC to single mode, disable QSPIC auto mode and
                         * disable flash device's continuous mode (some flash devices
                         * cannot enter standby correctly if continuous read mode is
                         * enabled).
                         */
                        qspi_int_activate_command_entry_mode(id);
                        if (dg_configFLASH_POWER_DOWN == 1) {
                                hw_qspi_cs_enable(id);
                                hw_qspi_write8(id, CMD_ENTER_POWER_DOWN);
                                hw_qspi_cs_disable(id);
                        }
                }
        }

        // Disable QSPI clock to save power
# if (dg_configUSE_HW_QSPI == 1)
        hw_qspi_clock_disable(HW_QSPIC);
# endif
# if (dg_configUSE_HW_QSPI2 == 1)
        hw_qspi_clock_disable(HW_QSPIC2);
# endif


#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */
}

#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)

__RETAINED_CODE void qspi_int_reset_device(HW_QSPIC_ID id)
{
        /*
         * If we initialize rst_cmd during declaration (e.g uint8_t rst_cmd[2] = { 0x66, 0x99 };),
         * compiler adds it to .rodata (flash). We need to run qspi_int_reset_device() from RAM
         * so we declare it without initialization and fill it later.
         */
        uint8_t rst_cmd[2];
        uint8_t power_up_cmd = CMD_RELEASE_POWER_DOWN;

        /* Release the FLASH/RAM from Power-Down mode to enable it to accept commands */
        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);
        flash_write(id, &power_up_cmd, 1);

        /* Reset continuous mode using both one and two break bytes to cover all cases */
        flash_reset_continuous_mode(id, HW_QSPI_BREAK_SEQ_SIZE_1B);
        flash_reset_continuous_mode(id, HW_QSPI_BREAK_SEQ_SIZE_2B);

        /* Reset QSPI FLASH/RAM in SINGLE mode */
        rst_cmd[0] = 0x66;
        rst_cmd[1] = 0x99;
        flash_write(id, &rst_cmd[0], 1);
        flash_write(id, &rst_cmd[1], 1);

        /* If the QSPI FLASH/RAM was in quad mode already it might have ignored the reset
         * in single mode above.
         * So Reset device in QUAD mode again to get it out of QPI mode.
         * If the FLASH/RAM already reset in single mode, it will ignore this step */
        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_QUAD);
        rst_cmd[0] = 0x66;
        rst_cmd[1] = 0x99;
        flash_write(id, &rst_cmd[0], 1);
        flash_write(id, &rst_cmd[1], 1);

        /* Go back to single mode. There will be commands first after this point */
        flash_set_bus_mode(id, HW_QSPI_BUS_MODE_SINGLE);

        /* wait for the FLASH/RAM reset to complete before proceeding */
#if (FLASH_AUTODETECT == 1)
        hw_clk_delay_usec(dg_configFLASH_AUTODETECT_RESET_DELAY);
#else
        hw_clk_delay_usec(QSPI_GET_DEVICE_PARAM(id, reset_delay_us));
#endif
}

__RETAINED_CODE static bool qspi_read_flash_jedec_id_cmd(HW_QSPIC_ID id, uint8_t *manufacturer_id,
                                                         uint8_t *device_type, uint8_t *density,
                                                         bool alt_method)
{
        uint8_t cmd[] = { CMD_READ_JEDEC_ID, 0, 0, 0 };
        uint8_t buffer[3];
        bool found = false;

        hw_qspi_set_access_mode(id, HW_QSPI_ACCESS_MODE_MANUAL);

        qspi_int_reset_device(id);

        if (alt_method == false) {
                // Standard read JEDEC device ID command
                flash_transact(id, cmd, 1, buffer, 3);
        } else {
                // Alternative read JEDEC device ID command
                flash_transact(id, cmd, 4, buffer, 3);
        }

        found = ((buffer[0] != 0xFF) && (buffer[0] != 0));

        if (found) {
                *manufacturer_id = buffer[0];
                *device_type = buffer[1];
                *density = buffer[2];
        }

        hw_qspi_set_access_mode(id, HW_QSPI_ACCESS_MODE_AUTO);

        return found;
}

__RETAINED_CODE bool qspi_read_flash_jedec_id(HW_QSPIC_ID id, uint8_t *manufacturer_id,
                                                        uint8_t *device_type, uint8_t *density)
{
        bool found;
        found = qspi_read_flash_jedec_id_cmd(id, manufacturer_id, device_type, density, false);
        if (found) {
                return true;
        }
        return qspi_read_flash_jedec_id_cmd(id, manufacturer_id, device_type, density, true);
}

#if (FLASH_AUTODETECT == 1)
__RETAINED_CODE static const qspi_flash_config_t * qspi_get_flash_config_by_jedec_id(uint8_t manufacturer_id,
                                                        uint8_t device_type, uint8_t device_density)
{
        for (uint8_t i = 0; i < ARRAY_LENGTH(qspi_memory_config_table); ++i) {
                uint8_t density_mask = QSPI_GET_DENSITY_MASK(qspi_memory_config_table[i]->device_density);

                if ( (qspi_memory_config_table[i]->manufacturer_id == manufacturer_id) &&
                     (qspi_memory_config_table[i]->device_type == device_type) &&
                     (QSPI_GET_DENSITY(qspi_memory_config_table[i]->device_density) ==
                      (device_density & density_mask))) {
                        return qspi_memory_config_table[i];
                }
        }

        return NULL;
}
#endif /* FLASH_AUTODETECT */


__STATIC_FORCEINLINE void set_read_pipe_delay(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
        uint8_t read_pipe_delay = sys_clk > sysclk_XTAL32M ? QSPI_READ_PIPE_DELAY_1V2 : QSPI_READ_PIPE_DELAY_0V9;

        hw_qspi_set_read_pipe_clock_delay(id, read_pipe_delay);
        hw_qspi_read_pipe_enable(id);
}

__RETAINED_CODE static void init_hw_qspi(HW_QSPIC_ID id)
{
        uint8_t idx __UNUSED = QSPI_GET_CONFIG_IDX(id);

        const qspi_config qspi_cfg = {
                QSPI_GET_DEVICE_PARAM(idx, address_size),
                __HW_QSPI_INIT_POL,
                HW_QSPI_SAMPLING_EDGE_POSITIVE
        };

        hw_qspi_cs_enable(id);
        hw_qspi_cs_disable(id);
        hw_qspi_init(id, &qspi_cfg);
        hw_qspi_set_div(id, HW_QSPI_DIV_1);

        sys_clk_t sys_clk = hw_clk_get_system_clock();

        set_read_pipe_delay(id, sys_clk);
}

__STATIC_FORCEINLINE void calculate_cs_idle_clk_cycles(const HW_QSPIC_ID id, sys_clk_t sys_clk,
                                                       uint8_t *read_cs_idle_clk_cycles,
                                                       uint8_t *erase_cs_idle_clk_cycles)
{
        uint32_t sys_clk_freq_hz = hw_clk_calculate_sys_clk_freq(sys_clk);
        uint8_t read_cs_idle_delay = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), read_cs_idle_delay_ns);
        uint8_t erase_cs_idle_delay = QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), erase_cs_idle_delay_ns);
        *read_cs_idle_clk_cycles = NSEC_TO_CLK_CYCLES(read_cs_idle_delay, sys_clk_freq_hz);
        *erase_cs_idle_clk_cycles = NSEC_TO_CLK_CYCLES(erase_cs_idle_delay, sys_clk_freq_hz);
}

__STATIC_FORCEINLINE void set_cs_idle_delays(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
        uint8_t read_cs_idle_clk_cycles = 0;
        uint8_t erase_cs_idle_clk_cycles = 0;

        calculate_cs_idle_clk_cycles(id, sys_clk, &read_cs_idle_clk_cycles, &erase_cs_idle_clk_cycles);
        hw_qspi_set_min_cs_high(id, read_cs_idle_clk_cycles);
        hw_qspi_set_min_erase_cs_high(id, erase_cs_idle_clk_cycles);
}

#if dg_configUSE_HW_QSPI2
static void psram_set_cs_active_max(HW_QSPIC_ID id, sys_clk_t sys_clk, uint32_t cs_active_time_max_us)
{
        uint32_t sys_clk_freq = hw_clk_calculate_sys_clk_freq(sys_clk);
        uint32_t qspi_clk_div = 1 << (uint32_t) hw_qspi_get_div(id);
        uint32_t cs_active_max_cycles = cs_active_time_max_us * (sys_clk_freq / (1000000 * qspi_clk_div));

        hw_qspi_set_tCEM(id, (uint16_t) cs_active_max_cycles);
}
#endif

__RETAINED_CODE void qspi_int_configure(HW_QSPIC_ID id)
{
        struct qspic_instructions qspi_init_config;
        uint8_t idx __UNUSED = QSPI_GET_CONFIG_IDX(id);
        HW_QSPI_BUS_MODE mode;

        sys_clk_t sys_clk = hw_clk_get_system_clock();
        uint8_t read_cs_hi_clk_cycles = 0;
        uint8_t erase_cs_hi_clk_cycles = 0;

        calculate_cs_idle_clk_cycles(id, sys_clk, &read_cs_hi_clk_cycles, &erase_cs_hi_clk_cycles);

        if (!qspi_is_device_present[idx]) {
                return;
        }

        mode = QSPI_GET_DEVICE_PARAM(idx, qpi_mode) ? HW_QSPI_BUS_MODE_QUAD : HW_QSPI_BUS_MODE_SINGLE;

        qspi_init_config.set_read_instruction = true;
        qspi_init_config.read_instruction.inst = QSPI_GET_DEVICE_PARAM(idx, fast_read_opcode);
        qspi_init_config.read_instruction.inst_mode = QSPI_GET_DEVICE_PARAM(idx, send_once);
        qspi_init_config.read_instruction.dummy_count = QSPI_GET_DEVICE_PARAM(idx, get_dummy_bytes)(id, sys_clk);
        qspi_init_config.read_instruction.inst_phase = mode;
        qspi_init_config.read_instruction.addr_phase = HW_QSPI_BUS_MODE_QUAD;
        qspi_init_config.read_instruction.dummy_phase = HW_QSPI_BUS_MODE_QUAD;
        qspi_init_config.read_instruction.data_phase = HW_QSPI_BUS_MODE_QUAD;
        qspi_init_config.read_instruction.read_cs_hi_cycles = read_cs_hi_clk_cycles;
        /* Setup instruction that will be used to periodically check erase operation status.
         * Check LSB which is 1 when erase is in progress. */
        qspi_init_config.set_read_status_instruction = true;
        qspi_init_config.read_status_instruction.inst = QSPI_GET_DEVICE_PARAM(idx, read_erase_progress_opcode);
        qspi_init_config.read_status_instruction.inst_phase = mode;
        qspi_init_config.read_status_instruction.receive_phase = mode;
        qspi_init_config.read_status_instruction.busy_pos = QSPI_GET_DEVICE_PARAM(idx, erase_in_progress_bit);
        qspi_init_config.read_status_instruction.busy_val = QSPI_GET_DEVICE_PARAM(idx, erase_in_progress_bit_high_level) ? 1 : 0;
        qspi_init_config.read_status_instruction.read_delay = 20;
        qspi_init_config.read_status_instruction.delay_sel = 0;
        /* Setup erase instruction that will be sent by QSPI controller to erase sector in automode. */
        qspi_init_config.set_erase_instruction = true;
        qspi_init_config.erase_instruction.inst = QSPI_GET_DEVICE_PARAM(idx, erase_opcode);
        qspi_init_config.erase_instruction.inst_phase = mode;
        qspi_init_config.erase_instruction.addr_phase = mode;
        qspi_init_config.erase_instruction.hclk_cycles = 15;
        qspi_init_config.erase_instruction.erase_cs_hi_cycles = erase_cs_hi_clk_cycles;
        /* QSPI controller must send write enable before erase, this sets it up. */
        qspi_init_config.set_write_enable_instruction = true;
        qspi_init_config.write_enable_instruction.inst = CMD_WRITE_ENABLE;
        qspi_init_config.write_enable_instruction.inst_phase = mode;
        /* Setup instruction pair that will temporarily suspend erase operation to allow read. */
        qspi_init_config.set_suspend_resume_instruction = true;
        qspi_init_config.suspend_resume_instruction.erase_suspend_inst = QSPI_GET_DEVICE_PARAM(idx, erase_suspend_opcode);
        qspi_init_config.suspend_resume_instruction.suspend_inst_phase = mode;
        qspi_init_config.suspend_resume_instruction.erase_resume_inst = QSPI_GET_DEVICE_PARAM(idx, erase_resume_opcode);
        qspi_init_config.suspend_resume_instruction.resume_inst_phase = mode;
        qspi_init_config.suspend_resume_instruction.minimum_delay = 7;

        // The QSPIC write instruction in auto access mode is only in scope for PSRAM memories.
        if (QSPI_GET_DEVICE_PARAM(idx, is_ram)) {
                qspi_init_config.set_write_instruction = true;
                qspi_init_config.write_instruction.inst = QSPI_GET_DEVICE_PARAM(idx, page_program_opcode);
                qspi_init_config.write_instruction.inst_phase = mode;
                qspi_init_config.write_instruction.addr_phase = HW_QSPI_BUS_MODE_QUAD;
                qspi_init_config.write_instruction.data_phase = HW_QSPI_BUS_MODE_QUAD;
        // The flash memories are written in manual access mode, thus the QSPIC write instruction
        // in auto access mode should be disabled.
        } else {
                qspi_init_config.set_write_instruction = false;
        }

        qspi_init_config.set_wrapping_burst_instruction = false;

        if (QSPI_GET_DEVICE_PARAM(idx, is_ram) == false) {
                /*
                 * This sequence is necessary if flash is working in continuous read mode, when instruction
                 * is not sent on every read access just address. Sending 0xFFFF will exit this mode.
                 * This sequence is sent only when QSPI is working in automode and decides to send one of
                 * instructions above.
                 * If flash is working in DUAL bus mode sequence should be 0xFFFF and size should be
                 * HW_QSPI_BREAK_SEQ_SIZE_2B.
                 */
                 hw_qspi_burst_break_sequence_enable(id, 0xFFFF, HW_QSPI_BUS_MODE_SINGLE,
                                                     QSPI_GET_DEVICE_PARAM(idx, break_seq_size), 0);
        }

        hw_qspi_set_instructions(id, &qspi_init_config);

#if (dg_configUSE_HW_QSPI2 == 1)
        if (QSPI_GET_DEVICE_PARAM(idx, is_ram)) {
                hw_qspi_set_sram_mode(HW_QSPIC2, true);
                hw_qspi_set_cs_mode(HW_QSPIC2, 0);

                 if (QSPI_GET_DEVICE_PARAM(idx, cs_active_time_max_us) != 0) {
                        hw_qspi_enable_tCEM(HW_QSPIC2);
                        psram_set_cs_active_max(id, sys_clk, QSPI_GET_DEVICE_PARAM(idx, cs_active_time_max_us));
                }
                hw_qspi_set_burst_length(HW_QSPIC2, QSPI_GET_DEVICE_PARAM(idx, burst_len));
        }
#endif

        hw_qspi_set_extra_byte(id, QSPI_GET_DEVICE_PARAM(idx, extra_byte), HW_QSPI_BUS_MODE_QUAD, 0);
        hw_qspi_set_address_size(id, QSPI_GET_DEVICE_PARAM(idx, address_size));

       	set_cs_idle_delays(id, sys_clk);
}

__STATIC_FORCEINLINE __UNUSED bool qspi_density_is_equal(uint16_t conf_density_with_mask, uint8_t jedec_density)
{
        uint8_t density_mask = (uint8_t)QSPI_GET_DENSITY_MASK(conf_density_with_mask);

        return (((uint8_t)QSPI_GET_DENSITY(conf_density_with_mask)) == (jedec_density & density_mask));
}
#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */

__RETAINED_CODE bool qspi_automode_init(void)
{
#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)

#if (dg_configUSE_HW_QSPI == 1)
        uint8_t qspi_control_idx = 0;
#else
        uint8_t qspi_control_idx = 1;
#endif


        hw_pd_power_up_ctrl();

#if (dg_configUSE_HW_QSPI == 1)
        // Initialize QSPIC
        init_hw_qspi(HW_QSPIC);
        hw_qspi_set_access_mode(HW_QSPIC, HW_QSPI_ACCESS_MODE_AUTO);
#endif /* (dg_configUSE_HW_QSPI == 1) */


#if (dg_configUSE_HW_QSPI2 == 1)
        // Initialize QSPIC2
        init_hw_qspi(HW_QSPIC2);
        hw_qspi_set_access_mode(HW_QSPIC2, HW_QSPI_ACCESS_MODE_AUTO);
#endif

#if FLASH_AUTODETECT
        bool autodetect[2] = { false, false };
#endif

#if FLASH1_AUTODETECT
        autodetect[0] = true;
#endif
#if QSPIC2_DEV_AUTODETECT
        autodetect[1] = true;
#endif

        do {
                HW_QSPIC_ID id = QSPI_GET_CONFIG_BASE_REG(qspi_control_idx);

#if __DBG_QSPI_ENABLED
#       if (dg_configUSE_HW_QSPI == 1)
                REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPI_DIV, 3);
#       endif
#       if (dg_configUSE_HW_QSPI2 == 1)
                REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPI2_DIV, 3);
#       endif
#endif

#if (FLASH_AUTODETECT) || (dg_configFLASH_CONFIG_VERIFY)
                uint8_t device_type;
                uint8_t device_density;
                uint8_t manufacturer_id;
#endif
#if (FLASH_AUTODETECT)
                const qspi_flash_config_t *flash_config_init = NULL;

                if (autodetect[qspi_control_idx]) {
                        qspi_is_device_present[qspi_control_idx] = qspi_read_flash_jedec_id_cmd(id, &manufacturer_id, &device_type, &device_density, false);

                        if (qspi_is_device_present[qspi_control_idx] == true) {
                                flash_config_init = qspi_get_flash_config_by_jedec_id(manufacturer_id,
                                                                          device_type, device_density);
                        }

                        if (!flash_config_init) {
                                qspi_is_device_present[qspi_control_idx] = qspi_read_flash_jedec_id_cmd(id, &manufacturer_id, &device_type, &device_density, true);
                                if (qspi_is_device_present[qspi_control_idx] == false) {
                                        qspi_control_idx++;
                                        continue;
                                }

                                flash_config_init = qspi_get_flash_config_by_jedec_id(manufacturer_id,
                                                                        device_type, device_density);

                                if (!flash_config_init) {
                                        /*
                                         * QSPI controller doesn't have any known device connected.
                                         * Mark QSPI device as not existing
                                         */
                                        qspi_is_device_present[qspi_control_idx] = false;
                                }
                        }
                }
                else
                {
                        if (id == HW_QSPIC) {
                                qspi_is_device_present[qspi_control_idx] = (dg_configUSE_HW_QSPI == 1);
#if (dg_configUSE_HW_QSPI == 1) && (dg_configFLASH_AUTODETECT == 0)
                                flash_config_init = &dg_configFLASH_CONFIG;
#endif
                        }
                        else {
                                qspi_is_device_present[qspi_control_idx] = (dg_configUSE_HW_QSPI2 == 1);
#if (dg_configUSE_HW_QSPI2 == 1) && (dg_configQSPIC2_DEV_AUTODETECT == 0)
                                flash_config_init = &dg_configQSPIC2_DEV_CONFIG;
#endif
                        }
                }
#else
#if dg_configFLASH_CONFIG_VERIFY
                if (qspi_read_flash_jedec_id(id, &manufacturer_id, &device_type, &device_density) &&
                        (manufacturer_id == QSPI_GET_DEVICE_PARAM(qspi_control_idx, manufacturer_id)) &&
                        (device_type == QSPI_GET_DEVICE_PARAM(qspi_control_idx, device_type)) &&
                        (qspi_density_is_equal(QSPI_GET_DEVICE_PARAM(qspi_control_idx, device_density), device_density)))
                {
                        qspi_is_device_present[qspi_control_idx] = true;
                } else {
                        qspi_is_device_present[qspi_control_idx] = false;
                }
#else
                if (id == HW_QSPIC) {
                        qspi_is_device_present[qspi_control_idx] = (dg_configUSE_HW_QSPI == 1);
                } else {
                        qspi_is_device_present[qspi_control_idx] = (dg_configUSE_HW_QSPI2 == 1);
                }
#endif
#endif
                if (qspi_is_device_present[qspi_control_idx] == false) {
                        qspi_control_idx++;
                        continue;
                }


#if FLASH_AUTODETECT
                ASSERT_WARNING(flash_config_init != NULL);
                /*
                 * Copy the selected flash struct from flash into retram
                 */
                memcpy(&flash_config[qspi_control_idx], flash_config_init, sizeof(qspi_flash_config_t));
#endif
                // Only QSPIC2 supports QSPI RAM devices
                ASSERT_WARNING(qspi_control_idx != 0 || QSPI_GET_DEVICE_PARAM(qspi_control_idx, is_ram) == false);

                // Copy the structure to RAM to use it while in command entry mode

                qspi_int_activate_command_entry_mode(id);

                QSPI_GET_DEVICE_PARAM(qspi_control_idx, initialize)(id);

                qspi_int_configure(id);

                qspi_int_deactivate_command_entry_mode(id);

                qspi_save_configuration(qspi_control_idx);

                qspi_control_idx++;
        } while (qspi_control_idx < QSPI_CONTROLLER_SUPPORT_NUM);


#if (dg_configUSE_HW_QSPI == 1)
        // Disable QSPIC1 clock is not used
        if (qspi_is_device_present[0] == false) {
                hw_qspi_clock_disable(HW_QSPIC);
        }
#endif

#if (dg_configUSE_HW_QSPI2 == 1)
        // Disable QSPIC2 clock is not used
        if (qspi_is_device_present[1] == false) {
                hw_qspi_clock_disable(HW_QSPIC2);
        }
#endif
        return true;
#else
        return false;
#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */
}

__RETAINED_CODE void qspi_automode_sys_clock_cfg(sys_clk_t sys_clk)
{
#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)
        uint8_t idx = 0;

        do {
                if (qspi_is_device_present[idx] == false) {
                        idx++;
                        continue;
                }

                hw_qspi_set_div(QSPI_GET_CONFIG_BASE_REG(idx),
                                sys_clk == sysclk_PLL160 ? HW_QSPI_DIV_2 : HW_QSPI_DIV_1);
                // The sys_clk_cfg() might put the flash in command entry mode, where the flash is
                // not available for code execution, therefore the global interrupts must be disabled.
                GLOBAL_INT_DISABLE();
                QSPI_GET_DEVICE_PARAM(idx, sys_clk_cfg)(QSPI_GET_CONFIG_BASE_REG(idx), sys_clk);
                GLOBAL_INT_RESTORE();

                set_read_pipe_delay(QSPI_GET_CONFIG_BASE_REG(idx), sys_clk);
                set_cs_idle_delays(QSPI_GET_CONFIG_BASE_REG(idx), sys_clk);

                idx++;
        } while (idx < QSPI_CONTROLLER_SUPPORT_NUM);
#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */
}

#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)


__RETAINED_CODE bool qspi_is_ram_device(const HW_QSPIC_ID id)
{
        return QSPI_GET_DEVICE_PARAM(QSPI_GET_CONFIG_IDX(id), is_ram);
}

#endif /* (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1) */
