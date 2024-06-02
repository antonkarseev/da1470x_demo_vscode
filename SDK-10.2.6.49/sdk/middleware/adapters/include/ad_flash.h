/**
 ****************************************************************************************
 *
 * @file ad_flash.h
 *
 * @brief Flash adapter API
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup FLASH_ADAPTER QSPI Flash Adapter
 *
 * \brief QSPI flash adapter
 *
 * \note For DA1469x devices two address sub-regions are defined:
 *       Address region 1: 0..QSPI_MEM2_VIRTUAL_BASE_ADDR-1
 *       Address region 2: starting at QSPI_MEM2_VIRTUAL_BASE_ADDR
 *       Any address in region 1 is mapped to QSPIC1.
 *       Any address in region 2 is mapped to QSPIC2.
 *
 * \{
 */

#ifndef AD_FLASH_H_
#define AD_FLASH_H_

#if dg_configFLASH_ADAPTER

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "qspi_automode.h"
#include "oqspi_automode.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define AD_FLASH_GET_SECTOR_SIZE(addr)  (FLASH_SECTOR_SIZE)
#define AD_FLASH_MAX_SECTOR_SIZE        (FLASH_SECTOR_SIZE)

/**
 * \brief Initialize flash access.
 *
 * This function must be called before any other ad_flash_* function
 */
void ad_flash_init(void);

/**
 * \brief Read flash memory
 *
 * \param [in] addr starting offset
 * \param [out] buf buffer to read data to
 * \param [in] len number of bytes to read
 *
 * \returns number of bytes read
 */
size_t ad_flash_read(uint32_t addr, uint8_t *buf, size_t len);

/**
 * \brief Write flash memory
 *
 * This function allows to write any number of bytes to flash.
 * Memory should be erased before.
 *
 * \note: User can pass buf pointing to QSPI mapped memory.
 *
 * \param [in] addr offset in flash to write data to
 * \param [in] buf pointer to data to write
 * \param [in] size number of bytes to write
 *
 * return number of bytes written
 *
 */
size_t ad_flash_write(uint32_t addr, const uint8_t *buf, size_t size);

/**
 * \brief Erase flash region
 *
 * \note: All sectors that have offset in range of <addr, addr + size) will be erased.
 * If addr is not sector aligned preceding data on sector that addr belong to will also be erased.
 * If addr + size is not sector aligned whole sector will also be erased.
 *
 * \param [in] addr starting offset of sector
 * \param [in] size number of bytes to erase
 *
 * \return true on success
 *
 */
bool ad_flash_erase_region(uint32_t addr, size_t size);

/**
 * \brief Get physical pointer for a virtual address in a memory device's contents
 *
 * \param [in] addr virtual address in a memory device
 *
 * \retval NULL if addr cannot be mapped to a memory device
 * \retval pointer that can be used to directly access the data pointed to by virtual addr
 *
 */
__STATIC_INLINE const void *ad_flash_get_ptr(uint32_t addr)
{
#if (dg_configUSE_HW_QSPI == 1) || (dg_configUSE_HW_QSPI2 == 1)
        if (qspi_is_valid_addr(addr)) {
                return qspi_automode_addr(addr);
        }
#endif
#if dg_configUSE_HW_OQSPI
        return oqspi_automode_get_physical_addr(addr);
#else
        return NULL;
#endif
}

/**
 * \brief Check if update without erase is possible
 *
 * Update is possible when only "0s" have to be written.
 *
 * This function test if its possible to write to flash without erase.
 * If function returns size, data did not change no erase needed.
 * If non negative value means that write can be performed some bits will be cleared on write
 * Negative value erase is needed.
 *
 * \param [in] addr starting offset of sector
 * \param [in] data_to_write data to write to flash
 * \param [in] size number of bytes to check
 *
 * \return -1 - erase is needed
 *         >= number of bytes that do not need to be written (same) 0 means that write should start
 *            from offset 0 but no erase is needed.
 *
 */
int ad_flash_update_possible(uint32_t addr, const uint8_t *data_to_write, size_t size);

/**
 * \brief Get flash erase size
 *
 * \param [in] addr Address of memory
 *
 * \return minimum size that can be erased
 *
 */
size_t ad_flash_erase_size(uint32_t addr);


/**
 * \brief Erase specific flash starting at \p addr
 *
 * \param [in] addr Start address of the flash memory to be erased
 *
 * \return false if \p addr is wrong; true otherwise
 */
bool ad_flash_chip_erase_by_addr(uint32_t addr);

/**
 * \brief Lock access to entire flash
 *
 * Function gets flash for exclusive usage
 *
 */
void ad_flash_lock(void);

/**
 * \brief Unlock access to entire flash
 *
 * Function release lock taken by \sa ad_flash_lock()
 *
 */
void ad_flash_unlock(void);

/**
 * Special base address that can be used with \sa ad_flash_skip_cache_flushing
 * to return to default cache flushing mode: all flash writes/erases trigger
 * cache flushing.
 */
#define AD_FLASH_ALWAYS_FLUSH_CACHE     ((uint32_t)-1)

/**
 * \brief Control cache flushing on modifications (writes or erases) to flash.
 *
 * This function can be used to enable or disable the triggering of cache flushing
 * when writes or erases occur in a specific flash region. Only one such flash region
 * can be defined.
 *
 * This feature is useful when the programmer knows in advance that a big flash region
 * is going to be updated (e.g. firmware update). However, flash reads from that region
 * should be avoided, as they might lead to cache incoherency.
 *
 * \note The effect of this function is at ad_flash_* layer and higher.
 *       For example, direct use of the hw_qspi_* API will not be affected by the use
 *       of this function.
 *
 * \param [in] base  Starting offset of the flash region that should not trigger a cache
 *                   flushing.
 *                   If \sa AD_FLASH_ALWAYS_FLUSH_CACHE is passed, selective cache flushing
 *                   is disabled, regardless of the value of size.
 * \param [in] size  The size of the flash region that should not trigger cache flushing.
 *
 */
void ad_flash_skip_cache_flushing(uint32_t base, uint32_t size);

#endif /* dg_configFLASH_ADAPTER */

#endif /* AD_FLASH_H_ */
/**
 \}
 \}
 */
