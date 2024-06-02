/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup I3C_ADAPTER I3C Adapter
 *
 * \brief Improved Inter Integrated Circuit adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_i3c.h
 *
 * @brief I3C device access API
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_I3C_H_
#define AD_I3C_H_

#if dg_configI3C_ADAPTER

#include "ad.h"
#include "hw_i3c.h"
#include "hw_dma.h"
#include "hw_gpio.h"
#include "osal.h"
#include "resmgmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def CONFIG_I3C_USE_SYNC_TRANSACTIONS
 *
 * \brief Controls whether I3C synchronous transaction API will be used
 *
 * I3C synchronous transaction API maintains state in retention RAM.
 * If the API is not to be used, setting this macro to 0 will save retention RAM.
 *
 */
#ifndef CONFIG_I3C_USE_SYNC_TRANSACTIONS
#define CONFIG_I3C_USE_SYNC_TRANSACTIONS        (1)
#endif

/**
 * \def CONFIG_I3C_USE_ASYNC_TRANSACTIONS
 *
 * \brief Controls whether I3C asynchronous transaction API will be used
 *
 */
#ifndef CONFIG_I3C_USE_ASYNC_TRANSACTIONS
#define CONFIG_I3C_USE_ASYNC_TRANSACTIONS       (1)
#endif

#if (CONFIG_I3C_USE_SYNC_TRANSACTIONS == 0) && (CONFIG_I3C_USE_ASYNC_TRANSACTIONS == 0)
#error "At least one macro CONFIG_I3C_USE_SYNC_TRANSACTIONS or CONFIG_I3C_USE_ASYNC_TRANSACTIONS must be set."
#endif

/*
 * Data types definitions section
 */

/**
 * \brief I3C Id
 */
#define HW_I3C          ((void *)I3C_BASE)
typedef void *HW_I3C_ID;

/**
 * \brief I3C Handle returned by ad_i3c_open()
 */
typedef void *ad_i3c_handle_t;

/**
 * \brief I3C I/O configuration
 *
 * I3C I/O configuration
 */

typedef struct {
        ad_io_conf_t scl;                      /**< configuration of scl signal */
        ad_io_conf_t sda;                      /**< configuration of sda signal */
        HW_GPIO_POWER voltage_level;
} ad_i3c_io_conf_t;

/**
 * \brief I3C driver configuration
 *
 * Configuration of I3C low level driver
 *
 */
typedef struct {
        i3c_config i3c;                       /**< I3C driver configuration     */
} ad_i3c_driver_conf_t;

/**
 * \brief I3C controller configuration
 *
 * Configuration of I3C controller
 *
 */
typedef struct {
        const HW_I3C_ID id;                   /**< Controller instance          */
        const ad_i3c_io_conf_t *io;           /**< I/O configuration            */
        const ad_i3c_driver_conf_t *drv;      /**< Driver configuration         */
} ad_i3c_controller_conf_t;

/**
 * \brief I3C adapter error codes
 *
 */
typedef enum {
        AD_I3C_ERROR_NONE =                      0,     /**< No error                                   */
        AD_I3C_ERROR_HANDLE_INVALID =           -1,     /**< The specified handle is not valid          */
        AD_I3C_ERROR_ID_INVALID =               -2,     /**< The controller configuration is invalid    */
        AD_I3C_ERROR_IO_CFG_INVALID =           -3,     /**< The IO configuration is invalid            */
        AD_I3C_ERROR_DRIVER_CONF_INVALID =      -4,     /**< Driver configuration is invalid            */
        AD_I3C_ERROR_CONTROLLER_BUSY =          -5,     /**< Controller is busy with another operation  */
        AD_I3C_ERROR_INVALID_INPUT_PARAM =      -6,     /**< Invalid input parameter(s)                 */
        AD_I3C_ERROR_CONTROLLER_ABORT_FAIL =    -7,     /**< Controller abort failed                    */
        AD_I3C_ERROR_TRANSFER_TIMEOUT =         -8      /**< Transfer timeout                           */
} AD_I3C_ERROR;

/**
 * \brief Asynchronous callback function
 *
 * \param [in] user_data data passed by user along with callback
 * \param [out] success operation status
 * \param [out] cmd_response I3C command response
 *
 */
typedef void (*ad_i3c_user_cb)(void *user_data, bool success, i3c_transfer_cmd_response *cmd_response);

/**
 * \brief Open I3C controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 * \param [in] conf  controller configuration
 *
 * \return !=0: handle that should be used in subsequent API calls, NULL: error
 *
 * \note The function will block until it acquires all controller resources
 *
 * \warning Controllers configuration \p conf must point to a valid \c ad_i3c_controller_conf_t
 *          struct till ad_i3c_close() is called.
 *
 */
ad_i3c_handle_t ad_i3c_open(const ad_i3c_controller_conf_t *conf);

/**
 * \brief Reconfigure I3C controller
 *
 * This function will apply a new I3C driver configuration.
 *
 * \param [in] handle handle returned from ad_i3c_open()
 * \param [in] conf pointer to driver configuration
 *
 * \return 0: success, <0: error code
 *
 * \sa ad_i3c_open()
 *
 */
AD_I3C_ERROR ad_i3c_reconfig(ad_i3c_handle_t handle, const ad_i3c_driver_conf_t *conf);

/**
 * \brief Close I3C controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_i3c_open())
 * - Releases the controller resources
 *
 * \param [in] handle handle returned from ad_i3c_open()
 * \param [in] force force close even if controller is occupied
 *
 * \return 0: success, <0: error code
 *
 * \sa ad_i3c_open()
 *
 */
AD_I3C_ERROR ad_i3c_close(ad_i3c_handle_t handle, bool force);

/**
* \brief Initialize controller pins to on / off io configuration
*
* This function should be called for setting pins to the correct level before external
* devices are powered up (e.g on system init). It does not need to be called before every
* ad_i3c_open() call.
*
* \param [in] id         controller instance
* \param [in] io_config  controller io configuration
* \param [in] state      on/off io configuration
*
* \return 0: success, <0: error code
*/
AD_I3C_ERROR ad_i3c_io_config(const HW_I3C_ID id, const ad_i3c_io_conf_t *io_config, AD_IO_CONF_STATE state);

#if (CONFIG_I3C_USE_SYNC_TRANSACTIONS == 1)
/**
 * \brief Perform a blocking private write transaction with time out
 *
 * This function performs a synchronous private write only transaction
 *
 * \param [in] handle handle returned by ad_i3c_open()
 * \param [in] wbuf buffer containing the data to be sent to the device
 * \param [in] wlen size of data to be sent to the device
 * \param [in] i3c_transfer_cfg I3C transfer configuration
 * \param [in] timeout time in OS ticks that is expected for write to complete
 *
 * \return 0: success, !0: error code
 *
 * \details In case of transfer error this function returns response status. The following macro
 * definitions could be used to parse response in order to check error status, transaction ID and
 * remaining data length if the transfer terminated early.
 *
 * \sa HW_I3C_RESPONSE_PORT_DATA_LEN, HW_I3C_RESPONSE_PORT_ERR_STATUS, HW_I3C_RESPONSE_PORT_TID
 *
 * \sa ad_i3c_open()
 *
 * \warning In DMA mode the supplied buffer address must be 32bit-aligned.
 *
 * \warning In case of AD_I3C_ERROR_CONTROLLER_ABORT_FAIL the adapter should close(ad_i3c_close()) and reopen(ad_i3c_open).
 *
 */
int ad_i3c_private_write(ad_i3c_handle_t handle, const uint8_t *wbuf, size_t wlen, i3c_private_transfer_config *i3c_transfer_cfg, OS_TICK_TIME timeout);

/**
 * \brief Perform a blocking private read transaction with time out
 *
 * This function performs a synchronous private read only transaction
 *
 * \param [in] handle handle returned by ad_i3c_open()
 * \param [out] rbuf buffer for incoming data
 * \param [in] rlen number of bytes to read
 * \param [in] i3c_transfer_cfg I3C transfer configuration
 * \param [in] timeout time in OS ticks that is expected for read to complete
 *
 * \return 0: success, !0: error code
 *
 * \details In case of transfer error this function returns response status. The following macro
 * definitions could be used to parse response in order to check error status, transaction ID and
 * remaining data length if the transfer terminated early.
 *
 * \sa HW_I3C_RESPONSE_PORT_DATA_LEN, HW_I3C_RESPONSE_PORT_ERR_STATUS, HW_I3C_RESPONSE_PORT_TID
 *
 * \sa ad_i3c_open()
 *
 * \warning In DMA mode the supplied buffer address and the transfer length must be word-aligned.
 *
 * \warning In case of AD_I3C_ERROR_CONTROLLER_ABORT_FAIL the adapter should close(ad_i3c_close()) and reopen(ad_i3c_open).
 *
 */
int ad_i3c_private_read(ad_i3c_handle_t handle, uint8_t *rbuf, size_t rlen, i3c_private_transfer_config *i3c_transfer_cfg, OS_TICK_TIME timeout);
#endif /* CONFIG_I3C_USE_SYNC_TRANSACTIONS */

#if (CONFIG_I3C_USE_ASYNC_TRANSACTIONS == 1)
/**
 * \brief Initiate a non blocking private write transaction
 *
 * This function initiates an asynchronous private write only transaction
 * Caller task should retry until function returns no error
 * Callback will be called when transaction is completed
 *
 * \param [in] handle handle returned by ad_i3c_open()
 * \param [in] wbuf buffer containing the data to be sent to the device (wbuf buffer
 *             should not be touched till callback is called)
 * \param [in] wlen size of data to be sent to the device
 * \param [in] i3c_transfer_cfg I3C transfer configuration
 * \param [in] cb callback to call after transaction is over (from ISR context)
 * \param [in] user_data user data passed to cb callback
 *
 * \return 0 on success, <0: error
 *
 * \details The response status is returned by callback function. The following macro definitions could
 * be used to parse response in order to check error status, transaction ID and remaining data length if
 * the transfer terminated early.
 *
 * \sa HW_I3C_RESPONSE_PORT_DATA_LEN, HW_I3C_RESPONSE_PORT_ERR_STATUS, HW_I3C_RESPONSE_PORT_TID
 *
 * \sa ad_i3c_open()
 *
 * \warning In DMA mode the supplied buffer address and the transfer length must be 32bit-aligned.
 *
 */
AD_I3C_ERROR ad_i3c_private_write_async(ad_i3c_handle_t handle, const uint8_t *wbuf, size_t wlen,
                                i3c_private_transfer_config *i3c_transfer_cfg, ad_i3c_user_cb cb, void *user_data);

/**
 * \brief Initiate a non blocking private read transaction
 *
 * This function initiates an asynchronous private read only transaction
 * Caller task should retry until function returns no error
 * Callback will be called when transaction is completed
 *
 * \param [in] handle handle returned by ad_i3c_open()
 * \param [out] rbuf buffer for incoming data (rbuf buffer should not be touched till
 *              callback is called)
 * \param [in] rlen number of bytes to read
 * \param [in] i3c_transfer_cfg I3C transfer configuration
 * \param [in] cb callback to call after transaction is over (from ISR context)
 * \param [in] user_data user data passed to cb callback
 *
 * \return 0 on success, <0: error
 *
 * \details The response status is returned by callback function. The following macro definitions could
 * be used to parse response in order to check error status, transaction ID and remaining data length if
 * the transfer terminated early.
 *
 * \sa HW_I3C_RESPONSE_PORT_DATA_LEN, HW_I3C_RESPONSE_PORT_ERR_STATUS, HW_I3C_RESPONSE_PORT_TID
 *
 * \sa ad_i3c_open()
 *
 * \warning In DMA mode the supplied buffer address and the transfer length must be 32bit-aligned.
 *
 */
AD_I3C_ERROR ad_i3c_private_read_async(ad_i3c_handle_t handle, uint8_t *rbuf, size_t rlen,
                                i3c_private_transfer_config *i3c_transfer_cfg, ad_i3c_user_cb cb, void *user_data);
#endif /* CONFIG_I3C_USE_ASYNC_TRANSACTIONS */

/**
 * \brief Initialize adapter
 *
 * \ Note: It should ONLY be called by the system.
 *
 */
void ad_i3c_init(void);

#ifdef __cplusplus
}
#endif

#endif /* dg_configI3C_ADAPTER */
#endif /* AD_I3C_H_ */

/**
 * \}
 * \}
 */
