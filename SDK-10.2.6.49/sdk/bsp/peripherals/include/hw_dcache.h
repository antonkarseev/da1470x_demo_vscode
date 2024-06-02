/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_DCACHE dCache Controller
 * \{
 * \brief dCache Controller LLD API
 */

/**
 *****************************************************************************************
 *
 * @file hw_dcache.h
 *
 * @brief Definition of API for the dCache Controller Low Level Driver.
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_DCACHE_H_
#define HW_DCACHE_H_

#if dg_configUSE_HW_DCACHE

#include <sdk_defs.h>

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief dCache Controller disable modes
 *
 */
typedef enum {
        HW_DCACHE_DISABLE_NORMAL        =  0,
        HW_DCACHE_DISABLE_POWERINGDOWN  =  1,
        HW_DCACHE_DISABLE_DEBUG         =  2,
        HW_DCACHE_DISABLE_INVALID,
} HW_DCACHE_DISABLE_MODE;

/*
 *  GLOBAL/CONSTANT VARIABLES DEFINITIONS
 *****************************************************************************************
 */

/**
 *  \brief Maximum cacheable area length definition
 *
 *   With a 1KB cacheable resolution (i.e. the target memory is addressed in blocks of 1KB),
 *   and a size of 131072KB of the target memory (MEMORY_QSPIC2_SIZE) the maximum cacheable length
 *   is 131072 = 131072KB / 1KB. In hex: 0x8000000 / 0x400 = 0x20000.
 */
#define HW_DCACHE_CACHEABLE_RESOLUTION     0x400                /* Blocks of 1KB in size */
#define HW_DCACHE_CACHEABLE_LEN_MAX        (MEMORY_QSPIC2_SIZE / HW_DCACHE_CACHEABLE_RESOLUTION)

/**
 * \brief Default cache line size
 *
 * The dCache cache line size is specified in the DA1470x SoC datasheet as 2 words = 8B.
 */
 #define HW_DCACHE_CACHE_LINE_SIZE_BYTES   0x8

/*
 *  OPERATION CONTROL RELATED FUNCTIONALITY DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Checks if the dCache Controller is write flushed.
 *
 * \return True if the dirty data are evicted from the cache RAM and written into the writeBuffer,
 *         False otherwise.
 *
 */
__STATIC_INLINE bool hw_dcache_is_write_flushed(void)
{
        return REG_GETF(DCACHE, DCACHE_CTRL_REG, DCACHE_WFLUSHED);
}

/**
 * \brief Clears the indication that a prior write flushing process is complete.
 *
 */
__STATIC_INLINE void hw_dcache_clear_write_flushed(void)
{
        REG_CLR_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_WFLUSHED);
}

/**
 * \brief Checks if the writeBuffer of the dCache Controller is empty.
 *
 * \return True if the dirty data are transferred from the writeBuffer into the target data
 *         memory Controller, False otherwise. This function is applicable only in the context
 *         of a system sleep or hibernation activity.
 *
 */
__STATIC_INLINE bool hw_dcache_wbuffer_is_empty(void)
{
        return REG_GETF(DCACHE, DCACHE_CTRL_REG, DCACHE_WBUFFER_EMPTY);
}

/**
 * \brief Triggers a write flushing operation.
 *
 * Writes back to the target data memory all cache lines with at least
 * one word marked as “dirty”. Dirty bits in the TAG area are reset to ‘zero’.
 *
 */
__STATIC_INLINE void hw_dcache_write_flush(void)
{
        REG_SET_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_WFLUSH);
        /* Wait until the write flushing process is complete. */
        while (hw_dcache_is_write_flushed() != true) {}

        /* If DCACHE retention is enabled we must trigger the toggle-type hardware
         * write flush bit TWICE so as to force it back to "0".*/
        if (REG_GETF(CRG_TOP, PMU_CTRL_REG, RETAIN_DCACHE)) {
                REG_SET_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_WFLUSH);
                while (hw_dcache_is_write_flushed() != true) {}
        }

        /* Clear the completion indication to be ready for the next invocation */
        hw_dcache_clear_write_flushed();
}

/**
 * \brief Enables the dCache Controller
 *
 * Enables the dCache Controller letting its cache RAM memory be visible only to the dCache
 * Controller for data caching purposes.
 *
 */
__STATIC_INLINE void hw_dcache_enable(void)
{
        REG_SET_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_ENABLE);
}

/**
 * \brief Disables the dCache Controller
 * \param [in] mode Controls which functionalities are imperative to be executed before
 *                  clearing the corresponding bit field. The name of each mode implies
 *                  the context in which it shall be employed (normal, powering down, debug).
 *
 *
 * Disables the dCache Controller bypassing it for all read/write requests and letting its
 * cache RAM memory be visible in the entire memory space (excluding the TAG area).
 *
 * \note When there is an application need to acquire debugging related insight via disabling
 * accordingly the dCache controller, it shall be manually enabled back before proceeding
 * further in the application's execution.
 *
 */
__STATIC_FORCEINLINE void hw_dcache_disable(HW_DCACHE_DISABLE_MODE mode)
{
        ASSERT_WARNING(mode < HW_DCACHE_DISABLE_INVALID);

        switch (mode){
        case HW_DCACHE_DISABLE_NORMAL:
                hw_dcache_write_flush();
                REG_CLR_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_ENABLE);
                return;
        case HW_DCACHE_DISABLE_POWERINGDOWN:
                hw_dcache_write_flush();
                while (hw_dcache_wbuffer_is_empty() != true) {}
                REG_CLR_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_ENABLE);
                return;
        case HW_DCACHE_DISABLE_DEBUG:
                REG_CLR_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_ENABLE);
                return;
        default:
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Checks if the dCache Controller is enabled
 *
 * \return True if the dCache Controller is enabled, False otherwise.
 *
 */
__STATIC_INLINE bool hw_dcache_is_enabled(void)
{
        return REG_GETF(DCACHE, DCACHE_CTRL_REG, DCACHE_ENABLE);
}

/**
 * \brief Deactivates the dCache Controller
 *
 * All accesses (AHB transfers) towards the target data memory are routed around the dCache Controller.
 * When in such an inactive state all caching behavior is deactivated and the dCache Controller
 * DATA RAM cells can only be employed as normal RAM after having invoked the hw_dcache_disable().
 *
 * It is highly recommended to call this function once, during the initialization of the application,
 * and on the grounds that it is imperative to circumvent the dCache Controller in order to
 * suffice a particular boost performance need.
 *
 * During the normal application lifecycle it is an application responsibility and highly recommended
 * to call this function only on the grounds that there is no ongoing activity on system level towards
 * the target data memory (and via the QSPIC2 controller), otherwise memory corruption will occur.
 *
 */
__STATIC_INLINE void hw_dcache_deactivate(void)
{
        /* It is imperative to follow a similar to when powering down prior to deactivating */
        if (hw_dcache_is_enabled()) {
                hw_dcache_write_flush();
                /* Wait until the write flushing process is entirely complete. */
                while (hw_dcache_wbuffer_is_empty() != true) {}
        }
        REG_SET_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_BYPASS);
}

/**
 * \brief Activates the dCache Controller
 *
 * All accesses (AHB transfers) towards the target data memory are routed via the dCache
 * Controller. When in such an active state all caching behavior is activated and data
 * will be cached or not depending on the DCACHE_ENABLE, DCACHE_LEN and DCACHE_BASE_ADDR
 * corresponding settings.
 *
 * It is highly recommended to call this function only on the grounds that there is no ongoing activity
 * on system level towards the target data memory (and via the QSPIC2 controller), otherwise memory
 * corruption will occur.
 *
 */
__STATIC_INLINE void hw_dcache_activate(void) {
        REG_CLR_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_BYPASS);
}

/**
 * \brief Checks if the dCache Controller is deactivated
 *
 * \return True if the dCache Controller is deactivated, False otherwise.
 *
 */
__STATIC_INLINE bool hw_dcache_is_deactivated(void)
{
        return REG_GETF(DCACHE, DCACHE_CTRL_REG, DCACHE_BYPASS);
}

/**
 * \brief Initializes the dCache Controller
 *
 * Triggers the initialization of the cache RAM by invalidating its TAG area (“dirty” and
 * “valid” bits are set to ‘zeros’). It is not recommended to unintentionally invoke this
 * function during the application lifecycle otherwise cache inconsistencies will arise.
 *
 */
__STATIC_INLINE void hw_dcache_init(void)
{
        REG_SET_BIT(DCACHE, DCACHE_CTRL_REG, DCACHE_INIT);
        /* Wait until the initialization process is complete. */
        while (REG_GETF(DCACHE, DCACHE_CTRL_REG, DCACHE_READY) != 1) {}
}

/**
 * \brief Get the target data memory cacheable area length and jointly the operational mode.
 *
 * \return The cacheable area length, in 1KB blocks. The actual cacheable
 *         area length will therefore be len * 1KB.
 *         A value of 0 indicates that the dCache Controller is in bypass mode. Any value
 *         greater than zero indicates that it is in caching mode.
 *
 */
__STATIC_INLINE uint32_t hw_dcache_get_cacheable_len(void)
{
        return REG_GETF(DCACHE, DCACHE_CTRL_REG, DCACHE_LEN);
}

/**
 * \brief Set the target data memory cacheable area length and jointly the operational mode.
 *
 * \param [in] len The cacheable area length, in 1KB blocks. The actual
 *                 cacheable area length will therefore be len * 1KB.
 *                 Valid values: [0,2^17]=[0,0x20000] to address a max of 1KB*131072 = 128MB
 *                 cacheable length (corresponding to the size of the target data memory).
 *
 *                 A value of 0 sets the dCache Controller in bypass mode. Any value greater
 *                 than zero will set it in caching mode.
 *
 *                 The application is responsible to define a base address and a length that in
 *                 total constitute a cacheable area that does not exceed the boundaries of the
 *                 target data memory area.
 *
 */
__STATIC_INLINE void hw_dcache_set_cacheable_len(uint32_t len)
{
        /* The assertion focuses only in checking the max value of the cacheable length,
         * i.e. independently of a previously set base address value */
        ASSERT_WARNING(len < HW_DCACHE_CACHEABLE_LEN_MAX + 1);

        /* A write flushing process is imperative to be executed before before setting the
         * dCache Controller from caching into bypass mode when enabled. This is mandatory
         * for cache coherency to assure that there are no "dirty" words in the cache RAM */
        if (hw_dcache_get_cacheable_len() > 0 && len == 0) {
                hw_dcache_write_flush();
        }

        REG_SETF(DCACHE, DCACHE_CTRL_REG, DCACHE_LEN, len);
}

/**
 * \brief Set the base address of the cacheable area for the target data memory
 *
 * \param [in] base The cacheable area base address. The specified base address
 *             is relative to the target data memory base address, as the latter
 *             is defined in the SoC memory map and it is addressed in blocks of 1KB.
 *             Valid values: [0,2^17-1]=[0,0x1FFFF] to be able to define up to the last block
 *             of the target data memory (of a 128MB length) as a cacheable block.
 *
 *             The application is responsible to translate the input value of the base address
 *             into a physical one for debugging or any other accessing purposes by multiplying
 *             it with the cacheable block resolution (1KB = 0x400) and adding it to the target
 *             data memory base address.
 *
 *             The application is responsible to define a base address and a length that in
 *             total constitute a cacheable area that does not exceed the boundaries of the
 *             target data memory area.
 *
 */
__STATIC_INLINE void hw_dcache_set_cacheable_base(uint32_t base)
{
        /* The assertion focuses only in checking the max value of the base address,
         * i.e. independently of a previously set length value */
        ASSERT_WARNING(base < (DCACHE_DCACHE_BASE_ADDR_REG_DCACHE_BASE_ADDR_Msk + 1));
        REG_SETF(DCACHE, DCACHE_BASE_ADDR_REG, DCACHE_BASE_ADDR, base);
}

/**
 * \brief Get the base address of the cacheable area for the target data memory
 *
 * \return The cacheable area base address relative to the target data memory base address,
 *         as the latter is defined in the SoC memory map.
 *
 */
__STATIC_INLINE uint32_t hw_dcache_get_cacheable_base(void)
{
        return REG_GETF(DCACHE, DCACHE_BASE_ADDR_REG, DCACHE_BASE_ADDR);
}

/*
 *  MRM RELATED FUNCTIONALITY DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Start the dcache MRM counters
 *
 * \note If Timer interval is not set to 0 using
 *       hw_dcache_mrm_set_tint, the timer interval will count down to 0.
 *       When zero is reached, an interrupt will be generated, and the
 *       counters will be disabled automatically.
 */
__STATIC_INLINE void hw_dcache_mrm_start_counters(void)
{
        REG_SET_BIT(DCACHE, DCACHE_MRM_CTRL_REG, MRM_START);
}

/**
 * \brief Freeze the dcache MRM counters
 *
 */
__STATIC_INLINE void hw_dcache_mrm_freeze_counters(void)
{
        REG_CLR_BIT(DCACHE, DCACHE_MRM_CTRL_REG, MRM_START);
}

/**
 * \brief Get the dcache MRM misses number
 *
 * \return The number of dcache misses
 *
 */
__STATIC_INLINE uint32_t hw_dcache_mrm_get_misses(void)
{
        return DCACHE->DCACHE_MRM_MISSES_REG;
}

/**
 * \brief Set the dcache MRM cache misses number
 *
 * This is primarily intended for clearing the misses number
 *
 * \param[in] misses The number of cache misses
 *
 */
__STATIC_INLINE void hw_dcache_mrm_set_misses(uint32_t misses)
{
        DCACHE->DCACHE_MRM_MISSES_REG = misses;
}

/**
 * \brief Get the dcache MRM interrupt threshold for misses
 *
 * \return The interrupt generation threshold (in misses)
 *
 */
__STATIC_INLINE uint32_t hw_dcache_mrm_get_misses_thres(void)
{
        return DCACHE->DCACHE_MRM_MISSES_THRES_REG;
}

/**
 * \brief Set the dcache MRM interrupt threshold for misses
 *
 * Defines the threshold (in misses) to trigger the interrupt generation.
 * A value of 0 disables interrupt generation
 *
 * \param [in] thres The interrupt generation threshold (in misses)
 *
 */
__STATIC_INLINE void hw_dcache_mrm_set_misses_thres(uint32_t thres)
{
        DCACHE->DCACHE_MRM_MISSES_THRES_REG = thres;
}

/**
 * \brief Get the dcache MRM misses threshold IRQ status
 *
 * \return True if an interrupt has been generated because the number
 *         of misses reached the programmed threshold (if !=0)
 */
__STATIC_INLINE bool hw_dcache_mrm_get_misses_thres_status(void)
{
        return REG_GETF(DCACHE, DCACHE_MRM_CTRL_REG, MRM_IRQ_MISSES_THRES_STATUS);
}

/**
 * \brief Clear the dcache MRM misses threshold IRQ status
 *
 *
 */
__STATIC_INLINE void hw_dcache_mrm_clr_misses_thres_status(void)
{
        REG_CLR_BIT(DCACHE, DCACHE_MRM_CTRL_REG, MRM_IRQ_MISSES_THRES_STATUS);
}

/**
 * \brief Get the dcache MRM cache hits number
 *
 * \return The number of cache hits
 *
 */
__STATIC_INLINE uint32_t hw_dcache_mrm_get_hits(void)
{
        return DCACHE->DCACHE_MRM_HITS_REG;
}

/**
 * \brief Set the dcache MRM cache hits number
 *
 * This is primarily intended for clearing the hits number
 *
 * \param[in] hits The number of cache hits
 *
 */
__STATIC_INLINE void hw_dcache_mrm_set_hits(uint32_t hits)
{
        DCACHE->DCACHE_MRM_HITS_REG = hits;
}

/**
 * \brief Get the dcache MRM interrupt threshold for hits
 *
 * \return The interrupt generation threshold (in hits)
 *
 */
__STATIC_INLINE uint32_t hw_dcache_mrm_get_hits_thres(void)
{
        return DCACHE->DCACHE_MRM_HITS_THRES_REG;
}

/**
 * \brief Set the dcache MRM interrupt threshold for hits
 *
 * Defines the threshold (in hits) to trigger the interrupt generation.
 * A value of 0 disables interrupt generation
 *
 * \param [in] thres The interrupt generation threshold (in hits)
 *
 */
__STATIC_INLINE void hw_dcache_mrm_set_hits_thres(uint32_t thres)
{
        DCACHE->DCACHE_MRM_HITS_THRES_REG = thres;
}

/**
 * \brief Get the dcache MRM hits threshold IRQ status
 *
 * \return True if an interrupt has been generated because the number
 *         of hits reached the programmed threshold (if !=0)
 */
__STATIC_INLINE bool hw_dcache_mrm_get_hits_thres_status(void)
{
        return REG_GETF(DCACHE, DCACHE_MRM_CTRL_REG, MRM_IRQ_HITS_THRES_STATUS);
}

/**
 * \brief Clear the dcache MRM hits threshold IRQ status
 *
 *
 */
__STATIC_INLINE void hw_dcache_mrm_clr_hits_thres_status(void)
{
        REG_CLR_BIT(DCACHE, DCACHE_MRM_CTRL_REG, MRM_IRQ_HITS_THRES_STATUS);
}

/**
 * \brief Get the dcache MRM cache evicts number
 *
 * \return The number of cache evicts
 *
 */
__STATIC_INLINE uint32_t hw_dcache_mrm_get_evicts(void)
{
        return DCACHE->DCACHE_MRM_EVICTS_REG;
}

/**
 * \brief Set the dcache MRM cache evicts number
 *
 * This is primarily intended for clearing the hits number
 *
 * \param[in] hits The number of cache evicts
 *
 */
__STATIC_INLINE void hw_dcache_mrm_set_evicts(uint32_t evicts)
{
        DCACHE->DCACHE_MRM_EVICTS_REG = evicts;
}

/**
 * \brief Get the dcache MRM interrupt threshold for evicts
 *
 * \return The interrupt generation threshold (in evicts)
 *
 */
__STATIC_INLINE uint32_t hw_dcache_mrm_get_evicts_thres(void)
{
        return DCACHE->DCACHE_MRM_EVICTS_THRES_REG;
}

/**
 * \brief Set the dcache MRM interrupt threshold for evicts
 *
 * Defines the threshold (in evicts) to trigger the interrupt generation.
 * A value of 0 disables interrupt generation
 *
 * \param [in] thres The interrupt generation threshold (in evicts)
 *
 */
__STATIC_INLINE void hw_dcache_mrm_set_evicts_thres(uint32_t thres)
{
        DCACHE->DCACHE_MRM_EVICTS_THRES_REG = thres;
}

/**
 * \brief Get the dcache MRM evicts threshold IRQ status
 *
 * \return True if an interrupt has been generated because the number
 *         of evicts reached the programmed threshold (if !=0)
 */
__STATIC_INLINE bool hw_dcache_mrm_get_evicts_thres_status(void)
{
        return REG_GETF(DCACHE, DCACHE_MRM_CTRL_REG, MRM_IRQ_EVICTS_THRES_STATUS);
}

/**
 * \brief Clear the dcache MRM evicts threshold IRQ status
 *
 *
 */
__STATIC_INLINE void hw_dcache_mrm_clr_evicts_thres_status(void)
{
        REG_CLR_BIT(DCACHE, DCACHE_MRM_CTRL_REG, MRM_IRQ_EVICTS_THRES_STATUS);
}

/**
 * \brief Get the dcache MRM monitoring time interval
 *
 * \return The monitoring time interval in clock cycles
 *
 */
__STATIC_INLINE uint32_t hw_dcache_mrm_get_tint(void)
{
        return DCACHE->DCACHE_MRM_TINT_REG & DCACHE_DCACHE_MRM_TINT_REG_MRM_TINT_Msk;
}

/**
 * \brief Set the dcache MRM monitoring time interval
 *
 * Defines the time interval for the monitoring in 32 MHz clock cycles.
 * Must be an 19-bit value max. When this time is reached, an interrupt
 * will be generated.
 * A value of 0 disables interrupt generation
 *
 * \param [in] tint Monitoring time interval in clock cycles
 *
 */
__STATIC_INLINE void hw_dcache_mrm_set_tint(uint32_t tint)
{
        ASSERT_WARNING((tint & ~DCACHE_DCACHE_MRM_TINT_REG_MRM_TINT_Msk) == 0)
                DCACHE->DCACHE_MRM_TINT_REG = tint;
}

/**
 * \brief Get the dcache MRM timer interval IRQ status
 *
 * \return True if an interrupt has been generated because the time
 *         interval counter reached the end (time interval != 0).
 *
 */
__STATIC_INLINE bool hw_dcache_mrm_get_tint_status(void)
{
        return REG_GETF(DCACHE, DCACHE_MRM_CTRL_REG, MRM_IRQ_TINT_STATUS);
}

/**
 * \brief Clear the dcache MRM timer interval IRQ status
 *
 */
__STATIC_INLINE void hw_dcache_mrm_clr_tint_status(void)
{
        REG_CLR_BIT(DCACHE, DCACHE_MRM_CTRL_REG, MRM_IRQ_TINT_STATUS);
}

/**
 * \brief Application defined callback for the dCache MRM interrupt.
 *
 * \note The application defined callback should be declared as __RETAINED_CODE.
 *
 */
typedef void (*hw_dcache_mrm_cb_t)(void);

/**
 * \brief Enable the dcache MRM interrupt generation
 *
 * The application should define its own callback. The latter is registered
 * and then invoked when the MRM interrupt is generated.
 *
 * \param [in] cb Callback defined by the application.
 *
 */
void hw_dcache_mrm_enable_interrupt(hw_dcache_mrm_cb_t cb);

/**
 * \brief Disable the dcache MRM interrupt generation
 *
 * \note The application defined callback is unregistered.
 *
 */
void hw_dcache_mrm_disable_interrupt(void);

#endif /* dg_configUSE_HW_DCACHE */

#endif /* HW_DCACHE_H_ */

/**
 * \}
 * \}
 */
