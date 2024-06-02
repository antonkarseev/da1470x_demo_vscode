/**
 ****************************************************************************************
 *
 * @file hw_usb_v2.c
 *
 * @brief Low level USB driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_USB

/*========================== Include files ==================================*/

#include "hw_usb.h"
#if (HW_USB_DMA_SUPPORT == 1)
#include "hw_dma.h"
#endif
#include "hw_clk.h"
#include "sys_power_mgr.h"
#include "sys_clock_mgr.h"
#include "qspi_automode.h"
/*========================== Local macro definitions ========================*/

/*========================== Global definitions =============================*/

__RETAINED static hw_usb_vbus_cb_t      hw_usb_vbus_cb;
__RETAINED static hw_usb_usb_cb_t       hw_usb_usb_cb;

#if (dg_configUSE_USB_ENUMERATION == 1)

static hw_usb_ep_data_t                 usb_endpoints[USB_EP_MAX];
static volatile HW_USB_NFSR_TYPE        ud_nfsr;
static uint16_t                         int_masks;

#if (HW_USB_DMA_SUPPORT == 1)

/* DMA objects for the USB EP that will use DMA */
static DMA_setup                        usb_tx_dma;
static DMA_setup                        usb_rx_dma;
#endif /* HW_USB_DMA_SUPPORT */

#if (dg_configUSB_SUSPEND_MODE == USB_SUSPEND_MODE_PAUSE )
__RETAINED static uint32_t              iser0, iser1;
#endif

#define EP0_SETUP_PACKET_SIZE           8

/*========================== Local data definitions =========================*/

#define EPREGS(epc, txc, txs, txd, rxc, rxs, rxd) \
                (volatile uint16_t*)epc, (volatile uint16_t*)txc, (volatile uint16_t*)txs, \
                (volatile uint16_t*)txd, (volatile uint16_t*)rxc, (volatile uint16_t*)rxs, \
                (volatile uint16_t*)rxd

/* Table for looking up endpoint registers */
static const hw_usb_ep_regs_t ep_regs[] = {
        { EPREGS(&(USB->USB_EPC0_REG),
                        &(USB->USB_TXC0_REG), &(USB->USB_TXS0_REG), &(USB->USB_TXD0_REG),
                        &(USB->USB_RXC0_REG), &(USB->USB_RXS0_REG), &(USB->USB_RXD0_REG)) },
        { EPREGS(&(USB->USB_EPC1_REG),
                        &(USB->USB_TXC1_REG), &(USB->USB_TXS1_REG), &(USB->USB_TXD1_REG),
                        0, 0, 0) },
        { EPREGS(&(USB->USB_EPC2_REG),
                        0, 0, 0,
                        &(USB->USB_RXC1_REG), &(USB->USB_RXS1_REG), &(USB->USB_RXD1_REG)) },
        { EPREGS(&(USB->USB_EPC3_REG),
                        &(USB->USB_TXC2_REG), &(USB->USB_TXS2_REG), &(USB->USB_TXD2_REG),
                        0, 0, 0) },
        { EPREGS(&(USB->USB_EPC4_REG),
                        0, 0, 0,
                        &(USB->USB_RXC2_REG), &(USB->USB_RXS2_REG), &(USB->USB_RXD2_REG)) },
        { EPREGS(&(USB->USB_EPC5_REG),
                        &(USB->USB_TXC3_REG), &(USB->USB_TXS3_REG), &(USB->USB_TXD3_REG),
                        0, 0, 0) },
        { EPREGS(&(USB->USB_EPC6_REG),
                        0, 0, 0,
                        &(USB->USB_RXC3_REG), &(USB->USB_RXS3_REG), &(USB->USB_RXD3_REG)) },
};

#undef EPREGS

/*========================== Function definitions ===========================*/

/* **************************************************************************************************************************************
 *
 *                                    FUNCTIONS FOR USB DATA EVENTS
 *
 * **************************************************************************************************************************************/

/**
 * \brief Disable USB interrupt.
 *
 * \return The last USB interrupt state.
 *
 */
static uint8_t save_usb_int(void)
{
        uint8_t state;

        state = REG_GETF(USB, USB_MAMSK_REG, USB_M_INTR);
        REG_CLR_BIT(USB, USB_MAMSK_REG, USB_M_INTR);

        return state;
}

/**
 * \brief Restore USB interrupt to previous state.
 *
 * \param[in] state The previous USB interrupt state.
 *
 */
__STATIC_FORCEINLINE void restore_usb_int(uint8_t state)
{
        REG_SETF(USB, USB_MAMSK_REG, USB_M_INTR, state);
}

#if (HW_USB_DMA_SUPPORT == 1)
__STATIC_FORCEINLINE void tx_ep_dma_transfer(hw_usb_ep_data_t* ep, uint8_t ep_nr, uint8_t *pd, uint16_t length)
{
        /* Enable the TX DMA operation in the USB MAC */
        REG_SETF(USB, USB_DMA_CTRL_REG, USB_DMA_TX, (ep_nr >> 1));

        /* The destination address is always the TXD of the selected USB EP*/
        usb_tx_dma.dest_address = (uint32_t) ep_regs[ep_nr].txd;

        /* Set the src address */
        usb_tx_dma.src_address = (uint32) pd;

        /* Set the length */
        usb_tx_dma.length = length;

        usb_tx_dma.user_data = (void*)((uint32_t)ep_nr);

        /* initialize the DMA */
        hw_dma_channel_initialization(&usb_tx_dma);

        /* Enable the DMA */
        hw_dma_channel_enable(usb_tx_dma.channel_number, HW_DMA_STATE_ENABLED);
}
#endif /* HW_USB_DMA_SUPPORT */

__STATIC_FORCEINLINE void tx_ep_transfer(hw_usb_ep_data_t* ep, uint8_t ep_nr, uint8_t *pd, uint16_t *length)
{
        volatile uint16_t *txc = ep_regs[ep_nr].txc;

        if (ep->flags.toggle) {
                *txc |= TXC_TOGGLE;
        } else {
                *txc &= ~TXC_TOGGLE;
        }

#if (HW_USB_DMA_SUPPORT == 1)
        if (usb_tx_dma.callback) {
                if ((ep_nr != USB_EP_DEFAULT) && (*length >= 1)) {
                        /* Already handle by tx_ep_dma_transfer */
                        tx_ep_dma_transfer(ep, ep_nr, pd, *length);
                        return;
                }
        }
#endif /* HW_USB_DMA_SUPPORT */

        volatile const uint16_t *txs = ep_regs[ep_nr].txs;
        volatile uint16_t *txd = ep_regs[ep_nr].txd;
        uint16_t remain = *length;
        uint16_t tcount = 0;

        do {
                tcount = *txs & TXS_TCOUNT_MASK;
                uint16_t n = MIN(tcount, remain);

                remain -= n;

                while (n--) {
                        *txd = *pd++;
                }
        } while (tcount && remain);

        *length -= remain;

        if (ep_nr == USB_EP_DEFAULT) {
                if (*(ep_regs[USB_EP_DEFAULT].rxc) & RXC_RX_EN) {
                        uint8_t state = save_usb_int();
                        *(ep_regs[USB_EP_DEFAULT].rxc) &= ~RXC_RX_EN;
                        restore_usb_int(state);
                }
        }
        *txc |= TXC_LAST | TXC_TX_EN;
}

/**
 * \brief Fill TX buffer for the endpoint.
 *
 * \param[in] ep_nr The endpoint number.
 *
 */
static void tx_fill(uint8_t ep_nr)
{
        hw_usb_ep_data_t* ep = &usb_endpoints[ep_nr];
        uint8_t *pd = ep->tx.buffer + ep->tx.actual_size;
        uint16_t saved = MIN(ep->tx.max_size - ep->tx.actual_size, ep->mps);

        tx_ep_transfer(ep, ep_nr, pd, &saved);

        /* save current values in case you want to retry after NAK */
        ep->tx.packet_size_txfill = ep->tx.packet_size;
        ep->tx.actual_size_txfill = ep->tx.actual_size;
        ep->tx.packet_size = saved;
        ep->tx.actual_size += saved;
}

static void tx_fill_retry(uint8_t ep_nr)
{
        hw_usb_ep_data_t* ep = &usb_endpoints[ep_nr];
        ep->tx.packet_size = ep->tx.packet_size_txfill;
        ep->tx.actual_size = ep->tx.actual_size_txfill;

        tx_fill(ep_nr);
}

static void tx_done(uint8_t ep_nr, hw_usb_ep_data_t* ep)
{
        if (ep->flags.tx_busy) {
                ep->flags.tx_busy = 0;
                hw_usb_ep_tx_done(ep_nr, ep->tx.buffer);
        } else {
                /*
                 * Even though TX was not active, indicate TxDone anyway.
                 * Useful for isochronous transfers...
                 */
                hw_usb_ep_tx_done(ep_nr, NULL);
        }
}

void hw_usb_tx_ep(uint8_t ep_nr)
{
        /* End point number should be lower than MAX EP */
        ASSERT_WARNING(ep_nr < USB_EP_MAX);

        hw_usb_ep_data_t* ep = &usb_endpoints[ep_nr];
        const hw_usb_ep_regs_t* er = &ep_regs[ep_nr];
        volatile uint16_t txs = *(er->txs);

        if (txs & TXS_TX_DONE) {
                if ((txs & TXS_ACK_STAT) ||
                    (ep->flags.type == HW_USB_DEVICE_FRAMEWORK_ENDPOINT_XFER_ISOC)) {
                        ep->flags.toggle = !ep->flags.toggle;
                        if (ep->tx.actual_size < ep->tx.max_size) {
                                tx_fill(ep_nr);
                        } else if ((ep->flags.zero_terminate) &&
                                   (ep->tx.packet_size == ep->mps)) {
                                tx_fill(ep_nr);
                        } else {
                                tx_done(ep_nr, ep);
                        }
                } else {
                        if (ep->flags.tx_busy) {
                                /* If we didn't get an ACK, refill FIFO */
                                ep->flags.tx_busy = 0;
                                tx_fill_retry(ep_nr);
                        }
                }
        }
}

void hw_usb_tx_event(void)
{
        uint16_t txev;

        txev = USB->USB_TXEV_REG & USB->USB_TXMSK_REG;

        if (txev & 0x0001) {
                hw_usb_tx_ep(1);
        }

        if (txev & 0x0002) {
                hw_usb_tx_ep(3);
        }

        if (txev & 0x0004) {
                hw_usb_tx_ep(5);
        }
}

/**
 * \brief Check if RX is active for the endpoint and complete it.
 *
 * \param[in] ep_nr The endpoint number.
 * \param[in] ep The endpoint data.
 *
 * \return Always true.
 *
 */
static bool rx_done(uint8_t ep_nr, hw_usb_ep_data_t* ep)
{
        bool reenable = true;

        if (ep->rx.max_size) {
                ep->rx.max_size = 0;
                /* send it to the data to the upper USB stack/APP layers */
                reenable = hw_usb_ep_rx_done(ep_nr, ep->rx.buffer, ep->rx.actual_size);
                ep->rx.actual_size = 0;
        }

        return reenable;
}

#if (HW_USB_DMA_SUPPORT == 1)
static __INLINE void rx_ep_dma_transfer(hw_usb_ep_data_t* ep, uint8_t ep_nr, bool setup)
{
        volatile const uint16_t *rxs = ep_regs[ep_nr].rxs;

        /* Enable the RX DMA operation in the USB MAC */
        REG_SETF(USB, USB_DMA_CTRL_REG, USB_DMA_RX, ((ep_nr-1) >> 1));

        /* The source address is always the RXD of the available USB EP */
        usb_rx_dma.src_address = (uint32_t) ep_regs[ep_nr].rxd;

        /* Set the dest address for the DMA */
        usb_rx_dma.dest_address = (uint32) ep->rx.buffer;

        /*
         * Program the max DMA size.
         * If there will be less than programmed data received
         * then cancel the DMA later.
         */

        /* Program the max DMA size */
        usb_rx_dma.length = ep->rx.max_size;

        /* Initialize the DMA structures and set the values to registers */
        hw_dma_channel_initialization(&usb_rx_dma);

        /* Start the DMA */
        hw_dma_channel_enable(usb_rx_dma.channel_number, HW_DMA_STATE_ENABLED);

        /*
         * Wait for the completion of the transfer.
         * When *rxs == 0 all data has been transfered
         */
        while ((*rxs) >> USB_USB_RXS1_REG_USB_RCOUNT_Pos);

        if (!hw_dma_is_channel_active(usb_rx_dma.channel_number)) {
                /* if the DMA is completed then the transfered bytes equals the size of the programmed DMA */
                ep->rx.actual_size = ep->rx.max_size;
        } else {
                /* if the DMA is not completed then there were less bytes to transfer
                 * than the programmed DMA size.
                 * get the number of transferred bytes from the DMA index register
                 */
                ep->rx.actual_size = hw_dma_transfered_bytes(usb_rx_dma.channel_number);

                /* stop the pending DMA */
                hw_dma_channel_enable(usb_rx_dma.channel_number, HW_DMA_STATE_DISABLED);
        }
}
#endif /* HW_USB_DMA_SUPPORT */

static __INLINE void rx_ep_transfer(hw_usb_ep_data_t* ep, uint8_t ep_nr, bool setup)
{
        volatile const uint16_t *rxs = ep_regs[ep_nr].rxs;
        volatile uint16_t rxsize = *rxs & RXS_RCOUNT_MASK;

        /* Prepare the buffer to transfer the data. One buffer is used by default */
        ep->rx.buffer = hw_usb_ep_get_rx_buffer(ep_nr, setup, &ep->rx.max_size);

#if (HW_USB_DMA_SUPPORT == 1)
        if (usb_rx_dma.callback) {
                if ((ep_nr != USB_EP_DEFAULT)) {
                        rx_ep_dma_transfer(ep, ep_nr, setup);
                        return;
                }
        }
#endif /* HW_USB_DMA_SUPPORT */
        volatile const uint16_t *rxd = ep_regs[ep_nr].rxd;
         if (rxsize > 0) {
                 ep->rx.actual_size = rxsize;
                 uint8_t *pb = ep->rx.buffer;

                 while (rxsize--) {
                         *pb++ = *rxd;
                 }
         } else {
                 ep->rx.actual_size = 0;
         }
}

/**
 * \brief Read RX data from endpoint FIFO.
 *
 * \param[in] ep_nr The endpoint number.
 * \param[in] setup Indication of SETUP packet type.
 *
 */
static void rx_ep_read(uint8_t ep_nr, bool setup)
{
        bool reenable = true;
        hw_usb_ep_data_t* ep = &usb_endpoints[ep_nr];
        volatile uint16_t *rxc = ep_regs[ep_nr].rxc;

        rx_ep_transfer(ep, ep_nr, setup);

        if (ep->rx.actual_size < ep->mps) {
                reenable = rx_done(ep_nr, ep);
        } else {
                if (ep->rx.actual_size == ep->rx.max_size) {
                        if (ep_nr != USB_EP_DEFAULT && ep->flags.zero_terminate) {
                                /* Wait for zero length packet.*/
                                reenable = true;
                        } else {
                                reenable = rx_done(ep_nr, ep);
                        }
                }
        }

        if (reenable) {
                /* Enable the USB EP */
                *rxc |= RXC_RX_EN;
        }
}

void hw_usb_rx_ep0(void)
{
        hw_usb_ep_data_t* ep = usb_endpoints;
        volatile uint16_t rxs = USB->USB_RXS0_REG;

        if (rxs & RXS_RX_LAST) {
                if (rxs & RXS_SETUP) {
                        if ((rxs & RXS_RCOUNT_MASK) == EP0_SETUP_PACKET_SIZE) {
                                USB->USB_EPC0_REG = USB->USB_EPC0_REG & (~EPC_STALL);
                                ep->flags.toggle = 1;
                                rx_ep_read(USB_EP_DEFAULT, true);
                        } else {
                                hw_usb_ep0_stall();
                        }
                } else {
                        if (rxs & RXS_RCOUNT_MASK) {
                                rx_ep_read(USB_EP_DEFAULT, false);
                        }
                }
        }
}

/**
 * \brief Receive on endpoint.
 *
 * \param[in] ep_nr The endpoint number.
 *
 */
static uint8_t rx_ep(uint8_t ep_nr)
{
        /* End point number should be lower than MAX EP */
        ASSERT_WARNING(ep_nr < USB_EP_MAX);

        hw_usb_ep_data_t* ep = &usb_endpoints[ep_nr];
        volatile uint16_t *rxc = ep_regs[ep_nr].rxc;
        uint16_t rxs = *(ep_regs[ep_nr].rxs);

        if (rxs & RXS_RX_ERR) {
                *rxc |= RXC_FLUSH;
                return rxs;
        }

        if (rxs & RXS_RX_LAST) {
                if (rxs & RXS_TOGGLE) {
                        ep->flags.toggle = 0;
                } else {
                        ep->flags.toggle = 1;
                }
                rx_ep_read(ep_nr, false);
        }

        return rxs;
}

void hw_usb_rx_event(void)
{
        uint16_t rxev;

        rxev = USB->USB_RXEV_REG & USB->USB_RXMSK_REG;

        if (rxev & 0x0001) {
                rx_ep(2);
        }

        if (rxev & 0x0002) {
                rx_ep(4);
        }

        if (rxev & 0x0004) {
                rx_ep(6);
        }
}

void hw_usb_nak_event_ep0(void)
{
        if (REG_GETF(USB, USB_EP0_NAK_REG, USB_EP0_OUTNAK)) {
                hw_usb_ep_data_t* ep = &usb_endpoints[USB_EP_DEFAULT];

                if (ep->flags.tx_busy) {
                        hw_usb_ep_nak(0);
                }
        }
}

void hw_usb_nak_event(void)
{
        uint16_t nak;

        nak = USB->USB_NAKEV_REG & USB->USB_NAKMSK_REG;

        /* Endpoints for Rx */
        if (nak & 0x0001) {
                hw_usb_ep_nak(1);
        }

        if (nak & 0x0002) {
                hw_usb_ep_nak(3);
        }

        if (nak & 0x0004) {
                hw_usb_ep_nak(5);
        }

        /* Endpoints for Tx */
        if (nak & 0x0010) {
                hw_usb_ep_nak(2);
        }

        if (nak & 0x0020) {
                hw_usb_ep_nak(4);
        }

        if (nak & 0x0040) {
                hw_usb_ep_nak(6);
        }
}

#if (dg_configUSB_SUSPEND_MODE == USB_SUSPEND_MODE_PAUSE)
void hw_usb_disable_irqs_on_suspend(void)
{
        /*
         * Disable all IRQs except for VBUS and USB
         */
        iser0 = NVIC->ISER[0];
        iser1 = NVIC->ISER[1];

        NVIC->ICER[0] = iser0;
        NVIC->ICER[1] = iser1;
        __DSB();
        __ISB();

        NVIC_EnableIRQ(VBUS_IRQn);
        NVIC_EnableIRQ(USB_IRQn);
        NVIC_EnableIRQ(XTAL32M_RDY_IRQn);
}

void hw_usb_enable_irqs_on_resume(void)
{
        /*
         * Restore all IRQs
         */
        NVIC->ISER[0] = iser0;
        NVIC->ISER[1] = iser1;

        NVIC->ICER[0] = ~NVIC->ISER[0];
        NVIC->ICER[1] = ~NVIC->ISER[1];
}
#endif

/* **************************************************************************************************************************************
 *
 *                                    FUNCTIONS FOR VBUS EVENTS
 *
 * **************************************************************************************************************************************/
void hw_usb_sd3_event(void)
{
#if (dg_configUSB_SUSPEND_MODE == USB_SUSPEND_MODE_PAUSE)
        uint32_t critical_section_status = 0;
#endif

        REG_SET_BIT(USB, USB_ALTMSK_REG, USB_M_RESET);

        /* have the USB stop working with PLL */
        REG_SETF(CRG_TOP, CLK_CTRL_REG, USB_CLK_SRC, 1);

        if (ud_nfsr == HW_USB_NFSR_NODE_OPERATIONAL) {
                ud_nfsr = HW_USB_NFSR_NODE_SUSPEND;
                REG_SETF(USB, USB_NFSR_REG, USB_NFS, ud_nfsr);
                /* Reset USB_ALTMSK_REG */
                REG_CLR_BIT(USB, USB_ALTMSK_REG, USB_M_SD3);

                /* Save the current Interrupts mask.
                 * We will resoter it on RESUME event */
                int_masks = USB->USB_MAMSK_REG;
                /* While in SUSPEND mode, we do not want to have any
                 * data related interrupt.
                 * From SUSPENDED we expect to get RESUME or RESET events
                 * or device un-plug*/
                USB->USB_MAMSK_REG = 0;
                REG_SET_BIT(USB, USB_MAMSK_REG, USB_M_ALT);
                REG_SET_BIT(USB, USB_MAMSK_REG, USB_M_INTR);

#if (dg_configUSB_SUSPEND_MODE == USB_SUSPEND_MODE_PAUSE)
                OS_ENTER_CRITICAL_SECTION_FROM_ISR(critical_section_status);
                hw_usb_disable_irqs_on_suspend();
                OS_LEAVE_CRITICAL_SECTION_FROM_ISR(critical_section_status);
#endif /* dg_configUSB_SUSPEND_MODE */
        }
}

void hw_usb_sd5_event(void)
{
}

void hw_usb_reset_event(void)
{
        volatile uint32_t reg;

        /* have the USB working with PLL */
        REG_SETF(CRG_TOP, CLK_CTRL_REG, USB_CLK_SRC, 0);

        /* Configure interrupt sources. */
        REG_SETF(USB, USB_TXMSK_REG, USB_M_TXFIFO31, 0x7);
        REG_SETF(USB, USB_RXMSK_REG, USB_M_RXFIFO31, 0x7);

        reg = USB->USB_MAMSK_REG;
        REG_SET_FIELD(USB, USB_MAMSK_REG, USB_M_EP0_NAK, reg, 1);
        REG_SET_FIELD(USB, USB_MAMSK_REG, USB_M_EP0_RX, reg, 1);
        REG_SET_FIELD(USB, USB_MAMSK_REG, USB_M_EP0_TX, reg, 1);
        REG_SET_FIELD(USB, USB_MAMSK_REG, USB_M_RX_EV, reg, 1);
        REG_SET_FIELD(USB, USB_MAMSK_REG, USB_M_ULD, reg, 1);
        REG_CLR_FIELD(USB, USB_MAMSK_REG, USB_M_NAK, reg);
        REG_CLR_FIELD(USB, USB_MAMSK_REG, USB_M_FRAME, reg);
        REG_SET_FIELD(USB, USB_MAMSK_REG, USB_M_TX_EV, reg, 1);
        USB->USB_MAMSK_REG = reg;

        if (ud_nfsr == HW_USB_NFSR_NODE_SUSPEND) {
                hw_usb_restore_int_mask_at_resume();
        }

        ud_nfsr = HW_USB_NFSR_NODE_RESET;
        REG_SETF(USB, USB_NFSR_REG, USB_NFS, ud_nfsr);
        REG_CLR_BIT(USB, USB_ALTMSK_REG, USB_M_RESET);
        hw_clk_delay_usec(100); //0.1ms Delay
        ud_nfsr = HW_USB_NFSR_NODE_OPERATIONAL;
        REG_SETF(USB, USB_NFSR_REG, USB_NFS, ud_nfsr);
        hw_usb_bus_event(UBE_RESET);

        REG_SET_BIT(USB, USB_ALTMSK_REG, USB_M_SD3);
        REG_SET_BIT(USB, USB_ALTMSK_REG, USB_M_RESUME);
}

void hw_usb_resume_event(void)
{
        REG_SET_BIT(USB, USB_ALTMSK_REG, USB_M_RESET);

        /* have the USB working with PLL */
        REG_SETF(CRG_TOP, CLK_CTRL_REG, USB_CLK_SRC, 0);

        if (ud_nfsr == HW_USB_NFSR_NODE_SUSPEND) {
                ud_nfsr = HW_USB_NFSR_NODE_OPERATIONAL;
                REG_SETF(USB, USB_NFSR_REG, USB_NFS, ud_nfsr);
#if (dg_configUSB_SUSPEND_MODE == USB_SUSPEND_MODE_PAUSE)
                OS_ENTER_CRITICAL_SECTION();
                hw_usb_enable_irqs_on_resume();
                OS_LEAVE_CRITICAL_SECTION();
#endif

                hw_usb_restore_int_mask_at_resume();
        }

}

void hw_usb_frame_event(void)
{
        uint16_t frame;

        frame = USB->USB_FNL_REG;
        frame |= (REG_GETF(USB, USB_FNH_REG, USB_FN_10_8) << 8);

        hw_usb_bus_frame(frame);
        REG_SET_BIT(USB, USB_ALTMSK_REG, USB_M_RESET);
}

void hw_usb_restore_int_mask_at_resume(void)
{
        REG_SET_BIT(USB, USB_ALTMSK_REG, USB_M_SD3);
        USB->USB_MAMSK_REG = int_masks;
}

void hw_usb_bus_attach(void)
{
        uint32_t reg;
        uint8_t state;

        state = save_usb_int();

        reg = USB->USB_FAR_REG;

        REG_CLR_FIELD(USB, USB_FAR_REG, USB_AD, reg);
        REG_SET_FIELD(USB, USB_FAR_REG, USB_AD_EN, reg, 1);

        USB->USB_FAR_REG = reg;

        ud_nfsr = HW_USB_NFSR_NODE_RESET;
        USB->USB_NFSR_REG = ud_nfsr;

        /* Clear pending interrupts */
        (void)(USB->USB_ALTEV_REG);

        reg = USB->USB_ALTMSK_REG;
        REG_CLR_FIELD(USB, USB_ALTMSK_REG, USB_M_RESUME, reg);
        REG_SET_FIELD(USB, USB_ALTMSK_REG, USB_M_RESET, reg, 1);
        REG_CLR_FIELD(USB, USB_ALTMSK_REG, USB_M_SD5, reg);
        REG_CLR_FIELD(USB, USB_ALTMSK_REG, USB_M_SD3, reg);

        USB->USB_ALTMSK_REG = reg;

        reg = USB->USB_MAMSK_REG;
        REG_SET_FIELD(USB, USB_MAMSK_REG, USB_M_ALT, reg, 1);
        USB->USB_MAMSK_REG = reg;

        restore_usb_int(state);
}

void hw_usb_bus_detach(void)
{
        uint32_t reg;
        uint8_t state;

        state = save_usb_int();

        ud_nfsr = HW_USB_NFSR_NODE_RESET;
        USB->USB_NFSR_REG = ud_nfsr;

        REG_CLR_BIT(USB, USB_MCTRL_REG, USB_NAT);

        /* Reset USB_ALTMSK_REG */
        reg = USB->USB_ALTMSK_REG;
        REG_CLR_FIELD(USB, USB_ALTMSK_REG, USB_M_RESUME, reg);
        REG_CLR_FIELD(USB, USB_ALTMSK_REG, USB_M_RESET, reg);
        REG_CLR_FIELD(USB, USB_ALTMSK_REG, USB_M_SD5, reg);
        REG_CLR_FIELD(USB, USB_ALTMSK_REG, USB_M_SD3, reg);
        USB->USB_ALTMSK_REG = reg;

        restore_usb_int(state);
}

void hw_usb_bus_resume(void)
{
        /* not implemented yet */
        ASSERT_WARNING(false);
}

void hw_usb_bus_address(uint8_t address)
{
        uint32_t reg;
        uint8_t state = save_usb_int();

        REG_SET_BIT(USB, USB_EPC0_REG, USB_DEF);

        reg = USB->USB_FAR_REG;
        REG_SET_FIELD(USB, USB_FAR_REG, USB_AD, reg, address);
        REG_SET_FIELD(USB, USB_FAR_REG, USB_AD_EN, reg, 1);
        USB->USB_FAR_REG = reg;

        restore_usb_int(state);
}

void hw_usb_ep_configure(uint8_t ep_nr, bool zero_terminate,
                const hw_usb_device_framework_ep_descriptor_t* config)
{
        hw_usb_ep_data_t* ep = &usb_endpoints[ep_nr];
        volatile uint16_t *epc = ep_regs[ep_nr].epc;

        ep->flags.zero_terminate = zero_terminate;
        ep->flags.toggle = 0;

        if (config) {
                ep->flags.type = config->attributes & HW_USB_DEVICE_FRAMEWORK_ENDPOINT_XFERTYPE_MASK;
                ep->mps = config->max_packet_size;
                *epc = config->endpoint_address & EPC_EP_MASK;
                if (ep->flags.type == HW_USB_DEVICE_FRAMEWORK_ENDPOINT_XFER_ISOC) {
                        *epc |= EPC_ISO;
                        if ((config->endpoint_address & HW_USB_DEVICE_FRAMEWORK_ENDPOINT_DIR_MASK) == HW_USB_DEVICE_FRAMEWORK_DIR_IN) {
                                *(ep_regs[ep_nr].txc) |= TXC_IGN_ISOMSK;
                        }
                }
        } else {
                ep->flags.type = HW_USB_DEVICE_FRAMEWORK_ENDPOINT_XFER_CONTROL;
                ep->mps = USB_EP0_SIZE;
        }

        *epc |= EPC_EP_EN;
}

void hw_usb_ep0_stall(void)
{
        uint8_t state;

        hw_usb_ep_stall(USB_EP_DEFAULT);
        hw_usb_ep_tx_start(USB_EP_DEFAULT, NULL, 0);

        state = save_usb_int();
        usb_endpoints[0].flags.tx_busy = 0;
        restore_usb_int(state);
}

void hw_usb_ep_stall(uint8_t ep_nr)
{
        volatile uint16_t *epc = ep_regs[ep_nr].epc;
        uint8_t state;

        state = save_usb_int();
        *epc |= EPC_STALL;
        restore_usb_int(state);
}

void hw_usb_ep_unstall(uint8_t ep_nr)
{
        hw_usb_ep_data_t* ep = &usb_endpoints[ep_nr];
        volatile uint16_t *epc = ep_regs[ep_nr].epc;
        uint8_t state;

        state = save_usb_int();
        *epc &= ~EPC_STALL;
        ep->flags.toggle = 0;
        restore_usb_int(state);
}

bool hw_usb_ep_is_stalled(uint8_t ep_nr)
{
        return (bool) (*(ep_regs[ep_nr].epc) & EPC_STALL);
}

void hw_usb_ep_rx_enable(uint8_t ep_nr)
{
        hw_usb_ep_data_t* ep = &usb_endpoints[ep_nr];
        volatile uint16_t *rxc = ep_regs[ep_nr].rxc;
        uint8_t state;

        state = save_usb_int();
        if (ep->rx.max_size == 0) {
                if (ep_nr != USB_EP_DEFAULT) {
                        *rxc |= RXC_IGN_SETUP;
                }
                *rxc |= RXC_RX_EN;
        }
        restore_usb_int(state);
}

void hw_usb_ep_tx_start(uint8_t ep_nr, uint8_t *buffer, uint16_t size)
{
        hw_usb_ep_data_t* ep = &usb_endpoints[ep_nr];
        uint8_t state = save_usb_int();

        /* noise can cause loss of ACK/NAK IRQ for previous SETUP response, so avoid ASSERT for EP0 */
        if (ep_nr == 0) {
                ep->flags.tx_busy = 0;
        }

        /*
         * Previous USB-TX is not complete.
         * Application code must wait until the TX is complete before sending another buffer to USB.
         * Upon USB-TX operation completion, the AppUSBTxDataDone(...) is called.
         * Then the AppUSBTxData(...) is safe to be called for the next buffer.
         * To call multiple concurrent USB-TX use queues for the buffers to avoid overwriting.
         * The current implementation is single buffer thus the use of ep->flags.tx_busy flag.
         */
        ASSERT_WARNING(ep->flags.tx_busy == 0);
        ep->tx.max_size = size;
        ep->tx.actual_size = 0;
        ep->tx.buffer = buffer;
        ep->flags.tx_busy = 1;
        tx_fill(ep_nr);

        restore_usb_int(state);
}

void hw_usb_ep_disable(uint8_t ep_nr, bool clearToggle)
{
        hw_usb_ep_data_t* ep = &usb_endpoints[ep_nr];
        const hw_usb_ep_regs_t* er = &ep_regs[ep_nr];
        uint8_t state;

        state = save_usb_int();

        if (er->txc) {
                *(er->txc) &= ~TXC_TX_EN;
                *(er->txc) |= TXC_FLUSH;

                if (*(er->txs)) {
                        *(er->txs) = 0;
                }
                tx_done(ep_nr, ep);
        }

        if (er->rxc) {
                *(er->rxc) &= ~RXC_RX_EN;
                *(er->rxc) |= RXC_FLUSH;

                if (*(er->rxs)) {
                        *(er->rxs) = 0;
                }

                rx_done(ep_nr, ep);
        }

        if (clearToggle) {
                ep->flags.toggle = 0;
        }

        restore_usb_int(state);
}

/**
 * \brief Endpoint NAK control. Default enabled for EP0.
 *
 * \param[in] ep_nr The endpoint number.
 * \param[in] enable Set to true to generate NAK for the endpoint.
 *
 */
void hw_usb_ep_set_nak(uint8_t ep_nr, bool enable)
{
        uint8_t state;

        if (ep_nr == USB_EP_DEFAULT) {
                state = save_usb_int();

                if (enable) {
                        REG_SET_BIT(USB, USB_MAMSK_REG, USB_M_EP0_NAK);
                } else {
                        REG_CLR_BIT(USB, USB_MAMSK_REG, USB_M_EP0_NAK);
                }

                restore_usb_int(state);
        }
        else {
                uint32_t mask = 0x0101 << (ep_nr - 1);

                state = save_usb_int();

                if (enable) {
                        USB->USB_NAKMSK_REG |= mask;
                } else {
                        USB->USB_NAKMSK_REG &= (~mask);
                }

                restore_usb_int(state);
        }
}

void hw_usb_enable_interrupt(void)
{
        /* Enable interrupt. */
        REG_SET_BIT(USB, USB_MAMSK_REG, USB_M_INTR);

        NVIC_SetPriority(USB_IRQn, USB_INTERRUPT_PRIO);
        NVIC_ClearPendingIRQ(USB_IRQn);
        NVIC_EnableIRQ(USB_IRQn);
}

#if (HW_USB_DMA_SUPPORT == 1)

#define HW_USB_DMA_DEFAULT_RX_PRIO      (HW_DMA_PRIO_4)
#define HW_USB_DMA_DEFAULT_TX_PRIO      (HW_DMA_PRIO_5)

/**
 * \brief Endpoint DMA TX complete callback
 *
 * \param[in] user_data (= ep_nr) The endpoint number.
 * \param[in] len (is not used currently)
 *
 */
__RETAINED_CODE void hw_usb_dma_tx_cb(void *user_data, uint16_t len)
{
        uint32_t ep_nr = (uint32_t)user_data;
        volatile uint16_t *txc = ep_regs[ep_nr].txc;

        /* set the LAST bit and Enable the endpoint upon the data DMA transfer completion
         * to TX the USB packet transmission on the USB lines
         */
        *txc |= TXC_TX_EN | TXC_LAST;
}

/**
 * \brief Endpoint DMA RX complete callback
 *
 * \param[in] user_data (is not used currently, normally = ep_nr, the endpoint number).
 * \param[in] len (is not used currently)
 *
 */
__RETAINED_CODE void hw_usb_dma_rx_cb(void *user_data, uint16_t len)
{

}

static void hw_usb_dma_cfg(const usb_config *cfg)
{
        if (cfg && cfg->use_dma) {
                  /*
                   * prepare DMA structures to be used for DMA transfer initialization for each of the selected EP to use DMA
                   * The rest of the EPs will operate with traditional polling
                   * It is not possible to have more than one RX and one TX EP with enabled DMA at the same time.
                   * Check datasheet for details.
                   */

                  /* Check if the channel numbers are valid */
                  if (cfg->rx_dma_channel == HW_DMA_CHANNEL_INVALID ||
                      cfg->tx_dma_channel == HW_DMA_CHANNEL_INVALID) {
                          /* Invalidate DMA Channel for TX */
                          usb_tx_dma.channel_number = HW_DMA_CHANNEL_INVALID;
                          /* Invalidate DMA channel for RX */
                          usb_rx_dma.channel_number = HW_DMA_CHANNEL_INVALID;
                          return;
                  }

                  /*  DMA for the TX ep (D-->H)  */
                  //Odd channels are only applicable for MEMORY to USB endpoint
                  ASSERT_ERROR((cfg->tx_dma_channel & 0x1) == 1);

                  /* Select DMA Channel for TX */
                  usb_tx_dma.channel_number = cfg->tx_dma_channel;

                  /* For USB we must choose the BYTE transfer mode*/
                  usb_tx_dma.bus_width = HW_DMA_BW_BYTE;

                  /* We will use the Interrupt upon completion of the DMA to set the TX_LAST flag as fast as possible */
                  usb_tx_dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;

                  usb_tx_dma.irq_nr_of_trans = 0;

                  /* Write without peripheral triggering for the TX channel only */
                  usb_tx_dma.dreq_mode = HW_DMA_DREQ_START;

                  /* the source is the RAM buffer and it does increment*/
                  usb_tx_dma.a_inc = HW_DMA_AINC_TRUE;

                  /* the destination is the RXD of the USB EP and it does not increment*/
                  usb_tx_dma.b_inc = HW_DMA_BINC_FALSE;

                  /* use normal (not circular) DMA mode*/
                  usb_tx_dma.circular = HW_DMA_MODE_NORMAL;

                  /* Select the priority of the DMA*/
                  if (cfg->dma_prio.use_prio) {
                          usb_tx_dma.dma_prio = cfg->dma_prio.tx_prio;
                  } else {
                          usb_tx_dma.dma_prio = HW_USB_DMA_DEFAULT_TX_PRIO;
                  }

                  /* By default we select the blocking mode. Also HW_DMA_IDLE_INTERRUPTING_MODE can be used*/
                  usb_tx_dma.dma_idle = HW_DMA_IDLE_BLOCKING_MODE;

                  usb_tx_dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;

                  /* Select the MUX source*/
                  usb_tx_dma.dma_req_mux = HW_DMA_TRIG_USB_RXTX;

                  /* There will be no callback*/
                  usb_tx_dma.callback = (hw_dma_transfer_cb)hw_usb_dma_tx_cb;

                  /* Since there will be no callback, there will be no user data for the callback */
                  usb_tx_dma.user_data = NULL;

                  /*  DMA for the RX ep (H-->D)  */
                  //Even channels are only applicable for USB endpoint to MEMORY
                  ASSERT_ERROR((cfg->rx_dma_channel & 0x1) == 0);

                  /* Select DMA Channel for RX */
                  usb_rx_dma.channel_number = cfg->rx_dma_channel;

                  /* For USB we must choose the BYTE transfer mode */
                  usb_rx_dma.bus_width = HW_DMA_BW_BYTE;

                  /* We will not use the Interrupts upon completion of the DMA for easier synch of events */
                  usb_rx_dma.irq_enable = HW_DMA_IRQ_STATE_DISABLED;

                  usb_rx_dma.irq_nr_of_trans = 0;

                  /* triggered by the peripheral */
                  usb_rx_dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;

                  /* the source is the TXD of the USB EP and it does not increment */
                  usb_rx_dma.a_inc = HW_DMA_AINC_FALSE;

                  /* the destination is the RAM buffer and it does increment */
                  usb_rx_dma.b_inc = HW_DMA_BINC_TRUE;

                  /* use normal (not circular) DMA mode */
                  usb_rx_dma.circular = HW_DMA_MODE_NORMAL;

                  /* Select the priority of the DMA */
                  if (cfg->dma_prio.use_prio) {
                          /* Check prio validity */
                          ASSERT_ERROR((cfg->dma_prio.tx_prio <= HW_DMA_PRIO_7 && cfg->dma_prio.tx_prio >= HW_DMA_PRIO_0) ||
                                  (cfg->dma_prio.rx_prio <= HW_DMA_PRIO_7 && cfg->dma_prio.rx_prio >= HW_DMA_PRIO_0));

                          usb_rx_dma.dma_prio = cfg->dma_prio.rx_prio;
                  } else {
                          usb_rx_dma.dma_prio = HW_USB_DMA_DEFAULT_RX_PRIO;
                  }

                  /*
                   * By default we select the blocking mode.
                   * Also HW_DMA_IDLE_INTERRUPTING_MODE can be used
                   */
                  usb_rx_dma.dma_idle = HW_DMA_IDLE_BLOCKING_MODE;

                  usb_rx_dma.dma_init = HW_DMA_INIT_AX_BX_BY;

                  /* Select the MUX source */
                  usb_rx_dma.dma_req_mux = HW_DMA_TRIG_USB_RXTX;

                  /* There will be no callback */
                  usb_rx_dma.callback = (hw_dma_transfer_cb) hw_usb_dma_rx_cb;

                  /* Since there will be no callback, there will be no user data for the callback */
                  usb_rx_dma.user_data = NULL;

                  REG_SETF(USB, USB_DMA_CTRL_REG, USB_DMA_EN, 1);
        } else {
                  /* Invalidate DMA Channel for TX */
                  usb_tx_dma.channel_number = HW_DMA_CHANNEL_INVALID;
                  /* Invalidate DMA channel for RX */
                  usb_rx_dma.channel_number = HW_DMA_CHANNEL_INVALID;
        }
}
#endif /* (HW_USB_DMA_SUPPORT == 1) */

void hw_usb_cfg(const usb_config *cfg)
{
#if HW_USB_DMA_SUPPORT
        hw_usb_dma_cfg(cfg);
#endif
}

void hw_usb_disable(void) {
        hw_usb_disable_interrupt();
        REG_CLR_BIT(USB, USB_MCTRL_REG, USBEN);
}

#endif /* dg_configUSE_USB_ENUMERATION */

/**
 * \brief USB initialization
 *
 */
void hw_usb_init(void)
{
#if dg_configUSE_USB_ENUMERATION
        memset(usb_endpoints, 0, sizeof(usb_endpoints));
#endif

}

/* **************************************************************************************************************************************
 *                                              FUNCTIONS FOR INTERRUPT HANDLING
 * **************************************************************************************************************************************/
void hw_usb_enable_vbus_interrupt(hw_usb_vbus_cb_t cb)
{
        ASSERT_WARNING(cb);
        hw_usb_vbus_cb = cb;
        hw_usb_clear_vbus_irq();
        NVIC_ClearPendingIRQ(VBUS_IRQn);
        NVIC_EnableIRQ(VBUS_IRQn);
}

void hw_usb_disable_vbus_interrupt(void)
{
        hw_usb_clear_vbus_irq();
        NVIC_DisableIRQ(VBUS_IRQn);
        NVIC_ClearPendingIRQ(VBUS_IRQn);
        hw_usb_vbus_cb = NULL;
}

void VBUS_Handler(void)
{
        HW_USB_VBUS_IRQ_STAT status;

        status = hw_usb_get_vbus_mask_status();

        hw_usb_clear_vbus_irq();

        if (hw_usb_vbus_cb) {
                hw_usb_vbus_cb(status);
        }
}

void hw_usb_enable_usb_interrupt(hw_usb_usb_cb_t cb)
{
        ASSERT_WARNING(cb);
        hw_usb_usb_cb = cb;
        hw_usb_get_and_clear_usb_event_status();
        NVIC_ClearPendingIRQ(USB_IRQn);
        REG_SET_BIT(USB, USB_MAMSK_REG, USB_M_INTR);
        NVIC_EnableIRQ(USB_IRQn);

}

void hw_usb_disable_usb_interrupt(void)
{
        REG_CLR_BIT(USB, USB_MAMSK_REG, USB_M_INTR);
        hw_usb_get_and_clear_usb_event_status();
        NVIC_DisableIRQ(USB_IRQn);
        NVIC_ClearPendingIRQ(USB_IRQn);
        hw_usb_usb_cb = NULL;
}

void USB_Handler(void)
{
        uint32_t status;

        status = hw_usb_get_and_clear_usb_event_status();

        if (hw_usb_usb_cb) {
                hw_usb_usb_cb(status);
        }
}

uint32_t OS_IS_IN_INT(void)
{
        return __get_IPSR();
}

#endif  /* dg_configUSE_HW_USB */

/* End of file. */
