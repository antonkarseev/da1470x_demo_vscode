/**
 *****************************************************************************************
 *
 * @file hw_dcache.c
 *
 * @brief Implementation of the dCache Controller Low Level Driver.
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#if dg_configUSE_HW_DCACHE

#include "hw_dcache.h"

__RETAINED static hw_dcache_mrm_cb_t hw_dcache_mrm_cb;

void hw_dcache_mrm_enable_interrupt(hw_dcache_mrm_cb_t cb)
{
        ASSERT_WARNING(cb);
        hw_dcache_mrm_cb = cb;
        REG_SET_BIT(DCACHE, DCACHE_MRM_CTRL_REG, MRM_IRQ_MASK);
        NVIC_ClearPendingIRQ(DCACHE_MRM_IRQn);
        NVIC_EnableIRQ(DCACHE_MRM_IRQn);
}

void hw_dcache_mrm_disable_interrupt(void)
{
        REG_CLR_BIT(DCACHE, DCACHE_MRM_CTRL_REG, MRM_IRQ_MASK);
        NVIC_DisableIRQ(DCACHE_MRM_IRQn);
        NVIC_ClearPendingIRQ(DCACHE_MRM_IRQn);
        hw_dcache_mrm_cb = NULL;
}

__RETAINED_CODE void DCACHE_MRM_Handler(void)
{
        if (hw_dcache_mrm_cb) {
                hw_dcache_mrm_cb();
        }
}

#endif /* dg_configUSE_HW_DCACHE */
