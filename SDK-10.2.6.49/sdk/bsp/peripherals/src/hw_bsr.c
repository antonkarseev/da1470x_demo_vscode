/**
 ****************************************************************************************
 *
 * @file hw_bsr.c
 *
 * @brief Implementation for the BSR Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "sdk_defs.h"
#include "hw_bsr.h"

#define HW_BSR_REG_LIMIT                (31) // HW BSR has 2 registers if BSR peripheral_id > 31 then it is addressed in second register
#define BSR_GET_POS(_pos)               (uint8_t)((_pos > HW_BSR_REG_LIMIT) ? (_pos - (HW_BSR_REG_LIMIT + 1)) : (_pos))
#define BSR_BUSY_STAT_REG(_pos)         *((_pos < HW_BSR_REG_LIMIT) ? \
                                        ((volatile uint32_t *)(MEMCTRL_BASE + offsetof(MEMCTRL_Type, BUSY_STAT_REG))) : \
                                        ((volatile uint32_t *)(MEMCTRL_BASE + offsetof(MEMCTRL_Type, BUSY_STAT_REG)) + 1))
#define BSR_BUSY_SET_REG(_pos)          *((_pos < HW_BSR_REG_LIMIT) ? \
                                        ((volatile uint32_t *)(MEMCTRL_BASE + offsetof(MEMCTRL_Type, BUSY_SET_REG))) : \
                                        ((volatile uint32_t *)(MEMCTRL_BASE + offsetof(MEMCTRL_Type, BUSY_SET_REG)) + 1))
#define BSR_BUSY_RESET_REG(_pos)        *((_pos < HW_BSR_REG_LIMIT) ? \
                                        ((volatile uint32_t *)(MEMCTRL_BASE + offsetof(MEMCTRL_Type, BUSY_RESET_REG))) : \
                                        ((volatile uint32_t *)(MEMCTRL_BASE + offsetof(MEMCTRL_Type, BUSY_RESET_REG)) + 1))

void hw_bsr_init(void)
{
        MEMCTRL->BUSY_RESET_REG = MEMCTRL->BUSY_STAT_REG;
        MEMCTRL->BUSY_RESET_REG2 = MEMCTRL->BUSY_STAT_REG2;
}

__RETAINED_HOT_CODE bool hw_bsr_try_lock(HW_BSR_MASTER_ID bsr_master_id, HW_BSR_PERIPH_ID per_id)
{
        uint8_t pos = BSR_GET_POS(per_id);
        ASSERT_ERROR((bsr_master_id & HW_BSR_MASTER_MASK) == bsr_master_id);
        ASSERT_WARNING((per_id % 2) == 0);
        ASSERT_WARNING(per_id < HW_BSR_PERIPH_ID_MAX);

        BSR_BUSY_SET_REG(per_id) = bsr_master_id << pos;
        if (((BSR_BUSY_STAT_REG(per_id) >> pos) & HW_BSR_MASTER_MASK) == bsr_master_id) {
                /* Update SW BSR counter in case HW BSR is used */
                return true;
        } else {
                return false;
        }
}

__RETAINED_HOT_CODE void hw_bsr_unlock(HW_BSR_MASTER_ID bsr_master_id, HW_BSR_PERIPH_ID per_id)
{
        uint8_t pos = BSR_GET_POS(per_id);
        ASSERT_ERROR((bsr_master_id & HW_BSR_MASTER_MASK) == bsr_master_id);

        ASSERT_WARNING((per_id % 2) == 0);
        ASSERT_WARNING(per_id < HW_BSR_PERIPH_ID_MAX);

        ASSERT_ERROR(((BSR_BUSY_STAT_REG(per_id) >> (pos)) & HW_BSR_MASTER_MASK) == bsr_master_id);
        BSR_BUSY_RESET_REG(per_id) = bsr_master_id << pos;
}

bool hw_bsr_is_locked(HW_BSR_MASTER_ID bsr_master_id, HW_BSR_PERIPH_ID per_id)
{
        uint8_t pos = BSR_GET_POS(per_id);
        ASSERT_ERROR((bsr_master_id & HW_BSR_MASTER_MASK) == bsr_master_id);

        ASSERT_WARNING((per_id % 2) == 0);
        ASSERT_WARNING(per_id < HW_BSR_PERIPH_ID_MAX);

        return (((BSR_BUSY_STAT_REG(per_id) >> (pos)) & HW_BSR_MASTER_MASK) == bsr_master_id);
}

