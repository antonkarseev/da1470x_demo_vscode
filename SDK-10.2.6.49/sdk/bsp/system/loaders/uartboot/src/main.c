/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief UART bootloader
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sdk_defs.h>
#include <hw_gpio.h>
#include <hw_uart.h>
#include <hw_timer.h>
#include <hw_otpc.h>
#include <hw_qspi.h>
#include <hw_oqspi.h>
#include <hw_watchdog.h>
#include <hw_clk.h>
#include <qspi_automode.h>
#include <oqspi_automode.h>
#include <ad_flash.h>
#include <ad_nvms.h>
#include <ad_nvms_ves.h>
#include <ad_nvms_direct.h>
#include <flash_partitions.h>
#include "sdk_crc16.h"
#if dg_configUSE_SYS_TCS
#include <sys_tcs.h>
#include "../../peripherals/src/hw_sys_internal.h"
#endif
#include "uartboot_types.h"
#include "protocol.h"

#define BOOTUART (HW_UART2)
#define BOOTUART_STEP                   3

#define glue(a,b) a##b
#define BAUDRATE_CONST(b) glue(HW_UART_BAUDRATE_, b)
#define BAUDRATE_CFG BAUDRATE_CONST(BAUDRATE)


#       define CFG_GPIO_BOOTUART_TX_PORT       HW_GPIO_PORT_0
#       define CFG_GPIO_BOOTUART_TX_PIN        HW_GPIO_PIN_8
#       define CFG_GPIO_BOOTUART_RX_PORT       HW_GPIO_PORT_2
#       define CFG_GPIO_BOOTUART_RX_PIN        HW_GPIO_PIN_1

/* These two values should always be related */
#define VERSION         (0x0004) // BCD
#define VERSION_STR     "0.0.0.4"

#define TMO_COMMAND     (2)
#define TMO_DATA        (5)
#define TMO_ACK         (3)

/*
 * this is 'magic' address which can be used in some commands to indicate some kind of temporary
 * storage, i.e. command needs to store some data but does not care where as long as it can be
 * accessed later
 */
#define ADDRESS_TMP     (0xFFFFFFFF)

/*
 *
 */
#define VIRTUAL_BUF_ADDRESS   (0x80000000)
#define VIRTUAL_BUF_MASK      (0xFFF00000)


#define IS_EMPTY_CHECK_SIZE     2048

/* Convert GPIO pad (1 byte) to GPIO port/pin */
#define GPIO_PAD_TO_PORT(pad)   (((pad) & 0xE0) >> 5)
#define GPIO_PAD_TO_PIN(pad)    ((pad) & 0x1F)

/* compile-time assertion */
#define C_ASSERT(cond) typedef char __c_assert[(cond) ? 1 : -1] __attribute__((unused))

#define UARTBOOT_LIVE_MARKER            "Live"

#define UNDETERMINED                    "Undetermined"

extern uint8_t __inputbuffer_start; // start of .inputbuffer section
extern uint8_t __inputbuffer_end;
uint32_t input_buffer_size;


/*
 * a complete flow for transmission handling (including in/out data) is as follows:
 *
 * <= <STX> <SOH> (ver1) (ver2)
 * => <SOH>
 * => (type) (len1) (len2)
 * call HOP_INIT
 * <= <ACK> / <NAK>
 * if len > 0
 *      => (data...)
 *      call HOP_DATA
 *      <= <ACK> / <NAK>
 *      <= (crc1) (crc2)
 *      => <ACK> / <NAK>
 * call HOP_EXEC
 * <= <ACK> / <NAK>
 * call HOP_SEND_LEN
 * if len > 0
 *      <= (len1) (len2)
 *      => <ACK> / <NAK>
 *      call HOP_SEND_DATA
 *      <= (data...)
 *      => (crc1) (crc2)
 *      <= <ACK> / <NAK>
 *
 * If NAK has been sent at some step, next steps shouldn't be performed.
 */

/* call type for command handler */
typedef enum {
        HOP_INIT,       // command header is received, i.e. type and length of incoming data
                        //      return false to NAK
        HOP_HEADER,     // full header is received
                        //      return false to NAK
        HOP_DATA,       // command data is received
                        //      return false to NAK
        HOP_EXEC,       // complete command data is received
                        //      return false to NAK
        HOP_SEND_LEN,   // need to send outgoing data length - use xmit_data()
                        //      return false if no data to be sent
        HOP_SEND_DATA,  // called for handler send data back - use xmit_data()
                        //      return false to abort
} HANDLER_OP;

/* UART configuration */
static uart_config UART_INIT = {
                .baud_rate              = HW_UART_BAUDRATE_115200,
                .data                   = HW_UART_DATABITS_8,
                .parity                 = HW_UART_PARITY_NONE,
                .stop                   = HW_UART_STOPBITS_1,
                .auto_flow_control      = 0,
                .use_fifo               = 1,
#if (HW_UART_DMA_SUPPORT == 1)
                .use_dma                = 0,
                .tx_dma_channel         = HW_DMA_CHANNEL_INVALID,
                .rx_dma_channel         = HW_DMA_CHANNEL_INVALID,
#endif /* HW_UART_DMA_SUPPORT */
};

static uint8_t uart_buf[32];                    // buffer for incoming data (control data only)

static volatile bool timer1_soh_tmo = true;     // timeout waiting for SOH flag

static volatile bool uart_soh = false;          // UART waiting for SOH flag

static bool uart_tmo = false;                   // timeout waiting for data from UART

static volatile uint16_t tick = 0;              // 1s tick counter

static volatile uint16_t uart_data_len = 0;     // length of data received from UART

static uint8_t array[AD_FLASH_MAX_SECTOR_SIZE]; // buffer used in safe_flash_write() below

/*
 * Valid port and pin values will be set in GPIO watchdog function. Port max and pin max aren't
 * a valid value - they won't be used as GPIO output without later initialization.
 */
static HW_GPIO_PORT gpio_wd_port = HW_GPIO_PORT_MAX;
static HW_GPIO_PIN gpio_wd_pin = HW_GPIO_PIN_MAX;
static uint32_t gpio_wd_timer_cnt;

#if dg_configNVMS_ADAPTER
static bool ad_nvms_init_called = false;    // ad_nvms_init() should be called once and only if needed
#endif


/**
 * \brief Send to RAM command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_send_to_ram {
        uint8_t *ptr;                   /**< Pointer to RAM where data will be written */
};

 /**
  * \brief Read from RAM command's parameters
  *
  */
__PACKED_STRUCT cmdhdr_read_from_ram {
        uint32_t ptr;                   /**< Pointer to RAM from where data will be read */
        uint16_t len;                   /**< Read data length in bytes */
};

/**
 * \brief Write RAM to QSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_write_ram_to_qspi {
        uint32_t ptr;                   /**< Pointer to RAM from where data will be read */
        uint16_t len;                   /**< Data length in bytes */
        uint32_t addr;                  /**< QSPI FLASH address, zero-based, where data will be written */
};

/**
 * \brief Erase QSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_erase_qspi {
        uint32_t addr;                  /**< QSPI FLASH erase start address; zero-based */
        uint32_t len;                   /**< Erase size in bytes */
};

/**
 * \brief Chip erase QSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_chip_erase_qspi {
        uint32_t addr;                  /**< QSPI FLASH erase start address; zero-based */
};

/**
 * \brief Execute code command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_execute_code {
        uint32_t addr;                  /**< Address of the function to call */
};

/**
 * \brief Write OTP command's parameters
 *
 * \note OTP cell size is 64-bits for DA1468x and 32-bits for DA1469x/DA1470x.
 *
 */
__PACKED_STRUCT cmdhdr_write_otp {
        uint32_t addr;                  /**< OTP cell offset */
};

/**
 * \brief Read OTP command's parameters
 *
 * \note OTP cell size is 64-bits for DA1468x and 32-bits for DA1469x/DA1470x.
 *
 */
__PACKED_STRUCT cmdhdr_read_otp {
        uint32_t addr;                  /**< OTP cell offset */
        uint16_t len;                   /**< Number of 32-bits words */
};

/**
 * \brief Read QSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_read_qspi {
        uint32_t addr;                  /**< Address in QSPI FLASH; zero-based */
        uint16_t len;                   /**< Read size in bytes */
};

#if dg_configNVMS_ADAPTER
/**
 * \brief Read partition command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_read_partition {
        uint32_t addr;                  /**< Offset from the partition's beginning */
        uint16_t len;                   /**< Read size in bytes */
        nvms_partition_id_t id;         /**< Partition ID */
};

/**
 * \brief Write partition command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_write_partition {
        uint32_t ptr;                   /**< Pointer to RAM from where data will be read */
        uint16_t len;                   /**< Write size in bytes */
        uint32_t addr;                  /**< Offset from the partition's beginning */
        nvms_partition_id_t id;         /**< Partition ID */
};
#endif

/**
 * \brief Get uartboot version command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_get_version {
};

/**
 * \brief Is empty QSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_is_empty_qspi {
        uint32_t size;                  /**< Check size in bytes */
        uint32_t start_address;         /**< QSPI FLASH check start address; zero-based */
};

/**
 * \brief Get QSPI state command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_get_qspi_state {
        uint8_t id;                     /**< QSPI controller ID */
};

/**
 * \brief Direct write to QSPI command's parameters
 *
 */
__PACKED_STRUCT  cmdhdr_direct_write_qspi {
        uint8_t read_back_verify;       /**< Verify written data (value other than 0 ) */
        uint32_t addr;                  /**< QSPI FLASH address, zero-based, where data will be written */
};

/**
 * \brief Write RAM to OQSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_write_ram_to_oqspi {
        uint32_t ptr;                   /**< Pointer to RAM from where data will be read */
        uint16_t len;                   /**< Data length in bytes */
        uint32_t addr;                  /**< OQSPI FLASH address, zero-based, where data will be written */
};

/**
 * \brief Erase OQSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_erase_oqspi {
        uint32_t addr;                  /**< OQSPI FLASH erase start address; zero-based */
        uint32_t len;                   /**< Erase size in bytes */
};

/**
 * \brief Chip erase OQSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_chip_erase_oqspi {
        uint32_t addr;                  /**< OQSPI FLASH erase start address; zero-based */
};

/**
 * \brief Read OQSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_read_oqspi {
        uint32_t addr;                  /**< Address in OQSPI FLASH; zero-based */
        uint16_t len;                   /**< Read size in bytes */
};

/**
 * \brief Is empty OQSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_is_empty_oqspi {
        uint32_t size;                  /**< Check size in bytes */
        uint32_t start_address;         /**< OQSPI FLASH check start address; zero-based */
};

/**
 * \brief Direct write to OQSPI command's parameters
 *
 */
__PACKED_STRUCT  cmdhdr_direct_write_oqspi {
        uint8_t read_back_verify;       /**< Verify written data (value other than 0 ) */
        uint32_t addr;                  /**< OQSPI FLASH address, zero-based, where data will be written */
};

/**
 * \brief Change UART's baudrate command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_change_baudrate {
        uint32_t baudrate;              /**< New UART baudrate */
};

/**
 * \brief GPIO external watchdog notification command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_gpio_wd {
        uint8_t gpio_pad;               /**< Encoded GPIO port and pin */
        uint8_t gpio_lvl;               /**< GPIO power source */
};

/*
 * union of all cmdhdr structures, this is used to create buffer to which command header will be
 * loaded so we can safely use payload buffer to keep data between commands
 */
union cmdhdr {
        struct cmdhdr_send_to_ram send_to_ram;
        struct cmdhdr_read_from_ram read_from_ram;
        struct cmdhdr_write_ram_to_qspi write_ram_to_qspi;
        struct cmdhdr_erase_qspi erase_qspi;
        struct cmdhdr_chip_erase_qspi chip_erase_qspi;
        struct cmdhdr_execute_code execute_code;
        struct cmdhdr_write_otp write_otp;
        struct cmdhdr_read_otp read_otp;
        struct cmdhdr_read_qspi read_qspi;
#if dg_configNVMS_ADAPTER
        struct cmdhdr_read_partition read_partition;
        struct cmdhdr_write_partition write_partition;
#endif
        struct cmdhdr_get_version get_version;
        struct cmdhdr_is_empty_qspi is_empty_qspi;
        struct cmdhdr_get_qspi_state get_qspi_state;
        struct cmdhdr_direct_write_qspi direct_write_qspi;
        struct cmdhdr_write_ram_to_oqspi write_ram_to_oqspi;
        struct cmdhdr_erase_oqspi erase_oqspi;
        struct cmdhdr_chip_erase_oqspi chip_erase_oqspi;
        struct cmdhdr_read_oqspi read_oqspi;
        struct cmdhdr_is_empty_oqspi is_empty_oqspi;
        struct cmdhdr_direct_write_oqspi direct_write_oqspi;
        struct cmdhdr_change_baudrate change_baudrate;
        struct cmdhdr_gpio_wd gpio_wd;
};

/* state of incoming command handler */
static struct cmd_state {
        uint8_t type;                           // type of command being handled
        uint16_t len;                           // command length (header and payload)
        union cmdhdr hdr;                       // command header
        uint16_t hdr_len;                       // command header length
        void *data;                             // command payload
        uint16_t data_len;                      // command payload length
        bool (* handler) (HANDLER_OP);          // command handler;

        uint16_t crc;                           // CRC of transmitted data;
} cmd_state;

typedef struct {
        char magic[4];
        volatile uint32_t run_swd;     /* This is set to 1 by debugger to enter SWD mode */
        volatile uint32_t cmd_num;     /* Debugger command sequence number, this field is
                                          incremented by debugger after arguments in uart_buf have
                                          been set for new command. Bootloader starts interpreting
                                          command when this number changes. This will prevent
                                          executing same command twice by accident */
        uint8_t *cmd_hdr_buf;          /* buffer for header stored here for debugger to see */
        uint8_t *buf;                  /* Big buffer for data transfer */
        volatile uint32_t ack_nak;     /* ACK or NAK for swd command */
} swd_interface_t;

const swd_interface_t swd_interface __attribute__((section (".swd_section"))) = {
        "DBGP", /* This marker is for debugger to search for swd_interface structure in memory */
        0,
        0,
        uart_buf,
        &__inputbuffer_start
};


__STATIC_INLINE bool is_valid_ptr_in_inputbuffer(uint32_t ptr)
{
        return ptr >= ((uint32_t)&__inputbuffer_start) &&
                                ptr < ((uint32_t)&__inputbuffer_end);
}

/**
 * \brief Translate 'magic' addresses into actual memory location
 *
 * \param [in,out] addr memory address
 *
 */
__STATIC_INLINE uint32_t translate_ram_addr(uint32_t addr)
{
        /*
         * ADDRESS_TMP will point to input buffer which is large enough to hold all received data
         * and it's not necessary to move data around since they are already received into this
         * buffer
         */
        if (addr == ADDRESS_TMP) {
                return (uint32_t) &__inputbuffer_start;
        } else if ((addr & VIRTUAL_BUF_MASK) == VIRTUAL_BUF_ADDRESS) {
                return (addr & ~VIRTUAL_BUF_MASK) + (uint32_t) &__inputbuffer_start;
        }

        return addr;
}

/**
 * \brief Check that given RAM address is in valid range
 *
 * This function handles 'magic' address to input buffer, therefore it should be called before
 * translate_ram_addr function.
 *
 * \param [in] addr memory address
 * \param [in] size requested size
 *
 * \return false if the address + size exceeds the temporary buffer (if address with
 *         VIRTUAL_BUF_MASK/ADDRESS_TMP passed), true otherwise
 *
 *  \sa translate_ram_addr
 *
 */
static bool check_ram_addr(uint32_t addr, uint32_t size)
{
        if (addr == ADDRESS_TMP) {
                addr = ((uint32_t) &__inputbuffer_start);
        } else if ((addr & VIRTUAL_BUF_MASK) == VIRTUAL_BUF_ADDRESS) {
                addr = (addr & ~VIRTUAL_BUF_MASK) + (uint32_t) &__inputbuffer_start;
        } else {
                /* Raw address - can be a SysRAM, CacheRam or register, do nothing */
                return true;
        }

        return (addr + size) <= (uint32_t) (&__inputbuffer_end);
}

static void timer1_soh_cb(void)
{
        hw_uart_abort_receive(BOOTUART);
        timer1_soh_tmo = true;
}

static void uart_soh_cb(uint8_t *data, uint16_t len)
{
        if (len == 1 && data[0] == SOH) {
                uart_soh = true;
        }
}

static void timer1_tick_cb(void)
{
        tick++;
}

static void timer_gpio_wd_cb(void)
{
        if (gpio_wd_timer_cnt == 0) {
                hw_gpio_set_active(gpio_wd_port, gpio_wd_pin);
        } else {
                hw_gpio_set_inactive(gpio_wd_port, gpio_wd_pin);
        }

        /* 15ms high, 2s low. Callback is called every 15ms by timer. 2000 / 15 = 133.33 */
        gpio_wd_timer_cnt = (gpio_wd_timer_cnt + 1) % 134;
}

static void uart_data_cb(void *user_data, uint16_t len)
{
        uart_data_len = len;
}

__STATIC_INLINE void xmit_hello(void)
{
        static const uint8_t msg[] = {
                        STX, SOH,
                        (VERSION & 0xFF00) >> 8, VERSION & 0x00FF };

        hw_uart_send(BOOTUART, msg, sizeof(msg), NULL, NULL);
}

__STATIC_INLINE void set_ack_nak_field(char sign)
{
        if (swd_interface.run_swd) {
                *((uint32_t *) &swd_interface.ack_nak) = sign;
        }
}

__STATIC_INLINE void xmit_ack(void)
{
        if (swd_interface.run_swd) {
                set_ack_nak_field(ACK);
                return;
        }

        hw_uart_write(BOOTUART, ACK);
}

__STATIC_INLINE void xmit_nak(void)
{
        if (swd_interface.run_swd) {
                set_ack_nak_field(NAK);
                return;
        }

        hw_uart_write(BOOTUART, NAK);
}

__STATIC_INLINE void xmit_crc16(uint16_t crc16)
{
        hw_uart_send(BOOTUART, (void *) &crc16, sizeof(crc16), NULL, NULL);
}

__STATIC_INLINE void xmit_data(const void *_buf, uint16_t len)
{
        uint8_t byt;
        uint16_t i;
        const uint8_t *buf = (const uint8_t *)_buf;

        for (i = 0; i < len; ++i) {
                byt = buf[i];

                hw_uart_write(BOOTUART, byt);
                crc16_update(&cmd_state.crc, &byt, 1);
        }
}

static bool recv_with_tmo(uint8_t *buf, uint16_t len, uint16_t tmo)
{
        if (!len) {
                return true;
        }

        tick = 0;
        uart_data_len = 0;
        uart_tmo = false;

        hw_timer_register_int(HW_TIMER, timer1_tick_cb);
        hw_timer_enable(HW_TIMER);
        hw_timer_enable_clk(HW_TIMER);

        hw_uart_receive(BOOTUART, buf, len, uart_data_cb, NULL);

        while (tick < tmo && uart_data_len == 0) {
                __WFI();
        }

        hw_timer_disable(HW_TIMER);

        /* abort if no data received */
        if (uart_data_len == 0) {
                uart_tmo = true;
                hw_uart_abort_receive(BOOTUART);
        }

        return !uart_tmo;
}

#if dg_configNVMS_ADAPTER
static uint16_t push_partition_entry_name(uint8_t *ram, nvms_partition_id_t id)
{
#define _STR_(token) #token
#define _PUSH_ENUM_AS_STRING_2_RAM_(_enum_)      \
        do {                                     \
                len = strlen(_STR_(_enum_)) + 1; \
                memcpy(ram, _STR_(_enum_), len); \
        } while (0);
#define _ALIGN32_(size) (((size) + 3) & (~0x3))

        uint16_t len;

        switch (id) {
        case NVMS_FIRMWARE_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_FIRMWARE_PART);
                break;

        case NVMS_PARAM_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PARAM_PART);
                break;

        case NVMS_BIN_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_BIN_PART);
                break;

        case NVMS_LOG_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_LOG_PART);
                break;

        case NVMS_GENERIC_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_GENERIC_PART);
                break;

        case NVMS_PLATFORM_PARAMS_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PLATFORM_PARAMS_PART);
                break;

        case NVMS_PARTITION_TABLE:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PARTITION_TABLE);
                break;

        case NVMS_FW_EXEC_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_FW_EXEC_PART);
                break;

        case NVMS_FW_UPDATE_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_FW_UPDATE_PART);
                break;

        case NVMS_PRODUCT_HEADER_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PRODUCT_HEADER_PART);
                break;

        case NVMS_IMAGE_HEADER_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_IMAGE_HEADER_PART);
                break;

        default:
                _PUSH_ENUM_AS_STRING_2_RAM_(UNKNOWN_PARTITION_ID);
        }

        /* len should be multiple of 4 to avoid unaligned loads/stores */
        return _ALIGN32_(len);
#undef _STR_
#undef _PUSH_ENUM_AS_STRING_2_RAM_
#undef _ALIGN32_
}

static uint16_t piggy_back_partition_entry(uint8_t *ram, const partition_entry_t *flash_entry)
{
        cmd_partition_entry_t *ram_entry = (cmd_partition_entry_t *)(ram);
        uint8_t *ram_str = &(ram_entry->name.str);
        ram_entry->start_address = flash_entry->start_address;
        ram_entry->size = flash_entry->size;
        ram_entry->sector_size = AD_FLASH_GET_SECTOR_SIZE(flash_entry->start_address);
        ram_entry->type = flash_entry->type;
        ram_entry->name.len = push_partition_entry_name((ram_str) , flash_entry->type);
        return sizeof(cmd_partition_entry_t) + ram_entry->name.len;
}

static bool piggy_back_partition_table(uint8_t *ram)
{
        uint16_t entry_size = 0;
        cmd_partition_table_t *ram_table = (cmd_partition_table_t *)ram;
        cmd_partition_entry_t *ram_entry = &(ram_table->entry);
        partition_entry_t flash_entry;
        uint32_t flash_addr = PARTITION_TABLE_ADDR;
        ram_table->len = 0;

        do {
                ad_flash_read(flash_addr, (uint8_t *)&flash_entry, sizeof(partition_entry_t));
                if (flash_entry.type != 0xFF && flash_entry.type != 0 && flash_entry.magic == 0xEA &&
                                flash_entry.valid == 0xFF) {
                        entry_size = piggy_back_partition_entry((uint8_t *)ram_entry, &flash_entry);

                        ram_entry = (cmd_partition_entry_t *)((uint8_t *)ram_entry + entry_size);
                        ram_table->len += entry_size;
                }

                flash_addr += sizeof(partition_entry_t);
        } while (flash_entry.type != 0xFF);
        ram_table->len += sizeof(cmd_partition_table_t);

        return true;
}
#endif

static size_t safe_flash_write(uint32_t flash_addr, const uint8_t *buf, size_t length)
{
        size_t written = 0;
        uint32_t sector_size = AD_FLASH_GET_SECTOR_SIZE(flash_addr);

        while (written < length) {
                uint32_t sector_start = flash_addr & ~(sector_size - 1);
                uint32_t sector_offset = flash_addr - sector_start;
                uint32_t chunk_size = sector_size - sector_offset;
                int off;

                if (chunk_size > length - written) {
                        chunk_size = length - written;
                }

                off = ad_flash_update_possible(flash_addr, buf, chunk_size);

                /* No write needed in this sector, same data */
                if (off == (int) chunk_size) {
                        goto advance;
                }

                /* Write without erase possible */
                if (off >= 0) {
                        ad_flash_write(flash_addr + off, buf + off, chunk_size - off);
                        goto advance;
                }

                /* If entire sector is to be written, no need to read old data */
                if (flash_addr == sector_start && chunk_size == sector_size) {
                        ad_flash_erase_region(flash_addr, sector_size);
                        ad_flash_write(flash_addr, buf, sector_size);
                } else {
                        ad_flash_read(sector_start, array, sector_size);

                        /* Overwrite old data with new one */
                        memcpy(array + sector_offset, buf, chunk_size);

                        /* Erase and write entire sector */
                        ad_flash_erase_region(sector_start, sector_size);
                        ad_flash_write(sector_start, array, sector_size);
                }
advance:
                written += chunk_size;
                buf += chunk_size;
                flash_addr += chunk_size;
        }

        return written;
}

static bool flash_content_cmp(uint32_t flash_addr, size_t length, uint8_t *read_buf, const uint8_t *buf)
{
        /* Read back data and verify */
        if (ad_flash_read(flash_addr, read_buf, length) != length) {
                return false;
        }

        return memcmp(buf, read_buf, length) == 0;
}

static bool prod_info_print_to_buffer(cmd_product_info_t *product_info, const char* format, ...)
{
        int16_t info_size;
        va_list argptr;

        va_start(argptr, format);
        info_size = vsnprintf(&product_info->str + product_info->len,
                                input_buffer_size - product_info->len - sizeof(product_info->len),
                                format, argptr);
        va_end(argptr);

        if (info_size < 0) {
                return false;
        }

        product_info->len += info_size;

        return true;
}

static bool product_info_helper(uint8_t *info)
{
        cmd_product_info_t *product_info = (cmd_product_info_t *)info;
        const char *res;

        product_info->len = 0;

        /* Retrieve and compose device classification attributes */

        if (false == prod_info_print_to_buffer(product_info,
                                "PRODUCT INFORMATION:\nDevice classification attributes:\n")) {
                return false;
        }

        res = UNDETERMINED;
        if (hw_sys_device_info_check(DEVICE_FAMILY_MASK, DA1470X)) {
                res = "DA1470x";
        }
        if (false == prod_info_print_to_buffer(product_info,
                                "Device family = %s\n", res)) {
                return false;
        }

        res = UNDETERMINED;
        if (hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_2798)) {
                res = "D2798";
        }
        else if (hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_3107)) {
                res = "D3107";
        }
        if (false == prod_info_print_to_buffer(product_info,
                                "Device chip ID = %s\n", res)) {
                return false;
        }

        res = UNDETERMINED;
        if (hw_sys_device_info_check(DEVICE_VARIANT_MASK, DA14701)) {
                res = "DA14701";
        }
        else if (hw_sys_device_info_check(DEVICE_VARIANT_MASK, DA14705)) {
                res = "DA14705";
        }
        else if (hw_sys_device_info_check(DEVICE_VARIANT_MASK, DA14706)) {
                res = "DA14706";
        }
        else if (hw_sys_device_info_check(DEVICE_VARIANT_MASK, DA14708)) {
                res = "DA14708";
        }
        if (false == prod_info_print_to_buffer(product_info,
                                "Device variant = %s\n", res)) {
                return false;
        }

        if (false == prod_info_print_to_buffer(product_info,
                                "Device version (revision|SWC) = ")) {
                return false;
        }

        res = UNDETERMINED;
        if (hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_A)) {
                res = "A";
        }
        else if (hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_B)) {
                res = "B";
        }

        if (false == prod_info_print_to_buffer(product_info,
                                "%s", res)) {
                return false;
        }

        res = UNDETERMINED;
        if (hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_0)) {
                res = "0";
        }
        else if (hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_1)) {
                res = "1";
        }
        if (false == prod_info_print_to_buffer(product_info,
                                "%s\n\n", res)) {
                return false;
        }

        /* Retrieve production information attributes as stored in the corresponding TCS group */
        #define TCS_PROD_INFO_LEN       2
        uint32_t *values = NULL;
        uint32_t production_info[TCS_PROD_INFO_LEN] = {0};
        uint8_t size = 0;

        sys_tcs_get_custom_values(SYS_TCS_GROUP_PROD_INFO, &values, &size);

        /* Check that the corresponding TCS group returned the expected number of entries */
        if (size != TCS_PROD_INFO_LEN) {
                return false;
        }

        memcpy(production_info, values, sizeof(production_info));

        /* Extract the production package coding that is stored in Byte 7 of the TCS group */
        uint8_t production_package_raw = (production_info[1] >> 24) & 0xFF;

        if (false == prod_info_print_to_buffer(product_info,
                                "Production layout information:\n")) {
                return false;
        }

        switch (production_package_raw) {
        case 0x00:
                res = "VFBGA142";
                break;
        default:
                res = UNDETERMINED;
                break;
        }

        if (false == prod_info_print_to_buffer(product_info,
                                "Package = %s\n", res)) {
                return false;
        }

        if (false == prod_info_print_to_buffer(product_info,
                                "Production testing information:\nTimestamp = 0x%08lX\n",
                production_info[0])) {
                return false;
        }

        /* Add to the string length one byte for the '\0' character */
        product_info->len = product_info->len + 1;

        product_info->len += sizeof(product_info->len);
        #undef TCS_PROD_INFO_LEN

        return true;
}

/*
 * Wrapper for writing from RAM buffer to FLASH memory. Verification of written data is performed
 * only if 'read_buf' pointer is not NULL.
 */
static bool flash_write(uint32_t flash_addr, const uint8_t *ram_ptr, size_t length, uint8_t *read_buf)
{
        HW_QSPIC_ID id = HW_QSPIC;


#if dg_configUSE_HW_QSPI2
        if (flash_addr >= QSPI_MEM2_VIRTUAL_BASE_ADDR) {
                id = HW_QSPIC2;
        }
#endif

        if (flash_addr + length > qspi_get_device_size(id) + QSPI_MEM1_VIRTUAL_BASE_ADDR) {
                /* Length of data to be written exceeds the QSPI FLASH memory end address so
                 * don't write the data. */
                return false;
        }

        if (qspi_is_ram_device(id)) {
                /* We can write data directly. No need to check if write is possible */
                if (ad_flash_write(flash_addr, ram_ptr, length) != length) {
                        return false;
                }
        } else {
                if (safe_flash_write(flash_addr, ram_ptr, length) != length) {
                        return false;
                }
        }

        return read_buf ? flash_content_cmp(flash_addr, length, read_buf, ram_ptr) : true;
}


/* handler for 'send data to RAM' */
static bool cmd_send_to_ram(HANDLER_OP hop)
{
        struct cmdhdr_send_to_ram *hdr = &cmd_state.hdr.send_to_ram;

        switch (hop) {
        case HOP_INIT:
                /* some payload is required, otherwise there's nothing to write */
                return cmd_state.data_len > 0;

        case HOP_HEADER:
                /*
                 * When data is written to RAM there is no need to store it in buffer
                 * and then copy to destination. Change address of data from buffer
                 * which is preset to what command wants;
                 */
                cmd_state.data = hdr->ptr;
                /*
                 * When address is explicitly set to ADDRESS_TMP or lays in range
                 * assigned for buffer, convert it to real address in RAM.
                 * hdr->ptr is not modified since it is needed for CRC calculation.
                 */
                if (!check_ram_addr((uint32_t) cmd_state.data, cmd_state.data_len)) {
                        return false;
                }

                cmd_state.data = (void *)translate_ram_addr((uint32_t)cmd_state.data);
                return true;

        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* Data was already put in correct place */

                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'read memory region from device' */
static bool cmd_read_from_ram(HANDLER_OP hop)
{
        struct cmdhdr_read_from_ram *hdr = &cmd_state.hdr.read_from_ram;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* nothing to do */
                return true;

        case HOP_EXEC:
                /* nothing to do */
                return true;

        case HOP_SEND_LEN:
                xmit_data(&hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                if (!check_ram_addr(hdr->ptr, hdr->len)) {
                        return false;
                }

                hdr->ptr = translate_ram_addr(hdr->ptr);
                xmit_data((const void *)hdr->ptr, hdr->len);

                return true;
        }
        return false;
}

/* handler for 'write RAM region to QSPI' */
static bool cmd_write_ram_to_qspi(HANDLER_OP hop)
{
        struct cmdhdr_write_ram_to_qspi *hdr = &cmd_state.hdr.write_ram_to_qspi;
        uint32_t read_buf_addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                if (!check_ram_addr(hdr->ptr, hdr->len)) {
                        return false;
                }

                /* check for 'magic' address */
                hdr->ptr = translate_ram_addr(hdr->ptr);

                return true;

        case HOP_EXEC:
#if VERIFY_QSPI_WRITE
                /* Read buffer is placed after write buffer and has the same length */
                if (!check_ram_addr(hdr->ptr, hdr->len * 2)) {
                        return false;
                }

                if (is_valid_ptr_in_inputbuffer(hdr->ptr)) {
                        read_buf_addr = hdr->ptr + hdr->len;       // move after written data
                } else {
                        /* Write is not performed from data buffer - used it for verification */
                        read_buf_addr = translate_ram_addr(ADDRESS_TMP);
                }

#else
                read_buf_addr = 0;
#endif /* VERIFY_QSPI_WRITE */

                hdr->addr += QSPI_MEM1_VIRTUAL_BASE_ADDR;

                return flash_write(hdr->addr, (const uint8_t *) hdr->ptr, hdr->len, (uint8_t *) read_buf_addr);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'erase region of flash' */
static bool cmd_erase_qspi(HANDLER_OP hop)
{
        struct cmdhdr_erase_qspi *hdr = &cmd_state.hdr.erase_qspi;
        uint32_t addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return hdr->len > 0;

        case HOP_EXEC:
                addr = hdr->addr;

                addr += QSPI_MEM1_VIRTUAL_BASE_ADDR;

                return ad_flash_erase_region(addr, hdr->len);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

void __attribute__((section("reboot_section"), noinline)) move_to_0_and_boot(void *start, size_t size)
{
        uint32_t *src = start;
        uint32_t *dst = (uint32_t *) MEMORY_SYSRAM3_BASE;
        int s = (int) ((size + 4) >> 2);
        int i;

        /* Perform a deinitialization */
        hw_clk_set_rchs_mode(RCHS_32);
        hw_clk_set_sysclk(SYS_CLK_IS_RCHS);
        /*
         * Disable interrupts to prevent calling handlers which will be replaced with new
         * application code. Also do not restore them (will be restored duirng reset procedure)
         * because pending interrupts will be handled immediately and will corrupt image with
         * stack data.
         */
        __disable_irq();

        for (i = 0; i < s; ++i) {
                dst[i] = src[i];
        }

        REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, SW_RESET);

        /*
         * Wait for reset in the infinite loop (this part of code shouldn't be reached due to
         * the triggered SW reset).
         */
        while (1);
}

/* handler for 'execute code on device' */
static bool cmd_execute_code(HANDLER_OP hop)
{
        struct cmdhdr_execute_code *hdr = &cmd_state.hdr.execute_code;
        void (* func) (void);

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                // ack only if address in within mapped memory
                //
                //            start addr   end addr
                // Remapped   00000000     04000000
                // ROM        07F00000     07F40000
                // OTPC       07F40000     07F80000
                // OTP        07F80000     07FC0000
                // DataRAM    07FC0000     07FE0000
                // QSPI       08000000     0BF00000
                // Buffer     80000000     80024000
                return true;
        case HOP_EXEC:
                if (!check_ram_addr(hdr->addr, 1)) {
                        return false;
                }

                /* 'xmit_ack' should be used here - function could not reach the end */
                xmit_ack();

                hdr->addr = translate_ram_addr(hdr->addr);

                /* make sure lsb is 1 (thumb mode) */
                func = (void *) (hdr->addr | 1);
                if ((uint32_t) func == ((uint32_t)&__inputbuffer_start) + 1) {
                        move_to_0_and_boot(&__inputbuffer_start,
                                                        &__inputbuffer_end - &__inputbuffer_start);
                } else {
                        func();
                }
                return true; // we actually should never reach this

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'write to OTP' */
static bool cmd_write_otp(HANDLER_OP hop)
{
        struct cmdhdr_write_otp *hdr = &cmd_state.hdr.write_otp;

        switch (hop) {
        case HOP_INIT:
                /* make sure data to be written length is multiply of word size (4 bytes) */
                return (cmd_state.data_len > 0) && ((cmd_state.data_len & 0x03) == 0);

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* make sure cell address is valid */
                return hdr->addr < HW_OTP_CELL_NUM;

        case HOP_EXEC:
                hw_otpc_prog(cmd_state.data, hdr->addr,
                        cmd_state.data_len >> 2);
                return true;
        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }
        return false;
}

/* handler for 'read from OTP' */
static bool cmd_read_otp(HANDLER_OP hop)
{
        struct cmdhdr_read_otp *hdr = &cmd_state.hdr.read_otp;
        static uint16_t size;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                size = hdr->len * sizeof(uint32_t);
                return hdr->addr < HW_OTP_CELL_NUM;

        case HOP_EXEC:
                hw_otpc_read(cmd_state.data, hdr->addr, hdr->len);
                return true;/* hw_otpc_read returns void*/

        case HOP_SEND_LEN:
                xmit_data(&size, sizeof(size));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, size);
                return true;
        }
        return false;
}

typedef __PACKED_STRUCT {
        bool    driver_configured;
        uint8_t manufacturer_id;
        uint8_t device_type;
        uint8_t density;
} qspi_status_t;

static bool get_qspi_state(uint8_t id, uint16_t *len, uint8_t *buf)
{
        qspi_status_t *qspi_status = (qspi_status_t *)buf;
        HW_QSPIC_ID hw_qspi_id;
        HW_QSPI_DIV hw_qspi_div;

        switch (id) {
        case 0:
                hw_qspi_id = HW_QSPIC;
                break;
#if dg_configUSE_HW_QSPI2
        case 1:
                hw_qspi_id = HW_QSPIC2;
                break;
#endif  /* dg_configUSE_HW_QSPI2 */
        default:
                return false;
        }

        if (qspi_get_config(hw_qspi_id, &qspi_status->manufacturer_id,
                                                &qspi_status->device_type, &qspi_status->density)) {
                (*len) = sizeof(*qspi_status);
                qspi_status->driver_configured = true;

                return true;
        }

        /*
         * FLASH memory is not connected or not supported by automode - decrease QSPI clock as much
         * as possible for better stability of communication. Current clock divider will be restored.
         */
        hw_qspi_div = hw_qspi_get_div(hw_qspi_id);
        hw_qspi_set_div(hw_qspi_id, HW_QSPI_DIV_8);

        if (qspi_read_flash_jedec_id(hw_qspi_id, &qspi_status->manufacturer_id,
                                                &qspi_status->device_type, &qspi_status->density)) {
                (*len) = sizeof(*qspi_status);
                qspi_status->driver_configured = false;
                hw_qspi_set_div(hw_qspi_id, hw_qspi_div);

                return true;
        }

        hw_qspi_set_div(hw_qspi_id, hw_qspi_div);

        return false;
}

/* handler for 'init_qspi' */
static bool cmd_get_qspi_state(HANDLER_OP hop)
{
        struct cmdhdr_get_qspi_state *hdr = &cmd_state.hdr.get_qspi_state;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                /* nothing to do */
                return true;

        case HOP_DATA:
                /* nothing to do */
                return true;

        case HOP_EXEC:
                return get_qspi_state(hdr->id, &cmd_state.data_len, cmd_state.data);

        case HOP_SEND_LEN:
                xmit_data(&cmd_state.data_len, sizeof(cmd_state.data_len));
                return true;

        case HOP_SEND_DATA:
                if (!cmd_state.data_len) {
                        return false;
                }

                xmit_data(cmd_state.data, cmd_state.data_len);
                return true;
        }

        return false;
}

/* Handler for 'gpio_wd'. */
static bool cmd_gpio_wd(HANDLER_OP hop)
{
        struct cmdhdr_gpio_wd *hdr = &cmd_state.hdr.gpio_wd;
        static HW_GPIO_PORT port;
        static HW_GPIO_PIN pin;
        static uint8_t volt_rail;

        timer_config timer_cfg = {
                .clk_src = HW_TIMER_CLK_SRC_EXT,
                .prescaler = 0x1f, // 32MHz / (31 + 1) = 1MHz
                .mode = HW_TIMER_MODE_TIMER,
                .timer = {
                        .direction = HW_TIMER_DIR_UP,
                        .reload_val = 15000, // interrupt every 15ms
                },
        };

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* Transform the input. */
                port = GPIO_PAD_TO_PORT(hdr->gpio_pad);
                pin = GPIO_PAD_TO_PIN(hdr->gpio_pad);
                volt_rail = hdr->gpio_lvl;

                /* Validate the GPIO port */
                if (port < HW_GPIO_PORT_0 || port >= HW_GPIO_PORT_MAX) {
                        return false;
                }

                /* Validate the GPIO pin */
                if (pin < HW_GPIO_PIN_0 || pin >= hw_gpio_port_num_pins[port]) {
                        return false;
                }

                /* Validate the GPIO voltage rail. 0 = 3.3V, 1 = 1.8V. */
                if (volt_rail > 1) {
                        return false;
                }

                return true;

        case HOP_EXEC:
                /* Disable timer here - avoid timer callback calling */
                hw_timer_disable(HW_TIMER2);

                /* Update global variable */
                gpio_wd_port = port;
                gpio_wd_pin = pin;

                /* Configure the pin voltage rail and function. */
                hw_gpio_configure_pin_power(port, pin, volt_rail ? HW_GPIO_POWER_VDD1V8P :
                                                                                HW_GPIO_POWER_V33);
                hw_gpio_set_pin_function(port, pin, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO);

                /* Reset counter used in callback. Initialize and start timer. */
                gpio_wd_timer_cnt = 0;
                hw_timer_init(HW_TIMER2, &timer_cfg);
                hw_timer_register_int(HW_TIMER2, timer_gpio_wd_cb);
                hw_timer_enable(HW_TIMER2);

                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'read QSPI' */
static bool cmd_read_qspi(HANDLER_OP hop)
{
        struct cmdhdr_read_qspi *hdr = &cmd_state.hdr.read_qspi;
        uint32_t addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* there's no payload for this command so we can safely read into buffer */
                addr = hdr->addr;

                addr += QSPI_MEM1_VIRTUAL_BASE_ADDR;

                return ad_flash_read(addr, cmd_state.data, hdr->len) == hdr->len;

        case HOP_SEND_LEN:
                xmit_data(&hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, hdr->len);
                return true;
        }

        return false;
}

/* handler for 'get_version on device' */
static bool cmd_get_version(HANDLER_OP hop)
{
        /* Send without the last character '\0' */
        const uint16_t msg_len = sizeof(VERSION_STR) - 1;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* nothing to do */
                return true;

        case HOP_EXEC:
                /* nothing to do */
                return true;

        case HOP_SEND_LEN:
                /* send length */
                xmit_data(&msg_len , sizeof(msg_len));
                return true;

        case HOP_SEND_DATA:
                /* send data */
                xmit_data(VERSION_STR, msg_len);
                return true;
        }

        return false;
}

static bool cmd_is_empty_qspi(HANDLER_OP hop)
{
        struct cmdhdr_is_empty_qspi *hdr = &cmd_state.hdr.is_empty_qspi;
        static int32_t return_val;
        uint32_t i = 0;
        uint32_t tmp_addr;
        uint32_t tmp_addr2;
        uint32_t start_addr;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* something is wrong - do not execute command if size is zero */
                return hdr->size != 0;

        case HOP_EXEC:
                /* Check read and pattern buffer addresses */
                if (!check_ram_addr(ADDRESS_TMP, 2 * IS_EMPTY_CHECK_SIZE)) {
                        return false;
                }

                tmp_addr = translate_ram_addr(ADDRESS_TMP); // get address to big buffer
                memset((uint8_t *) tmp_addr, 0xFF, IS_EMPTY_CHECK_SIZE);  // FF pattern
                tmp_addr2 = tmp_addr + IS_EMPTY_CHECK_SIZE; // address for read values
                cmd_state.data_len = sizeof(return_val);
                cmd_state.data = &return_val;
                start_addr = hdr->start_address;

                start_addr += QSPI_MEM1_VIRTUAL_BASE_ADDR;

                while (i < hdr->size) {
                        const uint32_t read_len = ((hdr->size - i) > IS_EMPTY_CHECK_SIZE ?
                                                        IS_EMPTY_CHECK_SIZE : (hdr->size - i));

                        if (ad_flash_read(start_addr + i, (uint8_t *) tmp_addr2, read_len) !=
                                                                                        read_len) {
                                return false;
                        }

                        if (memcmp((uint8_t *) tmp_addr, (uint8_t *) tmp_addr2, read_len)) {
                                uint32_t j;

                                for (j = 0; j < read_len; j++) {
                                        if (*(uint8_t *) (tmp_addr2 + j) != 0xFF) {
                                                break;
                                        }
                                }

                                return_val = (int32_t) (-1 * (i + j));
                                memcpy((uint8_t *) tmp_addr, &return_val, sizeof(return_val));
                                return true;
                        }

                        i += read_len;
                }

                return_val = hdr->size;
                memcpy((uint8_t *) tmp_addr, &return_val, sizeof(return_val));
                return true;
        case HOP_SEND_LEN:
                xmit_data(&cmd_state.data_len , sizeof(cmd_state.data_len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, cmd_state.data_len);
                return true;
        }

        return false;
}

#if dg_configNVMS_ADAPTER
static bool cmd_read_partition_table(HANDLER_OP hop)
{
        uint8 *ram = cmd_state.data;
        cmd_partition_table_t  *ram_table = (cmd_partition_table_t *)ram;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                return piggy_back_partition_table(cmd_state.data);

        case HOP_SEND_LEN:
                xmit_data(&ram_table->len, sizeof(ram_table->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(ram, ram_table->len);
                return true;
        }

        return false;
}

static bool cmd_read_partition(HANDLER_OP hop)
{
        struct cmdhdr_read_partition *hdr = &cmd_state.hdr.read_partition;
        nvms_t nvms;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                if (!ad_nvms_init_called) {
                        ad_nvms_init_called = true;
                        ad_nvms_init();
                }
                nvms = ad_nvms_open(hdr->id);

                return ad_nvms_read(nvms, hdr->addr, cmd_state.data, hdr->len) >= 0;

        case HOP_SEND_LEN:
                xmit_data(&hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, hdr->len);
                return true;
        }

        return false;
}

static bool cmd_write_partition(HANDLER_OP hop)
{
        struct cmdhdr_write_partition *hdr = &cmd_state.hdr.write_partition;
        nvms_t nvms;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* check for 'magic' address */
                if (!check_ram_addr(hdr->ptr, hdr->len)) {
                        return false;
                }

                hdr->ptr = translate_ram_addr(hdr->ptr);

                return true;

        case HOP_EXEC:
                if (!ad_nvms_init_called) {
                        ad_nvms_init_called = true;
                        ad_nvms_init();
                }
                nvms = ad_nvms_open(hdr->id);

                return ad_nvms_write(nvms, hdr->addr, (const uint8_t *) hdr->ptr, hdr->len) >= 0;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}
#endif

static bool cmd_chip_erase_qspi(HANDLER_OP hop)
{
        struct cmdhdr_chip_erase_qspi *hdr = &cmd_state.hdr.chip_erase_qspi;
        uint32_t addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return true;

        case HOP_EXEC:
                addr = hdr->addr;


                return ad_flash_chip_erase_by_addr(addr);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* This command is needed only by GDB Server interface */
static bool cmd_dummy(HANDLER_OP hop)
{
        char live_str[] = UARTBOOT_LIVE_MARKER;
        uint32_t tmp_addr;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                if (!check_ram_addr(ADDRESS_TMP, ARRAY_LENGTH(live_str))) {
                     return false;
                }

                tmp_addr = translate_ram_addr(ADDRESS_TMP); // get address to big buffer
                memcpy((uint8_t *) tmp_addr, live_str, ARRAY_LENGTH(live_str));
                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                return false;
        }

        return false;
}

static bool cmd_direct_write_to_qspi(HANDLER_OP hop)
{
        struct cmdhdr_direct_write_qspi *hdr = &cmd_state.hdr.direct_write_qspi;
        uint8_t *read_buffer;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len > 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* Read back buffer is placed just after data */
                read_buffer = ((uint8_t *)cmd_state.data) + cmd_state.data_len;

                hdr->addr += QSPI_MEM1_VIRTUAL_BASE_ADDR;

                return flash_write(hdr->addr, cmd_state.data, cmd_state.data_len,
                                                        hdr->read_back_verify ? read_buffer : NULL);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}


/*
 * Wrapper for writing from RAM buffer to OQSPI FLASH. Verification of written data is performed
 * only if 'read_buf' pointer is not NULL.
 */
static bool oqspi_write(uint32_t flash_addr, const uint8_t *ram_ptr, size_t length, uint8_t *read_buf)
{
        if (flash_addr + length > oqspi_get_device_size() + OQSPI_MEM1_VIRTUAL_BASE_ADDR) {
                /* Length of data to be written exceeds the OQSPI FLASH memory end address so
                 * don't write the data. */
                return false;
        }

        if (safe_flash_write(flash_addr, ram_ptr, length) != length) {
                return false;
        }

        return read_buf ? flash_content_cmp(flash_addr, length, read_buf, ram_ptr) : true;
}

/* handler for 'write RAM region to OQSPI' */
static bool cmd_write_ram_to_oqspi(HANDLER_OP hop)
{
        struct cmdhdr_write_ram_to_oqspi *hdr = &cmd_state.hdr.write_ram_to_oqspi;
        uint32_t read_buf_addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                if (!check_ram_addr(hdr->ptr, hdr->len)) {
                        return false;
                }

                /* check for 'magic' address */
                hdr->ptr = translate_ram_addr(hdr->ptr);

                return true;

        case HOP_EXEC:
#if VERIFY_OQSPI_WRITE
                /* Read buffer is placed after write buffer and has the same length */
                if (!check_ram_addr(hdr->ptr, hdr->len * 2)) {
                        return false;
                }

                if (is_valid_ptr_in_inputbuffer(hdr->ptr)) {
                        read_buf_addr = hdr->ptr + hdr->len;       // move after written data
                } else {
                        /* Write is not performed from data buffer - used it for verification */
                        read_buf_addr = translate_ram_addr(ADDRESS_TMP);
                }

#else
                read_buf_addr = 0;
#endif /* VERIFY_OQSPI_WRITE */

                return oqspi_write(hdr->addr, (const uint8_t *) hdr->ptr, hdr->len, (uint8_t *) read_buf_addr);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'erase region of flash' */
static bool cmd_erase_oqspi(HANDLER_OP hop)
{
        struct cmdhdr_erase_oqspi *hdr = &cmd_state.hdr.erase_oqspi;
        uint32_t addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return hdr->len > 0;

        case HOP_EXEC:
                addr = hdr->addr;

                return ad_flash_erase_region(addr, hdr->len);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'read OQSPI' */
static bool cmd_read_oqspi(HANDLER_OP hop)
{
        struct cmdhdr_read_oqspi *hdr = &cmd_state.hdr.read_oqspi;
        uint32_t addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* there's no payload for this command so we can safely read into buffer */
                addr = hdr->addr;

                return ad_flash_read(addr, cmd_state.data, hdr->len) == hdr->len;

        case HOP_SEND_LEN:
                xmit_data(&hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, hdr->len);
                return true;
        }

        return false;
}

static bool cmd_chip_erase_oqspi(HANDLER_OP hop)
{
        struct cmdhdr_chip_erase_oqspi *hdr = &cmd_state.hdr.chip_erase_oqspi;
        uint32_t addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return true;

        case HOP_EXEC:
                addr = hdr->addr;
                return ad_flash_chip_erase_by_addr(addr);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

static bool cmd_is_empty_oqspi(HANDLER_OP hop)
{
        struct cmdhdr_is_empty_oqspi *hdr = &cmd_state.hdr.is_empty_oqspi;
        static int32_t return_val;
        uint32_t i = 0;
        uint32_t tmp_addr;
        uint32_t tmp_addr2;
        uint32_t start_addr;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* something is wrong - do not execute command if size is zero */
                return hdr->size != 0;

        case HOP_EXEC:
                /* Check read and pattern buffer addresses */
                if (!check_ram_addr(ADDRESS_TMP, 2 * IS_EMPTY_CHECK_SIZE)) {
                        return false;
                }

                tmp_addr = translate_ram_addr(ADDRESS_TMP); // get address to big buffer
                memset((uint8_t *) tmp_addr, 0xFF, IS_EMPTY_CHECK_SIZE);  // FF pattern
                tmp_addr2 = tmp_addr + IS_EMPTY_CHECK_SIZE; // address for read values
                cmd_state.data_len = sizeof(return_val);
                cmd_state.data = &return_val;
                start_addr = hdr->start_address;

                while (i < hdr->size) {
                        const uint32_t read_len = ((hdr->size - i) > IS_EMPTY_CHECK_SIZE ?
                                                        IS_EMPTY_CHECK_SIZE : (hdr->size - i));

                        if (ad_flash_read(start_addr + i, (uint8_t *) tmp_addr2, read_len) !=
                                                                                        read_len) {
                                return false;
                        }

                        if (memcmp((uint8_t *) tmp_addr, (uint8_t *) tmp_addr2, read_len)) {
                                uint32_t j;

                                for (j = 0; j < read_len; j++) {
                                        if (*(uint8_t *) (tmp_addr2 + j) != 0xFF) {
                                                break;
                                        }
                                }

                                return_val = (int32_t) (-1 * (i + j));
                                memcpy((uint8_t *) tmp_addr, &return_val, sizeof(return_val));
                                return true;
                        }

                        i += read_len;
                }

                return_val = hdr->size;
                memcpy((uint8_t *) tmp_addr, &return_val, sizeof(return_val));
                return true;
        case HOP_SEND_LEN:
                xmit_data(&cmd_state.data_len , sizeof(cmd_state.data_len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, cmd_state.data_len);
                return true;
        }

        return false;
}

typedef __PACKED_STRUCT {
        bool    driver_configured;
        uint8_t manufacturer_id;
        uint8_t device_type;
        uint8_t density;
} oqspi_status_t;

static bool get_oqspi_state(uint16_t *len, uint8_t *buf)
{
        jedec_id_t jedec;
        oqspi_status_t *oqspi_status = (oqspi_status_t *) buf;

        oqspi_status->driver_configured = oqspi_get_config(&jedec);

        if (oqspi_status->driver_configured) {
                oqspi_status->manufacturer_id = jedec.manufacturer_id;
                oqspi_status->device_type = jedec.type;
                oqspi_status->density = jedec.density;
                (*len) = sizeof(*oqspi_status);
        }

        return oqspi_status->driver_configured;
}

/* handler for 'init_oqspi' */
static bool cmd_get_oqspi_state(HANDLER_OP hop)
{
        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                /* nothing to do */
                return true;

        case HOP_DATA:
                /* nothing to do */
                return true;

        case HOP_EXEC:
                return get_oqspi_state(&cmd_state.data_len, cmd_state.data);

        case HOP_SEND_LEN:
                xmit_data(&cmd_state.data_len, sizeof(cmd_state.data_len));
                return true;

        case HOP_SEND_DATA:
                if (!cmd_state.data_len) {
                        return false;
                }

                xmit_data(cmd_state.data, cmd_state.data_len);
                return true;
        }

        return false;
}

static bool cmd_direct_write_to_oqspi(HANDLER_OP hop)
{
        struct cmdhdr_direct_write_oqspi *hdr = &cmd_state.hdr.direct_write_oqspi;
        uint8_t *read_buffer;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len > 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* Read back buffer is placed just after data */
                read_buffer = ((uint8_t *)cmd_state.data) + cmd_state.data_len;

                return oqspi_write(hdr->addr, cmd_state.data, cmd_state.data_len,
                                                        hdr->read_back_verify ? read_buffer : NULL);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'get_product_info on device' */
static bool cmd_get_product_info(HANDLER_OP hop)
{
        uint8 *info = (uint8_t *)cmd_state.data;
        cmd_product_info_t *product_info = (cmd_product_info_t *)info;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:

                return product_info_helper(cmd_state.data);

        case HOP_SEND_LEN:
                xmit_data((void *) &product_info->len, sizeof(product_info->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(info, product_info->len);
                return true;
        }

        return false;
}

static bool convert_baudrate(uint32_t value, HW_UART_BAUDRATE *baudrate)
{
        switch (value) {
        case 4800:
                *baudrate = HW_UART_BAUDRATE_4800;
                break;
        case 9600:
                *baudrate = HW_UART_BAUDRATE_9600;
                break;
        case 14400:
                *baudrate = HW_UART_BAUDRATE_14400;
                break;
        case 19200:
                *baudrate = HW_UART_BAUDRATE_19200;
                break;
        case 28800:
                *baudrate = HW_UART_BAUDRATE_28800;
                break;
        case 38400:
                *baudrate = HW_UART_BAUDRATE_38400;
                break;
        case 57600:
                *baudrate = HW_UART_BAUDRATE_57600;
                break;
        case 115200:
                *baudrate = HW_UART_BAUDRATE_115200;
                break;
        case 230400:
                *baudrate = HW_UART_BAUDRATE_230400;
                break;
        case 500000:
                *baudrate = HW_UART_BAUDRATE_500000;
                break;
        case 1000000:
                *baudrate = HW_UART_BAUDRATE_1000000;
                break;
        default:
                return false;
        }

        return true;
}

static bool cmd_change_baudrate(HANDLER_OP hop)
{
        struct cmdhdr_change_baudrate *hdr = &cmd_state.hdr.change_baudrate;
        static HW_UART_BAUDRATE baudrate;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                // Update init baudrate
                return convert_baudrate(hdr->baudrate, &baudrate);

        case HOP_EXEC:
                UART_INIT.baud_rate = baudrate;
                hw_uart_reinit(BOOTUART, &UART_INIT);
                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* provided by linker script */
extern char __patchable_params;

static void init(void)
{
                timer_config t_cfg = {
                        .clk_src = HW_TIMER_CLK_SRC_EXT,
                        .prescaler = 0x1f, // 32MHz / (31 + 1) = 1MHz

                        .timer = {
                                .direction = HW_TIMER_DIR_UP,
                                .reload_val = 999999, // interrupt every 1s
                        },
                };
        uint32_t *pparams = (void*)&__patchable_params;
        HW_GPIO_PORT tx_port, rx_port;
        HW_GPIO_PIN tx_pin, rx_pin;

        /*
         * get UART parameters from patchable area, if their value is not 0xffffffff,
         * or else, use the CFG_* values
         */
        if (*pparams != 0xffffffff) {
                tx_port = (HW_GPIO_PORT)*pparams;
        } else {
                tx_port = CFG_GPIO_BOOTUART_TX_PORT;
        }
        if (*(++pparams) != 0xffffffff) {
                tx_pin = (HW_GPIO_PIN)*pparams;
        } else {
                tx_pin = CFG_GPIO_BOOTUART_TX_PIN;
        }
        if (*(++pparams) != 0xffffffff) {
                rx_port = (HW_GPIO_PORT)*pparams;
        } else {
                rx_port = CFG_GPIO_BOOTUART_RX_PORT;
        }
        if (*(++pparams) != 0xffffffff) {
                rx_pin = (HW_GPIO_PIN)*pparams;
        } else {
                rx_pin = CFG_GPIO_BOOTUART_RX_PIN;
        }
        if (*(++pparams) != 0xffffffff) {
                convert_baudrate(*pparams, &UART_INIT.baud_rate);
        }
        REG_SETF(CRG_TOP, PMU_CTRL_REG, SNC_SLEEP, 0);

        hw_gpio_set_pin_function(tx_port, tx_pin, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_UART2_TX);
        hw_gpio_set_pin_function(rx_port, rx_pin, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_UART2_RX);

        hw_uart_init(BOOTUART, &UART_INIT);

        hw_otpc_init();
        hw_otpc_set_speed(HW_OTPC_SYS_CLK_FREQ_96MHz);

        hw_timer_init(HW_TIMER, &t_cfg);

        ad_flash_init();

        /* Switch to PLL 96 MHz */

        /* Set RCHS as system clock @ 96MHz */
        qspi_automode_sys_clock_cfg(sysclk_RCHS_96);
        oqspi_automode_sys_clock_cfg(sysclk_RCHS_96);
        hw_clk_set_rchs_mode(RCHS_96);
        hw_clk_set_sysclk(SYS_CLK_IS_RCHS);
}

/* transmit announcement message every 1s and wait for <SOH> response */
static void wait_for_soh(void)
{
        uart_soh = false;
        timer1_soh_tmo = true;

        hw_timer_register_int(HW_TIMER, timer1_soh_cb);
        hw_timer_enable(HW_TIMER);
        hw_timer_enable_clk(HW_TIMER);

        while (!uart_soh) {
                if (timer1_soh_tmo) {
                        timer1_soh_tmo = false;
#if (SUPPRESS_HelloMsg == 0)
                        xmit_hello();
#endif
                        hw_uart_receive(BOOTUART, uart_buf, 1, (hw_uart_rx_callback) uart_soh_cb,
                                                                                        uart_buf);
                }

                __WFI();
        };

        hw_timer_disable(HW_TIMER);
}

static void process_header(void)
{
        memset(&cmd_state, 0, sizeof(cmd_state));
        cmd_state.data = &__inputbuffer_start;

        input_buffer_size = &__inputbuffer_end - &__inputbuffer_start;

        cmd_state.type = uart_buf[1];
        cmd_state.len = uart_buf[2] | (uart_buf[3] << 8);

        switch (cmd_state.type) {
        case CMD_WRITE:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.send_to_ram);
                cmd_state.handler = cmd_send_to_ram;
                break;
        case CMD_READ:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_from_ram);
                cmd_state.handler = cmd_read_from_ram;
                break;
        case CMD_COPY_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_ram_to_qspi);
                cmd_state.handler = cmd_write_ram_to_qspi;
                break;
        case CMD_ERASE_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.erase_qspi);
                cmd_state.handler = cmd_erase_qspi;
                break;
        case CMD_RUN:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.execute_code);
                cmd_state.handler = cmd_execute_code;
                break;
        case CMD_WRITE_OTP:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_otp);
                cmd_state.handler = cmd_write_otp;
                break;
        case CMD_READ_OTP:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_otp);
                cmd_state.handler = cmd_read_otp;
                break;
        case CMD_READ_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_qspi);
                cmd_state.handler = cmd_read_qspi;
                break;
        case CMD_GET_VERSION:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.get_version);
                cmd_state.handler = cmd_get_version;
                break;
        case CMD_CHIP_ERASE_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.chip_erase_qspi);
                cmd_state.handler = cmd_chip_erase_qspi;
                break;
        case CMD_IS_EMPTY_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.is_empty_qspi);
                cmd_state.handler = cmd_is_empty_qspi;
                break;
#if dg_configNVMS_ADAPTER
        case CMD_READ_PARTITION:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_partition);
                cmd_state.handler = cmd_read_partition;
                break;
        case CMD_WRITE_PARTITION:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_partition);
                cmd_state.handler = cmd_write_partition;
                break;
        case CMD_READ_PARTITION_TABLE:
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_read_partition_table;
                break;
#endif /* dg_configNVMS_ADAPTER */
        case CMD_GET_QSPI_STATE:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.get_qspi_state);
                cmd_state.handler = cmd_get_qspi_state;
                break;
        case CMD_GPIO_WD:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.gpio_wd);
                cmd_state.handler = cmd_gpio_wd;
                break;
        case CMD_DIRECT_WRITE_TO_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.direct_write_qspi);
                cmd_state.handler = cmd_direct_write_to_qspi;
                break;
        case CMD_COPY_OQSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_ram_to_oqspi);
                cmd_state.handler = cmd_write_ram_to_oqspi;
                break;
        case CMD_ERASE_OQSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.erase_oqspi);
                cmd_state.handler = cmd_erase_oqspi;
                break;
        case CMD_READ_OQSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_oqspi);
                cmd_state.handler = cmd_read_oqspi;
                break;
        case CMD_CHIP_ERASE_OQSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.chip_erase_oqspi);
                cmd_state.handler = cmd_chip_erase_oqspi;
                break;
        case CMD_IS_EMPTY_OQSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.is_empty_oqspi);
                cmd_state.handler = cmd_is_empty_oqspi;
                break;
        case CMD_GET_OQSPI_STATE:
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_get_oqspi_state;
                break;
        case CMD_DIRECT_WRITE_TO_OQSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.direct_write_oqspi);
                cmd_state.handler = cmd_direct_write_to_oqspi;
                break;

        case CMD_GET_PRODUCT_INFO:
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_get_product_info;
                break;

        case CMD_CHANGE_BAUDRATE:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.change_baudrate);
                cmd_state.handler = cmd_change_baudrate;
                break;
        case CMD_DUMMY:
                /* Dummy command - only for GDB server interface */
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_dummy;
                break;
        }

        /* store length of payload (command data excluding command header) */
        cmd_state.data_len = cmd_state.len - cmd_state.hdr_len;
}

/* wait for command header (type + length) */
static bool wait_for_cmd(void)
{
        int soh_len = 1;

        /*
         * uart_soh is set when SOH was already received in response to announcement thus we won't
         * receive another one here. By resetting this flag we make sure that for next command we'll
         * expect SOH to be received here.
         */
        if (uart_soh) {
                soh_len = 0;
        }
        uart_soh = false;

        if (!recv_with_tmo(uart_buf + 1 - soh_len, 3 + soh_len, TMO_COMMAND)) {
                return false;
        }

        process_header();

        return true;
}


static bool load_data(void)
{
        bool ret;

        /* receive command header */
        if (!recv_with_tmo((uint8_t *) &cmd_state.hdr, cmd_state.hdr_len, TMO_DATA)) {
                return false;
        }

        cmd_state.handler(HOP_HEADER);

        /* receive command payload */
        if (!recv_with_tmo(cmd_state.data, cmd_state.data_len,
                                        1 + cmd_state.data_len * (UART_INIT.baud_rate / 10))) {
                return false;
        }

        crc16_init(&cmd_state.crc);
        crc16_update(&cmd_state.crc, (uint8_t *) &cmd_state.hdr, cmd_state.hdr_len);
        crc16_update(&cmd_state.crc, cmd_state.data, cmd_state.data_len);

        ret = cmd_state.handler(HOP_DATA);

        if (!ret) {
                xmit_nak();
                return false;
        }

        xmit_ack();
        xmit_crc16(cmd_state.crc);

        ret = recv_with_tmo(uart_buf, 1, TMO_ACK);
        ret &= (uart_buf[0] == ACK);
        if (ret) {
                ret &= cmd_state.handler(HOP_EXEC);
        }

        ret ? xmit_ack() : xmit_nak();

        return ret;
}

static void swd_handle_header(void)
{
        const HANDLER_OP hop[] = { HOP_INIT, HOP_HEADER, HOP_DATA, HOP_EXEC };
        int i;

        process_header();
        memcpy(&cmd_state.hdr, uart_buf + 4, cmd_state.hdr_len);

        if (!cmd_state.handler) {
                return;
        }

        for (i = 0; i < (sizeof(hop) / sizeof(hop[0])); i++) {
                /* Don't perform next step if previous fails */
                if (!cmd_state.handler(hop[i])) {
                        set_ack_nak_field(NAK);
                        return;
                }
        }

        /* This point won't be reached if error occurs */
        set_ack_nak_field(ACK);
}

/*
 * swd_interface.run_swd is constant value 0
 * Debugger will setup it to 1 when uartboot is to be controlled from debugger
 */
void swd_loop(void) {
        uint32_t last_num = swd_interface.cmd_num;
        uint32_t current_num;
        while (swd_interface.run_swd) {
                current_num = swd_interface.cmd_num;
                if (last_num != current_num) {
                        last_num = current_num;
                        /* Debugger put header in uart_buf, process it */
                        swd_handle_header();
                }

                /* Make sure to enter breakpoint only when debugger is attached */
                if (REG_GETF(CRG_TOP, SYS_STAT_REG, DBG_IS_ACTIVE)) {
                        __BKPT(12);
                }
        }
}

int main()
{

        hw_watchdog_freeze();
        hw_gpio_pad_latch_enable_all();


        /* qspi */
        hw_qspi_set_div(HW_QSPIC, HW_QSPI_DIV_1);
        hw_qspi_clock_enable(HW_QSPIC);




        init();

        uint8_t data[2048];
        ad_flash_read(0x5000, data, 2048);

        swd_loop();

soh_loop:
        wait_for_soh();

cmd_loop:
        /* receive command header (type + length) */
        if (!wait_for_cmd()) {
                goto soh_loop;
        }

        /* NAK for commands we do not support or have faulty header, i.e. length is incorrect */
        if (!cmd_state.handler || !cmd_state.handler(HOP_INIT)) {
                xmit_nak();
                goto cmd_loop;
        }

        xmit_ack();
        /* receive data from CLI */
        if (cmd_state.len) {
                if (!load_data()) {
                        if (uart_tmo) {
                                goto soh_loop;
                        } else {
                                goto cmd_loop;
                        }
                }
        } else {
                if (!cmd_state.handler(HOP_EXEC)) {
                        xmit_nak();
                        goto cmd_loop;
                }
                xmit_ack();
        }

        /* send data length of response, if any */
        if (!cmd_state.handler(HOP_SEND_LEN)) {
                goto cmd_loop;
        }
        if (!recv_with_tmo(uart_buf, 1, 5) || uart_buf[0] != ACK) {
                goto soh_loop;
        }

        /* send response data */
        crc16_init(&cmd_state.crc);
        if (!cmd_state.handler(HOP_SEND_DATA)) {
                goto soh_loop;
        }

        /* receive and check CRC */
        if (!recv_with_tmo(uart_buf, 2, 5)) {
                goto soh_loop;
        }
        if (!memcmp(uart_buf, &cmd_state.crc, 2)) { // we're l-endian and CRC is transmitted lsb-first
                xmit_ack();
        } else {
                xmit_nak();
        }

        goto cmd_loop;

        return 0;
}

