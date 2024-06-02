/**
 ****************************************************************************************
 *
 * @file ad_i3c.c
 *
 * @brief I3C device access API implementation
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configI3C_ADAPTER

#include <stdarg.h>
#include "ad_i3c.h"
#include "hw_i3c.h"
#include "hw_dma.h"
#include "hw_clk.h"
#include "interrupts.h"
#include "resmgmt.h"
#include "sys_power_mgr.h"
#include "hw_sys.h"
#include "sdk_list.h"
#include "sys_bsr.h"

/**
 * \def CONFIG_AD_I3C_LOCKING
 *
 * \brief Controls whether I3C adapter resource locking is enabled
 *
 * By default, the I3C adapter internally handles concurrent accesses on an I3C bus by different masters
 * and tasks. If resource locking is disabled by setting this macro to 0, all such internal handling
 * is disabled, thus becoming the application's responsibility to handle concurrent accesses, using
 * the busy status register (BSR) driver and controlling the resource management.
 */
#ifndef CONFIG_AD_I3C_LOCKING
# define CONFIG_AD_I3C_LOCKING                  (1)
#endif /* CONFIG_AD_I3C_LOCKING */

/*
 * Resource allocation functions
 */
#if (CONFIG_AD_I3C_LOCKING == 1)
# define I3C_MUTEX_CREATE(mutex)                do { \
                                                        OS_ASSERT((mutex) == NULL); \
                                                        OS_MUTEX_CREATE(mutex); \
                                                        OS_ASSERT(mutex); \
                                                } while (0)
# define I3C_MUTEX_GET(mutex)                   do { \
                                                        OS_ASSERT(mutex); \
                                                        OS_MUTEX_GET((mutex), OS_MUTEX_FOREVER); \
                                                } while (0)
# define I3C_MUTEX_PUT(mutex)                   OS_MUTEX_PUT(mutex)

# if (SNC_PROCESSOR_BUILD)
#  define I3C_BSR_MASTER                        SYS_BSR_MASTER_SNC
# else
#  define I3C_BSR_MASTER                        SYS_BSR_MASTER_SYSCPU
# endif

# define I3C_BSR_ACQUIRE(periph_id)             sys_bsr_acquire(I3C_BSR_MASTER, (periph_id))
# define I3C_BSR_RELEASE(periph_id)             sys_bsr_acquire(I3C_BSR_MASTER, (periph_id))

# define I3C_RES_ACQUIRE_CTL()                  ad_i3c_res_acquire_ctl()
# define I3C_RES_RELEASE_CTL()                  ad_i3c_res_release_ctl()

# if (HW_I3C_DMA_SUPPORT == 1)
#   define I3C_RES_ACQUIRE_DMA(dma_channel)     ad_i3c_res_acquire_dma(dma_channel)
#   define I3C_RES_RELEASE_DMA(dma_channel)     ad_i3c_res_release_dma(dma_channel)
#endif
#else
# define I3C_MUTEX_CREATE(mutex)                do {} while (0)
# define I3C_MUTEX_GET(mutex)                   do {} while (0)
# define I3C_MUTEX_PUT(mutex)                   do {} while (0)

# define I3C_BSR_ACQUIRE(periph_id)             do {} while (0)
# define I3C_BSR_RELEASE(periph_id)             do {} while (0)

# define I3C_RES_ACQUIRE_CTL()                  do {} while (0)
# define I3C_RES_RELEASE_CTL()                  do {} while (0)

# if (HW_I3C_DMA_SUPPORT == 1)
#   define I3C_RES_ACQUIRE_DMA(dma_channel)     do {} while (0)
#   define I3C_RES_RELEASE_DMA(dma_channel)     do {} while (0)
#endif
#endif /* CONFIG_AD_I3C_LOCKING */

#define AD_I3C_SCL_GPIO_IS_VALID(port, pin) ((port == HW_GPIO_PORT_1) && (pin == HW_GPIO_PIN_12))
#define AD_I3C_SDA_GPIO_IS_VALID(port, pin) ((port == HW_GPIO_PORT_1) && (pin == HW_GPIO_PIN_11))

/**
 * \brief I3C adapter (internal) dynamic data
 *
 * Dynamic data structure of I3C controller
 *
 */
typedef struct {
        /**< I3C controller current configuration */
        const ad_i3c_controller_conf_t *conf;
#if (CONFIG_AD_I3C_LOCKING == 1)
        /**< Internal data */
        OS_TASK  owner;         /**< The task which opened the controller */
#endif
} ad_i3c_dynamic_data_t;

static ad_i3c_dynamic_data_t i3c_dynamic_data;


#if (CONFIG_I3C_USE_SYNC_TRANSACTIONS == 1) || (CONFIG_AD_I3C_LOCKING == 1)
/**
 * \brief I3C adapter (internal) static data
 *
 * Static data structure of I2C controller
 *
 */
typedef struct {
#if CONFIG_I3C_USE_SYNC_TRANSACTIONS
        OS_EVENT event; /**< Semaphore for blocking calls  */
#endif /* CONFIG_I3C_USE_SYNC_TRANSACTIONS */
#if (CONFIG_AD_I3C_LOCKING == 1)
        OS_MUTEX busy;  /**< Semaphore for thread safety   */
#endif
}ad_i3c_static_data_t;

__RETAINED static ad_i3c_static_data_t i3c_static_data;
#endif /* (CONFIG_I3C_USE_SYNC_TRANSACTIONS == 1) || (CONFIG_AD_I3C_LOCKING == 1) */

#define AD_I3C_IO_SIZE            (2)

/**
 * \brief Checks if the provided handle is valid
 */
#define AD_I3C_HANDLE_IS_VALID(x) (((x) == &i3c_dynamic_data) && (((ad_i3c_dynamic_data_t *)(x))->conf != NULL))

#if (CONFIG_AD_I3C_LOCKING == 1)
#if (HW_I3C_DMA_SUPPORT == 1)
static void ad_i3c_res_acquire_dma(HW_DMA_CHANNEL dma_channel)
{
        if (dma_channel < HW_DMA_CHANNEL_INVALID) {
                resource_acquire(RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                        RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1), RES_WAIT_FOREVER);
        }
}
#endif /* HW_I3C_DMA_SUPPORT */

static void ad_i3c_res_acquire_ctl(void)
{
        resource_acquire(RES_MASK(RES_ID_I3C), RES_WAIT_FOREVER);
}

#if (HW_I3C_DMA_SUPPORT == 1)
static void ad_i3c_res_release_dma(HW_DMA_CHANNEL dma_channel)
{
        if (dma_channel < HW_DMA_CHANNEL_INVALID) {
                resource_release(RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                        RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1));
        }
}
#endif /* HW_I3C_DMA_SUPPORT */

static void ad_i3c_res_release_ctl(void)
{
        resource_release(RES_MASK(RES_ID_I3C));

}
#endif /* CONFIG_AD_I3C_LOCKING */

static void  ad_i3c_release_resources(const ad_i3c_controller_conf_t *conf)
{
        /* Release resources */
#if (HW_I3C_DMA_SUPPORT == 1)
        I3C_RES_RELEASE_DMA(conf->drv->i3c.dma_channel_pair);
#endif

        I3C_RES_RELEASE_CTL();

        I3C_BSR_RELEASE(SYS_BSR_PERIPH_ID_I3C);

        /* Power down related Power Domains (if not used by someone else). */
        hw_sys_pd_com_disable();

        /* Allow sleep. */
        pm_sleep_mode_release(pm_mode_idle);
}

AD_I3C_ERROR ad_i3c_io_config(const HW_I3C_ID id, const ad_i3c_io_conf_t *io_config, AD_IO_CONF_STATE state)
{
        if (id != HW_I3C) {
                return AD_I3C_ERROR_ID_INVALID;
        }

        if (io_config == NULL) {
                return AD_I3C_ERROR_IO_CFG_INVALID;
        }

        if ((!AD_I3C_SCL_GPIO_IS_VALID(io_config->scl.port, io_config->scl.pin)) ||
                (!AD_I3C_SDA_GPIO_IS_VALID(io_config->sda.port, io_config->sda.pin))) {
                return AD_I3C_ERROR_IO_CFG_INVALID;
        }

        ad_io_configure(&io_config->scl, AD_I3C_IO_SIZE, io_config->voltage_level, state);

        ad_io_set_pad_latch(&io_config->scl, AD_I3C_IO_SIZE, AD_IO_PAD_LATCHES_OP_TOGGLE);

        return AD_I3C_ERROR_NONE;
}

static AD_I3C_ERROR ad_i3c_validate_driver_config(ad_i3c_handle_t handle, const ad_i3c_driver_conf_t *drv_conf)
{
        if (drv_conf == NULL) {
                return AD_I3C_ERROR_DRIVER_CONF_INVALID;
        }

#if (HW_I3C_DMA_SUPPORT == 1)
        ad_i3c_dynamic_data_t *i3c = (ad_i3c_dynamic_data_t *)handle;

        if (i3c->conf->drv->i3c.dma_channel_pair != drv_conf->i3c.dma_channel_pair) {
                return AD_I3C_ERROR_DRIVER_CONF_INVALID;
        }
#endif

        return AD_I3C_ERROR_NONE;
}

static bool ad_i3c_controller_is_busy(void)
{
        return (hw_i3c_is_occupied() || !hw_i3c_controler_is_idle());
}

AD_I3C_ERROR ad_i3c_reconfig(ad_i3c_handle_t handle, const ad_i3c_driver_conf_t *conf)
{
        if (!(AD_I3C_HANDLE_IS_VALID(handle))) {
                return AD_I3C_ERROR_HANDLE_INVALID;
        }
#if (CONFIG_AD_I3C_LOCKING == 1)
        ad_i3c_static_data_t *i3c_static = &i3c_static_data;
#endif

        I3C_MUTEX_GET(i3c_static->busy);

        if (ad_i3c_validate_driver_config(handle, conf) != AD_I3C_ERROR_NONE) {
                I3C_MUTEX_PUT(i3c_static->busy);
                return AD_I3C_ERROR_DRIVER_CONF_INVALID;
        }

        /* Check if I3C is enabled */
        if (hw_i3c_is_clk_enabled()) {
                if (ad_i3c_controller_is_busy()) {
                        I3C_MUTEX_PUT(i3c_static->busy);
                        return AD_I3C_ERROR_CONTROLLER_BUSY;
               }
        }

        if (hw_i3c_init(&conf->i3c) < 0) {
                I3C_MUTEX_PUT(i3c_static->busy);
                return AD_I3C_ERROR_DRIVER_CONF_INVALID;
        }

        I3C_MUTEX_PUT(i3c_static->busy);

        return AD_I3C_ERROR_NONE;
}

ad_i3c_handle_t ad_i3c_open(const ad_i3c_controller_conf_t *conf)
{
        /* Check input validity */
        OS_ASSERT(conf);
        OS_ASSERT(conf->drv);
        OS_ASSERT(conf->io);

        ad_i3c_dynamic_data_t *i3c = &i3c_dynamic_data;

        /* Prevent sleep. */
        pm_sleep_mode_request(pm_mode_idle);

        /* Start acquiring resources */

        /* Arbitrate on multiple masters. */
        I3C_BSR_ACQUIRE(SYS_BSR_PERIPH_ID_I3C);

#if (HW_I3C_DMA_SUPPORT == 1)
        HW_DMA_CHANNEL dma_channel = conf->drv->i3c.dma_channel_pair;
        /* Arbitrate on multiple tasks. */
        I3C_RES_ACQUIRE_DMA(dma_channel);
#endif

        /* Arbitrate on multiple tasks. */
        I3C_RES_ACQUIRE_CTL();

        /* Power up related Power Domains (if not already up). */
        hw_sys_pd_com_enable();

        /* Apply I/O configuration. */
        if (ad_i3c_io_config(conf->id, conf->io, AD_IO_CONF_ON) != AD_I3C_ERROR_NONE) {
                /* I/O configuration failed, release resources */
                ad_i3c_release_resources(conf);

                return NULL;
        }

        /* Update adapter data */
        i3c->conf = conf;
#if (CONFIG_AD_I3C_LOCKING == 1)
        i3c->owner = OS_GET_CURRENT_TASK();
#endif

        /* Configure I3C controller driver */
        if (ad_i3c_reconfig(i3c, conf->drv) != AD_I3C_ERROR_NONE) {
                /* Apply I/O de-configuration */
                ad_i3c_io_config(i3c->conf->id, i3c->conf->io, AD_IO_CONF_OFF);

                /* Driver configuration failed, release resources */
                ad_i3c_release_resources(i3c->conf);

                i3c->conf = NULL;

                return NULL;
        }

        ad_io_set_pad_latch(&conf->io->scl, AD_I3C_IO_SIZE, AD_IO_PAD_LATCHES_OP_ENABLE);

        return i3c;
}

static bool ad_i3c_controller_abort_transfer(void)
{
        hw_i3c_controller_abort_transfer();
        uint8_t abort_cnt = 0;
        while (HW_I3C_REG_GETF(I3C_DEVICE_CTRL_REG, ABORT)) {
                if (abort_cnt == 10) {
                        return false;
                }
                abort_cnt++;
                hw_clk_delay_usec(10);
        }

        return true;
}

AD_I3C_ERROR ad_i3c_close(ad_i3c_handle_t handle, bool force)
{
        if (!(AD_I3C_HANDLE_IS_VALID(handle))) {
                return AD_I3C_ERROR_HANDLE_INVALID;
        }

        ad_i3c_dynamic_data_t *i3c = (ad_i3c_dynamic_data_t *)handle;

#if (HW_I3C_DMA_SUPPORT == 1)
        HW_DMA_CHANNEL dma_channel = i3c->conf->drv->i3c.dma_channel_pair;
#endif

        OS_ENTER_CRITICAL_SECTION();
        if (!force) {
                if (ad_i3c_controller_is_busy()) {
                        OS_LEAVE_CRITICAL_SECTION();
                        return AD_I3C_ERROR_CONTROLLER_BUSY;
                }
        } else {
                /* Abort ongoing transactions */
                if (ad_i3c_controller_is_busy()) {
                        hw_i3c_reset_xfer_cb();
                        if (!ad_i3c_controller_abort_transfer()) {
                                OS_LEAVE_CRITICAL_SECTION();
                                return AD_I3C_ERROR_CONTROLLER_ABORT_FAIL;
                        }
                }
        }

        /* Disable the I3C controller */
        hw_i3c_deinit();

#if (HW_I3C_DMA_SUPPORT == 1)
        if (dma_channel < HW_DMA_CHANNEL_INVALID - 1) {
                hw_dma_channel_enable(i3c->conf->drv->i3c.dma_channel_pair, HW_DMA_STATE_DISABLED);
                hw_dma_channel_enable(i3c->conf->drv->i3c.dma_channel_pair + 1, HW_DMA_STATE_DISABLED);
        }
#endif

        /* Apply I/O de-configuration */
        ad_i3c_io_config(i3c->conf->id, i3c->conf->io, AD_IO_CONF_OFF);

#if (CONFIG_AD_I3C_LOCKING == 1)
        i3c->owner = NULL;
#endif

        /* Release resources */
        ad_i3c_release_resources(i3c->conf);

        i3c->conf = NULL;

        OS_LEAVE_CRITICAL_SECTION();

        return AD_I3C_ERROR_NONE;
}

#if (CONFIG_I3C_USE_SYNC_TRANSACTIONS == 1)
typedef struct {
        ad_i3c_static_data_t *i3c;
        bool transfer_success;
        i3c_transfer_cmd_response cmd_response;
} ad_i3c_cb_data_t;

static void ad_i3c_xfer_cb(void *cb_data, bool success, i3c_transfer_cmd_response *cmd_response)
{
        ad_i3c_cb_data_t *data = cb_data;
        ad_i3c_static_data_t *i3c = data->i3c;
        data->transfer_success = success;
        data->cmd_response.valid = cmd_response->valid;
        data->cmd_response.response = cmd_response->response;
        OS_EVENT_SIGNAL_FROM_ISR(i3c->event);
}

int ad_i3c_private_write(ad_i3c_handle_t handle, const uint8_t *wbuf, size_t wlen, i3c_private_transfer_config *i3c_transfer_cfg, OS_TICK_TIME timeout)
{
        if (!(AD_I3C_HANDLE_IS_VALID(handle))) {
                return AD_I3C_ERROR_HANDLE_INVALID;
        }

        ad_i3c_cb_data_t transaction_data;
        ad_i3c_static_data_t *i3c_static = &i3c_static_data;

        I3C_MUTEX_GET(i3c_static->busy);

        /* Check if I3C hw driver is in use and already occupied */
        if (hw_i3c_is_occupied()) {
                I3C_MUTEX_PUT(i3c_static->busy);
                return AD_I3C_ERROR_CONTROLLER_BUSY;
        }

        transaction_data.i3c = i3c_static;
        transaction_data.cmd_response.response = 0;
        transaction_data.cmd_response.valid = false;

        if (hw_i3c_private_write_buf(i3c_transfer_cfg, wbuf, wlen, ad_i3c_xfer_cb, (void *)&transaction_data) < 0) {
                 I3C_MUTEX_PUT(i3c_static->busy);
                 return AD_I3C_ERROR_INVALID_INPUT_PARAM;
         }

        OS_BASE_TYPE res;
        res = OS_EVENT_WAIT(i3c_static->event, timeout);
        if (res == OS_EVENT_SIGNALED) {
                I3C_MUTEX_PUT(i3c_static->busy);

                if ((transaction_data.transfer_success == false) && transaction_data.cmd_response.valid) {
                        return transaction_data.cmd_response.response;
                }

                return AD_I3C_ERROR_NONE;
        } else {
                /* Time out occurred, abort ongoing transfer */
                if (!ad_i3c_controller_abort_transfer()) {
                        hw_i3c_reset_xfer_cb();
                        I3C_MUTEX_PUT(i3c_static->busy);
                        return AD_I3C_ERROR_CONTROLLER_ABORT_FAIL;
                }
                OS_EVENT_CHECK(i3c_static->event);
                I3C_MUTEX_PUT(i3c_static->busy);
                return AD_I3C_ERROR_TRANSFER_TIMEOUT;
        }
}

int ad_i3c_private_read(ad_i3c_handle_t handle, uint8_t *rbuf, size_t rlen, i3c_private_transfer_config *i3c_transfer_cfg, OS_TICK_TIME timeout)
{
        if (!(AD_I3C_HANDLE_IS_VALID(handle))) {
                return AD_I3C_ERROR_HANDLE_INVALID;
        }

        ad_i3c_cb_data_t transaction_data;
        ad_i3c_static_data_t *i3c_static = &i3c_static_data;

        I3C_MUTEX_GET(i3c_static->busy);

        /* Check if I3C hw driver is in use and already occupied */
        if (hw_i3c_is_occupied()) {
                I3C_MUTEX_PUT(i3c_static->busy);
                return AD_I3C_ERROR_CONTROLLER_BUSY;
        }

        transaction_data.i3c = i3c_static;
        transaction_data.cmd_response.response = 0;
        transaction_data.cmd_response.valid = false;

        if (hw_i3c_private_read_buf(i3c_transfer_cfg, rbuf, rlen, ad_i3c_xfer_cb, (void *)&transaction_data) < 0) {
                 I3C_MUTEX_PUT(i3c_static->busy);
                 return AD_I3C_ERROR_INVALID_INPUT_PARAM;
         }

        OS_BASE_TYPE res;
        res = OS_EVENT_WAIT(i3c_static->event, timeout);
        if (res == OS_EVENT_SIGNALED) {
                I3C_MUTEX_PUT(i3c_static->busy);

                if ((transaction_data.transfer_success == false) && transaction_data.cmd_response.valid) {
                        return transaction_data.cmd_response.response;
                }

                return AD_I3C_ERROR_NONE;
        } else {
                /* Time out occurred, abort ongoing transfer */
                if (!ad_i3c_controller_abort_transfer()) {
                        hw_i3c_reset_xfer_cb();
                        I3C_MUTEX_PUT(i3c_static->busy);
                        return AD_I3C_ERROR_CONTROLLER_ABORT_FAIL;
                }
                OS_EVENT_CHECK(i3c_static->event);
                I3C_MUTEX_PUT(i3c_static->busy);
                return AD_I3C_ERROR_TRANSFER_TIMEOUT;
        }
}
#endif /* CONFIG_I3C_USE_SYNC_TRANSACTIONS */

#if (CONFIG_I3C_USE_ASYNC_TRANSACTIONS == 1)
AD_I3C_ERROR ad_i3c_private_write_async(ad_i3c_handle_t handle, const uint8_t *wbuf, size_t wlen,
                                    i3c_private_transfer_config *i3c_transfer_cfg, ad_i3c_user_cb cb, void *user_data)
{
        if (!(AD_I3C_HANDLE_IS_VALID(handle))) {
                return AD_I3C_ERROR_HANDLE_INVALID;
        }
#if (CONFIG_AD_I3C_LOCKING == 1)
        ad_i3c_static_data_t *i3c_static = &i3c_static_data;
#endif

        I3C_MUTEX_GET(i3c_static->busy);

        /* Check if I3C hw driver is in use and already occupied */
        if (hw_i3c_is_occupied()) {
                I3C_MUTEX_PUT(i3c_static->busy);
                return AD_I3C_ERROR_CONTROLLER_BUSY;
        }

        if (hw_i3c_private_write_buf(i3c_transfer_cfg, wbuf, wlen, cb, user_data) < 0) {
                 I3C_MUTEX_PUT(i3c_static->busy);
                 return AD_I3C_ERROR_INVALID_INPUT_PARAM;
         }

        I3C_MUTEX_PUT(i3c_static->busy);
        return AD_I3C_ERROR_NONE;
}

AD_I3C_ERROR ad_i3c_private_read_async(ad_i3c_handle_t handle, uint8_t *rbuf, size_t rlen,
                                    i3c_private_transfer_config *i3c_transfer_cfg, ad_i3c_user_cb cb, void *user_data)
{
        if (!(AD_I3C_HANDLE_IS_VALID(handle))) {
                return AD_I3C_ERROR_HANDLE_INVALID;
        }
#if (CONFIG_AD_I3C_LOCKING == 1)
        ad_i3c_static_data_t *i3c_static = &i3c_static_data;
#endif

        I3C_MUTEX_GET(i3c_static->busy);

        /* Check if I3C hw driver is in use and already occupied */
        if (hw_i3c_is_occupied()) {
                I3C_MUTEX_PUT(i3c_static->busy);
                return AD_I3C_ERROR_CONTROLLER_BUSY;
        }

        if (hw_i3c_private_read_buf(i3c_transfer_cfg, rbuf, rlen, cb, user_data) < 0) {
                 I3C_MUTEX_PUT(i3c_static->busy);
                 return AD_I3C_ERROR_INVALID_INPUT_PARAM;
         }

        I3C_MUTEX_PUT(i3c_static->busy);
        return AD_I3C_ERROR_NONE;
}
#endif /* CONFIG_I3C_USE_ASYNC_TRANSACTIONS */

void ad_i3c_init(void)
{
#if (CONFIG_I3C_USE_SYNC_TRANSACTIONS == 1)
        OS_EVENT_CREATE(i3c_static_data.event);
#endif
        I3C_MUTEX_CREATE(i3c_static_data.busy);
}

ADAPTER_INIT(ad_i3c_adapter, ad_i3c_init);

#endif /* dg_configI3C_ADAPTER */
