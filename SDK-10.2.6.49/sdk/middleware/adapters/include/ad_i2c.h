/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup I2C_ADAPTER I2C Adapter
 *
 * \brief Inter Integrated Circuit adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_i2c.h
 *
 * @brief I2C device access API
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_I2C_H_
#define AD_I2C_H_

#if dg_configI2C_ADAPTER

#include "ad.h"
#include "hw_i2c.h"
#if (HW_I2C_DMA_SUPPORT == 1)
#include "hw_dma.h"
#endif /* HW_I2C_DMA_SUPPORT */
#include "hw_gpio.h"
#include "osal.h"
#include "resmgmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def CONFIG_I2C_USE_ASYNC_TRANSACTIONS
 *
 * \brief Controls whether I2C asynchronous transaction API will be used
 *
 * I2C asynchronous transaction API (see "ad_i2c_xx_async" API) maintains state in
 * RAM for every I2C bus declared. If the API is not to be used, setting this macro
 * to 0 will save RAM.
 */
#ifndef CONFIG_I2C_USE_ASYNC_TRANSACTIONS
#define CONFIG_I2C_USE_ASYNC_TRANSACTIONS       (1)
#endif

/**
 * \def CONFIG_I2C_USE_SYNC_TRANSACTIONS
 *
 * \brief Controls whether I2C synchronous transaction API will be used
 *
 * I2C synchronous transaction API (see "ad_i2c_write_read", "ad_i2c_write", "ad_i2c_read" functions)
 * maintains state in retention RAM for every I2C bus declared. If the API is not to be used,
 * setting this macro to 0 will save retention RAM.
 */
#ifndef CONFIG_I2C_USE_SYNC_TRANSACTIONS
#define CONFIG_I2C_USE_SYNC_TRANSACTIONS        (1)
#endif

#if (CONFIG_I2C_USE_SYNC_TRANSACTIONS == 0) && (CONFIG_I2C_USE_ASYNC_TRANSACTIONS == 0)
#error "At least one macro CONFIG_I2C_USE_SYNC_TRANSACTIONS or CONFIG_I2C_USE_ASYNC_TRANSACTIONS must be set."
#endif

#if (HW_I2C_SLAVE_SUPPORT == 1)
#if (CONFIG_I2C_USE_SYNC_TRANSACTIONS == 0)
        #error "CONFIG_I2C_USE_SYNC_TRANSACTIONS must be set if HW_I2C_SLAVE_SUPPORT is set."
#endif
#endif

#ifndef I2C_DEFAULT_CLK_CFG
        #define I2C_DEFAULT_CLK_CFG .i2c.clock_cfg = { 0, 0, 0, 0, 0, 0 }
#endif

/*
 * Data types definitions section
 */

/**
 * \brief I2C Handle returned by ad_i2c_open()
 */
typedef void *ad_i2c_handle_t;

/**
 * \brief I2C I/O configuration
 *
 * I2C I/O configuration
 */

typedef struct {
        ad_io_conf_t scl; /**< configuration of scl signal */
        ad_io_conf_t sda; /**< configuration of sda signal */
        HW_GPIO_POWER voltage_level;
} ad_i2c_io_conf_t;

/**
 * \brief I2C driver configuration
 *
 * Configuration of I2C low level driver(s)
 *
 * \note There may be more than one driver configurations needed (e.g DMA)
 *
 */
typedef struct {
        i2c_config i2c;                 /**< I2C driver configuration */
#if (HW_I2C_DMA_SUPPORT == 1)
        HW_DMA_CHANNEL dma_channel;     /**< DMA channel */
#endif /* HW_I2C_DMA_SUPPORT */
} ad_i2c_driver_conf_t;

/**
 * \brief I2C controller configuration
 *
 * Configuration of I2C controller
 *
 */
typedef struct {
        const HW_I2C_ID id;                   /**< Controller instance */
        const ad_i2c_io_conf_t *io;           /**< I/O configuration */
        const ad_i2c_driver_conf_t *drv;      /**< Driver configuration */
} ad_i2c_controller_conf_t;

/**
 * \brief I2C adapter error codes
 *
 */
typedef enum {
        AD_I2C_ERROR_IO_CFG_INVALID      = -6,
        AD_I2C_ERROR_CONTROLLER_ABORT_FAIL = -5,
        AD_I2C_ERROR_TRANSFER_TIMEOUT = -4,
        AD_I2C_ERROR_CONTROLLER_BUSY = -3,
        AD_I2C_ERROR_DRIVER_CONF_INVALID = -2,
        AD_I2C_ERROR_HANDLE_INVALID      = -1,
        AD_I2C_ERROR_NONE                =  0,
} AD_I2C_ERROR;

/**
 * \brief Asynchronous callback function
 *
 */
typedef void (*ad_i2c_user_cb)(void *user_data, HW_I2C_ABORT_SOURCE error);

/**
 * \brief Initialize adapter
 *
 */
void ad_i2c_init(void);

/**
 * \brief Open I2C controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 *
 * \param [in] conf  controller configuration
 *
 * \return >0: non-NULL handle that should be used in subsequent API calls, NULL: error
 *
 * \note The function will block until it acquires all controller resources
 */
ad_i2c_handle_t ad_i2c_open(const ad_i2c_controller_conf_t *conf);

/**
 * \brief Reconfigure I2C controller
 *
 * This function will apply a new I2C driver configuration.
 *
 * \param [in] p handle returned from ad_i2c_open()
 * \param [in] conf pointer to driver configuration
 *
 * \return 0: success, <0: error code
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_reconfig(ad_i2c_handle_t p, const ad_i2c_driver_conf_t *conf);

/**
 * \brief Close I2C controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_i2c_open())
 * - Releases the controller resources
 *
 * \param [in] p handle returned from ad_i2c_open()
 * \param [in] force force close even if controller is occupied
 *
 * \return 0: success, <0: error code
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_close(ad_i2c_handle_t p, bool force);

/**
* \brief Initialize controller pins to on / off io configuration
*
* This function should be called for setting pins to the correct level before external
* devices are powered up (e.g on system init). It does not need to be called before every
* ad_i2c_open() call.
*
* \param [in] id         controller instance
* \param [in] io         controller io configuration
* \param [in] state      on/off io configuration
*
* \return 0: success, <0: error code
*/
int ad_i2c_io_config(HW_I2C_ID id, const ad_i2c_io_conf_t *io, AD_IO_CONF_STATE state);

#if CONFIG_I2C_USE_SYNC_TRANSACTIONS
/**
 * \brief Perform a blocking write transaction
 *
 * This function performs a synchronous write only transaction
 *
 * \param [in] p handle returned from ad_i2c_open()
 * \param [in] wbuf buffer containing the data to be sent to the device
 * \param [in] wlen size of data to be sent to the device
 * \param [in] condition_flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      No STOP or RESTART conditions will be generated at the end of the transaction.
 *      DA14680/1 will automatically generate a STOP condition when the last byte in the transmit
 *      buffer has been transmitted.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return 0 on success, value from HW_I2C_ABORT_SOURCE enum on failure
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_write(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t condition_flags);

/**
 * \brief Perform a blocking write transaction with time out
 *
 * This function performs a synchronous write only transaction
 *
 * \param [in] p handle returned from ad_i2c_open()
 * \param [in] wbuf buffer containing the data to be sent to the device
 * \param [in] wlen size of data to be sent to the device
 * \param [in] condition_flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      No STOP or RESTART conditions will be generated at the end of the transaction.
 *      DA14680/1 will automatically generate a STOP condition when the last byte in the transmit
 *      buffer has been transmitted.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 * \param [in] timeout time in OS ticks that is expected for write to complete. If time out occurs
 * AD_I2C_ERROR_TRANSFER_TIMEOUT error is returned.
 * \parblock
 *     :: OS_EVENT_FOREVER<br>
 *     function will block for ever expecting the completion of transfer. It behaves as ad_i2c_write() function.
 *
 *     :: 0<br>
 *     will poll once and check if the transaction is completed.
 * \endparblock
 * \return 0 on success, value from HW_I2C_ABORT_SOURCE enum on failure
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_write_with_to(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t condition_flags, OS_TICK_TIME timeout);

/**
 * \brief Perform a blocking read transaction
 *
 * This function performs a synchronous read only transaction
 *
 * \param [in] p handle returned from ad_i2c_open()
 * \param [out] rbuf buffer for incoming data
 * \param [in] rlen number of bytes to read
 * \param [in] condition_flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      No STOP or RESTART conditions will be generated at the end of the transaction.
 *      DA14680/1 will automatically generate a STOP condition when the last byte in the transmit
 *      buffer has been transmitted.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return 0 on success, value from HW_I2C_ABORT_SOURCE enum on failure
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_read(ad_i2c_handle_t p, uint8_t *rbuf, size_t rlen, uint8_t condition_flags);

/**
 * \brief Perform a blocking read transaction with time out
 *
 * This function performs a synchronous read only transaction
 *
 * \param [in] p handle returned from ad_i2c_open()
 * \param [out] rbuf buffer for incoming data
 * \param [in] rlen number of bytes to read
 * \param [in] condition_flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      No STOP or RESTART conditions will be generated at the end of the transaction.
 *      DA14680/1 will automatically generate a STOP condition when the last byte in the transmit
 *      buffer has been transmitted.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 * \param [in] timeout time in OS ticks that is expected for read to complete. If time out occurs
 * AD_I2C_ERROR_TRANSFER_TIMEOUT error is returned.
 * \parblock
 *     :: OS_EVENT_FOREVER<br>
 *     function will block for ever expecting the completion of transfer. It behaves as ad_i2c_read() function.
 *
 *     :: 0<br>
 *     will poll once and check if the transaction is completed.
 * \endparblock
 *
 * \return 0 on success, value from HW_I2C_ABORT_SOURCE enum on failure
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_read_with_to(ad_i2c_handle_t p, uint8_t *rbuf, size_t rlen, uint8_t condition_flags, OS_TICK_TIME timeout);

/**
 * \brief Perform synchronous write/read transaction
 *
 * This function performs a synchronous write and read transaction on I2C bus.
 *
 * \param [in] p handle returned from ad_i2c_open()
 * \param [in] wbuf buffer containing the data to be sent to the device
 * \param [in] wlen size of data to be sent to the device
 * \param [out] rbuf buffer to store the data read from the device
 * \param [in] rlen size of buffer pointed by rbuf
 * \param [in] condition_flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      No STOP or RESTART conditions will be generated at the end of the transaction.
 *      DA14680/1 will automatically generate a STOP condition when the last byte in the transmit
 *      buffer has been transmitted.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return 0 on success, value from HW_I2C_ABORT_SOURCE enum on failure
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_write_read(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf,
                      size_t rlen, uint8_t condition_flags);

/**
 * \brief Perform synchronous write/read transaction with time out
 *
 * This function performs a synchronous write and read transaction on I2C bus.
 *
 * \param [in] p handle returned from ad_i2c_open()
 * \param [in] wbuf buffer containing the data to be sent to the device
 * \param [in] wlen size of data to be sent to the device
 * \param [out] rbuf buffer to store the data read from the device
 * \param [in] rlen size of buffer pointed by rbuf
 * \param [in] condition_flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      No STOP or RESTART conditions will be generated at the end of the transaction.
 *      DA14680/1 will automatically generate a STOP condition when the last byte in the transmit
 *      buffer has been transmitted.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 * \param [in] timeout time in OS ticks that is expected for transaction to complete. If time out occurs
 * AD_I2C_ERROR_TRANSFER_TIMEOUT error is returned.
 * \parblock
 *     :: OS_EVENT_FOREVER<br>
 *     function will block for ever expecting the completion of transfer. It behaves as ad_i2c_write_read() function.
 *
 *     :: 0<br>
 *     will poll once and check if the transaction is completed.
 * \endparblock
 *
 * \return 0 on success, value from HW_I2C_ABORT_SOURCE enum on failure
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_write_read_with_to(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf,
                      size_t rlen, uint8_t condition_flags, OS_TICK_TIME timeout);
#endif /* CONFIG_I2C_USE_SYNC_TRANSACTIONS */

/**
 * \brief Wait while I2C master device is busy
 *
 * \param [in] p pointer returned from ad_i2c_open()
 *
 * \sa ad_i2c_open()
 *
 */
void ad_i2c_wait_while_master_busy(ad_i2c_handle_t p);

#if CONFIG_I2C_USE_ASYNC_TRANSACTIONS
/*
 * The following macros are used to construct asynchronous transactions on I2C device.
 */

/**
 * \brief Perform a non blocking write transaction
 *
 * This function performs an asynchronous write only transaction
 * Caller task should retry until function returns no error
 * Callback will be called when transaction is completed
 *
 * \param [in] p      handle returned from ad_i2c_open()
 * \param [in] wbuf   buffer containing the data to be sent to the device
 * \param [in] wlen   size of data to be sent to the device
 * \param [in] cb     callback to call after transaction is over (from ISR context)
 * \param [in] user_data user data passed to cb callback
 * \param [in] condition_flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      No STOP or RESTART conditions will be generated at the end of the transaction.
 *      DA14680/1 will automatically generate a STOP condition when the last byte in the transmit
 *      buffer has been transmitted.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return 0 on success, <0: error
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_write_async(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen,
                        ad_i2c_user_cb cb, void *user_data, uint8_t condition_flags);

/**
 * \brief Perform a non blocking read transaction
 *
 * This function performs an asynchronous read only transaction
 * Caller task should retry until function returns no error
 * Callback will be called when transaction is completed
 *
 * \param [in]  p         handle returned from ad_i2c_open()
 * \param [out] rbuf      buffer for incoming data
 * \param [in]  rlen      number of bytes to read
 * \param [in]  cb        callback to call after transaction is over (from ISR context)
 * \param [in]  user_data user data passed to cb callback
 * \param [in] condition_flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      No STOP or RESTART conditions will be generated at the end of the transaction.
 *      DA14680/1 will automatically generate a STOP condition when the last byte in the transmit
 *      buffer has been transmitted.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return 0 on success, <0: error
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_read_async(ad_i2c_handle_t p, uint8_t *rbuf, size_t rlen, ad_i2c_user_cb cb,
                       void *user_data, uint8_t condition_flags);

/**
 * \brief Perform write and asynchronous read I2C transaction
 *
 * This function performs asynchronous write and read transaction.
 *
 * \param [in] p handle returned from ad_i2c_open()
 * \param [in] wbuf data to send
 * \param [in] wlen number of bytes to write
 * \param [out] rbuf buffer to store the data read from the device
 * \param [in] rlen size of buffer pointed by rbuf
 * \param [in] cb callback to call after transaction is over (from ISR context)
 * \param [in] user_data user data to pass to \p cb
 * \param [in] condition_flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      No STOP or RESTART conditions will be generated at the end of the transaction.
 *      DA14680/1 will automatically generate a STOP condition when the last byte in the transmit
 *      buffer has been transmitted.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return 0 on success, <0: error
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_write_read_async(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf,
                            size_t rlen, ad_i2c_user_cb cb, void *user_data,
                            uint8_t condition_flags);

#endif /* CONFIG_I2C_USE_ASYNC_TRANSACTIONS */

#if (HW_I2C_SLAVE_SUPPORT == 1)

typedef void (* ad_i2c_slave_event)(ad_i2c_handle_t p, void *user_data);
typedef void (* ad_i2c_slave_data_event)(ad_i2c_handle_t p, uint16_t len, bool success,
                                         void *user_data);

/**
 * \brief Slave event callbacks.
 *
 * A callback field can be NULL (i.e. no callback configured). All callbacks are executed in the
 * corresponding I2C ISR context.
 */
typedef struct {
        ad_i2c_slave_data_event data_sent;      /**< called after data from output_buffer is sent */
        ad_i2c_slave_data_event data_received;  /**< called after data input_buffer is filled */

        ad_i2c_slave_event data_ready;          /**< called when data arrived but there is no input_buffer */
        ad_i2c_slave_event read_request;        /**< called when master wants to read but there is no output_buffer */
} i2c_dev_slave_event_callbacks_t;

/**
 * \brief Slave state bits.
 */
typedef enum {
        /** Slave stopped or uninitialized.*/
        AD_I2C_SLAVE_STATE_STOPPED = 0,

        /** Initial state. */
        AD_I2C_SLAVE_STATE_INIT = 0x1,

        /** Slave read pending. */
        AD_I2C_SLAVE_STATE_READ_PENDING = 0x2,

        /** Slave write pending. */
        AD_I2C_SLAVE_STATE_WRITE_PENDING = 0x4,
} AD_I2C_SLAVE_STATE;

typedef struct {
        const i2c_dev_slave_event_callbacks_t *event_callbacks;
        void *user_data;                        /**< User data to pass to all callbacks */
        const uint8_t *output_buffer;           /**< Data to sent when master wants to read */
        uint16_t output_buffer_len;
        uint8_t *input_buffer;                  /**< Buffer for data in case master writes */
        uint16_t input_buffer_len;

        /** State to support read/write or write/read operations with ad_i2c_start_slave(). */
        AD_I2C_SLAVE_STATE state;
        /** Event used for notification when slave's read or write is completed */
        OS_EVENT operation_done_event;
} i2c_slave_state_data_t;

/**
 * \brief Start slave transmission/reception
 *
 * When I2C is configured in slave mode, this function sets up input and/or output buffers
 * to use for master initiated transmission and/or reception. It also specifies user callbacks that
 * will be called when transmission or reception starts or finishes.
 *
 * If the user specifies valid (wbuf, wlen) pair, data will be sent on incoming read request from
 * master. After reception, data_sent() callback will be called.
 * If wbuf is NULL or wlen is 0, read_request() callback will be called to notify the user about
 * master read request.
 *
 * If (rbuf, rlen) pair is specified, data will be received when master starts writing. When data is
 * received, callback data_received() will be called.
 * If rbuf is NULL or rlen is 0, data_ready() callback will be called to notify user about master
 * write. If the user fails to read from the Rx FIFO before it becomes full then data loss may
 * occur.
 *
 *
 *
 * \param [in] p handle returned from ad_i2c_open()
 * \param [in] wbuf buffer for outgoing data
 * \param [in] wlen number of bytes to write in case master wants to read
 * \param [out] rbuf buffer for incoming data
 * \param [in] rlen number of bytes to read
 * \param [in] events structure with callback to call after transaction is over (from ISR context), if
 * NULL, no callbacks will be called.
 * \param [in] user_data user data to pass to each callback in events
 *
 * \return 0 on success, <0: error
 *
 * Example usage:
 * \code{.c}
 * {
 *   ad_i2c_handle_t handle = ad_i2c_open(conf);
 *   while (1) {
 *     uint8_t cmd[4];
 *     uint8_t response[6];
 *
 *     ad_i2c_start_slave(dev, NULL, 0, cmd, sizeof(cmd), slave_callbacks, NULL);
 *
 *     // wait while callbacks handle write request
 *     ...
 *     // prepare response in out buffer
 *     ad_i2c_start_slave(dev, response, sizeof(response), NULL, 0, slave_callbacks, NULL);
 *     ...
 *     // wait for master to read response
 *     ...
 *     ad_i2c_stop_slave(dev);
 *   }
 * }
 * \endcode
 *
 * \note All callbacks are called from within I2C ISR
 *
 * \sa ad_i2c_open()
 *
 */
int ad_i2c_start_slave(ad_i2c_handle_t p, const uint8_t *wbuf, uint16_t wlen, uint8_t *rbuf,
                       uint16_t rlen, const i2c_dev_slave_event_callbacks_t *events,
                       void *user_data);

/**
 * \brief Stop slave response
 *
 * This function will make a slave device stop responding to external master requests for read or
 * write operations. If such an operation is ongoing, the function will wait for its completion
 * before returning. The bus and device are released by this function.
 *
 * \param [in] p handle returned from ad_i2c_open()
 *
 * \return 0 on success, <0: error
 *
 * \warning This function should only be called only if ad_i2c_start_slave() has been already used
 *          for starting the slave operation.
 *
 * \sa ad_i2c_open()
 * \sa ad_i2c_start_slave()
 *
 */
int ad_i2c_stop_slave(ad_i2c_handle_t p);

/**
 * \brief Clear I2C slave read buffer
 *
 * This function will flush the I2C receive buffer
 *
 * \param [in] p handle returned from ad_i2c_open()
 *
 * \sa ad_i2c_open()
 *
 */
void ad_i2c_clear_read_slave(ad_i2c_handle_t p);

#endif

#ifdef __cplusplus
}
#endif

#endif /* dg_configI2C_ADAPTER */

#endif /* AD_I2C_H_ */

/**
 * \}
 * \}
 */
