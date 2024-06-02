/**
 * \addtogroup PLA_DRI_PER_COMM
 * \{
 * \addtogroup USB_DRIVER USB Driver
 * \{
 * \brief USB Driver Controller
 */

/**
 ****************************************************************************************
 *
 * @file hw_usb_v2.h
 *
 * @brief header for low level USB driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef USBDRIVER_V2_H
#define USBDRIVER_V2_H


#if dg_configUSE_HW_USB

/*========================== Local macro definitions & typedefs =============*/

/* Endpoint Control Registers. */
#define EPC_EP_MASK     USB_USB_EPC1_REG_USB_EP_Msk
/* Not in EPC0. */
#define EPC_EP_EN       USB_USB_EPC1_REG_USB_EP_EN_Msk
/* Not in EPC0. */
#define EPC_ISO         USB_USB_EPC1_REG_USB_ISO_Msk
/* Only in EPC0.*/
#define EPC_DEF         USB_USB_EPC0_REG_USB_DEF_Msk
#define EPC_STALL       USB_USB_EPC0_REG_USB_STALL_Msk

/* TX status registers */
#define TXS_TCOUNT_MASK USB_USB_TXS0_REG_USB_TCOUNT_Msk
#define TXS_TX_DONE     USB_USB_TXS0_REG_USB_TX_DONE_Msk
#define TXS_ACK_STAT    USB_USB_TXS0_REG_USB_ACK_STAT_Msk
/* Not in TXS0 */
#define TXS_TX_URUN     USB_USB_TXS1_REG_USB_TX_URUN_Msk

/* Transmit Command Registers */
#define TXC_TX_EN       USB_USB_TXC1_REG_USB_TX_EN_Msk
#define TXC_LAST        USB_USB_TXC1_REG_USB_LAST_Msk
#define TXC_TOGGLE      USB_USB_TXC1_REG_USB_TOGGLE_TX_Msk
#define TXC_FLUSH       USB_USB_TXC1_REG_USB_FLUSH_Msk
#define TXC_RFF         USB_USB_TXC1_REG_USB_RFF_Msk
#define TXC_TFWL_MASK   USB_USB_TXC1_REG_USB_TFWL_Msk
#define TXC_TFWL_4      (0x1 << 5)
#define TXC_TFWL_8      (0x2 << 5)
#define TXC_TFWL_16     (0x3 << 5)
#define TXC_IGN_ISOMSK  USB_USB_TXC1_REG_USB_IGN_ISOMSK_Msk

/* Receive Status Registers */
#define RXS_RCOUNT_MASK USB_USB_RXS1_REG_USB_RCOUNT_Msk
#define RXS_RX_LAST     USB_USB_RXS1_REG_USB_RX_LAST_Msk
#define RXS_TOGGLE      USB_USB_RXS1_REG_USB_TOGGLE_RX_Msk
#define RXS_SETUP       USB_USB_RXS1_REG_USB_SETUP_Msk
#define RXS_RX_ERR      USB_USB_RXS1_REG_USB_RX_ERR_Msk

/* Receive Command Registers */
#define RXC_RX_EN       USB_USB_RXC0_REG_USB_RX_EN_Msk
#define RXC_IGN_OUT     USB_USB_RXC0_REG_USB_IGN_OUT_Msk
#define RXC_IGN_SETUP   USB_USB_RXC0_REG_USB_IGN_SETUP_Msk
#define RXC_FLUSH       USB_USB_RXC0_REG_USB_FLUSH_Msk
#define RXC_RFWL_MASK   USB_USB_RXC1_REG_USB_RFWL_Msk
#define RXC_RFWL_4      (0x1 << 5)
#define RXC_RFWL_8      (0x2 << 5)
#define RXC_RFWL_16     (0x3 << 5)

/**
 * \brief USB EP registers
 */
typedef struct {
        volatile uint16_t *epc;    /**< Enpoint number */
        volatile uint16_t *txc;    /**< Transmit FIFO Controller */
        volatile uint16_t *txs;    /**< TX status */
        volatile uint16_t *txd;    /**< TX data transfer */
        volatile uint16_t *rxc;    /**< Receive FIFO Controller */
        volatile uint16_t *rxs;    /**< RX status */
        volatile uint16_t *rxd;    /**< RX data transfer */
} hw_usb_ep_regs_t;

/**
 * \brief USB EP transfer flags
 */
typedef struct {
        uint8_t type :2;                /**< Type of USB transfer : Control, iso, bulk or interrupt. */
        uint8_t zero_terminate :1;      /**< Zero termination enabled means that transfers are the exact multiple of endpoint packet size */
        uint8_t toggle :1;              /**< Specifies the PID used when transmitting the packet */
        uint8_t spare :3;               /**< Reserved flag in TXCx register*/
        volatile uint8_t tx_busy :1;    /**< Indicate if TX buffer is busy. */
} hw_usb_ep_flags_t;


/**
 * \brief USB EP info structure
 *  Can be used for EP0, IN or OUT EP
 */
typedef struct {
        uint16_t max_size;              /**< max size of ep's fifo */
        uint16_t actual_size;           /**< actual size of sent data*/
        uint16_t actual_size_txfill;    /**< actual size of data that tx buffer is filled*/
        uint16_t  packet_size;           /**< The size of the packet*/
        uint16_t  packet_size_txfill;    /**< Packet size of TX buffer. */
        uint8_t *buffer;                /**< Pointer to the EP's buffer */
} hw_usb_ep_buffer_t;

/**
 * \brief USB EP structure to use with the driver
 *  Can be used for IN and OUT EP
 */
typedef struct {
        hw_usb_ep_flags_t flags;  /**< USB EP transfer flags */
        hw_usb_ep_buffer_t tx;    /**< TX buffer of the endpoint */
        hw_usb_ep_buffer_t rx;    /**< RX buffer of the endpoint */
        uint16_t mps;              /**< Maximum packet size */
} hw_usb_ep_data_t;


/**
 * \enum HW_USB_NFSR_TYPE
 * \brief  Node Functional State Register.
 */
typedef enum {
        HW_USB_NFSR_NODE_RESET,         /**< Reset state for NFSR */
        HW_USB_NFSR_NODE_RESUME,        /**< Resume state for NFSR */
        HW_USB_NFSR_NODE_OPERATIONAL,   /**< Operational state for NFSR */
        HW_USB_NFSR_NODE_SUSPEND        /**< Suspend state for NFSR */
} HW_USB_NFSR_TYPE;

/**
 * \enum HW_USB_VBUS_IRQ_STAT
 * \brief VBUS IRQ status.
 *
 */
typedef enum {
        HW_USB_VBUS_IRQ_STAT_FALL = /**< VBUS irq has been programmed to hit on falling edge. */
                REG_MSK(CRG_TOP, VBUS_IRQ_MASK_REG, VBUS_IRQ_EN_FALL),
        HW_USB_VBUS_IRQ_STAT_RISE = /**< VBUS irq has been programmed to hit on rising edge. */
                REG_MSK(CRG_TOP, VBUS_IRQ_MASK_REG, VBUS_IRQ_EN_RISE)
} HW_USB_VBUS_IRQ_STAT;

/**
 * \brief USB VBUS callback
 *
 * \param [in] status: IRQ status.
 *
 */
typedef void (*hw_usb_vbus_cb_t)(HW_USB_VBUS_IRQ_STAT status);

/**
 * \brief USB callback
 *
 * \param [in] status: IRQ status.
 *
 */
typedef void (*hw_usb_usb_cb_t)(uint32_t status);


/*========================== Global function prototypes =====================*/

/**
 * \brief Must be called when an USB interrupt is handled.
 *
 */
void hw_usb_interrupt_handler(uint32_t status);

/**
 * \brief Process SD3 interrupt.
 *
 */
void hw_usb_sd3_event();

/**
 * \brief Process SD5 interrupt.
 *
 */
void hw_usb_sd5_event();

/**
 * \brief Process reset interrupt.
 *
 */
void hw_usb_reset_event();

/**
 * \brief Process resume interrupt.
 *
 */
void hw_usb_resume_event();

/**
 * \brief Process frame interrupt.
 *
 */
void hw_usb_frame_event();

/**
 * \brief Check for NAK interrupt from all endpoints.
 *
 */
void hw_usb_nak_event();

/**
 * \brief Check for interrupt from TX endpoints.
 *
 */
void hw_usb_tx_event();

/**
 * \brief Check for interrupt from RX endpoints.
 *
 */
void hw_usb_rx_event();

/**
 * \brief Check for NAK interrupt from endpoints 0.
 *
 */
void hw_usb_nak_event_ep0();

/**
 * \brief Receive on endpoint zero.
 *
 */
void hw_usb_rx_ep0(void);

/**
 * \brief Continue or complete TX for the endpoint.
 *
 * \param[in] ep_nr The endpoint number.
 *
 */
void hw_usb_tx_ep(uint8_t ep_nr);

/**
 * \brief Get the VBUS mask register.
 *
 * \return VBUS mask register.
 *
 */
__STATIC_INLINE HW_USB_VBUS_IRQ_STAT hw_usb_get_vbus_mask_status(void)
{
        return CRG_TOP->VBUS_IRQ_MASK_REG;
}

/*========================== USB pad related services ==================================*/

/**
 * \brief Enable the USB pads with a pull-up on D+.
 *
 * \note Corresponding pin / port must be configured as a USB pad in advance.
 */
__STATIC_INLINE void hw_usb_enable_usb_pads_with_pullup(void)
{
        GPREG->USBPAD_REG = REG_MSK(GPREG, USBPAD_REG, USBPAD_EN);
}

/**
 * \brief Enable the USB pads without activating the pull-up.
 *
 * \note Corresponding pin / port must be configured as a USB pad in advance.
 */
__STATIC_INLINE void hw_usb_enable_usb_pads_without_pullup(void)
{
        GPREG->USBPAD_REG = REG_MSK(GPREG, USBPAD_REG, USBPHY_FORCE_SW1_OFF)    |
                            REG_MSK(GPREG, USBPAD_REG, USBPAD_EN);
}

/**
 * \brief Disable the USB pads.
 *
 */
__STATIC_INLINE void hw_usb_disable_usb_pads(void)
{
        GPREG->USBPAD_REG = 0;
}

/*========================== VBUS IRQ related services =================================*/

/**
 * \brief Enable VBUS interrupt in NVIC.
 *
 * \param [in] cb: Callback function that will be called when the interrupt line is asserted
 *                 on rising or on falling edge.
 *
 */
void hw_usb_enable_vbus_interrupt(hw_usb_vbus_cb_t cb);

/**
 * \brief Disable VBUS interrupt in NVIC.
 *
 */
void hw_usb_disable_vbus_interrupt(void);

/**
 * \brief Program VBUS IRQ to hit when the VBUS level goes from low to high.
 *
 */
__STATIC_INLINE void hw_usb_program_vbus_irq_on_rising(void)
{
        REG_SET_BIT(CRG_TOP, VBUS_IRQ_MASK_REG, VBUS_IRQ_EN_RISE);
}

/**
 * \brief Program VBUS IRQ to hit when the VBUS level goes from high to low.
 *
 */
__STATIC_INLINE void hw_usb_program_vbus_irq_on_falling(void)
{
        REG_SET_BIT(CRG_TOP, VBUS_IRQ_MASK_REG, VBUS_IRQ_EN_FALL);
}

/**
 * \brief Mask VBUS IRQ so that it does not hit when the VBUS level changes.
 *
 */
__STATIC_INLINE void hw_usb_program_vbus_cancel_irq(void)
{
        CRG_TOP->VBUS_IRQ_MASK_REG = 0;
}

/**
 * \brief Clear VBUS IRQ.
 *
 */
__STATIC_INLINE void hw_usb_clear_vbus_irq(void)
{
        /* Writing any value will clear the IRQ line*/
        CRG_TOP->VBUS_IRQ_CLEAR_REG = 0x1;
}

/*========================== USB IRQ related services ==================================*/

/**
 * \brief Program USB IRQ to hit when [0,3] bits of USB_CHARGER_STAT_REG are set.
 *
 */
__STATIC_INLINE void hw_usb_program_usb_irq(void)
{
        REG_SET_BIT(USB, USB_MAMSK_REG, USB_M_CH_EV);
}

/**
 * \brief Mask USB IRQ so that it does not hit when when [0,3] bits of USB_CHARGER_STAT_REG are set.
 *
 */
__STATIC_INLINE void hw_usb_program_usb_cancel_irq(void)
{
        REG_CLR_BIT(USB, USB_MAMSK_REG, USB_M_CH_EV);
}

/**
 * \brief Get USB event.
 *
 */
__STATIC_INLINE uint32_t hw_usb_get_and_clear_usb_event_status(void)
{
        return USB->USB_MAEV_REG;
}

/**
 * \brief Enable USB interrupt in NVIC.
 *
 * \param [in] cb: Callback function that will be called when the interrupt line is asserted.
 *
 */
void hw_usb_enable_usb_interrupt(hw_usb_usb_cb_t cb);

/**
 * \brief Disable USB interrupt in NVIC.
 *
 */
void hw_usb_disable_usb_interrupt(void);

/**
 * \brief Re-enable IRQs that were disabled when the device was suspended
 *
 */
void hw_usb_enable_irqs_on_resume(void);

#endif  /* dg_configUSE_HW_USB */

/*======================== USB power related services ==================================*/

/**
 * \brief Check if VBUS power is available.
 *
 * \retval true: Powered by VBUS.
 * \retval false: Not Powered by VBUS.
 *
 * \note A ~20ms delay is needed for safely reading the power source.
 *
 */
__STATIC_FORCEINLINE bool hw_usb_is_powered_by_vbus(void)
{
        return REG_GETF(CRG_TOP, ANA_STATUS_REG, LDO_VSYS_OK);
}


#endif /* USBDRIVER_V2_H  */

/**
\}
\}
*/

