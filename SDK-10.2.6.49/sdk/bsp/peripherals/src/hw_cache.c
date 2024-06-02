/**
 *****************************************************************************************
 *
 * @file hw_cache.c
 *
 * @brief Implementation of the iCache Controller Low Level Driver.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */


#if dg_configUSE_HW_CACHE

#include "hw_cache.h"

__RETAINED static hw_cache_mrm_cb_t hw_cache_mrm_cb;

void hw_cache_mrm_enable_interrupt(hw_cache_mrm_cb_t cb)
{
        ASSERT_WARNING(cb);
        hw_cache_mrm_cb = cb;
        REG_SET_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_MASK);
#if MAIN_PROCESSOR_BUILD
        NVIC_ClearPendingIRQ(M33_Cache_MRM_IRQn);
        NVIC_EnableIRQ(M33_Cache_MRM_IRQn);
#endif
}

void hw_cache_mrm_disable_interrupt(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_MASK);
#if MAIN_PROCESSOR_BUILD
        NVIC_DisableIRQ(M33_Cache_MRM_IRQn);
        NVIC_ClearPendingIRQ(M33_Cache_MRM_IRQn);
#endif
        hw_cache_mrm_cb = NULL;
}

__RETAINED_CODE void MRM_Handler(void)
{
        if (hw_cache_mrm_cb) {
                hw_cache_mrm_cb();
        }
}

#endif /* dg_configUSE_HW_CACHE */

