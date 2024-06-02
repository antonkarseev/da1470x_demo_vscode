/**
 ****************************************************************************************
 *
 * @file sys_adc_da1470x.c
 *
 * @brief sys_adc_da1470x source file.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configUSE_SYS_ADC == 1)

#include "sys_adc.h"
#include "sys_bsr.h"
#include "sys_clock_mgr_internal.h"
#include "sys_timer.h"
#include "osal.h"
#if dg_configRF_ENABLE_RECALIBRATION
#include "ble_config.h"
#include "ad_ble.h"
#endif
/* Defining DISABLE_TEMP_FEED macro SNC feeds the shared space variable with 0's instead of the
 * gpadc measurement. Nevertheless the rest of the sys adc service functionality remains intact */
//#define DISABLE_TEMP_FEED
#if dg_configRF_ENABLE_RECALIBRATION && dg_configENABLE_RCHS_CALIBRATION
#define SYS_ADC_PERIOD_TICKS            OS_MS_2_TICKS(MIN(dg_configRF_CALIB_TEMP_POLL_INTV, RCHS_TEMP_POLL_INT))
#elif dg_configRF_ENABLE_RECALIBRATION
#define SYS_ADC_PERIOD_TICKS            OS_MS_2_TICKS(dg_configRF_CALIB_TEMP_POLL_INTV)
#elif dg_configENABLE_RCHS_CALIBRATION
#define SYS_ADC_PERIOD_TICKS            OS_MS_2_TICKS(RCHS_TEMP_POLL_INT)
#endif
#define mainBIT_SYS_ADC_EN              (1 << 1)
#define mainBIT_SYS_ADC_DIS             (1 << 2)
#define mainBIT_SYS_ADC_TMR_CALL        (1 << 3)
#define mainBIT_SYS_ADC_TRIGGER         (1 << 4)
#define SYS_ADC_PRIORITY                (OS_TASK_PRIORITY_NORMAL)
#define SYS_ADC_TIME_THRESHOLD          (SYS_ADC_PERIOD_TICKS / 2)

static void sys_adc_timer_callback(OS_TIMER pxTimer);

static OS_TASK handle_sys_adc;
static OS_TIMER sys_adc_timer;
static OS_TASK_FUNCTION(Sys_ADC, pvParameters);

__RETAINED static OS_TICK_TIME previous_tick;
__RETAINED static uint16_t cur_temp_value;
#if dg_configENABLE_RCHS_CALIBRATION
__RETAINED static uint16_t last_trigger_temp_value;
__RETAINED static uint32_t uncond_trigger_cnt;
#endif

void sys_adc_init(void)
{
        OS_BASE_TYPE status;
#if dg_configENABLE_RCHS_CALIBRATION
        ad_gpadc_handle_t handle = ad_gpadc_open(&TEMP_SENSOR_RADIO_INTERNAL);
#if CONFIG_GPADC_USE_SYNC_TRANSACTIONS
        ad_gpadc_read_nof_conv(handle, 1, &last_trigger_temp_value);
#else
        hw_gpadc_read(1, &last_trigger_temp_value, NULL, NULL);
#endif
        ad_gpadc_close(handle, true);
#endif
        sys_adc_timer = OS_TIMER_CREATE("Sys_adcSet",
                                SYS_ADC_PERIOD_TICKS,
                                OS_TIMER_RELOAD,
                                (void *) 0,                     // Timer id == none
                                sys_adc_timer_callback);        // Call-back
        OS_ASSERT(sys_adc_timer != NULL);

        /* Create Sys_ADC task */
        status = OS_TASK_CREATE("Sys_ADC",              /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                                Sys_ADC,                /* The function that implements the task. */
                                0,                      /* The parameter passed to the task. */
                                OS_MINIMAL_TASK_STACK_SIZE,
                                SYS_ADC_PRIORITY,       /* The priority assigned to the task. */
                                handle_sys_adc);
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);
}

static void sys_adc_timer_callback(OS_TIMER pxTimer)
{
        OS_TASK_NOTIFY(handle_sys_adc, mainBIT_SYS_ADC_TMR_CALL, OS_NOTIFY_SET_BITS);
}

/**
 * \brief Sys_ADC task
 *
 */
static OS_TASK_FUNCTION(Sys_ADC, pvParameters)
{
        uint32_t ulNotifiedValue;

        while (1) {
                OS_TASK_NOTIFY_WAIT(OS_TASK_NOTIFY_NONE, OS_TASK_NOTIFY_ALL_BITS, &ulNotifiedValue,
                                                                            OS_TASK_NOTIFY_FOREVER);

                if (ulNotifiedValue & mainBIT_SYS_ADC_EN) {

                        OS_TIMER_START(sys_adc_timer, OS_TIMER_FOREVER);

                } else if (ulNotifiedValue & mainBIT_SYS_ADC_DIS) {

                        OS_TIMER_STOP(sys_adc_timer, OS_TIMER_FOREVER);

                } else if (ulNotifiedValue & (mainBIT_SYS_ADC_TMR_CALL | mainBIT_SYS_ADC_TRIGGER)) {

                        ad_gpadc_handle_t sys_adc_handle = ad_gpadc_open(&TEMP_SENSOR_RADIO_INTERNAL);
#if CONFIG_GPADC_USE_SYNC_TRANSACTIONS
                        ad_gpadc_read_nof_conv(sys_adc_handle, 1, &cur_temp_value);
#else
                        hw_gpadc_read(1, &cur_temp_value, NULL, NULL);
#endif

#if dg_configRF_ENABLE_RECALIBRATION
#ifdef DISABLE_TEMP_FEED
                        rf_calibration_info = 0;
#else
                        rf_calibration_info = cur_temp_value;
#endif
#endif

                        ad_gpadc_close(sys_adc_handle, true);
#if dg_configENABLE_RCHS_CALIBRATION
                        if ((cur_temp_value > (last_trigger_temp_value + RCHS_TEMP_DRIFT)) ||
                                (cur_temp_value < (last_trigger_temp_value - RCHS_TEMP_DRIFT)) ||
                                (uncond_trigger_cnt == RCHS_UNCOND_TRIGGER - 1)) {
                                last_trigger_temp_value = cur_temp_value;
                                cm_rchs_calibration_notify();
                        }

                        if (ulNotifiedValue & mainBIT_SYS_ADC_TMR_CALL) {
                                uncond_trigger_cnt++;
                                if (uncond_trigger_cnt == RCHS_UNCOND_TRIGGER) {
                                        uncond_trigger_cnt = 0;
                                }
                        }
#endif
                        if (ulNotifiedValue & mainBIT_SYS_ADC_TRIGGER) {
                                OS_TIMER_RESET(sys_adc_timer, OS_TIMER_FOREVER);
                        }
                        previous_tick = OS_GET_TICK_COUNT();
                }
        }
}

void sys_adc_enable(void)
{
        OS_TASK_NOTIFY(handle_sys_adc, mainBIT_SYS_ADC_EN, OS_NOTIFY_SET_BITS);
}

void sys_adc_disable(void)
{
        OS_TASK_NOTIFY(handle_sys_adc, mainBIT_SYS_ADC_DIS, OS_NOTIFY_SET_BITS);
}

__RETAINED_HOT_CODE void sys_adc_trigger(void)
{
        OS_TICK_TIME current_tick =  OS_GET_TICK_COUNT();

        if ((current_tick - previous_tick) >= SYS_ADC_TIME_THRESHOLD) {

                OS_TIMER_STOP(sys_adc_timer, OS_TIMER_FOREVER);
                OS_TASK_NOTIFY(handle_sys_adc, mainBIT_SYS_ADC_TRIGGER, OS_NOTIFY_SET_BITS);
        }
}
#endif /* (dg_configUSE_SYS_ADC == 1) */
