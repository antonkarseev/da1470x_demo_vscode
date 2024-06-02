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
* @file sys_tcs.h
*
* @brief TCS Handler header file.
*
* Copyright (C) 2015-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef SYS_TCS_H___
#define SYS_TCS_H___

        #include "sys_tcs_da1470x.h"


/**
 * \brief retrieve the TCS values from a non_volatile memory. This memory depends on device family.
 * TCS register pair address, value  and/or custom value custom_trim_value in the Global TCS array
 *
 */
void sys_tcs_get_trim_values_from_cs(void);

/**
 * \brief get the pointer to tcs_attributes lookup table
 *
 */
__RETAINED_CODE sys_tcs_attr_t* sys_tcs_get_tcs_attributes_ptr(void);

/**
 * \brief get the pointer to tcs_data memory block
 *
 * \ warning if dg_configUSE_SYS_TCS = 0 it will return NULL
 */
__RETAINED_CODE uint32_t* sys_tcs_get_tcs_data_ptr(void);

/**
 * \brief get the number of the register pair values or custom values per gid
 *
 * \param [in] gid the TCS group id
 *
 * \return the number of trim values per group id
 */
uint8_t sys_tcs_get_size(SYS_TCS_GID gid);

/**
 * \brief get value type, register pair values or custom values per gid
 *
 * \param [in] gid the TCS group id
 *
 * \return the type of trim value per group id
 */
SYS_TCS_TYPE sys_tcs_get_value_type(SYS_TCS_GID gid);

/**
 * \brief get the custom_trim_values per gid
 *
 * \param [in] gid the TCS group id of the requested custom trim values
 * \param [out] values the pointer to the start of the custom trim values
 * \param [out] size the number of the custom trim values
 *
 * \warning if size is zero then there are no custom trim values for this gid,
 *  values points to invalid data. size should have been initialized to zero
 */
void sys_tcs_get_custom_values(SYS_TCS_GID gid, uint32_t **values, uint8_t *size);

/**
 * \brief handles the custom_trim_values per gid according to callback
 *
 * \param [in] gid the TCS group id of custom trim values to apply
 * \param [in] cb the callback that applies the custom trim values
 * \param [in] user_data the argument to callback function
 *
 * \warning callback is called only if custom trim values are configured
 */
void sys_tcs_apply_custom_values(SYS_TCS_GID gid, sys_tcs_custom_values_cb cb, void *user_data);

/**
 * \brief Get register value pairs contained in a group id of the TCS array
 * \param [in] gid the group id
 * \param [out] values the pointer to the start of the register pair values
 * \param [out] size the number of the register pair values
 *
 * \warning if size is zero then values is not a valid pointer. size should have been
 * initialized to zero
 */
void sys_tcs_get_reg_pairs(SYS_TCS_GID gid, uint32_t **values, uint8_t *size);

/**
 * \brief Apply the register value pairs contained in a group id of the TCS array.
 * \param [in] gid the group id
 *
 */
__RETAINED_HOT_CODE void sys_tcs_apply_reg_pairs(SYS_TCS_GID gid);

/**
 * \brief Get register value pairs contained in a group id of the TCS array
 * \param [in] gid the group id
 *
 * \return the pointer to the array containing these values or this group id
 */
uint32_t *sys_tcs_snc_get_reg_pair(SYS_TCS_GID gid);

/**
 * \brief Get the number of register-value pairs for this specific gid
 * \param [in] gid the group id
 *
 * \return the number of register-value entries of a given gid
 */
uint32_t sys_tcs_snc_get_reg_pair_num_of_entries(SYS_TCS_GID gid);

/**
 * \brief handles the custom_trim_values per gid according to callback
 *
 * \param [in] gid the TCS group id of custom trim values to apply
 * \param [in] user_data the argument to callback function
 * \param [in] val pointer to the returned values
 * \param [in] len size of returned values (bytes)
 *
 * \warning callback is called only if custom trim values are configured
 */
void sys_tcs_custom_values_system_cb(SYS_TCS_GID gid, void *user_data, uint32_t *val, uint8_t len);

#endif /* SYS_TCS_H___ */
/**
\}
\}
*/
