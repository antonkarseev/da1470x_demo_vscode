/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_OQSPI OQSPI Controller
 * \{
 * \brief Octal-SPI Flash Memory Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_oqspi.h
 *
 * @brief Definition of API for the OQSPI Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_OQSPI_H_
#define HW_OQSPI_H_


#if (dg_configUSE_HW_OQSPI)

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief OQSPIC memory access mode
 */
typedef enum {
        HW_OQSPI_ACCESS_MODE_MANUAL = 0,        /**< Manual Mode is selected */
        HW_OQSPI_ACCESS_MODE_AUTO = 1           /**< Auto Mode is selected */
} HW_OQSPI_ACCESS_MODE;

/**
 * \brief OQSPIC memory address size
 */
typedef enum {
        HW_OQSPI_ADDR_SIZE_24 = 0,      /**< 24 bits address */
        HW_OQSPI_ADDR_SIZE_32 = 1       /**< 32 bits address */
} HW_OQSPI_ADDR_SIZE;

/**
 * \brief OQSPIC burst length in automode when the read access in the AHB bus is an incremental
 *        burst of unspecified length
 *
 * \note  This setting is useful in case that the masters that make use of the incremental burst
 *        of unspecified length, require no more than 8 bytes. Set this setting to @ref
 *        HW_OQSPI_BURST_LEN_LIMIT_8_BYTES in order to optimize the cache controller read access
 *        performance.
 */
typedef enum {
        HW_OQSPI_BURST_LEN_LIMIT_UNSPECIFIED = 0,       /**< Unspecified length of the burst */
        HW_OQSPI_BURST_LEN_LIMIT_8_BYTES = 1,           /**< The length of the burst is considered
                                                             as equal to 8 bytes. The access in the
                                                             flash device will be implemented by the
                                                             controller as one or more different
                                                             bursts, until the AHB bus access to be
                                                             completed. Each burst in the flash
                                                             device will have maximum length of 8
                                                             bytes */
} HW_OQSPI_BURST_LEN_LIMIT;

/**
 * \brief OQSPIC bus mode
 */
typedef enum {
        HW_OQSPI_BUS_MODE_SINGLE = 0,   /**< Bus mode in single mode */
        HW_OQSPI_BUS_MODE_DUAL = 1,     /**< Bus mode in dual mode */
        HW_OQSPI_BUS_MODE_QUAD = 2,     /**< Bus mode in quad mode */
        HW_OQSPI_BUS_MODE_OCTA = 3      /**< Bus mode in octa mode */
} HW_OQSPI_BUS_MODE;

/**
 * \brief OQSPI Bus status
 */
typedef enum {
        HW_OQSPI_BUS_STATUS_IDLE = 0,           /**< The SPI Bus is idle */
        HW_OQSPI_BUS_STATUS_ACTIVE = 1,         /**< The SPI Bus is active. Read data, write data
                                                     or dummy data activity is in progress.*/
} HW_OQSPI_BUS_STATUS;

/**
 * \brief OQSPIC device busy status setting
 */
typedef enum {
        HW_OQSPI_BUSY_LEVEL_LOW = 0,   /**< The OQSPI device is busy when the pin level bit is low */
        HW_OQSPI_BUSY_LEVEL_HIGH = 1   /**< The OQSPI device is busy when the pin level bit is high */
} HW_OQSPI_BUSY_LEVEL;

/**
 * \brief OQSPIC clock divider
 */
typedef enum {
        HW_OQSPI_CLK_DIV_1 = 0,      /**< divide by 1 */
        HW_OQSPI_CLK_DIV_2 = 1,      /**< divide by 2 */
        HW_OQSPI_CLK_DIV_4 = 2,      /**< divide by 4 */
        HW_OQSPI_CLK_DIV_8 = 3       /**< divide by 8 */
} HW_OQSPI_CLK_DIV;

/**
 * \brief OQSPIC clock mode
 */
typedef enum {
        HW_OQSPI_CLK_MODE_LOW = 0,       /**< Mode 0: OSPI_SCK is low when OSPI_CS is high. */
        HW_OQSPI_CLK_MODE_HIGH = 1       /**< Mode 3: OSPI_SCK is high when OSPI_CS is high. */
} HW_OQSPI_CLK_MODE;

/**
 * \brief OQSPIC continuous mode
 */
typedef enum {
        HW_OQSPI_CONTINUOUS_MODE_DISABLE = 0,   /**< Disable continuous mode of operation */
        HW_OQSPI_CONTINUOUS_MODE_ENABLE = 1     /**< Enable continuous mode of operation */
} HW_OQSPI_CONTINUOUS_MODE;

/**
 * \brief OQSPIC direction change method in manual mode
 */
typedef enum {
        HW_OQSPI_DIR_CHANGE_MODE_EACH_ACCESS = 0,       /**< The bus direction switches to input
                                                             after each access */
        HW_OQSPI_DIR_CHANGE_MODE_DUMMY_ACCESS = 1,      /**< The bus direction switches to input
                                                             only after a dummy access */
} HW_OQSPI_DIR_CHANGE_MODE;

/**
 * \brief OQSPIC pads drive current strength
 *
 */
typedef enum {
        HW_OQSPI_DRIVE_CURRENT_4 = 0,       /**< 4 mA */
        HW_OQSPI_DRIVE_CURRENT_8 = 1,       /**< 8 mA */
        HW_OQSPI_DRIVE_CURRENT_12 = 2,      /**< 12 mA */
        HW_OQSPI_DRIVE_CURRENT_16 = 3,      /**< 16 mA */
} HW_OQSPI_DRIVE_CURRENT;

/**
 * \brief OQSPIC clock cycle where the bus switches to Hi-Z during the transmission of dummy bytes
 */
typedef enum {
        HW_OQSPI_DUMMY_MODE_LAST_CLK = 0,       /**< Switch to Hi-Z on the last clock cycle */
        HW_OQSPI_DUMMY_MODE_LAST_2_CLK = 1,     /**< Switch to  Hi-Z on the last two clock cycles */
} HW_OQSPI_DUMMY_MODE;

/**
 * \brief OQSPIC extra byte setting in auto access mode
 */
typedef enum {
        HW_OQSPI_EXTRA_BYTE_DISABLE = 0,        /**< Disable extra byte phase */
        HW_OQSPI_EXTRA_BYTE_ENABLE = 1,         /**< Enable extra byte phase */
} HW_OQSPI_EXTRA_BYTE;

/**
 * \brief OQSPIC extra byte half setting in auto access mode
 *
 * \note  This setting is out of scope if the extra byte is disabled or transferred in Octal mode.
 *        Especially in the latter case keep this setting disabled.
 */
typedef enum {
        HW_OQSPI_EXTRA_BYTE_HALF_DISABLE = 0,   /**< Transmit the complete extra byte */
        HW_OQSPI_EXTRA_BYTE_HALF_ENABLE = 1,    /**< The output switches to Hi-Z during the
                                                     transmission of the low nibble of the extra byte */
} HW_OQSPI_EXTRA_BYTE_HALF;

/**
 * \brief OQSPIC behavior in auto mode when the internal buffer is full and there are more data
 *        to be retrieved for the current burst
 */
typedef enum {
        HW_OQSPI_FULL_BUFFER_MODE_BLOCK = 0,            /**< The access in the flash device is not
                                                             terminated when the internal buffer has
                                                             no empty space. In this case the OQSPIC
                                                             clock is blocked until there is free
                                                             space */
        HW_OQSPI_FULL_BUFFER_MODE_TERMINATE = 1,        /**< The access in the flash device is
                                                             terminated when the internal buffer has
                                                             no empty space. A new access in the
                                                             flash device will be initiated when
                                                             the requested addresses are not present
                                                             in the internal buffer */
} HW_OQSPI_FULL_BUFFER_MODE;

/**
 * \brief OQSPIC HREADY signal mode when accessing the WRITEDATA, READDATA and DUMMYDATA registers
 *
 * \note This configuration is useful when the frequency of the OQSPI clock is much lower than
 *       the clock of the AMBA bus, in order to avoid locking the AMBA bus for a long time.
 *       When is set to HW_OQSPI_HREADY_MODE_WAIT there is no need to check the OSPIC_BUSY
 *       for detecting completion of the requested access.
 */
typedef enum {
        HW_OQSPI_HREADY_MODE_WAIT = 0,          /**< Adds wait states via hready signal when
                                                     accessing the OSPIC_WRITEDATA, OSPIC_READDATA
                                                     and OSPIC_DUMMYDATA registers. */
        HW_OQSPI_HREADY_MODE_NO_WAIT = 1        /**< Don't add wait states via the HREADY signal */
} HW_OQSPI_HREADY_MODE;

/**
 * \brief OQSPIC pad direction
 *
 * \note Set this enum to HW_OQSPI_IO_DIR_OUTPUT only when the SPI or Dual SPI mode is enabled in
 *       order to control the /WP signal. When the Quad or Octal SPI mode is enabled this setting
 *       MUST be set to HW_OQSPI_IO_DIR_AUTO_SEL.
 */
typedef enum {
        HW_OQSPI_IO_DIR_AUTO_SEL = 0,   /**< The OQSPI pad is determined by the controller. */
        HW_OQSPI_IO_DIR_OUTPUT = 1      /**< The OQSPI pad is output */
} HW_OQSPI_IO_DIR;

/**
 * \brief OQSPIC IO2/IO3 pad value
 *
 * \note Use this enum to set the value of OSPI_IOx when the corresponding HW_OQSPI_IO_DIR is set
 *       to HW_OQSPI_IO_DIR_OUTPUT.
 */
typedef enum {
        HW_OQSPI_IO_VALUE_LOW = 0,      /**<  Set the level of the OQSPI bus IO low */
        HW_OQSPI_IO_VALUE_HIGH = 1,     /**<  Set the level of the OQSPI bus IO high */
} HW_OQSPI_IO_VALUE;

/**
 * \brief OQSPIC IO4-7 pads values
 *
 * \note Use this enum to set the value of OSPIC_IO_UH_DAT when the corresponding HW_OQSPI_IO_DIR
 *       is set to HW_OQSPI_IO_DIR_OUTPUT.
 */
typedef enum {
        HW_OQSPI_IO4_7_VALUE_0000 = 0,  /**<  Set the level of the OQSPI bus IOs 4-7 to 0000 */
        HW_OQSPI_IO4_7_VALUE_0001,      /**<  Set the level of the OQSPI bus IOs 4-7 to 0001 */
        HW_OQSPI_IO4_7_VALUE_0010,      /**<  Set the level of the OQSPI bus IOs 4-7 to 0010 */
        HW_OQSPI_IO4_7_VALUE_0011,      /**<  Set the level of the OQSPI bus IOs 4-7 to 0011 */
        HW_OQSPI_IO4_7_VALUE_0100,      /**<  Set the level of the OQSPI bus IOs 4-7 to 0100 */
        HW_OQSPI_IO4_7_VALUE_0101,      /**<  Set the level of the OQSPI bus IOs 4-7 to 0101 */
        HW_OQSPI_IO4_7_VALUE_0110,      /**<  Set the level of the OQSPI bus IOs 4-7 to 0110 */
        HW_OQSPI_IO4_7_VALUE_0111,      /**<  Set the level of the OQSPI bus IOs 4-7 to 0111 */
        HW_OQSPI_IO4_7_VALUE_1000,      /**<  Set the level of the OQSPI bus IOs 4-7 to 1000 */
        HW_OQSPI_IO4_7_VALUE_1001,      /**<  Set the level of the OQSPI bus IOs 4-7 to 1001 */
        HW_OQSPI_IO4_7_VALUE_1010,      /**<  Set the level of the OQSPI bus IOs 4-7 to 1010 */
        HW_OQSPI_IO4_7_VALUE_1011,      /**<  Set the level of the OQSPI bus IOs 4-7 to 1011 */
        HW_OQSPI_IO4_7_VALUE_1100,      /**<  Set the level of the OQSPI bus IOs 4-7 to 1100 */
        HW_OQSPI_IO4_7_VALUE_1101,      /**<  Set the level of the OQSPI bus IOs 4-7 to 1101 */
        HW_OQSPI_IO4_7_VALUE_1110,      /**<  Set the level of the OQSPI bus IOs 4-7 to 1110 */
        HW_OQSPI_IO4_7_VALUE_1111,      /**<  Set the level of the OQSPI bus IOs 4-7 to 1111 */
} HW_OQSPI_IO4_7_VALUE;

/**
 * \brief OQSPIC AHB bus error response when a read is performed in the address space where the
 *        flash device is mapped and the Auto mode is not enabled
 */
typedef enum {
        HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE_IGNORE = 0,     /**< The read access is ignored and
                                                                  there is no error due to the read
                                                                  access */
        HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE_AHB_ERROR = 1,  /**< Respond with an AHB bus error */
} HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE;

/**
 * \brief The opcode length of the command phase
 */
typedef enum {
        HW_OQSPI_OPCODE_LEN_1_BYTE = 0,         /**< The opcode length is 1 byte */
        HW_OQSPI_OPCODE_LEN_2_BYTES = 1,        /**< The opcode length is 2 bytes. */
} HW_OQSPI_OPCODE_LEN;

/**
 * \brief OQSPIC read pipe setting
 *
 * \note When read pipe is disabled the sampling clock is determined by @ref HW_OQSPI_SAMPLING_EDGE
 *       otherwise by @ref HW_OQSPI_READ_PIPE_DELAY.
 */
typedef enum {
        HW_OQSPI_READ_PIPE_DISABLE = 0,         /**< Disable read pipe delay */
        HW_OQSPI_READ_PIPE_ENABLE = 1,          /**< Enable read pipe delay */
} HW_OQSPI_READ_PIPE;

/**
 * \brief OQSPIC Read pipe clock delay in relation to the falling edge of OSPI_SCK
 *
 * \note The read pipe delay should be set based on the voltage level of the power rail V12.
 *       Recommended values: V12 = 0.9V: HW_OQSPI_READ_PIPE_DELAY_0
 *                           V12 = 1.2V: HW_OQSPI_READ_PIPE_DELAY_7
 */
typedef enum {
        HW_OQSPI_READ_PIPE_DELAY_0 = 0,         /**< Set read pipe delay to 0 */
        HW_OQSPI_READ_PIPE_DELAY_1 = 1,         /**< Set read pipe delay to 1 */
        HW_OQSPI_READ_PIPE_DELAY_2 = 2,         /**< Set read pipe delay to 2 */
        HW_OQSPI_READ_PIPE_DELAY_3 = 3,         /**< Set read pipe delay to 3 */
        HW_OQSPI_READ_PIPE_DELAY_4 = 4,         /**< Set read pipe delay to 4 */
        HW_OQSPI_READ_PIPE_DELAY_5 = 5,         /**< Set read pipe delay to 5 */
        HW_OQSPI_READ_PIPE_DELAY_6 = 6,         /**< Set read pipe delay to 6 */
        HW_OQSPI_READ_PIPE_DELAY_7 = 7,         /**< Set read pipe delay to 7 */
} HW_OQSPI_READ_PIPE_DELAY;

/**
 * \brief Defines the value that is transferred on the OQSPI bus during the the dummy bytes phase.
 */
typedef enum {
        HW_OQSPI_READ_STATUS_DUMMY_VAL_UNCHANGED = 0,  /**< Keeps the data in the bus unchanged, until
                                                            the bus direction changes to input mode */
        HW_OQSPI_READ_STATUS_DUMMY_VAL_FORCED_ZERO = 1,/**< Forces the OQSPIC bus IOs to low as long as
                                                            the bus direction is not in input mode. */
} HW_OQSPI_READ_STATUS_DUMMY_VAL;

/**
 * \brief OQSPIC clock edge setting for the sampling of the incoming data when the read pipe is
 *        disabled
 */
typedef enum {
        HW_OQSPI_SAMPLING_EDGE_POS = 0,    /**< The incoming data sampling is triggered by the
                                                positive edge of OQSPIC clock signal */
        HW_OQSPI_SAMPLING_EDGE_NEG = 1     /**< The incoming data sampling is triggered by the
                                                negative edge of OQSPIC clock signal */
} HW_OQSPI_SAMPLING_EDGE;

/**
 * \brief OQSPIC pads slew rate
 *
 */
typedef enum {
        HW_OQSPI_SLEW_RATE_0 = 0,       /**< Rise = 1.7 V/ns, Fall = 1.9 V/ns (weak) */
        HW_OQSPI_SLEW_RATE_1 = 1,       /**< Rise = 2.0 V/ns, Fall = 2.3 V/ns */
        HW_OQSPI_SLEW_RATE_2 = 2,       /**< Rise = 2.3 V/ns, Fall = 2.6 V/ns */
        HW_OQSPI_SLEW_RATE_3 = 3        /**< Rise = 2.4 V/ns, Fall = 2.7 V/ns (strong) */
} HW_OQSPI_SLEW_RATE;

/**
 * \brief The status of sector/block erasing
 */
typedef enum {
        HW_OQSPI_ERASE_STATUS_NO = 0,             /**< no erase                           */
        HW_OQSPI_ERASE_STATUS_PENDING = 1,        /**< pending erase request              */
        HW_OQSPI_ERASE_STATUS_RUNNING = 2,        /**< erase procedure is running         */
        HW_OQSPI_ERASE_STATUS_SUSPENDED = 3,      /**< suspended erase procedure          */
        HW_OQSPI_ERASE_STATUS_FINISHING = 4       /**< finishing the erase procedure      */
} HW_OQSPI_ERASE_STATUS;
/*
 * MACROS DEFINITIONS
 *****************************************************************************************
 */
#define IS_HW_OQSPI_ACCESS_MODE(x)              (((x) == HW_OQSPI_ACCESS_MODE_MANUAL) || \
                                                 ((x) == HW_OQSPI_ACCESS_MODE_AUTO))

#define IS_HW_OQSPI_ADDR_SIZE(x)                (((x) == HW_OQSPI_ADDR_SIZE_24)   || \
                                                 ((x) == HW_OQSPI_ADDR_SIZE_32))

#define IS_HW_OQSPI_BUSY_LEVEL(x)               (((x) == HW_OQSPI_BUSY_LEVEL_LOW)   || \
                                                 ((x) == HW_OQSPI_BUSY_LEVEL_HIGH))

#define IS_HW_OQSPI_BUS_MODE(x)                 (((x) >= HW_OQSPI_BUS_MODE_SINGLE) && \
                                                 ((x) <= HW_OQSPI_BUS_MODE_OCTA))

#define IS_HW_OQSPI_BURST_LEN_LIMIT(x)          (((x) >= HW_OQSPI_BURST_LEN_LIMIT_UNSPECIFIED) || \
                                                 ((x) <= HW_OQSPI_BURST_LEN_LIMIT_8_BYTES))

#define IS_HW_OQSPI_CLK_DIV(x)                  (((x) >= HW_OQSPI_CLK_DIV_1) && \
                                                 ((x) <= HW_OQSPI_CLK_DIV_8))

#define IS_HW_OQSPI_CLK_MODE(x)                 (((x) == HW_OQSPI_CLK_MODE_LOW) || \
                                                 ((x) == HW_OQSPI_CLK_MODE_HIGH))

#define IS_HW_OQSPI_CONTINUOUS_MODE(x)          (((x) == HW_OQSPI_CONTINUOUS_MODE_DISABLE) || \
                                                 ((x) == HW_OQSPI_CONTINUOUS_MODE_ENABLE))

#define IS_HW_OQSPI_DIR_CHANGE_MODE(x)          (((x) == HW_OQSPI_DIR_CHANGE_MODE_EACH_ACCESS) || \
                                                 ((x) == HW_OQSPI_DIR_CHANGE_MODE_DUMMY_ACCESS))

#define IS_HW_OQSPI_DRIVE_CURRENT(x)            (((x) >= HW_OQSPI_DRIVE_CURRENT_4)  && \
                                                 ((x) <= HW_OQSPI_DRIVE_CURRENT_16))

#define IS_HW_OQSPI_DUMMY_MODE(x)               (((x) == HW_OQSPI_DUMMY_MODE_LAST_CLK) || \
                                                 ((x) == HW_OQSPI_DUMMY_MODE_LAST_2_CLK))

#define IS_HW_OQSPI_EXTRA_BYTE(x)               (((x) == HW_OQSPI_EXTRA_BYTE_DISABLE) || \
                                                 ((x) == HW_OQSPI_EXTRA_BYTE_ENABLE))

#define IS_HW_OQSPI_EXTRA_BYTE_HALF(x)          (((x) == HW_OQSPI_EXTRA_BYTE_HALF_DISABLE) || \
                                                 ((x) == HW_OQSPI_EXTRA_BYTE_HALF_ENABLE))

#define IS_HW_OQSPI_FULL_BUFFER_MODE(x)         (((x) == HW_OQSPI_FULL_BUFFER_MODE_BLOCK) || \
                                                 ((x) == HW_OQSPI_FULL_BUFFER_MODE_TERMINATE))

#define IS_HW_OQSPI_HREADY_MODE(x)              (((x) == HW_OQSPI_HREADY_MODE_WAIT) || \
                                                 ((x) == HW_OQSPI_HREADY_MODE_NO_WAIT))

#define IS_HW_OQSPI_IO_DIR(x)                   (((x) == HW_OQSPI_IO_DIR_AUTO_SEL) || \
                                                 ((x) == HW_OQSPI_IO_DIR_OUTPUT))

#define IS_HW_OQSPI_IO_VALUE(x)                 (((x) == HW_OQSPI_IO_VALUE_LOW) || \
                                                 ((x) == HW_OQSPI_IO_VALUE_HIGH))

#define IS_HW_OQSPI_IO4_7_VALUE(x)              (((x) >= HW_OQSPI_IO4_7_VALUE_0000)  && \
                                                 ((x) <= HW_OQSPI_IO4_7_VALUE_1111))

#define IS_HW_OQSPI_OPCODE_LEN(x)               (((x) == HW_OQSPI_OPCODE_LEN_1_BYTE)   || \
                                                 ((x) == HW_OQSPI_OPCODE_LEN_2_BYTES))

#define IS_HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE(x) (((x) == HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE_IGNORE) || \
                                                    ((x) == HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE_AHB_ERROR))

#define IS_HW_OQSPI_OPCODE_LEN(x)               (((x) == HW_OQSPI_OPCODE_LEN_1_BYTE)   || \
                                                 ((x) == HW_OQSPI_OPCODE_LEN_2_BYTES))

#define IS_HW_OQSPI_READ_PIPE(x)                (((x) == HW_OQSPI_READ_PIPE_DISABLE)   || \
                                                 ((x) == HW_OQSPI_READ_PIPE_ENABLE))

#define IS_HW_OQSPI_READ_PIPE_DELAY(x)          (((x) >= HW_OQSPI_READ_PIPE_DELAY_0)  && \
                                                 ((x) <= HW_OQSPI_READ_PIPE_DELAY_7))

#define IS_HW_OQSPI_READ_STATUS_DUMMY_VAL(x)    (((x) == HW_OQSPI_READ_STATUS_DUMMY_VAL_UNCHANGED) || \
                                                 ((x) == HW_OQSPI_READ_STATUS_DUMMY_VAL_FORCED_ZERO))

#define IS_HW_OQSPI_SAMPLING_EDGE(x)            (((x) == HW_OQSPI_SAMPLING_EDGE_POS) || \
                                                 ((x) == HW_OQSPI_SAMPLING_EDGE_NEG))

#define IS_HW_OQSPI_SLEW_RATE(x)                (((x) >= HW_OQSPI_SLEW_RATE_0)  && \
                                                 ((x) <= HW_OQSPI_SLEW_RATE_3))

#define SUSPEND_RESUME_COUNTER_FREQ_HZ          (222000)
/*
 * STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief This union is used in order to allow different size access when reading/writing to
 *        OQSPIF_READDATA_REG, OQSPIF_WRITEDATA_REG, OQSPIF_DUMMYDATA_REG because
 */
typedef union {
        __IO uint32_t  data32;
        __IO uint16_t  data16;
        __IO uint8_t   data8;
} hw_oqspi_data_t;

/**
 * \brief This struct is used in order to allow different size access when reading/writing to
 *        OQSPIF_READDATA_REG, OQSPIF_WRITEDATA_REG, OQSPIF_DUMMYDATA_REG because
 */
typedef struct {                                      /*!< (@ 0x36000000) OQSPIF Structure */
  __IOM uint32_t        OQSPIF_CTRLBUS_REG;           /*!< (@ 0x00000000) SPI Bus control register for the Manual mode */
  __IOM uint32_t        OQSPIF_CTRLMODE_REG;          /*!< (@ 0x00000004) Mode Control register */
  __IOM uint32_t        OQSPIF_RECVDATA_REG;          /*!< (@ 0x00000008) Received data for the Manual mode */
  __IOM uint32_t        OQSPIF_BURSTCMDA_REG;         /*!< (@ 0x0000000C) The way of reading in Auto mode (command register A) */
  __IOM uint32_t        OQSPIF_BURSTCMDB_REG;         /*!< (@ 0x00000010) The way of reading in Auto mode (command register B) */
  __IOM uint32_t        OQSPIF_STATUS_REG;            /*!< (@ 0x00000014) The status register of the OSPI controller */
  __IOM hw_oqspi_data_t OQSPIF_WRITEDATA_REG;         /*!< (@ 0x00000018) Write data to SPI Bus for the Manual mode */
  __IOM hw_oqspi_data_t OQSPIF_READDATA_REG;          /*!< (@ 0x0000001C) Read data from SPI Bus for the Manual mode */
  __IOM hw_oqspi_data_t OQSPIF_DUMMYDATA_REG;         /*!< (@ 0x00000020) Send dummy clocks to SPI Bus for the Manual mode */
  __IOM uint32_t        OQSPIF_ERASECTRL_REG;         /*!< (@ 0x00000024) OSPI Erase control register */
  __IOM uint32_t        OQSPIF_ERASECMDA_REG;         /*!< (@ 0x00000028) The way of erasing in Auto mode (command register A) */
  __IOM uint32_t        OQSPIF_ERASECMDB_REG;         /*!< (@ 0x0000002C) The way of erasing in Auto mode (command register B) */
  __IOM uint32_t        OQSPIF_ERASECMDC_REG;         /*!< (@ 0x00000030) The way of erasing in Auto mode (command register C) */
  __IOM uint32_t        OQSPIF_BURSTBRK_REG;          /*!< (@ 0x00000034) Read break sequence in Auto mode */
  __IOM uint32_t        OQSPIF_STATUSCMD_REG;         /*!< (@ 0x00000038) The way of reading the status of external device in Auto mode */
  __IOM uint32_t        OQSPIF_CHCKERASE_REG;         /*!< (@ 0x0000003C) Check erase progress in Auto mode */
  __IOM uint32_t        OQSPIF_GP_REG;                /*!< (@ 0x00000040) OSPI General Purpose control register */
  __IM  uint32_t        RESERVED[47];
  __IOM uint32_t        OQSPIF_CTR_CTRL_REG;          /*!< (@ 0x00000100) Control register for the decryption engine of the OSPIC */
  __IOM uint32_t        OQSPIF_CTR_SADDR_REG;         /*!< (@ 0x00000104) Start address of the encrypted content in the OSPI flash */
  __IOM uint32_t        OQSPIF_CTR_EADDR_REG;         /*!< (@ 0x00000108) End address of the encrypted content in the OSPI flash */
  __IOM uint32_t        OQSPIF_CTR_NONCE_0_3_REG;     /*!< (@ 0x0000010C) Nonce bytes 0 to 3 for the AES-CTR algorithm */
  __IOM uint32_t        OQSPIF_CTR_NONCE_4_7_REG;     /*!< (@ 0x00000110) Nonce bytes 4 to 7 for the AES-CTR algorithm */
  __IOM uint32_t        OQSPIF_CTR_KEY_0_3_REG;       /*!< (@ 0x00000114) Key bytes 0 to 3 for the AES-CTR algorithm */
  __IOM uint32_t        OQSPIF_CTR_KEY_4_7_REG;       /*!< (@ 0x00000118) Key bytes 4 to 7 for the AES-CTR algorithm */
  __IOM uint32_t        OQSPIF_CTR_KEY_8_11_REG;      /*!< (@ 0x0000011C) Key bytes 8 to 11 for the AES-CTR algorithm */
  __IOM uint32_t        OQSPIF_CTR_KEY_12_15_REG;     /*!< (@ 0x00000120) Key bytes 12 to 15 for the AES-CTR algorithm */
  __IOM uint32_t        OQSPIF_CTR_KEY_16_19_REG;     /*!< (@ 0x00000124) Key bytes 16 to 19 for the AES-CTR algorithm */
  __IOM uint32_t        OQSPIF_CTR_KEY_20_23_REG;     /*!< (@ 0x00000128) Key bytes 20 to 23 for the AES-CTR algorithm */
  __IOM uint32_t        OQSPIF_CTR_KEY_24_27_REG;     /*!< (@ 0x0000012C) Key bytes 24 to 27 for the AES-CTR algorithm */
  __IOM uint32_t        OQSPIF_CTR_KEY_28_31_REG;     /*!< (@ 0x00000130) Key bytes 28 to 31 for the AES-CTR algorithm */
} hw_oqspi_regs_t;

/**
 * \brief OQSPIC manual access mode configuration structure
 */
typedef struct {
        HW_OQSPI_DIR_CHANGE_MODE                dir_change_mode : 1;            /**< Bus direction change method */
        HW_OQSPI_DUMMY_MODE                     dummy_mode : 1;                 /**< Dummy phase mode */
        HW_OQSPI_HREADY_MODE                    hready_mode : 1;                /**< HREADY signal mode */
        HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE    mapped_addr_rd_acc_response : 1;/**< AHB bus error response */
} hw_oqspi_manualmode_config_t;

/**
 * \brief OQSPIC auto access mode configuration structure
 */
typedef struct {
        HW_OQSPI_BURST_LEN_LIMIT        burst_len_limit : 1;    /**< Burst length limit */
        HW_OQSPI_FULL_BUFFER_MODE       full_buffer_mode : 1;   /**< Full buffer mode */
} hw_oqspi_automode_config_t;

/**
 * \brief OQSPIC configuration structure
 */
typedef struct {
        HW_OQSPI_ADDR_SIZE              address_size : 1;       /**< Memory address size */
        HW_OQSPI_CLK_DIV                clk_div : 2;            /**< Clock divider */
        HW_OQSPI_CLK_MODE               clock_mode : 1;         /**< Clock mode */
        HW_OQSPI_DRIVE_CURRENT          drive_current : 2;      /**< Drive current */
        HW_OQSPI_OPCODE_LEN             opcode_len : 1;         /**< Opcode length */
        HW_OQSPI_READ_PIPE              read_pipe : 1;          /**< Read pipe enable */
        HW_OQSPI_READ_PIPE_DELAY        read_pipe_delay : 3;    /**< Read pipe delay */
        HW_OQSPI_SAMPLING_EDGE          sampling_edge :1;       /**< Incoming data sampling edge */
        HW_OQSPI_SLEW_RATE              slew_rate : 2;          /**< IOs slew rate */
        hw_oqspi_automode_config_t      auto_mode_cfg;          /**< Auto access mode configuration struct */
        hw_oqspi_manualmode_config_t    manual_mode_cfg;        /**< Manual access mode configuration struct */
} hw_oqspi_config_t;

/**
 * \brief Read instruction configuration structure (auto access mode)
 */
typedef struct {
        HW_OQSPI_BUS_MODE               opcode_bus_mode : 2;    /**< Bus mode of the opcode phase */
        HW_OQSPI_BUS_MODE               addr_bus_mode : 2;      /**< Bus mode of the address phase */
        HW_OQSPI_BUS_MODE               extra_byte_bus_mode : 2;/**< Bus mode of the extra byte phase */
        HW_OQSPI_BUS_MODE               dummy_bus_mode : 2;     /**< Bus mode of the dummy phase */
        HW_OQSPI_BUS_MODE               data_bus_mode : 2;      /**< Bus mode of the data phase */
        HW_OQSPI_CONTINUOUS_MODE        continuous_mode : 1;    /**< Set continuous mode of operation */
        HW_OQSPI_EXTRA_BYTE             extra_byte_cfg : 1;     /**< Enable Extra Byte */
        HW_OQSPI_EXTRA_BYTE_HALF        extra_byte_half_cfg : 1;/**< Enable Extra Byte Half */
        uint8_t                         opcode;                 /**< Read command opcode */
        uint8_t                         extra_byte_value;       /**< Extra Byte value */
        uint16_t                        cs_idle_delay_nsec;     /**< The minimum CS idle delay in nsec
                                                                     between two consecutive Read commands */
} hw_oqspi_read_instr_config_t;

/**
 * \brief OQSPIC Erase instruction configuration structure (auto access mode)
 */
typedef struct {
        HW_OQSPI_BUS_MODE       opcode_bus_mode : 2;    /**< Bus mode of the opcode phase */
        HW_OQSPI_BUS_MODE       addr_bus_mode : 2;      /**< Bus mode of the address phase */
        uint32_t                hclk_cycles : 4;        /**< The number of AMBA AHB hclk cycles
                                                             (0..15) without memory read requests before
                                                             executing an erase or erase resume command.
                                                             Use this setting to delay one of the
                                                             aforementioned commands otherwise keep it 0. */
        uint8_t                 opcode;                 /**< Erase command opcode */
        uint16_t                cs_idle_delay_nsec;     /**< The minimum CS idle delay in nsec
                                                             between a Write Enable, Erase, Erase
                                                             Suspend or Erase Resume command and
                                                             the next consecutive command */
} hw_oqspi_erase_instr_config_t;

/**
 * \brief OQSPIC read status instruction configuration structure (auto access mode)
 */
typedef struct {
        HW_OQSPI_BUS_MODE               opcode_bus_mode : 2;    /**< The bus mode of the opcode phase */
        HW_OQSPI_BUS_MODE               receive_bus_mode : 2;   /**< The bus mode of the receive data phase */
        HW_OQSPI_BUS_MODE               dummy_bus_mode : 2;     /**< The bus mode of the dummy bytes phase */
        HW_OQSPI_READ_STATUS_DUMMY_VAL  dummy_value : 1;        /**< The value that is transferred on
                                                                     the OSPI bus during the dummy cycles phase */
        HW_OQSPI_BUSY_LEVEL             busy_level : 1;         /**< Busy bit level */
        uint32_t                        busy_pos : 3;           /**< The position of the Busy bit in
                                                                     the status register (0 - 7) */
        uint8_t                         dummy_bytes;            /**< The number of dummy bytes  (0 - 16) */
        uint8_t                         opcode;                 /**< Read Status command opcode  */
        uint16_t                        delay_nsec;             /**< The minimum delay in nsec between a
                                                                     Read Status command and the previous
                                                                     Erase command. Usually NOT needed
                                                                     thus is set equal to 0. */
} hw_oqspi_read_status_instr_config_t;

/**
 * \brief OQSPIC write enable instruction configuration structure (auto access mode)
 */
typedef struct {
        HW_OQSPI_BUS_MODE       opcode_bus_mode: 2;     /**< Bus mode of the opcode phase */
        uint8_t                 opcode;                 /**< Write Enable command opcode  */
} hw_oqspi_write_enable_instr_config_t;

/**
 * \brief OQSPIC Page Program instruction configuration structure (manual access mode)
 */
typedef struct {
        HW_OQSPI_BUS_MODE       opcode_bus_mode : 2;    /**< The bus mode of the opcode phase */
        HW_OQSPI_BUS_MODE       addr_bus_mode : 2;      /**< The bus mode of the address phase */
        HW_OQSPI_BUS_MODE       data_bus_mode : 2;      /**< The bus mode of the address phase */
        uint8_t                 opcode;                 /**< Page Program command opcode  */
} hw_oqspi_page_program_instr_config_t;

/**
 * \brief OQSPIC Erase suspend/resume instruction structure (auto access mode)
 */
typedef struct {
        HW_OQSPI_BUS_MODE               suspend_bus_mode : 2;   /**< Bus mode during the erase suspend
                                                                     command phase */
        HW_OQSPI_BUS_MODE               resume_bus_mode : 2;    /**< Bus mode during the erase resume
                                                                     command phase */
        uint8_t                         suspend_opcode;         /**< Erase suspend instruction code */
        uint8_t                         resume_opcode;          /**< Erase resume instruction code */
        uint8_t                         suspend_latency_usec;   /**< The minimum required latency (usec)
                                                                     to suspend an erase operation.
                                                                     The next consecutive read command
                                                                     cannot be issued before this time
                                                                     has elapsed. */
        uint8_t                         resume_latency_usec;    /**< The minimum required latency (usec)
                                                                     to resume an erase operation.
                                                                     Once the resume command is issued,
                                                                     the currently suspended erase
                                                                     operation resumes within this time. */
        uint16_t                        res_sus_latency_usec;   /**< The minimum required latency (usec)
                                                                     between an erase resume and the
                                                                     next consequent erase suspend
                                                                     command */
} hw_oqspi_suspend_resume_instr_config_t;

/**
 * \brief OQSPIC Exit Continuous Mode instruction configuration structure
 */
typedef struct {
        HW_OQSPI_BUS_MODE       opcode_bus_mode : 2;    /**< Bus mode during the opcode phases */
        uint32_t                sequence_len : 4;       /**< The sequence length in bytes */
        uint32_t                disable_second_half : 1;/**< Disable the output during the
                                                             second half [3:0] of the sequence .
                                                             Not applicable in Octa Bus mode. */
        uint8_t                 opcode;                 /**< Exit Continuous Mode instruction code */
} hw_oqspi_exit_continuous_mode_instr_config_t;

/**
 * \brief OQSPIC AES-CTR decryption configuration structure
 */
typedef struct {
        uint8_t         nonce[8];               /*!< AES-CTR decryption nonce value */
        uint8_t         key[32];                /*!< AES-CTR decryption key value */
        uint32_t        start_addr;             /*!< AES-CTR decryption start address */
        uint32_t        end_addr;               /*!< AES-CTR decryption end address */
} hw_oqspi_aes_ctr_config_t;


/**
 * \brief Enable OQSPI controller clock
 */
__STATIC_FORCEINLINE void hw_oqspi_clock_enable(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, OQSPIF_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable OQSPI controller clock
 */
__STATIC_FORCEINLINE void hw_oqspi_clock_disable(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_AMBA_REG, OQSPIF_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Enable CS on OQSPI bus in manual access mode
 */
__STATIC_FORCEINLINE void hw_oqspi_cs_enable(void)
{
        REG_SET_BIT(OQSPIF, OQSPIF_CTRLBUS_REG, OSPIC_EN_CS);
}

/**
 * \brief Disable CS on OQSPI bus in manual access mode.
 */
__STATIC_FORCEINLINE void hw_oqspi_cs_disable(void)
{
        REG_SET_BIT(OQSPIF, OQSPIF_CTRLBUS_REG, OSPIC_DIS_CS);
}

/**
 * \brief       Get OQSPIC Bus status
 *
 * \sa          HW_OQSPI_BUS_STATUS
 */
__STATIC_FORCEINLINE HW_OQSPI_BUS_STATUS hw_oqspi_get_bus_status(void)
{
        return (HW_OQSPI_BUS_STATUS) REG_GETF(OQSPIF, OQSPIF_STATUS_REG, OSPIC_BUSY);
}

/**
 * \brief       Set OQSPIC clock divider
 *
 * \param [in]  div OQSPIC clock divider
 *
 * \sa          HW_OQSPI_CLK_DIV
 */
__STATIC_FORCEINLINE void hw_oqspi_set_div(HW_OQSPI_CLK_DIV div)
{
        ASSERT_WARNING(IS_HW_OQSPI_CLK_DIV(div));

        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_AMBA_REG, OQSPIF_DIV, div);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief       Get OQSPIC clock divider
 *
 * \return      OQSPIC clock divider setting
 *
 * \sa          HW_OQSPI_CLK_DIV
 */
__STATIC_FORCEINLINE HW_OQSPI_CLK_DIV hw_oqspi_get_div(void)
{
        return (HW_OQSPI_CLK_DIV) REG_GETF(CRG_TOP, CLK_AMBA_REG, OQSPIF_DIV);
}

/**
 * \brief       Enable using the upper 4 pins of the OQSPI controller as GPIO.
 *
 * \warning     By enabling this feature, the OQSPIC does not control the aforementioned pins
 *              anymore and therefore the OCTA bus mode cannot be used.
 */
__STATIC_FORCEINLINE void hw_oqspi_use_io4_7_as_gpio(void)
{
        REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, OQSPI_GPIO_MODE);
}

/**
 * \brief       Enable the OQSPIC to make use of the upper 4 pins for transmitting data from/to the
 *              connected memory.
 */
__STATIC_FORCEINLINE void hw_oqspi_use_io4_7_for_octa_bus(void)
{
        REG_CLR_BIT(CRG_TOP, CLK_AMBA_REG, OQSPI_GPIO_MODE);
}

/**
 * \brief       Check whether the upper 4 pins of the OQSPIC are used as GPIO.
 *
 * \return      True, if the upper 4 pins of the OQSPIC are used as GPIO.
 */
__STATIC_FORCEINLINE bool hw_oqspi_are_io4_7_gpio(void)
{
        return (REG_GETF(CRG_TOP, CLK_AMBA_REG, OQSPI_GPIO_MODE));
}

/**
 * \brief       Set OQSPIC bus mode in manual access mode
 *
 * \param [in]  bus_mode OQSPIC bus mode in manual access mode
 *
 * \sa          HW_OQSPI_BUS_MODE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_manual_access_bus_mode(HW_OQSPI_BUS_MODE bus_mode)
{
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(bus_mode));

        OQSPIF->OQSPIF_CTRLBUS_REG  = 1 << bus_mode;
}

/**
 * \brief       Set OQSPIC access mode
 *
 * \param [in]  access_mode OQSPIC access mode

 * \sa          HW_OQSPI_ACCESS_MODE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_access_mode(HW_OQSPI_ACCESS_MODE access_mode)
{
        ASSERT_WARNING(IS_HW_OQSPI_ACCESS_MODE(access_mode));
        // During erasing where OSPIC_ERASE_EN = 1, OSPIC_AUTO_MD switches in read only mode
        ASSERT_WARNING(REG_GETF(OQSPIF, OQSPIF_ERASECTRL_REG, OSPIC_ERASE_EN) == 0);

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_AUTO_MD, access_mode);
}

/**
 * \brief       Get OQSPIC access mode
 *
 * \return      OQSPIC access mode
 *
 * \sa          HW_OQSPI_ACCESS_MODE
 */
__STATIC_FORCEINLINE HW_OQSPI_ACCESS_MODE hw_oqspi_get_access_mode(void)
{
        return (HW_OQSPI_ACCESS_MODE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_AUTO_MD);
}

/**
 * \brief       Set OQSPIC clock mode
 *
 * \param [in]  clk_mode OQSPIC clock mode
 *
 * \sa          HW_OQSPI_CLK_MODE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_clock_mode(HW_OQSPI_CLK_MODE clk_mode)
{
        ASSERT_WARNING(IS_HW_OQSPI_CLK_MODE(clk_mode));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_CLK_MD, clk_mode);
}

/**
 * \brief       Get OQSPIC clock mode
 *
 * \return      OQSPIC clock mode
 *
 * \sa          HW_OQSPI_CLK_MODE
 */
__STATIC_FORCEINLINE HW_OQSPI_CLK_MODE hw_oqspi_get_clock_mode(void)
{
        return (HW_OQSPI_CLK_MODE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_CLK_MD);
}

/**
 * \brief       Set OSPI_IO2 direction
 *
 * \param [in]  dir OSPI_IO2 direction
 *
 * \note        Set OSPI_IO2 direction to HW_OQSPI_IO_DIR_OUTPUT only in Single or Dual SPI mode
 *              to control the /WP signal. When the Quad or Octal SPI is enabled, dir MUST be set
 *              to HW_OQSPI_IO_DIR_AUTO_SEL.
 *
 * \sa          HW_OQSPI_IO_DIR
 */
__STATIC_FORCEINLINE void hw_oqspi_set_io2_direction(HW_OQSPI_IO_DIR dir)
{
        ASSERT_WARNING(IS_HW_OQSPI_IO_DIR(dir));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO2_OEN, dir);
}

/**
 * \brief       Get OSPI_IO2 direction
 *
 * \return      OSPI_IO2 direction
 *
 * \sa          HW_OQSPI_IO_DIR
 */
__STATIC_FORCEINLINE HW_OQSPI_IO_DIR hw_oqspi_get_io2_direction(void)
{
        return (HW_OQSPI_IO_DIR) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO2_OEN);
}

/**
 * \brief       Set OSPI_IO3 direction
 *
 * \param [in]  dir OSPI_IO3 direction
 *
 * \note        Set OSPI_IO3 direction to HW_OQSPI_IO_DIR_OUTPUT only in Single or Dual SPI mode
 *              to control the /WP signal. When the Quad or Octal SPI is enabled, dir MUST be set
 *              to HW_OQSPI_IO_DIR_AUTO_SEL.
 *
 * \sa          HW_OQSPI_IO_DIR
 */
__STATIC_FORCEINLINE void hw_oqspi_set_io3_direction(HW_OQSPI_IO_DIR dir)
{
        ASSERT_WARNING(IS_HW_OQSPI_IO_DIR(dir));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO3_OEN, dir);
}

/**
 * \brief       Get OSPI_IO3 direction
 *
 * \return      OSPI_IO3 direction
 *
 * \sa          HW_OQSPI_IO_DIR
 */
__STATIC_FORCEINLINE HW_OQSPI_IO_DIR hw_oqspi_get_io3_direction(void)
{
        return (HW_OQSPI_IO_DIR) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO3_OEN);
}

/**
 * \brief       Set OSPI_IO4 - OSPI_IO7 direction
 *
 * \param [in]  dir OSPI_IO4 - OSPI_IO7 direction
 *
 * \note        Set OSPI_IO4 - OSPI_IO7 direction to HW_OQSPI_IO_DIR_OUTPUT only in Single or Dual
 *              SPI mode to control the /WP signal. When the Quad or Octal SPI is enabled, dir MUST
 *              be set to HW_OQSPI_IO_DIR_AUTO_SEL.
 *
 * \sa          HW_OQSPI_IO_DIR
 */
__STATIC_FORCEINLINE void hw_oqspi_set_io4_7_direction(HW_OQSPI_IO_DIR dir)
{
        ASSERT_WARNING(IS_HW_OQSPI_IO_DIR(dir));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO_UH_OEN, dir);
}

/**
 * \brief       Get OSPI_IO4 - OSPI_IO7 direction
 *
 * \return      OSPI_IO4 - OSPI_IO7 direction
 *
 * \sa          HW_OQSPI_IO_DIR
 */
__STATIC_FORCEINLINE HW_OQSPI_IO_DIR hw_oqspi_get_io4_7_direction(void)
{
        return (HW_OQSPI_IO_DIR) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO_UH_OEN);
}

/**
 * \brief       Set the value of OSPI_IO2 pad when OSPI_IO2 direction is output
 *
 * \param [in]  value The value of OSPI_IO2 pad
 *
 * \sa          HW_OQSPI_IO_VALUE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_io2_value(HW_OQSPI_IO_VALUE value)
{
        ASSERT_WARNING(IS_HW_OQSPI_IO_VALUE(value));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO2_DAT, (uint32_t) value);
}

/**
 * \brief       Get the value of OSPI_IO2 pad when OSPI_IO2 direction is output
 *
 * \return      The value of OSPI_IO2 pad
 *
 * \sa          HW_OQSPI_IO_VALUE
 */
__STATIC_FORCEINLINE HW_OQSPI_IO_VALUE hw_oqspi_get_io2_value(void)
{
        return (HW_OQSPI_IO_VALUE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO2_DAT);
}

/**
 * \brief       Set the value of OSPI_IO3 pad when OSPI_IO3 direction is output
 *
 * \param [in]  value The value of OSPI_IO3 pad
 *
 * \sa          HW_OQSPI_IO_VALUE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_io3_value(HW_OQSPI_IO_VALUE value)
{
        ASSERT_WARNING(IS_HW_OQSPI_IO_VALUE(value));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO3_DAT, (uint32_t) value);
}

/**
 * \brief       Get the value of OSPI_IO3 pad when OSPI_IO3 direction is output
 *
 * \return      The value of OSPI_IO3 pad
 *
 * \sa          HW_OQSPI_IO_VALUE
 */
__STATIC_FORCEINLINE HW_OQSPI_IO_VALUE hw_oqspi_get_io3_value(void)
{
        return (HW_OQSPI_IO_VALUE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO3_DAT);
}

/**
 * \brief       Set the value of OSPI_IO4-7 pads when OSPI_IO4-7 direction is output
 *
 * \param [in]  value The value of OSPI_IO4-7 pads
 *
 * \sa          HW_OQSPI_IO4_7_VALUE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_io4_7_value(HW_OQSPI_IO4_7_VALUE value)
{
        ASSERT_WARNING(IS_HW_OQSPI_IO4_7_VALUE(value));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO_UH_DAT, (uint32_t) value);
}

/**
 * \brief       Get the value of OSPI_IO4-7 pad when OSPI_IO4-7 direction is output
 *
 * \return      The value of OSPI_IO4-7 pad
 *
 * \sa          HW_OQSPI_IO4_7_VALUE
 */
__STATIC_FORCEINLINE HW_OQSPI_IO4_7_VALUE hw_oqspi_get_io4_7_value(void)
{
        return (HW_OQSPI_IO4_7_VALUE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO_UH_DAT);
}

/**
 * \brief       Set the direction and the level of OQSPIC IOs based on the Bus Mode
 *
 * \param [in]  bus_mode The OQSPIC Bus Mode
 *
 * \sa          bus_mode
 */
__STATIC_FORCEINLINE void hw_oqspi_set_io(HW_OQSPI_BUS_MODE bus_mode)
{
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(bus_mode));

        uint32_t ctrlmode_reg = OQSPIF->OQSPIF_CTRLMODE_REG;

        if (bus_mode == HW_OQSPI_BUS_MODE_SINGLE) {
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO2_OEN, ctrlmode_reg, 1);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO2_DAT, ctrlmode_reg, 1);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO3_OEN, ctrlmode_reg, 1);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO3_DAT, ctrlmode_reg, 1);
        } else {
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO2_OEN, ctrlmode_reg, 0);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO2_DAT, ctrlmode_reg, 0);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO3_OEN, ctrlmode_reg, 0);
                REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO3_DAT, ctrlmode_reg, 0);
        }

        REG_SET_FIELD(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_IO_UH_OEN, ctrlmode_reg, 0);

        OQSPIF->OQSPIF_CTRLMODE_REG = ctrlmode_reg;
}

/**
 * \brief       Set OQSPIC HReady signal mode
 *
 * \param [in]  mode HReady signal mode
 *
 * \sa          HW_OQSPI_HREADY_MODE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_hready_mode(HW_OQSPI_HREADY_MODE mode)
{
        ASSERT_WARNING(IS_HW_OQSPI_HREADY_MODE(mode));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_HRDY_MD, mode);
}

/**
 * \brief       Get OQSPIC HReady signal mode
 *
 * \return      HReady signal mode
 *
 * \sa          HW_OQSPI_HREADY_MODE
 */
__STATIC_FORCEINLINE HW_OQSPI_HREADY_MODE hw_oqspi_get_hready_mode(void)
{
        return (HW_OQSPI_HREADY_MODE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_HRDY_MD);
}

/**
 * \brief       Set OQSPIC read sampling edge
 *
 * \param [in]  edge Read sampling edge
 *
 * \sa          HW_OQSPI_SAMPLING_EDGE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_read_sampling_edge(HW_OQSPI_SAMPLING_EDGE edge)
{
        ASSERT_WARNING(IS_HW_OQSPI_SAMPLING_EDGE(edge));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_RXD_NEG, edge);
}

/**
 * \brief       Get OQSPIC read sampling edge
 *
 * \return      Read sampling edge
 *
 * \sa          HW_OQSPI_SAMPLING_EDGE
 */
__STATIC_FORCEINLINE HW_OQSPI_SAMPLING_EDGE hw_oqspi_get_read_sampling_edge(void)
{
        return (HW_OQSPI_SAMPLING_EDGE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_RXD_NEG);
}

/**
 * \brief       Set OQSPIC data read pipe status
 *
 * \param [in]  read_pipe Status of data read pipe
 *
 * \sa          HW_OQSPI_READ_PIPE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_read_pipe(HW_OQSPI_READ_PIPE read_pipe)
{
        ASSERT_WARNING(IS_HW_OQSPI_READ_PIPE(read_pipe));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_RPIPE_EN, read_pipe);
}

/**
 * \brief       Get OQSPIC read pipe status
 *
 * \return      Status of data read pipe
 *
 * \sa          HW_OQSPI_READ_PIPE
 */
__STATIC_FORCEINLINE HW_OQSPI_READ_PIPE hw_oqspi_get_read_pipe(void)
{
        return (HW_OQSPI_READ_PIPE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_RPIPE_EN);
}

/**
 * \brief       Set the OQSPIC read pipe clock delay
 *
 * \param [in]  delay Read pipe clock delay
 *
 * \sa          HW_OQSPI_READ_PIPE_DELAY
 */
__STATIC_FORCEINLINE void hw_oqspi_set_read_pipe_clock_delay(HW_OQSPI_READ_PIPE_DELAY delay)
{
        ASSERT_WARNING(IS_HW_OQSPI_READ_PIPE_DELAY(delay));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_PCLK_MD, delay);
}

/**
 * \brief       Get OQSPIC read pipe clock delay
 *
 * \return      Read pipe clock delay
 *
 * \sa          HW_OQSPI_READ_PIPE_DELAY
 */
__STATIC_FORCEINLINE HW_OQSPI_READ_PIPE_DELAY hw_oqspi_get_read_pipe_clock_delay(void)
{
        return (HW_OQSPI_READ_PIPE_DELAY) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_PCLK_MD);
}

/**
 * \brief       Set OQSPIC full buffer mode
 *
 * \note        This setting has meaning only for the read in auto mode
 *
 * \param [in]  full_buffer_mode OQSPIC full buffer mode
 *
 * \sa          HW_OQSPI_FULL_BUFFER_MODE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_full_buffer_mode(HW_OQSPI_FULL_BUFFER_MODE full_buffer_mode)
{
        ASSERT_WARNING(IS_HW_OQSPI_FULL_BUFFER_MODE(full_buffer_mode));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_BUF_LIM_EN, full_buffer_mode);
}

/**
 * \brief       Get OQSPIC full buffer mode
 *
 * \return      OQSPIC full buffer mode
 *
 * \sa          HW_OQSPI_FULL_BUFFER_MODE
 */
__STATIC_FORCEINLINE HW_OQSPI_FULL_BUFFER_MODE hw_oqspi_get_full_buffer_mode(void)
{
        return (HW_OQSPI_FULL_BUFFER_MODE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_BUF_LIM_EN);
}

/**
 * \brief       Set OQSPIC address size
 *
 * \param [in]  addr_size OQSPIC address size
 *
 * \sa          HW_OQSPI_ADDR_SIZE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_address_size(HW_OQSPI_ADDR_SIZE addr_size)
{
        ASSERT_WARNING(IS_HW_OQSPI_ADDR_SIZE(addr_size));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_USE_32BA, addr_size);
}

/**
 * \brief       Get OQSPIC address size
 *
 * \return      OQSPIC address size
 *
 * \sa          HW_OQSPI_ADDR_SIZE
 */
__STATIC_FORCEINLINE HW_OQSPI_ADDR_SIZE hw_oqspi_get_address_size(void)
{
        return (HW_OQSPI_ADDR_SIZE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_USE_32BA);
}

/**
 * \brief       Set OQSPIC opcode length in auto mode
 *
 * \param [in]  opcode_len OQSPIC opcode length in auto mode
 *
 * \sa          HW_OQSPI_OPCODE_LEN
 */
__STATIC_FORCEINLINE void hw_oqspi_set_opcode_len(HW_OQSPI_OPCODE_LEN opcode_len)
{
        ASSERT_WARNING(IS_HW_OQSPI_OPCODE_LEN(opcode_len));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_CMD_X2_EN, opcode_len);
}

/**
 * \brief       Get OQSPIC opcode length in auto mode
 *
 * \return      OQSPIC opcode length in auto mode
 *
 * \sa          HW_OQSPI_OPCODE_LEN
 */
__STATIC_FORCEINLINE HW_OQSPI_OPCODE_LEN hw_oqspi_get_opcode_len(void)
{
        return (HW_OQSPI_OPCODE_LEN) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_CMD_X2_EN);
}

/**
 * \brief       Set OQSPIC dummy mode
 *
 * \param [in]  dummy_mode OQSPIC dummy mode
 *
 * \sa          HW_OQSPI_DUMMY_MODE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_dummy_mode(HW_OQSPI_DUMMY_MODE dummy_mode)
{
        ASSERT_WARNING(IS_HW_OQSPI_DUMMY_MODE(dummy_mode));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_DMY_MD, dummy_mode);
}

/**
 * \brief       Get OQSPIC dummy mode
 *
 * \return      OQSPIC dummy mode
 *
 * \sa          HW_OQSPI_DUMMY_MODE
 */
__STATIC_FORCEINLINE HW_OQSPI_DUMMY_MODE hw_oqspi_get_dummy_mode(void)
{
        return (HW_OQSPI_DUMMY_MODE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_DMY_MD);
}

/**
 * \brief       Set OQSPIC direction change mode in manual access mode
 *
 * \param [in]  dir_change_mode OQSPIC direction change mode
 *
 * \sa          HW_OQSPI_DIR_CHANGE_MODE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_dir_change_mode(HW_OQSPI_DIR_CHANGE_MODE dir_change_mode)
{
        ASSERT_WARNING(IS_HW_OQSPI_DIR_CHANGE_MODE(dir_change_mode));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_MAN_DIRCHG_MD, dir_change_mode);
}

/**
 * \brief       Get OQSPIC direction change mode in manual access mode
 *
 * \return      OQSPIC direction change mode
 *
 * \sa          HW_OQSPI_DIR_CHANGE_MODE
 */
__STATIC_FORCEINLINE HW_OQSPI_DIR_CHANGE_MODE hw_oqspi_get_dir_change_mode(void)
{
        return (HW_OQSPI_DIR_CHANGE_MODE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_MAN_DIRCHG_MD);
}

/**
 * \brief       Set OQSPIC AHB bus error response when a read is performed in the address space
 *              where the flash device is mapped and the Auto mode is not enabled
 *
 * \param [in]  read_access_response AHB bus read access response
 *
 * \sa          HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_mapped_addr_read_access_response(
                                HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE read_access_response)
{
        ASSERT_WARNING(IS_HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE(read_access_response));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_RD_ERR_EN, read_access_response);
}

/**
 * \brief       Get OQSPIC AHB bus error response when a read is performed in the address space
 *              where the flash device is mapped and the Auto mode is not enabled
 *
 * \return      AHB bus read access response
 *
 * \sa          HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE
 */
__STATIC_FORCEINLINE HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE hw_oqspi_get_mapped_addr_read_access_response(void)
{
        return (HW_OQSPI_MAPPED_ADDR_RD_ACC_RESPONSE) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_RD_ERR_EN);
}

/**
 * \brief       Set OQSPIC burst length in auto mode when the read access in the AHB bus is an
 *              incremental burst of unspecified length
 *
 * \param [in]  burst_len_limit burst length limit
 *
 * \sa          HW_OQSPI_BURST_LEN_LIMIT
 */
__STATIC_FORCEINLINE void hw_oqspi_set_burst_len_limit(HW_OQSPI_BURST_LEN_LIMIT burst_len_limit)
{
        ASSERT_WARNING(IS_HW_OQSPI_BURST_LEN_LIMIT(burst_len_limit));

        REG_SETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_INC_LIM_EN, burst_len_limit);
}

/**
 * \brief       Get OQSPIC burst length in auto mode when the read access in the AHB bus is an
 *              incremental burst of unspecified length
 *
 * \return      burst length limit
 *
 * \sa          HW_OQSPI_BURST_LEN_LIMIT
 */
__STATIC_FORCEINLINE HW_OQSPI_BURST_LEN_LIMIT hw_oqspi_get_burst_len_limit(void)
{
        return (HW_OQSPI_BURST_LEN_LIMIT) REG_GETF(OQSPIF, OQSPIF_CTRLMODE_REG, OSPIC_INC_LIM_EN);
}

/**
 * \brief Set slew rate of OQSPIC pads
 *
 * \param [in] slew_rate OQSPIC pads slew rate
 *
 * \sa          HW_OQSPI_SLEW_RATE
 */
__STATIC_FORCEINLINE void hw_oqspi_set_slew_rate(HW_OQSPI_SLEW_RATE slew_rate)
{
        ASSERT_WARNING(IS_HW_OQSPI_SLEW_RATE(slew_rate));

        REG_SETF(OQSPIF, OQSPIF_GP_REG, OSPIC_PADS_SLEW , slew_rate);
}

/**
 * \brief       Get slew rate of OQSPIC pads
 *
 * \return      Slew rate of OQSPIC pads
 *
 * \sa          HW_OQSPI_SLEW_RATE
 */
__STATIC_FORCEINLINE HW_OQSPI_SLEW_RATE hw_oqspi_get_slew_rate(void)
{
        return (HW_OQSPI_SLEW_RATE) REG_GETF(OQSPIF, OQSPIF_GP_REG, OSPIC_PADS_SLEW);
}

/**
 * \brief       Set drive current of OQSPIC pads
 *
 * \param [in]  drive_current OQSPIC pads drive current
 *
 * \sa          HW_OQSPI_DRIVE_CURRENT
 */
__STATIC_FORCEINLINE void hw_oqspi_set_drive_current(HW_OQSPI_DRIVE_CURRENT drive_current)
{
        ASSERT_WARNING(IS_HW_OQSPI_DRIVE_CURRENT(drive_current));

        REG_SETF(OQSPIF, OQSPIF_GP_REG, OSPIC_PADS_DRV, drive_current);
}

/**
 * \brief       Get drive current of OQSPIC pads
 *
 * \return      Drive current of OQSPIC pads
 *
 * \sa          HW_OQSPI_DRIVE_CURRENT
 */
__STATIC_FORCEINLINE HW_OQSPI_DRIVE_CURRENT hw_oqspi_get_drive_current(void)
{
        return (HW_OQSPI_DRIVE_CURRENT) REG_GETF(OQSPIF, OQSPIF_GP_REG, OSPIC_PADS_DRV);
}

/**
 * \brief       Set the number of dummy bytes in auto access mode
 *
 * \param [in]  dummy_bytes Number of dummy bytes (0 - 32)
 */
__STATIC_FORCEINLINE void hw_oqspi_set_dummy_bytes(uint8_t dummy_bytes)
{
        ASSERT_WARNING(dummy_bytes <= 32);

        if (dummy_bytes == 0) {
                REG_CLR_BIT(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DMY_EN);
                REG_SETF(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DMY_NUM, 0);
        } else {
                REG_SET_BIT(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DMY_EN);
                REG_SETF(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DMY_NUM, (dummy_bytes - 1));
        }
}

/**
 * \brief       Get the number of dummy bytes in auto access mode
 *
 * \return      Number of dummy bytes (0 - 32)
 */
__STATIC_FORCEINLINE uint8_t hw_oqspi_get_dummy_bytes(void)
{
        if (REG_GETF(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DMY_EN) == 0) {
                return 0;
        }

        return (uint8_t) (REG_GETF(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DMY_NUM) + 1);
}

/**
 * \brief       Set the number of dummy bytes during the read status instruction in auto access mode
 *
 * \param [in]  dummy_bytes Number of dummy bytes (0 - 16)
 */
__STATIC_FORCEINLINE void hw_oqspi_set_read_status_dummy_bytes(uint8_t dummy_bytes)
{
        ASSERT_WARNING(dummy_bytes <= 16);

        if (dummy_bytes == 0 ) {
                REG_CLR_BIT(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_EN);
        } else {
                REG_SETF(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_NUM, (dummy_bytes - 1));
                REG_SET_BIT(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_EN);
        }
}

/**
 * \brief       Get the number of dummy bytes during the read status instruction in auto access mode
 *
 * \return      Number of dummy bytes (0 - 16)
 */
__STATIC_FORCEINLINE uint8_t hw_oqspi_get_read_status_dummy_bytes(void)
{
        if (REG_GETF(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_EN) == 0) {
                return 0;
        }

        return (uint8_t) (REG_GETF(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_NUM) + 1);
}

/**
 * \brief Set the minimum number of clocks cycles that CS stays in idle mode, between two
 *        consecutive read commands
 *
 * \param [in] cs_idle_delay_nsec The minimum time in nsec that the CS signal stays idle
 * \param [in] clk_freq_hz        The OQSPI controller clock frequency (in Hz)
 *
 */
__STATIC_FORCEINLINE void hw_oqspi_set_read_cs_idle_delay(uint16_t cs_idle_delay_nsec,
                                                          uint32_t clk_freq_hz)
{
        uint32_t cs_idle_delay_clk;

        cs_idle_delay_clk = NSEC_TO_CLK_CYCLES(cs_idle_delay_nsec, clk_freq_hz);

        ASSERT_WARNING(cs_idle_delay_clk < 8);
        REG_SETF(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_CS_HIGH_MIN, cs_idle_delay_clk);
}

/**
 * \brief Set the minimum number of clocks cycles that CS stays in idle mode, between a write enable,
 *        erase, erase suspend and erase resume instruction and the next consecutive command.
 *
 * \param [in] cs_idle_delay_nsec The minimum time in nsec that the CS signal stays idle
 * \param [in] clk_freq_hz        The OQSPI controller clock frequency (in Hz)
 *
 */
__STATIC_FORCEINLINE void hw_oqspi_set_erase_cs_idle_delay(uint16_t cs_idle_delay_nsec,
                                                           uint32_t clk_freq_hz)
{
        uint32_t cs_idle_delay_clk;

        cs_idle_delay_clk = NSEC_TO_CLK_CYCLES(cs_idle_delay_nsec, clk_freq_hz);

        ASSERT_WARNING(cs_idle_delay_clk < 32);
        REG_SETF(OQSPIF, OQSPIF_ERASECMDB_REG, OSPIC_ERS_CS_HI, cs_idle_delay_clk);
}

/**
 * \brief       Generate 32 bits data transfer from the external device to the OQSPIC (manual mode)
 *
 * \return      32 bits value read from the bus
 */
__STATIC_FORCEINLINE uint32_t hw_oqspi_read32(void)
{
        hw_oqspi_regs_t *oqspi_regs = (hw_oqspi_regs_t *) (OQSPIF_BASE);

        return oqspi_regs->OQSPIF_READDATA_REG.data32;
}

/**
 * \brief       Generate 16 bits data transfer from the external device to the OQSPIC (manual mode)
 *
 * \return      16 bits value read from the bus
 */
__STATIC_FORCEINLINE uint16_t hw_oqspi_read16(void)
{
        hw_oqspi_regs_t *oqspi_regs = (hw_oqspi_regs_t *) (OQSPIF_BASE);

        return oqspi_regs->OQSPIF_READDATA_REG.data16;
}

/**
 * \brief       Generate 8 bits data transfer from the external device to the OQSPIC (manual mode)
 *
 * \return      8 bits value read from the bus
 */
__STATIC_FORCEINLINE uint8_t hw_oqspi_read8(void)
{
        hw_oqspi_regs_t *oqspi_regs = (hw_oqspi_regs_t *) (OQSPIF_BASE);

        return oqspi_regs->OQSPIF_READDATA_REG.data8;
}

/**
 * \brief       Generate 32 bits data transfer from the OQSPIC to the external device (manual mode)
 *
 * \param [in]  data 32 bits value to be written on the device
 */
__STATIC_FORCEINLINE void hw_oqspi_write32(uint32_t data)
{
        hw_oqspi_regs_t *oqspi_regs = (hw_oqspi_regs_t *) (OQSPIF_BASE);

        oqspi_regs->OQSPIF_WRITEDATA_REG.data32 = SWAP32(data);
}

/**
 * \brief       Generate 16 bits data transfer from the OQSPIC to the external device (manual mode)
 *
 * \param [in]  data 16 bits value to be written on the device
 */
__STATIC_FORCEINLINE void hw_oqspi_write16(uint16_t data)
{
        hw_oqspi_regs_t *oqspi_regs = (hw_oqspi_regs_t *) (OQSPIF_BASE);

        oqspi_regs->OQSPIF_WRITEDATA_REG.data16 = SWAP16(data);
}

/**
 * \brief       Generate 8 bits data transfer from the OQSPIC to the external device (manual mode)
 *
 * \param [in]  data 8 bits value to be written on the device
 */
__STATIC_FORCEINLINE void hw_oqspi_write8(uint8_t data)
{
        hw_oqspi_regs_t *oqspi_regs = (hw_oqspi_regs_t *) (OQSPIF_BASE);

        oqspi_regs->OQSPIF_WRITEDATA_REG.data8 = data;
}

/**
 * \brief       Generate clock pulses on the SPI bus for a 32-bit transfer
 *
 * \note        During the last clock of this activity in the SPI bus, the OSPI_IOx data pads are
 *              in hi-z state. The number of generated pulses is equal to: (size of AHB bus access)
 *              / (size of SPI bus). The size of SPI bus can be 1, 2, 4 or 8 for Single, Dual, Quad
 *              or Octal SPI mode respectively.
 */
__STATIC_FORCEINLINE void hw_oqspi_dummy32(void)
{
        hw_oqspi_regs_t *oqspi_regs = (hw_oqspi_regs_t *) (OQSPIF_BASE);

        oqspi_regs->OQSPIF_DUMMYDATA_REG.data32 = 0;
}

/**
 * \brief       Generate clock pulses on the SPI bus for a 16-bit transfer
 *
 * \note        During the last clock of this activity in the SPI bus, the OSPI_IOx data pads are
 *              in hi-z state. The number of generated pulses is equal to: (size of AHB bus access)
 *              / (size of SPI bus). The size of SPI bus can be 1, 2, 4 or 8 for Single, Dual, Quad
 *              or Octal SPI mode respectively.
 */
__STATIC_FORCEINLINE void hw_oqspi_dummy16(void)
{
        hw_oqspi_regs_t *oqspi_regs = (hw_oqspi_regs_t *) (OQSPIF_BASE);

        oqspi_regs->OQSPIF_DUMMYDATA_REG.data16 = 0;
}

/**
 * \brief       Generate clock pulses on the SPI bus for an 8-bit transfer
 *
 * \note        During the last clock of this activity in the SPI bus, the OSPI_IOx data pads are
 *              in hi-z state. The number of generated pulses is equal to: (size of AHB bus access)
 *              / (size of SPI bus). The size of SPI bus can be 1, 2, 4 or 8 for Single, Dual, Quad
 *              or Octal SPI mode respectively.
 */
__STATIC_FORCEINLINE void hw_oqspi_dummy8(void)
{
        hw_oqspi_regs_t *oqspi_regs = (hw_oqspi_regs_t *) (OQSPIF_BASE);

        oqspi_regs->OQSPIF_DUMMYDATA_REG.data8 = 0;
}

/**
 * \brief       Initialize the OQSPI controller (OQSPIC)
 *
 * \param [in]  cfg     Pointer to OQSPIC configuration structure.
 *
 * \sa          hw_oqspi_config_t
 */
__RETAINED_CODE void hw_oqspi_init(const hw_oqspi_config_t *cfg);

/**
 * \brief       Initialize the read instruction of the OQSPIC
 *
 * \param [in]  cfg             Pointer to configuration structure of the read instruction.
 * \param [in]  dummy_bytes     The number of dummy bytes.
 * \param [in]  sys_clk_freq_hz The system clock frequency in Hz, which is used to calculate the
 *                              minimum OQSPI bus clock cycles that the Chip Select (CS) signal must
 *                              remain high between two consecutive read instructions.
 *
 * \sa          hw_oqspi_read_instr_config_t
 */
__STATIC_FORCEINLINE void hw_oqspi_read_instr_init(const hw_oqspi_read_instr_config_t *cfg,
                                                   uint8_t dummy_bytes, uint32_t sys_clk_freq_hz)
{
        uint32_t ospi_clk_freq_hz = sys_clk_freq_hz >> (uint32_t) hw_oqspi_get_div();
        uint32_t delay_clk_cycles = NSEC_TO_CLK_CYCLES(cfg->cs_idle_delay_nsec, ospi_clk_freq_hz);
        uint32_t burstcmda_reg = OQSPIF->OQSPIF_BURSTCMDA_REG;
        uint32_t burstcmdb_reg = OQSPIF->OQSPIF_BURSTCMDB_REG;

        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->opcode_bus_mode));
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->addr_bus_mode));
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->extra_byte_bus_mode));
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->dummy_bus_mode));
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->data_bus_mode));
        ASSERT_WARNING(IS_HW_OQSPI_EXTRA_BYTE(cfg->extra_byte_cfg));
        ASSERT_WARNING(IS_HW_OQSPI_EXTRA_BYTE_HALF(cfg->extra_byte_half_cfg));
        ASSERT_WARNING(IS_HW_OQSPI_CONTINUOUS_MODE(cfg->continuous_mode));
        ASSERT_WARNING(dummy_bytes < 32);
        ASSERT_WARNING(delay_clk_cycles < 8);

        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDA_REG, OSPIC_INST, burstcmda_reg, cfg->opcode);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDA_REG, OSPIC_EXT_BYTE, burstcmda_reg, cfg->extra_byte_value);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDA_REG, OSPIC_INST_TX_MD, burstcmda_reg, cfg->opcode_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDA_REG, OSPIC_ADR_TX_MD, burstcmda_reg, cfg->addr_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDA_REG, OSPIC_EXT_TX_MD, burstcmda_reg, cfg->extra_byte_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDA_REG, OSPIC_DMY_TX_MD, burstcmda_reg, cfg->dummy_bus_mode);

        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DAT_RX_MD, burstcmdb_reg, cfg->data_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_EXT_BYTE_EN, burstcmdb_reg, cfg->extra_byte_cfg);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_EXT_HF_DS, burstcmdb_reg, cfg->extra_byte_half_cfg);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_INST_MD, burstcmdb_reg, cfg->continuous_mode);

        if (dummy_bytes == 0) {
                REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DMY_EN, burstcmdb_reg, 0);
                REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DMY_NUM, burstcmdb_reg, 0);
        } else {
                REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DMY_EN, burstcmdb_reg, 1);
                REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_DMY_NUM, burstcmdb_reg, (dummy_bytes - 1));
        }

        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_INST_MD, burstcmdb_reg, cfg->continuous_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_CS_HIGH_MIN, burstcmdb_reg, delay_clk_cycles);

        OQSPIF->OQSPIF_BURSTCMDA_REG = burstcmda_reg;
        OQSPIF->OQSPIF_BURSTCMDB_REG = burstcmdb_reg;
}

/**
 * \brief       Initialize the erase instruction of the OQSPIC
 *
 * \param [in]  cfg             Pointer to configuration structure of the erase instruction.
 * \param [in]  sys_clk_freq_hz The system clock frequency in Hz, which is used to calculate the
 *                              minimum OQSPI bus clock cycles that the Chip Select (CS) signal
 *                              remain must high between an erase instruction and the next
 *                              consecutive instruction.
 *
 * \sa          hw_oqspi_erase_instr_config_t
 */
__STATIC_FORCEINLINE void hw_oqspi_erase_instr_init(const hw_oqspi_erase_instr_config_t *cfg,
                                                    uint32_t sys_clk_freq_hz)
{
        uint32_t ospi_clk_freq_hz = sys_clk_freq_hz >> (uint32_t) hw_oqspi_get_div();
        uint32_t delay_clk_cycles = NSEC_TO_CLK_CYCLES(cfg->cs_idle_delay_nsec, ospi_clk_freq_hz);
        uint32_t erasecmdb_reg = OQSPIF->OQSPIF_ERASECMDB_REG;

        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->opcode_bus_mode));
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->addr_bus_mode));
        ASSERT_WARNING(cfg->hclk_cycles < 16);
        ASSERT_WARNING(delay_clk_cycles < 32);

        REG_SETF(OQSPIF, OQSPIF_ERASECMDA_REG, OSPIC_ERS_INST, cfg->opcode);

        REG_SET_FIELD(OQSPIF, OQSPIF_ERASECMDB_REG, OSPIC_ERS_TX_MD, erasecmdb_reg, cfg->opcode_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_ERASECMDB_REG, OSPIC_EAD_TX_MD, erasecmdb_reg, cfg->addr_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_ERASECMDB_REG, OSPIC_ERSRES_HLD, erasecmdb_reg, cfg->hclk_cycles);
        REG_SET_FIELD(OQSPIF, OQSPIF_ERASECMDB_REG, OSPIC_ERS_CS_HI, erasecmdb_reg, delay_clk_cycles);

        OQSPIF->OQSPIF_ERASECMDB_REG = erasecmdb_reg;
}

/**
 * \brief       Initialize the read status register instruction of the OQSPIC
 *
 * \param [in]  cfg             Pointer to configuration structure of the read status register
 *                              instruction.
 * \param [in]  sys_clk_freq_hz The system clock frequency in Hz, which is used to calculate the
 *                              minimum required delay, in OQSPI bus clock cycles, between an
 *                              erase or erase resume instruction and the next consecutive
 *                              read status register instruction.
 *
 * \sa          hw_oqspi_read_status_instr_config_t
 */
__STATIC_FORCEINLINE void hw_oqspi_read_status_instr_init(const hw_oqspi_read_status_instr_config_t *cfg,
                                                          uint32_t sys_clk_freq_hz)
{
        uint32_t ospi_clk_freq_hz = sys_clk_freq_hz >> (uint32_t) hw_oqspi_get_div();
        uint32_t delay_clk_cycles = NSEC_TO_CLK_CYCLES(cfg->delay_nsec, ospi_clk_freq_hz);
        uint32_t statuscmd_reg = OQSPIF->OQSPIF_STATUSCMD_REG;

        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->opcode_bus_mode));
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->receive_bus_mode));
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->dummy_bus_mode));
        ASSERT_WARNING(IS_HW_OQSPI_BUSY_LEVEL(cfg->busy_level));
        ASSERT_WARNING(IS_HW_OQSPI_READ_STATUS_DUMMY_VAL(cfg->dummy_value));
        ASSERT_WARNING(cfg->busy_pos < 8);
        ASSERT_WARNING(cfg->dummy_bytes <= 16);
        ASSERT_WARNING(delay_clk_cycles < 64);

        REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_INST, statuscmd_reg, cfg->opcode);
        REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_TX_MD, statuscmd_reg, cfg->opcode_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_RX_MD, statuscmd_reg, cfg->receive_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_TX_MD, statuscmd_reg, cfg->dummy_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_BUSY_POS, statuscmd_reg, cfg->busy_pos);
        REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_BUSY_VAL, statuscmd_reg, cfg->busy_level);
        REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RESSTS_DLY, statuscmd_reg, delay_clk_cycles);
        REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_STSDLY_SEL, statuscmd_reg, 0);

        if (cfg->dummy_bytes == 0) {
                REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_NUM, statuscmd_reg, 0);
                REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_EN, statuscmd_reg, 0);
        } else {
                REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_NUM, statuscmd_reg, (cfg->dummy_bytes - 1));
                REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_EN, statuscmd_reg, 1);
        }

        REG_SET_FIELD(OQSPIF, OQSPIF_STATUSCMD_REG, OSPIC_RSTAT_DMY_ZERO, statuscmd_reg, cfg->dummy_value);

        OQSPIF->OQSPIF_STATUSCMD_REG = statuscmd_reg;
}

/**
 * \brief       Initialize the write enable instruction of the OQSPIC
 *
 * \param [in]  cfg     Pointer to configuration structure of the write enable instruction.
 *
 * \sa          hw_oqspi_write_enable_instr_config_t
 */
__STATIC_FORCEINLINE void hw_oqspi_write_enable_instr_init(const hw_oqspi_write_enable_instr_config_t *cfg)
{
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->opcode_bus_mode));

        REG_SETF(OQSPIF, OQSPIF_ERASECMDA_REG, OSPIC_WEN_INST, cfg->opcode);
        REG_SETF(OQSPIF, OQSPIF_ERASECMDB_REG, OSPIC_WEN_TX_MD, cfg->opcode_bus_mode);
}

/**
 * \brief       Initialize the program and erase suspend/resume instruction of the OQSPIC
 *
 * \param [in]  cfg     Pointer to configuration structure of the program and erase suspend/resume
 *                      instruction.
 *
 * \sa          hw_oqspi_suspend_resume_instr_config_t
 */
__STATIC_FORCEINLINE void hw_oqspi_suspend_resume_instr_init(const hw_oqspi_suspend_resume_instr_config_t *cfg)
{
        uint32_t suspend_latency_clk_cycles = NSEC_TO_CLK_CYCLES((1000 * cfg->suspend_latency_usec), SUSPEND_RESUME_COUNTER_FREQ_HZ);
        uint32_t res_sus_latency_clk_cycles = NSEC_TO_CLK_CYCLES((1000 * cfg->res_sus_latency_usec), SUSPEND_RESUME_COUNTER_FREQ_HZ);

        uint32_t erasecmda_reg = OQSPIF->OQSPIF_ERASECMDA_REG;
        uint32_t erasecmdb_reg = OQSPIF->OQSPIF_ERASECMDB_REG;

        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->suspend_bus_mode));
        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->resume_bus_mode));
        ASSERT_WARNING(suspend_latency_clk_cycles <= 63);
        ASSERT_WARNING(res_sus_latency_clk_cycles <= 255);

        REG_SET_FIELD(OQSPIF, OQSPIF_ERASECMDA_REG, OSPIC_SUS_INST, erasecmda_reg, cfg->suspend_opcode);
        REG_SET_FIELD(OQSPIF, OQSPIF_ERASECMDA_REG, OSPIC_RES_INST, erasecmda_reg, cfg->resume_opcode);

        REG_SET_FIELD(OQSPIF, OQSPIF_ERASECMDB_REG, OSPIC_SUS_TX_MD, erasecmdb_reg, cfg->suspend_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_ERASECMDB_REG, OSPIC_RES_TX_MD, erasecmdb_reg, cfg->resume_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_ERASECMDB_REG, OSPIC_RESSUS_DLY, erasecmdb_reg, res_sus_latency_clk_cycles);

        OQSPIF->OQSPIF_ERASECMDA_REG = erasecmda_reg;
        OQSPIF->OQSPIF_ERASECMDB_REG = erasecmdb_reg;

        REG_SETF(OQSPIF, OQSPIF_ERASECMDC_REG, OSPIC_SUSSTS_DLY, suspend_latency_clk_cycles);
}

/**
 * \brief       Initialize the exit from continuous mode instruction of the OQSPIC
 *
 * \param [in]  cfg     Pointer to configuration structure of the exit from continuous mode instruction.
 *
 * \sa          hw_oqspi_exit_continuous_mode_instr_config_t
 */
__STATIC_FORCEINLINE void hw_oqspi_exit_continuous_mode_instr_init(const hw_oqspi_exit_continuous_mode_instr_config_t *cfg)
{
        uint32_t burstbrk_reg = OQSPIF->OQSPIF_BURSTBRK_REG;

        ASSERT_WARNING(IS_HW_OQSPI_BUS_MODE(cfg->opcode_bus_mode));
        ASSERT_WARNING(cfg->sequence_len <= 16);

        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTBRK_REG, OSPIC_BRK_WRD, burstbrk_reg, cfg->opcode);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTBRK_REG, OSPIC_BRK_TX_MD, burstbrk_reg, cfg->opcode_bus_mode);
        REG_SET_FIELD(OQSPIF, OQSPIF_BURSTBRK_REG, OSPIC_SEC_HF_DS, burstbrk_reg, cfg->disable_second_half);

        if (cfg->sequence_len == 0) {
                REG_SET_FIELD(OQSPIF, OQSPIF_BURSTBRK_REG, OSPIC_BRK_EN, burstbrk_reg, 0);
                REG_SET_FIELD(OQSPIF, OQSPIF_BURSTBRK_REG, OSPIC_BRK_SZ, burstbrk_reg, 0);
        } else {
                REG_SET_FIELD(OQSPIF, OQSPIF_BURSTBRK_REG, OSPIC_BRK_EN, burstbrk_reg, 1);
                REG_SET_FIELD(OQSPIF, OQSPIF_BURSTBRK_REG, OSPIC_BRK_SZ, burstbrk_reg, (cfg->sequence_len - 1));
        }

        OQSPIF->OQSPIF_BURSTBRK_REG = burstbrk_reg;
}

/**
 * \brief       Set the address of the block/sector that is requested to be erased.
 */
__STATIC_FORCEINLINE void hw_oqspi_set_erase_address(uint32_t erase_addr)
{
        REG_SETF(OQSPIF, OQSPIF_ERASECTRL_REG, OSPIC_ERS_ADDR, erase_addr);
}

/**
 * \brief Trigger erase block/sector
 */
__STATIC_FORCEINLINE void hw_oqspi_trigger_erase(void)
{
        REG_SET_BIT(OQSPIF, OQSPIF_ERASECTRL_REG, OSPIC_ERASE_EN);
}

/**
 * \brief       Get erase status
 *
 * \return      The status of sector/block erasing
 *
 * \sa          HW_OQSPI_ERASE_STATUS
 */
__STATIC_FORCEINLINE HW_OQSPI_ERASE_STATUS hw_oqspi_get_erase_status(void)
{
        // Dummy access to OQSPIF_CHCKERASE_REG in order to trigger a read status command
        REG_SETF(OQSPIF, OQSPIF_CHCKERASE_REG,  OSPIC_CHCKERASE, 0);
        return (HW_OQSPI_ERASE_STATUS) REG_GETF(OQSPIF, OQSPIF_ERASECTRL_REG, OSPIC_ERS_STATE);
}

/**
 * \brief       Disable the erase resume procedure. The erase will not be resumed after the
 *              expiration of the OSPIC_ERSRES_HLD unless re-enabling the corresponding setting by
 *              calling hw_oqspi_enable_erase_resume().
 *
 * \sa          hw_oqspi_enable_erase_resume
 */
__STATIC_FORCEINLINE void hw_oqspi_disable_erase_resume(void)
{
        REG_SET_BIT(OQSPIF, OQSPIF_ERASECTRL_REG, OSPIC_ERS_RES_DIS);
}

/**
 * \brief       Enable the erase resume procedure
 *
 * \sa          hw_oqspi_disable_erase_resume
 */
__STATIC_FORCEINLINE void hw_oqspi_enable_erase_resume(void)
{
        REG_CLR_BIT(OQSPIF, OQSPIF_ERASECTRL_REG, OSPIC_ERS_RES_DIS);
}

/**
 * \brief       Erase block/sector of flash memory
 *
 * \note        Before erasing the flash memory, it is mandatory to set up the erase instructions
 *              first by calling hw_oqspi_erase_instr_init().
 *
 * \note        Call hw_oqspi_get_erase_status() to check whether the erase operation has finished.
 *
 * \note        Before switching the OSPI controller to manual mode check that
 *              hw_oqspi_get_erase_status() == HW_OQSPI_ERASE_STATUS_NO.
 *
 * \param [in]  addr memory address of the block/sector to be erased
 *
 * \sa          hw_oqspi_erase_instr_init
 * \sa          hw_oqspi_get_erase_status
 */
__RETAINED_CODE void hw_oqspi_erase_block(uint32_t addr);

/**
 * \brief       Enable the AES-CTR decryption
 */
__STATIC_FORCEINLINE void hw_oqspi_enable_aes_ctr(void)
{
        REG_SET_BIT(OQSPIF, OQSPIF_CTR_CTRL_REG, OSPIC_CTR_EN);
        __ISB();
}

/**
 * \brief       Disable the AES-CTR decryption
 */
__STATIC_FORCEINLINE void hw_oqspi_disable_aes_ctr(void)
{
       REG_CLR_BIT(OQSPIF, OQSPIF_CTR_CTRL_REG, OSPIC_CTR_EN);
       __ISB();
}

/**
 * \brief       Set the nonce value used by AES-CTR decryption algorithm
 *
 * \details     The OQSPI controller decrypts Flash contents on-the-fly using AES-CTR. AES-CTR uses
 *              a 16-byte counter block (CTRB). The first 8 bytes of CTRB consist of the NONCE while
 *              the other 8-bytes are produced automatically by the hardware.
 *
 * \param [in]  nonce Pointer to the buffer of the nonce value (8 bytes)
 */
__RETAINED_CODE void hw_oqspi_set_aes_ctr_nonce(const uint8_t *nonce);

/**
 * \brief       Set the key for AES-CTR decryption
 *
 * \param [in]  key Pointer to the buffer of the key value (32 bytes)
 */
__RETAINED_CODE void hw_oqspi_set_aes_ctr_key(const uint8_t *key);

/**
 * \brief       Set the OQSPI flash memory address range where its contents will be decrypted
 *
 * \param [in]  saddr Start address of the decryption area in the OQSPI Flash
 * \param [in]  eaddr End address of the decryption area in the OQSPI Flash
 *
 * \note        Use relative (NOT physical) addresses for both saddr and eaddr
 *
 * \note        The start and the end addresses must fulfill the following conditions:
 *              (a) Must be both 1KB (0x400) aligned. The bits [9:0] are always considered as 0
 *              (b) 'start address' > 'end address' therefore
 *                  'start address' > 'end address' + 0x3FF
 */
__STATIC_FORCEINLINE void hw_oqspi_set_aes_ctr_addr_range(uint32_t saddr, uint32_t eaddr)
{
        ASSERT_ERROR((eaddr > (saddr + 0x3FF)) || ((eaddr == 0x0) && (saddr == 0x0)) );

        REG_SETF(OQSPIF, OQSPIF_CTR_SADDR_REG, OSPIC_CTR_SADDR, saddr >> 10);
        REG_SETF(OQSPIF, OQSPIF_CTR_EADDR_REG, OSPIC_CTR_EADDR, eaddr >> 10);
}

/**
 * \brief       OQSPI controller AES-CTR decryption initialization function
 *
 * \details     Use this function in order to initialize the AES-CTR decryption functionality
 *              of the OQSPIC. Instantiate a type hw_oqspi_aes_ctr_config_t struct, initialize
 *              it with the desired settings and call this function passing the pointer of the
 *              struct as input argument.
 *
 * \param [in]  cfg pointer to OQSPIC AES-CTR decryption configuration structure
 *
 * \sa          hw_oqspi_aes_ctr_config_t
 */
__RETAINED_CODE void hw_oqspi_aes_ctr_init(const hw_oqspi_aes_ctr_config_t *cfg);

/**
 * \brief Set an extra byte to use with read instructions
 *
 * \param [in] extra_byte an extra byte transferred after the address asking memory to
 *                        stay in continuous read mode or wait for a normal instruction
 *                        after CS goes inactive
 * \param [in] bus_mode the mode of the SPI bus during the extra byte phase.
 * \param [in] half_disable_out true  - disable (hi-z) output during the transmission
 *                                      of bits [3:0] of extra byte
 *                              false - transmit the complete extra byte
 *
 * \sa hw_oqspi_set_read_instruction
 *
 */
__STATIC_FORCEINLINE void hw_oqspi_set_extra_byte(uint8_t extra_byte, HW_OQSPI_BUS_MODE bus_mode,
                                                  bool half_disable_out)
{
        REG_SETF(OQSPIF, OQSPIF_BURSTCMDA_REG, OSPIC_EXT_BYTE, extra_byte);
        REG_SETF(OQSPIF, OQSPIF_BURSTCMDA_REG, OSPIC_EXT_TX_MD, bus_mode);

        REG_SETF(OQSPIF, OQSPIF_BURSTCMDB_REG, OSPIC_EXT_BYTE_EN, 1);
        REG_SETF(OQSPIF, OQSPIF_BURSTCMDB_REG,  OSPIC_EXT_HF_DS, half_disable_out);
}

/**
 * \brief Enable the 'exit from continuous read mode' sequence in automode
 *
 */
__STATIC_FORCEINLINE void hw_oqspi_exit_continuous_mode_sequence_enable(void)
{
        REG_SET_BIT(OQSPIF, OQSPIF_BURSTBRK_REG, OSPIC_BRK_EN);
}

/**
 * \brief Disable the 'exit from continuous read mode' sequence in automode
 *
 */
__STATIC_FORCEINLINE void hw_oqspi_exit_continuous_mode_sequence_disable(void)
{
        REG_CLR_BIT(OQSPIF, OQSPIF_BURSTBRK_REG, OSPIC_BRK_EN);
}
#endif /* dg_configUSE_HW_OQSPI */


#endif /* HW_OQSPI_H_ */

/**
 * \}
 * \}
 */
