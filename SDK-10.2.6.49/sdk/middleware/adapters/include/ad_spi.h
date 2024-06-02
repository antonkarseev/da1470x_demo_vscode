/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup SPI_ADAPTER SPI Adapter
 *
 * \brief Adapter for SPI controller
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_spi.h
 *
 * @brief SPI Controller access API
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/*
 *
 * Terminology:
 *      Adapter:    A middleware abstraction layer for using a controller. It provides the following services:
 *                  - Arbitration in accessing the controller (between multiple masters, multiple tasks)
 *                  - Configuration of the controller
 *                  - Block sleep while the controller is in use
 *      Controller: Top level view of SPI peripheral, including the complete configuration needed for being fully functional (IOs, DMAs, Driver configuration)
 *      Driver:     The Low Level Driver associated with the controller
 *      SPI:        The controller name (e.g SPI1, SPI2)
 *
 * API example usage:
 *      ad_spi_init():           Called by the system on power up for initializing the adapter. It should NOT be called by the application
 *      ad_spi_io_config():      Called by the application for configuring the controller interface before powering up the external connected devices (e.g sensors)
 *
 *      ad_spi_open();           Called by the application for starting a transaction. The sleep is blocked until ad_spi_close() is called. No other task or master can start a new transaction using the same controller instance
 *      ad_spi_write();          Blocking write - caller task will block until the resource becomes available
 *      ad_spi_read();           Blocking read  - caller task will block until the resource becomes available
 *      ad_spi_reconfig();
 *      ad_spi_write_async();    Non blocking write - caller task should retry until function returns no error. It will then be notified when transaction is completed
 *      ad_spi_read_async();     Non blocking read  - caller task should retry until function returns no error. It will then be notified when transaction is completed
 *      ad_spi_close();          Called by the application for releasing the resource. System is allowed to go to sleep (if no other module blocks sleep)
 *
 *
 *
 * */

#ifndef AD_SPI_H_
#define AD_SPI_H_

#if dg_configSPI_ADAPTER

#include "ad.h"
#include "hw_spi.h"
#include "hw_gpio.h"
#include "osal.h"
#include "resmgmt.h"
#include "sdk_defs.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def CONFIG_SPI_USE_ASYNC_TRANSACTIONS
 *
 * \brief Controls whether SPI asynchronous transaction API will be used
 *
 */
#ifndef CONFIG_SPI_USE_ASYNC_TRANSACTIONS
# define CONFIG_SPI_USE_ASYNC_TRANSACTIONS      (1)
#endif

/**
 * \def CONFIG_SPI_USE_SYNC_TRANSACTIONS
 *
 * \brief Controls whether SPI synchronous transaction API will be used
 *
 * SPI synchronous transaction API maintains state in retention RAM for every SPI bus declared.
 * If the API is not to be used, setting this macro to 0 will save retention RAM.
 */
#ifndef CONFIG_SPI_USE_SYNC_TRANSACTIONS
# define CONFIG_SPI_USE_SYNC_TRANSACTIONS       (1)
#endif

#if (CONFIG_SPI_USE_SYNC_TRANSACTIONS == 0) && (CONFIG_SPI_USE_ASYNC_TRANSACTIONS == 0)
#error "At least one macro CONFIG_SPI_USE_SYNC_TRANSACTIONS or CONFIG_SPI_USE_ASYNC_TRANSACTIONS must be set"
#endif

/*
 * Data types definitions section
 */

/**
 * \brief SPI Handle returned by ad_spi_open()
 */
typedef void *ad_spi_handle_t;

/**
 * \brief SPI I/O configuration
 *
 * SPI I/O configuration
 */
typedef struct {
        ad_io_conf_t spi_do;            /**< configuration of DO signal */
        ad_io_conf_t spi_clk;           /**< configuration of CLK signal */
        ad_io_conf_t spi_di;            /**< configuration of DI signal */
        uint8_t cs_cnt;                 /**< Number of configured CS pins */
        const ad_io_conf_t* spi_cs;     /**< configuration of CS's signals */
        HW_GPIO_POWER voltage_level;    /**< Voltage level of the adapters pins */
}  ad_spi_io_conf_t;


/**
 * \brief SPI driver configuration
 *
 * Configuration of  SPI low level driver(s)
 *
 * \note There may be more than one driver configurations needed (e.g DMA)
 *
 */
typedef struct {
        spi_config spi;         /**< Low level driver configuration */
} ad_spi_driver_conf_t;

/**
 * \brief SPI controller configuration
 *
 * Configuration of SPI controller
 *
 */
typedef struct {
        const HW_SPI_ID                 id;         /**< Controller instance*/
        const ad_spi_io_conf_t          *io;        /**< IO configuration*/
        const ad_spi_driver_conf_t      *drv;       /**< Driver configuration*/
} ad_spi_controller_conf_t;


/**
 * \brief enum with return values of API calls
 */
typedef enum {
        AD_SPI_ERROR_NONE                              =  0,
        AD_SPI_ERROR_HANDLE_INVALID                    = -1,
        AD_SPI_ERROR_ADAPTER_NOT_OPEN                  = -2,
        AD_SPI_ERROR_CONFIG_SPI_ROLE_INVALID           = -3,
        AD_SPI_ERROR_CONFIG_DMA_CHANNEL_INVALID        = -4,
        AD_SPI_ERROR_CONFIG_SPI_CS_INVALID             = -5,
        AD_SPI_ERROR_TRANSF_IN_PROGRESS                = -6,
        AD_SPI_ERROR_NO_SPI_CLK_PIN                    = -7,
        AD_SPI_ERROR_DRIVER_CLOCK_DIV_INVALID          = -8,
        AD_SPI_ERROR_DRIVER_CONF_INVALID               = -9,
        AD_SPI_ERROR_IO_CFG_INVALID                    = -10,
        AD_SPI_ERROR_CONFIG_RX_TX_TL_INVALID           = -11,
} AD_SPI_ERROR;

/*
 * Adapters mandatory function prototypes section
 *
 * This section includes the prototypes of the functions
 * which are mandatory for all adapters
 */



/**
 * \brief Initialize adapter
 *
 * \ Note: It should ONLY be called by the system.
 *
 */
void ad_spi_init(void);


/**
 * \brief Open SPI controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 *
 * \param [in] conf  controller configuration
 *
 * \return >0: handle that should be used in subsequent API calls, NULL: error
 *
 * \note The function will block until it acquires all controller resources
 */
ad_spi_handle_t ad_spi_open(const ad_spi_controller_conf_t *conf);

/**
 * \brief Reconfigure SPI controller
 *
 * This function will apply a new SPI driver configuration.
 *
 * \param [in] handle handle returned from ad_spi_open()
 * \param [in] drv_conf   new driver configuration
 *
 * \return 0: success, <0: error code
 *
 * \sa ad_spi_open()
 *
 */
int ad_spi_reconfig(ad_spi_handle_t handle, const ad_spi_driver_conf_t *drv_conf);

/**
 * \brief Close SPI controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_spi_open())
 * - Releases the controller resources
 *
 * \param [in] handle handle returned from ad_spi_open()
 * \param [in] force force close and abort ongoing transaction
 *
 * \return 0: success, <0: error code
 *
 * \sa ad_spi_open()
 *
 */
int ad_spi_close(ad_spi_handle_t handle, bool force);

/**
* \brief Initialize controller pins to on / off io configuration
*
* This function should be called for setting pins to the correct level before external
* devices are powered up (e.g on system init). It does not need to be called before every
* ad_spi_open() call.
*
* \param [in] id         controller instance
* \param [in] io         controller io configuration
* \param [in] state      on/off io configuration
*
* \return 0: success, <0: error code
*/
int ad_spi_io_config (HW_SPI_ID id, const ad_spi_io_conf_t *io, AD_IO_CONF_STATE state);


/*
 * Adapter specific function prototypes section
 *
 * This section includes the prototypes of the functions
 * which are specific to each adapter.
 * An example function prototype is shown below
 */

typedef void (*ad_spi_user_cb)(void *user_data, uint16_t transferred);

#if (CONFIG_SPI_USE_SYNC_TRANSACTIONS == 1)

/**
 * \brief Perform a blocking write transaction
 *
 * This function performs a synchronous write only transaction
 *
 * \param [in] handle handle returned from ad_spi_open()
 * \param [in] wbuf   buffer containing the data to be sent to the device
 * \param [in] wlen   size of data to be sent to the device
 *
 * \return 0 on success, <0: error
 *
 * \note In slave mode, the first word to be transmitted should be written to
 * the TX buffer and the remaining words are written to the TX FIFO.
 * Moreover, there are cases where the ad_spi_write() is non-blocking.
 * In such cases, the application should resolve this by a higher layer protocol.
 * Otherwise, ad_spi_writeread() can be used where the incoming buffer is set to NULL,
 * i.e. used in write only mode. This is always blocking because the terminating event
 * is triggered when all bytes are received.
 * - TX FIFO width = 8 bits and depth = 4 Bytes
 * - TX Buffer = 32 bits
 *
 * 1. SPI_WORD_LENGTH = 3..7 => word size = 1 Byte:
 *   a. wlen = 1 (DMA is not enabled),
 *   b. wlen = 2 and SPI_TX_TL = 0..4,
 *   c. wlen = 3 and SPI_TX_TL = 1..4,
 *   d. wlen = 4 and SPI_TX_TL = 2..4,
 *   e. wlen = 5 and SPI_TX_TL = 3..4, and
 *   f. wlen > 5 and SPI_TX_TL = 4.
 * 2. SPI_WORD_LENGTH = 8..15 => word size = 2 Bytes:
 *   a. wlen = 2 (DMA is not enabled),
 *   b. wlen = 4 and SPI_TX_TL = 0..4,
 *   c. wlen = 6 and SPI_TX_TL = 2..4,
 *   d. wlen >= 8 and SPI_TX_TL = 4.
 * 3. SPI_WORD_LENGTH = 16..31 => word size = 4 Bytes:
 *   a. wlen = 4 (DMA is not enabled),
 *   b. wlen = 8 and SPI_TX_TL = 0..4, and
 *   c. wlen >= 12 and SPI_TX_TL = 4.
 *
 * The mechanism that fetches data into the TX fifo and from TX fifo to TX buffer is as follows:
 *
 * DMA mode:
 * When data is to be transmitted in Slave mode using DMA, the first word is written to
 * SPI_TXBUFFER_FORCE_REG and the DMA is programmed to send the remaining words.
 * When the number of bytes in the TX fifo is less or equal to the TX threshold level
 * set by the user (SPI_TX_TL), an SPI_STATUS_TX_EMPTY event triggers the DMA to
 * fetch new data into the TX FIFO. When the last byte is added in the TX fifo,
 * the dma has finished, the TX dma callback is called and ad_spi_write() returns.
 * This means that for the TX Path, there is no signal notifying about the actual transmission
 * of the last byte on the bus.
 *
 * Interrupt mode:
 * When the number of bytes in the TX fifo is less or equal to the TX threshold level
 * set by the user (SPI_TX_TL), an SPI_STATUS_TX_EMPTY event triggers the spi interrupt handler,
 * which adds new data in the TX fifo. The user callback is called when the last byte
 * is added in the TX fifo.

 * Case 1.a: In the special case where only one word needs to be sent,
 * DMA cannot not be enabled to send zero words, although DMA may have been requested
 * by the user during driver initialization. Instead, interrupt mode will be enabled
 * for receiving an asynchronous notification and eventually call the user callback
 * as required by the driver API.
 *
 * Case 1.b: In the special case where two words needs to be sent, DMA will be enabled.
 * The first word is written to SPI_TXBUFFER_FORCE_REG and the DMA is programmed to send
 * the remaining word. The first time, the TX fifo is always empty i.e. it always has less or
 * equal bytes to the TX threshold level set by the user (SPI_TX_TL).
 * Therefore, an SPI_STATUS_TX_EMPTY event triggers the DMA to fetch a new word into the TX FIFO.
 * When the last and only one word is added in the TX fifo, the dma has finished and
 * the TX dma callback is called.
 *
 * \note Supplied buffer address and length must be non-zero and SPI-word-aligned
 */
int ad_spi_write(ad_spi_handle_t handle, const uint8_t *wbuf, size_t wlen);

/**
 * \brief Perform a blocking read transaction
 *
 * This function performs a synchronous read only transaction
 *
 * \param [in]  handle handle returned from ad_spi_open()
 * \param [out] rbuf   buffer for incoming data
 * \param [in]  rlen   number of bytes to read
 *
 * \return 0 on success, <0: error
 *
 * \sa ad_spi_open()
 * \sa ad_spi_close()
 *
 * \note Supplied buffer address and length must be SPI-word-aligned (no alignment needed for
 * 9-bit SPI word configurations).
 */
int ad_spi_read(ad_spi_handle_t handle, uint8_t *rbuf, size_t rlen);

/**
 * \brief Perform a blocking write and read transaction
 *
 * This function performs a synchronous write and read transaction
 * If wbuf is NULL then the function is used to read data
 * If rbuf is NULL then the function is used to write data
 * Buffers wbuf and rbuf cannot be both NULL
 *
 * \param [in] handle handle returned from ad_spi_open()
 * \param [in] wbuf   buffer containing the data to be sent to the device
 * \param [in] rbuf   buffer for incoming data
 * \param [in] len    size of data to be sent/read to the device
 *
 * \return 0 on success, <0: error
 *
 * \note Supplied buffer address and length must be SPI-word-aligned
 * (DA1468X and DA1469X: no alignment needed for 9-bit SPI word configurations).
 */
int ad_spi_write_read(ad_spi_handle_t handle, const uint8_t *wbuf, uint8_t *rbuf, size_t len);

#endif /* CONFIG_SPI_USE_SYNC_TRANSACTIONS */


/**
 * \brief Activate chip select for a specific device
 *
 * \param [in] handle The handle to the SPI device
 *
 * \warning The task must own the controller before calling this function
 * \note This function should be called only by SPI master.
 */
void ad_spi_activate_cs(ad_spi_handle_t handle);

/**
 * \brief Deactivate chip select for a specific device
 *
 * \param [in] handle The handle to the SPI device
 *
 * \warning The task must own the bus before calling this function.
 * \note This function should be called only by SPI master.
 */
void ad_spi_deactivate_cs(ad_spi_handle_t handle);

/**
 * \brief Wait for SPI interface to be idle and deactivate chip select
 *
 * \param [in] handle The handle to the SPI device
 *
 * \warning The device and the bus resources must have been acquired before calling this function.
 * \note This function should be called only by SPI master.
 */
void ad_spi_deactivate_cs_when_spi_done(ad_spi_handle_t handle);


#if (CONFIG_SPI_USE_ASYNC_TRANSACTIONS == 1)

/**
 * \brief Perform a non blocking write transaction
 *
 * This function performs an asynchronous write only transaction
 * Caller task should retry until function returns no error
 * Callback will be called when transaction is completed
 *
 * \param [in] handle handle returned from ad_spi_open()
 * \param [in] wbuf   buffer containing the data to be sent to the device
 * \param [in] wlen   size of data to be sent to the device
 * \param [in] cb     callback to call after transaction is over (from ISR context)
 * \param [in] user_data user data passed to cb callback
 *
 * \sa ad_spi_open()
 *
 * \return 0 on success, <0: error
 *
 */
int ad_spi_write_async(ad_spi_handle_t handle, const uint8_t *wbuf, size_t wlen, ad_spi_user_cb cb,
        void *user_data);

/**
 * \brief Perform a blocking read transaction
 *
 * This function performs a synchronous read only transaction
 *
 * \param [in]  handle handle returned from ad_spi_open()
 * \param [out] rbuf   buffer for incoming data
 * \param [in]  rlen   number of bytes to read
 * \param [in]  cb     callback to call after transaction is over (from ISR context)
 * \param [in]  user_data user data passed to cb callback
 *
 * \sa ad_spi_open()
 *
 * \return 0 on success, <0: error
 *
 */

int ad_spi_read_async(ad_spi_handle_t handle, const uint8_t *rbuf, size_t rlen, ad_spi_user_cb cb,
        void *user_data);

/**
 * \brief Perform asynchronous SPI write and then read transaction. It can be used as write command
 * and read buffer operation.
 *
 * This function performs write command and read buffer transaction on SPI bus.
 * The device and the bus resources must have been acquired and the chip select must be
 * activated before calling this function. This can be achieved by calling ad_spi_start().
 * The callback function is executed when transaction has been completed. At this point SPI data
 * are available to be used by the application.
 * In the callback function one of the following actions can be performed:
 * - Read or write additional data by calling ad_spi_async_read() / ad_spi_async_write()
 * - Notify application task that SPI transaction has been completed.  ad_spi_end() must be
 *   called from application task context.
 *
 * \param [in] handle handle returned from ad_spi_open()
 * \param [in] wbuf data to send
 * \param [in] wlen number of bytes to write
 * \param [out] rbuf buffer for incoming data
 * \param [in] rlen number of bytes to read
 * \param [in] cb callback to call after transaction is over (from ISR context)
 * \param [in] user_data user data to pass to \p cb
 *
 * \sa ad_spi_open()
 *
 * \return 0 on success, <0: error
 *
 */

int ad_spi_write_read_async(ad_spi_handle_t handle, const uint8_t *wbuf, size_t wlen,
                         uint8_t *rbuf, size_t rlen, ad_spi_user_cb cb, void *user_data);

#endif /* CONFIG_SPI_USE_ASYNC_TRANSACTIONS */

#ifdef __cplusplus
}
#endif

#endif /* dg_configSPI_ADAPTER */

#endif /* AD_SPI_H_ */

/**
 * \}
 * \}
 */
