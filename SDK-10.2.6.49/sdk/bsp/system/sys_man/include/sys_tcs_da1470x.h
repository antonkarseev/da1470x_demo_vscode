/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_TCS_HANDLER TCS Handler
 * \brief TCS Handler
 * \{
 */

/**
****************************************************************************************
*
* @file sys_tcs_da1470x.h
*
* @brief TCS Handler header file.
*
* Copyright (C) 2020-2021 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef SYS_TCS_DA1470X_H_
#define SYS_TCS_DA1470X_H_

#include "sdk_defs.h"

#define TCS_DATA_SIZE           256   /**< max number of entries in words (4 bytes). */
#define GID_EMPTY               0x1FF

/**
 * \enum SYS_TCS_TYPE
 * \brief custom TCS value type.
 *
 */
typedef enum {
        SYS_TCS_TYPE_TRIM_VAL = 0,      /**< trimmed value */
        SYS_TCS_TYPE_REG_PAIR = 1,      /**< register pair value */
} SYS_TCS_TYPE;

/**
 * \brief the configured group ids.
 *
 */
typedef enum {

        /* Power Domains Section */
        SYS_TCS_GROUP_PD_SYS = 0x01,                    /**< PD_SYS group id */
        SYS_TCS_GROUP_PD_SNC = 0x02,                    /**< PD_SNC group id */
        SYS_TCS_GROUP_PD_MEM = 0x03,                    /**< PD_MEM group id */
        SYS_TCS_GROUP_PD_TMR = 0x04,                    /**< PD_TMR group id */
        SYS_TCS_GROUP_PD_AUDIO = 0x05,                  /**< PD_AUDIO group id */
        SYS_TCS_GROUP_PD_RAD = 0x06,                    /**< PD_RAD group id */
        SYS_TCS_GROUP_PD_SYNTH = 0x07,                  /**< PD_SYNTH group id */
        SYS_TCS_GROUP_PD_GPU = 0x08,                    /**< PD_GPU group id */
        SYS_TCS_GROUP_PD_CTRL = 0x09,                   /**< PD_CTRL group id */

        /* System Section */
        SYS_TCS_GROUP_BD_ADDR = 0x20,                   /**< BD_ADDR group id */

        /* Analog Section */
        SYS_TCS_GROUP_SD_ADC_SINGLE_MODE = 0x40,        /**< SD_ADC_SINGLE group id */
        SYS_TCS_GROUP_SD_ADC_DIFF_MODE = 0x41,          /**< SD_ADC_DIFF group id */
        SYS_TCS_GROUP_GP_ADC_SINGLE_MODE = 0x42,        /**< GP_ADC_SINGLE group id */
        SYS_TCS_GROUP_GP_ADC_DIFF_MODE = 0x43,          /**< GP_ADC_DIFF group id */
        SYS_TCS_GROUP_TEMP_SENS_25C = 0x44,             /**< TEMP_SENS_25C group id */
        SYS_TCS_GROUP_TEMP_SENS_RD_BG_CH_25C = 0x45,    /**< TEMP_SENS_RD_BG_CH_25C group id */
        SYS_TCS_GROUP_BUCK_TRIM = 0x46,                 /**< BUCK_TRIM group id */
        SYS_TCS_GROUP_RCHS_64MHZ = 0x47,                /**< RCHS_64MHZ group id */
        SYS_TCS_GROUP_RCLP_32KHZ = 0x48,                /**< RCLP_32KHZ group id */

        /* Radio Section */
        SYS_TCS_GROUP_PD_RAD_MODE1 = 0x80,              /**< PD_RAD_MODE1 group id */
        SYS_TCS_GROUP_PD_SYNTH_MODE1 = 0x81,            /**< PD_SYNTH_MODE1 group id */
        SYS_TCS_GROUP_PD_RAD_MODE2 = 0x82,              /**< PD_RAD_MODE2 group id */
        SYS_TCS_GROUP_PD_SYNTH_MODE2 = 0x83,            /**< PD_SYNTH_MODE2 group id */
        SYS_TCS_GROUP_PD_RAD_COEFF = 0x84,              /**< PD_RAD_COEFF group id */

        /* Production Test Section */
        SYS_TCS_GROUP_CHIP_ID = 0xC0,                   /**< CHIP_ID group id */
        SYS_TCS_GROUP_PROD_INFO = 0xC1,                 /**< PROD_INFO group id */
        SYS_TCS_GROUP_PROD_WAFER = 0xC2,                /**< PROD_WAFER group id */
        SYS_TCS_GROUP_TESTPROGRAM_VERSION = 0xC3,       /**< TESTPROGRAM_VERSION  group id */

        SYS_TCS_GROUP_MAX = 0xE0                        /**< Maximum supported group id */
} SYS_TCS_GID;

/**
 * \struct sys_tcs_attr_t
 * \brief attributes per custom value group id
 *
 */
typedef struct {
        uint16_t value_type : 1;        /**< TCS entry type */
        uint16_t start : 9;             /**< TCS entry start position  */
        uint16_t size : 6;              /**< TCS entry type size in words */
} sys_tcs_attr_t;

/*
 * Reset values of trimmed registers
 */
#define DEFAULT_CHARGER_TEST_CTRL_REG   0x00001F28

/**
 * \brief TCS custom trim values callback
 *
 * \param [in] values_group the TCS group id custom trim values belong to
 * \param [in] user_data user specific data
 * \param [in] values custom trim values
 * \param [in] size the number of the custom trim values
 *
 */
typedef void (*sys_tcs_custom_values_cb)( SYS_TCS_GID values_group , void *user_data, uint32_t *values, uint8_t size);

/**
 * \brief retrieve the TCS values from CS located in OTP or flash and then store
 * TCS register pair address, value  and/or custom value custom_trim_value in the Global TCS array
 *
 */
void sys_tcs_get_trim_values_from_cs(void);

/**
 * \brief get the number of entries stored in tcs_data
 *
 * \return the number of stored TCS entries. Each entry is 32bit
 */
uint32_t sys_tcs_get_tcs_data_size(void);

/**
 * \brief check if the register addresses included in reg_address are configured in CS
 * \param [in] reg_address pointer to array containing the register addresses
 * \param [in] num number of register addresses
 * \param [out] trimmed_reg pointer to array containing information whether the corresponding
 * register is included in CS or not
 *
 * \return true if all register addresses are included in CS
 *
 * \warning trimmed_reg should be initialized to false otherwise will report false positive
 * that register address found.
 */
bool sys_tcs_reg_pairs_in_cs(const uint32_t* reg_address, uint8_t num, bool *trimmed_reg);

#endif /* SYS_TCS_DA1470X_H_ */
/**
\}
\}
*/
