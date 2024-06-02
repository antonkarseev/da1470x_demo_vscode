/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup GPADC_ADAPTER GPADC Adapter
 *
 * \brief General Purpose Analog-Digital Converter adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_gpadc.h
 *
 * @brief GPADC adapter API
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configGPADC_ADAPTER == 1)

#ifndef AD_GPADC_H_
#define AD_GPADC_H_

#include "ad.h"
#include "hw_gpadc.h"
#include "hw_gpio.h"
#include "osal.h"
#include "resmgmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def CONFIG_GPADC_USE_ASYNC_TRANSACTIONS
 *
 * \brief Controls whether GPADC asynchronous transaction API will be used
 *
 * If the API is not to be used, setting this macro to 0 will save retention RAM,
 * namely the callback pointer and its argument.
 */
#ifndef CONFIG_GPADC_USE_ASYNC_TRANSACTIONS
#define CONFIG_GPADC_USE_ASYNC_TRANSACTIONS     (1)
#endif

/**
 * \def CONFIG_GPADC_USE_SYNC_TRANSACTIONS
 *
 * \brief Controls whether GPADC synchronous transaction API will be used
 */
#ifndef CONFIG_GPADC_USE_SYNC_TRANSACTIONS
#define CONFIG_GPADC_USE_SYNC_TRANSACTIONS      (1)
#endif

/**
 * \brief GPADC I/O configuration
 *
 * GPADC I/O configuration
 *
 * \note Setting voltage_level to HW_GPIO_POWER_NONE is not allowed
 */

typedef struct ad_gpadc_io_conf {
        ad_io_conf_t input0;                    /**< I/O pin for POSITIVE Input */
        ad_io_conf_t input1;                    /**< I/O pin for NEGATIVE Input */
        HW_GPIO_POWER voltage_level;            /**< Setting to HW_GPIO_POWER_NONE is not allowed */
} ad_gpadc_io_conf_t;

/**
 * \brief GPADC driver configuration
 *
 * Configuration of GPADC low level driver
 *
 */
typedef gpadc_config ad_gpadc_driver_conf_t;

/**
 * \brief GPADC controller instance
 */
#define HW_GPADC_1                      ((void *)GPADC_BASE)

/**
 * \brief GPADC controller configuration
 *
 * Configuration of GPADC controller
 *
 */
typedef struct ad_gpadc_controller_conf {
        const HW_GPADC_ID               id;        /**< Controller instance*/
        const ad_gpadc_io_conf_t       *io;        /**< IO configuration*/
        const ad_gpadc_driver_conf_t   *drv;       /**< Driver configuration*/
} ad_gpadc_controller_conf_t;

/**
 * \brief GPADC Handle returned by ad_gpadc_open()
 */
typedef void *ad_gpadc_handle_t;

/**
 * \brief enum with return values of API calls
 */
typedef enum {
        AD_GPADC_ERROR_NONE                    =  0,
        AD_GPADC_ERROR_HANDLE_INVALID          = -1,
        AD_GPADC_ERROR_CHANGE_NOT_ALLOWED      = -2,
        AD_GPADC_ERROR_ADAPTER_NOT_OPEN        = -3,
        AD_GPADC_ERROR_CONFIG_INVALID          = -4,
        AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS  = -5,
        AD_GPADC_ERROR_TIMEOUT                 = -6,
        AD_GPADC_ERROR_OTHER                   = -7,
        AD_GPADC_ERROR_IO_CFG_INVALID          = -8,
} AD_GPADC_ERROR;

/**
 * \brief Initialize GPADC adapter and some required variables
 *
 * \warning     Do not call this function directly. It is called
 *              automatically during power manager initialization.
 *
 */
void ad_gpadc_init(void);

/**
 * \brief GPADC adapter callback function
 *
 * \param [in] user_data   pointer to user data
 * \param [in] value       number of remaining conversions
 */
typedef void (*ad_gpadc_user_cb)(void *user_data, int value);


#if (CONFIG_GPADC_USE_SYNC_TRANSACTIONS == 1)
/**
 * \brief Read value of the measurement from the selected source
 *
 * This function reads measurement value synchronously.
 *
 * \param [in] handle   handle to GPADC source
 * \param [out] value   pointer to data to be read from selected source. Output isn't a raw value, it has been processed with apply correction function
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 * \return 0 on success, negative value on error
 *
 */
DEPRECATED_MSG("API no longer supported, use ad_gpadc_read_nof_conv() instead.")
int ad_gpadc_read(const ad_gpadc_handle_t handle, uint16_t *value);


/**
 * \brief Read the raw value of the measurement from the selected source
 *
 * This function reads measurement value synchronously.
 *
 * \param [in] handle   handle to GPADC source
 * \param [out] value   pointer to data to be read from selected source. Output is a raw value
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 * \return 0 on success, negative value on error
 *
 */
DEPRECATED_MSG("API no longer supported, use ad_gpadc_read_nof_conv() instead.")
int ad_gpadc_read_raw(const ad_gpadc_handle_t handle, uint16_t *value);

/**
 * \brief Attempt to read the measurement from the selected source within a timeout period.
 *
 * This function attempts to read measurement value synchronously
 *
 * \param [in]  handle handle to GPADC source
 * \param [out] value pointer to data to be read from selected source. Output isn't a raw value, it has been processed with apply correction function
 * \param [in]  timeout number of ticks to wait
 *              0 - no wait take GPADC if it is available
 *              RES_WAIT_FOREVER - wait until GPADC becomes available
 *              Other value specifies how many ticks to wait until GPADC becomes available
 *
 * \return 0 on success, negative value on error
 *
 */
DEPRECATED_MSG("API no longer supported, use ad_gpadc_read_nof_conv() instead.")
int ad_gpadc_read_to(const ad_gpadc_handle_t handle, uint16_t *value, uint32_t timeout);

/**
 * \brief Attempt to read the raw measurement from the selected source within a timeout period.
 *
 * This function attempts to read measurement value synchronously
 *
 * \param [in]  handle handle to GPADC source
 * \param [out] value pointer to data to be read from selected source. Output is a raw value
 * \param [in]  timeout number of ticks to wait
 *              0 - no wait take GPADC if it is available
 *              RES_WAIT_FOREVER - wait until GPADC becomes available
 *              Other value specifies how many ticks to wait until GPADC becomes available
 *
 * \return 0 on success, negative value on error
 *
 */
DEPRECATED_MSG("API no longer supported, use ad_gpadc_read_nof_conv() instead.")
int ad_gpadc_read_raw_to(const ad_gpadc_handle_t handle, uint16_t *value, uint32_t timeout);

/**
 * \brief Read synchronously nof_conv conversions from the selected source
 *
 * This function starts synchronous measurement read. Blocking read  - caller task will block until the resource becomes available
 *
 * \param [in]  handle           handle to GPADC source
 * \param [in]  nof_conv         number of conversions to be delivered. Must be non-zero
 * \param [out] outbuf           pointer to conversion results buffer. Output buffer contains raw values
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 * \return 0 on success, negative value on error
 *
 */
int ad_gpadc_read_nof_conv(const ad_gpadc_handle_t handle, int nof_conv, uint16_t *outbuf);

#endif /* CONFIG_GPADC_USE_SYNC_TRANSACTIONS */

#if (CONFIG_GPADC_USE_ASYNC_TRANSACTIONS == 1)

/**
 * \brief Read asynchronously nof_conv conversions from the selected source
 *
 * This function starts asynchronous measurement read. Non blocking read operation
 *
 * \param [in]  handle           handle to GPADC source
 * \param [in]  nof_conv         number of conversions to be delivered. Must be non-zero
 * \param [out] outbuf           pointer to conversion results buffer. Output buffer contains raw values
 * \param [in]  read_async_cb    user callback fired after read operation completes
 * \param [in]  user_data        pointer to user data passed to callback
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 * \return 0 on success, negative value on error
 *
 */
int ad_gpadc_read_nof_conv_async(const ad_gpadc_handle_t handle, int nof_conv, uint16_t *outbuf, ad_gpadc_user_cb read_async_cb, void *user_data);

/**
 * \brief Read asynchronously value of the measurement from the selected source
 *
 * This function starts asynchronous measurement read.
 *
 * \param [in] handle           handle to GPADC source
 * \param [in] read_async_cb    user callback fired after read operation completes
 * \param [in] user_data        pointer to user data
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 * \return 0 on success, negative value on error
 *
 */
DEPRECATED_MSG("API no longer supported, use ad_gpadc_read_nof_conv_async() instead.")
int ad_gpadc_read_async(ad_gpadc_handle_t handle, ad_gpadc_user_cb read_async_cb, void *user_data);

/**
 * \brief Read asynchronously the measurement from the selected source directly through the result register
 *
 * This function starts asynchronous measurement read.
 *
 * \param [in] handle           handle to GPADC source
 * \param [in] read_async_cb    user callback fired after read operation completes
 * \param [in] user_data        pointer to user data
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 * \return 0 on success, negative value on error
 *
 */
DEPRECATED_MSG("API no longer supported, use ad_gpadc_read_nof_conv_async() instead")
int ad_gpadc_read_raw_async(ad_gpadc_handle_t handle, ad_gpadc_user_cb read_async_cb, void *user_data);
#endif /* CONFIG_GPADC_USE_ASYNC_TRANSACTIONS */

/**
 * \brief Return maximum value that can be read for ADC source
 *
 * A GPADC raw value can have 10 to 16 valid bits (left aligned) depending on
 * oversampling specified in source description. This function will return value
 * 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF or 0xFFFF depending on oversampling,
 * offering a right-aligned representation for the maximum value
 * that the GPADC can return (i.e. when the measured voltage equals to Vref).
 *
 * \param [in] drv   GPADC driver configuration structure
 *
 * \return value 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF or 0xFFFF
 *
 */
uint16_t ad_gpadc_get_source_max(const ad_gpadc_driver_conf_t *drv);

/**
 * \brief Open GPADC controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 *
 * \param [in] conf controller configuration
 *
 * \return pointer to adapter instance - should be used in subsequent API calls,
 *         NULL on error
 *
 * \note The function will block until it acquires all controller resources
 */
ad_gpadc_handle_t ad_gpadc_open(const ad_gpadc_controller_conf_t *conf);

/**
 * \brief Reconfigure GPADC controller
 *
 * This function will apply a new GPADC driver configuration.
 *
 * \param [in] p     pointer returned from ad_gpadc_open()
 * \param [in] drv   GPADC driver configuration structure
 *
 * \return 0 on success, negative value on error
 */
int ad_gpadc_reconfig(const ad_gpadc_handle_t p, const ad_gpadc_driver_conf_t *drv);

/**
* \brief Initialize controller pins to on / off io configuration
*
* This function should be called for setting pins to the correct level before external
* devices are powered up (e.g on system init). It does not need to be called before every
* ad_gpadc_open() call.
*
* \note This function applies the i/o configuration for the negative side
* only if the negative pin in \ref ad_gpadc_io_conf_t is not set to
* \ref HW_GPIO_PORT_NONE / \ref HW_GPIO_PIN_NONE.
* The positive side i/o is configured unconditionally.
*
* \param [in] id         controller instance
* \param [in] io         controller io configuration
* \param [in] state      on/off io configuration
*
* \return 0 on success, negative value on error
*/
int ad_gpadc_io_config (const HW_GPADC_ID id, const ad_gpadc_io_conf_t *io, AD_IO_CONF_STATE state);

/**
 * \brief Close GPADC controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_gpadc_open())
 * - Releases the controller resources
 *
 * \param [in] p        pointer returned from ad_gpadc_open()
 * \param [in] force    force close even if an async read is pending
 *
* \return 0 on success, negative value on error
 */
int ad_gpadc_close(ad_gpadc_handle_t p, bool force);

/**
 * \brief Convert raw value read from GPADC to temperature value in degrees Celsius
 *
 * \param [in] drv         GPADC driver configuration structure, NULL to use the current ADC settings
 * \param [in] raw_value   raw value returned from ad_gpadc_read_nof_conv() or ad_gpadc_read_nof_conv_async()
 *
 * \return value of temperature in degrees Celsius
 *
 */
int ad_gpadc_conv_to_temp(const ad_gpadc_driver_conf_t *drv, uint16_t raw_value);

/**
 * \brief Convert value read from GPADC to battery voltage in mV.
 *        The same configuration which was used to obtain the adc_value
 *        is needed for the conversion.
 *
 * \param [in] drv     GPADC driver configuration structure, NULL to use the current ADC settings
 * \param [in] value   value returned from ad_gpadc_read() or ad_gpadc_read_async()
 *
 * \return battery voltage in mV
 *
 */
DEPRECATED_MSG("API no longer supported, use ad_gpadc_conv_raw_to_batt_mvolt() instead")
uint16_t ad_gpadc_conv_to_batt_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t value);

/**
 * \brief Convert raw value read from GPADC to battery voltage in mV.
 *        The same configuration which was used to obtain the adc_value
 *        is needed for the conversion.
 *
 * \param [in] drv         GPADC driver configuration structure, NULL to use the current ADC settings
 * \param [in] raw_value   value returned from ad_gpadc_read_nof_conv() or ad_gpadc_read_nof_conv_async()
 *
 * \return battery voltage in mV
 *
 */
uint16_t ad_gpadc_conv_raw_to_batt_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t raw_value);

/**
 * \brief Convert raw value read from GPADC to voltage in mV.
 *        The same configuration which was used to obtain the adc_value
 *        is needed for the conversion.
 *
 * \param [in] drv         GPADC driver configuration structure, NULL to use the current ADC settings
 * \param [in] raw_value   value returned from ad_gpadc_read_nof_conv() or ad_gpadc_read_nof_conv_async()
 *
 * \return voltage in mV
 *
 */
int ad_gpadc_conv_to_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t raw_value);



#ifdef __cplusplus
}
#endif

#endif /* AD_GPADC_H_ */

#endif /* dg_configGPADC_ADAPTER */

/**
 * \}
 * \}
 */
