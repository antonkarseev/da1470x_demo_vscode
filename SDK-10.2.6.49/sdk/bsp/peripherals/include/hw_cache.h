/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_CACHE iCache Controller
 * \{
 * \brief iCache Controller LLD common API
 */

/**
 *****************************************************************************************
 *
 * @file hw_cache.h
 *
 * @brief Definition of API for the iCache Controller Low Level Driver.
 *
 * Copyright (C) 2015-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_CACHE_H_
#define HW_CACHE_H_

#if dg_configUSE_HW_CACHE

#include <sdk_defs.h>

        #include "hw_cache_da1470x.h"

/*
 * COMMON API MRM RELATED FUNCTIONALITY DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Set the cache MRM interrupt threshold for misses
 *
 * Defines the threshold (in misses) to trigger the interrupt generation.
 * A value of 0 disables interrupt generation
 *
 * \param [in] thres The interrupt generation threshold (in misses)
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_misses_thres(uint32_t thres)
{
        CACHE->CACHE_MRM_MISSES_THRES_REG = thres;
}

/**
 * \brief Get the cache MRM interrupt threshold for misses
 *
 * \return The interrupt generation threshold (in misses)
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_misses_thres(void)
{
        return CACHE->CACHE_MRM_MISSES_THRES_REG;
}

/**
 * \brief Set the cache MRM interrupt threshold for hits
 *
 * Defines the threshold (in hits) to trigger the interrupt generation.
 * A value of 0 disables interrupt generation
 *
 * \param [in] thres The interrupt generation threshold (in hits)
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_hits_thres(uint32_t thres)
{
        CACHE->CACHE_MRM_HITS_THRES_REG = thres;
}

/**
 * \brief Get the cache MRM interrupt threshold for hits
 *
 * \return The interrupt generation threshold (in hits)
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_hits_thres(void)
{
        return CACHE->CACHE_MRM_HITS_THRES_REG;
}

/**
 * \brief Get the cache MRM misses threshold IRQ status
 *
 * \return True if an interrupt has been generated because the number
 *         of misses reached the programmed threshold (if !=0)
 */
__STATIC_INLINE bool hw_cache_mrm_get_misses_thres_status(void)
{
        return REG_GETF(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_MISSES_THRES_STATUS);
}

/**
 * \brief Clear the cache MRM misses threshold IRQ status
 *
 *
 */
__STATIC_INLINE void hw_cache_mrm_clr_misses_thres_status(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_MISSES_THRES_STATUS);
}

/**
 * \brief Get the cache MRM hits threshold IRQ status
 *
 * \return True if an interrupt has been generated because the number
 *         of hits reached the programmed threshold (if !=0)
 */
__STATIC_INLINE bool hw_cache_mrm_get_hits_thres_status(void)
{
        return REG_GETF(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_HITS_THRES_STATUS);
}

/**
 * \brief Clear the cache MRM hits threshold IRQ status
 *
 *
 */
__STATIC_INLINE void hw_cache_mrm_clr_hits_thres_status(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_HITS_THRES_STATUS);
}

/**
 * \brief Set the cache MRM monitoring time interval
 *
 * Defines the time interval for the monitoring in 32 MHz clock cycles.
 * Must be an 19-bit value max. When this time is reached, an interrupt
 * will be generated.
 * A value of 0 disables interrupt generation
 *
 * \param [in] tint Monitoring time interval in clock cycles
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_tint(uint32_t tint)
{
        ASSERT_WARNING((tint & ~CACHE_CACHE_MRM_TINT_REG_MRM_TINT_Msk) == 0);
                CACHE->CACHE_MRM_TINT_REG = tint;
}

/**
 * \brief Get the cache MRM monitoring time interval
 *
 * \return The monitoring time interval in clock cycles
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_tint(void)
{
        return CACHE->CACHE_MRM_TINT_REG & CACHE_CACHE_MRM_TINT_REG_MRM_TINT_Msk;
}

/**
 * \brief Get the cache MRM timer interval IRQ status
 *
 * \return True if an interrupt has been generated because the time
 *         interval counter reached the end (time interval != 0).
 *
 */
__STATIC_INLINE bool hw_cache_mrm_get_tint_status(void)
{
        return REG_GETF(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_TINT_STATUS);
}

/**
 * \brief Clear the cache MRM timer interval IRQ status
 *
 */
__STATIC_INLINE void hw_cache_mrm_clr_tint_status(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_TINT_STATUS);
}

/**
 * \brief Start MRM counters
 *
 * \note If Timer interval is not set to 0 using
 *       hw_cache_mrm_set_tint, the timer interval will count down to 0.
 *       When zero is reached, an interrupt will be generated, and the
 *       counters will be disabled automatically.
 */
__STATIC_INLINE void hw_cache_mrm_start_counters(void)
{
        REG_SET_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_START);
}

/**
 * \brief Freeze MRM counters
 *
 */
__STATIC_INLINE void hw_cache_mrm_freeze_counters(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_START);
}

/**
 * \brief Get the cache MRM misses number
 *
 * \return The number of cache misses
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_misses(void)
{
        return CACHE->CACHE_MRM_MISSES_REG;
}

/**
 * \brief Set the cache MRM cache misses number
 *
 * This is primarily intended for clearing the misses number
 *
 * \param[in] misses The number of cache misses
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_misses(uint32_t misses)
{
        CACHE->CACHE_MRM_MISSES_REG = misses;
}

/**
 * \brief Get the cache MRM cache hits number
 *
 * \return The number of cache hits
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_hits(void)
{
        return CACHE->CACHE_MRM_HITS_REG;
}

/**
 * \brief Set the cache MRM cache hits number
 *
 * This is primarily intended for clearing the hits number
 *
 * \param[in] hits The number of cache hits
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_hits(uint32_t hits)
{
        CACHE->CACHE_MRM_HITS_REG = hits;
}

/**
 * \brief Application defined callback for the MRM interrupt.
 *
 * \note The application defined callback should be declared as  __RETAINED_CODE.
 *
 */
typedef void (*hw_cache_mrm_cb_t)(void);

/**
 * \brief Enable the MRM interrupt generation
 *
 * The application should define its own callback. The latter is registered
 * and then invoked when the MRM interrupt is generated.
 *
 * \param [in] cb Callback defined by the application.
 *
 */
void hw_cache_mrm_enable_interrupt(hw_cache_mrm_cb_t cb);

/**
 * \brief Disable the MRM interrupt generation
 *
 * \note The application defined called is unregistered.
 *
 */
void hw_cache_mrm_disable_interrupt(void);

#endif /* dg_configUSE_HW_CACHE */

#endif /* HW_CACHE_H_ */

/**
 * \}
 * \}
 */
