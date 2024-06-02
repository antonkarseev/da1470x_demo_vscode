/**
 ****************************************************************************************
 *
 * @file hw_sys_regs.c
 *
 * @brief Implementation for system registers, including Register Configuration
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "hw_clk.h"
#include "hw_gpio.h"
#include "hw_pd.h"
#include "hw_sys_regs.h"

#define NUM_OF_REG_CONFIG_ENTRIES       5

__RETAINED static hw_sys_reg_config_t hw_sys_reg_config[NUM_OF_REG_CONFIG_ENTRIES];
__RETAINED static uint32_t hw_sys_reg_num_of_config_entries;


uint32_t hw_sys_reg_add_config(const hw_sys_reg_config_t *config, uint32_t num_of_entries)
{
       ASSERT_ERROR(hw_sys_reg_num_of_config_entries + num_of_entries <= NUM_OF_REG_CONFIG_ENTRIES);

       uint32_t ret = hw_sys_reg_num_of_config_entries;

       for (uint32_t i = 0; i < num_of_entries; i++) {
               hw_sys_reg_config[hw_sys_reg_num_of_config_entries + i] = config[i];
       }
       hw_sys_reg_num_of_config_entries += num_of_entries;

       return ret;
}

hw_sys_reg_config_t *hw_sys_reg_get_config(uint32_t index)
{
        ASSERT_WARNING(index == 0 || index < hw_sys_reg_num_of_config_entries);

        return &hw_sys_reg_config[index];
}

void hw_sys_reg_modify_config(uint32_t index, __IO uint32_t *addr, uint32_t value)
{
        ASSERT_ERROR(index < hw_sys_reg_num_of_config_entries);

        hw_sys_reg_config[index].value = value;

        // The address must be written after the value to prevent race condition with other hosts
        hw_sys_reg_config[index].addr = addr;
}

uint32_t *hw_sys_reg_get_num_of_config_entries(void)
{
        return &hw_sys_reg_num_of_config_entries;
}

__RETAINED_CODE void hw_sys_reg_apply_config(void)
{
        uint32_t *p = (uint32_t *)&hw_sys_reg_config;

        while (p < (uint32_t *)&hw_sys_reg_config + 2 * hw_sys_reg_num_of_config_entries) {
                *(uint32_t *)(*p) = *(p+1);
                p += 2;
        }
}

