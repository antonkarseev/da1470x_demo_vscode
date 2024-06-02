/**
 ****************************************************************************************
 *
 * @file hw_gpadc_v2.c
 *
 * @brief Implementation of the GPADC Low Level Driver.
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_GPADC

#include "hw_gpadc.h"
#include "sys_tcs.h"

/**
 * \brief A macro to define default temperature calibration point for the DIE_TEMP sensor
 */
#define HW_GPADC_TEMP_CALIB_POINT               (2500)

/**
 * \brief A macro to define default adc calibration point for the DIE_TEMP sensor
 */
#define HW_GPADC_ADC_CALIB_POINT                (693 << (HW_GPADC_UNUSED_BITS))

/*
 * DIE_TEMP temperature coefficient
 * as LSB per Celsius degree in the 16-bit resolution scale
 */
const int16_t die_temp_coefficient = 149;

/*
 * Default calibration point for the DIE_TEMP sensor.
 * Realistic but NOT at all accurate.
 */
#define DIE_TEMP_CALIBRATION_POINT_DEF          {.temp = HW_GPADC_TEMP_CALIB_POINT, .adc = HW_GPADC_ADC_CALIB_POINT}

/*
 * DIFFTEMP temperature coefficient
 * as LSB per Celsius degree in the 16-bit resolution scale
 */
const int16_t difftemp_coefficient = -82;

/*
 * Default calibration point for the DIFFTEMP sensor.
 * Realistic but NOT at all accurate.
 */
#define DIFFTEMP_CALIBRATION_POINT_DEF          {.temp = 2500, .adc = HW_GPADC_MID_SCALE_ADC}

/*
 * Declaring the variable calibration points
 */
__RETAINED_RW static hw_gpadc_calibration_point_t die_temp_calibration_point = DIE_TEMP_CALIBRATION_POINT_DEF;
__RETAINED_RW static hw_gpadc_calibration_point_t difftemp_calibration_point = DIFFTEMP_CALIBRATION_POINT_DEF;


void hw_gpadc_check_tcs_custom_values(int16_t se_gain_error, int16_t se_offset_error, int16_t diff_gain_error, int16_t diff_offset_error)
{
        if ((se_gain_error == 0) && (se_offset_error == 0)) {
                sys_tcs_apply_custom_values(SYS_TCS_GROUP_GP_ADC_SINGLE_MODE, sys_tcs_custom_values_system_cb, NULL);
        }
        if ((diff_gain_error == 0) && (diff_offset_error == 0)) {
                sys_tcs_apply_custom_values(SYS_TCS_GROUP_GP_ADC_DIFF_MODE, sys_tcs_custom_values_system_cb, NULL);
        }
        if ((die_temp_calibration_point.temp == HW_GPADC_TEMP_CALIB_POINT) && (die_temp_calibration_point.adc == HW_GPADC_ADC_CALIB_POINT)) {
                sys_tcs_apply_custom_values(SYS_TCS_GROUP_TEMP_SENS_25C, sys_tcs_custom_values_system_cb, NULL);
        }
}

int16_t hw_gpadc_get_voltage(void)
{
        return hw_gpadc_convert_to_millivolt(NULL, hw_gpadc_get_raw_value());
}

/***************************************************************************
 ******************      TEMPERATURE SENSOR functions  *********************
 ***************************************************************************/

void hw_gpadc_store_ambient_calibration_point(uint16_t raw_val, int16_t temp)
{
        gpadc_config temp_cfg;

        temp_cfg.positive = HW_GPADC_INP_DIE_TEMP;
        temp_cfg.input_mode = HW_GPADC_INPUT_MODE_SINGLE_ENDED;

        die_temp_calibration_point.temp = temp;
        die_temp_calibration_point.adc  = hw_gpadc_apply_correction(&temp_cfg, raw_val);
}

__STATIC_INLINE bool get_tempsens_conversion_factors(const HW_GPADC_TEMP_SENSORS sensor, hw_gpadc_calibration_point_t *cp, int16_t *tc)
{
        *tc = 0; /* Invalid coefficient */

        switch (sensor) {
        case HW_GPADC_TEMP_SENSOR_DIE_TEMP:
                *cp = die_temp_calibration_point;
                *tc = die_temp_coefficient;
                break;
        case HW_GPADC_TEMP_SENSOR_NEAR_RADIO:
        case HW_GPADC_TEMP_SENSOR_NEAR_BANDGAP:
        case HW_GPADC_TEMP_SENSOR_NEAR_CHARGER:
                *cp = difftemp_calibration_point;
                *tc = difftemp_coefficient;
                break;
        default:
                return false;
        }

        return true;
}

int16_t hw_gpadc_convert_to_celsius_x100_util(const gpadc_config *cfg, uint16_t raw_val)
{
        hw_gpadc_calibration_point_t cp = {0, 0};
        int16_t tc = 0;
        uint16_t corrected;
        HW_GPADC_INPUT_POSITIVE positive = cfg ? cfg->positive : hw_gpadc_get_positive();
        HW_GPADC_TEMP_SENSORS sensor = 0; /* invalid sensor */

        if (positive == HW_GPADC_INP_DIE_TEMP) {
                /*
                 * Apply correction only if the DIE_TEMP sensor is in use
                 */
                corrected = hw_gpadc_apply_correction(cfg, raw_val);
                sensor = HW_GPADC_TEMP_SENSOR_DIE_TEMP;
        } else if (positive == HW_GPADC_INP_DIFF_TEMP) {
                corrected = raw_val;
                /*
                 * Convert the difftemp sensor field to sensor enum member
                 */
                sensor = cfg ? cfg->temp_sensor : hw_gpadc_get_temp_sensor() + HW_GPADC_NO_TEMP_SENSOR;
        } else {
                /*
                 * Invalid input channel
                 */
                ASSERT_WARNING(0);
                corrected = raw_val;
        }
        ASSERT_WARNING(get_tempsens_conversion_factors(sensor, &cp, &tc) == true);

        int32_t accurate_ratio = (int32_t)((corrected - cp.adc) * 100) / (int32_t)tc;
        return cp.temp + (int16_t)accurate_ratio;
}

uint16_t hw_gpadc_convert_celsius_x100_to_raw_val_util(const gpadc_config *cfg, int16_t temperature)
{
        hw_gpadc_calibration_point_t cp = {0, 0};
        int16_t tc = 0;
        HW_GPADC_TEMP_SENSORS sensor = 0; /* invalid sensor */
        HW_GPADC_INPUT_POSITIVE positive = cfg ? cfg->positive : hw_gpadc_get_positive();

        if (positive == HW_GPADC_INP_DIE_TEMP) {
                sensor = HW_GPADC_TEMP_SENSOR_DIE_TEMP;
        } else if (positive == HW_GPADC_INP_DIFF_TEMP) {
                /*
                 * Convert the difftemp sensor field to sensor enum member
                 */
                sensor = cfg ? cfg->temp_sensor : hw_gpadc_get_temp_sensor() + HW_GPADC_NO_TEMP_SENSOR;
        }
        ASSERT_WARNING(get_tempsens_conversion_factors(sensor, &cp, &tc) == true);

        int32_t accurate_ratio = ((temperature - cp.temp) * tc) / 100;
        return (uint16_t)((int32_t)cp.adc + accurate_ratio);
}

#endif /* dg_configUSE_HW_GPADC */
