/**
 ****************************************************************************************
 *
 * @file hw_sys_internal.h
 *
 * @brief System Driver Internal header file.
 *
 * This file contains system related administration definitions that enable detecting and applying
 * the identification of the target SDK board where the SDK firmware executes on.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_SYS_INTERNAL_H_
#define HW_SYS_INTERNAL_H_


/**
 * \brief       Populate hw_sys_device_info_data with device information retrieved from the
 *              corresponding chip registers (Family, Device Chip ID, Revision and Step)
 *
 * \return      True, if the retrieved information is valid, otherwise false.
 *
 * \warning     The device variant information is not populated by this function, the
 *              hw_sys_dev_variant_init() must be used instead.
 */
bool hw_sys_device_info_init(void);

/**
 * \brief       Populate hw_sys_device_info_data with the device variant information retrieved from the
 *              corresponding TCS entry.
 *
 * \return      True, if the retrieved information is valid, otherwise false.
 */
bool hw_sys_device_variant_init(void);

/**
 * \brief       Check if a specific device information aspect matches the one of the target device.
 *
 * Use this function to check if a device information attribute equals to specific value using as input
 * arguments the public macros (no prefixed with underscore) defined in "bsp_device_definitions_internal.h".
 *
 * \param[in]   mask The device information attribute mask to be checked.
 * \param[in]   attribute The device information attribute value to compare with.
 *
 * \return      The result of comparison.
 *
 * Examples:
 *
 * \code
 * bool check;
 *
 * check = hw_sys_device_info_check(DEVICE_FAMILY_MASK, DA1468X);
 * check = hw_sys_device_info_check(DEVICE_FAMILY_MASK, DA1469X);
 *
 * check = hw_sys_device_info_check(DEVICE_VARIANT_MASK, DA14695);
 * check = hw_sys_device_info_check(DEVICE_VARIANT_MASK, DA14699);
 *
 * check = hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_2522);
 * check = hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_3080);
 *
 * check = hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_A);
 * check = hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_B);
 *
 * check = hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_0);
 * check = hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_1);
 *
 * check = hw_sys_device_info_check(DEVICE_STEP_MASK, DEVICE_STEP_A);
 * check = hw_sys_device_info_check(DEVICE_STEP_MASK, DEVICE_STEP_B);
 * check = hw_sys_device_info_check(DEVICE_STEP_MASK, DEVICE_STEP_C);
 * \endcode
 */
bool hw_sys_device_info_check(uint32_t mask, uint32_t attribute);

/**
 * \brief       Get the hw_sys_device_info_data where all device information attributes are
 *              populated.
 *
 * \return      hw_sys_device_info_data
 */
uint32_t hw_sys_get_device_info(void);

/**
 * \brief       Check that the firmware and the chip that it runs on are compatible with each other.
 *
 * \return      True, if the chip version is compliant, otherwise false.
 */
bool hw_sys_is_compatible_chip(void);


#endif /* HW_SYS_INTERNAL_H_ */

/**
 * \}
 * \}
 */
