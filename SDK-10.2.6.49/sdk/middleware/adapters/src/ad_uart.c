/**
 ****************************************************************************************
 *
 * @file ad_uart.c
 *
 * @brief UART adapter implementation
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUART_ADAPTER

#include "hw_uart.h"
#include "hw_dma.h"
#include "ad_uart.h"
#include "sys_power_mgr.h"
#include "sys_bsr.h"
#include "hw_sys.h"

/**
 * \def CONFIG_AD_UART_LOCKING
 *
 * \brief Controls whether UART adapter resource locking will be enabled
 *
 * By default, the UART adapter internally handles concurrent accesses to a UART controller by different masters
 * and tasks. If resource locking is disabled by setting this macro to 0, all such internal handling
 * is disabled, thus becoming the application's responsibility to handle concurrent accesses, using
 * the busy status register (BSR) driver and controlling the resource management.
 */
#ifndef CONFIG_AD_UART_LOCKING
# define CONFIG_AD_UART_LOCKING                 ( 1 )
#endif /* CONFIG_AD_UART_LOCKING */

/*
 * Resource allocation functions
 */
#if (CONFIG_AD_UART_LOCKING == 1)
# define UART_RES_TYPE_ACQUIRE(handle, res_type, timeout) \
        ad_uart_res_acquire((handle), (res_type), (timeout))
# define UART_RES_TYPE_RELEASE(handle, res_type) \
        ad_uart_res_release((handle), (res_type))

#  if SNC_PROCESSOR_BUILD
#   define UART_BSR_MASTER              SYS_BSR_MASTER_SNC
#  else
#   define UART_BSR_MASTER              SYS_BSR_MASTER_SYSCPU
#  endif /* SNC_PROCESSOR_BUILD */
#  define UART_BSR_ACQUIRE(periph_id)    sys_bsr_acquire(UART_BSR_MASTER, (periph_id))
#  define UART_BSR_RELEASE(periph_id)    sys_bsr_release(UART_BSR_MASTER, (periph_id))

# define UART_RES_ACQUIRE(resource_mask, timeout) \
        resource_acquire((resource_mask), (timeout))
# define UART_RES_RELEASE(resource_mask) \
        resource_release(resource_mask)
#else
# define UART_RES_TYPE_ACQUIRE(handle, res_type, timeout)       ({ AD_UART_ERROR_NONE; })
# define UART_RES_TYPE_RELEASE(handle, res_type)                do {} while (0)

#  define UART_BSR_ACQUIRE(periph_id)                           do {} while (0)
#  define UART_BSR_RELEASE(periph_id)                           do {} while (0)

# define UART_RES_ACQUIRE(resource_mask, timeout)               do {} while (0)
# define UART_RES_RELEASE(resource_mask)                        do {} while (0)
#endif /* CONFIG_AD_UART_LOCKING */

/*
 * Power domain enable/disable functions
 */
# if SNC_PROCESSOR_BUILD
#  define UART_PD_ENABLE()               do {} while (0)
#  define UART_PD_DISABLE()              do {} while (0)
# else
#  define UART_PD_ENABLE()               hw_sys_pd_com_enable();
#  define UART_PD_DISABLE()              hw_sys_pd_com_disable();
# endif /* SNC_PROCESSOR_BUILD */

static int ad_uart_gpio_configure(const ad_uart_controller_conf_t *ad_uart_ctrl_conf);
static void ad_uart_gpio_deconfigure(const ad_uart_controller_conf_t *ad_uart_ctrl_conf);

#define AD_UART_RX_TX_SIZE           (2)
#define AD_UART_RTSN_CTSN_SIZE       (2)

#define AD_UART_HANDLE_IS_VALID(handle) (((handle == &ad_uart_dynamic_conf_uart1) || (handle == &ad_uart_dynamic_conf_uart2) || \
                                        (handle == &ad_uart_dynamic_conf_uart3)) && (((ad_uart_data_t *)handle)->ctrl))

#if (CONFIG_AD_UART_LOCKING == 1)
static int ad_uart_res_acquire(ad_uart_handle_t handle, AD_UART_RES_TYPE res_type, uint32_t timeout);
static void ad_uart_res_release(ad_uart_handle_t handle, AD_UART_RES_TYPE res_type);
#endif /* CONFIG_AD_UART_LOCKING */

static int ad_uart_apply_controller_config(ad_uart_handle_t handle);

#if (CONFIG_UART_USE_SYNC_TRANSACTIONS == 1)
static void ad_uart_signal_event_write(void *args, uint16_t transferred);
static void ad_uart_signal_event_read(void *args, uint16_t transferred);
#endif

#if (CONFIG_UART_USE_ASYNC_TRANSACTIONS == 1)
static void ad_uart_signal_event_async_write(void *args, uint16_t transferred);
static void ad_uart_signal_event_async_read(void *args, uint16_t transferred);
#endif

static int ad_uart_gpio_config(HW_UART_ID id, const ad_uart_io_conf_t *io,
                               AD_IO_CONF_STATE state, bool is_ext_api);
static bool ad_uart_is_controller_busy(HW_UART_ID id);

static int ad_uart_gpio_config(const HW_UART_ID id, const ad_uart_io_conf_t *io,
                               AD_IO_CONF_STATE state, bool is_ext_api)
{
        uint8_t size = AD_UART_RX_TX_SIZE; /* for HW_UART1 configure only rx and tx */

        if (id == HW_UART2 || id == HW_UART3)
        {
                if (AD_IO_PIN_PORT_VALID(io->ctsn.port, io->ctsn.pin) &&
                        AD_IO_PIN_PORT_VALID(io->rtsn.port, io->rtsn.pin)) {
                        size += AD_UART_RTSN_CTSN_SIZE;
                }
        }

        if (ad_io_configure(&io->rx, size, io->voltage_level, state) != AD_IO_ERROR_NONE) {
                return AD_UART_ERROR_IO_CFG_INVALID;
        }
        if (state == AD_IO_CONF_ON) {
                ad_io_set_pad_latch(&io->rx, size, AD_IO_PAD_LATCHES_OP_ENABLE);
                if (is_ext_api) {
                        ad_io_set_pad_latch(&io->rx, size, AD_IO_PAD_LATCHES_OP_DISABLE);
                }
        } else {
                ad_io_set_pad_latch(&io->rx, size, AD_IO_PAD_LATCHES_OP_TOGGLE);
        }

        return AD_UART_ERROR_NONE;
}

static bool ad_uart_is_controller_busy(HW_UART_ID id)
{
        return (hw_uart_rx_in_progress(id) ||
                hw_uart_tx_in_progress(id) || hw_uart_is_busy(id) || !hw_uart_transmit_empty(id));
}


static int ad_uart_gpio_configure(const ad_uart_controller_conf_t *ad_uart_ctrl_conf)
{
        const ad_uart_io_conf_t *io = ad_uart_ctrl_conf->io;
        HW_UART_ID id = ad_uart_ctrl_conf->id;
        bool auto_flow_control = ad_uart_ctrl_conf->drv->hw_conf.auto_flow_control;
        HW_GPIO_PORT ctsn_port = io->ctsn.port;
        HW_GPIO_PIN ctsn_pin = io->ctsn.pin;
        HW_GPIO_PORT rtsn_port = io->rtsn.port;
        HW_GPIO_PIN rtsn_pin = io->rtsn.pin;

        int ret = AD_UART_ERROR_NONE;

        /* Sanity checks */

        if (id == HW_UART1) {
                if (auto_flow_control) {
                        OS_ASSERT(0);
                        ret = AD_UART_ERROR_GPIO_CONF_INVALID;
                }
        } else if ((id == HW_UART2) || (id == HW_UART3)) {
                if (auto_flow_control && !AD_IO_PIN_PORT_VALID(ctsn_port, ctsn_pin)) {
                        OS_ASSERT(0);
                        ret = AD_UART_ERROR_GPIO_CONF_INVALID;
                }
                if (auto_flow_control && !AD_IO_PIN_PORT_VALID(rtsn_port, rtsn_pin)) {
                        OS_ASSERT(0);
                        ret = AD_UART_ERROR_GPIO_CONF_INVALID;
                }
        }
        else {
                OS_ASSERT(0);
                ret = AD_UART_ERROR_CONTROLLER_CONF_INVALID;
        }

        ad_uart_gpio_config(id, io, AD_IO_CONF_ON, false);

        return ret;
}

static void ad_uart_gpio_deconfigure(const ad_uart_controller_conf_t *ad_uart_ctrl_conf)
{
        const ad_uart_io_conf_t *io = ad_uart_ctrl_conf->io;
        HW_UART_ID id = ad_uart_ctrl_conf->id;

        ad_uart_gpio_config(id, io, AD_IO_CONF_OFF, false);
}


#if (CONFIG_UART_USE_SYNC_TRANSACTIONS == 1)
/**
 * \brief UART adapter events
 */
typedef struct {
        OS_EVENT event_write;                   /* Event used for synchronization in accessing UART controller for sending data. */
        OS_EVENT event_read;                    /* Event used for synchronization in accessing UART controller for receiving data. */
} ad_uart_events_t;
#endif /* CONFIG_UART_USE_SYNC_TRANSACTIONS */

/**
 * \brief UART adapter dynamic data
 */
typedef struct {
#if (CONFIG_AD_UART_LOCKING == 1)
        struct {
                OS_TASK owner;                  /* Task that acquired this resource */
                int8_t acquire_count;
        } res_states[AD_UART_RES_TYPES];        /* This keeps track of number of acquisitions for this resource */
#endif /* CONFIG_AD_UART_LOCKING */

        int8_t open_count;                      /* Reference counter incremented in ad_uart_open(), decremented in ad_uart_close()*/
#if (CONFIG_UART_USE_ASYNC_TRANSACTIONS == 1)
        ad_uart_user_cb read_cb;                /* User function to call after asynchronous read finishes */
        ad_uart_user_cb write_cb;               /* User function to call after asynchronous write finishes */
        void *read_cb_data;                     /* Data to pass to read_cb */
        void *write_cb_data;                    /* Data to pass to write_cb */
#endif /* CONFIG_UART_USE_ASYNC_TRANSACTIONS */
#if (dg_configUART_RX_CIRCULAR_DMA == 1)
        bool use_rx_circular_dma;               /* true if UART is using circular DMA on RX */
#if (CONFIG_UART_USE_ASYNC_TRANSACTIONS == 1)
        void *read_cb_ptr;                      /* original pointer passed to read, used only with circular DMA */
#endif /* CONFIG_UART_USE_ASYNC_TRANSACTIONS */
#endif /* dg_configUART_RX_CIRCULAR_DMA */
        const ad_uart_controller_conf_t *ctrl;  /* Pointer at the controller structure passed in ad_uart_open() */
} ad_uart_data_t;

#if (CONFIG_UART_USE_SYNC_TRANSACTIONS == 1)
typedef struct {
        ad_uart_data_t *ad_uart_data;
        uint16_t transferred;
} ad_uart_cb_data_t;

__RETAINED static ad_uart_events_t ad_uart_events1;
__RETAINED static ad_uart_events_t ad_uart_events2;
__RETAINED static ad_uart_events_t ad_uart_events3;
#endif /* CONFIG_UART_USE_SYNC_TRANSACTIONS */

static ad_uart_data_t ad_uart_dynamic_conf_uart1;
static ad_uart_data_t ad_uart_dynamic_conf_uart2;
static ad_uart_data_t ad_uart_dynamic_conf_uart3;

#if (CONFIG_UART_USE_SYNC_TRANSACTIONS == 1)
__STATIC_INLINE ad_uart_events_t* ad_uart_get_events_by_hw_id(const HW_UART_ID id)
{
        return ((id == HW_UART1) ? &ad_uart_events1 : ((id == HW_UART2) ? &ad_uart_events2 : &ad_uart_events3));
}
#endif /* CONFIG_UART_USE_SYNC_TRANSACTIONS */

__STATIC_INLINE resource_mask_t uart_resource_mask(const HW_UART_ID id)
{
        return (1 << (id == HW_UART1 ? RES_ID_UART1 : (id == HW_UART2 ? RES_ID_UART2 : RES_ID_UART3)));
}

#if (HW_UART_DMA_SUPPORT == 1)
__STATIC_INLINE resource_mask_t dma_resource_mask(int num)
{
        const resource_mask_t res_mask[] = {
                RES_MASK(RES_ID_DMA_CH0),
                RES_MASK(RES_ID_DMA_CH1),
                RES_MASK(RES_ID_DMA_CH2),
                RES_MASK(RES_ID_DMA_CH3),
                RES_MASK(RES_ID_DMA_CH4),
                RES_MASK(RES_ID_DMA_CH5),
                RES_MASK(RES_ID_DMA_CH6),
                RES_MASK(RES_ID_DMA_CH7),
        };
        if (num < HW_DMA_CHANNEL_INVALID) {
                return res_mask[num];
        } else {
                return 0;
        }
}
#endif /* HW_UART_DMA_SUPPORT */

#if (CONFIG_AD_UART_LOCKING == 1)
static int ad_uart_res_acquire(ad_uart_handle_t handle, AD_UART_RES_TYPE res_type, uint32_t timeout)
{
        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;
        OS_TASK *owner = &ad_uart_data->res_states[res_type].owner;
        int8_t *acquire_count = &ad_uart_data->res_states[res_type].acquire_count;
        OS_TASK current_task = OS_GET_CURRENT_TASK();

        if (timeout == RES_WAIT_FOREVER) {

                if (current_task == *owner) {
                        (*acquire_count)++;
                        return AD_UART_ERROR_NONE;
                }
        }

        uint32_t resource_mask = 0;

        switch (res_type) {
        case AD_UART_RES_TYPE_CONFIG:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_CONFIG : (id == HW_UART2 ? RES_ID_UART2_CONFIG : RES_ID_UART3_CONFIG));
                break;
        case AD_UART_RES_TYPE_WRITE:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_WRITE : (id == HW_UART2 ? RES_ID_UART2_WRITE : RES_ID_UART3_WRITE));
                break;
        case AD_UART_RES_TYPE_READ:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_READ : (id == HW_UART2 ? RES_ID_UART2_READ : RES_ID_UART3_READ));
                break;
        default:
                /* Invalid argument. */
                OS_ASSERT(0);

        }

        if (resource_acquire(resource_mask, timeout)) {
                *owner = current_task;
                (*acquire_count)++;
                return AD_UART_ERROR_NONE;
        } else {
                return AD_UART_ERROR_RESOURCE_NOT_AVAILABLE;
        }

}

static void ad_uart_res_release(ad_uart_handle_t handle, AD_UART_RES_TYPE res_type)
{

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;
        OS_TASK *owner = &ad_uart_data->res_states[res_type].owner;
        int8_t *acquire_count = &ad_uart_data->res_states[res_type].acquire_count;

        /* A device release can only happen from the same task that owns it, or from an ISR */
        OS_ASSERT(in_interrupt() ||
                (OS_GET_CURRENT_TASK() == *owner));

        if (--(*acquire_count) == 0) {
                *owner = NULL;

                uint32_t resource_mask = 0;

                switch (res_type) {
                case AD_UART_RES_TYPE_CONFIG:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_CONFIG : (id == HW_UART2 ? RES_ID_UART2_CONFIG : RES_ID_UART3_CONFIG));
                break;
        case AD_UART_RES_TYPE_WRITE:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_WRITE : (id == HW_UART2 ? RES_ID_UART2_WRITE : RES_ID_UART3_WRITE));
                break;
        case AD_UART_RES_TYPE_READ:
                resource_mask = 1 << (id == HW_UART1 ? RES_ID_UART1_READ : (id == HW_UART2 ? RES_ID_UART2_READ : RES_ID_UART3_READ));
                break;
                default:
                        /* Invalid argument. */
                        OS_ASSERT(0);
                }
                resource_release(resource_mask);
        }
}
#endif /* CONFIG_AD_UART_LOCKING */

static int ad_uart_apply_controller_config(ad_uart_handle_t handle)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;
        const uart_config_ex *drv = &ad_uart_data->ctrl->drv->hw_conf;
#if (dg_configUART_RX_CIRCULAR_DMA == 1)
        bool *use_rx_circular_dma;
#endif
        int ret;

        UART_RES_TYPE_ACQUIRE(handle, AD_UART_RES_TYPE_CONFIG, RES_WAIT_FOREVER);

        /* Check ad_uart_close() for being faster */
        if (!ad_uart_data->open_count) {
                OS_ASSERT(0);
                UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_CONFIG);
                return AD_UART_ERROR_DEVICE_CLOSED;
        }

        if (hw_uart_init_ex(id, drv) == HW_UART_CONFIG_ERR_NOERR) {
                ret = AD_UART_ERROR_NONE;
        } else {
                OS_ASSERT(0);
                UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_CONFIG);
                return AD_UART_ERROR_CONTROLLER_CONF_INVALID;
        }

#if (dg_configUART_RX_CIRCULAR_DMA == 1)
        use_rx_circular_dma = &ad_uart_data->use_rx_circular_dma;
        /*
         * If circular DMA or RX is enabled on UART, we automatically use it in adapter. However,
         * it can be enabled separately for each UART so we need to check this and configure adapter
         * in runtime appropriately.
         */

        if (((id == HW_UART1) && (dg_configUART1_RX_CIRCULAR_DMA_BUF_SIZE > 0)) ||
                ((id == HW_UART2) && (dg_configUART2_RX_CIRCULAR_DMA_BUF_SIZE > 0))) {
                *use_rx_circular_dma = true;
                hw_uart_enable_rx_circular_dma(id);
        }
        else if ((id == HW_UART3) && (dg_configUART3_RX_CIRCULAR_DMA_BUF_SIZE > 0)) {
                *use_rx_circular_dma = true;
                hw_uart_enable_rx_circular_dma(id);
        }
#endif /* dg_configUART_RX_CIRCULAR_DMA */

        UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_CONFIG);
        return ret;
}


ad_uart_handle_t ad_uart_open(const ad_uart_controller_conf_t *ad_uart_ctrl_conf)
{
        OS_ASSERT(ad_uart_ctrl_conf &&
                  ad_uart_ctrl_conf->drv && ad_uart_ctrl_conf->io && ad_uart_ctrl_conf->id);

        ad_uart_data_t *ret = NULL;
        HW_UART_ID id = ad_uart_ctrl_conf->id;
        __UNUSED uint32_t resource_mask = uart_resource_mask(id);
#if (HW_UART_DMA_SUPPORT == 1)
        bool use_dma = ad_uart_ctrl_conf->drv->hw_conf.use_dma;
        HW_DMA_CHANNEL rx_dma_channel = ad_uart_ctrl_conf->drv->hw_conf.rx_dma_channel;
        HW_DMA_CHANNEL tx_dma_channel = ad_uart_ctrl_conf->drv->hw_conf.tx_dma_channel;
#endif /* HW_UART_DMA_SUPPORT */

        pm_sleep_mode_request(pm_mode_idle);

        /* Start acquiring resources */

        /* Arbitrate on multiple masters. */
        UART_BSR_ACQUIRE(id == HW_UART1 ? SYS_BSR_PERIPH_ID_UART1 : (id == HW_UART2 ? SYS_BSR_PERIPH_ID_UART2 : SYS_BSR_PERIPH_ID_UART3));

#if (HW_UART_DMA_SUPPORT == 1)
        if (use_dma) {
                resource_mask |= dma_resource_mask(rx_dma_channel);
                resource_mask |= dma_resource_mask(tx_dma_channel);
        }
#endif /* HW_UART_DMA_SUPPORT */

        /* Arbitrate on multiple tasks. */
        UART_RES_ACQUIRE(resource_mask, RES_WAIT_FOREVER);

        UART_PD_ENABLE();

        /* Apply I/O configuration. */
        ad_uart_gpio_configure(ad_uart_ctrl_conf);

        /* Handle dynamic data */

        if (id == HW_UART1) {
                /* Bind controller configuration to the dynamic data. */
                ad_uart_dynamic_conf_uart1.ctrl = ad_uart_ctrl_conf;
                OS_ASSERT(ad_uart_dynamic_conf_uart1.open_count++ == 0);
                ret =  &ad_uart_dynamic_conf_uart1;
        } else if (id == HW_UART2) {
                /* Bind controller configuration to the dynamic data. */
                ad_uart_dynamic_conf_uart2.ctrl = ad_uart_ctrl_conf;
                OS_ASSERT(ad_uart_dynamic_conf_uart2.open_count++ == 0);
                ret =  &ad_uart_dynamic_conf_uart2;
        }
        else {
               /* Bind controller configuration to the dynamic data. */
               ad_uart_dynamic_conf_uart3.ctrl = ad_uart_ctrl_conf;
               OS_ASSERT(ad_uart_dynamic_conf_uart3.open_count++ == 0);
               ret =  &ad_uart_dynamic_conf_uart3;
       }

        /* Apply configuration */
        if (ad_uart_apply_controller_config(ret)) {
                /* Apply I/O de-configuration. */
                ad_uart_gpio_deconfigure(ad_uart_ctrl_conf);
                UART_PD_DISABLE();
                /* Handle dynamic data */
                /* Decrease the open_count so the UART could be re-opened
                 * with a correct configuration.
                 * Normally this would be done in ad_uart_close()
                 * but since there will be no handler we cannot use it
                 * and we have to take care of it here */
                OS_ASSERT(ret->open_count-- == 1);
                /* Unbind controller configuration from dynamic data. */
                ret->ctrl = NULL;
                /* From now on dynamic data are invalidated. */

                UART_RES_RELEASE(resource_mask);

                /* Arbitrate on multiple masters. */
                UART_BSR_RELEASE(id == HW_UART1 ? SYS_BSR_PERIPH_ID_UART1 :
                                (id == HW_UART2 ? SYS_BSR_PERIPH_ID_UART2 : SYS_BSR_PERIPH_ID_UART3));
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }

        return ret;
}

int ad_uart_close(ad_uart_handle_t handle, bool force)

{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;

        HW_UART_ID id = ad_uart_data->ctrl->id;
        __UNUSED uint32_t resource_mask = uart_resource_mask(id);
#if (HW_UART_DMA_SUPPORT == 1)
        bool use_dma = ad_uart_data->ctrl->drv->hw_conf.use_dma;
        HW_DMA_CHANNEL rx_dma_channel = ad_uart_data->ctrl->drv->hw_conf.rx_dma_channel;
        HW_DMA_CHANNEL tx_dma_channel = ad_uart_data->ctrl->drv->hw_conf.tx_dma_channel;
#endif /* HW_UART_DMA_SUPPORT */

        OS_ENTER_CRITICAL_SECTION();

        if (!force) {
                if (ad_uart_is_controller_busy(id)
#if dg_configUART_RX_CIRCULAR_DMA
                    || ad_uart_data->use_rx_circular_dma
#endif
                   ) {
                        OS_LEAVE_CRITICAL_SECTION();
                        return AD_UART_ERROR_CONTROLLER_BUSY;
                }
        } else {
                hw_uart_abort_receive(id);
                hw_uart_abort_send(id);
        }

        hw_uart_deinit(id);

        OS_LEAVE_CRITICAL_SECTION();


        const ad_uart_controller_conf_t *ad_uart_ctrl_conf = ad_uart_data->ctrl;
        /* Apply I/O de-configuration. */
        ad_uart_gpio_deconfigure(ad_uart_ctrl_conf);
        /* Handle dynamic data */

        OS_ASSERT(ad_uart_data->open_count-- == 1);
        /* Unbind controller configuration from dynamic data. */
        ad_uart_data->ctrl = NULL;
        /* From now on dynamic data are invalidated. */

        UART_PD_DISABLE();

        /* Start releasing resources. */

#if (HW_UART_DMA_SUPPORT == 1)
        if (use_dma) {
                resource_mask |= dma_resource_mask(rx_dma_channel);
                resource_mask |= dma_resource_mask(tx_dma_channel);
        }
#endif /* HW_UART_DMA_SUPPORT */
        UART_RES_RELEASE(resource_mask);

        UART_BSR_RELEASE(id == HW_UART1 ? SYS_BSR_PERIPH_ID_UART1 : (id == HW_UART2 ? SYS_BSR_PERIPH_ID_UART2 : SYS_BSR_PERIPH_ID_UART3));

        pm_sleep_mode_release(pm_mode_idle);

        return AD_UART_ERROR_NONE;
}

int ad_uart_reconfig(ad_uart_handle_t handle, const ad_uart_driver_conf_t *ad_drv)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        int ret;
        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;
        const uart_config_ex *drv = &ad_drv->hw_conf;
#if (HW_UART_DMA_SUPPORT == 1)
        HW_DMA_CHANNEL current_tx_dma_channel = ad_uart_data->ctrl->drv->hw_conf.tx_dma_channel;
        HW_DMA_CHANNEL current_rx_dma_channel = ad_uart_data->ctrl->drv->hw_conf.rx_dma_channel;
        HW_DMA_CHANNEL new_tx_dma_channel = drv->tx_dma_channel;
        HW_DMA_CHANNEL new_rx_dma_channel = drv->rx_dma_channel;

        /* Sanity checks */
        if (new_tx_dma_channel != current_tx_dma_channel) {
                OS_ASSERT(0);
                return AD_UART_ERROR_CONTROLLER_CONF_INVALID;
        }

        if (new_rx_dma_channel != current_rx_dma_channel) {
                OS_ASSERT(0);
                return AD_UART_ERROR_CONTROLLER_CONF_INVALID;
        }
#endif /* HW_UART_DMA_SUPPORT */

        UART_RES_TYPE_ACQUIRE(handle, AD_UART_RES_TYPE_CONFIG, RES_WAIT_FOREVER);

        /* Check ad_uart_close() for being faster */
        if (!ad_uart_data->open_count) {
                OS_ASSERT(0);
                UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_CONFIG);
                return AD_UART_ERROR_DEVICE_CLOSED;
        }

        if (ad_uart_is_controller_busy(id)) {
                OS_ASSERT(0);
                UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_CONFIG);
                return AD_UART_ERROR_CONTROLLER_BUSY;
        }

        if (hw_uart_init_ex(id, drv) == HW_UART_CONFIG_ERR_NOERR) {
                ret = AD_UART_ERROR_NONE;
        } else {
                OS_ASSERT(0);
                ret = AD_UART_ERROR_CONTROLLER_CONF_INVALID;
        }

        UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_CONFIG);

        return ret;
}

#if (CONFIG_UART_USE_SYNC_TRANSACTIONS == 1)

static void ad_uart_signal_event_write(void *args, uint16_t transferred)
{
        ad_uart_cb_data_t *cb_data = (ad_uart_cb_data_t *) args;
        cb_data->transferred = transferred;
        OS_EVENT_SIGNAL_FROM_ISR(ad_uart_get_events_by_hw_id(cb_data->ad_uart_data->ctrl->id)->event_write);
}

int ad_uart_write(ad_uart_handle_t handle, const char *wbuf, size_t wlen)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        ad_uart_cb_data_t cb_data = {ad_uart_data, 0};

        UART_RES_TYPE_ACQUIRE(handle, AD_UART_RES_TYPE_WRITE, RES_WAIT_FOREVER);

        /* Check ad_uart_close() for being faster */
        if (!ad_uart_data->open_count) {
                OS_ASSERT(0);
                UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_WRITE);
                return AD_UART_ERROR_DEVICE_CLOSED;
        }

        hw_uart_send(id, (const uint8_t *) wbuf, wlen, ad_uart_signal_event_write, &cb_data);
        OS_EVENT_WAIT(ad_uart_get_events_by_hw_id(id)->event_write, RES_WAIT_FOREVER);

        UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_WRITE);

        return AD_UART_ERROR_NONE;
}

int ad_uart_complete_async_read(ad_uart_handle_t handle)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

#if dg_configUART_RX_CIRCULAR_DMA
        if (ad_uart_data->use_rx_circular_dma) {
                return hw_uart_copy_dma_rx_to_user_buffer(id);
        }
#endif
        /* Force callback */
        return hw_uart_abort_receive(id);
}

static void ad_uart_signal_event_read(void *args, uint16_t transferred)
{
        ad_uart_cb_data_t *cb_data = (ad_uart_cb_data_t *) args;
        cb_data->transferred = transferred;
        OS_EVENT event_read = ad_uart_get_events_by_hw_id(cb_data->ad_uart_data->ctrl->id)->event_read;
        /* The callback might also get called directly by:
         * - hw_uart_abort_receive(), or
         * - hw_uart_receive() (in case the data are available on the circular buffer),
         * so not necessarily in interrupt context, so we better handle both context cases.
         */
        if (in_interrupt()) {
                OS_EVENT_SIGNAL_FROM_ISR(event_read);
        } else {
                OS_EVENT_SIGNAL(event_read);
        }
}

int ad_uart_read(ad_uart_handle_t handle, char *rbuf, size_t rlen, OS_TICK_TIME timeout)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        ad_uart_cb_data_t cb_data = {ad_uart_data, 0};

        UART_RES_TYPE_ACQUIRE(handle, AD_UART_RES_TYPE_READ, RES_WAIT_FOREVER);

        /* Check ad_uart_close() for being faster */
        if (!ad_uart_data->open_count) {
                OS_ASSERT(0);
                UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_READ);
                return AD_UART_ERROR_DEVICE_CLOSED;
        }

        /* If there is a pending read event clear it out.
         * This may occur while waiting to take the event_read semaphore and
         * the configured timeout expires.
         */
        OS_EVENT event_read = ad_uart_get_events_by_hw_id(id)->event_read;
        OS_EVENT_CHECK(event_read);

        hw_uart_receive(id, (uint8_t *) rbuf, rlen, ad_uart_signal_event_read, &cb_data);
        /* Wait for receiving the read event */
        OS_EVENT_WAIT(event_read, timeout);
        /* Needs to be called to cover the following cases:
         * 1. A circular DMA is used, the data from the circular
         *     buffer will be copied to the application buffer.
         * 2. A timeout occurs and we need to abort gracefully.
         */
        hw_uart_abort_receive(id);

        UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_READ);

        return cb_data.transferred;
}
#endif /* CONFIG_UART_USE_SYNC_TRANSACTIONS */

#if (CONFIG_UART_USE_ASYNC_TRANSACTIONS == 1)

static void ad_uart_signal_event_async_write(void *args, uint16_t transferred)
{
        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *) args;
        if (ad_uart_data->write_cb) {
                ad_uart_data->write_cb(ad_uart_data->write_cb_data, transferred);
        }
        UART_RES_TYPE_RELEASE(ad_uart_data, AD_UART_RES_TYPE_WRITE);
}

int ad_uart_write_async(ad_uart_handle_t handle, const char *wbuf, size_t wlen,
                           ad_uart_user_cb cb, void *user_data)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        if (UART_RES_TYPE_ACQUIRE(handle, AD_UART_RES_TYPE_WRITE, 0) == AD_UART_ERROR_NONE) {
                /* Check ad_uart_close() for being faster */
                if (!ad_uart_data->open_count) {
                        OS_ASSERT(0);
                        UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_WRITE);
                        return AD_UART_ERROR_DEVICE_CLOSED;
                }

                ad_uart_data->write_cb = cb;
                ad_uart_data->write_cb_data = user_data;

                hw_uart_send(id, (const uint8_t *) wbuf, wlen, ad_uart_signal_event_async_write, ad_uart_data);

                return AD_UART_ERROR_NONE;
        } else {
                return AD_UART_ERROR_RESOURCE_NOT_AVAILABLE;
        }
}

static void ad_uart_signal_event_async_read(void *args, uint16_t transferred)
{
        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *) args;
#if dg_configUART_RX_CIRCULAR_DMA
        if (ad_uart_data->use_rx_circular_dma) {
                hw_uart_copy_dma_rx_to_user_buffer(ad_uart_data->ctrl->id);
        }
#endif
        if (ad_uart_data->read_cb) {
                ad_uart_data->read_cb(ad_uart_data->read_cb_data, transferred);
        }
        UART_RES_TYPE_RELEASE(ad_uart_data, AD_UART_RES_TYPE_READ);
}

int ad_uart_read_async(ad_uart_handle_t handle, char *rbuf, size_t rlen, ad_uart_user_cb cb,
                         void *user_data)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        if (UART_RES_TYPE_ACQUIRE(handle, AD_UART_RES_TYPE_READ, 0) == AD_UART_ERROR_NONE) {
                /* Check ad_uart_close() for being faster */
                if (!ad_uart_data->open_count) {
                        OS_ASSERT(0);
                        UART_RES_TYPE_RELEASE(handle, AD_UART_RES_TYPE_READ);
                        return AD_UART_ERROR_DEVICE_CLOSED;
                }

                ad_uart_data->read_cb = cb;
                ad_uart_data->read_cb_data = user_data;

#if (dg_configUART_RX_CIRCULAR_DMA == 1)
                ad_uart_data->read_cb_ptr = rbuf;
#endif /* dg_configUART_RX_CIRCULAR_DMA */

                hw_uart_receive(id, (uint8_t *) rbuf, rlen, ad_uart_signal_event_async_read, ad_uart_data);

                return AD_UART_ERROR_NONE;
        } else {
                return AD_UART_ERROR_RESOURCE_NOT_AVAILABLE;
        }
}

int ad_uart_complete_async_write(ad_uart_handle_t handle)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        /* Force callback */
        return hw_uart_abort_send(id);
}
#endif /* CONFIG_UART_USE_ASYNC_TRANSACTIONS */

HW_UART_ID ad_uart_get_hw_uart_id(ad_uart_handle_t handle)
{
        OS_ASSERT(AD_UART_HANDLE_IS_VALID(handle));

        ad_uart_data_t *ad_uart_data = (ad_uart_data_t *)handle;
        HW_UART_ID id = ad_uart_data->ctrl->id;

        return id;
}

int ad_uart_io_config(HW_UART_ID id, const ad_uart_io_conf_t *io, AD_IO_CONF_STATE state)
{
        int ret;
        ret = ad_uart_gpio_config(id, io, state, true);

        return ret;
}


void ad_uart_init(void)
{
#if (CONFIG_UART_USE_SYNC_TRANSACTIONS == 1)
        OS_EVENT_CREATE(ad_uart_events1.event_write);
        OS_EVENT_CREATE(ad_uart_events1.event_read);
        OS_EVENT_CREATE(ad_uart_events2.event_write);
        OS_EVENT_CREATE(ad_uart_events2.event_read);
        OS_EVENT_CREATE(ad_uart_events3.event_write);
        OS_EVENT_CREATE(ad_uart_events3.event_read);
#endif /* CONFIG_UART_USE_SYNC_TRANSACTIONS */
}

ADAPTER_INIT(ad_uart_adapter, ad_uart_init)

#endif /* dg_configUART_ADAPTER */
