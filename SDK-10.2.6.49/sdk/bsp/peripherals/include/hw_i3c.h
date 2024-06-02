/**
 * \addtogroup PLA_DRI_PER_COMM
 * \{
 * \addtogroup HW_I3C I3C Driver
 * \{
 * \brief I3C Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_i3c.h
 *
 * @brief Definition of API for the I3C Low Level Driver.
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_I3C_H_
#define HW_I3C_H_

#if dg_configUSE_HW_I3C

#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"

/**
 * \def HW_I3C_DMA_SUPPORT
 *
 * \brief DMA support for I3C
 *
 */
#define HW_I3C_DMA_SUPPORT              dg_configI3C_DMA_SUPPORT

#if HW_I3C_DMA_SUPPORT
#include "hw_dma.h"
#endif

/**
 * \brief Create a contiguous bitmask starting at bit position \p lo and ending at
 * position \p hi.
 *
 * \param [in] lo low position of the bitmask.
 * \param [in] hi high position of the bitmask.
 *
 */
#define GENMASK(hi, lo) \
        (((~0UL) << (lo)) & (~0UL >> (32 - 1 - (hi))))

/**
 * \brief Return the value of a given number \p x starting from bit position \p lo and ending at
 * position \p hi.
 *
 * \param [in] x number
 * \param [in] hi high position of the bitmask.
 * \param [in] lo low position of the bitmask.
 *
 */
#define GET_FIELD_VAL(x, hi, lo)        (((x) & GENMASK((hi), (lo))) >> (lo))

// Response macros

/**
 * \brief Get data length from response.
 *
 * \param [in] x response
 *
 * \details
 * For Write transfers, this field represents the remaining data length
 * of the transfer if the transfer is terminated early:
 * (remaining data length = requested data length - transferred data length)
 * For Read transfers, this field represents the actual amount of data
 * received in bytes.
 * For Address Assignment command, this field represents the remaining
 * device count.
 *
 */
#define HW_I3C_RESPONSE_PORT_DATA_LEN(x)               GET_FIELD_VAL(x, 15, 0)

/**
 * \brief Get error status from response.
 *
 * \param [in] x response
 *
 * \details Defines the Error Type of the processed command
 *
 * \sa HW_I3C_RESPONSE
 *
 */
#define HW_I3C_RESPONSE_PORT_ERR_STATUS(x)             GET_FIELD_VAL(x, 31, 28)

/**
 * \brief Get transaction ID from response.
 *
 * \param [in] x response
 *
 * \details This Field is used as the identification tag for the commands.
 * The I3C controller returns the ID received through commands.
 *
 * \sa HW_I3C_TRANSACTION_ID
 *
 */
#define HW_I3C_RESPONSE_PORT_TID(x)                    GET_FIELD_VAL(x, 27, 24)

/**
 * \brief Callback called on interrupt from I3C controller
 *
 * \param [in] mask interrupt events mask
 *
 */
typedef void (*hw_i3c_interrupt_callback)(uint32_t mask);

/**
 * \brief I3C CMD response definition
 *
 */
typedef struct
{
        uint32 response;
        bool   valid;
} i3c_transfer_cmd_response;

/**
 * \brief Callback called upon completion of transfer in non-blocking mode
 *
 * \param [in] user_data data passed by user along with callback
 * \param [out] success operation status
 * \param [out] cmd_response I3C command response
 *
 */
typedef void (*hw_i3c_xfer_callback)(void *user_data, bool success, i3c_transfer_cmd_response *cmd_response);

/**
 * \brief HOT JOIN request ID
 *
 */
#define HW_I3C_HOT_JOIN_ID              (0x2)

/**
 * \brief In band interrupt status
 *
 */
typedef enum {
        HW_I3C_IBI_STATUS_ACK,           /**< I3C controller responded with ACK to the IBI request      */
        HW_I3C_IBI_STATUS_NACK,          /**< I3C controller responded with NACK to the IBI request     */
} HW_I3C_IBI_STATUS;

/**
 * \brief In band interrupt type
 *
 */
typedef enum {
        HW_I3C_IBI_TYPE_SIR,            /**< IBI type is slave interrupt request  */
        HW_I3C_IBI_TYPE_HJ,             /**< IBI type is Hot Join request         */
} HW_I3C_IBI_TYPE;

/**
 * \brief In band interrupt RnW bit
 *
 */
typedef enum {
        HW_I3C_IBI_RNW_BIT_WRITE,       /**< IBI RnW bit is write */
        HW_I3C_IBI_RNW_BIT_READ,        /**< IBI RnW bit is read  */
} HW_I3C_IBI_RNW_BIT;

/**
 * \brief I3C IBI request structure definition
 *
 */
typedef struct {
     uint8_t                    ibi_id;           /**< Dynamic address for SIR request or 0x2 for Hot Join request      */
     HW_I3C_IBI_STATUS          ibi_status;       /**< ACK or NACK In Band Interrupt                                    */
     HW_I3C_IBI_TYPE            ibi_type;         /**< In Band Interrupt type (SIR or HJ request)                       */
     HW_I3C_IBI_RNW_BIT         ibi_rnw_bit;      /**< In Band Interrupt RnW bit                                        */
} i3c_ibi_sir_hj_request;

/**
 * \brief Callback called upon Slave Interrupt or Hot Join request
 *
 * \param [in] ibi_sir_hj_status_id In Band Interrupt request
 *
 */
typedef void (*hw_i3c_ibi_sir_hj_callback)(i3c_ibi_sir_hj_request ibi_sir_hj_status_id);

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief I3C API Error Codes
 *
 */
typedef enum {
        HW_I3C_ERROR_RESPONSE           = -2,           /**< Error during I3C transaction       */
        HW_I3C_ERROR_INVALID_PARAMETER  = -1,           /**< Invalid parameters                 */
        HW_I3C_ERROR_NONE               =  0            /**< No error                           */
} HW_I3C_ERROR;

/**
* \brief Response codes
*
*/
typedef enum {
        HW_I3C_RESPONSE_ERROR_NO_ERROR        = 0,      /**< No error                                   */
        HW_I3C_RESPONSE_ERROR_CRC             = 1,      /**< CRC error                                  */
        HW_I3C_RESPONSE_ERROR_PARITY          = 2,      /**< Parity error                               */
        HW_I3C_RESPONSE_ERROR_FRAME           = 3,      /**< Framing error in HDR-DDR                   */
        HW_I3C_RESPONSE_ERROR_IBA_NACK        = 4,      /**< IBA Nack'ed                                */
        HW_I3C_RESPONSE_ERROR_ADDRESS_NACK    = 5,      /**< Address Nack'ed                            */
        HW_I3C_RESPONSE_ERROR_OVER_UNDER_FLOW = 6,      /**< Receive/Transmit Buffer Overflow/Underflow */
        HW_I3C_RESPONSE_ERROR_TRANSF_ABORT    = 8,      /**< Transfer Aborted                           */
        HW_I3C_RESPONSE_ERROR_I2C_W_NACK_ERR  = 9,      /**< I2C Slave Write Data NACK Error            */
}  HW_I3C_RESPONSE;

/**
* \brief Device Address Table Locations
*
*/
typedef enum {
       HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION_1,           /**< Address Table Location of Device1 */
       HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION_2,           /**< Address Table Location of Device2 */
       HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION_3,           /**< Address Table Location of Device3 */
       HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION_4,           /**< Address Table Location of Device4 */
       HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION_5,           /**< Address Table Location of Device5 */
       HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION_6,           /**< Address Table Location of Device6 */
       HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION_7,           /**< Address Table Location of Device7 */
       HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION_8,           /**< Address Table Location of Device8 */
} HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION;

/**
* \brief Slave device type
*
* \sa hw_i3c_set_slave_device_address_table
*/
typedef enum {
       HW_I3C_SLAVE_DEVICE_I3C        = 0,               /**< I3C slave device        */
       HW_I3C_SLAVE_DEVICE_LEGACY_I2C = 1,               /**< Legacy I2C slave device */
} HW_I3C_SLAVE_DEVICE;

#if (HW_I3C_DMA_SUPPORT == 1)
/**
* \brief DMA channel-pairs for I3C
*
*/
typedef enum
{
        HW_I3C_DMA_CHANNEL_PAIR_0_1 = 0,          /**< Channel 0 for RX, channel 1 for TX */
        HW_I3C_DMA_CHANNEL_PAIR_2_3 = 2,          /**< Channel 2 for RX, channel 3 for TX */
        HW_I3C_DMA_CHANNEL_PAIR_4_5 = 4,          /**< Channel 4 for RX, channel 5 for TX */
        HW_I3C_DMA_CHANNEL_PAIR_6_7 = 6,          /**< Channel 6 for RX, channel 7 for TX */
} HW_I3C_DMA_CHANNEL_PAIR;
#endif

//===================== FIFOS/QUEUES thresholds ===================================

/**
 * \brief Define the I3C IBI STATUS QUEUE threshold level
 *
 * \sa hw_i3c_set_ibi_status_queue_threshold
 */
typedef enum {
        HW_I3C_IBI_STATUS_QUEUE_TL_1,           /**< I3C IBI Status QUEUE threshold level is 1 entry   */
        HW_I3C_IBI_STATUS_QUEUE_TL_2,           /**< I3C IBI Status QUEUE threshold level is 2 entries */
        HW_I3C_IBI_STATUS_QUEUE_TL_3,           /**< I3C IBI Status QUEUE threshold level is 3 entries */
        HW_I3C_IBI_STATUS_QUEUE_TL_4,           /**< I3C IBI Status QUEUE threshold level is 4 entries */
        HW_I3C_IBI_STATUS_QUEUE_TL_5,           /**< I3C IBI Status QUEUE threshold level is 5 entries */
        HW_I3C_IBI_STATUS_QUEUE_TL_6,           /**< I3C IBI Status QUEUE threshold level is 6 entries */
        HW_I3C_IBI_STATUS_QUEUE_TL_7,           /**< I3C IBI Status QUEUE threshold level is 7 entries */
} HW_I3C_IBI_STATUS_QUEUE_TL;

/**
 * \brief Define the I3C Response QUEUE threshold level
 *
 * \sa hw_i3c_set_resp_queue_threshold
 */
typedef enum {
        HW_I3C_RESP_QUEUE_TL_1,                 /**< I3C Response QUEUE threshold level is 1 entry   */
        HW_I3C_RESP_QUEUE_TL_2,                 /**< I3C Response QUEUE threshold level is 2 entries */
        HW_I3C_RESP_QUEUE_TL_3,                 /**< I3C Response QUEUE threshold level is 3 entries */
        HW_I3C_RESP_QUEUE_TL_4,                 /**< I3C Response QUEUE threshold level is 4 entries */
} HW_I3C_RESP_QUEUE_TL;

/**
 * \brief Define the I3C Command QUEUE empty threshold level
 *
 * \sa hw_i3c_set_cmd_empty_queue_threshold
 */
typedef enum {
        HW_I3C_CMD_EMPTY_QUEUE_TL_0,            /**< I3C Command QUEUE empty threshold level is 0 entries */
        HW_I3C_CMD_EMPTY_QUEUE_TL_1,            /**< I3C Command QUEUE empty threshold level is 1 entry   */
        HW_I3C_CMD_EMPTY_QUEUE_TL_2,            /**< I3C Command QUEUE empty threshold level is 2 entries */
        HW_I3C_CMD_EMPTY_QUEUE_TL_3,            /**< I3C Command QUEUE empty threshold level is 3 entries */
        HW_I3C_CMD_EMPTY_QUEUE_TL_4,            /**< I3C Command QUEUE empty threshold level is 4 entries */
        HW_I3C_CMD_EMPTY_QUEUE_TL_5,            /**< I3C Command QUEUE empty threshold level is 5 entries */
        HW_I3C_CMD_EMPTY_QUEUE_TL_6,            /**< I3C Command QUEUE empty threshold level is 6 entries */
        HW_I3C_CMD_EMPTY_QUEUE_TL_7,            /**< I3C Command QUEUE empty threshold level is 7 entries */
} HW_I3C_CMD_EMPTY_QUEUE_TL;

/**
 * \brief Define the I3C TX/RX start threshold level
 *
 * \note Each entry can hold 4 bytes of data.
 *
 */
typedef enum {
        HW_I3C_FIFO_START_TL_1,                 /**< I3C TX/RX FIFO start threshold level is 1 entry    */
        HW_I3C_FIFO_START_TL_4,                 /**< I3C TX/RX FIFO start threshold level is 4 entries  */
        HW_I3C_FIFO_START_TL_8,                 /**< I3C TX/RX FIFO start threshold level is 8 entries  */
        HW_I3C_FIFO_START_TL_16,                /**< I3C TX/RX FIFO start threshold level is 16 entries */
        HW_I3C_FIFO_START_TL_32,                /**< I3C TX/RX FIFO start threshold level is 32 entries */
} HW_I3C_FIFO_START_TL;

/**
 * \brief Define the I3C TX FIFO empty threshold level that triggers the TX_THLD_STAT interrupt.
 *
 * \note Each entry can hold 4 bytes of data.
 *
 */
typedef enum {
        HW_I3C_TX_FIFO_EMPTY_TL_1,              /**< I3C TX FIFO empty threshold level is 1 entry    */
        HW_I3C_TX_FIFO_EMPTY_TL_4,              /**< I3C TX FIFO empty threshold level is 4 entries  */
        HW_I3C_TX_FIFO_EMPTY_TL_8,              /**< I3C TX FIFO empty threshold level is 8 entries  */
        HW_I3C_TX_FIFO_EMPTY_TL_16,             /**< I3C TX FIFO empty threshold level is 16 entries */
        HW_I3C_TX_FIFO_EMPTY_TL_32,             /**< I3C TX FIFO empty threshold level is 32 entries */
} HW_I3C_TX_FIFO_EMPTY_TL;

/**
 * \brief Define the I3C RX FIFO threshold level that triggers the RX_THLD_STAT interrupt.
 *
 * \note Each entry can hold 4 bytes of data.
 *
 */
typedef enum {
        HW_I3C_RX_FIFO_USED_TL_1,                    /**< I3C RX FIFO threshold level is 1 entry    */
        HW_I3C_RX_FIFO_USED_TL_4,                    /**< I3C RX FIFO threshold level is 4 entries  */
        HW_I3C_RX_FIFO_USED_TL_8,                    /**< I3C RX FIFO threshold level is 8 entries  */
        HW_I3C_RX_FIFO_USED_TL_16,                   /**< I3C RX FIFO threshold level is 16 entries */
        HW_I3C_RX_FIFO_USED_TL_32,                   /**< I3C RX FIFO threshold level is 32 entries */
} HW_I3C_RX_FIFO_USED_TL;

/**
 * \brief I3C interrupt source
 *
 * Can be used as bit-mask.
 *
 */
typedef enum {
        HW_I3C_INT_TX_THLD_STS = I3C_I3C_INTR_STATUS_REG_TX_THLD_STS_Msk,                   /**< Transmit Buffer Threshold Status */
        HW_I3C_INT_RX_THLD_STS = I3C_I3C_INTR_STATUS_REG_RX_THLD_STS_Msk,                   /**< Receive Buffer Threshold Status  */
        HW_I3C_INT_IBI_THLD_STS  = I3C_I3C_INTR_STATUS_REG_IBI_THLD_STS_Msk,                /**< IBI Buffer Threshold Status      */
        HW_I3C_INT_CMD_QUEUE_READY_STS  = I3C_I3C_INTR_STATUS_REG_CMD_QUEUE_READY_STS_Msk,  /**< Command Queue Ready              */
        HW_I3C_INT_RESP_READY_STS = I3C_I3C_INTR_STATUS_REG_RESP_READY_STS_Msk,             /**< Response Queue Ready Status      */
        HW_I3C_INT_TRANSFER_ABORT_STS  = I3C_I3C_INTR_STATUS_REG_TRANSFER_ABORT_STS_Msk,    /**< Transfer Abort Status            */
        HW_I3C_INT_TRANSFER_ERR_STS = I3C_I3C_INTR_STATUS_REG_TRANSFER_ERR_STS_Msk,         /**< Transfer Error Status            */
} HW_I3C_INT;

/**
 * \brief I3C SCL timings structure definition
 *
 * +----------+--------------------------------------------+---------------------------------------------+
 * |          |                  Mixed bus                 |                   Pure bus                  |
 * |          +--------------------------+-----------------+---------------------------+-----------------+
 * |          |   Core clock at 160 MHz  | SCL speed (MHz) |    Core clock at 32 MHz   | SCL speed (MHz) |
 * +----------+--------------------------+-----------------+---------------------------+-----------------+
 * |  I2C FM  |    i2c_fm_hcnt = 0xC8    |       0.4       |     i2c_fm_hcnt = 0x28    |       0.4       |
 * |          |    i2c_fm_lcnt = 0xC8    |                 |     i2c_fm_lcnt = 0x28    |                 |
 * +----------+--------------------------+-----------------+---------------------------+-----------------+
 * |  I2C FM+ |  i2c_fm_plus_hcnt = 0x50 |        1        |  i2c_fm_plus_hcnt = 0x10  |        1        |
 * |          |  i2c_fm_plus_lcnt = 0x50 |                 |  i2c_fm_plus_lcnt = 0x10  |                 |
 * +----------+--------------------------+-----------------+---------------------------+-----------------+
 * | I3C SDR0 |     i3c_pp_hcnt = 0x7    |      12.5       |     i3c_pp_hcnt = 0x5     |       3.2       |
 * |          |     i3c_pp_lcnt = 0x6    |                 |     i3c_pp_lcnt = 0x5     |                 |
 * +----------+--------------------------+-----------------+---------------------------+-----------------+
 * |  I3C OD  |     i3c_od_hcnt = 0x5    |       4.3       |     i3c_od_hcnt = 0x5     |       3.2       |
 * |          |     i3c_od_lcnt = 0x20   |                 |      i3c_od_lcnt = 0x5    |                 |
 * +----------+--------------------------+-----------------+---------------------------+-----------------+
 * | I3C SDR1 | i3c_sdr1_ext_lcnt = 0xD  |        8        |  i3c_sdr1_ext_lcnt = 0x7  |        3        |
 * +----------+--------------------------+-----------------+---------------------------+-----------------+
 * | I3C SDR2 | i3c_sdr2_ext_lcnt = 0x14 |        6        |  i3c_sdr2_ext_lcnt = 0x8  |       2.4       |
 * +----------+--------------------------+-----------------+---------------------------+-----------------+
 * | I3C SDR3 | i3c_sdr3_ext_lcnt = 0x21 |        4        |  i3c_sdr3_ext_lcnt = 0xB  |        2        |
 * +----------+--------------------------+-----------------+---------------------------+-----------------+
 * | I3C SDR4 | i3c_sdr4_ext_lcnt = 0x49 |        2        |  i3c_sdr4_ext_lcnt = 0x12 |       1.4       |
 * +----------+--------------------------+-----------------+---------------------------+-----------------+
 *
 */
typedef struct {
        uint16_t i2c_fm_hcnt;           /**< I2C fast mode SCL high count       */
        uint16_t i2c_fm_lcnt;           /**< I2C fast mode SCL low count        */
        uint16_t i2c_fm_plus_lcnt;      /**< I2C fast mode plus SCL low count   */
        uint8_t  i2c_fm_plus_hcnt;      /**< I2C fast mode plus SCL high count  */
        uint8_t i3c_pp_hcnt;            /**< I3C push-pull SCL high count       */
        uint8_t i3c_pp_lcnt;            /**< I3C push-pull SCL low count        */
        uint8_t i3c_od_hcnt;            /**< I3C open-drain SCL high count      */
        uint8_t i3c_od_lcnt;            /**< I3C open-drain SCL low count       */
        uint8_t i3c_sdr1_ext_lcnt;      /**< I3C SDR1 extended SCL low count    */
        uint8_t i3c_sdr2_ext_lcnt;      /**< I3C SDR2 extended SCL low count    */
        uint8_t i3c_sdr3_ext_lcnt;      /**< I3C SDR3 extended SCL low count    */
        uint8_t i3c_sdr4_ext_lcnt;      /**< I3C SDR4 extended SCL low count    */
} i3c_scl_config;

/**
 * \brief Maximum number of I3C/I2C slave devices
 *
 */
#define HW_I3C_SLAVE_DEV_MAX                (0x8)

/**
 * \brief I3C Device Address Table structure definition
 *
 * \warning The dynamic address must be other than the reserved addresses mentioned in Table-8
 * "I3C Slave Address Restrictions" of the MIPI I3C Specification.
 *
 */
typedef struct {
        HW_I3C_SLAVE_DEVICE     slave_type;                /**< Slave device type               */
        uint8_t                 static_address;            /**< 7-bit slave static/I2C address  */
        uint8_t                 dynamic_address;           /**< 7-bit slave dynamic address     */
} i3c_dat_config;

/**
* \brief I3C In band Interrupts structure definition
*
*/
typedef struct {
       hw_i3c_ibi_sir_hj_callback  ibi_sir_hj_cb;             /**< Callback to call after Slave Interrupt or Hot Join reception   */
} hw_i3c_ibi_sir_hj_config;

#if (HW_I3C_DMA_SUPPORT == 1)
/**
 * \brief I3C DMA priority configuration
 *
 * \note DMA channel priorities are configured to their default values
 * when use_prio = false
 */
typedef hw_dma_periph_prio_t hw_i3c_dma_prio_t;
#endif

/**
 * \brief I3C configuration structure definition
 *
 */
typedef struct {
        bool                      select_divn;                            /**< Select the clock source (DIVN/DIV1 clock)                 */
        i3c_scl_config            i3c_scl_cfg;                            /**< I3C clock (SCL) settings, refer to datasheet for details. */
        i3c_dat_config            i3c_dat_cfg[HW_I3C_SLAVE_DEV_MAX];      /**< I3C/I2C slave devices configuration                       */
        bool                      hot_join_accept;                        /**< Hot-Join Ack/Nack Control                                 */
        bool                      iba;                                    /**< Include Broadcast Address (0x7E) in private transfers     */
        hw_i3c_ibi_sir_hj_config  i3c_ibi_sir_hj_cfg;                     /**< I3C In Band Interrupts configuration                      */
#if (HW_I3C_DMA_SUPPORT == 1)
        bool                      use_dma;                                /**< DMA functionality enable/disable                          */
        HW_I3C_DMA_CHANNEL_PAIR   dma_channel_pair;                       /**< DMA channel pair                                          */
        hw_i3c_dma_prio_t         dma_prio;                               /**< DMA channel priority                                      */
#endif /* HW_I3C_DMA_SUPPORT */
} i3c_config;

/**
 * \brief I3C private transfer speed for I3C and I2C mode
 *
 */
typedef enum {
        HW_I3C_PRIVATE_TRANSFER_SPEED_SDR0_I3C_FAST_MODE_I2C,           /**< SDR0 for I3C mode and 400kb/s for I2C mode         */
        HW_I3C_PRIVATE_TRANSFER_SPEED_SDR1_I3C_FAST_MODE_PLUS_I2C,      /**< SDR1 for I3C mode and 1Mb/s for I2C mode           */
        HW_I3C_PRIVATE_TRANSFER_SPEED_SDR2_I3C,                         /**< SDR2 for I3C mode, valid only in I3C mode          */
        HW_I3C_PRIVATE_TRANSFER_SPEED_SDR3_I3C,                         /**< SDR3 for I3C mode, valid only in I3C mode          */
        HW_I3C_PRIVATE_TRANSFER_SPEED_SDR4_I3C,                         /**< SDR4 for I3C mode, valid only in I3C mode          */
        HW_I3C_PRIVATE_TRANSFER_SPEED_FAST_MODE_I3C = 0x7,              /**< 400kb/s for I3C mode, valid only in I3C mode       */
} HW_I3C_PRIVATE_TRANSFER_SPEED;

/**
 * \brief I3C Transaction ID
 *
 */
typedef enum {
        HW_I3C_TRANSACTION_ID_0,                /**< Transaction ID 0 */
        HW_I3C_TRANSACTION_ID_1,                /**< Transaction ID 1 */
        HW_I3C_TRANSACTION_ID_2,                /**< Transaction ID 2 */
        HW_I3C_TRANSACTION_ID_3,                /**< Transaction ID 3 */
        HW_I3C_TRANSACTION_ID_4,                /**< Transaction ID 4 */
        HW_I3C_TRANSACTION_ID_5,                /**< Transaction ID 5 */
        HW_I3C_TRANSACTION_ID_6,                /**< Transaction ID 6 */
        HW_I3C_TRANSACTION_ID_7,                /**< Transaction ID 7 */
        HW_I3C_TRANSACTION_ID_8,                /**< Transaction ID 8 */
} HW_I3C_TRANSACTION_ID;

/**
 * \brief I3C bus condition after transfer completion
 *
 */
typedef enum {
        HW_I3C_TRANSFER_TOC_RESTART,            /**< I3C controller issues RESTART condition after the transfer completion */
        HW_I3C_TRANSFER_TOC_STOP,               /**< I3C controller issues STOP condition after the transfer completion    */
} HW_I3C_TRANSFER_TOC;

/**
 * \brief I3C private transfer configuration structure definition
 *
 */
typedef struct {
        HW_I3C_PRIVATE_TRANSFER_SPEED           i3c_tranfer_speed;              /**< I3C private transfer speed                                   */
        HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION     slave_dev_idx;                  /**< Slave Device index in the Device Address Table               */
        HW_I3C_TRANSACTION_ID                   i3c_tid;                        /**< I3C transaction ID                                           */
        HW_I3C_TRANSFER_TOC                     termination_on_completion;      /**< Add STOP or RESTART condition after the transfer completion  */
        bool                                    response_on_completion;         /**< Generate response after the execution of the command         */
                                                                                /**< In case of RESTART condition, response is generated when the */
                                                                                /**< next data byte is written.                                   */
        i3c_transfer_cmd_response               cmd_response;                   /**< I3C command response, used only in blocking transfers        */
} i3c_private_transfer_config;

/**
 * \brief I3C Common Command Codes(CCC) commands
 *
 */
typedef enum {
        /* Broadcast CCC commands */
        HW_I3C_CCC_ID_B_RSTDAA = 0x06,     /**< Reset Dynamic Address Assignment   */
        HW_I3C_CCC_ID_B_ENTDAA = 0x07,     /**< Enter Dynamic Address Assignment   */
        HW_I3C_CCC_ID_B_SETMWL = 0x09,     /**< Set Max Write Length               */
        HW_I3C_CCC_ID_B_SETMRL = 0x0A,     /**< Set Max Read Length                */
} HW_I3C_CCC_ID;

/**
 * \brief I3C Common Command Code(CCC) configuration structure definition
 *
 */
typedef struct {
        HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION     slave_dev_idx;                  /**< Slave Device index in the Device Address Table  */
        HW_I3C_TRANSACTION_ID                   i3c_tid;                        /**< I3C transaction ID            */
        HW_I3C_TRANSFER_TOC                     termination_on_completion;      /**< Add STOP or RESTART condition after the transfer completion  */
        bool                                    response_on_completion;         /**< Generate response after the execution of the command         */
                                                                                /**< In case of RESTART condition, response is generated when the */
                                                                                /**< next command is executed.                                    */
        i3c_transfer_cmd_response               cmd_response;                   /**< I3C command response       */
        HW_I3C_CCC_ID                           i3c_ccc_command_id;             /**< I3C CCC command code ID    */
        uint8_t                                 i3c_ccc_data_len;               /**< I3C CCC payload length     */
        uint8_t                                 *i3c_ccc_data;                  /**< I3C CCC payload            */
        uint8_t                                 i3c_dev_count;                  /**< I3C Slave's count          */
} i3c_ccc_transfer_config;

//===================== Read/Write functions ===================================

/**
 * \brief Write a value to an I3C register field
 *
 * \param [in] reg the register to access
 * \param [in] field the register field to write
 * \param [in] val value to be written
 *
 */
#define HW_I3C_REG_SETF(reg, field, val)  REG_SETF(I3C, reg, field, val)

/**
 * \brief Get the value of an I3C register field
 *
 * \param [in] reg the register to access
 * \param [in] field the register field to read
 *
 * \return the value of the register field
 *
 */
#define HW_I3C_REG_GETF(reg, field)  REG_GETF(I3C, reg, field)

/**
 * \brief Set a bit of a I3C register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 */
#define HW_I3C_REG_SET_BIT(reg, field)      REG_SET_BIT(I3C, reg, field)

/**
 * \brief Clear a bit of a I3C register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 */
#define HW_I3C_REG_CLR_BIT(reg, field)      REG_CLR_BIT(I3C, reg, field)

/*
 * Low Level Register Access Functions
 *
 */

/******************* I3C_DEVICE_CTRL_REG Functions **********************/

/**
 * \brief Enable I3C controller
 *
 */
__STATIC_INLINE void hw_i3c_enable_controller(void)
{
        HW_I3C_REG_SET_BIT(I3C_DEVICE_CTRL_REG, ENABLE);
}

/**
 * \brief Disable I3C controller
 *
 */
__STATIC_INLINE void hw_i3c_disable_controller(void)
{
        HW_I3C_REG_CLR_BIT(I3C_DEVICE_CTRL_REG, ENABLE);
}

/**
 *
 * \brief Get I3C Controller enable status
 *
 * \return false: Disabled, true: Enabled
 *
 */
__STATIC_INLINE bool hw_i3c_is_controller_enabled(void)
{
        return (bool) HW_I3C_REG_GETF(I3C_DEVICE_CTRL_REG, ENABLE);
}

/**
 * \brief Resume I3C controller from halt state
 *
 * The I3C controller goes to the halt state due to any type of error
 * in the transfer.
 *
 */
__STATIC_INLINE void hw_i3c_controller_resume(void)
{
        HW_I3C_REG_SET_BIT(I3C_DEVICE_CTRL_REG, RESUME);
}

/**
 * \brief Request the controller to abort any ongoing I3C bus transfer
 *
 * In response to an abort request, the controller issues the STOP condition
 * after the on-going data byte is transmitted or received. The controller
 * then generates an interrupt, sets the INTR_STATUS[TRANSFER_ABORT_STAT] bit
 * and enters the halt state.
 *
 * The controller then waits for the application to issue the resume command
 * by calling the hw_i3c_controller_resume() to exit the halt state. The
 * application is expected to flush/drain all the queues and the FIFOs
 * before resuming the controller.
 *
 * \sa hw_i3c_reset_tx_fifo, hw_i3c_reset_rx_fifo
 *
 */
__STATIC_INLINE void hw_i3c_controller_abort_transfer(void)
{
        HW_I3C_REG_SET_BIT(I3C_DEVICE_CTRL_REG, ABORT);
}


#if (HW_I3C_DMA_SUPPORT == 1)
/**
 * \brief Set DMA_ENABLE in I3C Device Control Register
 *
 * \param [in] i3c_dma_enable       false: Disable, true: Enable
 *
 */
__STATIC_INLINE void hw_i3c_set_dma_enable(bool i3c_dma_enable)
{
        HW_I3C_REG_SETF(I3C_DEVICE_CTRL_REG, DMA_ENABLE_I3C, i3c_dma_enable);
}
#endif

/**
 * \brief Set Hot-Join Ack/Nack Control in I3C Device Control Register
 *
 * \param [in] i3c_hot_join_ctrl        true:  Ack Hot-Join requests
 *                                      false: Nack and auto-disable Hot-Join request
 *
 */
__STATIC_INLINE void hw_i3c_set_hot_join_accept(bool i3c_hot_join_ctrl)
{
        HW_I3C_REG_SETF(I3C_DEVICE_CTRL_REG, HOT_JOIN_CTRL, !i3c_hot_join_ctrl);
}

/**
 * \brief Include I3C Broadcast Address (0x7E) for private transfers.
 *
 * \param [in] iba       true:  Include I3C Broadcast Address
 *                       false: Do not include I3C Broadcast Address
 *
 * \note If the I3C Broadcast Address is not included for private transfers,
 *       In-band Interrupts (IBI) driven from Slaves might not win arbitration,
 *       potentially delaying acceptance of the IBIs.
 */
__STATIC_INLINE void hw_i3c_set_include_bcast_addr(bool iba)
{
        HW_I3C_REG_SETF(I3C_DEVICE_CTRL_REG, IBA_INCLUDE, iba);
}

/**************** I3C_COMMAND_QUEUE_PORT_REG Function ******************/

/**
 *
 * \brief Write I3C COMMAND to I3C_COMMAND_QUEUE_PORT_REG Register
 *
 * \param [in] command
 *
 */
__STATIC_INLINE void hw_i3c_enqueue_command(uint32_t command)
{
        I3C->I3C_COMMAND_QUEUE_PORT_REG = command;
}

/**************** I3C_RESPONSE_QUEUE_PORT_REG Function ******************/

/**
 *
 * \brief Read I3C RESPONSE from I3C_RESPONSE_QUEUE_PORT_REG Register
 *
 * \return Response
 *
 * \details The response status for each Command is written into the Response Queue
 * by the controller if response_on_completion is set or if transfer error occurs.
 * The Response Queue can be read through this register. It is expected that this
 * register is read whenever there is a response entry in RESPONSE QUEUE.
 *
 */
__STATIC_INLINE uint32_t hw_i3c_dequeue_response(void)
{
        return I3C->I3C_RESPONSE_QUEUE_PORT_REG;
}

/****************** I3C_RX_TX_DATA_PORT_REG Functions *******************/

/**
 *
 * \brief Write data to TX FIFO using I3C_RX_TX_DATA_PORT_REG Register
 *
 * \param [in] tx_data Transmitted data
 *
 * \details The transmit data should always be packed as 4-byte aligned data
 * words and written to the Transmit Data Port register. The number of transmitted
 * bytes is controlled from the pushed transfer argument in command queue.
 * If the transfer length is not aligned to 4-bytes, then the additional bytes
 * are ignored from the controller.
 *
 */
__STATIC_INLINE void hw_i3c_write_tx_port(uint32_t tx_data)
{
        I3C->I3C_RX_TX_DATA_PORT_REG = tx_data;
}

/**
 *
 * \brief Read data from the RX FIFO using I3C_RX_TX_DATA_PORT_REG Register
 *
 * \return Received Data
 *
 * \details The Receive data is always packed in 4-byte aligned data words
 * and stored in the RX-Data Buffer. The number of received bytes is controlled
 * from the pushed transfer argument in command queue. If the command length
 * is not aligned to the 4-bytes, then the additional data bytes in the last
 * word have to be ignored.
 */
__STATIC_INLINE uint32_t hw_i3c_read_rx_port(void)
{
        return I3C->I3C_RX_TX_DATA_PORT_REG;
}

/****************** I3C_IBI_QUEUE_STATUS_REG Functions ******************/

/**
 *
 * \brief Read I3C In Band Interrupt from IBI Queue
 *
 * \return Received IBI
 *
 */
__STATIC_INLINE uint32_t hw_i3c_dequeue_ibi(void)
{
        return I3C->I3C_IBI_QUEUE_STATUS_DATA_REG;
}

/*************** I3C_DATA_BUFFER_STS_LEVEL_REG Functions ***************/

/**
 *
 * \brief Get number of I3C TX Buffer empty locations from I3C_DATA_BUFFER_STAT_LEVEL_REG Register
 *
 * \return Number of empty locations in TX Buffer.
 *
 * \note Each location can hold 4 bytes of data.
 */
__STATIC_INLINE uint8_t hw_i3c_get_tx_buffer_empty_locations(void)
{
        return HW_I3C_REG_GETF(I3C_DATA_BUFFER_STAT_LEVEL_REG, TX_BUF_EMPTY_LOC);
}

/**
 *
 * \brief Get number of I3C RX Buffer valid entries from I3C_DATA_BUFFER_STAT_LEVEL_REG Register
 *
 * \return Number of valid entries in RX Buffer.
 *
 * \note Each entry can hold 4 bytes of data.
 *
 */
__STATIC_INLINE uint8_t hw_i3c_get_rx_buffer_level(void)
{
        return HW_I3C_REG_GETF(I3C_DATA_BUFFER_STAT_LEVEL_REG, RX_BUF_BLR);
}

/***************** I3C_QUEUE_STATUS_LEVEL_REG Functions *****************/

/**
 *
 * \brief Get number of I3C RESP Queue valid response entries from I3C_QUEUE_STATUS_LEVEL_REG Register
 *
 * \return Number of valid data entries in the Response Queue.
 *
 */
__STATIC_INLINE uint8_t hw_i3c_get_resp_queue_level(void)
{
        return HW_I3C_REG_GETF(I3C_QUEUE_STATUS_LEVEL_REG, RESP_BUF_BLR);
}

/**
 *
 * \brief Get number of I3C CMD Queue empty entries from I3C_QUEUE_STATUS_LEVEL_REG Register
 *
 * \return Number of empty entries in CMD Queue.
 *
 */
__STATIC_INLINE uint8_t hw_i3c_get_cmd_queue_empty_entries(void)
{
        return HW_I3C_REG_GETF(I3C_QUEUE_STATUS_LEVEL_REG, CMD_QUEUE_EMPTY_LOC);
}

/***************** I3C_QUEUE_THLD_CTRL_REG Functions ********************/

/**
 * \brief Set threshold level in the IBI Status Queue that triggers an IBI_THLD_STS interrupt
 *
 * An interrupt will be generated once the number of entries in the IBI Queue
 * is greater than or equal to \p level.
 *
 * \param [in] level threshold level
 *
 */
__STATIC_INLINE void hw_i3c_set_ibi_status_queue_threshold(HW_I3C_IBI_STATUS_QUEUE_TL level)
{
        HW_I3C_REG_SETF(I3C_QUEUE_THLD_CTRL_REG, IBI_STATUS_THLD, level);
}

/**
 * \brief Set threshold level in the Response Queue that triggers a RESP_READY_STAT interrupt
 *
 * An interrupt will be generated once the number of entries in the Response Queue
 * is greater than or equal to \p level.
 *
 * \param [in] level threshold level
 *
 */
__STATIC_INLINE void hw_i3c_set_resp_queue_threshold(HW_I3C_RESP_QUEUE_TL level)
{
        HW_I3C_REG_SETF(I3C_QUEUE_THLD_CTRL_REG, RESP_BUF_THLD, level);
}

/**
 * \brief Set threshold level of empty entries in the Command Queue that triggers a
 * CMD_QUEUE_READY_STAT interrupt.
 *
 * An interrupt will be generated once the number of empty entries in the Command Queue is
 * greater than or equal to  \p level.
 *
 * A value of 0 sets the threshold to indicate that the queue is completely empty.
 *
 * \param [in] level threshold level
 *
 */
__STATIC_INLINE void hw_i3c_set_cmd_empty_queue_threshold(HW_I3C_CMD_EMPTY_QUEUE_TL level)
{
        HW_I3C_REG_SETF(I3C_QUEUE_THLD_CTRL_REG, CMD_EMPTY_BUF_THLD, level);
}

/*************** I3C_DATA_BUFFER_THLD_CTRL_REG Functions ****************/

/**
 * \brief Set threshold level in the Receive Buffer that initiates a read transfer.
 *
 * When the controller is set up to initiate a read transfer, it waits until either
 * one of the conditions below is met before initiating a read transfer on the I3C Interface:
 *
 *    - "Data length" (as specified in the command) number of entries are empty in the Receive FIFO
 *    - "Threshold" number of entries (or more) are empty in the Receive FIFO
 *
 * \note Each entry can hold 4 bytes of data.
 *
 * \param [in] level threshold level
 *
 */
__STATIC_INLINE void hw_i3c_set_rx_start_threshold(HW_I3C_FIFO_START_TL level)
{
        HW_I3C_REG_SETF(I3C_DATA_BUFFER_THLD_CTRL_REG, RX_START_THLD, level);
}

/**
 * \brief Set threshold level in the Transmit Buffer that initiates a write transfer.
 *
 * When the controller is set up to initiate a write transfer, it waits until either
 * one of the conditions below is met before initiating a write transfer on the I3C Interface:
 *
 *    - "Data length" (as specified in the command) number of entries are filled in the Transmit FIFO
 *    - "Threshold" number of entries (or more) are available in the Transmit FIFO
 *
 * \note Each entry can hold 4 bytes of data.
 *
 * \param [in] level threshold level
 *
 */
__STATIC_INLINE void hw_i3c_set_tx_start_threshold(HW_I3C_FIFO_START_TL level)
{
        HW_I3C_REG_SETF(I3C_DATA_BUFFER_THLD_CTRL_REG, TX_START_THLD, level);
}

/**
 * \brief Get threshold level of the Receive FIFO that triggers a RX_THLD_STAT interrupt
 *
 * An RX_THLD_STAT interrupt will be generated once the number of entries in the Receive Buffer
 * is greater than or equal to \p level.
 *
 * \note If the programmed value is greater than the buffer depth, then threshold will be set to 32.
 * \note Each entry can hold 4 bytes of data.
 *
 * \param [in] level threshold level
 *
 */
__STATIC_INLINE void hw_i3c_set_rx_buffer_threshold(HW_I3C_RX_FIFO_USED_TL level)
{
        HW_I3C_REG_SETF(I3C_DATA_BUFFER_THLD_CTRL_REG, RX_BUF_THLD, level);
}

/**
 * \brief Get threshold level of the Receive FIFO that triggers a RX_THLD_STAT interrupt
 *
 * \return Threshold level of the Receive FIFO.
 */
__STATIC_INLINE HW_I3C_RX_FIFO_USED_TL hw_i3c_get_rx_buffer_threshold(void)
{
       return (HW_I3C_RX_FIFO_USED_TL) HW_I3C_REG_GETF(I3C_DATA_BUFFER_THLD_CTRL_REG, RX_BUF_THLD);
}

/**
 * \brief Set threshold level in the Transmit FIFO that triggers a TX_THLD_STAT interrupt
 *
 * An TX_THLD_STAT interrupt will be generated once the number of empty entries in the Transmit Buffer
 * is greater than or equal to \p level.
 *
 * \note If the programmed value is greater than the buffer depth, then threshold will be set to 32.
 * \note Each entry can hold 4 bytes of data.
 *
 * \param [in] level threshold level
 *
 */
__STATIC_INLINE void hw_i3c_set_tx_empty_buffer_threshold(HW_I3C_TX_FIFO_EMPTY_TL level)
{
        HW_I3C_REG_SETF(I3C_DATA_BUFFER_THLD_CTRL_REG, TX_EMPTY_BUF_THLD, level);
}

/**
 * \brief Get threshold level of the Transmit FIFO that triggers a TX_THLD_STAT interrupt
 *
 * \return Threshold level of of the Transmit FIFO.
 */
__STATIC_INLINE HW_I3C_TX_FIFO_EMPTY_TL hw_i3c_get_tx_empty_buffer_threshold(void)
{
       return (HW_I3C_TX_FIFO_EMPTY_TL) HW_I3C_REG_GETF(I3C_DATA_BUFFER_THLD_CTRL_REG, TX_EMPTY_BUF_THLD);
}

/******************** I3C_IBI_QUEUE_CTRL_REG Functions **********************/

/**
 * \brief Notify Rejected Slave Interrupt Request(SIR) Control
 *
 * \param [in] ntf_on_rejection  true:  Notify SIR rejected enable. Writes IBI Status to the
 *                                      IBI FIFO (hence notifying the application) when a slave
 *                                      interrupt request is NACKed and auto-disabled based on the
 *                                      SIR_REJECT field of I3C_DEV_ADDR_TABLEX_LOC1_REG register.
 *
 *                               false: Notify SIR rejected disable. Suppress passing the IBI Status to the
 *                                      IBI FIFO (hence not notifying the application) when a slave
 *                                      interrupt request is NACKed and auto-disabled based on the
 *                                      the SIR_REJECT field of I3C_DEV_ADDR_TABLEX_LOC1_REG register.
 *
 * \sa hw_i3c_set_slave_interrupt_request_rejection_enable
 *
 */
__STATIC_INLINE void hw_i3c_set_ntf_on_slave_interrupt_request_rejection_enable(bool ntf_on_rejection)
{
        HW_I3C_REG_SETF(I3C_IBI_QUEUE_CTRL_REG, NOTIFY_SIR_REJECTED, ntf_on_rejection);
}

/**
 * \brief Notify Rejected Hot-Join Control
 *
 * \param [in] ntf_on_rejection  true:  Notify Hot-Join rejected enable. Writes IBI Status to the
 *                                      IBI FIFO (hence notifying the application) when a Hot Join
 *                                      request is NACKed and auto-disabled based on the HOT_JOIN_CTRL
 *                                      field of I3C_DEVICE_CTRL_REG register.
 *
 *                               false: Notify Hot-Join rejected disable. Suppress passing the IBI Status to the
 *                                      IBI FIFO (hence not notifying the application) when a Hot Join
 *                                      request is NACKed and auto-disabled based on the HOT_JOIN_CTRL
 *                                      field of I3C_DEVICE_CTRL_REG register.
 *
 * \sa hw_i3c_set_hot_join_accept
 *
 */
__STATIC_INLINE void hw_i3c_set_ntf_on_hot_join_rejection_enable(bool ntf_on_rejection)
{
        HW_I3C_REG_SETF(I3C_IBI_QUEUE_CTRL_REG, NOTIFY_HJ_REJECTED, ntf_on_rejection);
}

/******************** I3C_RESET_CTRL_REG Functions **********************/

/**
 * \brief Exercise IBI Queue reset
 *
 */
__STATIC_INLINE void hw_i3c_reset_ibi_queue(void)
{
        HW_I3C_REG_SET_BIT(I3C_RESET_CTRL_REG, IBI_QUEUE_RST);
        while (HW_I3C_REG_GETF(I3C_RESET_CTRL_REG, IBI_QUEUE_RST) != 0);
}

/**
 * \brief Exercise Receive Buffer reset
 *
 */
__STATIC_INLINE void hw_i3c_reset_rx_fifo(void)
{
        HW_I3C_REG_SET_BIT(I3C_RESET_CTRL_REG, RX_FIFO_RST);
        while (HW_I3C_REG_GETF(I3C_RESET_CTRL_REG, RX_FIFO_RST) != 0);
}

/**
 * \brief Exercise Transmit Buffer reset
 *
 */
__STATIC_INLINE void hw_i3c_reset_tx_fifo(void)
{
        HW_I3C_REG_SET_BIT(I3C_RESET_CTRL_REG, TX_FIFO_RST);
        while (HW_I3C_REG_GETF(I3C_RESET_CTRL_REG, TX_FIFO_RST) != 0);
}

/**
 * \brief Exercise Response Queue reset
 *
 */
__STATIC_INLINE void hw_i3c_reset_resp_queue(void)
{
        HW_I3C_REG_SET_BIT(I3C_RESET_CTRL_REG, RESP_QUEUE_RST);
        while (HW_I3C_REG_GETF(I3C_RESET_CTRL_REG, RESP_QUEUE_RST) != 0);
}

/**
 * \brief Exercise Command Queue reset
 *
 */
__STATIC_INLINE void hw_i3c_reset_cmd_queue(void)
{
        HW_I3C_REG_SET_BIT(I3C_RESET_CTRL_REG, CMD_QUEUE_RST);
        while (HW_I3C_REG_GETF(I3C_RESET_CTRL_REG, CMD_QUEUE_RST) != 0);
}

/**
 * \brief Exercise software reset
 *
 * \note This function resets FIFOs/Queues and all I3C registers except:
 *       I3C_DEV_ADDR_TABLE_LOCx_REG and I3C_DEV_CHAR_TABLEy_LOCx_REG registers.
 *
 *
 */
__STATIC_INLINE void hw_i3c_software_reset(void)
{
        HW_I3C_REG_SET_BIT(I3C_RESET_CTRL_REG, SOFT_RST);
        while (HW_I3C_REG_GETF(I3C_RESET_CTRL_REG, SOFT_RST) != 0);
}

/**
 * \brief Check I3C controller idle state
 *
 * \return
 *              \retval true if Master is in idle state and all FIFOs/Queues are empty
 *              \retval false otherwise
 *
 * \details "MASTER_IDLE" field reflects whether the Master Controller is in idle
 * state or not. This bit is set when all the Queues(Command ,Response ,IBI) and
 * Buffers(Transmit and Receive) are empty along with the Master State machine is
 * in idle state.
 *
 */
__STATIC_INLINE bool hw_i3c_controler_is_idle(void)
{
        return HW_I3C_REG_GETF(I3C_PRESENT_STATE_REG, MASTER_IDLE);
}

/**
 * \brief Get I3C transaction status
 *
 * \return status of ongoing non-blocking transaction
 *
 */
bool hw_i3c_is_occupied(void);

/**
 * \brief Reset transfer callback
 *
 * This function sets transfer callback to NULL.
 *
 */
void hw_i3c_reset_xfer_cb(void);

//==================== Configuration functions =================================

/**
 * \brief Initialize peripheral divider register - select clock source and enable I3C clock.
 *
 * \note This function is called by hw_i3c_init. No need to call it when using the hw_i3c_init interface.
 *
 * \param [in] select_divn
 *              True = Select DIVN clock source
 *              False = Select DIV1 clock source
 *
 */
void hw_i3c_init_clk_reg(bool select_divn);

/**
 * \brief De-initialize peripheral divider register - disable I3C clock.
 *
 * \note The function is called by hw_i3c_deinit. No need to call it when using the hw_i3c_deinit interface.
 *
 */
void hw_i3c_deinit_clk_reg(void);

/**
 * \brief Get the status of the I3C interface clock source
 *
 * \return
 *              \retval false if I3C interface clock source is disabled,
 *              \retval true otherwise
 *
 */
bool hw_i3c_is_clk_enabled(void);

/**
 * \brief Initialize the I3C controller
 *
 * \param [in] cfg pointer to I3C configuration struct
 *
 * \return HW_I3C_ERROR_NONE if no error occurred, else error code.
 *
 */
HW_I3C_ERROR hw_i3c_init(const i3c_config *cfg);

/**
 * \brief Disable the I3C controller
 *
 */
void hw_i3c_deinit(void);

/**
 * \brief Set slave device address and type
 *
 * \param [in] static_address 7-bit slave static address
 * \param [in] dynamic_address 7-bit slave dynamic address
 * \param [in] slave_type I3C or Legacy I2C slave device
 * \param [in] slave_dev_loc slave device location in address table
 *
 * \warning The dynamic address must be other than the reserved addresses mentioned in Table-8
 * "I3C Slave Address Restrictions" of the MIPI I3C Specification.
 */
void hw_i3c_set_slave_device_address(uint8_t static_address, uint8_t dynamic_address, HW_I3C_SLAVE_DEVICE slave_type, HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION slave_dev_loc);

/**
 * \brief Set rejection on Slave Interrupt Request(SIR)
 *
 * \param [in] slave_dev_loc slave device location in address table
 * \param [in] i3c_sir_rejection_ctrl  true:  NACK and send directed auto disable CCC
 *                                     false: ACK the SIR Request
 *
 */
void hw_i3c_set_slave_interrupt_request_rejection_enable(HW_I3C_SLAVE_ADDRESS_TABLE_LOCATION slave_dev_loc, bool i3c_sir_rejection_ctrl);

//===================== Read/Write functions ===================================

/**
 * \brief Set CCC to I3C slave
 *
 * Initiates I3C CCC transfer
 *
 * \param [in,out] i3c_ccc_cfg I3C CCC transfer configuration  and response when response
 *  on completion is required
 *
 * \return HW_I3C_ERROR_NONE if no error occurred, else error code
 *
 * \warning This function does not check for errors during transmission when response_on_completion
 * field of i3c_ccc_cfg is false. It is the caller's responsibility to handle any errors during
 * transmission and to resume the controller from HALT state.The response status of a transfer can
 * be retrieved at the end of the transmission using i3c_transfer_cfg->cmd_response parameter.
 * The following macro definitions could be used to parse response in order to check error status,
 * transaction ID and remaining data length if the transfer terminated early.
 *
 * \sa HW_I3C_RESPONSE_PORT_DATA_LEN, HW_I3C_RESPONSE_PORT_ERR_STATUS, HW_I3C_RESPONSE_PORT_TID
 *
 */
HW_I3C_ERROR hw_i3c_set_ccc(i3c_ccc_transfer_config *i3c_ccc_cfg);

/**
 * \brief Write array of bytes to I3C
 *
 * Initiates I3C transmission, no data is received (Write only mode)
 * If no callback is provided this function waits for the transfer to finish.
 * If a callback is provided, the function sets up the transfer in interrupt mode
 * and ends immediately.
 * If i3c_transfer_cfg->response_on_completion is false, this function ends immediately
 * without waiting for response.
 * In callback mode data pointed by out_buf should not be touched till callback is called.
 *
 * \param [in,out] i3c_transfer_cfg I3C transfer configuration and response when response
 *  on completion is required in blocking mode
 * \param [in] out_buf data to send
 * \param [in] len data length in bytes
 * \param [in] cb callback to call after transfer is finished, if no callback is provided
 *             the transfer will be initiated in blocking mode
 * \param [in] user_data parameter for callback
 *
 * \return HW_I3C_ERROR_NONE if no error occurred, else error code
 *
 * \details In blocking mode the response status of a transfer can be retrieved at the end of the
 * transmission using i3c_transfer_cfg->cmd_response parameter. In callback mode the response status
 * is returned by callback function. The following macro definitions could be used to parse response
 * in order to check error status, transaction ID and remaining data length if the transfer terminated
 * early.
 *
 * \sa HW_I3C_RESPONSE_PORT_DATA_LEN, HW_I3C_RESPONSE_PORT_ERR_STATUS, HW_I3C_RESPONSE_PORT_TID
 *
 * \details The address type which is used for private transfers I2C(static)/I3C(dynamic) is controlled
 * directly from I3C controller based on the type of device (I3C or Legacy I2C slave device) placed
 * in I3C_DEV_ADDR_TABLE_LOCX_REG.
 *
 * \sa hw_i3c_set_slave_device_address
 *
 * \warning In DMA mode the supplied buffer address must be 32bit-aligned.
 *
 */
HW_I3C_ERROR hw_i3c_private_write_buf(i3c_private_transfer_config *i3c_transfer_cfg, const uint8_t *out_buf, uint16_t len, hw_i3c_xfer_callback cb, void *user_data);

/**
 * \brief Read array of bytes through I3C
 *
 * Initiates I3C read transfer.
 * If no callback is provided this function waits for the transfer to finish.
 * If a callback is provided, the function sets up the transfer in interrupt mode
 * and ends immediately.
 * If i3c_transfer_cfg->response_on_completion is false, this function ends immediately
 * without waiting for response.
 * In callback mode data pointed by in_buf should not be touched till callback is called.
 *
 * \param [in,out] i3c_transfer_cfg I3C private transfer configuration and response when response
 *  on completion is required in blocking mode
 * \param [out] in_buf buffer for incoming data
 * \param [in] len size of in_buf in bytes
 * \param [in] cb callback to call after transfer is finished, if no callback is provided
 *             the transfer will be initiated in blocking mode
 * \param [in] user_data parameter for callback
 *
 * \return HW_I3C_ERROR_NONE if no error occurred, else error code
 *
 * \details In blocking mode the response status of a transfer can be retrieved at the end of the
 * transmission using i3c_transfer_cfg->cmd_response parameter. In callback mode the response status
 * is returned by callback function. The following macro definitions could be used to parse response
 * in order to check error status, transaction ID and remaining data length if the transfer terminated
 * early.
 *
 * \sa HW_I3C_RESPONSE_PORT_DATA_LEN, HW_I3C_RESPONSE_PORT_ERR_STATUS, HW_I3C_RESPONSE_PORT_TID
 *
 * \details The address type which is used for private transfers I2C(static)/I3C(dynamic) is controlled
 * directly from I3C controller based on the type of device (I3C or Legacy I2C slave device) placed
 * in I3C_DEV_ADDR_TABLE_LOCX_REG.
 *
 * \sa hw_i3c_set_slave_device_address
 *
 * \warning In DMA mode the supplied buffer address and the transfer length must be 32bit-aligned.
 *
 */
HW_I3C_ERROR hw_i3c_private_read_buf(i3c_private_transfer_config *i3c_transfer_cfg, uint8_t *in_buf, uint16_t len, hw_i3c_xfer_callback cb, void *user_data);

//============== Interrupt handling ============================================

/**
 * \brief Register interrupt handler
 *
 * \param [in] cb callback function
 *
 * \note This function does not need to be called unless you are reimplementing the
 *       nonblocking API's interrupt handler routines to add special functionality.
 *
 */
void hw_i3c_register_interrupt_callback(hw_i3c_interrupt_callback cb);

#endif /* dg_configUSE_HW_I3C */
#endif /* HW_I3C_H_ */

/**
 * \}
 * \}
 */
