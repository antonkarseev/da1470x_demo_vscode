/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup HW_PMU Power Manager Driver
\{
\brief Power Manager
*/

/**
****************************************************************************************
*
* @file hw_pmu_da1470x.h
*
* @brief Power Manager header file for DA1470x.
*
* Copyright (C) 2020-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef HW_PMU_DA1470x_H_
#define HW_PMU_DA1470x_H_


#if dg_configUSE_HW_PMU

#include "sdk_defs.h"

/**
 * \brief PMU API Error Codes
 *
 */
typedef enum {
        HW_PMU_ERROR_NOERROR                    = 0,    //!< No Error
        HW_PMU_ERROR_INVALID_ARGS               = 1,    //!< Invalid arguments
        HW_PMU_ERROR_NOT_ENOUGH_POWER           = 2,    //!< Current LDO config cannot supply enough power for this config
        HW_PMU_ERROR_RCLP_ON                    = 3,    //!< RCLP is on
        HW_PMU_ERROR_RCLP_LP                    = 4,    //!< RCLP set as LP clock
        HW_PMU_ERROR_XTAL32M_ON                 = 5,    //!< XTAL32M is on
        HW_PMU_ERROR_RCX_ON                     = 6,    //!< RCX is on
        HW_PMU_ERROR_RCX_LP                     = 7,    //!< RCX set as LP clock
        HW_PMU_ERROR_XTAL32K_ON                 = 8,    //!< XTAL32K is on
        HW_PMU_ERROR_XTAL32K_LP                 = 9,    //!< XTAL32K set as LP clock
        HW_PMU_ERROR_RCHS_ON                    = 10,   //!< RCHS is on
        HW_PMU_ERROR_PLL_ON                     = 11,   //!< PLL is on
        HW_PMU_ERROR_HIGH_SPEED_CLK_ON          = 12,   //!< A high speed clock is on
        HW_PMU_ERROR_WAKEUP_SOURCE_ON           = 13,   //!< A wakeup source is on
        HW_PMU_ERROR_UFAST_WAKEUP_ON            = 14,   //!< Ultra fast wakeup is on
        HW_PMU_ERROR_ACTION_NOT_POSSIBLE        = 15,   //!< Action not possible to execute
        HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY     = 16,   //!< Other loads dependency
        HW_PMU_ERROR_BOD_IS_ACTIVE              = 17,   //!< BOD is active
        HW_PMU_ERROR_USB_PHY_ON                 = 18,   //!< USB_PHY is on
        HW_PMU_ERROR_OTP_ON                     = 19    //!< OTP is on
} HW_PMU_ERROR_CODE;


/**
 * \brief PMU API Source type
 *
 * This allows the user to select whether he wants to use a high-efficiency and high-ripple
 * source (DCDC) or low-efficiency and low-ripple source (LDO) for a Power Rail
 */
typedef enum {
        HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE          = 0,    //!< Low ripple source (LDO)
        HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY    = 1,    //!< High efficiency (and ripple) source (DCDC)
        HW_PMU_SRC_TYPE_VSYS                    = 2,    //!< Bypass mode for VLED rail
        HW_PMU_SRC_TYPE_AUTO                    = 3,    //!< Power selection done automatically by the HW
        HW_PMU_SRC_TYPE_CLAMP                   = 4,    //!< Clamp power source
        HW_PMU_SRC_TYPE_1V8P                    = 5     //!< 1V8P rail power source
} HW_PMU_SRC_TYPE;

/**
 * \brief Power rail state (enabled or disabled)
 *
 *  Depending on the context it either imply disabled / enabled in sleep state or
 *  active / wakeup state.
 */
typedef enum {
        POWER_RAIL_DISABLED = 0,        //!< The rail is disabled
        POWER_RAIL_ENABLED  = 1         //!< The rail is enabled
} HW_PMU_POWER_RAIL_STATE;


/***************************************** VLED **************************************************/

/**
 * \brief Voltage level options for the VLED power rail
 *
 */
typedef enum {
        HW_PMU_VLED_VOLTAGE_4V5         = 0,    //!< 4.5V
        HW_PMU_VLED_VOLTAGE_4V75        = 1,    //!< 4.75V
        HW_PMU_VLED_VOLTAGE_5V0         = 2,    //!< 5.0V
} HW_PMU_VLED_VOLTAGE;

/**
 * \brief Maximum load current options for the VLED power rail
 *
 */
typedef enum {
        HW_PMU_VLED_MAX_LOAD_0_300     = 0,     //!< 300uA supplied by BOOST_DCDC
        HW_PMU_VLED_MAX_LOAD_150       = 1,     //!< 150mA supplied by BOOST_DCDC
} HW_PMU_VLED_MAX_LOAD;

/**
 * \brief VLED power rail configuration
 *
 */
typedef struct {
        HW_PMU_VLED_VOLTAGE voltage;
        HW_PMU_VLED_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_VLED_RAIL_CONFIG;


/***************************************** VSYS **************************************************/

/**
 * \brief Voltage level options for the VSYS power rail
 *
 */
typedef enum {
        HW_PMU_VSYS_VOLTAGE_4V2 = 3,    //!< 4.2V
        HW_PMU_VSYS_VOLTAGE_4V4 = 2,    //!< 4.4V
        HW_PMU_VSYS_VOLTAGE_4V6 = 1,    //!< 4.6V
        HW_PMU_VSYS_VOLTAGE_4V8 = 0,    //!< 4.8V
} HW_PMU_VSYS_VOLTAGE;

/**
 * \brief Maximum load current options for the VSYS power rail
 *
 */
typedef enum {
        HW_PMU_VSYS_MAX_LOAD_1000 = 0   //!< 1000mA
} HW_PMU_VSYS_MAX_LOAD;

/**
 * \brief VSYS power rail configuration
 *
 */
typedef struct {
        HW_PMU_VSYS_VOLTAGE voltage;
        HW_PMU_VSYS_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_VSYS_RAIL_CONFIG;

/***************************************** V30 ***************************************************/

/**
 * \brief Voltage level options for the 3V0 power rail
 *
 */
typedef enum {
        /* active state */
        HW_PMU_3V0_VOLTAGE_3V0          = 0,    //!< 3.00V during active state
        HW_PMU_3V0_VOLTAGE_3V3          = 3,    //!< 3.30V during active state
        /* sleep state
         * Only the two least significant bits of the enumerated value are used
         * for programming the rail.
         */
        HW_PMU_3V0_VOLTAGE_SLEEP_3V0    = 4,    //!< 3.00V during sleep state
        HW_PMU_3V0_VOLTAGE_SLEEP_3V3    = 7     //!< 3.30V during sleep state
} HW_PMU_3V0_VOLTAGE;


/**
 * \brief Maximum load current options for the 3V0 power rail
 *
 */
typedef enum {
        HW_PMU_3V0_MAX_LOAD_1   = 0,    //!< 1mA
        HW_PMU_3V0_MAX_LOAD_10  = 1,    //!< 10mA
        HW_PMU_3V0_MAX_LOAD_150 = 2,    //!< 150mA
        HW_PMU_3V0_MAX_LOAD_160 = 3     //!< 160mA
} HW_PMU_3V0_MAX_LOAD;

/**
 * \brief 3V0 power rail configuration
 *
 */
typedef struct {
        HW_PMU_3V0_VOLTAGE voltage;
        HW_PMU_3V0_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_3V0_RAIL_CONFIG;

/***************************************** V18 ***************************************************/

/**
 * \brief Voltage level options for the 1V8 power rail
 *
 */
typedef enum {
        HW_PMU_1V8_VOLTAGE_1V2 = 0,     //!< 1.2V
        HW_PMU_1V8_VOLTAGE_1V8 = 1      //!< 1.8V
} HW_PMU_1V8_VOLTAGE;


/**
 * \brief Maximum load current options for the 1V8 power rail
 *
 */
typedef enum {
        HW_PMU_1V8_MAX_LOAD_100 = 0     //!< 100mA
} HW_PMU_1V8_MAX_LOAD;


/**
 * \brief 1V8 power rail configuration
 *
 */
typedef struct {
        HW_PMU_1V8_VOLTAGE voltage;
        HW_PMU_1V8_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_1V8_RAIL_CONFIG;

/***************************************** V18P **************************************************/

/**
 * \brief Voltage level options for the 1V8P power rail
 *
 */
typedef enum {
        HW_PMU_1V8P_VOLTAGE_1V8 = 0     //!< 1.8V
} HW_PMU_1V8P_VOLTAGE;


/**
 * \brief Maximum load current options for the 1V8P power rail
 *
 */
typedef enum {
        HW_PMU_1V8P_MAX_LOAD_100 = 0    //!< 100mA
} HW_PMU_1V8P_MAX_LOAD;


/**
 * \brief 1V8P power rail configuration
 *
 */
typedef struct {
        HW_PMU_1V8P_VOLTAGE voltage;
        HW_PMU_1V8P_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_1V8P_RAIL_CONFIG;

/***************************************** V18F **************************************************/

/**
 * \brief Voltage level options for the 1V8F power rail
 *
 */
typedef enum {
        HW_PMU_1V8F_VOLTAGE_1V8 = 0     //!< 1.8V
} HW_PMU_1V8F_VOLTAGE;


/**
 * \brief Maximum load current options for the 1V8F power rail
 *
 */
typedef enum {
        HW_PMU_1V8F_MAX_LOAD_100 = 0    //!< 100mA
} HW_PMU_1V8F_MAX_LOAD;


/**
 * \brief 1V8F power rail configuration
 *
 */
typedef struct {
        HW_PMU_1V8F_VOLTAGE voltage;
        HW_PMU_1V8F_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_1V8F_RAIL_CONFIG;

/***************************************** V14 ***************************************************/

/**
 * \brief Voltage level options for the 1V4 power rail
 *
 */
typedef enum {
        HW_PMU_1V4_VOLTAGE_1V2 = 0,     //!< 1.2V
        HW_PMU_1V4_VOLTAGE_1V3 = 1,     //!< 1.3V
        HW_PMU_1V4_VOLTAGE_1V4 = 2,     //!< 1.4V
        HW_PMU_1V4_VOLTAGE_1V5 = 3      //!< 1.5V
} HW_PMU_1V4_VOLTAGE;


/**
 * \brief Maximum load current options for the 1V4 power rail
 *
 */
typedef enum {
        HW_PMU_1V4_MAX_LOAD_20 = 0      //!< 20mA
} HW_PMU_1V4_MAX_LOAD;


/**
 * \brief 1V4 power rail configuration
 *
 */
typedef struct {
        HW_PMU_1V4_VOLTAGE voltage;
        HW_PMU_1V4_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_1V4_RAIL_CONFIG;

/***************************************** V12 ***************************************************/

/**
 * \brief Voltage level options for the 1V2 power rail
 *
 */
typedef enum {
        /* active state */
        HW_PMU_1V2_VOLTAGE_0V75 = 0,            //!< 0.75V during active state
        HW_PMU_1V2_VOLTAGE_0V90 = 1,            //!< 0.90V during active state
        HW_PMU_1V2_VOLTAGE_1V20 = 2,            //!< 1.20V during active state
        /* sleep state
         * Only the two least significant bits of the enumerated value are used
         * for programming the rail.
         */
        HW_PMU_1V2_VOLTAGE_SLEEP_0V75 = 4,      //!< 0.75V during sleep state
        HW_PMU_1V2_VOLTAGE_SLEEP_0V90 = 5,      //!< 0.90V during sleep state
        HW_PMU_1V2_VOLTAGE_SLEEP_1V20 = 6,      //!< 1.20V during sleep state
        /* hibernation state */
        HW_PMU_1V2_VOLTAGE_HIBERNATION = 7,     //!< voltage level depending on V12 clamp trim setting
} HW_PMU_1V2_VOLTAGE;


/**
 * \brief Maximum load current options for the 1V2 power rail
 *
 */
typedef enum {
        HW_PMU_1V2_MAX_LOAD_1   = 0,    //!< 1mA
        HW_PMU_1V2_MAX_LOAD_150 = 1     //!< 150mA
} HW_PMU_1V2_MAX_LOAD;


/**
 * \brief 1V2 power rail configuration
 *
 */
typedef struct {
        HW_PMU_1V2_VOLTAGE voltage;
        HW_PMU_1V2_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_1V2_RAIL_CONFIG;


/***************************************** VLED **************************************************/
/**
 * \brief Set the voltage level of VLED rail
 *
 * This function sets the voltage level of the VLED rail, applicable both for wakeup / active
 * and sleep state.
 *
 * \param[in] voltage The voltage of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters \sa HW_PMU_VLED_VOLTAGE
 */
HW_PMU_ERROR_CODE hw_pmu_vled_set_voltage(HW_PMU_VLED_VOLTAGE voltage);

/**
 * \brief Set VLED rail wakeup / active configuration
 *
 * This function sets the VLED rail configuration during the wakeup / active state.
 * This is effective immediately.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 */
HW_PMU_ERROR_CODE hw_pmu_vled_onwakeup_enable(HW_PMU_VLED_MAX_LOAD max_load);

/**
 * \brief Disable VLED rail in wakeup / active state
 *
 * This function disables all the VLED power sources used during the wakeup / active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_vled_onwakeup_disable(void);

/**
 * \brief Set VLED rail sleep configuration
 *
 * This function sets the VLED rail configuration during the sleep state.
 * This is effective immediately.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 */
HW_PMU_ERROR_CODE hw_pmu_vled_onsleep_enable(HW_PMU_VLED_MAX_LOAD max_load);

/**
 * \brief Disable VLED rail in sleep state
 *
 * This function disables all the VLED power sources used during the sleep state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_vled_onsleep_disable(void);

/**
 * \brief Get the VLED rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VLED has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vled_active_config(HW_PMU_VLED_RAIL_CONFIG *rail_config);

/**
 * \brief Get the VLED rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VLED has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vled_onwakeup_config(HW_PMU_VLED_RAIL_CONFIG *rail_config);

/**
 * \brief Get the VLED rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VLED has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vled_onsleep_config(HW_PMU_VLED_RAIL_CONFIG *rail_config);

/***************************************** VSYS **************************************************/

/**
 * \brief Set the voltage level of VSYS rail
 *
 * This function sets the voltage level of the VSYS rail, applicable both for wakeup / active
 * and sleep state.
 *
 * \param[in] voltage The voltage of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters \sa HW_PMU_VSYS_VOLTAGE
 */
HW_PMU_ERROR_CODE hw_pmu_vsys_set_voltage(HW_PMU_VSYS_VOLTAGE voltage);

/**
 * \brief Get the VSYS rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VSYS has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vsys_active_config(HW_PMU_VSYS_RAIL_CONFIG *rail_config);

/**
 * \brief Get the VSYS rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VSYS has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vsys_onwakeup_config(HW_PMU_VSYS_RAIL_CONFIG *rail_config);

/**
 * \brief Get the VSYS rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VSYS has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vsys_onsleep_config(HW_PMU_VSYS_RAIL_CONFIG *rail_config);

/***************************************** V30 ***************************************************/

/**
 * \brief Set the voltage level of 3V0 rail
 *
 * This function sets the voltage level of the 3V0 rail during active / wakeup and sleep state.
 *
 * \param[in] voltage The voltage of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters \sa HW_PMU_3V0_VOLTAGE
 */
HW_PMU_ERROR_CODE hw_pmu_3v0_set_voltage(HW_PMU_3V0_VOLTAGE voltage);

/**
 * \brief Set 3V0 rail wakeup / active configuration
 *
 * This function sets the 3V0 rail configuration during the wakeup / active state.
 * This is effective immediately.
 * Depending on the input parameter, the appropriate power source will be selected:
 * - High current - Enable LDO_V30 disable other power sources.
 * - Low current  - Enable LDO_V30_RET disable other power sources.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                 Source
 *  HW_PMU_3V0_MAX_LOAD_150     LDO_V30
 *  HW_PMU_3V0_MAX_LOAD_10      LDO_V30_RET
 */
HW_PMU_ERROR_CODE hw_pmu_3v0_onwakeup_enable(HW_PMU_3V0_MAX_LOAD max_load);

/**
 * \brief Disable 3V0 rail in wakeup / active state
 *
 * This function disables all the 3V0 power sources used during the wakeup / active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_3v0_onwakeup_disable(void);

/**
 * \brief Enable 3V0 rail in sleep state
 *
 * This function enables the 3V0 power sources used during the sleep state.
 * The only power sources are LDO_V30 or LDO_V30_RET.
 *
 * If it is not available HW_PMU_ERROR_NOT_ENOUGH_POWER error code will be returned.
 *
 * param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                  Source
 *  HW_PMU_3V0_MAX_LOAD_150      LDO_V30
 *  HW_PMU_3V0_MAX_LOAD_10       LDO_V30_RET
 *  HW_PMU_3V0_MAX_LOAD_1        V30 Low Power Clamp
 *
 *  \note V30 Low Power Clamp selection is only for testing purposes.
 */
HW_PMU_ERROR_CODE hw_pmu_3v0_onsleep_enable(HW_PMU_3V0_MAX_LOAD max_load);

/**
 * \brief Disable 3V0 rail in sleep state
 *
 * This function disables all the 3V0 power sources used during the sleep state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_3v0_onsleep_disable(void);

/**
 * \brief Get the 3V0 rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 3V0 has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_3v0_active_config(HW_PMU_3V0_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 3V0 rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 3V0 has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_3v0_onwakeup_config(HW_PMU_3V0_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 3V0 rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 3V0 has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_3v0_onsleep_config(HW_PMU_3V0_RAIL_CONFIG *rail_config);

/***************************************** V18 ***************************************************/

/**
 * \brief Set the voltage level of 1V8 rail
 *
 * This function sets the voltage level of the 1V8 rail, applicable both for wakeup / active
 * and sleep state.
 *
 * \param[in] voltage The voltage of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters \sa HW_PMU_1V8_VOLTAGE
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_set_voltage(HW_PMU_1V8_VOLTAGE voltage);

/**
 * \brief Set 1V8 rail wakeup / active configuration
 *
 * This function sets the 1V8 rail configuration during the wakeup / active state.
 * This is effective immediately. The only power source is SIMO DCDC.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                   Source
 *  HW_PMU_1V8_MAX_LOAD_100       SIMO DCDC
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_enable(HW_PMU_1V8_MAX_LOAD max_load);

/**
 * \brief Disable 1V8 rail in wakeup / active state
 *
 * This function disables all the 1V8 power sources used during the wakeup / active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_disable(void);

/**
 * \brief Set 1V8 rail sleep configuration
 *
 * This function sets the 1V8 rail configuration during the sleep state.
 * This is effective immediately. The only power source is SIMO DCDC.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                 Source
 *  HW_PMU_1V8_MAX_LOAD_100     SIMO DCDC
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onsleep_enable(HW_PMU_1V8_MAX_LOAD max_load);

/**
 * \brief Disable 1V8 rail in sleep state
 *
 * This function disables all the 1V8 power sources used during the sleep state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onsleep_disable(void);

/**
 * \brief Get the 1V8 rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8 has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_active_config(HW_PMU_1V8_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V8 rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8 has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onwakeup_config(HW_PMU_1V8_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V8 rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8 has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onsleep_config(HW_PMU_1V8_RAIL_CONFIG *rail_config);

/***************************************** V18P **************************************************/

/**
 * \brief Set 1V8P rail wakeup / active configuration
 *
 * This function sets the 1V8P rail configuration during the wakeup / active state.
 * This is effective immediately.
 * The only power source is SIMO DCDC
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                   Source
 *  HW_PMU_1V8P_MAX_LOAD_100      SIMO DCDC
 */
HW_PMU_ERROR_CODE hw_pmu_1v8p_onwakeup_enable(HW_PMU_1V8P_MAX_LOAD max_load);

/**
 * \brief Disable 1V8P rail in wakeup / active state
 *
 * This function disables all the 1V8P power sources used during the wakeup / active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8p_onwakeup_disable(void);

/**
 * \brief Set 1V8P rail sleep configuration
 *
 * This function sets the 1V8P rail configuration during the sleep state.
 * This is effective immediately. The only power source is SIMO DCDC.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                 Source
 *  HW_PMU_1V8P_MAX_LOAD_100    SIMO DCDC
 */
HW_PMU_ERROR_CODE hw_pmu_1v8p_onsleep_enable(HW_PMU_1V8P_MAX_LOAD max_load);

/**
 * \brief Disable 1V8P rail in sleep state
 *
 * This function disables all the 1V8P power sources used during the sleep state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8p_onsleep_disable(void);

/**
 * \brief Get the 1V8P rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8P has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_active_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V8P rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8P has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_onwakeup_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V8P rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8P has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8p_onsleep_config(HW_PMU_1V8P_RAIL_CONFIG *rail_config);

/***************************************** V18F **************************************************/

/**
 * \brief Set 1V8F rail wakeup / active configuration
 *
 * This function sets the 1V8F rail configuration during the wakeup / active state.
 * This is effective immediately. The only power source is 1V8P.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                   Source
 *  HW_PMU_1V8F_MAX_LOAD_100      1V8P
 */
__RETAINED_CODE HW_PMU_ERROR_CODE hw_pmu_1v8f_onwakeup_enable(HW_PMU_1V8F_MAX_LOAD max_load);

/**
 * \brief Disable 1V8F rail in wakeup / active state
 *
 * This function disables all the 1V8F power sources used during the wakeup / active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
__RETAINED_CODE HW_PMU_ERROR_CODE hw_pmu_1v8f_onwakeup_disable(void);

/**
 * \brief Set 1V8F rail sleep configuration
 *
 * This function sets the 1V8F rail configuration during the sleep state.
 * This is effective immediately. The only power source is 1V8P.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                 Source
 *  HW_PMU_1V8F_MAX_LOAD_100    1V8P
 */
HW_PMU_ERROR_CODE hw_pmu_1v8f_onsleep_enable(HW_PMU_1V8F_MAX_LOAD max_load);

/**
 * \brief Disable 1V8F rail in sleep state
 *
 * This function disables all the 1V8F power sources used during the sleep state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
__RETAINED_CODE HW_PMU_ERROR_CODE hw_pmu_1v8f_onsleep_disable(void);

/**
 * \brief Get the 1V8F rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8F has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8f_active_config(HW_PMU_1V8F_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V8F rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8F has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8f_onwakeup_config(HW_PMU_1V8F_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V8F rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8F has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8f_onsleep_config(HW_PMU_1V8F_RAIL_CONFIG *rail_config);

/***************************************** V14 ***************************************************/

/**
 * \brief Set the voltage level of 1V4 rail
 *
 * This function sets the voltage level of the 1V4 rail, applicable both for wakeup / active and
 * sleep state.
 *
 * \param[in] voltage The voltage of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters \sa HW_PMU_1V4_VOLTAGE
 */
HW_PMU_ERROR_CODE hw_pmu_1v4_set_voltage(HW_PMU_1V4_VOLTAGE voltage);

/**
 * \brief Set 1V4 rail wakeup / active configuration
 *
 * This function sets the 1V4 rail configuration during the wakeup / active state.
 * This is effective immediately. The only power source is SIMO DCDC.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                 Source
 *  HW_PMU_1V4_MAX_LOAD_20      SIMO DCDC
 */
HW_PMU_ERROR_CODE hw_pmu_1v4_onwakeup_enable(HW_PMU_1V4_MAX_LOAD max_load);

/**
 * \brief Disable 1V4 rail in wakeup / active state
 *
 * This function disables all the 1V4 power sources used during the wakeup / active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v4_onwakeup_disable(void);

/**
 * \brief Set 1V4 rail sleep configuration
 *
 * This function sets the 1V4 rail configuration during the sleep state.
 * This is effective immediately. The only power source is SIMO DCDC.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                 Source
 *  HW_PMU_1V4_MAX_LOAD_20      SIMO DCDC
 */
HW_PMU_ERROR_CODE hw_pmu_1v4_onsleep_enable(HW_PMU_1V4_MAX_LOAD max_load);

/**
 * \brief Disable 1V4 rail in sleep state
 *
 * This function disables all the 1V4 power sources used during the sleep state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v4_onsleep_disable(void);

/**
 * \brief Get the 1V4 rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V4 has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v4_active_config(HW_PMU_1V4_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V4 rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V4 has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v4_onwakeup_config(HW_PMU_1V4_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V4 rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V4 has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v4_onsleep_config(HW_PMU_1V4_RAIL_CONFIG *rail_config);

/***************************************** V12 ***************************************************/

/**
 * \brief Set the voltage level of 1V2 rail
 *
 * This function sets the voltage level of the 1V2 rail during active / wakeup and sleep state.
 *
 * \param[in] voltage The voltage of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters \sa HW_PMU_1V2_VOLTAGE (except HW_PMU_1V2_VOLTAGE_HIBERNATION)
 */
__RETAINED_HOT_CODE HW_PMU_ERROR_CODE hw_pmu_1v2_set_voltage(HW_PMU_1V2_VOLTAGE voltage);

/**
 * \brief Set 1V2 rail wakeup / active configuration
 *
 * This function sets the 1V2 rail configuration during the wakeup / active state.
 * This is effective immediately. The only power source is SIMO DCDC.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                 Source
 *  HW_PMU_1V2_MAX_LOAD_150     SIMO DCDC
 *
 */
HW_PMU_ERROR_CODE hw_pmu_1v2_onwakeup_enable(HW_PMU_1V2_MAX_LOAD max_load);

/**
 * \brief Disable 1V2 rail in wakeup / active state
 *
 * This function disables all the 1V2 power sources used during the wakeup / active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v2_onwakeup_disable(void);

/**
 * \brief Set 1V2 rail sleep configuration
 *
 * This function sets the 1V2 rail configuration during the sleep state.
 * This is effective immediately. The only power source is SIMO DCDC.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                 Source
 *  HW_PMU_1V2_MAX_LOAD_150     SIMO DCDC
 *  HW_PMU_1V2_MAX_LOAD_1       V12 Low Power Clamp
 *
 *  \note V12 Low Power Clamp selection is only for testing purposes.
 */
HW_PMU_ERROR_CODE hw_pmu_1v2_onsleep_enable(HW_PMU_1V2_MAX_LOAD max_load);

/**
 * \brief Disable 1V2 rail in sleep state
 *
 * This function disables all the 1V2 power sources used during the sleep state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v2_onsleep_disable(void);

/**
 * \brief Get the 1V2 rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V2 has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
__RETAINED_HOT_CODE HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_active_config(HW_PMU_1V2_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V2 rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V2 has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_onwakeup_config(HW_PMU_1V2_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V2 rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V2 has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
__RETAINED_HOT_CODE HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v2_onsleep_config(HW_PMU_1V2_RAIL_CONFIG *rail_config);


/**
 * \brief Populate the trim-values of the rails
 *
 * \return Returns HW_PMU_ERROR_NOERROR on success or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_store_trim_values(void);

#endif /* HW_PMU_DA1470x_H_ */
#endif /* dg_configUSE_HW_PMU */

/**
\}
\}
*/
