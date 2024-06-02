/**
\addtogroup PLA_DRI_PER_OTHER
\{
\addtogroup HW_SYS_REGS System Registers API
\{
\brief System Driver
*/

/**
 ****************************************************************************************
 *
 * @file hw_sys_regs.h
 *
 * @brief System Registers API.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_SYS_REGS_H_
#define HW_SYS_REGS_H_


#include "sdk_defs.h"

/**
 * \brief Register configuration
 *
 */
typedef struct {
        __IO uint32_t *addr;    //!< Register address
        uint32_t value;         //!< Register value
} hw_sys_reg_config_t;

/**
 * \brief Add register configuration entries in the system register configuration table
 *
 * \param [in] config pointer to the structure containing the register configuration
 * \param [in] num_of_entries the number of entries in the register configuration structure
 *
 * \return the index of the first entry in the configuration table
 */
uint32_t hw_sys_reg_add_config(const hw_sys_reg_config_t *config, uint32_t num_of_entries);

/**
 * \brief Get a register configuration entry
 *
 * \param [in] index the index of the entry in the register configuration table
 *
 * \return a pointer to the register configuration entry
 */
hw_sys_reg_config_t *hw_sys_reg_get_config(uint32_t index);

/**
 * \brief Modify a register configuration entry
 *
 * \param [in] index the index of the entry in the register configuration table
 * \param [in] addr the new register address
 * \param [in] value the new register value
 */
void hw_sys_reg_modify_config(uint32_t index, __IO uint32_t *addr, uint32_t value);

/**
 * \brief Get the number of entries in the system register configuration table
 *
 * \return a pointer to the number of entries
 */
uint32_t *hw_sys_reg_get_num_of_config_entries(void);

/**
 * \brief Apply system register configuration
 *
 * Configure non-retained system registers using the entries in the system register
 * configuration table.
 */
__RETAINED_CODE void hw_sys_reg_apply_config(void);


#endif /* HW_SYS_REGS_H_ */

/**
 * \}
 * \}
 */
