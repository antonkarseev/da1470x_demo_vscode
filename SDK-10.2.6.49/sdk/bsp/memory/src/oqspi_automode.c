
/**
 ****************************************************************************************
 *
 * @file oqspi_automode.c
 *
 * @brief OQSPI flash memory automode API source file.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_OQSPI

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sdk_defs.h"

#include "hw_clk.h"
#include "hw_pmu.h"

#include "hw_oqspi.h"
#include "hw_cache.h"
#include "oqspi_automode.h"
#include "oqspi_automode_internal.h"

__RETAINED_RW HW_OQSPI_BUS_MODE manual_access_bus_mode = HW_OQSPI_BUS_MODE_SINGLE;

__RETAINED_CODE static void oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE bus_mode, bool forced);
__RETAINED_CODE static void oqspi_flash_write_enable(HW_OQSPI_BUS_MODE bus_mode);
__RETAINED_CODE static bool oqspi_flash_is_busy(HW_OQSPI_BUS_MODE bus_mode);
__RETAINED_CODE static void oqspi_enter_manual_access_mode(void);
__RETAINED_CODE static void oqspi_flash_write_enable(HW_OQSPI_BUS_MODE bus_mode);
__UNUSED __RETAINED_CODE static void oqspi_enter_qpi_mode(void);
__UNUSED __RETAINED_CODE static bool oqspi_exit_qpi(void);

/*
 * OQSPI controller allows to execute code directly from OQSPI flash.
 * When code is executing from flash there is no possibility to reprogram it.
 * To be able to modify flash memory while it is used for code execution it must me assured that
 * during the time needed for erase/write no code is running from flash.
 */

#define READ_PIPE_DELAY_0V9             (HW_OQSPI_READ_PIPE_DELAY_0)
#define READ_PIPE_DELAY_1V2             (HW_OQSPI_READ_PIPE_DELAY_7)

#if (dg_configOQSPI_FLASH_AUTODETECT == 0)
        #if !defined(dg_configOQSPI_FLASH_CONFIG)
        #error Please define dg_configOQSPI_FLASH_CONFIG !!!
        #endif
#endif

#if dg_configOQSPI_FLASH_AUTODETECT
# include dg_configOQSPI_MEMORY_CONFIG_TABLE_HEADER

__RETAINED oqspi_flash_config_t oqspi_flash_config;

#else /* dg_configOQSPI_FLASH_AUTODETECT */
# ifndef dg_configOQSPI_FLASH_HEADER_FILE
# error dg_configOQSPI_FLASH_HEADER_FILE must be defined !!!
# endif
#include dg_configOQSPI_FLASH_HEADER_FILE
#endif /* dg_configOQSPI_FLASH_AUTODETECT */

#if dg_configOQSPI_FLASH_AUTODETECT
# define OQSPI_GET_DEVICE_PARAM(param) (oqspi_flash_config.param)
#else
# define OQSPI_GET_DEVICE_PARAM(param) (dg_configOQSPI_FLASH_CONFIG.param)
#endif

/**
 * The read pipe clock delay depends on the voltage level of the 1V2 power rail.
 * According to the hw specifications the optimal settings are:
 * - POWER_RAIL_1V2 = 0V9 --> Read pipe delay = 0
 * - POWER_RAIL_1V2 = 1V2 --> Read pipe delay = 7
 *
 * Moreover, the voltage level of the 1V2 power rail relates to the system clock frequency:
 * - SYS_CLK_FREQ = 32 MHz --> POWER_RAIL_1V2 = 0V9
 * - SYS_CLK_FREQ > 32 MHz --> POWER_RAIL_1V2 = 1V2
 *
 * The read pipe clock delay is set based on the system clock frequency because it is more convenient.
 *
 *  Allowed settings
 * ----------------------------------------------------------------------------------------------|
 * | System clock frequency | OQSPIC divider | 1V2 voltage level | Read pipe delay | Recommended |
 * ----------------------------------------------------------------------------------------------|
 * |        32MHz           |        1       |         0V9       |        0        |      Y      |
 * |        32MHz           |        1       |         1V2       |        0        |      N      |
 * |        32MHz           |        1       |         1V2       |        7        |      N      |
 * |        64MHz           |        1       |         1V2       |        7        |      Y      |
 * |        96MHz           |        1       |         1V2       |        7        |      Y      |
 * |       160MHz           |        2       |         1V2       |        7        |      Y      |
 * ----------------------------------------------------------------------------------------------|
 *
 *  Forbidden settings
 * ---------------------------------------------------------------------------------
 * | System clock frequency | OQSPIC divider | 1V2 voltage level | Read pipe delay |
 * ---------------------------------------------------------------------------------
 * |        32MHz           |        x       |         0V9       |        7        |
 * |  64Mhz/96MHz/160MHz    |        x       |         0V9       |        x        |
 * |  64Mhz/96MHz/160MHz    |        x       |         1V2       |        0        |
 * ---------------------------------------------------------------------------------
 *
 * x: don't care
 */
__RETAINED_CODE static void oqspi_set_read_pipe_clock_delay(sys_clk_t sys_clk)
{
        HW_OQSPI_READ_PIPE_DELAY read_pipe_delay = READ_PIPE_DELAY_0V9;

        if (sys_clk > sysclk_XTAL32M) {
                read_pipe_delay = READ_PIPE_DELAY_1V2;
        }

        hw_oqspi_set_read_pipe_clock_delay(read_pipe_delay);
}

/**
 * \brief Check if the device is busy
 *
 * \return bool True if the BUSY bit is set else false.
 *
 * \warning This function checks the value of the BUSY bit in the Status Register 1 of the Flash. It
 *        is the responsibility of the caller to call the function in the right context. The
 *        function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE static bool oqspi_flash_is_busy(HW_OQSPI_BUS_MODE bus_mode)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUSY_LEVEL busy_level = OQSPI_GET_DEVICE_PARAM(read_status_instr_cfg.busy_level);

        return OQSPI_GET_DEVICE_PARAM(callback.is_busy_cb)(bus_mode, busy_level);
}

/**
 * \brief Read the Status Register 1 of the Flash
 *
 * \return uint8_t The value of the Status Register 1 of the Flash.
 */
__RETAINED_CODE static uint8_t oqspi_flash_read_status_register(HW_OQSPI_BUS_MODE bus_mode)
{
        __DBG_OQSPI_VOLATILE__ uint8_t status;

        status = OQSPI_GET_DEVICE_PARAM(callback.read_status_reg_cb)(bus_mode);

        return status;
}

/**
 * \brief Write the Status Register 1 of the Flash
 *
 * \param[in] value The value to be written.
 *
 * \note This function blocks until the Flash has processed the command. No verification that the
 *        value has been actually written is done though. It is up to the caller to decide whether
 *        such verification is needed or not and execute it on its own.
 */
__RETAINED_CODE __UNUSED static void oqspi_flash_write_status_register(uint8_t value)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE opcode_bus_mode = OQSPI_GET_DEVICE_PARAM(read_status_instr_cfg.opcode_bus_mode);

        OQSPI_GET_DEVICE_PARAM(callback.write_status_reg_cb)(opcode_bus_mode, value);

        /* Wait for the Flash to process the command */
        while (oqspi_flash_is_busy(opcode_bus_mode));
}

/**
 * \brief Write an arbitrary number of bytes to the Flash and then read an arbitrary number of bytes
 *       from the Flash in one transaction
 *
 * \param[in] wbuf Pointer to the beginning of the buffer that contains the data to be written
 * \param[in] wlen The number of bytes to be written
 * \param[in] rbuf Pointer to the beginning of the buffer than the read data are stored
 * \param[in] rlen The number of bytes to be read
 *
 * \note The data are transferred as bytes (8 bits wide). No optimization is done in trying to use
 *       faster access methods (i.e. transfer words instead of bytes whenever it is possible).
 */
__RETAINED_CODE static void oqspi_flash_transact(const uint8_t *wbuf, uint32_t wlen, uint8_t *rbuf,
                                                 uint32_t rlen)
{
        hw_oqspi_cs_enable();

        for (uint32_t i = 0; i < wlen; ++i) {
                hw_oqspi_write8(wbuf[i]);
        }

        for (uint32_t i = 0; i < rlen; ++i) {
                rbuf[i] = hw_oqspi_read8();
        }

        hw_oqspi_cs_disable();
}

/*
 * Send flash command
 */
__RETAINED_CODE static void oqspi_flash_cmd(const uint8_t opcode, HW_OQSPI_BUS_MODE bus_mode)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_OPCODE_LEN opcode_len = OQSPI_GET_DEVICE_PARAM(opcode_len);

        hw_oqspi_cs_enable();

        if (USE_DUAL_BYTE_OPCODE(opcode_len, bus_mode)) {
                hw_oqspi_write16(CONVERT_OPCODE_TO_DUAL_BYTE(opcode));
        } else {
                hw_oqspi_write8(opcode);
        }

        hw_oqspi_cs_disable();
}

/**
 * \brief Set WEL (Write Enable Latch) bit of the Status Register of the Flash
 * \details The WEL bit must be set prior to every Page Program, Quad Page Program, Sector Erase,
 *       Block Erase, Chip Erase, Write Status Register and Erase/Program Security Registers
 *       instruction. In the case of Write Status Register command, any status bits will be written
 *       as non-volatile bits.
 *
 *
 * \note This function blocks until the Flash has processed the command and it will be repeated if,
 *       for any reason, the command was not successfully executed by the Flash.
 */
__RETAINED_CODE static void oqspi_flash_write_enable(HW_OQSPI_BUS_MODE bus_mode)
{
        __DBG_OQSPI_VOLATILE__ uint8_t status;
        __DBG_OQSPI_VOLATILE__ uint8_t opcode = OQSPI_GET_DEVICE_PARAM(write_enable_instr_cfg.opcode);

        do {
                oqspi_flash_cmd(opcode, bus_mode);
                /* Verify */
                do {
                        status = oqspi_flash_read_status_register(bus_mode);
                } while (status & OQSPI_STATUS_REG_BUSY_MASK);
        } while (!(status & OQSPI_STATUS_REG_WEL_MASK));
}

__RETAINED_CODE static void oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE bus_mode, bool forced)
{
        if ((manual_access_bus_mode != bus_mode) || forced) {
                hw_oqspi_set_manual_access_bus_mode(bus_mode);
                hw_oqspi_set_io(bus_mode);
                manual_access_bus_mode = bus_mode;
        }
}

/*
 * In order to exit from continuous mode of operation the OQSPI_EXIT_CONTINUOUS_MODE_OPCODE must be
 * shifted in the extra byte phase of a read access command.
 */
__RETAINED_CODE static void oqspi_flash_exit_continuous_mode_cmd(HW_OQSPI_ADDR_SIZE addr_size)
{
        hw_oqspi_cs_enable();
        hw_oqspi_write32(OQSPI_EXIT_CONTINUOUS_MODE_WORD);

        if (addr_size == HW_OQSPI_ADDR_SIZE_32) {
                hw_oqspi_write8(OQSPI_EXIT_CONTINUOUS_MODE_BYTE);
        }

        hw_oqspi_cs_disable();
}

__RETAINED_CODE static void oqspi_flash_exit_continuous_mode(void)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_ADDR_SIZE addr_size = OQSPI_GET_DEVICE_PARAM(address_size);

        oqspi_flash_exit_continuous_mode_cmd(addr_size);
}

/**
 * Enter QPI mode
 */
__UNUSED __RETAINED_CODE static void oqspi_enter_qpi_mode(void)
{
        hw_oqspi_cs_enable();
        hw_oqspi_write8(OQSPI_ENTER_QPI_OPCODE);
        hw_oqspi_cs_disable();
}

/**
 * Exit QPI mode
 */
__UNUSED __RETAINED_CODE static bool oqspi_exit_qpi(void)
{
        hw_oqspi_cs_enable();
        hw_oqspi_write8(OQSPI_EXIT_QPI_OPCODE);
        hw_oqspi_cs_disable();

        return true;
}

/*
 * Enter Manual Access Mode. This function turns the OQSPI Flash memory out of the Continuous Mode
 * of operation, if enabled.
 */
__RETAINED_CODE static void oqspi_enter_manual_access_mode(void)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_CONTINUOUS_MODE continuous_mode = OQSPI_GET_DEVICE_PARAM(read_instr_cfg.continuous_mode);

        if (hw_oqspi_get_access_mode() == HW_OQSPI_ACCESS_MODE_AUTO) {
                hw_oqspi_set_access_mode(HW_OQSPI_ACCESS_MODE_MANUAL);

                if (continuous_mode == HW_OQSPI_CONTINUOUS_MODE_ENABLE) {
                        oqspi_flash_exit_continuous_mode();
                }
        }
}

__RETAINED_CODE void oqspi_automode_int_enter_auto_access_mode(void)
{
        /*
         * Before switching to Auto Access Mode set the direction of all OQSPIC IOs so that they are
         * selected by the controller.
         */
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE bus_mode = OQSPI_GET_DEVICE_PARAM(read_instr_cfg.data_bus_mode);

        hw_oqspi_set_io(bus_mode);
        hw_oqspi_set_access_mode(HW_OQSPI_ACCESS_MODE_AUTO);
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

__RETAINED_CODE uint32_t oqspi_automode_int_flash_write_page(uint32_t addr, const uint8_t *buf, uint32_t size)
{
        __DBG_OQSPI_VOLATILE__ uint32_t i = 0;
        __DBG_OQSPI_VOLATILE__ uint32_t odd = ((uint32_t) buf) & 3;
        __DBG_OQSPI_VOLATILE__ uint32_t size_aligned32;
        __DBG_OQSPI_VOLATILE__ uint32_t page_boundary;
        __DBG_OQSPI_VOLATILE__ uint8_t opcode = OQSPI_GET_DEVICE_PARAM(page_program_instr_cfg.opcode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE write_enable_bus_mode = OQSPI_GET_DEVICE_PARAM(write_enable_instr_cfg.opcode_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE opcode_bus_mode = OQSPI_GET_DEVICE_PARAM(page_program_instr_cfg.opcode_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE addr_bus_mode = OQSPI_GET_DEVICE_PARAM(page_program_instr_cfg.addr_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE data_bus_mode = OQSPI_GET_DEVICE_PARAM(page_program_instr_cfg.data_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_ADDR_SIZE addr_size = OQSPI_GET_DEVICE_PARAM(address_size);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_OPCODE_LEN opcode_len = OQSPI_GET_DEVICE_PARAM(opcode_len);

        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_PAGE_PROG);

        /* Reduce max write size, that can reduce interrupt latency time */
        if (size > dg_configOQSPI_FLASH_MAX_WRITE_SIZE) {
                size = dg_configOQSPI_FLASH_MAX_WRITE_SIZE;
        }

        /* Make sure write will not cross page boundary */
        page_boundary = OQSPI_FLASH_PAGE_SIZE - (addr & 0xFF);
        if (size > page_boundary) {
                size = page_boundary;
        }

        oqspi_enter_manual_access_mode();
        oqspi_set_manual_access_bus_mode(write_enable_bus_mode, false);
        oqspi_flash_write_enable(write_enable_bus_mode);

        oqspi_set_manual_access_bus_mode(opcode_bus_mode, false);
        hw_oqspi_cs_enable();

        if (USE_DUAL_BYTE_OPCODE(opcode_len, opcode_bus_mode)) {
                hw_oqspi_write16(CONVERT_OPCODE_TO_DUAL_BYTE(opcode));
        } else {
                hw_oqspi_write8(opcode);
        }

        oqspi_set_manual_access_bus_mode(addr_bus_mode, false);

        if (addr_size == HW_OQSPI_ADDR_SIZE_32) {
                hw_oqspi_write32(addr);
        } else {
                hw_oqspi_write8((uint8_t) ((addr >> 16) & 0xFF));
                hw_oqspi_write8((uint8_t) ((addr >> 8) & 0xFF));
                hw_oqspi_write8((uint8_t) (addr & 0xFF));
        }

        oqspi_set_manual_access_bus_mode(data_bus_mode, false);

        if (odd) {
                odd = 4 - odd;
                for (i = 0; i < odd && i < size; ++i) {
                        hw_oqspi_write8(buf[i]);
                }
        }

        size_aligned32 = ((size - i) & ~0x3);

        if (size_aligned32) {
                fast_write_to_fifo32((uint32_t)(buf + i), (uint32_t)(buf + i + size_aligned32),
                                     (uint32_t) &(OQSPIF->OQSPIF_WRITEDATA_REG));
                i += size_aligned32;
        }

        for (; i < size; ++i) {
                hw_oqspi_write8(buf[i]);
        }

        hw_oqspi_cs_disable();

        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_PAGE_PROG);

        return i;
}

#if !dgconfigOQSPI_ERASE_IN_AUTOMODE
/*
 * Erase a sector of the Flash in manual mode.
 *
 * Before calling this function you need to disable the interrupts and switch to Manual Access Mode
 * calling the oqspi_enter_manual_access_mode().
 *
 * This function does not block until the Flash has
 * processed the command! When calling this function the OQSPI controller remains to manual mode.
 * The function must be called with interrupts disabled.
 */
__RETAINED_CODE static void oqspi_flash_erase_sector_manual(uint32_t addr)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE write_enable_bus_mode = OQSPI_GET_DEVICE_PARAM(write_enable_instr_cfg.opcode_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE opcode_bus_mode = OQSPI_GET_DEVICE_PARAM(erase_instr_cfg.opcode_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE addr_bus_mode = OQSPI_GET_DEVICE_PARAM(erase_instr_cfg.addr_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_OPCODE_LEN opcode_len = OQSPI_GET_DEVICE_PARAM(opcode_len);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_ADDR_SIZE addr_size = OQSPI_GET_DEVICE_PARAM(address_size);
        __DBG_OQSPI_VOLATILE__ uint8_t opcode = OQSPI_GET_DEVICE_PARAM(erase_instr_cfg.opcode);

        oqspi_enter_manual_access_mode();
        oqspi_set_manual_access_bus_mode(write_enable_bus_mode, false);
        oqspi_flash_write_enable(write_enable_bus_mode);

        oqspi_set_manual_access_bus_mode(opcode_bus_mode, false);

        hw_oqspi_cs_enable();

        if (USE_DUAL_BYTE_OPCODE(opcode_len, opcode_bus_mode)) {
                hw_oqspi_write16(CONVERT_OPCODE_TO_DUAL_BYTE(opcode));
        } else {
                hw_oqspi_write8(opcode);
        }

        oqspi_set_manual_access_bus_mode(addr_bus_mode, false);

        if (addr_size == HW_OQSPI_ADDR_SIZE_32) {
                hw_oqspi_write32(addr);
        } else {
                hw_oqspi_write8((uint8_t) ((addr >> 16) & 0xFF));
                hw_oqspi_write8((uint8_t) ((addr >> 8) & 0xFF));
                hw_oqspi_write8((uint8_t) (addr & 0xFF));
        }

        hw_oqspi_cs_disable();
        /*
         * Flash stays in manual mode.
         */

}
#endif /* !dgconfigOQSPI_ERASE_IN_AUTOMODE */

#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS
__RETAINED_CODE void oqspi_automode_int_resume(void)
{
        __DBG_OQSPI_VOLATILE__ uint8_t resume_opcode = OQSPI_GET_DEVICE_PARAM(suspend_resume_instr_cfg.resume_opcode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE suspended_bus_mode = OQSPI_GET_DEVICE_PARAM(suspend_resume_instr_cfg.suspend_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE resume_bus_mode = OQSPI_GET_DEVICE_PARAM(suspend_resume_instr_cfg.resume_bus_mode);
        __DBG_OQSPI_VOLATILE__ oqspi_is_suspended_cb_t is_suspended = OQSPI_GET_DEVICE_PARAM(callback.is_suspended_cb);
        __DBG_OQSPI_VOLATILE__ uint32_t resume_latency = OQSPI_GET_DEVICE_PARAM(suspend_resume_instr_cfg.resume_latency_usec);

        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_RESUME);

        oqspi_enter_manual_access_mode();
        oqspi_set_manual_access_bus_mode(suspended_bus_mode, false);

        if (!is_suspended(suspended_bus_mode)) {
                return;
        }

        do {
                // Send Resume command
                oqspi_set_manual_access_bus_mode(resume_bus_mode, false);
                oqspi_flash_cmd(resume_opcode, resume_bus_mode);
                oqspi_set_manual_access_bus_mode(suspended_bus_mode, false);
        }  while (is_suspended(suspended_bus_mode));

        hw_clk_delay_usec(resume_latency);

        // Flash stays in manual mode.
        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_RESUME);
}

__RETAINED_CODE void oqspi_automode_int_suspend(void)
{
        __DBG_OQSPI_VOLATILE__ uint8_t opcode = OQSPI_GET_DEVICE_PARAM(suspend_resume_instr_cfg.suspend_opcode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE suspend_bus_mode = OQSPI_GET_DEVICE_PARAM(suspend_resume_instr_cfg.suspend_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE busy_bus_mode = OQSPI_GET_DEVICE_PARAM(read_status_instr_cfg.opcode_bus_mode);
        __DBG_OQSPI_VOLATILE__ uint32_t suspend_latency = OQSPI_GET_DEVICE_PARAM(suspend_resume_instr_cfg.suspend_latency_usec);

        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_SUSPEND_ACTION);

        oqspi_enter_manual_access_mode();
        oqspi_set_manual_access_bus_mode(busy_bus_mode, false);

        // Check if an operation is ongoing.
        while (oqspi_flash_is_busy(busy_bus_mode)) {
                oqspi_set_manual_access_bus_mode(suspend_bus_mode, false);
                oqspi_flash_cmd(opcode, suspend_bus_mode);
                oqspi_set_manual_access_bus_mode(busy_bus_mode, false);
        }

        // Wait for SUS bit to be updated
        hw_clk_delay_usec(suspend_latency);

        // Flash stays in manual mode.
        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_SUSPEND_ACTION);
}

__RETAINED_CODE bool oqspi_automode_int_is_suspended(void)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE bus_mode = OQSPI_GET_DEVICE_PARAM(suspend_resume_instr_cfg.suspend_bus_mode);

        return OQSPI_GET_DEVICE_PARAM(callback.is_suspended_cb)(bus_mode);
}

__RETAINED_CODE bool oqspi_automode_int_is_busy(void)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE bus_mode = OQSPI_GET_DEVICE_PARAM(read_status_instr_cfg.opcode_bus_mode);

        oqspi_enter_manual_access_mode();
        oqspi_set_manual_access_bus_mode(bus_mode, false);

        return oqspi_flash_is_busy(bus_mode);
}
#endif /* dg_configUSE_SYS_BACKGROUND_FLASH_OPS */

__RETAINED_CODE bool oqspi_is_valid_addr(uint32_t addr)
{
        __DBG_OQSPI_VOLATILE__ uint32_t size_mbits = OQSPI_GET_DEVICE_PARAM(size_mbits);

        if (addr >= MEMORY_OQSPIC_SIZE) {
                return false;
        }

        return (addr * 8) < size_mbits;
}

uint32_t oqspi_get_device_size(void)
{
        __DBG_OQSPI_VOLATILE__ uint32_t size_mbits = OQSPI_GET_DEVICE_PARAM(size_mbits);

        return (size_mbits / 8);
}

bool oqspi_get_config(jedec_id_t *jedec)
{
        jedec->manufacturer_id = OQSPI_GET_DEVICE_PARAM(jedec.manufacturer_id);
        jedec->type = OQSPI_GET_DEVICE_PARAM(jedec.type);
        jedec->density = OQSPI_GET_DEVICE_PARAM(jedec.density);

        if (jedec->manufacturer_id == 0xFF || jedec->manufacturer_id == 0x00) {
                return false;
        }

        return true;
}

/**
 * \brief Check if the Flash can accept commands
 *
 * \return bool True if the Flash is not busy else false.
 *
 */
__RETAINED_CODE static bool oqspi_flash_is_writable(void)
{
        __DBG_OQSPI_VOLATILE__ bool writable;
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE busy_bus_mode = OQSPI_GET_DEVICE_PARAM(read_status_instr_cfg.opcode_bus_mode);

        // Disable the interrupts as long as the OQSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();

        oqspi_enter_manual_access_mode();

        // Check if flash is ready.
        writable = !(oqspi_flash_is_busy(busy_bus_mode));

        oqspi_automode_int_enter_auto_access_mode();

        // Re-enable the interrupts since the OQSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();

        return writable;
}

__RETAINED_CODE uint32_t oqspi_automode_write_flash_page(uint32_t addr, const uint8_t *buf, uint32_t size)
{
        ASSERT_WARNING(size > 0);

        __DBG_OQSPI_VOLATILE__ uint32_t written;
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE busy_bus_mode = OQSPI_GET_DEVICE_PARAM(read_status_instr_cfg.opcode_bus_mode);

        // Disable the interrupts as long as the OQSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();

        // Wait until the flash memory is ready
        oqspi_enter_manual_access_mode();
        oqspi_set_manual_access_bus_mode(busy_bus_mode, false);
        while (oqspi_flash_is_busy(busy_bus_mode));

        written = oqspi_automode_int_flash_write_page(addr, buf, size);

        /* Wait the write command to be completed */
        oqspi_set_manual_access_bus_mode(busy_bus_mode, false);
        while (oqspi_flash_is_busy(busy_bus_mode));

        oqspi_automode_int_enter_auto_access_mode();

        // Re-enable the interrupts since the OQSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();

        return written;
}

__RETAINED_CODE void oqspi_automode_erase_flash_sector(uint32_t addr)
{
        DBG_SET_HIGH(FLASH_DEBUG, FLASHDBG_SECTOR_ERASE);

        while (!oqspi_flash_is_writable());

#if dgconfigOQSPI_ERASE_IN_AUTOMODE
        hw_oqspi_erase_block(addr);

        while (hw_oqspi_get_erase_status() != HW_OQSPI_ERASE_STATUS_NO);
#else
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE busy_bus_mode = OQSPI_GET_DEVICE_PARAM(read_status_instr_cfg.opcode_bus_mode);

        // Disable the interrupts as long as the OQSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();

        oqspi_enter_manual_access_mode();
        oqspi_flash_erase_sector_manual(addr);
        while (oqspi_flash_is_busy(busy_bus_mode));
        oqspi_automode_int_enter_auto_access_mode();

        // Re-enable the interrupts since the OQSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();
#endif
        DBG_SET_LOW(FLASH_DEBUG, FLASHDBG_SECTOR_ERASE);
}

void oqspi_automode_erase_chip(void)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE write_enable_bus_mode = OQSPI_GET_DEVICE_PARAM(write_enable_instr_cfg.opcode_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE erase_bus_mode = OQSPI_GET_DEVICE_PARAM(erase_instr_cfg.opcode_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE busy_bus_mode = OQSPI_GET_DEVICE_PARAM(read_status_instr_cfg.opcode_bus_mode);

        // Disable the interrupts as long as the OQSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();

        oqspi_enter_manual_access_mode();
        oqspi_flash_write_enable(write_enable_bus_mode);
        oqspi_flash_cmd(OQSPI_CHIP_ERASE_OPCODE, erase_bus_mode);
        while (oqspi_flash_is_busy(busy_bus_mode));
        oqspi_automode_int_enter_auto_access_mode();

        // Re-enable the interrupts since the OQSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();
}

uint32_t oqspi_automode_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
        memcpy(buf, oqspi_automode_get_physical_addr(addr), len);
        return len;
}

const void *oqspi_automode_get_physical_addr(uint32_t virtual_addr)
{
        return (const void *) (MEMORY_OQSPIC_S_BASE + virtual_addr);
}

__RETAINED_CODE static void oqspi_flash_init_callback(HW_OQSPI_BUS_MODE bus_mode)
{
        __DBG_OQSPI_VOLATILE__ sys_clk_t sys_clk = hw_clk_get_system_clock();

        // Disable the interrupts as long as the OQSPIC remains in manual access mode.
        GLOBAL_INT_DISABLE();

        oqspi_enter_manual_access_mode();
        oqspi_set_manual_access_bus_mode(bus_mode, true);
        OQSPI_GET_DEVICE_PARAM(callback.initialize_cb)(bus_mode, sys_clk);
        oqspi_automode_int_enter_auto_access_mode();

        // Re-enable the interrupts, since the OQSPIC switched back to auto access mode.
        GLOBAL_INT_RESTORE();
}

__RETAINED_CODE void oqspi_automode_flash_power_up(void)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE opcode_bus_mode = OQSPI_GET_DEVICE_PARAM(read_instr_cfg.opcode_bus_mode);

        // Disable the interrupts as long as the OQSPIC remains in manual access mode.
        GLOBAL_INT_DISABLE();

        hw_oqspi_clock_enable();

        // The bus mode is not retained during sleep mode and has to be configured when the system
        // wakes up, otherwise the release from power down command will be sent in single SPI bus mode.
        oqspi_set_manual_access_bus_mode(opcode_bus_mode, true);

#if dg_configOQSPI_FLASH_POWER_DOWN
        __DBG_OQSPI_VOLATILE__ uint32_t release_pd_delay = OQSPI_GET_DEVICE_PARAM(delay.release_power_down_usec);

        oqspi_enter_manual_access_mode();
        oqspi_flash_cmd(OQSPI_RELEASE_POWER_DOWN_OPCODE, opcode_bus_mode);
        hw_clk_delay_usec(release_pd_delay);
#elif dg_configOQSPI_FLASH_POWER_OFF
        __DBG_OQSPI_VOLATILE__ uint32_t power_up_delay = OQSPI_GET_DEVICE_PARAM(delay.power_up_usec);

        hw_clk_delay_usec(power_up_delay);
        hw_oqspi_set_access_mode(HW_OQSPI_ACCESS_MODE_MANUAL);
        // When the flash memory is powered off, it switches to single SPI bus mode, hence the
        // initialization callback has to be called in single SPI bus mode.
        oqspi_flash_init_callback(HW_OQSPI_BUS_MODE_SINGLE);
#endif

        oqspi_automode_int_enter_auto_access_mode();
        // Re-enable the interrupts, since the OQSPIC switched back to auto access mode.
        GLOBAL_INT_RESTORE();
}

__RETAINED_CODE void oqspi_automode_flash_power_down(void)
{
        oqspi_enter_manual_access_mode();

#if dg_configOQSPI_FLASH_POWER_DOWN
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE opcode_bus_mode = OQSPI_GET_DEVICE_PARAM(read_instr_cfg.opcode_bus_mode);
        __DBG_OQSPI_VOLATILE__ uint32_t power_down_delay = OQSPI_GET_DEVICE_PARAM(delay.power_down_usec);

        oqspi_flash_cmd(OQSPI_ENTER_POWER_DOWN_OPCODE, opcode_bus_mode);
        hw_clk_delay_usec(power_down_delay);
#elif dg_configOQSPI_FLASH_POWER_OFF
        /*
         * If the read instruction opcode is in single SPI bus mode, then the flash memory
         * has to exit from QPI/OPI mode (Quad/Octa memories respectively). Normally, the
         * memory switches to single SPI bus mode, when it is powered off. However, it has
         * been observed that in some use cases, the PMU controller might switch the V18F
         * rail on, before it has been completely discharged and this prevents the memory
         * from switching to single SPI bus mode (e.g. when a BLE application is attempting
         * to erase/write the XiP flash). In this case, when the system wakes up again the
         * OQSPIC will attempt to re-initialize the flash memory in single SPI bus mode,
         * whereas the memory is still configured in QPI/OPI bus mode. The command "exit
         * from QPI/OPI" makes sure that this inconsistency will never takes place.
         */
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE opcode_bus_mode = OQSPI_GET_DEVICE_PARAM(read_instr_cfg.opcode_bus_mode);

        if (opcode_bus_mode != HW_OQSPI_BUS_MODE_SINGLE) {
                        ASSERT_ERROR(OQSPI_GET_DEVICE_PARAM(callback.exit_opi_qpi_cb)());
        }
#endif

        // Disable OQSPI clock to save power
        hw_oqspi_clock_disable();
}

__RETAINED_CODE static void oqspi_flash_reset_cmd(bool use_dual_byte_opcode)
{
        hw_oqspi_cs_enable();

        if (use_dual_byte_opcode) {
                hw_oqspi_write16(CONVERT_OPCODE_TO_DUAL_BYTE(OQSPI_RESET_EN_OPCODE));
        } else {
                hw_oqspi_write8(OQSPI_RESET_EN_OPCODE);
        }

        hw_oqspi_cs_disable();

        hw_oqspi_cs_enable();

        if (use_dual_byte_opcode) {
                hw_oqspi_write16(CONVERT_OPCODE_TO_DUAL_BYTE(OQSPI_RESET_OPCODE));
        } else {
                hw_oqspi_write8(OQSPI_RESET_OPCODE);
        }

        hw_oqspi_cs_disable();
}

__RETAINED_CODE static void oqspi_flash_release_power_down(HW_OQSPI_BUS_MODE bus_mode, bool use_dual_byte_opcode)
{
        __DBG_OQSPI_VOLATILE__ uint32_t power_down_delay = OQSPI_GET_DEVICE_PARAM(delay.power_down_usec);

        oqspi_set_manual_access_bus_mode(bus_mode, false);
        hw_oqspi_cs_enable();

        if (use_dual_byte_opcode) {
                hw_oqspi_write16(CONVERT_OPCODE_TO_DUAL_BYTE(OQSPI_RELEASE_POWER_DOWN_OPCODE));
        } else {
                hw_oqspi_write8(OQSPI_RELEASE_POWER_DOWN_OPCODE);
        }

        hw_oqspi_cs_disable();

        hw_clk_delay_usec(power_down_delay);
}

/**
 * Apply all possible OQSPI Flash reset sequences to make sure that any type of Flash memory under
 * any possible configuration will be reset successfully
 */
__RETAINED_CODE static void oqspi_flash_reset(void)
{
        // Disable the interrupts as long as the OQSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();
        oqspi_enter_manual_access_mode();

#if dg_configOQSPI_FLASH_AUTODETECT


        bool apply_octa_bus_reset = true;

        if (hw_oqspi_are_io4_7_gpio()) {
                apply_octa_bus_reset = false;
        }

        hw_oqspi_set_access_mode(HW_OQSPI_ACCESS_MODE_MANUAL);

        // Apply all possible "exit from continuous mode" sequences
        if (apply_octa_bus_reset) {
                oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_OCTA, false);
                oqspi_flash_exit_continuous_mode_cmd(HW_OQSPI_ADDR_SIZE_32);
        }

        oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_QUAD, false);
        oqspi_flash_exit_continuous_mode_cmd(HW_OQSPI_ADDR_SIZE_32);

        oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_SINGLE, false);
        oqspi_flash_exit_continuous_mode_cmd(HW_OQSPI_ADDR_SIZE_32);

        // Apply all possible "release from power down" sequences
        if (apply_octa_bus_reset) {
                oqspi_flash_release_power_down(HW_OQSPI_BUS_MODE_OCTA, false);
                oqspi_flash_release_power_down(HW_OQSPI_BUS_MODE_OCTA, true);
        }

        oqspi_flash_release_power_down(HW_OQSPI_BUS_MODE_QUAD, false);
        oqspi_flash_release_power_down(HW_OQSPI_BUS_MODE_SINGLE, false);

        // Apply all possible reset sequences
        if (apply_octa_bus_reset) {
                oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_OCTA, false);
                oqspi_flash_reset_cmd(false);
                oqspi_flash_reset_cmd(true);
        }

        oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_QUAD, false);
        oqspi_flash_reset_cmd(false);

        oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_SINGLE, false);
        oqspi_flash_reset_cmd(false);

        hw_clk_delay_usec(dg_configOQSPI_FLASH_AUTODETECT_RESET_DELAY);
#else /* dg_configOQSPI_FLASH_AUTODETECT */
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE opcode_bus_mode = OQSPI_GET_DEVICE_PARAM(read_instr_cfg.opcode_bus_mode);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_OPCODE_LEN opcode_len = OQSPI_GET_DEVICE_PARAM(opcode_len);
        __DBG_OQSPI_VOLATILE__ bool use_dual_byte_opcode = USE_DUAL_BYTE_OPCODE(opcode_len, opcode_bus_mode);
        __DBG_OQSPI_VOLATILE__ uint32_t reset_delay = OQSPI_GET_DEVICE_PARAM(delay.reset_usec);

        oqspi_flash_release_power_down(opcode_bus_mode, use_dual_byte_opcode);
        oqspi_flash_release_power_down(HW_OQSPI_BUS_MODE_SINGLE, false);

        oqspi_set_manual_access_bus_mode(opcode_bus_mode, false);
        oqspi_flash_reset_cmd(use_dual_byte_opcode);

        oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_SINGLE, false);
        oqspi_flash_reset_cmd(false);

        hw_clk_delay_usec(reset_delay);
#endif /* dg_configOQSPI_FLASH_AUTODETECT */

        oqspi_automode_int_enter_auto_access_mode();
        // Re-enable the interrupts since the OQSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();
}

#if (dg_configOQSPI_FLASH_CONFIG_VERIFY || dg_configOQSPI_FLASH_AUTODETECT)
__RETAINED_CODE static bool oqspi_match_jedec_id(jedec_id_t *jedec, const oqspi_flash_config_t *flash_cfg)
{
        if ((jedec->manufacturer_id == flash_cfg->jedec.manufacturer_id) &&
            (jedec->type == flash_cfg->jedec.type) &&
            ((jedec->density & flash_cfg->jedec.density_mask) == flash_cfg->jedec.density)) {
                return true;
        }

        return false;
}
#endif /* (dg_configOQSPI_FLASH_CONFIG_VERIFY || dg_configOQSPI_FLASH_AUTODETECT) */

#if dg_configOQSPI_FLASH_AUTODETECT
/**
 * Searches in oqspi_memory_config_table[] whether the input jedec id matches one of the supported memories.
 * If yes, it copies the corresponding oqspi_memory_config_table[i] to oqspi_flash_config and returns true.
 * Otherwise, it returns false.
 */
__RETAINED_CODE static bool oqspi_flash_detect(jedec_id_t *jedec)
{
        __DBG_OQSPI_VOLATILE__ bool jedec_id_matched = false;

        for (uint8_t i = 0; i < ARRAY_LENGTH(oqspi_memory_config_table); ++i) {
                jedec_id_matched = oqspi_match_jedec_id(jedec, oqspi_memory_config_table[i]);

                if (jedec_id_matched) {
                        memcpy(&oqspi_flash_config, oqspi_memory_config_table[i], sizeof(oqspi_flash_config_t));
                        break;
                }
        }

        return jedec_id_matched;
}
#endif /* dg_configOQSPI_FLASH_AUTODETECT */

__RETAINED_CODE static bool read_jedec_id(jedec_id_t *jedec, bool alt_method)
{
        bool found = false;

        /*
         * The read jedec ID takes place while the OQSPI controller is in manual mode, hence all
         * pointer used in the context of this function must be in SYSRAM. However, if the pointers
         * are initialized following the classic way, e.g.:
         *
         * uint8_t cmd[4] = { OQSPI_READ_JEDEC_ID_OPCODE, 0x00, 0x00, 0x00 };
         * jedec_id_t jedec_buf = { 0xFF, 0xFF, 0xFF, 0xFF };
         *
         * the compiler puts them in the .rodata section in flash memory and they won't be accessible.
         */

        uint8_t cmd[4];
        jedec_id_t jedec_buf;
        // The normal read jedec id command consists of 1 byte whereas the alternative one of 4.
        uint32_t cmd_len = alt_method ? sizeof(cmd) : 1;

        cmd[0] = OQSPI_READ_JEDEC_ID_OPCODE;
        cmd[1] = 0x00;
        cmd[2] = 0x00;
        cmd[3] = 0x00;

        jedec_buf.manufacturer_id = 0xFF;
        jedec_buf.type = 0xFF;
        jedec_buf.density = 0xFF;
        jedec_buf.density_mask = 0xFF;
        // Disable the interrupts as long as the OQSPIC remains in manual access mode
        GLOBAL_INT_DISABLE();

        // Switch to manual access mode in order to read the jedec id
        oqspi_enter_manual_access_mode();
        oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_SINGLE, false);
        oqspi_flash_transact(cmd, cmd_len, (uint8_t*) &jedec_buf, 3);
        // Once the jedec id was read, switch back to auto access mode in order to match/detect the
        // flash memory. This is mandatory because the read jedec id will be compared with data
        // located in the flash memory (oqspi_memory_config_table or dg_configOQSPI_FLASH_CONFIG). The
        // preliminary OQSPIC configuration has already enabled XiP at low performance mode.
        oqspi_automode_int_enter_auto_access_mode();

        // Re-enable the interrupts since the OQSPIC switched back to auto access mode
        GLOBAL_INT_RESTORE();

#if dg_configOQSPI_FLASH_AUTODETECT
        found = oqspi_flash_detect(&jedec_buf);
#elif dg_configOQSPI_FLASH_CONFIG_VERIFY
        found = oqspi_match_jedec_id(&jedec_buf, &dg_configOQSPI_FLASH_CONFIG);
#else
        found = (jedec_buf.manufacturer_id != 0xFF) && (jedec_buf.manufacturer_id != 0x00);
#endif

        if (found) {
                // Copy the first 3 bytes (id, type and density) of the jedec_buf to jedec,
                // since the fourth one (density_mask) should not be overlapped.
                memcpy(jedec, &jedec_buf, 3);
        }

        return found;
}

__RETAINED_CODE bool oqspi_read_flash_jedec_id(jedec_id_t *jedec)
{
        __DBG_OQSPI_VOLATILE__ bool found = false;
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_CLK_DIV div = hw_oqspi_get_div();

        oqspi_flash_reset();

read_id:
        found = read_jedec_id(jedec, false);

        if (!found) {
                found = read_jedec_id(jedec, true);
        }

        // If both jedec id commands fail, try again with max OQSPIC clock divider (8).
        if (!found && hw_oqspi_get_div() != HW_OQSPI_CLK_DIV_8) {
                hw_oqspi_set_div(HW_OQSPI_CLK_DIV_8);
                goto read_id;
        }

        // Restore the OQSPIC clock divider and switch to auto access mode
        hw_oqspi_set_div(div);

        return found;
}

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE) || dg_configOQSPI_FLASH_AUTODETECT || \
     dg_configOQSPI_FLASH_CONFIG_VERIFY
/**
 * Initialize the OQSPI Controller with a preliminary setup which is applicable to all flash memories.
 */
__RETAINED_CODE static void oqspi_controller_preliminary_init(void)
{
        hw_oqspi_config_t oqspic_cfg;
        hw_oqspi_read_instr_config_t oqspic_read_instr_cfg;
        uint32_t sys_clk_freq = hw_clk_get_sysclk_freq();

        oqspic_cfg.address_size                                 = HW_OQSPI_ADDR_SIZE_24;
        oqspic_cfg.clk_div                                      = HW_OQSPI_CLK_DIV_1;
        oqspic_cfg.clock_mode                                   = HW_OQSPI_CLK_MODE_LOW;
        oqspic_cfg.drive_current                                = dg_configOQSPI_DRIVE_CURRENT;
        oqspic_cfg.opcode_len                                   = HW_OQSPI_OPCODE_LEN_1_BYTE;
        oqspic_cfg.read_pipe                                    = HW_OQSPI_READ_PIPE_ENABLE;
        oqspic_cfg.read_pipe_delay                              = READ_PIPE_DELAY_0V9;
        oqspic_cfg.sampling_edge                                = HW_OQSPI_SAMPLING_EDGE_POS;
        oqspic_cfg.slew_rate                                    = dg_configOQSPI_SLEW_RATE;
        oqspic_cfg.auto_mode_cfg.burst_len_limit                = HW_OQSPI_BURST_LEN_LIMIT_UNSPECIFIED;
        oqspic_cfg.auto_mode_cfg.full_buffer_mode               = HW_OQSPI_FULL_BUFFER_MODE_BLOCK;
        oqspic_cfg.manual_mode_cfg.dir_change_mode              = HW_OQSPI_DIR_CHANGE_MODE_DUMMY_ACCESS;
        oqspic_cfg.manual_mode_cfg.dummy_mode                   = HW_OQSPI_DUMMY_MODE_LAST_2_CLK;
        oqspic_cfg.manual_mode_cfg.hready_mode                  = HW_OQSPI_HREADY_MODE_WAIT;
        oqspic_cfg.manual_mode_cfg.mapped_addr_rd_acc_response  = HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE_IGNORE;

        oqspic_read_instr_cfg.opcode_bus_mode                   = HW_OQSPI_BUS_MODE_SINGLE,
        oqspic_read_instr_cfg.addr_bus_mode                     = HW_OQSPI_BUS_MODE_SINGLE,
        oqspic_read_instr_cfg.extra_byte_bus_mode               = HW_OQSPI_BUS_MODE_SINGLE,
        oqspic_read_instr_cfg.dummy_bus_mode                    = HW_OQSPI_BUS_MODE_SINGLE,
        oqspic_read_instr_cfg.data_bus_mode                     = HW_OQSPI_BUS_MODE_SINGLE,
        oqspic_read_instr_cfg.continuous_mode                   = HW_OQSPI_CONTINUOUS_MODE_DISABLE,
        oqspic_read_instr_cfg.extra_byte_cfg                    = HW_OQSPI_EXTRA_BYTE_DISABLE,
        oqspic_read_instr_cfg.extra_byte_half_cfg               = HW_OQSPI_EXTRA_BYTE_HALF_DISABLE,
        oqspic_read_instr_cfg.opcode                            = OQSPI_READ3B_OPCODE,
        oqspic_read_instr_cfg.extra_byte_value                  = 0xFF,
        oqspic_read_instr_cfg.cs_idle_delay_nsec                = 10,

        hw_oqspi_init(&oqspic_cfg);
        hw_oqspi_read_instr_init(&oqspic_read_instr_cfg, 0, sys_clk_freq);
        oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE_SINGLE, true);
}
#endif

/**
 * Initialize the OQSPI controller based on the OQSPI flash driver.
 */
__RETAINED_CODE static void oqspi_controller_init(void)
{
        hw_oqspi_config_t oqspic_cfg;
        sys_clk_t sys_clk = hw_clk_get_system_clock();
        uint32_t sys_clk_freq = hw_clk_calculate_sys_clk_freq(sys_clk);
        uint8_t dummy_bytes = OQSPI_GET_DEVICE_PARAM(callback.get_dummy_bytes_cb)(sys_clk);

        oqspic_cfg.address_size = OQSPI_GET_DEVICE_PARAM(address_size);
        oqspic_cfg.clk_div = HW_OQSPI_CLK_DIV_1;
        oqspic_cfg.clock_mode = OQSPI_GET_DEVICE_PARAM(clk_mode);
        oqspic_cfg.drive_current = dg_configOQSPI_DRIVE_CURRENT;
        oqspic_cfg.opcode_len = OQSPI_GET_DEVICE_PARAM(opcode_len);
        oqspic_cfg.read_pipe = HW_OQSPI_READ_PIPE_ENABLE;
        oqspic_cfg.read_pipe_delay = READ_PIPE_DELAY_0V9;
        oqspic_cfg.sampling_edge = HW_OQSPI_SAMPLING_EDGE_POS;
        oqspic_cfg.slew_rate = dg_configOQSPI_SLEW_RATE;
        oqspic_cfg.auto_mode_cfg.burst_len_limit = HW_OQSPI_BURST_LEN_LIMIT_UNSPECIFIED;
        oqspic_cfg.auto_mode_cfg.full_buffer_mode = HW_OQSPI_FULL_BUFFER_MODE_BLOCK;
        oqspic_cfg.manual_mode_cfg.dir_change_mode = HW_OQSPI_DIR_CHANGE_MODE_DUMMY_ACCESS;
        oqspic_cfg.manual_mode_cfg.dummy_mode = HW_OQSPI_DUMMY_MODE_LAST_2_CLK;
        oqspic_cfg.manual_mode_cfg.hready_mode = HW_OQSPI_HREADY_MODE_WAIT;
        oqspic_cfg.manual_mode_cfg.mapped_addr_rd_acc_response = HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE_IGNORE;

        hw_oqspi_init(&oqspic_cfg);
        hw_oqspi_read_instr_init(&OQSPI_GET_DEVICE_PARAM(read_instr_cfg), dummy_bytes, sys_clk_freq);
        hw_oqspi_read_status_instr_init(&OQSPI_GET_DEVICE_PARAM(read_status_instr_cfg), sys_clk_freq);
        hw_oqspi_write_enable_instr_init(&OQSPI_GET_DEVICE_PARAM(write_enable_instr_cfg));
#if dgconfigOQSPI_ERASE_IN_AUTOMODE
        hw_oqspi_erase_instr_init(&OQSPI_GET_DEVICE_PARAM(erase_instr_cfg), sys_clk_freq);
        hw_oqspi_exit_continuous_mode_instr_init(&OQSPI_GET_DEVICE_PARAM(exit_continuous_mode_instr_cfg));
#endif

#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS
        hw_oqspi_suspend_resume_instr_init(&OQSPI_GET_DEVICE_PARAM(suspend_resume_instr_cfg));
#endif
}

#if (dg_configOQSPI_FLASH_CONFIG_VERIFY || dg_configOQSPI_FLASH_AUTODETECT)
/**
 * Initializes the OQSPI controller when either dg_configOQSPI_FLASH_AUTODETECT or
 * dg_configOQSPI_FLASH_CONFIG_VERIFY is enabled.
 */
__RETAINED_CODE static bool oqspi_flash_detect_init(void)
{
        bool matched;
        jedec_id_t jedec;

        oqspi_controller_preliminary_init();
        matched = oqspi_read_flash_jedec_id(&jedec);

        /*
         * If the memory is detected/matched, initialize the OQSPIC for high performance based on the
         * flash driver's configuration structure. Otherwise, the connected memory is considered as
         * unknown and the OQSPIC configuration remains as applied by oqspi_controller_preliminary_init().
         * The latter maintains a low performance reliable functionality in single SPI bus mode,
         * which is applicable to all memories.
         */
        if (matched) {
                oqspi_flash_init_callback(HW_OQSPI_BUS_MODE_SINGLE);
                oqspi_controller_init();
        }

        return matched;
}

#else /* (dg_configOQSPI_FLASH_CONFIG_VERIFY || dg_configOQSPI_FLASH_AUTODETECT) */

__RETAINED_CODE static void oqspi_flash_no_detect_init(void)
{
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_BUS_MODE opcode_bus_mode;

# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
        /*
         * In RAM build the bootrom is not involved, therefore the OQSPI flash memory can be in an
         * unknown state. Thus, a reset sequence is first applied, in order to make sure that the
         * memory is switched to single SPI bus mode, and in turns the initialization callback
         * is called (in single bus mode).
         */
        oqspi_controller_preliminary_init();
        oqspi_flash_reset();
        opcode_bus_mode = HW_OQSPI_BUS_MODE_SINGLE;
# else /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE) */
        opcode_bus_mode = OQSPI_GET_DEVICE_PARAM(read_instr_cfg.opcode_bus_mode);
# endif /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE) */
        oqspi_flash_init_callback(opcode_bus_mode);
        oqspi_controller_init();
}

#endif /* (dg_configOQSPI_FLASH_CONFIG_VERIFY) || (dg_configOQSPI_FLASH_AUTODETECT) */

__RETAINED_CODE bool oqspi_automode_init(void)
{
        __DBG_OQSPI_VOLATILE__ bool flash_detected;

#if IS_CACHED_FLASH
        ASSERT_WARNING((hw_cache_get_extflash_cacheable_len() != 0) || (hw_cache_is_enabled() == 1));
#endif

#if (dg_configOQSPI_FLASH_CONFIG_VERIFY || dg_configOQSPI_FLASH_AUTODETECT)
        flash_detected = oqspi_flash_detect_init();
        ASSERT_WARNING(flash_detected);
#else
        oqspi_flash_no_detect_init();
        flash_detected = true;
#endif
        return flash_detected;
}

__RETAINED_CODE void oqspi_automode_sys_clock_cfg(sys_clk_t sys_clk)
{
        __DBG_OQSPI_VOLATILE__ uint16_t read_cs_idle_delay = OQSPI_GET_DEVICE_PARAM(read_instr_cfg.cs_idle_delay_nsec);
        __DBG_OQSPI_VOLATILE__ uint16_t erase_cs_idle_delay = OQSPI_GET_DEVICE_PARAM(erase_instr_cfg.cs_idle_delay_nsec);
        __DBG_OQSPI_VOLATILE__ HW_OQSPI_CLK_DIV oqspi_div = (sys_clk == sysclk_PLL160) ? HW_OQSPI_CLK_DIV_2 : HW_OQSPI_CLK_DIV_1;
        __DBG_OQSPI_VOLATILE__ uint32_t oqspic_clk_freq = hw_clk_calculate_sys_clk_freq(sys_clk) >> (uint32_t) oqspi_div;

        oqspi_set_read_pipe_clock_delay(sys_clk);
        hw_oqspi_set_div(oqspi_div);
        hw_oqspi_set_read_cs_idle_delay(read_cs_idle_delay, oqspic_clk_freq);
        hw_oqspi_set_erase_cs_idle_delay(erase_cs_idle_delay, oqspic_clk_freq);

        GLOBAL_INT_DISABLE();
#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS
        __DBG_OQSPI_VOLATILE__ bool resume_before_writing_regs = OQSPI_GET_DEVICE_PARAM(resume_before_writing_regs);
        /*
         * When the system clock switches, the XiP flash might be suspended due to an ongoing flash
         * erase operation. Some flash memories reject commands such as `write status register`,
         * `write config register` etc while being in erase suspend mode, thus a flash erase resume
         * command must be issued in advance. Otherwise, the sys_clk_cfg_cb() will fail to update the
         * corresponding memory settings and the execution will end up to a bus fault, because the
         * OQSPIC won't be able to access the memory.
         */
        if (resume_before_writing_regs) {
                oqspi_automode_int_resume();
                oqspi_automode_int_enter_auto_access_mode();
        }
#endif

        /*
         * The sys_clk_cfg_cb() might switch the OQSPIC to manual access mode, where the flash
         * memory is not available for XiP, therefore the interrupts are disable during its call.
         */
        OQSPI_GET_DEVICE_PARAM(callback.sys_clk_cfg_cb)(sys_clk);
        GLOBAL_INT_RESTORE();
}

#endif /* dg_configUSE_HW_OQSPI */
