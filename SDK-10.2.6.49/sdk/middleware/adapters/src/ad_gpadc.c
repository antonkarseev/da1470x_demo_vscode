/**
 ****************************************************************************************
 *
 * @file ad_gpadc.c
 *
 * @brief GPADC adapter implementation
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configGPADC_ADAPTER == 1)

#include <stdarg.h>
#include <stdint.h>
#include "ad.h"
#include "ad_gpadc.h"
#include "interrupts.h"
#include "resmgmt.h"
#include "hw_gpadc.h"
#include "sdk_defs.h"
#include "hw_gpio.h"
#include "hw_sys.h"
#include "sys_bsr.h"

#if (CONFIG_GPADC_USE_SYNC_TRANSACTIONS == 0) && (CONFIG_GPADC_USE_ASYNC_TRANSACTIONS == 0)
#error "At least one macro CONFIG_GPADC_USE_SYNC_TRANSACTIONS or CONFIG_GPADC_USE_ASYNC_TRANSACTIONS must be set"
#endif

/**
 * \def CONFIG_AD_GPADC_LOCKING
 *
 * \brief Controls whether GPADC adapter resource locking is enabled
 *
 * By default, the GPADC adapter internally handles concurrent accesses to GPADC by different masters
 * and tasks. If resource locking is disabled by setting this macro to 0, all such internal handling
 * is disabled, thus becoming the application's responsibility to handle concurrent accesses, using
 * the busy status register (BSR) driver and controlling the resource management.
 */
#ifndef CONFIG_AD_GPADC_LOCKING
# define CONFIG_AD_GPADC_LOCKING        ( 1 )
#endif /* CONFIG_AD_GPADC_LOCKING */

/*
 * Resource allocation functions
 */
#if (CONFIG_AD_GPADC_LOCKING == 1)
# define GPADC_MUTEX_CREATE(mutex)                      do { \
                                                                OS_ASSERT((mutex) == NULL); \
                                                                OS_MUTEX_CREATE(mutex); \
                                                                OS_ASSERT(mutex); \
                                                        } while (0)

# define GPADC_MUTEX_GET(mutex)                         do { \
                                                                OS_ASSERT(mutex); \
                                                                OS_MUTEX_GET((mutex), OS_MUTEX_FOREVER); \
                                                        } while (0)

# define GPADC_MUTEX_GET_TIMEOUT(mutex, timeout)        ({ \
                                                                OS_BASE_TYPE ret; \
                                                                OS_ASSERT(mutex); \
                                                                ret = OS_MUTEX_GET((mutex), (timeout)); \
                                                                ret; \
                                                        })

# define GPADC_MUTEX_PUT(mutex)         OS_MUTEX_PUT(mutex)

# if (SNC_PROCESSOR_BUILD)
#  define GPADC_BSR_MASTER           SYS_BSR_MASTER_SNC
# else
#  define GPADC_BSR_MASTER           SYS_BSR_MASTER_SYSCPU
# endif

# define GPADC_BSR_ACQUIRE(periph_id)   sys_bsr_acquire(GPADC_BSR_MASTER, (periph_id))
# define GPADC_BSR_RELEASE(periph_id)   sys_bsr_release(GPADC_BSR_MASTER, (periph_id))

# define GPADC_RES_ACQUIRE(timeout)     ad_gpadc_acquire(timeout)
# define GPADC_RES_RELEASE()            ad_gpadc_release()
#else /* CONFIG_AD_GPADC_LOCKING == 0 */
# define GPADC_MUTEX_CREATE(mutex)      do {} while (0)
# define GPADC_MUTEX_GET(mutex)         do {} while (0)
# define GPADC_MUTEX_PUT(mutex)         do {} while (0)

# define GPADC_BSR_ACQUIRE(periph_id)   do {} while (0)
# define GPADC_BSR_RELEASE(periph_id)   do {} while (0)

# define GPADC_RES_ACQUIRE(timeout)     ({ true; })
# define GPADC_RES_RELEASE()            do {} while (0)
#endif /* CONFIG_AD_GPADC_LOCKING */

#define AD_GPADC_ASSERT_HANDLE_VALID(__handle)                          \
        OS_ASSERT(__handle == dynamic_data.handle && __handle != NULL); \
        if (__handle != dynamic_data.handle || __handle == NULL) {      \
                return AD_GPADC_ERROR_HANDLE_INVALID;                   \
        }

/**
 * \brief GPADC adapter (internal) data
 *
 * Data structure of GPADC controller
 *
 */
typedef struct ad_gpadc_data {
        /**< GPADC controller current configuration */
        ad_gpadc_controller_conf_t      *conf;                  /**< Current GPADC configuration */
#if (CONFIG_GPADC_USE_ASYNC_TRANSACTIONS == 1)
        ad_gpadc_user_cb                read_cb;                /**< User function to call after asynchronous read finishes */
        void                            *user_data;             /**< User data for callback */
#endif
        ad_gpadc_handle_t               handle;                 /**< The handle for the active controller */
        /**< Internal data */
#if (CONFIG_AD_GPADC_LOCKING == 1)
        OS_MUTEX                        busy;                   /**< Semaphore for thread safety */
#endif
        OS_EVENT                        sync_event;
        bool                            read_in_progress;       /**< Asynchronous read in progress indication */
        bool                            latch_input0;           /**< flag to indicate if input 0 needs latching */
        bool                            latch_input1;           /**< flag to indicate if input 1 needs latching */
} ad_gpadc_data;

__RETAINED static ad_gpadc_data dynamic_data;

#include <sys_power_mgr.h>

int ad_gpadc_io_config(const HW_GPADC_ID id, const ad_gpadc_io_conf_t *io, AD_IO_CONF_STATE state)
{
        uint8_t size;
        AD_GPADC_ERROR ret = AD_GPADC_ERROR_NONE;

        if (io == NULL) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        size = AD_IO_PIN_PORT_VALID(io->input1.port, io->input1.pin) ? 2 : 1;

        if (ad_io_configure(&io->input0, size, io->voltage_level, state) != AD_IO_ERROR_NONE) {
                return AD_GPADC_ERROR_IO_CFG_INVALID;
        }

        GPADC_MUTEX_GET(dynamic_data.busy);

        /* If ad_gpadc_io_config() is called before ad_gpadc_open()
         * the handle does not exist yet.
         * Trusting the user I/O configuration without drv validation checks. */
        if (dynamic_data.handle == NULL) {
                if (ad_io_set_pad_latch(&io->input0, size, AD_IO_PAD_LATCHES_OP_TOGGLE) != AD_IO_ERROR_NONE) {
                        ret = AD_GPADC_ERROR_IO_CFG_INVALID;
                }
                goto io_config_end;
        }

        /* If ad_gpadc_io_config() is called through ad_gpadc_open()
         * the dynamic_data contains valid info on which pins need to be latched. */
        if (dynamic_data.latch_input0) {
                if (ad_io_set_pad_latch(&io->input0, 1, AD_IO_PAD_LATCHES_OP_TOGGLE) != AD_IO_ERROR_NONE) {
                        ret = AD_GPADC_ERROR_IO_CFG_INVALID;
                        goto io_config_end;
                }
        }
        if (dynamic_data.latch_input1) {
                if (ad_io_set_pad_latch(&io->input1, 1, AD_IO_PAD_LATCHES_OP_TOGGLE) != AD_IO_ERROR_NONE) {
                        ret = AD_GPADC_ERROR_IO_CFG_INVALID;
                        goto io_config_end;
                }
        }

io_config_end:

        GPADC_MUTEX_PUT(dynamic_data.busy);

        return ret;
}

void ad_gpadc_init(void)
{
        dynamic_data.conf = NULL;
        GPADC_MUTEX_CREATE(dynamic_data.busy);
        OS_EVENT_CREATE(dynamic_data.sync_event);
}

#if (CONFIG_AD_GPADC_LOCKING == 1)
static bool ad_gpadc_acquire(uint32_t timeout)
{
        if (resource_acquire(RES_MASK(RES_ID_GPADC), timeout)) {
                return true;
        }

        return false;
}

static void ad_gpadc_release(void)
{
        resource_release(RES_MASK(RES_ID_GPADC));
}
#endif /* CONFIG_AD_GPADC_LOCKING */

static bool validate_drv_config(const ad_gpadc_controller_conf_t *conf)
{
        if (conf->drv->input_mode == HW_GPADC_INPUT_MODE_SINGLE_ENDED) {
                switch (conf->drv->positive) {
                case HW_GPADC_INPUT_ADC0:
                case HW_GPADC_INPUT_ADC1:
                case HW_GPADC_INPUT_ADC2:
                case HW_GPADC_INPUT_ADC3:
                        if (!conf->io) {
                                /* Mandatory GPIO configuration for the above inputs */
                                return false;
                        }
                        dynamic_data.latch_input0 = true;
                        dynamic_data.latch_input1 = false;
                        break;
                case HW_GPADC_INP_MUX1:
                case HW_GPADC_INP_DIFF_TEMP:
                case HW_GPADC_INP_MUX2:
                case HW_GPADC_INP_DIE_TEMP:
                case HW_GPADC_INP_NC:
                case HW_GPADC_INP_I_SENSE_BUS:
                case HW_GPADC_INP_V30:
                case HW_GPADC_INP_V18F:
                case HW_GPADC_INP_V12:
                case HW_GPADC_INP_V18:
                case HW_GPADC_INP_V14:
                case HW_GPADC_INP_V18P:
                case HW_GPADC_INP_VSYS:
                case HW_GPADC_INP_VBUS:
                case HW_GPADC_INP_VBAT:
                        dynamic_data.latch_input0 = false;
                        dynamic_data.latch_input1 = false;
                        break;
                default:
                        return false;
                }
        } else {
                /* HW_GPADC_INPUT_MODE_DIFFERENTIAL */
                if (!conf->io) {
                        /* Only GPIO configurations are valid in this mode */
                        return false;
                }
                switch (conf->drv->positive) {
                case HW_GPADC_INPUT_ADC0:
                case HW_GPADC_INPUT_ADC1:
                case HW_GPADC_INPUT_ADC2:
                case HW_GPADC_INPUT_ADC3:
                        break;
                default:
                        return false;
                }
                switch (conf->drv->negative) {
                case HW_GPADC_INPUT_ADC0:
                case HW_GPADC_INPUT_ADC1:
                case HW_GPADC_INPUT_ADC2:
                case HW_GPADC_INPUT_ADC3:
                        break;
                default:
                        return false;
                }
                dynamic_data.latch_input0 = true;
                dynamic_data.latch_input1 = true;
        }
        return true;
}

static int ad_gpadc_check_and_apply_config(const ad_gpadc_controller_conf_t *conf, AD_IO_CONF_STATE onoff)
{
        if (!conf || !conf->drv) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        /* Validate input channel combinations and mark GPIO's to-be-latched */
        if (!validate_drv_config(conf)) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        /* Apply I/O configuration and latching */
        if (conf->io) {
                return ad_gpadc_io_config(conf->id, conf->io, onoff);
        }

        return AD_GPADC_ERROR_NONE;
}

int ad_gpadc_reconfig(const ad_gpadc_handle_t handle, const ad_gpadc_driver_conf_t *drv)
{
        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        if (drv == NULL) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        GPADC_MUTEX_GET(dynamic_data.busy);

        if (dynamic_data.conf == NULL) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                // Use ad_gpadc_open instead
                return AD_GPADC_ERROR_ADAPTER_NOT_OPEN;
        }

        if ((dynamic_data.conf->drv->positive != drv->positive) ||
                (dynamic_data.conf->drv->negative != drv->negative)) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                // Not allowed to change input with reconfig
                return AD_GPADC_ERROR_CHANGE_NOT_ALLOWED;
        }

        if (dynamic_data.read_in_progress) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
        }

        dynamic_data.conf->drv = (ad_gpadc_driver_conf_t *) drv;

        hw_gpadc_configure(dynamic_data.conf->drv);

        GPADC_MUTEX_PUT(dynamic_data.busy);

        return AD_GPADC_ERROR_NONE;
}

ad_gpadc_handle_t ad_gpadc_open(const ad_gpadc_controller_conf_t *conf)
{
        if (conf == NULL) {
                return NULL;
        }

        /* The driver configuration is mandatory */
        if (conf->drv == NULL) {
                return NULL;
        }

        pm_sleep_mode_request(pm_mode_idle);

        GPADC_BSR_ACQUIRE(SYS_BSR_PERIPH_ID_GPADC);
        GPADC_RES_ACQUIRE(RES_WAIT_FOREVER);

        if (dynamic_data.conf != NULL) {
                //use ad_gpadc_reconfig instead
                GPADC_RES_RELEASE();
                GPADC_BSR_RELEASE(SYS_BSR_PERIPH_ID_GPADC);
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }

        /* Power-up the ADC block */
        hw_sys_pd_com_enable();

        if (ad_gpadc_check_and_apply_config(conf, AD_IO_CONF_ON) < 0) {
                /* Power-down the ADC block */
                hw_sys_pd_com_disable();
                GPADC_RES_RELEASE();
                GPADC_BSR_RELEASE(SYS_BSR_PERIPH_ID_GPADC);
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }

        hw_gpadc_init((gpadc_config*) conf->drv, true);

        dynamic_data.conf        = (ad_gpadc_controller_conf_t *) conf;
        dynamic_data.handle      = (ad_gpadc_handle_t *) conf;

        return dynamic_data.handle;
}

int ad_gpadc_close(ad_gpadc_handle_t handle, bool force)
{
        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        OS_ENTER_CRITICAL_SECTION();
        if (dynamic_data.read_in_progress) {
                if (force) {
                        hw_gpadc_unregister_interrupt();
                        dynamic_data.read_in_progress = false;
                } else {
                        OS_LEAVE_CRITICAL_SECTION();
                        return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
                }
        }
        OS_LEAVE_CRITICAL_SECTION();

        hw_gpadc_init(NULL, false);

        /* The ad_gpadc_check_and_apply_config() has verified the I/O configuration in open(),
         * we call here the ad_gpadc_io_config() with no checks to apply the OFF configuration.
         */
        ad_gpadc_io_config(handle, dynamic_data.conf->io, AD_IO_CONF_OFF);

        dynamic_data.conf = NULL;
        dynamic_data.handle = NULL;

        /* Power-down the ADC block */
        hw_sys_pd_com_disable();

        GPADC_RES_RELEASE();
        GPADC_BSR_RELEASE(SYS_BSR_PERIPH_ID_GPADC);

        pm_sleep_mode_release(pm_mode_idle);

        return AD_GPADC_ERROR_NONE;
}

#if (CONFIG_GPADC_USE_ASYNC_TRANSACTIONS == 1)
__STATIC_INLINE void ad_gpadc_cb_with_get_function(uint16_t (*get_function)(void))
{
        int val;

        if (!dynamic_data.read_in_progress) {
                return;
        }

        val = get_function();

        dynamic_data.read_cb(dynamic_data.user_data, val);

        dynamic_data.read_in_progress = false;
}

static void ad_gpadc_cb(void *param, uint32_t to_go)
{
        ad_gpadc_cb_with_get_function(hw_gpadc_get_value);
}

static void ad_gpadc_raw_cb(void *param, uint32_t to_go)
{
        ad_gpadc_cb_with_get_function(hw_gpadc_get_raw_value);
}

__STATIC_INLINE int ad_gpadc_read_async_with_val_type(const ad_gpadc_handle_t handle, ad_gpadc_user_cb read_async_cb, void *user_data, bool need_raw)
{
        if (!read_async_cb) {
                return AD_GPADC_ERROR_OTHER;
        }

        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        GPADC_MUTEX_GET(dynamic_data.busy);

        if (dynamic_data.read_in_progress) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
        }

        dynamic_data.read_in_progress = true;

        dynamic_data.read_cb = read_async_cb;
        dynamic_data.user_data = user_data;

        if (need_raw) {
                if (!hw_gpadc_read(1, NULL, ad_gpadc_raw_cb, NULL)) {
                        GPADC_MUTEX_PUT(dynamic_data.busy);
                        return AD_GPADC_ERROR_OTHER;
                }
        } else {
                if (!hw_gpadc_read(1, NULL, ad_gpadc_cb, NULL)) {
                        GPADC_MUTEX_PUT(dynamic_data.busy);
                        return AD_GPADC_ERROR_OTHER;
                }
        }

        GPADC_MUTEX_PUT(dynamic_data.busy);

        return AD_GPADC_ERROR_NONE;
}

int ad_gpadc_read_async(ad_gpadc_handle_t handle, ad_gpadc_user_cb read_async_cb, void *user_data)
{
        return ad_gpadc_read_async_with_val_type(handle, read_async_cb, user_data, false);
}

int ad_gpadc_read_raw_async(ad_gpadc_handle_t handle, ad_gpadc_user_cb read_async_cb, void *user_data)
{
        return ad_gpadc_read_async_with_val_type(handle, read_async_cb, user_data, true);
}

static void gpadc_cb_wrapper_async(void *param, uint32_t to_go)
{
        if (!dynamic_data.read_in_progress) {
                return;
        }

        if (dynamic_data.read_cb != NULL) {
                dynamic_data.read_cb(dynamic_data.user_data, to_go);
        }

        dynamic_data.read_in_progress = false;
}

int ad_gpadc_read_nof_conv_async(const ad_gpadc_handle_t handle, int nof_conv, uint16_t *outbuf, ad_gpadc_user_cb read_async_cb, void *user_data)
{
        AD_GPADC_ASSERT_HANDLE_VALID(handle);

        GPADC_MUTEX_GET(dynamic_data.busy);

        if (nof_conv > 1) {
                hw_gpadc_set_continuous(true);
        }

        if (dynamic_data.read_in_progress) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
        }

        dynamic_data.read_in_progress = true;

        dynamic_data.read_cb = read_async_cb;
        dynamic_data.user_data = user_data;

        if (!hw_gpadc_read(nof_conv, outbuf, gpadc_cb_wrapper_async, NULL)) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_OTHER;
        }

        GPADC_MUTEX_PUT(dynamic_data.busy);

        return AD_GPADC_ERROR_NONE;
}

#endif /* CONFIG_GPADC_USE_ASYNC_TRANSACTIONS */

#if (CONFIG_GPADC_USE_SYNC_TRANSACTIONS == 1)

static int ad_gpadc_read_internal_to(ad_gpadc_controller_conf_t *conf, uint16_t *value,
        uint32_t timeout, uint16_t (*hw_gpadc_measurement_func)(void))
{
        OS_BASE_TYPE acquisition_result;

        ASSERT_WARNING(hw_gpadc_measurement_func);

        acquisition_result = GPADC_MUTEX_GET_TIMEOUT(dynamic_data.busy, timeout);

        if (acquisition_result == false) {
                return AD_GPADC_ERROR_OTHER;
        }

        if (dynamic_data.read_in_progress) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
        }

        if (!hw_gpadc_read(1, NULL, NULL, NULL)) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_OTHER;
        }

        *value = hw_gpadc_measurement_func();

        GPADC_MUTEX_PUT(dynamic_data.busy);

        return AD_GPADC_ERROR_NONE;
}

int ad_gpadc_read(const ad_gpadc_handle_t handle, uint16_t *value)
{
        bool acquired __UNUSED;
        int ret;

        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        ret = ad_gpadc_read_internal_to(dynamic_data.conf, value, RES_WAIT_FOREVER, hw_gpadc_get_value);

        return ret;
}

int ad_gpadc_read_raw(const ad_gpadc_handle_t handle, uint16_t *value)
{
        bool acquired __UNUSED;
        int ret;

        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        ret = ad_gpadc_read_internal_to(dynamic_data.conf, value, RES_WAIT_FOREVER, hw_gpadc_get_raw_value);
        OS_ASSERT(ret == AD_GPADC_ERROR_NONE);

        return ret;
}

int ad_gpadc_read_raw_to(const ad_gpadc_handle_t handle, uint16_t *value, uint32_t timeout)
{
        int ret;

        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        ret = ad_gpadc_read_internal_to(dynamic_data.conf, value, timeout, hw_gpadc_get_raw_value);

        return ret;
}

int ad_gpadc_read_to(const ad_gpadc_handle_t handle, uint16_t *value, uint32_t timeout)
{
        int ret;

        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        ret = ad_gpadc_read_internal_to(dynamic_data.conf, value, timeout, hw_gpadc_get_value);

        return ret;
}

static void gpadc_cb_wrapper_sync(void *param, uint32_t to_go)
{
        OS_EVENT_SIGNAL_FROM_ISR(dynamic_data.sync_event);
}

int ad_gpadc_read_nof_conv(const ad_gpadc_handle_t handle, int nof_conv, uint16_t *outbuf)
{
        AD_GPADC_ASSERT_HANDLE_VALID(handle);

        GPADC_MUTEX_GET(dynamic_data.busy);

        if (nof_conv > 1) {
                hw_gpadc_set_continuous(true);
        }

        if (dynamic_data.read_in_progress) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
        }

        if (!hw_gpadc_read(nof_conv, outbuf, gpadc_cb_wrapper_sync, NULL)) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_OTHER;
        }

        OS_EVENT_WAIT(dynamic_data.sync_event, OS_EVENT_FOREVER);

        GPADC_MUTEX_PUT(dynamic_data.busy);

        return AD_GPADC_ERROR_NONE;
}
#endif /* CONFIG_GPADC_USE_SYNC_TRANSACTIONS */

uint16_t ad_gpadc_get_source_max(const ad_gpadc_driver_conf_t *drv)
{
        HW_GPADC_OVERSAMPLING ovs = drv ? drv->oversampling : hw_gpadc_get_oversampling();
        return 0xFFFF >> (HW_GPADC_UNUSED_BITS - MIN(HW_GPADC_UNUSED_BITS, ovs));
}

inline int ad_gpadc_conv_to_temp(const ad_gpadc_driver_conf_t *drv, uint16_t value)
{
        return hw_gpadc_convert_to_celsius_x100_util(drv, value) / 100;
}

uint16_t ad_gpadc_conv_to_batt_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t value)
{
        uint16_t source_max =  ad_gpadc_get_source_max(drv);
        uint32_t compensated_adc;
        uint32_t attn_scaler = drv ? drv->input_attenuator : hw_gpadc_get_input_attenuator_state();

        attn_scaler++;
        compensated_adc = hw_gpadc_internal_scaler_compensate(HW_GPADC_INP_VBAT, value);
        return ((uint32_t) HW_GPADC_VREF_MILLIVOLT * attn_scaler * compensated_adc) / source_max;
}

uint16_t ad_gpadc_conv_raw_to_batt_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t raw_value)
{
        uint16_t source_max =  UINT16_MAX;
        uint16_t value =  hw_gpadc_apply_correction(drv, raw_value);
        uint32_t compensated_adc;
        uint32_t attn_scaler = drv ? drv->input_attenuator : hw_gpadc_get_input_attenuator_state();

        attn_scaler++;
        compensated_adc = hw_gpadc_internal_scaler_compensate(HW_GPADC_INP_VBAT, value);
        return ((uint32_t) HW_GPADC_VREF_MILLIVOLT * attn_scaler * compensated_adc) / source_max;
}

inline int ad_gpadc_conv_to_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t raw_value)
{
        return hw_gpadc_convert_to_millivolt(drv, raw_value);
}

ADAPTER_INIT(ad_gpadc_adapter, ad_gpadc_init);

#endif /* dg_configGPADC_ADAPTER */
