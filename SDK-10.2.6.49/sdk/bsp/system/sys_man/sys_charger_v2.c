/**
 ****************************************************************************************
 *
 * @file sys_charger_v2.c
 *
 * @brief Charger System Service
 *
 * Copyright (C) 2018-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if (dg_configUSE_SYS_CHARGER == 1)

#ifndef OS_PRESENT
# error "The USB system service is not available for bare-metal applications"
#endif

#ifndef SYS_CHARGER_MAX_QUEUE_SIZE
#define SYS_CHARGER_MAX_QUEUE_SIZE      (16)
#endif

#include "sys_charger.h"
#include "sys_usb.h"
#include "sys_usb_internal.h"
#include "osal.h"
#include "hw_usb_charger.h"

/* Potentially available port-detection implementations */
#define IN_SW                   0
#define IN_HW                   1


#if !defined(dg_configUSE_HW_PORT_DETECTION) || (dg_configUSE_HW_PORT_DETECTION == 0)
#define PORT_DETECTION_IMPL                     IN_SW
#else
#define PORT_DETECTION_IMPL                     IN_HW
#endif


/************************************** Private definitions ***************************************/

#define SYS_CHARGER_SW_FSM_DCD_DEBOUNCE_PERIOD         10       /* 100ms */
#define SYS_CHARGER_SW_FSM_DCD_TIMEOUT                 60       /* 600ms */
#define SYS_CHARGER_SW_FSM_50ms_SAFE_READOUT_MARGIN     5       /*  50ms */
#define SYS_CHARGER_SW_FSM_10ms_SAFE_READOUT_MARGIN     1       /*  10ms */
#define SYS_CHARGER_20ms_SAFE_READOUT_MARGIN           20       /*  20ms */

/************************************** Private types *********************************************/

/*
 * List of exchanged messages between tasks / ISR's
 */
typedef enum {
        SYS_CHARGER_MSG_VBUS_UNKNOWN            = 0,            /* VBUS state is unknown */
        SYS_CHARGER_MSG_VBUS_ATTACH             = (1 << 1),     /* VBUS is attached  */
        SYS_CHARGER_MSG_VBUS_DETACH             = (1 << 2),     /* VBUS is detached */
        SYS_CHARGER_MSG_DCD_TRUE                = (1 << 3),     /* Data contact is detected */
        SYS_CHARGER_MSG_DCD_FALSE               = (1 << 4),     /* Data contact is not detected */

        SYS_CHARGER_MSG_START_SW_FSM            = (1 << 5),     /* Start the SW FSM */
        SYS_CHARGER_MSG_STOP_SW_FSM             = (1 << 6),     /* Stop the SW FSM */

        SYS_CHARGER_MSG_KICK_SW_FSM             = (1 << 7),     /* Kick the 10ms counter of the SW FSM */

        SYS_CHARGER_MSG_USB_ENUMERATED          = (1 << 8),     /* Enumeration is completed */
        SYS_CHARGER_MSG_USB_SUSPENDED           = (1 << 9),     /* USB is suspended */
        SYS_CHARGER_MSG_USB_RESUMED             = (1 << 10),    /* USB is resumed */

#if (PORT_DETECTION_IMPL == IN_HW)

        SYS_CHARGER_MSG_CHG_DET_COMPLETED_SDP   = (1 << 11),    /* SDP port detected */
        SYS_CHARGER_MSG_CHG_DET_COMPLETED_CDP   = (1 << 12),    /* CDP port detected */
        SYS_CHARGER_MSG_CHG_DET_COMPLETED_DCP   = (1 << 13),    /* DCP port detected */
        SYS_CHARGER_MSG_CHG_DET_UNKNOWN         = (1 << 14)     /* Unknown or unsupported port */

#endif /* dg_configUSE_HW_PORT_DETECTION */
} SYS_CHARGER_MSG_STAT;

/*
 * List of SW FSM states
 */
typedef enum {
        SYS_CHARGER_SW_FSM_STATE_IDLE = 0,              /* Idle, SW FSM is suspended. */
        SYS_CHARGER_SW_FSM_STATE_ATTACHED,              /* Attached */
        SYS_CHARGER_SW_FSM_STATE_DCD,                   /* Data Contact Detection */
        SYS_CHARGER_SW_FSM_STATE_PRIMARY_DETECTION,     /* Primary contact detection SDP or DCP / CDP */
        SYS_CHARGER_SW_FSM_STATE_SECONDARY_DETECTION,   /* Secondary contact detection DCP or CDP */
        SYS_CHARGER_SW_FSM_STATE_SDP                    /* SDP */
} SYS_CHARGER_SW_FSM_STATE;

/************************************** Forward Declarations **************************************/

static void sys_charger_shallow_copy_configuration(const sys_charger_configuration_t* conf);
#if (PORT_DETECTION_IMPL == IN_SW)
static void charger_fsm_with_sw_port_detection(void);
static void sys_charger_kick_sw_fsm_timer_cb(OS_TIMER timer);
#endif
static void sys_charger_hw_fsm_ok_isr_cb(HW_CHARGER_FSM_IRQ_STAT_OK status);
static void sys_charger_hw_fsm_nok_isr_cb(HW_CHARGER_FSM_IRQ_STAT_NOK status);
static OS_TASK_FUNCTION(sys_charger_ok_task, pvParameters);
static OS_TASK_FUNCTION(sys_charger_nok_task, pvParameters);
static OS_TASK_FUNCTION(sys_charger_kick_sw_fsm_task, pvParameters);
static void sys_charger_program_hw_fsm(void);
static void sys_charger_start_hw_fsm(void);
static void sys_charger_stop_hw_fsm(void);
static void sys_charger_set_const_current_level(HW_CHARGER_I_LEVEL charge_current);

#if (PORT_DETECTION_IMPL == IN_HW)

static void charger_fsm_with_hw_port_detection(void);
static void sys_charger_hw_usb_charger_chg_det_isr_cb(uint32_t status);
static void sys_charger_start_hw_charger_detection_fsm(void);
static void sys_charger_stop_hw_charger_detection_fsm(void);

#endif /* PORT_DETECTION_IMPL == IN_HW */

#if (dg_configSYS_CHARGER_OSC_CHECK_EN == 1)
/* Global variables declaration */
static volatile bool sys_charger_reached_cc_state;
static volatile uint32_t sys_charger_vbus_irq_cnt;
static volatile uint32_t sys_charger_precc_cc_osc_detected;
static volatile bool sys_charger_fsm_notif_enable;
static volatile uint32_t sys_charger_precc_cc_osc_det_complete;
static OS_TIMER sys_charger_osc_check_tim;
#endif /* dg_configSYS_CHARGER_OSC_CHECK_EN */

/************************************** Private variables *****************************************/

/************************************** OS handlers ***********************************************/

__RETAINED static OS_TASK sys_charger_kick_sw_fsm_task_h;
__RETAINED static OS_TASK sys_charger_ok_task_h;
__RETAINED static OS_TASK sys_charger_nok_task_h;
#if (PORT_DETECTION_IMPL == IN_SW)
__RETAINED static OS_TIMER sys_charger_kick_sw_fsm_timer_h;
#endif
__RETAINED static OS_QUEUE sys_charger_ok_task_msg_queue;

/************************************** Housekeeping variables ************************************/

__RETAINED static const sys_charger_configuration_t* sys_charger_configuration;

#if (PORT_DETECTION_IMPL == IN_SW)
static void sys_charger_kick_sw_fsm_timer_cb(OS_TIMER timer)
{
        OS_TASK_NOTIFY(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_KICK_SW_FSM, OS_NOTIFY_SET_BITS);
}
#endif

/************************************** ISR callbacks *********************************************/

static void sys_charger_hw_fsm_ok_isr_cb(HW_CHARGER_FSM_IRQ_STAT_OK status)
{
        HW_CHARGER_MAIN_FSM_STATE fsm_state;

        hw_charger_clear_ok_irq();
        fsm_state = hw_charger_get_main_fsm_state();
#if (dg_configSYS_CHARGER_OSC_CHECK_EN == 1)
        sys_charger_vbus_irq_cnt++;
        if (sys_charger_fsm_notif_enable &&
           OS_QUEUE_MESSAGES_WAITING_FROM_ISR(sys_charger_ok_task_msg_queue) < (SYS_CHARGER_MAX_QUEUE_SIZE - 1)) {
                // Make sure there is at least one spot empty in the queue to send the oscillation notification
                OS_ASSERT(OS_QUEUE_OK == OS_QUEUE_PUT_FROM_ISR(sys_charger_ok_task_msg_queue, &fsm_state));
        }
#else
        OS_ASSERT(OS_QUEUE_OK == OS_QUEUE_PUT_FROM_ISR(sys_charger_ok_task_msg_queue, &fsm_state));
#endif /* dg_configSYS_CHARGER_OSC_CHECK_EN*/
}

static void sys_charger_hw_fsm_nok_isr_cb(HW_CHARGER_FSM_IRQ_STAT_NOK status)
{
        hw_charger_clear_nok_irq();

        /* Process only the events the charging profile is interested in. */
        status &= hw_charger_get_nok_irq_mask();

        OS_TASK_NOTIFY_FROM_ISR(sys_charger_nok_task_h, status, OS_NOTIFY_SET_BITS);
}

#if (PORT_DETECTION_IMPL == IN_HW)

static void sys_charger_hw_usb_charger_chg_det_isr_cb(uint32_t status)
{
        DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_CH_EVT);

        if (status & HW_USB_CHARGER_DET_STAT_COMPLETED) {
                SYS_CHARGER_MSG_STAT value;

                if (status & HW_USB_CHARGER_DET_STAT_DCP_PORT) {
                        value = SYS_CHARGER_MSG_CHG_DET_COMPLETED_DCP;
                } else if (status & HW_USB_CHARGER_DET_STAT_SDP_PORT) {
                        value = SYS_CHARGER_MSG_CHG_DET_COMPLETED_SDP;
                } else if (status & HW_USB_CHARGER_DET_STAT_CDP_PORT) {
                        value = SYS_CHARGER_MSG_CHG_DET_COMPLETED_CDP;
                } else {
                        ASSERT_WARNING(0);
                        value = SYS_CHARGER_MSG_CHG_DET_UNKNOWN;
                }

                OS_TASK_NOTIFY_FROM_ISR(sys_charger_kick_sw_fsm_task_h, value, OS_NOTIFY_SET_BITS);
        }

        DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_CH_EVT);
}

#endif /* PORT_DETECTION_IMPL == IN_HW */

#if (dg_configSYS_CHARGER_OSC_CHECK_EN == 1)
static void sys_charger_osc_check_tim_cb(OS_TIMER timer)
{
        if (sys_charger_vbus_irq_cnt > dg_configSYS_CHARGER_VBUS_IRQ_CNT_THRESH) {
                sys_charger_precc_cc_osc_detected = true;
        } else {
                sys_charger_precc_cc_osc_detected = false;
                sys_charger_fsm_notif_enable = true;
        }

        sys_charger_vbus_irq_cnt = 0;
        sys_charger_precc_cc_osc_det_complete = true;

        HW_CHARGER_MAIN_FSM_STATE fsm_state = HW_CHARGER_MAIN_FSM_STATE_ERROR;
        OS_ASSERT(OS_QUEUE_OK == OS_QUEUE_PUT(sys_charger_ok_task_msg_queue, &fsm_state, OS_TASK_NOTIFY_NO_WAIT));
}
#endif /* dg_configSYS_CHARGER_OSC_CHECK_EN */

/************************************** sys_usb hooks *********************************************/

void sys_usb_int_charger_hook_attach(void)
{
        DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_VBUS);


# if (PORT_DETECTION_IMPL == IN_HW)
        sys_charger_start_hw_charger_detection_fsm();
# elif (PORT_DETECTION_IMPL == IN_SW)
        hw_usb_charger_start_contact_detection();
# endif

        OS_TASK_NOTIFY(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_START_SW_FSM, OS_NOTIFY_SET_BITS);
}

void sys_usb_int_charger_hook_detach(void)
{
        DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_VBUS);

        sys_charger_stop_hw_fsm();

#if (PORT_DETECTION_IMPL == IN_HW)
        sys_charger_stop_hw_charger_detection_fsm();
#endif /* PORT_DETECTION_IMPL == IN_HW */

#if (dg_configSYS_CHARGER_OSC_CHECK_EN == 1)
        sys_charger_reached_cc_state = false;
#endif

        OS_TASK_NOTIFY(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_STOP_SW_FSM, OS_NOTIFY_SET_BITS);
}

#if (PORT_DETECTION_IMPL == IN_SW)
void sys_usb_int_charger_hook_ch_event(void)
{

        SYS_CHARGER_MSG_STAT value;

        DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_CH_EVT);

        uint32_t usb_charger_stat = hw_usb_charger_get_charger_status();

        if (hw_usb_charger_has_data_pin_contact_detected(usb_charger_stat)) {
                value = SYS_CHARGER_MSG_DCD_TRUE;

        } else {
                value = SYS_CHARGER_MSG_DCD_FALSE;
        }

        hw_usb_program_usb_cancel_irq();

        OS_TASK_NOTIFY_FROM_ISR(sys_charger_kick_sw_fsm_task_h, value, OS_NOTIFY_SET_BITS);

        DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_CH_EVT);
}
#endif


void sys_usb_int_charger_hook_suspend_event(void)
{
        DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_SUS);
        OS_TASK_NOTIFY_FROM_ISR(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_USB_SUSPENDED, OS_NOTIFY_SET_BITS);
}

void sys_usb_int_charger_hook_resume_event(void)
{
        DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_SUS);
        OS_TASK_NOTIFY(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_USB_RESUMED, OS_NOTIFY_SET_BITS);
}

void sys_usb_charger_enumeration_done(void)
{
        DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_ENUM_DONE);
        OS_TASK_NOTIFY_FROM_ISR(sys_charger_kick_sw_fsm_task_h, SYS_CHARGER_MSG_USB_ENUMERATED, OS_NOTIFY_SET_BITS);
        DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_ENUM_DONE);
}

/************************************** Processing Tasks ******************************************/

static OS_TASK_FUNCTION(sys_charger_ok_task, pvParameters)
{
        HW_CHARGER_MAIN_FSM_STATE fsm_state;
        OS_BASE_TYPE              res;
#if (dg_configSYS_CHARGER_OSC_CHECK_EN == 1)
        sys_charger_fsm_notif_enable = true;
        sys_charger_vbus_irq_cnt = 0;
        sys_charger_reached_cc_state = false;
        sys_charger_osc_check_tim = NULL;
#endif /* dg_configSYS_CHARGER_OSC_CHECK_EN */
                do {
                        res = OS_QUEUE_GET(sys_charger_ok_task_msg_queue, &fsm_state, OS_QUEUE_FOREVER);

                        if (res == OS_QUEUE_EMPTY) {
                                continue;
                        }

                        switch (fsm_state) {

                        case HW_CHARGER_MAIN_FSM_STATE_POWER_UP:
                        case HW_CHARGER_MAIN_FSM_STATE_INIT:
                                break;
                        case HW_CHARGER_MAIN_FSM_STATE_DISABLED:
                                sys_charger_ext_hook_hw_fsm_disabled();
                                break;
                        case HW_CHARGER_MAIN_FSM_STATE_PRE_CHARGE:
                                DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_PRE_CH);
                                sys_charger_ext_hook_precharging();
                                DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_PRE_CH);
#if (dg_configSYS_CHARGER_OSC_CHECK_EN == 1)
                                if (sys_charger_reached_cc_state && REG_GETF(CHARGER, CHARGER_STATUS_REG, MAIN_VBAT_COMP_OUT)) {
                                        // After reaching CC state, the charger should not return to the pre-charge state
                                        // Start the timer to check if this is caused by charger oscillation
                                        // Disable notifications to the application, until the check is complete.
                                        sys_charger_fsm_notif_enable = false;
                                        if (sys_charger_osc_check_tim == NULL) {
                                                sys_charger_osc_check_tim  =
                                                        OS_TIMER_CREATE("OSC_CHECK_TIM",
                                                                         OS_MS_2_TICKS(dg_configSYS_CHARGER_OSC_CHECK_TIMER_INTERVAL_MS),
                                                                         OS_TIMER_ONCE,
                                                                         (void *) 0,
                                                                         sys_charger_osc_check_tim_cb);
                                                OS_TIMER_START(sys_charger_osc_check_tim, OS_TIMER_SUCCESS);
                                        }
                                }
#endif /* dg_configSYS_CHARGER_OSC_CHECK_EN */
                                break;
                        case HW_CHARGER_MAIN_FSM_STATE_CC_CHARGE:
                        case HW_CHARGER_MAIN_FSM_STATE_CV_CHARGE:
                                DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_CH);
                                sys_charger_ext_hook_charging();
                                DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_CH);
#if (dg_configSYS_CHARGER_OSC_CHECK_EN == 1)
                                sys_charger_reached_cc_state = true;
#endif /* dg_configSYS_CHARGER_OSC_CHECK_EN */
                                break;
                        case HW_CHARGER_MAIN_FSM_STATE_END_OF_CHARGE:
                                DBG_CONFIGURE_HIGH(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_EOC);
                                sys_charger_ext_hook_charged();
                                DBG_CONFIGURE_LOW(SYS_CHARGER_TIMING_DEBUG, SYS_CHARGER_DBG_EOC);
                                break;
                        case HW_CHARGER_MAIN_FSM_STATE_TDIE_PROT:
                        case HW_CHARGER_MAIN_FSM_STATE_TBAT_PROT:
                        case HW_CHARGER_MAIN_FSM_STATE_BYPASSED:
#if (dg_configSYS_CHARGER_OSC_CHECK_EN == 1)
                                break;
#endif /* dg_configSYS_CHARGER_OSC_CHECK_EN */
                        case HW_CHARGER_MAIN_FSM_STATE_ERROR:
#if (dg_configSYS_CHARGER_OSC_CHECK_EN == 1)
                                if (sys_charger_precc_cc_osc_det_complete) {
                                        if (sys_charger_osc_check_tim) {
                                                OS_TIMER_DELETE(sys_charger_osc_check_tim, OS_TIMER_SUCCESS);
                                        }

                                        if (sys_charger_precc_cc_osc_detected) {
                                                sys_charger_stop_hw_charger_detection_fsm();
                                                sys_charger_stop_hw_fsm();
                                                sys_charger_ext_hook_oscillation_detected();
                                        }

                                        sys_charger_fsm_notif_enable = true;
                                        sys_charger_precc_cc_osc_det_complete = false;
                                        sys_charger_precc_cc_osc_detected = false;
                                        sys_charger_osc_check_tim = NULL;
                                }
#endif /* dg_configSYS_CHARGER_OSC_CHECK_EN */
                                break;
                        default:
                                /* We should not reach here. */
                                OS_ASSERT(0);
                        }
                } while (1);
}

static OS_TASK_FUNCTION(sys_charger_nok_task, pvParameters)
{
        uint32_t status;

        do {
                OS_TASK_NOTIFY_WAIT(OS_TASK_NOTIFY_NONE,
                                    OS_TASK_NOTIFY_ALL_BITS,
                                    (uint32_t *)&status,
                                    OS_TASK_NOTIFY_FOREVER);

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(TBAT_ERROR)) {
                        sys_charger_ext_hook_tbat_error();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(TDIE_ERROR)) {
                        sys_charger_ext_hook_tdie_error();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(VBAT_OVP_ERROR)) {
                        sys_charger_ext_hook_ovp_error();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(TOTAL_CHARGE_TIMEOUT)) {
                        sys_charger_ext_hook_total_charge_timeout();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(CV_CHARGE_TIMEOUT)) {
                        sys_charger_ext_hook_cv_charge_timeout();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(CC_CHARGE_TIMEOUT)) {
                        sys_charger_ext_hook_cc_charge_timeout();
                }

                if (status & HW_CHARGER_FSM_IRQ_STAT_NOK_MASK(PRECHARGE_TIMEOUT)) {
                        sys_charger_ext_hook_pre_charge_timeout();
                }

        } while (1);
}

#if (PORT_DETECTION_IMPL == IN_SW)
static void charger_fsm_with_sw_port_detection(void)
{
        SYS_CHARGER_SW_FSM_STATE state;

        uint32_t ulNotifiedValue;
        uint32_t tick_cntr = 0;         /* 10ms per tick */
        uint32_t dcd_cntr = 0;          /* data contact detection counter */
        bool dcd_result = false;        /* data contact detection result */
        state = SYS_CHARGER_SW_FSM_STATE_IDLE;

        do {
                OS_TASK_NOTIFY_WAIT(OS_TASK_NOTIFY_NONE,
                                    OS_TASK_NOTIFY_ALL_BITS,
                                    &ulNotifiedValue,
                                    OS_TASK_NOTIFY_FOREVER);

                if (ulNotifiedValue & SYS_CHARGER_MSG_KICK_SW_FSM) {
                        tick_cntr++;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_START_SW_FSM) {
                        tick_cntr = 0;
                        dcd_result = false;
                        state = SYS_CHARGER_SW_FSM_STATE_ATTACHED;
                        OS_TIMER_RESET(sys_charger_kick_sw_fsm_timer_h, OS_TIMER_FOREVER);
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_STOP_SW_FSM) {
                        state = SYS_CHARGER_SW_FSM_STATE_IDLE;

                        OS_TIMER_STOP(sys_charger_kick_sw_fsm_timer_h, OS_TIMER_FOREVER);
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_DCD_TRUE) {
                        dcd_result = true;
                        dcd_cntr = tick_cntr;
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_DCD_FALSE) {
                        dcd_result = false;
                        dcd_cntr = UINT32_MAX;
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_USB_ENUMERATED) {
                        /* Enumeration is done, we are good to apply the requested current level. */
                        sys_charger_set_const_current_level(sys_charger_configuration->hw_charging_profile.cc_level);
                        continue;

                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_USB_SUSPENDED) {
                        sys_charger_stop_hw_fsm();
                        continue;

                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_USB_RESUMED) {
                        sys_charger_start_hw_fsm();
                        continue;
                }

                switch (state) {

                case SYS_CHARGER_SW_FSM_STATE_ATTACHED:
                        state = SYS_CHARGER_SW_FSM_STATE_DCD;
                        break;
                case SYS_CHARGER_SW_FSM_STATE_DCD:
                        if ((dcd_result && (tick_cntr > (dcd_cntr + SYS_CHARGER_SW_FSM_DCD_DEBOUNCE_PERIOD))) ||
                             (tick_cntr >  SYS_CHARGER_SW_FSM_DCD_TIMEOUT)) {
                                hw_usb_program_usb_cancel_irq();
                                hw_usb_charger_start_primary_detection();
                                state = SYS_CHARGER_SW_FSM_STATE_PRIMARY_DETECTION;
                                tick_cntr = 0;
                        }
                        break;
                case SYS_CHARGER_SW_FSM_STATE_PRIMARY_DETECTION:
                        if (tick_cntr == SYS_CHARGER_SW_FSM_50ms_SAFE_READOUT_MARGIN) {
                                tick_cntr = 0;
                                HW_USB_CHARGER_PRIMARY_CONN_TYPE prim_con_type = hw_usb_charger_get_primary_detection_result();
                                /* CDP and DCP result are mapped on the same value. So only a single
                                 * comparison would be enough. Added here for clarity reasons only.
                                 */

                                if ((prim_con_type == HW_USB_CHARGER_PRIMARY_CONN_TYPE_CDP) ||
                                    (prim_con_type == HW_USB_CHARGER_PRIMARY_CONN_TYPE_DCP)) {

                                        hw_usb_charger_start_secondary_detection();
                                        state = SYS_CHARGER_SW_FSM_STATE_SECONDARY_DETECTION;
                                } else {
                                        hw_usb_charger_disable_detection();
                                        state = SYS_CHARGER_SW_FSM_STATE_SDP;
                                }
                        }
                        break;
                case SYS_CHARGER_SW_FSM_STATE_SECONDARY_DETECTION:
                        if (tick_cntr == SYS_CHARGER_SW_FSM_50ms_SAFE_READOUT_MARGIN) {
                                tick_cntr = 0;
                                HW_USB_CHARGER_SECONDARY_CONN_TYPE sec_con_type = hw_usb_charger_get_secondary_detection_result();
                                hw_usb_charger_disable_detection();
                                if (sec_con_type == HW_USB_CHARGER_SECONDARY_CONN_TYPE_CDP) { /* min 1500 mA */
#if dg_configUSE_USB_ENUMERATION
                                        /* Ready for enumeration if needed so. */
                                        sys_usb_finalize_attach();
#endif

                                } else if (sec_con_type == HW_USB_CHARGER_SECONDARY_CONN_TYPE_DCP) { /* min 500 mA */
                                        hw_usb_charger_set_dp_high();
                                } else {
                                        /* We should not reach here. */
                                        OS_ASSERT(0);
                                }
                                state = SYS_CHARGER_SW_FSM_STATE_IDLE;
                                sys_charger_program_hw_fsm();
                                sys_charger_start_hw_fsm();
                        }
                        break;
                case SYS_CHARGER_SW_FSM_STATE_SDP:
                        if (tick_cntr == SYS_CHARGER_SW_FSM_10ms_SAFE_READOUT_MARGIN) {
                                HW_CHARGER_I_LEVEL cc_level;
                                sys_charger_program_hw_fsm();
                                cc_level = hw_charger_get_const_current_level();
                                /* Override the programmed CC level if needed.
                                 * JEITA CC values for warm/cool should be lower anyway by spec.
                                 */
                                if (cc_level >= HW_CHARGER_I_LEVEL_100) {
                                        sys_charger_set_const_current_level(HW_CHARGER_I_LEVEL_90);
                                }

                                /* Should be appeared as connected to be able to draw 100mA.
                                 * For up to 500mA enumeration is expected to update the new CC level.
                                 */

#if dg_configUSE_USB_ENUMERATION
                                /* Ready for enumeration if needed so. */
                                sys_usb_finalize_attach();
#endif

                                state = SYS_CHARGER_SW_FSM_STATE_IDLE;
                                sys_charger_start_hw_fsm();
                        }
                        break;
                case SYS_CHARGER_SW_FSM_STATE_IDLE:
                        OS_TIMER_STOP(sys_charger_kick_sw_fsm_timer_h, OS_TIMER_FOREVER);
                        break;

                } /* switch */
        } while (1);
}
#endif  /* PORT_DETECTION_IMPL == IN_SW */

#if (PORT_DETECTION_IMPL == IN_HW)
static void charger_fsm_with_hw_port_detection(void)
{
        uint32_t ulNotifiedValue;

        do {
                OS_TASK_NOTIFY_WAIT(OS_TASK_NOTIFY_NONE,
                                    OS_TASK_NOTIFY_ALL_BITS,
                                    &ulNotifiedValue,
                                    OS_TASK_NOTIFY_FOREVER);


                if (ulNotifiedValue & SYS_CHARGER_MSG_START_SW_FSM) {
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_STOP_SW_FSM) {
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_USB_ENUMERATED) {
                        /* Enumeration is done, we are good to apply the requested current level. */
                        sys_charger_set_const_current_level(sys_charger_configuration->hw_charging_profile.cc_level);
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_USB_SUSPENDED) {
                        sys_charger_stop_hw_fsm();
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_USB_RESUMED) {
                        sys_charger_start_hw_fsm();
                        continue;
                }

                if (ulNotifiedValue & SYS_CHARGER_MSG_CHG_DET_COMPLETED_DCP) {
                        hw_usb_charger_set_dp_high();
                        sys_charger_program_hw_fsm();
                        sys_charger_start_hw_fsm();

                } else if (ulNotifiedValue & SYS_CHARGER_MSG_CHG_DET_COMPLETED_SDP) {
                        HW_CHARGER_I_LEVEL cc_level;
                        sys_charger_program_hw_fsm();
                        cc_level = hw_charger_get_const_current_level();
                        /* Override the programmed CC level if needed.
                         * JEITA CC values for warm/cool should be lower anyway by spec.
                         */
                        if (cc_level >= HW_CHARGER_I_LEVEL_100) {
                                sys_charger_set_const_current_level(HW_CHARGER_I_LEVEL_90);
                        }

                        /* Should be appeared as connected to be able to draw 100mA.
                         * For up to 500mA enumeration is expected to update the new CC level.
                         */

#if dg_configUSE_USB_ENUMERATION
                        /* Ready for enumeration if needed so. */
                        sys_usb_finalize_attach();
#endif

                        sys_charger_start_hw_fsm();

                } else if (ulNotifiedValue & SYS_CHARGER_MSG_CHG_DET_COMPLETED_CDP) {
#if dg_configUSE_USB_ENUMERATION
                        /* Ready for enumeration if needed so. */
                        sys_usb_finalize_attach();
#endif
                        sys_charger_program_hw_fsm();
                        sys_charger_start_hw_fsm();

                }

        } while (1);
}
#endif /* (PORT_DETECTION_IMPL == IN_HW) */

static OS_TASK_FUNCTION(sys_charger_kick_sw_fsm_task, pvParameters)
{
#if (PORT_DETECTION_IMPL == IN_SW)
        charger_fsm_with_sw_port_detection();
#endif
#if (PORT_DETECTION_IMPL == IN_HW)
        charger_fsm_with_hw_port_detection();
#endif
}

/************************************** Helper Functions ******************************************/

static void sys_charger_shallow_copy_configuration(const sys_charger_configuration_t* conf)
{
        sys_charger_configuration = conf;
}

static void sys_charger_program_hw_fsm(void)
{
        hw_charger_program_charging_profile(&sys_charger_configuration->hw_charging_profile);
}

static void sys_charger_start_hw_fsm(void)
{
        hw_charger_enable_fsm_ok_interrupt(sys_charger_hw_fsm_ok_isr_cb);
        hw_charger_enable_fsm_nok_interrupt(sys_charger_hw_fsm_nok_isr_cb);
        hw_charger_set_clock_mode(true);
        hw_charger_set_analog_circuitry_operating_mode(true);
        hw_charger_set_fsm_operating_mode(true);
}

static void sys_charger_stop_hw_fsm(void)
{
        hw_charger_disable_fsm_ok_interrupt();
        hw_charger_disable_fsm_nok_interrupt();
        hw_charger_set_analog_circuitry_operating_mode(false);
        hw_charger_set_fsm_operating_mode(false);
}

#if (PORT_DETECTION_IMPL == IN_HW)

static void sys_charger_start_hw_charger_detection_fsm(void)
{
        hw_usb_charger_enable_charge_detection_interrupt(sys_charger_hw_usb_charger_chg_det_isr_cb);
        hw_usb_charger_set_charge_detection_fsm_operating_mode(true);
        hw_charger_set_clock_mode(true);
}

static void sys_charger_stop_hw_charger_detection_fsm(void)
{
        hw_usb_charger_disable_detection_interrupt();
        hw_usb_charger_set_charge_detection_fsm_operating_mode(false);
}

#endif /* PORT_DETECTION_IMPL == IN_HW */

static void sys_charger_set_const_current_level(HW_CHARGER_I_LEVEL charge_current)
{
        if (hw_charger_get_sw_lock_mode() && hw_charger_get_sw_lock_status()) {
                /* SW lock is active and the register / register fields are protected.
                 * Apply the unlock sequence first to be able to program the charger.
                 */
                hw_charger_apply_sw_unlock_sequence();

                hw_charger_set_const_current_level(charge_current);

                /* Programming charger done. Lock registers / register fields. */
                hw_charger_apply_sw_lock_sequence();
        } else {
                hw_charger_set_const_current_level(charge_current);
        }

}

void sys_charger_init(const sys_charger_configuration_t* conf)
{
        OS_ASSERT(conf);

        sys_charger_shallow_copy_configuration(conf);

        OS_TASK_CREATE("CH_OK",                         /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        sys_charger_ok_task,            /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        OS_MINIMAL_TASK_STACK_SIZE,     /* The number of bytes to allocate to the
                                                           stack of the task. */
                        OS_TASK_PRIORITY_HIGHEST - 2,   /* The priority assigned to the task. */
                        sys_charger_ok_task_h);         /* The task handle */

        OS_ASSERT(sys_charger_ok_task_h);

        OS_QUEUE_CREATE(sys_charger_ok_task_msg_queue, sizeof(HW_CHARGER_MAIN_FSM_STATE), SYS_CHARGER_MAX_QUEUE_SIZE);
        ASSERT_ERROR(sys_charger_ok_task_msg_queue != NULL);

        OS_TASK_CREATE("CH_NOK",                        /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        sys_charger_nok_task,           /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        OS_MINIMAL_TASK_STACK_SIZE,     /* The number of bytes to allocate to the
                                                           stack of the task. */
                        OS_TASK_PRIORITY_HIGHEST - 2,   /* The priority assigned to the task. */
                        sys_charger_nok_task_h);        /* The task handle */

        OS_ASSERT(sys_charger_nok_task_h);

        OS_TASK_CREATE("SW_FSM",                        /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        sys_charger_kick_sw_fsm_task,   /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        OS_MINIMAL_TASK_STACK_SIZE,     /* The number of bytes to allocate to the
                                                            stack of the task. */
                        OS_TASK_PRIORITY_HIGHEST - 3,    /* The priority assigned to the task. */
                        sys_charger_kick_sw_fsm_task_h); /* The task handle */

        OS_ASSERT(sys_charger_kick_sw_fsm_task_h);

#if (PORT_DETECTION_IMPL == IN_SW)
        sys_charger_kick_sw_fsm_timer_h =
                OS_TIMER_CREATE("SW_FSM_TIM",
                                 OS_MS_2_TICKS(10),     /* Expire after 10 msec */
                                 OS_TIMER_SUCCESS,      /* Run repeatedly */
                                 (void *) 0,
                                 sys_charger_kick_sw_fsm_timer_cb);

        OS_ASSERT(sys_charger_kick_sw_fsm_timer_h != NULL);
#endif
}

#endif /* (dg_configUSE_SYS_CHARGER == 1) */
