/**
 ****************************************************************************************
 *
 * @file Xtal_TRIM.h
 *
 * @brief Xtal trim API
 *
 * Copyright (C) 2015-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef XTAL_TRIM_H_
#define XTAL_TRIM_H_

#define AUTO_XTAL_TEST_DBG_EN           0                       // Enable/Disable debug parameters.

// Status codes
#define XTAL_OPERATION_SUCCESS          (0)             // XTAL calibration success.
#define PULSE_OUT_OF_RANGE_ERROR        (-1)            // Pulse found in the pulse pin assigned GPIO was out of acceptable range
#define NO_PULSE_ERROR                  (-2)            // No pulse found, or pulse > 740 ms (measure_pulse aborts)
#define WRITING_VAL_TO_OTP_ERROR        (-3)            // Failed to write value in OTP.
#define INVALID_GPIO_ERROR              (-4)            // Wrong GPIO configuration.
#define WRONG_XTAL_SOURCE_ERROR         (-5)            // Incorrect pulse detected.
#define XTAL_CALIBRATION_ERROR          (-6)            // XTAL calibration error.

int16_t auto_trim(HW_GPIO_PORT port, HW_GPIO_PIN pin);

uint16_t run_xtal32m_cap_meas(void);

void delay(uint32_t dd);
void Setting_Trim(uint32_t Trim_Value);

uint32_t pulse_counter(void);                                   // counting pulses during 500 msec
uint32_t MEASURE_PULSE(int32_t datareg1, int32_t shift_bit1);   // see assembly code

#if AUTO_XTAL_TEST_DBG_EN
void TRIM_test (int S1, int S2);                                // testing
#endif

#endif /* XTAL_TRIM_H_ */

