/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup PMU_ADAPTER PMU Adapter
 *
 * \brief Power Management Unit adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_pmu.h
 *
 * @brief PMU adapter API
 *
 * Copyright (C) 2017-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_PMU_H_
#define AD_PMU_H_

#if dg_configPMU_ADAPTER

#include "hw_pmu.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * \brief Rail selection
 *
 */
typedef enum {
        PMU_RAIL_1V2 = 1,       //!< 1V2 rail
        PMU_RAIL_1V4,           //!< 1V4 rail
        PMU_RAIL_1V8F,          //!< 1V8F rail
        PMU_RAIL_1V8P,          //!< 1V8P rail
        PMU_RAIL_1V8,           //!< 1V8 rail
        PMU_RAIL_3V0,           //!< 3V0 rail
        PMU_RAIL_VLED,          //!< VLED rail
        PMU_RAIL_VSYS,          //!< VSYS rail
} AD_PMU_RAIL;

/**
 * \brief Rail configuration
 *
 */
typedef struct {
        bool enabled_onwakeup;                                  //!< true if rail is enabled in wakeup / active state
        bool enabled_onsleep;                                   //!< true if rail is enabled in sleep state
        union {

                struct {
                        HW_PMU_1V2_VOLTAGE voltage_onwakeup;    //!< 1V2 rail voltage configuration in wakeup / active state
                        HW_PMU_1V2_VOLTAGE voltage_onsleep;     //!< 1V2 rail voltage configuration in sleep state
                        HW_PMU_1V2_MAX_LOAD current_onwakeup;   //!< 1V2 rail current configuration in wakeup / active state
                        HW_PMU_1V2_MAX_LOAD current_onsleep;    //!< 1V2 rail current configuration in sleep state
                } rail_1v2;                                     //!< 1V2 rail voltage and current configuration

                struct {
                        HW_PMU_1V4_VOLTAGE voltage_common;      //!< 1V4 rail common voltage configuration for wakeup / active / sleep state
                        HW_PMU_1V4_MAX_LOAD current_onwakeup;   //!< 1V4 rail current configuration in wakeup / active state
                        HW_PMU_1V4_MAX_LOAD current_onsleep;    //!< 1V4 rail current configuration in sleep state
                } rail_1v4;                                     //!< 1V4 rail voltage and current configuration

                struct {
                        HW_PMU_1V8F_MAX_LOAD current_onwakeup;  //!< 1V8F rail current configuration in wakeup / active state
                        HW_PMU_1V8F_MAX_LOAD current_onsleep;   //!< 1V8F rail current configuration in sleep state
                } rail_1v8f;                                    //!< 1V8F rail current configuration

                struct {
                        HW_PMU_1V8P_MAX_LOAD current_onwakeup;  //!< 1V8P rail current configuration in wakeup / active state
                        HW_PMU_1V8P_MAX_LOAD current_onsleep;   //!< 1V8P rail current configuration in sleep state
                } rail_1v8p;                                    //!< 1V8P rail current configuration

                struct {
                        HW_PMU_1V8_VOLTAGE voltage_common;      //!< 1V8 rail common voltage configuration for wakeup / active / sleep
                        HW_PMU_1V8_MAX_LOAD current_onwakeup;   //!< 1V8 rail current configuration in wakeup / active state
                        HW_PMU_1V8_MAX_LOAD current_onsleep;    //!< 1V8 rail current configuration in sleep state
                } rail_1v8;                                     //!< 1V8 rail voltage and current configuration

                struct {
                        HW_PMU_3V0_VOLTAGE voltage_onwakeup;    //!< 3V0 rail voltage configuration in wakeup / active state
                        HW_PMU_3V0_VOLTAGE voltage_onsleep;     //!< 3V0 rail voltage configuration in sleep state
                        HW_PMU_3V0_MAX_LOAD current_onwakeup;   //!< 3V0 rail current configuration in wakeup / active state
                        HW_PMU_3V0_MAX_LOAD current_onsleep;    //!< 3V0 rail current configuration in sleep state
                } rail_3v0;                                     //!< 3V0 rail voltage and current configuration

                struct {
                        HW_PMU_VLED_VOLTAGE voltage_common;     //!< VLED rail common voltage configuration for wakeup / active / sleep
                        HW_PMU_VLED_MAX_LOAD current_onwakeup;  //!< VLED rail current configuration in wakeup / active state
                        HW_PMU_VLED_MAX_LOAD current_onsleep;   //!< VLED rail current configuration in sleep state
                } rail_vled;                                    //!< VLED rail voltage and current configuration

                struct {
                        HW_PMU_VSYS_VOLTAGE voltage_common;     //!< VSYS rail common voltage configuration for wakeup / active / sleep
                } rail_vsys;                                    //!< VSYS rail voltage configuration

        };                                                      //!< rail voltage and current configuration

} ad_pmu_rail_config_t;

/**
 * \brief Configure a power rail
 *
 * \return 0 if the rail is configured, >0 otherwise
 *
 * \param[in] rail the rail to configure
 * \param[in] config a pointer to the structure holding the rail configuration
 */
int ad_pmu_configure_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* dg_configPMU_ADAPTER */


#endif /* AD_PMU_H_ */

/**
 * \}
 * \}
 */
