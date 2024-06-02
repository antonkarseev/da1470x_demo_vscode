/**
 ****************************************************************************************
 *
 * @file hw_charger.c
 *
 * @brief Implementation of the HW Charger Low Level Driver.
 *
 * Copyright (C) 2018-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configUSE_HW_CHARGER == 1)

#include "hw_charger.h"

__RETAINED static hw_charger_fsm_ok_cb_t  hw_charger_fsm_ok_cb;
__RETAINED static hw_charger_fsm_nok_cb_t hw_charger_fsm_nok_cb;


static const uint16_t charger_i_level_to_miliamp_lut[] = {
          5,  10,  15,  20,  25,  30,  35,  40,  45,  50,
         55,  60,  65,  70,  75,  80,  90, 100, 110, 120,
        130, 140, 150, 160, 170, 180, 190, 200, 210, 220,
        230, 240,
        270, 300, 330, 360, 390, 420, 450, 480,
        510, 540, 570, 600, 630, 660, 690, 720
};


uint16_t hw_charger_i_level_to_miliamp(HW_CHARGER_I_LEVEL level)
{
        ASSERT_WARNING(level <= HW_CHARGER_I_LEVEL_720);
        return charger_i_level_to_miliamp_lut[level];
}

void hw_charger_enable_fsm_ok_interrupt(hw_charger_fsm_ok_cb_t cb)
{
        ASSERT_ERROR(cb);
        hw_charger_fsm_ok_cb = cb;
        hw_charger_clear_ok_irq();
        NVIC_ClearPendingIRQ(CHARGER_STATE_IRQn);
        NVIC_EnableIRQ(CHARGER_STATE_IRQn);
}

void hw_charger_disable_fsm_ok_interrupt(void)
{
        hw_charger_clear_ok_irq();
        NVIC_DisableIRQ(CHARGER_STATE_IRQn);
        NVIC_ClearPendingIRQ(CHARGER_STATE_IRQn);
        hw_charger_fsm_ok_cb = NULL;
}

void hw_charger_enable_fsm_nok_interrupt(hw_charger_fsm_nok_cb_t cb)
{
        ASSERT_ERROR(cb);
        hw_charger_fsm_nok_cb = cb;
        hw_charger_clear_nok_irq();
        NVIC_ClearPendingIRQ(CHARGER_ERROR_IRQn);
        NVIC_EnableIRQ(CHARGER_ERROR_IRQn);
}

void hw_charger_disable_fsm_nok_interrupt(void)
{
        hw_charger_clear_nok_irq();
        NVIC_DisableIRQ(CHARGER_ERROR_IRQn);
        NVIC_ClearPendingIRQ(CHARGER_ERROR_IRQn);
        hw_charger_fsm_nok_cb = NULL;
}

void hw_charger_program_charging_profile(const hw_charger_charging_profile_t *prof)
{
        ASSERT_ERROR(prof);


        const bool initial_sw_lock_mode = hw_charger_get_sw_lock_mode();

        if (initial_sw_lock_mode) {
                if (hw_charger_get_sw_lock_status()) {
                        /* SW lock is active, apply the unlock sequence
                         * first to be able to program the charger.
                         */
                        hw_charger_apply_sw_unlock_sequence();
                }
        }


        /* Process control flags */

        if ((prof->ctrl_flags) & HW_CHARGER_CTRL_ENABLE_DIE_TEMP_PROTECTION) {
                hw_charger_set_die_temp_protection_limit(prof->die_temp_limit);
                hw_charger_set_die_temp_protection_mode(true);
        } else {
                hw_charger_set_die_temp_protection_mode(false);
        }

        if ((prof->ctrl_flags) & HW_CHARGER_CTRL_ENABLE_BAT_TEMP_PROTECTION) {
                hw_charger_set_bat_temp_protection_mode(true);
        } else {
                hw_charger_set_bat_temp_protection_mode(false);
        }

        if ((prof->ctrl_flags) & HW_CHARGER_CTRL_ENABLE_BAT_LOW_TEMP) {
                hw_charger_set_bat_low_temp_mode(true);
        } else {
                hw_charger_set_bat_low_temp_mode(false);
        }

        if ((prof->ctrl_flags) & HW_CHARGER_CTRL_HALT_CHARGE_TIMERS_ON_TEMP_PROTECTION_STATES) {
                hw_charger_halt_timers_on_temp_protection_states(true);
        } else {
                hw_charger_halt_timers_on_temp_protection_states(false);
        }

        if ((prof->ctrl_flags) & HW_CHARGER_CTRL_RESUME_FROM_DIE_PROTECTION_STATE) {
                hw_charger_set_resume_behavior_on_die_temp_protection_state(true);
        } else {
                hw_charger_set_resume_behavior_on_die_temp_protection_state(false);
        }

        if ((prof->ctrl_flags) & HW_CHARGER_CTRL_RESUME_FROM_ERROR_STATE) {
                hw_charger_set_resume_mode(true);
        } else {
                hw_charger_set_resume_mode(false);
        }

        if ((prof->ctrl_flags) & HW_CHARGER_CTRL_ENABLE_JEITA_SUPPORT) {

                /* Process JEITA temperature parameters. Only warmer, warm, cool and cooler limits are of interest. */
                hw_charger_set_jeita_warm_temp_limit(prof->bat_temp_warm_limit);
                hw_charger_set_jeita_cool_temp_limit(prof->bat_temp_cool_limit);
                hw_charger_set_jeita_cooler_temp_limit(prof->bat_temp_cooler_limit);
                hw_charger_set_jeita_warmer_temp_limit(prof->bat_temp_warmer_limit);
                /* Process JEITA voltage parameters */

                hw_charger_set_jeita_warm_ovp_level(prof->jeita_ovp_warm_level);
                hw_charger_set_jeita_cool_ovp_level(prof->jeita_ovp_cool_level);

                hw_charger_set_jeita_warm_replenish_level(prof->jeita_replenish_v_warm_level);
                hw_charger_set_jeita_cool_replenish_level(prof->jeita_replenish_v_cool_level);

                hw_charger_set_jeita_warm_precharged_voltage_threshold(prof->jeita_precharged_v_warm_thr);
                hw_charger_set_jeita_cool_precharged_voltage_threshold(prof->jeita_precharged_v_cool_thr);

                hw_charger_set_jeita_warm_const_voltage_level(prof->jeita_cv_warm_level);
                hw_charger_set_jeita_cool_const_voltage_level(prof->jeita_cv_cool_level);

                hw_charger_set_jeita_cooler_ovp_level(prof->jeita_ovp_cooler_level);
                hw_charger_set_jeita_cooler_replenish_level(prof->jeita_replenish_v_cooler_level);
                hw_charger_set_jeita_cooler_precharged_voltage_threshold(prof->jeita_precharged_v_cooler_thr);
                hw_charger_set_jeita_cooler_const_voltage_level(prof->jeita_cv_cooler_level);

                hw_charger_set_jeita_warmer_ovp_level(prof->jeita_ovp_warmer_level);
                hw_charger_set_jeita_warmer_replenish_level(prof->jeita_replenish_v_warmer_level);
                hw_charger_set_jeita_warmer_precharged_voltage_threshold(prof->jeita_precharged_v_warmer_thr);
                hw_charger_set_jeita_warmer_const_voltage_level(prof->jeita_cv_warmer_level);

                /* Process JEITA current parameters */

                hw_charger_set_jeita_warm_precharge_const_current_level(prof->jeita_precharge_cc_warm_level);
                hw_charger_set_jeita_cool_precharge_const_current_level(prof->jeita_precharge_cc_cool_level);

                hw_charger_set_jeita_warm_const_current_level(prof->jeita_cc_warm_level);
                hw_charger_set_jeita_cool_const_current_level(prof->jeita_cc_cool_level);

                hw_charger_set_jeita_cooler_precharge_const_current_level(prof->jeita_precharge_cc_cooler_level);
                hw_charger_set_jeita_cooler_const_current_level(prof->jeita_cc_cooler_level);

                hw_charger_set_jeita_warmer_precharge_const_current_level(prof->jeita_precharge_cc_warmer_level);
                hw_charger_set_jeita_warmer_const_current_level(prof->jeita_cc_warmer_level);

                hw_charger_set_jeita_support_mode(true);
        } else {
                hw_charger_set_jeita_support_mode(false);
        }

        hw_charger_set_bat_temp_monitor_mode(prof->tbat_monitor_mode);

        /* Process IRQ parameters */

        hw_charger_set_ok_irq_mask(prof->irq_ok_mask);
        hw_charger_set_nok_irq_mask(prof->irq_nok_mask);

        /* Process voltage parameters */

        hw_charger_set_ovp_level(prof->ovp_level);
        hw_charger_set_replenish_level(prof->replenish_v_level);
        hw_charger_set_precharged_voltage_threshold(prof->precharged_v_thr);
        hw_charger_set_const_voltage_level(prof->cv_level);

        /* Process current parameters */

        hw_charger_set_eoc_current_threshold(prof->eoc_i_thr);
        hw_charger_set_precharge_const_current_level(prof->precharge_cc_level);
        hw_charger_set_const_current_level(prof->cc_level);

        /* Process temperature parameters */

        hw_charger_set_jeita_hot_temp_limit(prof->bat_temp_hot_limit);
        hw_charger_set_jeita_cold_temp_limit(prof->bat_temp_cold_limit);

        /* Process charging timeout parameters */

        hw_charger_set_max_precharging_timeout(prof->max_precharge_timeout);
        hw_charger_set_max_cc_charging_timeout(prof->max_cc_charge_timeout);
        hw_charger_set_max_cv_charging_timeout(prof->max_cv_charge_timeout);
        hw_charger_set_max_total_charging_timeout(prof->max_total_charge_timeout);


        /* Programming charger done. Lock registers if needed so. */

        if (initial_sw_lock_mode) {
                /* Retain the original lock status */
                hw_charger_apply_sw_lock_sequence();
        } else if ((prof->ctrl_flags) & HW_CHARGER_CTRL_ENABLE_SW_LOCK) {
                /* The SW lock mode is chosen by the charging profile. */
                hw_charger_enable_sw_lock_mode();
                hw_charger_apply_sw_lock_sequence();
        }

}

void hw_charger_program_fine_tuning_settings(const hw_charger_fine_tuning_settings_t *settings)
{
        ASSERT_ERROR(settings);

        hw_charger_set_vbat_comparator_settling_time(settings->vbat_comparator_settling_time);
        hw_charger_set_ovp_comparator_settling_time(settings->ovp_comparator_settling_time);
        hw_charger_set_tdie_comparator_settling_time(settings->tdie_comparator_settling_time);
        hw_charger_set_tbat_comparator_settling_time(settings->tbat_comparator_settling_time);
        hw_charger_set_tbat_hot_comparator_settling_time(settings->tbat_hot_comparator_settling_time);
        hw_charger_set_tbat_monitoring_time(settings->tbat_monitoring_time);
        hw_charger_set_charger_powering_up_time(settings->charger_powering_up_time);
        hw_charger_set_eoc_interval_check_threshold(settings->eoc_interval_check_threshold);
}

void Charger_State_Handler(void)
{
        HW_CHARGER_FSM_IRQ_STAT_OK status;

        status = hw_charger_get_ok_irq_status();

        if (hw_charger_fsm_ok_cb) {
                hw_charger_fsm_ok_cb(status);
        }
}

void Charger_Error_Handler(void)
{
        HW_CHARGER_FSM_IRQ_STAT_NOK status;

        status = hw_charger_get_nok_irq_status();

        if (hw_charger_fsm_nok_cb) {
                hw_charger_fsm_nok_cb(status);
        }
}

#endif /* (dg_configUSE_HW_CHARGER == 1) */
