/**
 ****************************************************************************************
 *
 * @file hw_uart.c
 *
 * @brief Implementation of the UART Low Level Driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_UART


#include <stdint.h>
#include <string.h>
#include "hw_uart.h"

#include "hw_pd.h"
#include "hw_clk.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#if (dg_configUART1_SOFTWARE_FIFO_SIZE && (dg_configUART_SOFTWARE_FIFO == 1))
#if (dg_configUART_RX_CIRCULAR_DMA == 1) && (dg_configUART1_RX_CIRCULAR_DMA_BUF_SIZE > 0)
#error UART1 can not be configured to use software FIFO and circular DMA FIFO at the same time
#endif
__RETAINED static uint8_t uart1_sw_fifo[dg_configUART1_SOFTWARE_FIFO_SIZE];
#else
#define uart1_sw_fifo   (NULL)
#endif

#if (dg_configUART2_SOFTWARE_FIFO_SIZE && (dg_configUART_SOFTWARE_FIFO == 1))
#if (dg_configUART_RX_CIRCULAR_DMA == 1) && (dg_configUART2_RX_CIRCULAR_DMA_BUF_SIZE > 0)
#error UART2 can not be configured to use software FIFO and circular DMA FIFO at the same time
#endif
__RETAINED static uint8_t uart2_sw_fifo[dg_configUART2_SOFTWARE_FIFO_SIZE];
#else
#define uart2_sw_fifo   (NULL)
#endif

#if (dg_configUART3_SOFTWARE_FIFO_SIZE && (dg_configUART_SOFTWARE_FIFO == 1))
#if (dg_configUART_RX_CIRCULAR_DMA == 1) && (dg_configUART3_RX_CIRCULAR_DMA_BUF_SIZE > 0)
#error UART3 can not be configured to use software FIFO and circular DMA FIFO at the same time
#endif
__RETAINED static uint8_t uart3_sw_fifo[dg_configUART3_SOFTWARE_FIFO_SIZE];
#else
#define uart3_sw_fifo   (NULL)
#endif

typedef uint16_t fifo_size_t;
typedef uint16_t fifo_ptr_t;

#if (dg_configUART_RX_CIRCULAR_DMA == 1)

#if dg_configUART1_RX_CIRCULAR_DMA_BUF_SIZE > 0
__RETAINED static uint8_t uart1_rx_dma_buf[dg_configUART1_RX_CIRCULAR_DMA_BUF_SIZE];
#else
#define uart1_rx_dma_buf (NULL)
#endif

#if dg_configUART2_RX_CIRCULAR_DMA_BUF_SIZE > 0
__RETAINED static uint8_t uart2_rx_dma_buf[dg_configUART2_RX_CIRCULAR_DMA_BUF_SIZE];
#else
#define uart2_rx_dma_buf (NULL)
#endif

#if dg_configUART3_RX_CIRCULAR_DMA_BUF_SIZE > 0
__RETAINED static uint8_t uart3_rx_dma_buf[dg_configUART3_RX_CIRCULAR_DMA_BUF_SIZE];
#else
#define uart3_rx_dma_buf (NULL)
#endif

#endif /* dg_configUART_RX_CIRCULAR_DMA */

#if (HW_UART_DMA_SUPPORT == 1)
#define HW_UART_DEFAULT_DMA_RX_PRIO     (HW_DMA_PRIO_2)
#define HW_UART_DEFAULT_DMA_TX_PRIO     (HW_DMA_PRIO_2)
#endif

typedef struct {
#ifdef HW_UART_ENABLE_USER_ISR
        hw_uart_interrupt_isr user_isr;
#endif
        const uint8_t           *tx_buffer;
        void                    *tx_user_data;
        hw_uart_tx_callback     tx_cb;
        uint16_t                tx_len;
        uint16_t                tx_ix;

        void                    *rx_user_data;
        uint8_t                 *rx_buffer;
        hw_uart_rx_callback     rx_cb;
        uint16_t                rx_len;
        uint16_t                rx_ix;

        uint8_t                 tx_fifo_on:1;
        uint8_t                 rx_fifo_on:1;
        uint8_t                 tx_fifo_level:2;
        uint8_t                 rx_fifo_level:2;

        hw_uart_err_callback    err_cb;
        void                    *err_user_data;
#if (dg_configUART_SOFTWARE_FIFO == 1)
        uint8_t                 *rx_soft_fifo;
        fifo_size_t             rx_soft_fifo_size;
        fifo_ptr_t              rx_soft_fifo_rd_ptr;
        fifo_ptr_t              rx_soft_fifo_wr_ptr;
#endif /* dg_configUART_SOFTWARE_FIFO */
#if (HW_UART_DMA_SUPPORT == 1)
        uint8_t                 use_dma:1;
        DMA_setup               tx_dma;
        DMA_setup               rx_dma;
#if (dg_configUART_RX_CIRCULAR_DMA == 1)
        bool                    rx_dma_active;
        uint8_t                 *rx_dma_buf;
        uint16_t                rx_dma_buf_size;
        uint16_t                rx_dma_head;
#endif /* dg_configUART_RX_CIRCULAR_DMA */
#endif /* HW_UART_DMA_SUPPORT */
} UART_Data;

__RETAINED_RW static UART_Data uart_data[] = {
        {
#if (dg_configUART_SOFTWARE_FIFO == 1)
                .rx_soft_fifo = uart1_sw_fifo,
                .rx_soft_fifo_size = dg_configUART1_SOFTWARE_FIFO_SIZE,
#endif
#if (dg_configUART_RX_CIRCULAR_DMA == 1)
                .rx_dma_buf = uart1_rx_dma_buf,
                .rx_dma_buf_size = dg_configUART1_RX_CIRCULAR_DMA_BUF_SIZE,
#endif
        },
        {
#if (dg_configUART_SOFTWARE_FIFO == 1)
                .rx_soft_fifo = uart2_sw_fifo,
                .rx_soft_fifo_size = dg_configUART2_SOFTWARE_FIFO_SIZE,
#endif
#if (dg_configUART_RX_CIRCULAR_DMA == 1)
                .rx_dma_buf = uart2_rx_dma_buf,
                .rx_dma_buf_size = dg_configUART2_RX_CIRCULAR_DMA_BUF_SIZE,
#endif
        },
        {
#if (dg_configUART_SOFTWARE_FIFO == 1)
                .rx_soft_fifo = uart3_sw_fifo,
                .rx_soft_fifo_size = dg_configUART3_SOFTWARE_FIFO_SIZE,
#endif
#if (dg_configUART_RX_CIRCULAR_DMA == 1)
                .rx_dma_buf = uart3_rx_dma_buf,
                .rx_dma_buf_size = dg_configUART3_RX_CIRCULAR_DMA_BUF_SIZE,
#endif
        },
};

#define UART_INT(id) ((id) == HW_UART1 ? (UART_IRQn) : ((id) == HW_UART2 ? (UART2_IRQn) : (UART3_IRQn)))
#define UARTIX(id) ((id) == HW_UART1 ? 0 : ((id) == HW_UART2 ? 1 : 2))
#define UARTID(ud) ((ud) == uart_data ? HW_UART1 : ((ud) == &uart_data[1] ? HW_UART2 : HW_UART3))
#define UARTDATA(id) (&uart_data[UARTIX(id)])

#ifdef HW_UART_ENABLE_USER_ISR
void hw_uart_set_isr(HW_UART_ID uart, hw_uart_interrupt_isr isr)
{
        uart_data[UARTIX(uart)].user_isr = isr;
}
#endif

//===================== Read/Write functions ===================================

uint8_t hw_uart_read(HW_UART_ID uart)
{
        // Wait until received data are available
        while (hw_uart_read_buf_empty(uart));

        // Read element from the receive FIFO
        return UBA(uart)->UART2_RBR_THR_DLL_REG;
}

void hw_uart_write(HW_UART_ID uart, uint8_t data)
{
        // Wait if Transmit Holding Register is full
        while (hw_uart_write_buf_full(uart));

        // Write data to the transmit FIFO
        UBA(uart)->UART2_RBR_THR_DLL_REG = data;
}

void hw_uart_write_buffer(HW_UART_ID uart, const void *data, uint16_t len)
{
        const uint8_t *p = data;

        while (len > 0) {
                hw_uart_write(uart, *p++);
                len--;
        }
}


__STATIC_INLINE void hw_uart_enable_rx_int(HW_UART_ID uart, bool enable)
{
        /* The process of updating IER_DLH_REG should be atomic in order to be done properly. Since
         * this function is called by various other functions that can be called from both within
         * and out of interrupt context, we better avoid the possibility of concurrent attempts to
         * update this register by always temporarily disabling interrupts. */

        GLOBAL_INT_DISABLE();
        HW_UART_REG_SETF(uart, IER_DLH, ERBFI_DLH0, enable);
        GLOBAL_INT_RESTORE();

        NVIC_EnableIRQ(UART_INT(uart));
}

__STATIC_INLINE void hw_uart_enable_tx_int(HW_UART_ID uart, bool enable)
{
        GLOBAL_INT_DISABLE();

        uint16_t ier_dlh_reg = UBA(uart)->UART2_IER_DLH_REG;
        REG_SET_FIELD(UART2, UART2_IER_DLH_REG, ETBEI_DLH1, ier_dlh_reg, enable);
        REG_SET_FIELD(UART2, UART2_IER_DLH_REG, PTIME_DLH7, ier_dlh_reg, enable);
        UBA(uart)->UART2_IER_DLH_REG = ier_dlh_reg;

        GLOBAL_INT_RESTORE();

        NVIC_EnableIRQ(UART_INT(uart));
}


HW_UART_CONFIG_ERR hw_uart_send(HW_UART_ID uart, const void *data, uint16_t len, hw_uart_tx_callback cb,
                                                                                void *user_data)
{
        UART_Data *ud = &uart_data[UARTIX(uart)];

        if (cb == NULL) {
                hw_uart_write_buffer(uart, data, len);
                ud->tx_ix = 0;
                ud->tx_len = 0;
                return HW_UART_CONFIG_ERR_NOERR;
        }
        ud->tx_buffer = data;
        ud->tx_user_data = user_data;
        ud->tx_len = len;
        ud->tx_ix = 0;
        ud->tx_cb = cb;

#if (HW_UART_DMA_SUPPORT == 1)
        if (ud->tx_dma.channel_number != HW_DMA_CHANNEL_INVALID && len > 1) {
                if (ud->tx_dma.burst_mode != HW_DMA_BURST_MODE_DISABLED  ) {
                        if (((ud->tx_dma.burst_mode == HW_DMA_BURST_MODE_4x) && (len%4 != 0 )) ||
                            ((ud->tx_dma.burst_mode == HW_DMA_BURST_MODE_8x) && (len%8 != 0 ))) {
                                return HW_UART_CONFIG_ERR_TX_SIZE;
                        }
                }
                ud->tx_dma.src_address = (uint32) data;
                ud->tx_dma.length = len;
                // DMA requested
                hw_uart_clear_dma_request(uart);
                hw_dma_channel_initialization(&ud->tx_dma);
                hw_dma_channel_enable(ud->tx_dma.channel_number, HW_DMA_STATE_ENABLED);
                return HW_UART_CONFIG_ERR_NOERR;
        }
#endif /* HW_UART_DMA_SUPPORT */
        hw_uart_enable_tx_int(uart, true);
        return HW_UART_CONFIG_ERR_NOERR;
}

#if (dg_configUART_SOFTWARE_FIFO == 1)

#define SOFTWARE_FIFO_PRESENT(ud) ((ud)->rx_soft_fifo != NULL)

/*
 * Copy bytes from software FIFO to user provided buffer.
 * This function allows software FIFO interrupts to read more data while copy takes place.
 *
 * Function returns true if all requested data is already in user buffer.
 */
static bool hw_uart_drain_rx(HW_UART_ID uart, UART_Data *ud, int len)
{
        /*
         * This function is called with RX interrupt disabled.
         *
         * Get current software FIFO pointers before enabling interrupt again.
         * ud->rx_len is still set to 0, so interrupt will not try to copy data to
         * application buffer till all data from software FIFO is drained to user buffer.
         */
        fifo_ptr_t rd_ptr = ud->rx_soft_fifo_rd_ptr;
        fifo_ptr_t wr_ptr = ud->rx_soft_fifo_wr_ptr;
        int idx = 0;

        /*
         * ud->rx_ix is 0, set ud->rx_len to 0 to prevent interrupt from using it
         * before all data from software FIFO is moved to application buffer.
         */
        ud->rx_len = 0;

        hw_uart_enable_rx_int(uart, true);

        while (idx < len) {

                if (wr_ptr == rd_ptr) {
                        /*
                         * No more data in software FIFO, at least from state before
                         * interrupt was enabled.
                         * Disable interrupt and update read pointer to reflect position
                         * of already copied data.
                         */
                        hw_uart_enable_rx_int(uart, false);
                        ud->rx_soft_fifo_rd_ptr = rd_ptr;

                        /*
                         * Check if write pointer moved, if so interrupt put some more data
                         * in buffer. Remember current write position and try again with
                         * interrupts enabled.
                         */
                        if (ud->rx_soft_fifo_wr_ptr != wr_ptr) {
                                wr_ptr = ud->rx_soft_fifo_wr_ptr;
                                hw_uart_enable_rx_int(uart, true);
                                continue;
                        }

                        /*
                         * All was copied from software FIFO.
                         * Setup user transaction ix and len so when interrupts are enabled
                         * again (not in this function) interrupt or DMA can continue.
                         */
                        ud->rx_ix = idx;
                        ud->rx_len = len;

                        return false;
                }

                /* Copy from software FIFO to user provided buffer */
                ud->rx_buffer[idx++] = ud->rx_soft_fifo[rd_ptr++];

                if (rd_ptr >= ud->rx_soft_fifo_size) {
                        rd_ptr = 0;
                }
        }

        /*
         * User buffer is filled, block interrupt so end of transmission callback will not be
         * called from interrupt.
         */
        hw_uart_enable_rx_int(uart, false);
        ud->rx_soft_fifo_rd_ptr = rd_ptr;
        ud->rx_len = len;
        ud->rx_ix = len;

        return true;
}

void hw_uart_read_buffer(HW_UART_ID uart, void *data, uint16_t len)
{
        UART_Data *ud = UARTDATA(uart);
        uint8_t *p = data;

        /*
         * Disable RX interrupt before draining software FIFO.
         */
        hw_uart_enable_rx_int(uart, false);
        if (SOFTWARE_FIFO_PRESENT(ud)) {
                /*
                 * Drain uses ud fields so setup them accordingly.
                 */
                ud->rx_buffer = data;

                hw_uart_drain_rx(uart, ud, len);
                len -= ud->rx_ix;
                p += ud->rx_ix;
        }
        /*
         * Read all remaining bytes with RX interrupt still disabled.
         */
        while (len > 0) {
                *p++ = hw_uart_read(uart);
                len--;
        }
        ud->rx_ix = 0;
        ud->rx_len = 0;
        hw_uart_enable_rx_int(uart, SOFTWARE_FIFO_PRESENT(ud));
}

void hw_uart_set_soft_fifo(HW_UART_ID uart, uint8_t *buf, uint8_t size)
{
        UART_Data *ud = UARTDATA(uart);

        hw_uart_enable_rx_int(uart, false);

        ud->rx_soft_fifo = buf;
        ud->rx_soft_fifo_size = size;
        ud->rx_soft_fifo_rd_ptr = 0;
        ud->rx_soft_fifo_wr_ptr = 0;

        hw_uart_enable_rx_int(uart, buf != NULL);
}

#else

#define SOFTWARE_FIFO_PRESENT(ud)  false

void hw_uart_read_buffer(HW_UART_ID uart, void *data, uint16_t len)
{
        uint8_t *p = data;

        while (len > 0) {
                *p++ = hw_uart_read(uart);
                len--;
        }
}
#endif /* dg_configUART_SOFTWARE_FIFO */

static void hw_uart_fire_rx_callback(UART_Data *ud)
{
        hw_uart_rx_callback cb = ud->rx_cb;
        ud->rx_cb = NULL;
        /* Just finished receiving, disable RX interrupts unless software FIFO is enabled */
        hw_uart_enable_rx_int(UARTID(ud), SOFTWARE_FIFO_PRESENT(ud));
        if (cb) {
                cb(ud->rx_user_data, ud->rx_len);
        }
}

__STATIC_INLINE void hw_uart_fire_tx_callback(UART_Data *ud)
{
        hw_uart_tx_callback cb = ud->tx_cb;
        if (cb) {
                ud->tx_cb = NULL;
                cb(ud->tx_user_data, ud->tx_len);
        }
}

HW_UART_CONFIG_ERR hw_uart_receive(HW_UART_ID uart, void *data, uint16_t len, hw_uart_rx_callback cb,
                                                                                void *user_data)
{
        UART_Data *ud = UARTDATA(uart);

        if (cb == NULL) {
                hw_uart_read_buffer(uart, data, len);
                ud->rx_ix = 0;
                ud->rx_len = 0;
                return HW_UART_CONFIG_ERR_NOERR;
        }

        ud->rx_buffer = data;
        ud->rx_user_data = user_data;
        hw_uart_enable_rx_int(uart, false);
        ud->rx_len = len;
        ud->rx_ix = 0;
        ud->rx_cb = cb;
#if (dg_configUART_SOFTWARE_FIFO == 1)
        if (SOFTWARE_FIFO_PRESENT(ud)) {
                if (hw_uart_drain_rx(uart, ud, len)) {
                        hw_uart_fire_rx_callback(ud);
                        return HW_UART_CONFIG_ERR_NOERR;
                }
        }
#endif /* dg_configUART_SOFTWARE_FIFO */

#if (HW_UART_DMA_SUPPORT == 1)
#if (dg_configUART_RX_CIRCULAR_DMA == 1)
        if (ud->rx_dma_buf_size > 0) {
                ASSERT_ERROR(len < ud->rx_dma_buf_size);

                uint16_t new_int, cur_idx;
                bool data_ready = false;

                ASSERT_ERROR(ud->rx_dma_active == false);

                /* Calculate index of end of requested data (do not wrap it!) */
                new_int = ud->rx_dma_head + ud->rx_len - 1;

                /* Freeze DMA so it does not move pointers while we try to update them */
                hw_dma_freeze();

                cur_idx = hw_dma_transfered_bytes(ud->rx_dma.channel_number);

                /* cur_idx is lower than rx_head only if it wrapped-around - fix it */
                if (cur_idx < ud->rx_dma_head) {
                        cur_idx += ud->rx_dma_buf_size;
                }

                /*
                 * If DMA has not read past the calculated index, we can set it and just wait for an
                 * interrupt. In other case, data are ready immediately in buffer.
                 */
                if (cur_idx <= new_int) {
                        new_int %= ud->rx_dma_buf_size;
                        hw_dma_channel_update_int_ix(ud->rx_dma.channel_number, new_int);
                        ud->rx_dma_active = true;
                } else {
                        hw_dma_channel_update_int_ix(ud->rx_dma.channel_number, cur_idx - 1);
                        data_ready = true;
                }

                /* Unfreeze DMA now, it can start reading */
                hw_dma_unfreeze();

                /* Fire callback immediately if data already available in buffer */
                if (data_ready) {
                        hw_uart_fire_rx_callback(ud);
                }

                return HW_UART_CONFIG_ERR_NOERR;
        }
#endif /* dg_configUART_RX_CIRCULAR_DMA */

        if (ud->rx_dma.channel_number != HW_DMA_CHANNEL_INVALID && (ud->rx_len - ud->rx_ix > 1)) {
                if (ud->rx_dma.burst_mode != HW_DMA_BURST_MODE_DISABLED  ) {
                        if (((ud->rx_dma.burst_mode == HW_DMA_BURST_MODE_4x) && (len%4 != 0 ))
                                || ((ud->rx_dma.burst_mode == HW_DMA_BURST_MODE_8x) && (len%8 != 0 ))) {
                                return HW_UART_CONFIG_ERR_RX_SIZE;
                        }
                }
                /* rx_ix could already be changed by hw_uart_drain_rx() */
                ud->rx_dma.dest_address = (uint32) data + ud->rx_ix;
                ud->rx_dma.length = ud->rx_len - ud->rx_ix;
                hw_uart_clear_dma_request(uart);
                /* Prepare and start DMA */
                hw_dma_channel_initialization(&ud->rx_dma);
                hw_dma_channel_enable(ud->rx_dma.channel_number, HW_DMA_STATE_ENABLED);
                return HW_UART_CONFIG_ERR_NOERR;
        }
#endif /* HW_UART_DMA_SUPPORT */
        /* Interrupt driven */
        hw_uart_enable_rx_int(uart, true);
        return HW_UART_CONFIG_ERR_NOERR;
}

void hw_uart_receive_error_checking(HW_UART_ID uart, void *data, uint16_t len, hw_uart_rx_callback cb,
                                    void *user_data, hw_uart_err_callback err_cb, void *error_data) {

        if (err_cb != NULL) {
                UART_Data *ud = UARTDATA(uart);
                ud->err_cb = err_cb;
                ud->err_user_data = error_data;

                hw_uart_linestat_int_set(uart, true);
        }

        hw_uart_receive(uart, data, len, cb, user_data);
}

static void hw_uart_irq_stop_receive(HW_UART_ID uart)
{
        UART_Data *ud = &uart_data[UARTIX(uart)];

        // Disable RX interrupt
        hw_uart_enable_rx_int(uart, false);

        if (ud->err_cb != NULL) {
                // Disable line status error interrupts
                hw_uart_linestat_int_set(uart, false);
        }

        ud->rx_len = ud->rx_ix;

        hw_uart_fire_rx_callback(ud);
}

#if (dg_configUART_RX_CIRCULAR_DMA == 1)
uint16_t hw_uart_copy_dma_rx_to_user_buffer(HW_UART_ID uart)
{
        UART_Data *ud = &uart_data[UARTIX(uart)];

        uint16_t cur_idx;
        uint16_t to_copy = 0;
        hw_uart_rx_callback cb;

        ud->rx_dma_active = false;
        cb = ud->rx_cb;

        if (cb) {
                ud->rx_cb = NULL;
                /*
                 * User callback was not fired, since rx_dma_active is false it will not
                 * fire even if requested number of bytes was received when abort was initiated.
                 */
                cur_idx = hw_dma_transfered_bytes(ud->rx_dma.channel_number);

                if (ud->rx_ix < ud->rx_len) {
                        if (cur_idx < ud->rx_dma_head) {
                                cur_idx += ud->rx_dma_buf_size;
                        }
                        to_copy = cur_idx - ud->rx_dma_head;
                        if (to_copy >= ud->rx_len - ud->rx_ix) {
                                to_copy = ud->rx_len - ud->rx_ix;
                        }
                }
        } else {
                /*
                 * Callback already fired, circular buffer holds enough data.
                 */
                to_copy = ud->rx_len - ud->rx_ix;
        }

        hw_uart_copy_rx_circular_dma_buffer(uart, ud->rx_buffer + ud->rx_ix, to_copy);
        ud->rx_ix += to_copy;
        ud->rx_len = ud->rx_ix;

        if (cb) {
                cb(ud->rx_user_data, ud->rx_len);
        }
        return ud->rx_ix;
}
#endif /* dg_configUART_RX_CIRCULAR_DMA */

uint16_t hw_uart_abort_receive(HW_UART_ID uart)
{
        UART_Data *ud = &uart_data[UARTIX(uart)];

#if (HW_UART_DMA_SUPPORT == 1)
        /* Check if the DMA Rx channel is valid and active. */
        if (ud->rx_dma.channel_number != HW_DMA_CHANNEL_INVALID &&
            hw_dma_is_channel_active(ud->rx_dma.channel_number)) {
                /* Stop DMA even if DMA circular buffer is used */
                hw_dma_channel_stop(ud->rx_dma.channel_number);
        }
#endif
        hw_uart_irq_stop_receive(uart);

        if (ud->err_cb != NULL) {
                // Disable line status error interrupts
                hw_uart_linestat_int_set(uart, false);
        }
        return ud->rx_ix;
}

uint16_t hw_uart_abort_send(HW_UART_ID uart)
{
        UART_Data *ud = &uart_data[UARTIX(uart)];

#if (HW_UART_DMA_SUPPORT == 1)
        /* Check if the DMA Tx channel is valid and active. */
        if (ud->tx_dma.channel_number != HW_DMA_CHANNEL_INVALID &&
            hw_dma_is_channel_active(ud->tx_dma.channel_number)) {
                hw_dma_channel_stop(ud->tx_dma.channel_number);
        }
#endif /* HW_UART_USE_DMA_SUPPORT */

        // Disable TX interrupts
        NVIC_DisableIRQ(UART_INT(uart));
        hw_uart_enable_tx_int(uart, false);
        NVIC_EnableIRQ(UART_INT(uart));
        ud->rx_len = ud->rx_ix;
        hw_uart_fire_tx_callback(ud);

        return ud->tx_ix;
}

uint16_t hw_uart_peek_received(const HW_UART_ID uart)
{
        UART_Data *ud = &uart_data[UARTIX(uart)];

#if (HW_UART_DMA_SUPPORT == 1)
        if (ud->rx_dma.channel_number != HW_DMA_CHANNEL_INVALID)
                ud->rx_ix = hw_dma_transfered_bytes(ud->rx_dma.channel_number);
#endif
        return ud->rx_ix;
}

uint16_t hw_uart_peek_transmitted(const HW_UART_ID uart)
{
        UART_Data *ud = &uart_data[UARTIX(uart)];

#if (HW_UART_DMA_SUPPORT == 1)
        if (ud->tx_dma.channel_number != HW_DMA_CHANNEL_INVALID)
                ud->tx_ix = hw_dma_transfered_bytes(ud->tx_dma.channel_number);
#endif
        return ud->tx_ix;
}

//============== Interrupt handling ============================================

__STATIC_INLINE void hw_uart_tx_isr(HW_UART_ID uart)
{
        UART_Data *ud = UARTDATA(uart);

        while (ud->tx_ix < ud->tx_len) {
                if (ud->tx_fifo_on) {
                        if (!hw_uart_transmit_fifo_not_full(uart)) {
                                break;
                        }
                } else if (!hw_uart_thr_empty_getf(uart)) {
                        break;
                }
                hw_uart_txdata_setf(uart, ud->tx_buffer[ud->tx_ix++]);
        }
        // Everything sent?
        if (ud->tx_ix >= ud->tx_len) {
                // Disable TX interrupts and fire callback
                hw_uart_enable_tx_int(uart, false);
                hw_uart_fire_tx_callback(ud);
        }
}

__STATIC_INLINE void hw_uart_rx_isr(HW_UART_ID uart)
{
        UART_Data *ud = UARTDATA(uart);

        if (SOFTWARE_FIFO_PRESENT(ud)) {
#if (dg_configUART_SOFTWARE_FIFO == 1)
                for (;;) {
                        fifo_ptr_t wr_ptr = ud->rx_soft_fifo_wr_ptr + 1;
                        if (wr_ptr >= ud->rx_soft_fifo_size) {
                                wr_ptr = 0;
                        }
                        if (wr_ptr == ud->rx_soft_fifo_rd_ptr) {
                                /* Software FIFO full, disable interrupt since no one is reading */
                                hw_uart_enable_rx_int(uart, false);
                                return;
                        }
                        if (!hw_uart_is_data_ready(uart)) {
                                break;
                        }
                        ud->rx_soft_fifo[ud->rx_soft_fifo_wr_ptr] = hw_uart_rxdata_getf(uart);

                        /*
                         * Application read is in progress. Copy data from software FIFO to
                         * user provided buffer.
                         */
                        if (ud->rx_ix < ud->rx_len) {
                                ud->rx_buffer[ud->rx_ix++] = ud->rx_soft_fifo[ud->rx_soft_fifo_wr_ptr];
                                /*
                                 * When application read is in progress, rx_ix < rx_len
                                 * This interrupt is enabled only if all data was already copied from
                                 * FIFO to user buffer. It is safe to modify rx_soft_fifo_rd_ptr.
                                 */
                                ud->rx_soft_fifo_rd_ptr = wr_ptr;
                        }

                        ud->rx_soft_fifo_wr_ptr = wr_ptr;
                }
#endif /* dg_configUART_SOFTWARE_FIFO */
        } else {
                while (ud->rx_ix < ud->rx_len) {
                        if (hw_uart_is_data_ready(uart)) {
                                ud->rx_buffer[ud->rx_ix++] = hw_uart_rxdata_getf(uart);
                        } else {
                                break;
                        }
                }
        }

        // Everything read?
        if (ud->rx_len > 0 && ud->rx_ix >= ud->rx_len) {
                // Disable RX interrupts and fire callback
                hw_uart_irq_stop_receive(uart);
        }

}

__STATIC_INLINE void hw_uart_rx_timeout_isr(HW_UART_ID uart)
{
        UART_Data *ud = UARTDATA(uart);

        hw_uart_rx_isr(uart);

        /*
         * Not everything was received yet, disable interrupt anyway since
         * some data was received.
         */
        if (ud->rx_ix > 0 && ud->rx_ix < ud->rx_len) {
                // Disable RX interrupts fire callback if present
                hw_uart_irq_stop_receive(uart);
        }
}

__STATIC_INLINE void hw_uart_error_isr(HW_UART_ID uart) {
        UART_Data *ud = UARTDATA(uart);
        if (ud->err_cb) {
                ud->err_cb(ud->err_user_data, hw_uart_error_getf(uart));
        }
}

void UART_Interrupt_Handler(HW_UART_ID uart)
{
        HW_UART_INT int_id;

        for (;;) {
                int_id = hw_uart_get_interrupt_id(uart);
                switch (int_id) {
                case HW_UART_INT_TIMEOUT:
                        hw_uart_rx_timeout_isr(uart);
                        break;
                case HW_UART_INT_MODEM_STAT:
                        break;
                case HW_UART_INT_NO_INT_PEND:
                        return;
                        break;
                case HW_UART_INT_THR_EMPTY:
                        hw_uart_tx_isr(uart);
                        break;
                case HW_UART_INT_RECEIVED_AVAILABLE:
                        hw_uart_rx_isr(uart);
                        break;
                case HW_UART_INT_RECEIVE_LINE_STAT:
                        hw_uart_error_isr(uart);
                        break;
                case HW_UART_INT_BUSY_DETECTED:
#ifdef CONFIG_UART_IGNORE_BUSY_DETECT
                        hw_uart_transmit_fifo_empty(uart);
#else
                        /*
                         * Stop here means that timing rules for access divisor latch were not
                         * followed. See description of register RBR_THR_DLL.
                         */
                        __BKPT(0);
#endif
                        break;
                }
        }
}

/**
 * \brief UART1 Interrupt Handler
 *
 */
void UART_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

#ifdef HW_UART_ENABLE_USER_ISR
        if (uart_data[UARTIX(HW_UART1)].user_isr) {
                uart_data[UARTIX(HW_UART1)].user_isr();
        } else {
                UART_Interrupt_Handler(HW_UART1);
        }
#else
        UART_Interrupt_Handler(HW_UART1);
#endif

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

/**
 * \brief UART2 Interrupt Handler
 *
 */
void UART2_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

#ifdef HW_UART_ENABLE_USER_ISR
        if (uart_data[UARTIX(HW_UART2)].user_isr) {
                uart_data[UARTIX(HW_UART2)].user_isr();
        } else {
                UART_Interrupt_Handler(HW_UART2);
        }
#else
        UART_Interrupt_Handler(HW_UART2);
#endif

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#if (dg_configUSE_HW_ISO7816 == 0)
/**
 * \brief UART3 Interrupt Handler
 *
 */
void UART3_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

#ifdef HW_UART_ENABLE_USER_ISR
        if (uart_data[UARTIX(HW_UART3)].user_isr) {
                uart_data[UARTIX(HW_UART3)].user_isr();
        } else {
                UART_Interrupt_Handler(HW_UART3);
        }
#else
        UART_Interrupt_Handler(HW_UART3);
#endif

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
#endif /* dg_configUSE_HW_ISO7816 */

//==================== Configuration functions =================================

/*
 * \brief Configure the serial clock input of the UART (divN or div1)
 */
__STATIC_INLINE void hw_uart_set_sclk(const HW_UART_ID uart, bool sclk)
{
        __IOM uint32_t *reg = sclk ? &CRG_SNC->SET_CLK_SNC_REG : &CRG_SNC->RESET_CLK_SNC_REG;
        if (uart == HW_UART1) {
                *reg = REG_MSK(CRG_SNC, CLK_SNC_REG, UART_CLK_SEL);
        } else if (uart == HW_UART2) {
                *reg = REG_MSK(CRG_SNC, CLK_SNC_REG, UART2_CLK_SEL);
        } else if (uart == HW_UART3) {
                *reg = REG_MSK(CRG_SNC, CLK_SNC_REG, UART3_CLK_SEL);
        }
}

HW_UART_BAUDRATE hw_uart_baudrate_get(HW_UART_ID uart)
{
        uint32_t divisor, baud_rate;

        // Set Divisor Latch Access Bit in LCR register to access DLL & DLH registers
        HW_UART_REG_SETF(uart, LCR, UART_DLAB, 1);
        // Read baud rate low byte from DLL register
        divisor = (0xFF & UBA(uart)->UART2_RBR_THR_DLL_REG) << 8;
        // Read baud rate high byte from DLH register
        divisor += (0xFF & UBA(uart)->UART2_IER_DLH_REG) << 16;
        // Read baud rate fraction byte from DLF register
        divisor += 0xFF & UBA(uart)->UART2_DLF_REG;
        // Reset Divisor Latch Access Bit in Line Control Register
        HW_UART_REG_SETF(uart, LCR, UART_DLAB, 0);
        // DLAB will not reset if UART is busy, for example if UART Rx line is LOW
        // while UART_DLAB is SET. HW_UART_INT_BUSY_DETECTED could be caused
        // in different case. The caller could temporary disable any UARTx_RX GPIOs
        // to avoid LOW state on UART Rx.
        ASSERT_ERROR(HW_UART_REG_GETF(uart, LCR, UART_DLAB) == 0);

        baud_rate = divisor;
        if (((uart == HW_UART1) && (REG_GETF(CRG_SNC, CLK_SNC_REG, UART_CLK_SEL)))   ||
            ((uart == HW_UART2) && (REG_GETF(CRG_SNC, CLK_SNC_REG, UART2_CLK_SEL)))  ||
            ((uart == HW_UART3) && (REG_GETF(CRG_SNC, CLK_SNC_REG, UART3_CLK_SEL)))) {
                sys_clk_is_t sys_clk = hw_clk_get_sysclk();
                if (sys_clk == SYS_CLK_IS_PLL) {
                        switch (divisor) {
                                case 0x00000305:
                                        baud_rate = HW_UART_BAUDRATE_3000000;
                                        break;
#if MAIN_PROCESSOR_BUILD
                                case 0x0000010B:
                                        baud_rate = HW_UART_BAUDRATE_6000000;
                                        break;
#endif
                        }
                } else if ((sys_clk == SYS_CLK_IS_RCHS) && (hw_clk_get_rchs_mode() == RCHS_96)) {
                        switch (divisor) {
                                case 0x00000200:
                                        baud_rate = HW_UART_BAUDRATE_3000000;
                                        break;
#if MAIN_PROCESSOR_BUILD
                                case 0x00000100:
                                        baud_rate = HW_UART_BAUDRATE_6000000;
                                        break;
#endif
                        }
                }
        }
        return (HW_UART_BAUDRATE) baud_rate;
}

void hw_uart_baudrate_set(HW_UART_ID uart, HW_UART_BAUDRATE baud_rate)
{
        uint32_t divisor = baud_rate;
        bool sclk;
        sclk = false; /* Use DivN */

        if (baud_rate < 0x100) { /* HW_UART_BAUDRATE_2000000 = 0x100*/
                /* Requested baud rate is greater than 2MBps. In this case, a high speed system clock is required. */
                sys_clk_is_t sys_clk = hw_clk_get_sysclk();
                if (sys_clk == SYS_CLK_IS_PLL) {
                        switch (baud_rate) {
                                case HW_UART_BAUDRATE_3000000:
                                        divisor = 0x00000305;
                                        break;
#if MAIN_PROCESSOR_BUILD
                                case HW_UART_BAUDRATE_6000000:
                                        divisor = 0x0000010B;
                                        break;
#endif
                                default:
                                        ASSERT_ERROR(0); /* Specified baud rate is invalid. */

                                }
                        sclk = true; /* Use Div1 */
                } else if ((sys_clk == SYS_CLK_IS_RCHS) && (hw_clk_get_rchs_mode() == RCHS_96)) {
                        switch (baud_rate) {
                                case HW_UART_BAUDRATE_3000000:
                                        divisor = 0x00000200;
                                        break;
#if MAIN_PROCESSOR_BUILD
                                case HW_UART_BAUDRATE_6000000:
                                        divisor = 0x00000100;
                                        break;
#endif
                                default:
                                        ASSERT_ERROR(0); /* Specified baud rate is invalid. */
                        }
                        sclk = true; /* Use Div1 */
                } else {
                        /* The specified baud rate divider settings are not applicable.
                         * In case of using a high baud rate value of the HW_UART_BAUDRATE enum,
                         * then this means that the currently selected system clock is not high
                         * enough for achieving the specified baud rate. */
                        ASSERT_ERROR(0);
                }
        }
                hw_uart_set_sclk(uart, sclk);

        // Set Divisor Latch Access Bit in LCR register to access DLL & DLH registers
        HW_UART_REG_SETF(uart, LCR, UART_DLAB, 1);
        // Set fraction byte of baud rate
        UBA(uart)->UART2_DLF_REG = 0xFF & divisor;
        // Set low byte of baud rate
        UBA(uart)->UART2_RBR_THR_DLL_REG = 0xFF & (divisor >> 8);
        // Set high byte of baud rare
        UBA(uart)->UART2_IER_DLH_REG = 0xFF & (divisor >> 16);
        // Reset Divisor Latch Access Bit in LCR register
        HW_UART_REG_SETF(uart, LCR, UART_DLAB, 0);
        // DLAB will not reset if UART is busy, for example if UART Rx line is LOW
        // while UART_DLAB is SET. HW_UART_INT_BUSY_DETECTED could be caused
        // in different case. The caller could temporary disable any UARTx_RX GPIOs
        // to avoid LOW state on UART Rx.
        ASSERT_ERROR(HW_UART_REG_GETF(uart, LCR, UART_DLAB) == 0);
}

//=========================== FIFO control functions ===========================

uint8_t hw_uart_fifo_en_getf(HW_UART_ID uart)
{
        uint16_t fifo_enabled;


        fifo_enabled = (UBA(uart)->UART2_IIR_FCR_REG & 0x00C0); /* Bits[7:6] */

        switch (fifo_enabled) {
        case 0x00C0:
                return 1;
        case 0x0000:
                return 0;
        default:
                ASSERT_ERROR(0);
                return 255;     // To satisfy the compiler
        }
}

uint8_t hw_uart_tx_fifo_tr_lvl_getf(HW_UART_ID uart)
{

        return (UBA(uart)->UART2_STET_REG & HW_UART_REG_FIELD_MASK(2, STET, UART_SHADOW_TX_EMPTY_TRIGGER))
                >> HW_UART_REG_FIELD_POS(2, STET, UART_SHADOW_TX_EMPTY_TRIGGER);
}

//=========================== DMA control functions ============================

#if (HW_UART_DMA_SUPPORT == 1)

static void hw_uart_rx_dma_callback(void *user_data, uint32_t len)
{
        UART_Data *ud = user_data;
        hw_uart_rx_callback cb = ud->rx_cb;

        ud->rx_cb = NULL;
        ud->rx_ix += len;
        if (cb) {
                ud->rx_len = ud->rx_ix;
                hw_uart_enable_rx_int(UARTID(ud), SOFTWARE_FIFO_PRESENT(ud));
                cb(ud->rx_user_data, ud->rx_ix);
        }
}

static void hw_uart_tx_dma_callback(void *user_data, uint32_t len)
{
        UART_Data *ud = user_data;
        hw_uart_tx_callback cb = ud->tx_cb;

        ud->tx_cb = NULL;
        ud->tx_ix = len;
        if (cb) {
                cb(ud->tx_user_data, len);
        }
}

void hw_uart_configure_dma_channels(HW_UART_ID uart, const uart_config_ex *uart_init)
{
        ASSERT_WARNING(uart_init != NULL);

        UART_Data *ud = UARTDATA(uart);
        int8_t tx_channel = uart_init->tx_dma_channel;
        int8_t rx_channel = uart_init->rx_dma_channel;
        HW_DMA_PRIO rx_priority = HW_UART_DEFAULT_DMA_RX_PRIO;
        HW_DMA_PRIO tx_priority = HW_UART_DEFAULT_DMA_TX_PRIO;

        if (uart_init->dma_prio.use_prio) {
                rx_priority = uart_init->dma_prio.rx_prio;
                tx_priority = uart_init->dma_prio.tx_prio;
        }

        uint8_t tx_burst_mode = uart_init->tx_dma_burst_lvl;
        uint8_t rx_burst_mode = uart_init->rx_dma_burst_lvl;

        /* Only specific DMA channels are allowed (or HW_DMA_CHANNEL_INVALID for no DMA) */
        ASSERT_ERROR(tx_channel >= HW_DMA_CHANNEL_0 && tx_channel <= HW_DMA_CHANNEL_INVALID);

        /* Only specific DMA channels are allowed (or HW_DMA_CHANNEL_INVALID for no DMA) */
        ASSERT_ERROR(rx_channel >= HW_DMA_CHANNEL_0 && rx_channel <= HW_DMA_CHANNEL_INVALID);

        if (tx_channel == HW_DMA_CHANNEL_INVALID && rx_channel == HW_DMA_CHANNEL_INVALID) {
                ud->use_dma = 0;
                ud->rx_dma.channel_number = HW_DMA_CHANNEL_INVALID;
                ud->tx_dma.channel_number = HW_DMA_CHANNEL_INVALID;
        } else {
                if (tx_channel != HW_DMA_CHANNEL_INVALID && rx_channel != HW_DMA_CHANNEL_INVALID) {//not both invalid
                        ASSERT_ERROR(tx_channel != rx_channel);//not equal
                        ASSERT_ERROR(tx_channel>>1 == rx_channel>>1);//on same pair
                }
                if (tx_channel != HW_DMA_CHANNEL_INVALID) {
                        ASSERT_ERROR(tx_channel & 1);//odd number
                }
                if (rx_channel != HW_DMA_CHANNEL_INVALID) {
                        ASSERT_ERROR((rx_channel & 1) == 0);//even number
                }

                ud->use_dma = 1;

                ud->rx_dma.channel_number = rx_channel;
                ud->rx_dma.bus_width = HW_DMA_BW_BYTE;
                ud->rx_dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
                ud->rx_dma.dma_req_mux = UARTIX(uart) == 0 ? HW_DMA_TRIG_UART_RXTX :
                                        (UARTIX(uart) == 1 ? HW_DMA_TRIG_UART2_RXTX :
                                                             HW_DMA_TRIG_UART3_RXTX);
                ud->rx_dma.irq_nr_of_trans = 0;
                ud->rx_dma.a_inc = HW_DMA_AINC_FALSE;
                ud->rx_dma.b_inc = HW_DMA_BINC_TRUE;
                ud->rx_dma.circular = HW_DMA_MODE_NORMAL;
                ud->rx_dma.dma_prio = rx_priority;
                ud->rx_dma.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE; /* Not used by the HW in this case */
                ud->rx_dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
                ud->rx_dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
                ud->rx_dma.burst_mode = (rx_burst_mode == 0 ? HW_DMA_BURST_MODE_DISABLED :
                                        (rx_burst_mode == 1 ? HW_DMA_BURST_MODE_4x : HW_DMA_BURST_MODE_8x));
                ud->rx_dma.src_address = (uint32_t) &UBA(uart)->UART2_RBR_THR_DLL_REG;
                ud->rx_dma.dest_address = 0;  // Change during transmission
                ud->rx_dma.length = 0; // Change during transmission
                ud->rx_dma.callback = hw_uart_rx_dma_callback;
                ud->rx_dma.user_data = ud;

                ud->tx_dma.channel_number = tx_channel;
                ud->tx_dma.bus_width = HW_DMA_BW_BYTE;
                ud->tx_dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
                ud->tx_dma.dma_req_mux = UARTIX(uart) == 0 ? HW_DMA_TRIG_UART_RXTX :
                                        (UARTIX(uart) == 1 ? HW_DMA_TRIG_UART2_RXTX :
                                                             HW_DMA_TRIG_UART3_RXTX);
                ud->tx_dma.irq_nr_of_trans = 0;
                ud->tx_dma.a_inc = HW_DMA_AINC_TRUE;
                ud->tx_dma.b_inc = HW_DMA_BINC_FALSE;
                ud->tx_dma.circular = HW_DMA_MODE_NORMAL;
                ud->tx_dma.dma_prio = tx_priority;
                ud->tx_dma.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE; /* Not used by the HW in this case */
                ud->tx_dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
                ud->tx_dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
                ud->tx_dma.burst_mode = (tx_burst_mode == 0 ? HW_DMA_BURST_MODE_DISABLED :
                                        (tx_burst_mode == 1 ? HW_DMA_BURST_MODE_4x : HW_DMA_BURST_MODE_8x));
                ud->tx_dma.src_address = 0; // Change during transmission
                ud->tx_dma.dest_address = (uint32_t) &UBA(uart)->UART2_RBR_THR_DLL_REG;
                ud->tx_dma.length = 0; // Change during transmission
                ud->tx_dma.callback = hw_uart_tx_dma_callback;
                ud->tx_dma.user_data = ud;
        }
}

#if (dg_configUART_RX_CIRCULAR_DMA == 1)

static void hw_uart_rx_circular_dma_callback(void *user_data, dma_size_t len)
{
        UART_Data *ud = user_data;
        hw_uart_rx_callback cb = ud->rx_cb;

        if (!ud->rx_dma_active) {
                return;
        }

        ud->rx_cb = NULL;
        ud->rx_dma_active = false;

        if (cb) {
                cb(ud->rx_user_data, ud->rx_len);
        }
}

void hw_uart_enable_rx_circular_dma(HW_UART_ID uart)
{
        UART_Data *ud = UARTDATA(uart);

        ASSERT_ERROR(ud->rx_dma_buf_size > 0);

        hw_dma_channel_enable(ud->rx_dma.channel_number, HW_DMA_STATE_DISABLED);

        /* Need to reconfigure few things for circular operation... */
        ud->rx_dma.circular = HW_DMA_MODE_CIRCULAR;
        ud->rx_dma.dest_address = (uint32_t) ud->rx_dma_buf;
        ud->rx_dma.length = ud->rx_dma_buf_size;
        ud->rx_dma.callback = hw_uart_rx_circular_dma_callback;
        ud->rx_dma.user_data = ud;

        /* Reset DMA buffer read pointer */
        ud->rx_dma_head = 0;

        /* Start DMA now since it should be always-running */
        hw_uart_clear_dma_request(uart);
        hw_dma_channel_initialization(&ud->rx_dma);
        hw_dma_channel_enable(ud->rx_dma.channel_number, HW_DMA_STATE_ENABLED);
}

void hw_uart_copy_rx_circular_dma_buffer(HW_UART_ID uart, uint8_t *buf, uint16_t len)
{
        UART_Data *ud = UARTDATA(uart);

        ASSERT_ERROR(len < ud->rx_dma_buf_size);

        if (ud->rx_dma_head + len <= ud->rx_dma_buf_size) {
                memcpy(buf, &ud->rx_dma_buf[ud->rx_dma_head], len);
        } else {
                uint16_t chunk_len = ud->rx_dma_buf_size - ud->rx_dma_head;
                memcpy(buf, &ud->rx_dma_buf[ud->rx_dma_head], chunk_len);
                memcpy(buf + chunk_len, &ud->rx_dma_buf[0], len - chunk_len);
        }

        /* This should be protected so ISR does not try to read rx_dma_head while we update it */
        GLOBAL_INT_DISABLE();
        ud->rx_dma_head = (ud->rx_dma_head + len) % ud->rx_dma_buf_size;
        GLOBAL_INT_RESTORE();
}

#endif /* dg_configUART_RX_CIRCULAR_DMA */

#endif /* HW_UART_DMA_SUPPORT */

#if (HW_UART_DMA_SUPPORT == 1)
static HW_UART_CONFIG_ERR fifo_dma_burst_not_match(const uart_config_ex *uart_init)
{
        HW_UART_CONFIG_ERR err = HW_UART_CONFIG_ERR_NOERR;

        if (uart_init->rx_dma_burst_lvl) {
                if (((uart_init->rx_fifo_tr_lvl == 0) || (uart_init->rx_fifo_tr_lvl == 3)) ||
                    ((uart_init->rx_fifo_tr_lvl == 1) && (!(uart_init->rx_dma_burst_lvl == 1))) ||
                    ((uart_init->rx_fifo_tr_lvl == 2) && (!(uart_init->rx_dma_burst_lvl == 2)))) {
                        err = HW_UART_CONFIG_ERR_RX_FIFO;
                }
        }

        if (uart_init->tx_dma_burst_lvl) {
                if (((uart_init->tx_fifo_tr_lvl == 0) || (uart_init->tx_fifo_tr_lvl == 1)) ||
                    ((uart_init->tx_fifo_tr_lvl == 2) && (!(uart_init->tx_dma_burst_lvl == 1))) ||
                    ((uart_init->tx_fifo_tr_lvl == 3) && (!(uart_init->tx_dma_burst_lvl == 2)))) {
                        if (err) {
                                err = HW_UART_CONFIG_ERR_RXTX_FIFO;
                        } else {
                                err = HW_UART_CONFIG_ERR_TX_FIFO;
                        }
                }
        }

        return err;
}

#endif /* HW_UART_DMA_SUPPORT */

//=========================== Line control functions ============================

__STATIC_INLINE void hw_uart_enable(const HW_UART_ID uart)
{
        GLOBAL_INT_DISABLE();
        if (uart == HW_UART1) {
                REG_SET_BIT(CRG_SNC, SET_CLK_SNC_REG, UART_ENABLE);
        } else if (uart == HW_UART2) {
                REG_SET_BIT(CRG_SNC, SET_CLK_SNC_REG, UART2_ENABLE);
        } else if (uart == HW_UART3) {
                REG_SET_BIT(CRG_SNC, SET_CLK_SNC_REG, UART3_ENABLE);
        }
        GLOBAL_INT_RESTORE();
}

HW_UART_CONFIG_ERR hw_uart_init_ex(HW_UART_ID uart, const uart_config_ex *uart_init)
{
        UART_Data *ud = UARTDATA(uart);
        bool use_fifo;
#if (HW_UART_DMA_SUPPORT == 1)
        bool use_burst;
#endif /* HW_UART_DMA_SUPPORT */
        HW_UART_CONFIG_ERR error  = HW_UART_CONFIG_ERR_NOERR;


        /*
         * Read UART_USR_REG to clear any pending busy interrupt.
         */
        hw_uart_enable(uart);
        hw_uart_transmit_fifo_empty(uart);
        use_fifo = (uart_init->use_fifo);
#if (HW_UART_DMA_SUPPORT == 1)
        use_burst = (uart_init->rx_dma_burst_lvl || uart_init->tx_dma_burst_lvl);

        if (use_burst && use_fifo && uart_init->use_dma) {
                if ((error = fifo_dma_burst_not_match(uart_init))) {
                     return error;
                }
        }
#endif /* HW_UART_DMA_SUPPORT */

        if (use_fifo) {
                ud->rx_fifo_on = 1;
                ud->tx_fifo_on = 1;
                hw_uart_enable_fifo(uart);
                ud->rx_fifo_level = uart_init->rx_fifo_tr_lvl;
                hw_uart_rx_fifo_tr_lvl_setf(uart, uart_init->rx_fifo_tr_lvl);
                ud->tx_fifo_level = uart_init->tx_fifo_tr_lvl;
                hw_uart_tx_fifo_tr_lvl_setf(uart, uart_init->tx_fifo_tr_lvl);
        } else {
                ud->rx_fifo_on = 0;
                ud->tx_fifo_on = 0;
                hw_uart_disable_fifo(uart);
        }

        hw_uart_baudrate_set(uart, uart_init->baud_rate);

        // Set Parity
        UBA(uart)->UART2_LCR_REG = (uart_init->parity) << 3;
        // Set Data Bits
        HW_UART_REG_SETF(uart, LCR, UART_DLS, uart_init->data);
        // Set Stop Bits
        HW_UART_REG_SETF(uart, LCR, UART_STOP, uart_init->stop);
        // Set Auto flow control
        HW_UART_REG_SETF(uart, MCR, UART_AFCE, uart_init->auto_flow_control);
        HW_UART_REG_SETF(uart, MCR, UART_RTS, uart_init->auto_flow_control);
        ud->tx_cb = NULL;
        ud->rx_cb = NULL;
        ud->rx_len = 0;
        ud->tx_len = 0;
#if (HW_UART_DMA_SUPPORT == 1)
        ud->use_dma = 0;
        ud->rx_dma.channel_number = HW_DMA_CHANNEL_INVALID;
        ud->tx_dma.channel_number = HW_DMA_CHANNEL_INVALID;
        if (uart_init->use_dma) {
                hw_uart_configure_dma_channels(uart, uart_init);
        }
#endif
        return error;
}

void hw_uart_reinit_ex(HW_UART_ID uart, const uart_config_ex *uart_init)
{
        UART_Data *ud = UARTDATA(uart);
        bool use_fifo;

        hw_uart_enable(uart);

        /*
         * Read UART_USR_REG to clear any pending busy interrupt.
         */
        hw_uart_transmit_fifo_empty(uart);

        use_fifo = (uart_init->use_fifo == 1);

        if (use_fifo) {
                hw_uart_enable_fifo(uart);
                hw_uart_rx_fifo_tr_lvl_setf(uart, uart_init->rx_fifo_tr_lvl);
                hw_uart_tx_fifo_tr_lvl_setf(uart, uart_init->tx_fifo_tr_lvl);
        } else {
               hw_uart_disable_fifo(uart);
        }

        hw_uart_baudrate_set(uart, uart_init->baud_rate);

        // Set Parity
        UBA(uart)->UART2_LCR_REG = (uart_init->parity) << 3;
        // Set Data Bits
        HW_UART_REG_SETF(uart, LCR, UART_DLS, uart_init->data);
        // Set Stop Bits
        HW_UART_REG_SETF(uart, LCR, UART_STOP, uart_init->stop);
        // Set Auto flow control
        HW_UART_REG_SETF(uart, MCR, UART_AFCE, uart_init->auto_flow_control);
        HW_UART_REG_SETF(uart, MCR, UART_RTS, uart_init->auto_flow_control);

        if (ud->rx_cb && ud->rx_len != ud->rx_ix) {
#if (HW_UART_DMA_SUPPORT == 1)
                if (ud->rx_len > 1 && uart_init->use_dma && uart_init->rx_dma_channel != HW_DMA_CHANNEL_INVALID) {

                } else
#endif /* HW_UART_DMA_SUPPORT */
                {
                        // Interrupt driven
                        hw_uart_enable_rx_int(uart, true);
                }
        }
}

void hw_uart_init(HW_UART_ID uart, const uart_config *uart_init)
{
        UART_Data *ud = UARTDATA(uart);
        bool use_fifo;

        use_fifo = (uart_init->use_fifo == 1);

        /*
         * Read UART_USR_REG to clear any pending busy interrupt.
         */
        hw_uart_enable(uart);
        hw_uart_transmit_fifo_empty(uart);

        if (!use_fifo) {
                ud->rx_fifo_on = 0;
                ud->tx_fifo_on = 0;
                hw_uart_disable_fifo(uart);
        } else {
                ud->rx_fifo_on = 1;
                ud->tx_fifo_on = 1;
                hw_uart_enable_fifo(uart);
                hw_uart_rx_fifo_tr_lvl_setf(uart, 0);
                hw_uart_tx_fifo_tr_lvl_setf(uart, 0);
        }

        hw_uart_baudrate_set(uart, uart_init->baud_rate);

        // Set Parity
        UBA(uart)->UART2_LCR_REG = (uart_init->parity) << 3;
        // Set Data Bits
        HW_UART_REG_SETF(uart, LCR, UART_DLS, uart_init->data);
        // Set Stop Bits
        HW_UART_REG_SETF(uart, LCR, UART_STOP, uart_init->stop);
        // Set Auto flow control
        HW_UART_REG_SETF(uart, MCR, UART_AFCE, uart_init->auto_flow_control);
        HW_UART_REG_SETF(uart, MCR, UART_RTS, uart_init->auto_flow_control);
        ud->tx_cb = NULL;
        ud->rx_cb = NULL;
        ud->rx_len = 0;
        ud->tx_len = 0;
#if (HW_UART_DMA_SUPPORT == 1)
        ud->use_dma = 0;
        ud->rx_dma.channel_number = HW_DMA_CHANNEL_INVALID;
        ud->tx_dma.channel_number = HW_DMA_CHANNEL_INVALID;
        if (uart_init->use_dma) {
                uart_config_ex tiny_uart_ex; /* hw_uart_configure_dma_channels only need dma channel info and burst mode level*/
                tiny_uart_ex.tx_dma_channel  = uart_init->tx_dma_channel;
                tiny_uart_ex.rx_dma_channel  = uart_init->rx_dma_channel;
                tiny_uart_ex.dma_prio.use_prio = false;
                tiny_uart_ex.tx_dma_burst_lvl = 0;
                tiny_uart_ex.rx_dma_burst_lvl = 0;
                hw_uart_configure_dma_channels(uart, &tiny_uart_ex);
        }
#endif /* HW_UART_DMA_SUPPORT */
}

void hw_uart_reinit(HW_UART_ID uart, const uart_config *uart_init)
{
        UART_Data *ud = UARTDATA(uart);
        bool use_fifo;

        use_fifo = (uart_init->use_fifo == 1);

        hw_uart_enable(uart);

        /*
         * Read UART_USR_REG to clear any pending busy interrupt.
         */
        hw_uart_transmit_fifo_empty(uart);

        if (use_fifo) {
                hw_uart_enable_fifo(uart);
                hw_uart_rx_fifo_tr_lvl_setf(uart, 0);
                hw_uart_tx_fifo_tr_lvl_setf(uart, 0);
        } else {
                hw_uart_disable_fifo(uart);
        }

        hw_uart_baudrate_set(uart, uart_init->baud_rate);

        // Set Parity
        UBA(uart)->UART2_LCR_REG = (uart_init->parity) << 3;
        // Set Data Bits
        HW_UART_REG_SETF(uart, LCR, UART_DLS, uart_init->data);
        // Set Stop Bits
        HW_UART_REG_SETF(uart, LCR, UART_STOP, uart_init->stop);
        // Set Auto flow control
        HW_UART_REG_SETF(uart, MCR, UART_AFCE, uart_init->auto_flow_control);
        HW_UART_REG_SETF(uart, MCR, UART_RTS, uart_init->auto_flow_control);

        if (ud->rx_cb && ud->rx_len != ud->rx_ix) {
#if (HW_UART_DMA_SUPPORT == 1)
                if (ud->rx_len > 1 && uart_init->use_dma && uart_init->rx_dma_channel != HW_DMA_CHANNEL_INVALID) {
                } else
#endif  /* HW_UART_DMA_SUPPORT */
                {
                        // Interrupt driven
                        hw_uart_enable_rx_int(uart, true);
                }
        }
}

void hw_uart_deinit(HW_UART_ID uart)
{
        GLOBAL_INT_DISABLE();

        NVIC_DisableIRQ(UART_INT(uart));
        NVIC_ClearPendingIRQ(UART_INT(uart));

        //ASSERT_ERROR(hw_pd_com_is_up());
        /* Reset the controller */
        HW_UART_REG_SETF(uart, SRR, UART_UR, 1);
        HW_UART_REG_SETF(uart, SRR, UART_UR, 0);
        /* Disable clocks */
        if (uart == HW_UART1) {
                REG_SET_BIT(CRG_SNC, RESET_CLK_SNC_REG, UART_ENABLE);
        } else if (uart == HW_UART2) {
                REG_SET_BIT(CRG_SNC, RESET_CLK_SNC_REG, UART2_ENABLE);
        } else {
                REG_SET_BIT(CRG_SNC, RESET_CLK_SNC_REG, UART3_ENABLE);
        }

        GLOBAL_INT_RESTORE();
}


void hw_uart_cfg_get(HW_UART_ID uart, uart_config *uart_cfg)
{
#if (HW_UART_DMA_SUPPORT == 1)
        UART_Data *ud = UARTDATA(uart);
#endif

        uart_cfg->baud_rate = hw_uart_baudrate_get(uart);

        // Fill-in the rest of the configuration settings
        uart_cfg->data = HW_UART_REG_GETF(uart, LCR, UART_DLS);
        uart_cfg->parity = UBA(uart)->UART2_LCR_REG;
        uart_cfg->parity &= ((1 << UART_UART_LCR_REG_UART_EPS_Pos) | (1 << UART_UART_LCR_REG_UART_PEN_Pos));
        uart_cfg->parity = uart_cfg->parity >> UART_UART_LCR_REG_UART_PEN_Pos;
        uart_cfg->stop = HW_UART_REG_GETF(uart, LCR, UART_STOP);
#if HW_UART_DMA_SUPPORT
        uart_cfg->tx_dma_channel = ud->tx_dma.channel_number;
        uart_cfg->rx_dma_channel = ud->rx_dma.channel_number;
        uart_cfg->use_dma = ud->use_dma;
#endif
        uart_cfg->auto_flow_control = hw_uart_afce_getf(uart);
}

//=========================== Modem control functions ==========================


uint8_t hw_uart_afce_getf(HW_UART_ID uart)
{
        // Get the value of the AFCE bit from the Modem Control Register
        return 0xFF & HW_UART_REG_GETF(uart, MCR, UART_AFCE);
}

void hw_uart_afce_setf(HW_UART_ID uart, uint8_t afce)
{
        // Set the value of the AFCE bit in the Modem Control Register
        HW_UART_REG_SETF(uart, MCR, UART_AFCE, afce);
}

uint8_t hw_uart_loopback_getf(HW_UART_ID uart)
{
        // Get the value of the loop back (LB) bit from the Modem Control Register
        return (uint8_t) HW_UART_REG_GETF(uart, MCR, UART_LB);
}

void hw_uart_loopback_setf(HW_UART_ID uart, uint8_t lb)
{
        // Set the value of the loop back (LB) bit in the Modem Control Register
        HW_UART_REG_SETF(uart, MCR, UART_LB, lb);
}

uint8_t hw_uart_rts_getf(HW_UART_ID uart)
{
        // Get the value of the RTS bit from the Modem Control Register
        return 0xFF & HW_UART_REG_GETF(uart, MCR, UART_RTS);
}

void hw_uart_rts_setf(HW_UART_ID uart, uint8_t rtsn)
{
        // Set the value of the RTS bit in the Modem Control Register
        HW_UART_REG_SETF(uart, MCR, UART_RTS, rtsn);
}

//=========================== Line status functions ============================

HW_UART_ERROR hw_uart_error_getf(HW_UART_ID uart)
{
        /* Read Line Status Register once because errors are cleared after reading it. */
        uint32_t lsr = ((UART2_Type *)uart)->UART2_LSR_REG;

        if (REG_GET_FIELD(UART2, UART2_LSR_REG, UART_OE, lsr)) {
                return HW_UART_ERR_OE;
        }

        if (REG_GET_FIELD(UART2, UART2_LSR_REG, UART_PE, lsr)) {
                return HW_UART_ERR_PE;
        }

        if (REG_GET_FIELD(UART2, UART2_LSR_REG, UART_FE, lsr)) {
                return HW_UART_ERR_FE;
        }

        if (REG_GET_FIELD(UART2, UART2_LSR_REG, UART_BI, lsr)) {
                return HW_UART_ERR_BI;
        }

        if (REG_GET_FIELD(UART2, UART2_LSR_REG, UART_RFE, lsr)) {
                return HW_UART_ERR_RFE;
        }
        return HW_UART_ERR_NOERROR;
}

uint8_t hw_uart_rx_fifo_err_getf(HW_UART_ID uart)
{
        // Get Receiver FIFO Error bit
        return (uint8_t) HW_UART_REG_GETF(uart, LSR, UART_RFE);
}

uint8_t hw_uart_is_tx_fifo_empty(HW_UART_ID uart)
{
        // Get Transmitter Empty bit from Line Status Register
        return HW_UART_REG_GETF(uart, LSR, UART_TEMT) != 0;
}

uint8_t hw_uart_thr_empty_getf(HW_UART_ID uart)
{
        // Get Transmit Holding Register Empty bit value from Line Status Register
        return HW_UART_REG_GETF(uart, LSR, UART_THRE);
}

uint8_t hw_uart_break_int_getf(HW_UART_ID uart)
{
        // Get Break Interrupt bit value from Line Status Register
        return HW_UART_REG_GETF(uart, LSR, UART_BI);
}

uint8_t hw_uart_frame_err_getf(HW_UART_ID uart)
{
        // Get Framing Error bit value from Line Status Register
        return HW_UART_REG_GETF(uart, LSR, UART_FE);
}

uint8_t hw_uart_parity_err_getf(HW_UART_ID uart)
{
        // Get Parity Error bit value from Line Status Register
        return HW_UART_REG_GETF(uart, LSR, UART_PE);
}

uint8_t hw_uart_overrun_err_getf(HW_UART_ID uart)
{
        // Get Overrun Error bit value from Line Status Register
        return HW_UART_REG_GETF(uart, LSR, UART_OE);
}

//=========================== Modem status functions ===========================

uint8_t hw_uart_cts_getf(HW_UART_ID uart)
{
        // Get CTS bit from Modem Control Register
        return (uint8_t) HW_UART_REG_GETF(uart, MSR, UART_CTS);
}

uint8_t hw_uart_delta_cts_getf(HW_UART_ID uart)
{
        // Get the DCTS bit value from the Modem Control Register
        return (uint8_t) HW_UART_REG_GETF(uart, MSR, UART_DCTS);
}

bool hw_uart_tx_in_progress(const HW_UART_ID uart)
{
        UART_Data *ud = UARTDATA(uart);
        return ud->tx_cb != NULL;
}

bool hw_uart_rx_in_progress(const HW_UART_ID uart)
{
        UART_Data *ud = UARTDATA(uart);
        return ud->rx_cb != NULL;
}

#endif /* dg_configUSE_HW_UART */

