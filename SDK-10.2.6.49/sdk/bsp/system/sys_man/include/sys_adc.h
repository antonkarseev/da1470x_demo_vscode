/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_ADC Analog Digital Converter Service
 *
 * \brief Functions for ADC Service
 *
 * \{
 */
/**
 ****************************************************************************************
 *
 * @file sys_adc.h
 *
 * @brief sys_adc header file.
 *
 * Copyright (C) 2018-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configUSE_SYS_ADC == 1)

#ifndef SYS_ADC_H_
#define SYS_ADC_H_

#include "ad_gpadc.h"
///@cond INTERNAL
#include "../adapters/src/sys_platform_devices_internal.h"
///@endcond

/**
 * \brief Initialize sys_adc service
 */
void sys_adc_init(void);
/**
 * \brief Enable sys_adc service
 *
 */
void sys_adc_enable(void);

/**
 * \brief Disable sys_adc service
 *
 */
void sys_adc_disable(void);

/**
 * \brief Trigger sys_adc service
 *
 * \note Function triggers the sys adc service in case the time difference
 * between the upcoming and last GPADC measurement exceeds a specified time threshold
 */
__RETAINED_HOT_CODE void sys_adc_trigger(void);


#endif /* SYS_ADC_H_ */

#endif /* (dg_configUSE_SYS_ADC == 1) */

/**
 \}
 \}
 */
