/**
 ****************************************************************************************
 *
 * @file sys_usb_v2.c
 *
 * @brief System USB
 *
 * Copyright (C) 2018-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configUSE_SYS_USB == 1)

#ifndef OS_PRESENT
# error "The USB system service is not available for bare-metal applications"
#endif

#include "sys_usb.h"
#include "sys_usb_internal.h"
#include "osal.h"
#include "sys_power_mgr.h"
#include "sys_clock_mgr.h"
#if HW_USB_DMA_SUPPORT
#include "hw_dma.h"
#endif
#include "hw_gpio.h"
#if dg_configUSE_SYS_CHARGER
#include "hw_charger.h"
#endif
#include "resmgmt.h"

/************************************** Private definitions ***************************************/

#define SYS_USB_20ms_SAFE_READOUT_MARGIN           20       /*  20ms */

/************************************** Forward Declarations **************************************/

static OS_TASK_FUNCTION(sys_usb_task, pvParameters);
static void sys_usb_process_attach(void);
static void sys_usb_process_detach(void);
static void sys_usb_process_vbus_event(void);
__STATIC_FORCEINLINE bool sys_usb_is_vbus_available(void);
__STATIC_FORCEINLINE void sys_usb_ldo_sys_dietemp_mode(bool mode);
#if dg_configUSE_SYS_CHARGER
static void sys_usb_ldo_sys_dietemp_mode_conditional_lock(bool mode);
#endif

#if dg_configUSE_USB_ENUMERATION
static void sys_usb_assert_usb_data_pin_conf(void);
static void sys_usb_idle_on_suspend(bool set_idle);
extern void set_sdk_callbacks_1470x();
extern void set_emusb_1470x_driver();
#endif /* dg_configUSE_USB_ENUMERATION */

/************************************** OS handlers ***********************************************/

__RETAINED static OS_TASK sys_usb_task_h;

/************************************** Housekeeping variables ************************************/

#if HW_USB_DMA_SUPPORT
__RETAINED static sys_usb_conf_t sys_usb_config;
__RETAINED static usb_config lld_bkup_config;
#endif

__RETAINED static bool sys_usb_is_process_attach_completed;

#if dg_configUSE_USB_ENUMERATION
__RETAINED static bool sys_usb_is_pll_activated;
__RETAINED_RW static bool sys_usb_is_suspended = false;
#endif

/************************************** Private types *********************************************/

/*
 * List of exchanged messages between tasks / ISR's
 */
typedef enum {
        SYS_USB_TASK_MSG_VBUS_UNKNOWN     = 0,           /* VBUS state is unknown */
        SYS_USB_TASK_MSG_VBUS_RISE        = (1 << 1),    /* VBUS is attached  */
        SYS_USB_TASK_MSG_VBUS_FALL        = (1 << 2),    /* VBUS is detached */
        SYS_USB_TASK_MSG_USB_RESET        = (1 << 3),    /* USB RESET detected */
        SYS_USB_TASK_MSG_USB_SUSPEND      = (1 << 4),    /* USB SUSPEND detected */
        SYS_USB_TASK_MSG_USB_RESUME       = (1 << 5),    /* USB RESUME detected */
        SYS_USB_TASK_MSG_SYS_CLOCK_PLL48  = (1 << 6),    /* restore the USB clock PLL48*/
        SYS_USB_TASK_MSG_SYS_CLOCK_DIVN   = (1 << 7),    /* set the DIVN clock*/
} SYS_USB_TASK_MSG_STAT;

/************************************** ISR callbacks *********************************************/

static void sys_usb_vbus_isr_cb(HW_USB_VBUS_IRQ_STAT status)
{
        if (status & HW_USB_VBUS_IRQ_STAT_RISE) {
                OS_TASK_NOTIFY_FROM_ISR(sys_usb_task_h, SYS_USB_TASK_MSG_VBUS_RISE, OS_NOTIFY_SET_BITS);
        } else if (status & HW_USB_VBUS_IRQ_STAT_FALL) {
                OS_TASK_NOTIFY_FROM_ISR(sys_usb_task_h, SYS_USB_TASK_MSG_VBUS_FALL, OS_NOTIFY_SET_BITS);
        }
}

__UNUSED static void sys_usb_usb_isr_cb(uint32_t status)
{
        hw_usb_interrupt_handler(status);
}
/************************************** Processing Tasks ******************************************/

#if HW_USB_DMA_SUPPORT
static resource_mask_t dma_resource_mask(HW_DMA_CHANNEL num)
{
        const resource_mask_t res_mask[] = {
                RES_MASK(RES_ID_DMA_CH0), RES_MASK(RES_ID_DMA_CH1),
                RES_MASK(RES_ID_DMA_CH2), RES_MASK(RES_ID_DMA_CH3),
                RES_MASK(RES_ID_DMA_CH4), RES_MASK(RES_ID_DMA_CH5),
                RES_MASK(RES_ID_DMA_CH6), RES_MASK(RES_ID_DMA_CH7)
        };

        return res_mask[num];
}
#endif /* HW_USB_DMA_SUPPORT */

static OS_TASK_FUNCTION(sys_usb_task, pvParameters)
{
        uint32_t        notif;
        OS_BASE_TYPE    ret;

        /* Initialize VBUS IRQ mask assuming that VBUS is not present
         * to start at a known state.
         */
        /* IRQ on rising edge triggered when USB is plugged in */
        hw_usb_program_vbus_irq_on_rising();
        /* IRQ on falling edge triggered when USB is plugged out */
        hw_usb_program_vbus_irq_on_falling();

        /* Check if VBUS is actually present in case USB was attached
         * before this task started. This check is needed as IRQs for plug-in/-out
         * are edge triggered, not level, and maybe the related event not be caught.
         */

        if (sys_usb_is_vbus_available()) {
                /* VBUS IRQ has been missed (since it was not enabled when USB was attached).
                 * A fake interrupt is triggered to let the VBUS IRQ handler place the system in
                 * the correct state.
                 */
                NVIC_SetPendingIRQ(VBUS_IRQn);
        }

        while (true) {
                ret = OS_TASK_NOTIFY_WAIT(OS_TASK_NOTIFY_NONE, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
                if (ret == OS_TASK_NOTIFY_FAIL) {
                        continue;
                }

#if dg_configUSE_USB_ENUMERATION
                ASSERT_WARNING(cm_cpu_clk_get() != cpuclk_160M || cm_apb_get_clock_divider() != apb_div1);
#endif

                /* Handle VBUS event notification for plug-in/-out*/
                if ((notif & SYS_USB_TASK_MSG_VBUS_RISE) ||
                    (notif & SYS_USB_TASK_MSG_VBUS_FALL)) {
                        sys_usb_process_vbus_event();
                }

#if dg_configUSE_USB_ENUMERATION
                /* Handle USB_SUSPEND event */
                if (notif & SYS_USB_TASK_MSG_USB_SUSPEND) {
                        /* Host requested the device to suspend.
                         * We need to reduce power consumption.
                         * We will go from active to idle (WFI) */
                        if ((sys_usb_is_suspended == false) && (sys_usb_is_pll_activated == true)) {
                                hw_usb_sd3_event(); // Take care of HW
                                hw_usb_bus_event(UBE_SUSPEND); // Notify emUSB
                                sys_usb_int_charger_hook_suspend_event(); // Notify Charger
                                sys_usb_idle_on_suspend(true); // Go to IDLE (WFI)
                        }
                }

                /* Handle USB CLOCK for SUSPEND event to set USB clock to DIVN */
                if (notif & SYS_USB_TASK_MSG_SYS_CLOCK_DIVN) {
                        if (sys_usb_is_pll_activated) {
                                cm_sys_disable_pll_usb();
                                sys_usb_is_pll_activated = false;
                        }
                }

                /* USB clock set to PLL48 MUST be completed before handling usb events */
                if (notif & SYS_USB_TASK_MSG_SYS_CLOCK_PLL48) {
                        if (!sys_usb_is_pll_activated) {
                                cm_sys_enable_pll_usb();
                                sys_usb_is_pll_activated = true;
                        }
                }

                /* Handle USB RESET event */
                if (notif & SYS_USB_TASK_MSG_USB_RESET) {
                        OS_ASSERT(sys_usb_is_pll_activated == true);
                        /* Host has exited suspend state. Go from idle to active */
                        sys_usb_idle_on_suspend(false);
                        /* Take care of HW */
                        hw_usb_reset_event();
                        /* Notify emUSB */
                        hw_usb_bus_event(UBE_RESET);
                }

                /* Handle USB RESUME event */
                if (notif & SYS_USB_TASK_MSG_USB_RESUME) {
                        /* Host exited suspend state.
                         * Restore operation and clocks.
                         * Go from idle to active
                         */
                        OS_ASSERT(sys_usb_is_pll_activated == true);
                        /* Take care of HW */
                        hw_usb_resume_event();
                        /* Notify emUSB */
                        hw_usb_bus_event(UBE_RESUME);
                        /* Host has exited suspend state, go from idle to active */
                        sys_usb_idle_on_suspend(false);
                        /* Notify Charger */
                        sys_usb_int_charger_hook_resume_event();
                }
#endif /* dg_configUSE_USB_ENUMERATION */
        }
}

static void sys_usb_process_vbus_event(void)
{
        /* Safe reading of register values */
        OS_DELAY_MS(SYS_USB_20ms_SAFE_READOUT_MARGIN);


        if (sys_usb_is_vbus_available()) {
                // VBUS_AVAILABLE == 1
                sys_usb_process_attach();
        } else {
                // VBUS_AVAILABLE == 0
                sys_usb_process_detach();
        }

}

static void sys_usb_process_attach(void)
{
        if (sys_usb_is_process_attach_completed) {
                return;
        }

        /* Safe guard to properly call hw_usb_is_powered_by_vbus() */

        /* sys_usb_process_attach() and sys_usb_process_detach() work in a pipeline
         * fashion. That means the two processes will never interfere with each other. This
         * is guaranteed by sys_usb_vbus_task(). As a result, there will be no race
         * threat for using sys_usb_is_process_attach_completed
         */
        sys_usb_is_process_attach_completed = true;

#if dg_configUSE_USB_ENUMERATION
        cm_sys_enable_pll_usb();
#endif

#if dg_configUSE_SYS_CHARGER
        sys_usb_ldo_sys_dietemp_mode_conditional_lock(true);
#else
        sys_usb_ldo_sys_dietemp_mode(true);
#endif

        /* System is not allowed to go to sleep when the USB is in use,
         * as the USB functionality will be lost. The USB host supplies the power to device,
         * so we don't care about power consumption and we should not drain the battery */
        pm_sleep_mode_request(pm_mode_active);


        /* Propagate VBUS plug-in event to application level */
        sys_usb_ext_hook_attach();

#if dg_configUSE_USB_ENUMERATION || dg_configUSE_SYS_CHARGER
        /* Enable USB and charger interrupts and register USB ISR callback */
        hw_usb_enable_usb_interrupt(sys_usb_usb_isr_cb);
#endif

        /* At this time Host sees a device that gets power from VBUS. Until enumeration Host
         * provide power to the device up to 100mA. Host is able to reduce this value if
         * enumeration didn't take place within a reasonable time
         */

#if dg_configUSE_USB_ENUMERATION
        /* Check if the USB pads are properly configured. If USB pads
         * are configured with HW_GPIO_FUNC_GPIO and USB is used then
         * USB block might be permanently damaged */
        sys_usb_assert_usb_data_pin_conf();
#endif

#if HW_USB_DMA_SUPPORT
        if (sys_usb_config.lld.use_dma) {
                resource_mask_t dma_res = resource_acquire(dma_resource_mask(sys_usb_config.lld.tx_dma_channel) |
                                                           dma_resource_mask(sys_usb_config.lld.rx_dma_channel),
                                                           OS_EVENT_NO_WAIT);

                /* Try to get the DMA channels needed for the USB TX/RX.
                 * If any of them is not available, then invalidate the DMA channels
                 * and set the flag for acquired DMA channels to false,
                 * so the USB will continue in interrupt mode for this plug-in.
                 * Upon detach there will be no attempt to release a non-acquired resource,
                 * which would trigger an assertion.
                 */
                if (dma_res) {
                        sys_usb_config.acquired_dma = true;
                } else {
                        sys_usb_config.lld.tx_dma_channel = HW_DMA_CHANNEL_INVALID;
                        sys_usb_config.lld.rx_dma_channel = HW_DMA_CHANNEL_INVALID;
                        sys_usb_config.acquired_dma = false;
                }
        }

        /* Configure the USB module */
        hw_usb_cfg(&sys_usb_config.lld);
#endif /* HW_USB_DMA_SUPPORT */

#if !dg_configUSE_SYS_CHARGER
# if dg_configUSE_USB_ENUMERATION
        sys_usb_finalize_attach();
# endif /* dg_configUSE_USB_ENUMERATION */
#else /* dg_configUSE_SYS_CHARGER */

        /* Power-on the USB pads without any pull-ups to use them for the charger detection */
        hw_usb_enable_usb_pads_without_pullup();

        /* USB-block notification regarding a charging event is no more needed. */
# if dg_configUSE_HW_PORT_DETECTION
        hw_usb_program_usb_cancel_irq();
# else
        hw_usb_program_usb_irq();
# endif /* dg_configUSE_HW_PORT_DETECTION */

        /* Enable charger interrupts and start the Charger Detection */
        sys_usb_int_charger_hook_attach();
#endif /* dg_configUSE_SYS_CHARGER */
}

/*
 * The Detach process happens when the USB is unplugged
 */
static void sys_usb_process_detach(void)
{
        /* Do not proceed in detaching process in case of an uncompleted attach cycle */
        if (sys_usb_is_process_attach_completed) {
                sys_usb_is_process_attach_completed = false;

                /* First disable the interrupts */
                hw_usb_disable_usb_interrupt();

                /* Then disable the pads so there will be no fake events interrupts */
                hw_usb_disable_usb_pads();

#if dg_configUSE_SYS_CHARGER
                /* Program the temperature protection in the charger */
                sys_usb_ldo_sys_dietemp_mode_conditional_lock(false);
#else
                sys_usb_ldo_sys_dietemp_mode(false);
#endif

#if dg_configUSE_USB_ENUMERATION
# if (dg_configUSB_SUSPEND_MODE == USB_SUSPEND_MODE_PAUSE)
                if (sys_usb_is_suspended == false) {
                        OS_ENTER_CRITICAL_SECTION();
                        hw_usb_enable_irqs_on_resume();
                        OS_LEAVE_CRITICAL_SECTION();
                }
# endif /* dg_configUSB_SUSPEND_MODE */
                sys_usb_idle_on_suspend(false);
#endif /* dg_configUSE_USB_ENUMERATION */
                /* Release the pm_mode_active and allow the system to resume sleep/wake cycles */
                pm_sleep_mode_release(pm_mode_active);

#if dg_configUSE_SYS_CHARGER
                /* Inform system that USB is unplugged and handled as needed by Charger */
                sys_usb_int_charger_hook_detach();
#endif

                /* Propagate VBUS plug-out event to application level */
                sys_usb_ext_hook_detach();

#if dg_configUSE_USB_ENUMERATION
                hw_usb_bus_detach();

                REG_SETF(CRG_TOP, CLK_CTRL_REG, USB_CLK_SRC, 0);
                /* Indicate the sys USB to lower the clock as we do not need the PLL */
                OS_TASK_NOTIFY(sys_usb_task_h, SYS_USB_TASK_MSG_SYS_CLOCK_DIVN, OS_NOTIFY_SET_BITS);
                cm_sys_disable_pll_usb();
                USB->USB_MCTRL_REG = 0;
#endif /* dg_configUSE_USB_ENUMERATION */

#if HW_USB_DMA_SUPPORT
                if (sys_usb_config.lld.use_dma) {
                        if (sys_usb_config.acquired_dma) {
                                resource_release(dma_resource_mask(sys_usb_config.lld.tx_dma_channel) |
                                                 dma_resource_mask(sys_usb_config.lld.rx_dma_channel));
                        }

                        OPT_MEMCPY((void*)&sys_usb_config.lld, (void*)&lld_bkup_config, sizeof(usb_config));

                        sys_usb_config.acquired_dma = false;
                }
#endif /* HW_USB_DMA_SUPPORT */
        }
}

#if dg_configUSE_USB_ENUMERATION
void sys_usb_cfg(const sys_usb_conf_t *cfg)
{
#if HW_USB_DMA_SUPPORT
        if (cfg && cfg->lld.use_dma) {
                OPT_MEMCPY((void*)&sys_usb_config.lld, (void*)&cfg->lld, sizeof(usb_config));
                sys_usb_config.acquired_dma = false;
        }
        OPT_MEMCPY((void*)&lld_bkup_config, (void*)&sys_usb_config.lld, sizeof(usb_config));
#endif /* HW_USB_DMA_SUPPORT */
}
#endif /* dg_configUSE_USB_ENUMERATION */

void sys_usb_init(void)
{
        sys_usb_is_process_attach_completed = false;

#if dg_configUSE_USB_ENUMERATION
        /* Set the SDK USB callbacks */
        set_sdk_callbacks_1470x();

        /* Set the emUSB (Segger USB Stack) */
        set_emusb_1470x_driver();
#endif /* dg_configUSE_USB_ENUMERATION */

        OS_TASK_CREATE("VBUS",                          /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                       sys_usb_task,                    /* The function that implements the task. */
                       NULL,                            /* The parameter passed to the task. */
                       OS_MINIMAL_TASK_STACK_SIZE,      /* The number of bytes to allocate to the
                                                           stack of the task. */
                       OS_TASK_PRIORITY_HIGHEST - 2,    /* The priority assigned to the task. */
                       sys_usb_task_h);                 /* The task handle */

        OS_ASSERT(sys_usb_task_h);

        /* Enable the VBUS Interrupt (PHY) in NVIC */
        hw_usb_enable_vbus_interrupt(sys_usb_vbus_isr_cb);
}

#if dg_configUSE_USB_ENUMERATION
void sys_usb_finalize_attach(void)
{
        /* USB Pads will be powered-on when the USB will be in use. */
        hw_usb_disable_usb_pads();

        /* select initialize clock for the USB */
        hw_usb_init();

        /* enable the USB block */
        hw_usb_node_enable();

        /* prepare the USB block */
        hw_usb_bus_attach();

        /* Now that everything is ready, announce device presence to the USB host */
        hw_usb_node_attach();

        /* Call USB functionality at application level
         * where the enumeration is implemented */
        sys_usb_ext_hook_begin_enumeration();
}
#endif /* dg_configUSE_USB_ENUMERATION */

/**
 * \brief USB Interrupt handling function.
 *
 */
void hw_usb_interrupt_handler(uint32_t status)
{
        __UNUSED uint16_t maev = status;

        /* USB events have higher priority than charger event */
#if dg_configUSE_USB_ENUMERATION
        /* Handshake packets for EP0 used in enumeration */
        if (maev & USB_USB_MAEV_REG_USB_EP0_NAK_Msk) {
                hw_usb_nak_event_ep0();
        }

        /* EP0 Data Events */
        if (maev & USB_USB_MAEV_REG_USB_EP0_TX_Msk) {
                hw_usb_tx_ep(0);
        }

        if (maev & USB_USB_MAEV_REG_USB_EP0_RX_Msk) {
                hw_usb_rx_ep0();
        }

        /* Handshake packets for EP1-EP6 used in enumeration */
        if (maev & USB_USB_MAEV_REG_USB_FRAME_Msk) {
                hw_usb_frame_event();
        }

        if (maev & USB_USB_MAEV_REG_USB_NAK_Msk) {
                hw_usb_nak_event();
        }

        /* EP1-EP6 Data Events */
        if (maev & USB_USB_MAEV_REG_USB_TX_EV_Msk) {
                hw_usb_tx_event();
        }

        if (maev & USB_USB_MAEV_REG_USB_RX_EV_Msk) {
                hw_usb_rx_event();
        }

        /* VBUS events */
        if (maev & USB_USB_MAEV_REG_USB_ALT_Msk) {
                uint8_t altev;

                altev = USB->USB_ALTEV_REG;
                altev &= USB->USB_ALTMSK_REG;

                /* Transition to NodeReset.
                 * Reset takes place in enumeration (2 Resets are sent) for synch,
                 * in other cases Reset means that the USB device is unresponsive */
                if (altev & USB_USB_ALTEV_REG_USB_RESET_Msk) {
                        /* clear the event */
                        REG_CLR_BIT(USB, USB_ALTMSK_REG, USB_M_RESET);

                        /* Notify the sys_usb task for the rest of the actions needed for the event */
                        uint32_t notification_value =
                                SYS_USB_TASK_MSG_SYS_CLOCK_PLL48 | SYS_USB_TASK_MSG_USB_RESET;
                        OS_TASK_NOTIFY_FROM_ISR(sys_usb_task_h, notification_value,
                                OS_NOTIFY_SET_BITS);


                        /* RESET received, do not process anything else */
                        return;
                }
                /* Transition to NodeSuspend */
                if (altev & USB_USB_ALTEV_REG_USB_SD3_Msk) {
                        /* Notify the sys_usb task for the rest of the actions needed for the event */
                        uint32_t notification_value =
                                SYS_USB_TASK_MSG_SYS_CLOCK_DIVN | SYS_USB_TASK_MSG_USB_SUSPEND;
                        OS_TASK_NOTIFY_FROM_ISR(sys_usb_task_h, notification_value,
                                OS_NOTIFY_SET_BITS);
                }

                /* Transition to NodeSuspend for SD5 is N/A */
                if (altev & USB_USB_ALTEV_REG_USB_SD5_Msk) {
                        /* Take care of HW first */
                        hw_usb_sd5_event();
                }

                /* Transition to NodeResume */
                if (altev & USB_USB_ALTEV_REG_USB_RESUME_Msk) {
                        /* Notify the sys_usb task for the rest of the actions needed for the event */
                        uint32_t notification_value =
                                SYS_USB_TASK_MSG_USB_RESUME | SYS_USB_TASK_MSG_SYS_CLOCK_PLL48;
                        OS_TASK_NOTIFY_FROM_ISR(sys_usb_task_h, notification_value,
                                OS_NOTIFY_SET_BITS);
                }
        }
#endif /* dg_configUSE_USB_ENUMERATION */

#if dg_configUSE_SYS_CHARGER
        /* Charger Events */
        if (maev & REG_MSK(USB, USB_MAEV_REG, USB_CH_EV)) {
#  if dg_configUSE_HW_PORT_DETECTION
                /* No need for sys_usb to notify sys_charger. */
#  else
                sys_usb_int_charger_hook_ch_event();
#  endif /* dg_configUSE_HW_PORT_DETECTION */
      }

#endif /* dg_configUSE_SYS_CHARGER */
}

/************************************** Helper Functions ******************************************/

#if dg_configUSE_USB_ENUMERATION
static void sys_usb_assert_usb_data_pin_conf(void)
{
        HW_GPIO_MODE mode;
        HW_GPIO_FUNC func;

        hw_gpio_get_pin_function(HW_GPIO_PORT_2, HW_GPIO_PIN_10, &mode, &func);

        OS_ASSERT(mode == HW_GPIO_MODE_INPUT);
        OS_ASSERT(func == HW_GPIO_FUNC_USB);

        hw_gpio_get_pin_function(HW_GPIO_PORT_2, HW_GPIO_PIN_11, &mode, &func);

        OS_ASSERT(mode == HW_GPIO_MODE_INPUT);
        OS_ASSERT(func == HW_GPIO_FUNC_USB);
}

static void sys_usb_idle_on_suspend(bool set_idle)
{
        if (set_idle) {
#if (dg_configUSB_SUSPEND_MODE != USB_SUSPEND_MODE_NONE)
                if (!sys_usb_is_suspended) {
                        pm_sleep_mode_request(pm_mode_idle);
                        pm_sleep_mode_release(pm_mode_active);
                }
#endif /* dg_configUSB_SUSPEND_MODE */
                sys_usb_is_suspended = true;
        } else {
#if (dg_configUSB_SUSPEND_MODE != USB_SUSPEND_MODE_NONE)
                if (sys_usb_is_suspended) {
                        pm_sleep_mode_request(pm_mode_active);
                        pm_sleep_mode_release(pm_mode_idle);
                }
#endif /* dg_configUSB_SUSPEND_MODE */
                sys_usb_is_suspended = false;
        }
}
#endif /* dg_configUSE_USB_ENUMERATION */

__STATIC_FORCEINLINE bool sys_usb_is_vbus_available(void)
{
        return (CRG_TOP->ANA_STATUS_REG & REG_MSK(CRG_TOP, ANA_STATUS_REG, VBUS_AVAILABLE));
}

__STATIC_FORCEINLINE void sys_usb_ldo_sys_dietemp_mode(bool mode)
{
        RAW_SETF(0x51000604, 0x0001C000, mode);
}

#if dg_configUSE_SYS_CHARGER
static void sys_usb_ldo_sys_dietemp_mode_conditional_lock(bool mode)
{
        if (hw_charger_get_sw_lock_mode() && hw_charger_get_sw_lock_status()) {
                /* SW lock is active and the register / register fields are protected.
                 * Apply the unlock sequence first to be able to program the charger.
                 */
                hw_charger_apply_sw_unlock_sequence();

                sys_usb_ldo_sys_dietemp_mode(mode);

                /* Programming charger done. Lock registers / register fields. */
                hw_charger_apply_sw_lock_sequence();
        } else {
                sys_usb_ldo_sys_dietemp_mode(mode);
        }
}
#endif /* dg_configUSE_SYS_CHARGER */

#endif /* dg_configUSE_SYS_USB */
