/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_CACHE iCache Controller
 * \{
 * \brief iCache Controller DA1470x specific LLD API
 */

/**
 *****************************************************************************************
 *
 * @file hw_cache_da1470x.h
 *
 * @brief Definition of DA1470x specific API for the iCache Controller Low Level Driver.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_CACHE_DA1470X_H_
#define HW_CACHE_DA1470X_H_


#include <sdk_defs.h>

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Cacheable flash Region Sizes as defined in DA1470x datasheet
 *
 */

typedef enum {
        HW_CACHE_FLASH_REGION_SZ_256KB = 0,
        HW_CACHE_FLASH_REGION_SZ_512KB = 1,
        HW_CACHE_FLASH_REGION_SZ_1MB = 2,
        HW_CACHE_FLASH_REGION_SZ_2MB = 3,
        HW_CACHE_FLASH_REGION_SZ_4MB = 4,
        HW_CACHE_FLASH_REGION_SZ_8MB = 5,
        HW_CACHE_FLASH_REGION_SZ_16MB = 6,
        HW_CACHE_FLASH_REGION_SZ_32MB = 7,
        HW_CACHE_FLASH_REGION_SZ_64MB = 8,
        HW_CACHE_FLASH_REGION_SZ_128MB = 9,
        HW_CACHE_FLASH_REGION_SZ_INVALID,              /* Used as iteration terminator */
} HW_CACHE_FLASH_REGION_SZ;

typedef uint16_t flash_region_base_t;

/* With a 64KB cacheable resolution (i.e. the target memory is addressed in blocks of 64KB),
 * and a size of 131072KB of the target memory (MEMORY_OQSPIC_SIZE) the maximum cacheable length
 * is 131072 = 131072KB / 64KB. In hex: 0x8000000 / 0x10000 = 0x800. */
#define HW_CACHE_CACHEABLE_RESOLUTION     0x10000                /* Blocks of 64KB in size */
#define HW_CACHE_CACHEABLE_LEN_MAX        MEMORY_OQSPIC_SIZE / HW_CACHE_CACHEABLE_RESOLUTION

#define HW_CACHE_FLASH_DEFAULT_REGION_BASE     0x1800
#define HW_CACHE_FLASH_MAX_REGION_BASE         0x1FFF
#define HW_CACHE_FLASH_DEFAULT_REGION_OFFSET   0x0
/* Maximum offset = 0xFFF = 2^12-1 as the register field is 12 bits in length and addressed in words (* 4) => 16KB */
#define HW_CACHE_FLASH_MAX_REGION_OFFSET       0xFFF
typedef uint16_t flash_region_offset_t;

#define HW_CACHE_FLASH_DEFAULT_REGION_SZ       HW_CACHE_FLASH_REGION_SZ_512KB

/*
 * FAMILY SPECIFIC GENERIC FUNCTIONALITY DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Enables the iCache Controller
 *
 * The iCache Controller is enabled by setting the CACHERAM_MUX to '1'. This action enables
 * the corresponding HW block, letting the RAM memory of the block be visible only to the
 * iCache Controller for caching purposes.
 *
 */
__STATIC_INLINE void hw_cache_enable(void)
{
        REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX);
        /* Wait until the CACHERAM_MUX=1 (because of the APB Bridge). */
        while (REG_GETF(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX) != 1) {}
}

/**
 * \brief Disables the iCache Controller
 *
 * The iCache Controller is disabled by setting the CACHERAM_MUX to '0'. This action disables
 * the corresponding HW block, bypassing the iCache Controller for all read requests
 * and letting the RAM memory of the block be visible in the entire memory space.
 *
 */
__STATIC_INLINE void hw_cache_disable(void)
{
        REG_CLR_BIT(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX);
        /* Wait until the CACHERAM_MUX=0 (because of the APB Bridge). */
        while (REG_GETF(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX) != 0) {}
}

/**
 * \brief Checks if the iCache Controller is enabled
 *
 * \return True if the iCache Controller is enabled, False otherwise.
 *
 *
 */
__STATIC_INLINE bool hw_cache_is_enabled(void)
{
        return REG_GETF(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX);
}

/**
 * \brief Set the external flash cacheable memory length
 *
 * \param [in] len The external flash cacheable memory length, in 64KB blocks. The actual
 *                 cacheable memory length will therefore be len * 64KB.
 *                 Valid values: [0, 2048] to address a max 128MB cacheable length.
 *                 A value of 0 sets the iCache Controller in bypass mode for the read
 *                 requests targeting the cacheable external flash memory area. Any value greater
 *                 than zero will set it in caching mode.
 *
 * \note Indicates the size of the instruction code that will be cached in an execution lifecycle.
 *       Differs from region size (hw_cache_flash_set/get_region_size()). Runtime reconfigurable.
 */
__STATIC_INLINE void hw_cache_set_extflash_cacheable_len(uint32_t len)
{
        ASSERT_WARNING(len <= HW_CACHE_CACHEABLE_LEN_MAX);
        REG_SETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN, len);
}

/**
 * \brief Get the external flash cacheable memory length
 *
 * \return The flash cacheable memory length, in 64KB blocks. The actual cacheable
 *         memory length will therefore be len * 64KB.
 */
__STATIC_INLINE int hw_cache_get_extflash_cacheable_len(void)
{
        return REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN);
}

/**
 * \brief Set the cacheable memory length. Backwards compatibility wrapper.
 *
 * \param [in] len See hw_cache_set_extflash_cacheable_len for details.
 *
 * \deprecated API no longer supported, use hw_cache_set_extflash_cacheable_len.
 */
DEPRECATED_MSG("API no longer supported, use hw_cache_set_extflash_cacheable_len.")
__STATIC_INLINE void hw_cache_set_len(uint32_t len)
{
        hw_cache_set_extflash_cacheable_len(len);
}

/**
 * \brief Get the cacheable memory length. Backwards compatibility wrapper.
 *
 * \return See hw_cache_get_extflash_cacheable_len for details.
 * \deprecated API no longer supported, use hw_cache_get_extflash_cacheable_len.
 */
DEPRECATED_MSG("API no longer supported, use hw_cache_get_extflash_cacheable_len.")
__STATIC_INLINE int hw_cache_get_len(void)
{
        return hw_cache_get_extflash_cacheable_len();
}

/**
 * \brief Enable flushing the iCache Controller (cache RAM cells) contents. For debugging only.
 */
__STATIC_INLINE void hw_cache_enable_flushing(void)
{
        REG_CLR_BIT(CACHE, CACHE_CTRL2_REG, CACHE_FLUSH_DISABLE);
}

/**
 * \brief Disable flushing the iCache Controller (cache RAM cells) contents. For debugging only.
 */
__STATIC_INLINE void hw_cache_disable_flushing(void)
{
        REG_SET_BIT(CACHE, CACHE_CTRL2_REG, CACHE_FLUSH_DISABLE);
}

/**
 * \brief Checks if the iCache Controller flushing is disabled. For debugging only.
 *
 * \return True if the iCache Controller flushing is disabled, False otherwise.
 *
 *
 */
__STATIC_INLINE bool hw_cache_is_flushing_disabled(void)
{
        return REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_FLUSH_DISABLE);
}

/**
 * \brief Check if the flushing process is complete
 *
 * \return True if flushing is complete, False if the iCache controller flushing is still
 *              in progress or there is no pending flushing termination indication as it
 *              will be cleared via a prior hw_cache_clear_flushed() call.
 */
__STATIC_INLINE bool hw_cache_is_flushed(void)
{
        return REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_FLUSHED);
}

/**
 * \brief Clear the indication that a prior flushing process is complete
 */
__STATIC_INLINE void hw_cache_clear_flushed(void)
{
        /* a negative edge must be created by SW to clear the bit */
        REG_SET_BIT(CACHE, CACHE_CTRL2_REG, CACHE_FLUSHED);
        REG_CLR_BIT(CACHE, CACHE_CTRL2_REG, CACHE_FLUSHED);
        __DSB();
}

/**
 * \brief Flush the iCache controller contents
 *
 * Note: The very first flushing occurred after power on reset when the iCache Controller
 * is enabled for the first time by the booter.
 */
__STATIC_INLINE void hw_cache_flush(void)
{
        if (!hw_cache_is_flushing_disabled()) {
                hw_cache_disable();
                hw_cache_enable();
                /* Wait for the completion of the flushing process */
                while (hw_cache_is_flushed() == 0);

                /* Clear the indication that the flushing is complete */
                hw_cache_clear_flushed();
        }
}

/**
* \brief iCache controller status
*
* \return True if iCache controller is enabled, initialized and ready for a cacheable access.
*
*/
__STATIC_INLINE bool hw_cache_is_ready(void)
{
      return REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_READY);
}

/**
 * \brief Enable Critical Word First
 *
 * It affects which word is fetched first on a cache line refill.
 */
__STATIC_INLINE void hw_cache_enable_cwf(void)
{
        REG_CLR_BIT(CACHE, CACHE_CTRL2_REG, CACHE_CWF_DISABLE);
}

/**
 * \brief Disable Critical Word First
 *
 * It affects which word is fetched first on a cache line refill.
 *
 * Note: When CWF is disabled, the cache line refill is performed with a burst
 */
__STATIC_INLINE void hw_cache_disable_cwf(void)
{
        REG_SET_BIT(CACHE, CACHE_CTRL2_REG, CACHE_CWF_DISABLE);
}

/**
 * \brief Checks if the iCache Controller Critical-Word-First mode is disabled
 *
 * \return True if the iCache Controller CWF is disabled, False otherwise.
 */
__STATIC_INLINE bool hw_cache_is_cwf_disabled(void)
{
        return REG_GETF(CACHE, CACHE_CTRL2_REG, CACHE_CWF_DISABLE);
}

/*
 * CACHEABLE FLASH RELATED FUNCTIONALITY DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Set the flash region base. Indicates where caching will start from.
 *
 * \param [in] base The Flash region base corresponds to the flash address bits
 *             [31:16]. Default values is '0x1800'. Bits [31:27] are fixed to '0b00011'.
 *             Therefore, valid values are from 0x1800 to 0x1FFF. This address should be
 *             region 'size'-param aligned (hw_cache_flash_set_region_size()).
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_set_region_base(flash_region_base_t base)
{
        ASSERT_WARNING((base >= HW_CACHE_FLASH_DEFAULT_REGION_BASE) &&
                (base <= HW_CACHE_FLASH_MAX_REGION_BASE));
        REG_SETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE, base);
}

/**
 * \brief Get the flash region base
 *
 * \return The flash region base to use with the iCache controller
 */
__STATIC_INLINE flash_region_base_t hw_cache_flash_get_region_base(void)
{
        return REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE);
}

/**
 * \brief Set the flash region offset. Indicates where remapping will start from.
 *
 * This value (expressed in words) is added to flash region base
 * (see hw_cache_flash_set/get_region_base()) to calculate the
 * the starting address within the flash memory area that will
 * be remapped to 0x0 and XiPed.
 *
 * \param [in] offset flash region offset in 32-bit words. Max: 0xFFF
 *              since the corresponding register bit field area is 3 nibbles
 *              in length.
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_set_region_offset(flash_region_offset_t offset)
{
        ASSERT_WARNING(offset <= HW_CACHE_FLASH_MAX_REGION_OFFSET);
        REG_SETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_OFFSET, offset);
}

/**
 * \brief Get the flash region offset
 *
 * \return The region offset to be used in conjunction with the region base to indicate
 * the starting address within the flash memory area that will be remapped.
 */
__STATIC_INLINE flash_region_offset_t hw_cache_flash_get_region_offset(void)
{
        return REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_OFFSET);
}

/**
 * \brief Set the flash region size
 *
 * \param [in] sz The cacheable region size to use with the iCache controller
 *
 * This is the size of the flash memory that is cacheable and remappable in which one
 * or more FW images and SW library modules may reside and can be XiPed.
 *
 * \note Differs from the cache length (see hw_cache_set/get_extflash_cacheable_len()).
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_set_region_size(HW_CACHE_FLASH_REGION_SZ sz)
{
        ASSERT_WARNING(sz < HW_CACHE_FLASH_REGION_SZ_INVALID);
        REG_SETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_SIZE, sz);
}

/**
 * \brief Get the flash region size
 *
 * \return The flash region size to use with the iCache controller
 */
__STATIC_INLINE HW_CACHE_FLASH_REGION_SZ hw_cache_flash_get_region_size(void)
{
        return REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_SIZE);
}

/**
 * \brief Configure the flash memory region that will be cacheable
 *
 * This is an alternative API to hw_cache_flash_set_region_base()/_size()/_offset(). It automatically
 * configures the entire flash region in one call.
 *
 * See the relevant called functions for input parameter definition.
 *
 * \note The updated value takes effect only after a software reset.
 */
__STATIC_INLINE void hw_cache_flash_configure_region(flash_region_base_t base, flash_region_offset_t offset,
        HW_CACHE_FLASH_REGION_SZ sz)
{
        hw_cache_flash_set_region_base(base);
        hw_cache_flash_set_region_offset(offset);
        hw_cache_flash_set_region_size(sz);
}

/*
 * MRM RELATED FUNCTIONALITY DEFINITIONS
 *****************************************************************************************
 */
/**
 * \brief Get the iCache controller MRM hits with 1 Wait State number
 *
 * \return The number of hits with 1 Wait State
 *
 */
__STATIC_INLINE uint32_t hw_cache_mrm_get_hits_with_one_wait_state(void)
{
        return CACHE->CACHE_MRM_HITS1WS_REG;
}

/**
 * \brief Set the iCache controller MRM hits with 1 Wait State number
 *
 * This is primarily intended for clearing the register
 *
 * \param[in] hits The number of hits with 1 Wait State
 *
 */
__STATIC_INLINE void hw_cache_mrm_set_hits_with_one_wait_state(uint32_t hits)
{
        CACHE->CACHE_MRM_HITS1WS_REG = hits;
}

#endif /* HW_CACHE_DA1470X_H_ */

/**
 * \}
 * \}
 */
