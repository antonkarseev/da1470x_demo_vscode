/**
 ****************************************************************************************
 *
 * @file ad_pmu_internal.h
 *
 * @brief PMU internal adapter API - Should be excluded from documentation
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_PMU_INTERNAL_H_
#define AD_PMU_INTERNAL_H_

#if dg_configPMU_ADAPTER

/**
 * \brief Requests the system to force the voltage level of 1V2 rail to 1.2V
 *
 * The voltage level of 1V2 rail raises to 1.2V. The last configuration of 1V2 is not lost.
 * It will be restored when ad_pmu_1v2_force_max_voltage_release() is called equal times
 * the number of ad_pmu_1v2_force_max_voltage_request() calls.
 *
 * \warning The PM Adapter API user MUST ensure that any request is matched by the respective release.
 *          Otherwise the system will reach an error-state!
 */
void ad_pmu_1v2_force_max_voltage_request(void);

/**
 * \brief Restore the 1V2 rail configuration. It terminates a matching request.
 *
 * The 1V2 rail is restored to the configuration that was applied before calling
 * ad_pmu_1v2_force_max_voltage_request().
 *
 * \warning This function MUST be called always to terminate a matching ad_pmu_1v2_force_max_voltage_request().
 *          If called alone the system will reach an error-state!
 */
void ad_pmu_1v2_force_max_voltage_release(void);

/**
 * \brief Prepare for sleep.
 *
 * Hook for PMU actions before going to sleep.
 */
__RETAINED_HOT_CODE void ad_pmu_prepare_for_sleep(void);

/**
 * \brief Restore for system wake-up.
 *
 * Hook for PMU actions after waking up.
 */
void ad_pmu_restore_for_wake_up(void);

#endif /* dg_configPMU_ADAPTER */


#endif /* AD_PMU_INTERNAL_H_ */
