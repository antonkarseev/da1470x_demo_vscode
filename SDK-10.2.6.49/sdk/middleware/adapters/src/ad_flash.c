/**
 ****************************************************************************************
 *
 * @file ad_flash.c
 *
 * @brief Flash adapter implementation
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configFLASH_ADAPTER

#include <string.h>

#include "osal.h"
#include "ad_flash.h"
#include "qspi_automode.h"

# if dg_configUSE_HW_OQSPI
# include "oqspi_automode.h"
# endif
# if dg_configUSE_SYS_BACKGROUND_FLASH_OPS
# include "sys_background_flash_ops.h"
# include "../../sys_man/sys_background_flash_ops_internal.h"
# endif

#ifdef OS_PRESENT
#include "sys_power_mgr.h"
#endif

#include "hw_cache.h"
#include "hw_sys.h"

/**
 * Enable/Disable run-time checks for possible cache incoherence:
 *  - 1 to enable
 *  - 0 to disable
 */
#define DETECT_CACHE_INCOHERENCE_DANGER         0

#define FLASH_PAGE_SIZE   0x0100

/*
 * In case that user tries to write data to flash passing as source QSPI mapped flash address
 * data must be first copied to RAM as during write QSPI controller can't read flash.
 * To achieve this, small on stack buffer will be used to copy data to RAM first.
 * This define specifies how much stack can be used for this purpose.
 */
#define ON_STACK_BUFFER_SIZE 16

__RETAINED static bool initialized;
#ifdef OS_PRESENT
__RETAINED static OS_MUTEX flash_mutex;
#endif
__RETAINED static uint32_t no_cache_flush_base;
__RETAINED static uint32_t no_cache_flush_end;

__STATIC_INLINE bool is_flash_addr_cached(uint32_t addr)
{
        uint32_t cache_len;
        uint32_t cache_base;

        if (hw_sys_get_memory_remapping() != HW_SYS_REMAP_ADDRESS_0_TO_OQSPI_FLASH) {
                return false;
        }

        /*
         * Cacheable area is N * 64KB
         *
         * N == 0 --> no caching, the iCache controller is then in bypass mode.
         */
        cache_len = hw_cache_get_extflash_cacheable_len();

        cache_base = hw_cache_flash_get_region_base() << CACHE_CACHE_FLASH_REG_FLASH_REGION_BASE_Pos;
        cache_base -= MEMORY_OQSPIC_BASE;

        return ((addr >= cache_base) && (addr < cache_base + (cache_len << 16)));
}

__STATIC_INLINE bool is_base_within_flushable_area(uint32_t base, uint32_t size)
{
        if ((base >= no_cache_flush_base) && ((base + size) <= no_cache_flush_end))
                return false;
        else
                return true;
}

void ad_flash_init(void)
{
        if (!initialized) {
                initialized = true;
#ifdef OS_PRESENT
                OS_MUTEX_CREATE(flash_mutex);
                OS_ASSERT(flash_mutex);
#endif

                ad_flash_lock();
#if dg_configUSE_HW_OQSPI
                if (dg_configCODE_LOCATION != NON_VOLATILE_IS_OQSPI_FLASH) {
                        oqspi_automode_flash_power_up();
                }
#endif
                qspi_automode_flash_power_up();
                no_cache_flush_base = AD_FLASH_ALWAYS_FLUSH_CACHE;
                no_cache_flush_end = 0;

                ad_flash_unlock();
        }
}

size_t ad_flash_read(uint32_t addr, uint8_t *buf, size_t len)
{
        size_t read = 0;
        ASSERT_WARNING(buf);

#if dg_configUSE_HW_OQSPI
        bool addr_is_in_oqspi = oqspi_is_valid_addr(addr);
#endif

#if DETECT_CACHE_INCOHERENCE_DANGER
        /* An address within the cacheable area and a read space excluded
         * from being flushed (see ad_flash_skip_cache_flushing()) create a condition
         * for potential cache incoherence.
         */
#if dg_configUSE_HW_OQSPI
        if (addr_is_in_oqspi) {
                OS_ASSERT(!is_flash_addr_cached(addr) || is_base_within_flushable_area(addr, len));
        }
#endif
#endif /* DETECT_CACHE_INCOHERENCE_DANGER */

        ad_flash_lock();
#if dg_configUSE_HW_OQSPI
        if (addr_is_in_oqspi) {
                read = oqspi_automode_read(addr, buf, len);
        } else
#endif
        {
#if dg_configUSE_HW_QSPI || dg_configUSE_HW_QSPI2
                read = qspi_automode_read(addr, buf, len);
#endif
        }
        ad_flash_unlock();
        return read;
}

__STATIC_INLINE bool is_qspi_address(const void *buf)
{

  #if (dg_configUSE_HW_QSPI == 0) && (dg_configUSE_HW_QSPI2 == 0)
        return false;
  #endif

  #if dg_configUSE_HW_QSPI
        if (IS_QSPIC_ADDRESS((uint32_t) buf)) {
                return true;
        }
  #endif /* dg_configUSE_HW_QSPI */

  #if dg_configUSE_HW_QSPI2
        if (IS_QSPIC2_ADDRESS((uint32_t) buf)) {
                return true;
        }
  #endif /* dg_configUSE_HW_QSPI2 */
        return false;

}

__STATIC_INLINE bool is_oqspi_address(const void *buf)
{
  #if dg_configUSE_HW_OQSPI
        if (IS_OQSPIC_ADDRESS((uint32_t) buf)) {
                return true;
        }
        else if (IS_OQSPIC_S_ADDRESS((uint32_t) buf)) {
                return true;
        }

        /* cppcheck-suppress unsignedPositive */
        return IS_REMAPPED_ADDRESS(buf) && (hw_sys_get_memory_remapping() == HW_SYS_REMAP_ADDRESS_0_TO_OQSPI_FLASH);
  #else
        return false;
  #endif /* dg_configUSE_HW_OQSPI */
}


#if dg_configUSE_HW_OQSPI
static size_t ad_flash_write_from_oqspi(uint32_t addr, const uint8_t *oqspi_buf, size_t size)
{
        size_t offset = 0;
        uint8_t buf[ON_STACK_BUFFER_SIZE];

        /* Get the FLASH offset of oqspi_buf */
        if (IS_OQSPIC_ADDRESS((uint32_t) oqspi_buf)) {
                oqspi_buf -= MEMORY_OQSPIC_BASE;
        } else if (IS_OQSPIC_S_ADDRESS((uint32_t) oqspi_buf)) {
                oqspi_buf -= MEMORY_OQSPIC_S_BASE;
        } else {
                uint32_t flash_region_base_offset;
                flash_region_base_offset = hw_cache_flash_get_region_base() << CACHE_CACHE_FLASH_REG_FLASH_REGION_BASE_Pos;
                flash_region_base_offset += hw_cache_flash_get_region_offset() << 2;
                flash_region_base_offset -= MEMORY_OQSPIC_BASE;
                oqspi_buf += flash_region_base_offset;
        }

        /* cppcheck-suppress unsignedPositive */
        ASSERT_WARNING(IS_REMAPPED_ADDRESS(oqspi_buf));

        /* Get automode address of oqspi_buf */
        oqspi_buf = (const uint8_t*) oqspi_automode_get_physical_addr((uint32_t) oqspi_buf);

        /*
         * oqspi_automode_write_flash_page can't write data if source address points to OQSPI mapped
         * memory. To write data from OQSPI flash, it will be first copied to small on stack
         * buffer. This buffer can be accessed when the OQSPI controller is working in manual mode.
         */
        while (offset < size) {
                size_t chunk = sizeof(buf) > size - offset ? size - offset: sizeof(buf);
                memcpy(buf, oqspi_buf + offset, chunk);

#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS
                size_t written = sys_background_flash_ops_write_page(addr + offset, buf, chunk);
#else
                size_t written = oqspi_automode_write_flash_page(addr + offset, buf, chunk);
#endif
                offset += written;
        }
        return offset;
}
#endif /* dg_configUSE_HW_OQSPI */

static bool should_flush(uint32_t addr, size_t size)
{
        return is_flash_addr_cached(addr & ~(AD_FLASH_GET_SECTOR_SIZE(addr) - 1))
                        && is_base_within_flushable_area(addr, size);
}

static void flush_icache(uint32_t addr, size_t size)
{
#if dg_configUSE_HW_OQSPI
        if (oqspi_is_valid_addr(addr))
#endif
        {
                if (should_flush(addr, size)) {
                        hw_cache_flush();
                }
        }
}

/* typedef's for the two different write internal APIs */
typedef size_t (* fp_write_via_ram)(uint32_t _addr, const uint8_t *_buf, size_t _size);
typedef uint32_t (* fp_write_direct)(uint32_t addr, const uint8_t *buf, uint32_t size);

static bool write_conflicts_with_xip(uint32_t addr, const uint8_t *buf, fp_write_via_ram *fp)
{
#if dg_configUSE_HW_OQSPI
        /* cppcheck-suppress unsignedPositive */
        if (is_oqspi_address(buf) && IS_OQSPI_MEM1_VIRTUAL_ADDRESS(addr)) {
                *fp = ad_flash_write_from_oqspi;
                /* buf and destination addresses belong on the running XIP flash .*/
                return true;
        }
#endif
        return false;
}

static fp_write_direct get_write_direct_func(uint32_t addr)
{
#if dg_configUSE_HW_QSPI || dg_configUSE_HW_QSPI2
        if (qspi_is_valid_addr(addr)) {
                return qspi_automode_write_flash_page;
        }
#endif
#if dg_configUSE_HW_OQSPI
        if (oqspi_is_valid_addr(addr)) {
#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS
                return sys_background_flash_ops_write_page;
#else
                return oqspi_automode_write_flash_page;
#endif
        }
#endif
        return NULL;
}
size_t ad_flash_write(uint32_t addr, const uint8_t *buf, size_t size)
{
        size_t written;
        size_t offset = 0;
        bool buf_conflicts_with_xip;
        union {
                fp_write_via_ram via_ram;
                fp_write_direct direct;
        } write_api;

        ASSERT_WARNING(buf);

        /* assume that buf lies either completely in a xSPI or completely outside any xSPI device */
        buf_conflicts_with_xip = write_conflicts_with_xip(addr, buf, &write_api.via_ram);
        if (!buf_conflicts_with_xip) {
                write_api.direct = get_write_direct_func(addr);
                if (NULL == write_api.direct) {
                        return 0;
                }
        }

        ad_flash_lock();

        while (offset < size) {
                /*
                 * If buf conflicts with the current XIP flash memory, copy source data to RAM first.
                 */
                if (buf_conflicts_with_xip) {
                        written = write_api.via_ram(addr + offset, buf + offset, size - offset);

                } else {
                        /*
                         * Try to write everything, qspi_automode/oqspi_automode will reduce
                         * this value to accommodate page boundary and maximum write size limitation
                         */
                        written = write_api.direct(addr + offset, buf + offset, size - offset);
                }
                offset += written;
        }

        flush_icache(addr, size);

        ad_flash_unlock();

        return size;
}

/* typedef for the internal API for erasing a sector */
typedef void (* fp_erase_sector)(uint32_t addr);

static fp_erase_sector get_erase_sector_func(uint32_t addr)
{
#if dg_configUSE_HW_QSPI || dg_configUSE_HW_QSPI2
        if (qspi_is_valid_addr(addr)) {
                return qspi_automode_erase_flash_sector;
        }
#endif
#if dg_configUSE_HW_OQSPI
        if (oqspi_is_valid_addr(addr)) {
#if dg_configUSE_SYS_BACKGROUND_FLASH_OPS
                return sys_background_flash_ops_erase_sector;
#else
                return oqspi_automode_erase_flash_sector;
#endif
        }
#endif
        return NULL;
}

bool ad_flash_erase_region(uint32_t addr, size_t size)
{
        uint32_t flash_offset = addr & ~(AD_FLASH_GET_SECTOR_SIZE(addr) - 1);
        fp_erase_sector erase_sector = get_erase_sector_func(addr);
        if (NULL == erase_sector) {
                return false;
        }

        ad_flash_lock();

        while (flash_offset < addr + size) {
                erase_sector(flash_offset);
                flash_offset += AD_FLASH_GET_SECTOR_SIZE(addr);
        }

        flush_icache(addr, size);

        ad_flash_unlock();

        return true;
}


bool ad_flash_chip_erase_by_addr(uint32_t addr)
{
#if dg_configUSE_HW_QSPI
        if (addr == QSPI_MEM1_VIRTUAL_BASE_ADDR) {
                ad_flash_lock();
                qspi_automode_erase_chip_by_id(HW_QSPIC);
                ad_flash_unlock();

                return true;
        }
#endif

#if dg_configUSE_HW_QSPI2
        if ((addr == QSPI_MEM2_VIRTUAL_BASE_ADDR) && !qspi_is_ram_device(HW_QSPIC2)) {
                ad_flash_lock();
                qspi_automode_erase_chip_by_id(HW_QSPIC2);
                ad_flash_unlock();

                return true;
        }
#endif

#if dg_configUSE_HW_OQSPI
        if (addr == OQSPI_MEM1_VIRTUAL_BASE_ADDR) {
                ad_flash_lock();
                oqspi_automode_erase_chip();
                ad_flash_unlock();

                return true;
        }
#endif

        /* Wrong start address */
        return false;
}

static bool get_automode_addr(uint32_t addr, const uint8_t **automode_addr)
{
#if dg_configUSE_HW_QSPI || dg_configUSE_HW_QSPI2
        if (qspi_is_valid_addr(addr)) {
                *automode_addr = qspi_automode_addr(addr);
                return true;
        }
#endif
#if dg_configUSE_HW_OQSPI
        if (oqspi_is_valid_addr(addr)) {
                *automode_addr = oqspi_automode_get_physical_addr(addr);
                return true;
        }
#endif
        return false;
}

int ad_flash_update_possible(uint32_t addr, const uint8_t *data_to_write, size_t size)
{
        int i;
        int same;
        const uint8_t *old;

        if (!get_automode_addr(addr, &old)) {
                ASSERT_WARNING(0);
                return -1;
        }

        ASSERT_WARNING(data_to_write);

        /* Check if new data is same as old one, in which case no write will be needed */
        for (i = 0; i < size && old[i] == data_to_write[i]; ++i) {
        }

        /* This much did not change */
        same = i;

        /* Check if new data can be stored by clearing bits only */
        for (; i < size ; ++i) {
                if ((old[i] & data_to_write[i]) != data_to_write[i])
                        /*
                         * Found byte that needs to have at least one bit set and it was cleared,
                         * erase will be needed.
                         */
                        return -1;
        }
        return same;
}

size_t ad_flash_erase_size(uint32_t addr)
{
        return AD_FLASH_GET_SECTOR_SIZE(addr);
}

void ad_flash_lock(void)
{
#ifdef OS_PRESENT
        OS_MUTEX_GET(flash_mutex, OS_MUTEX_FOREVER);
#endif
}

void ad_flash_unlock(void)
{
#ifdef OS_PRESENT
        OS_MUTEX_PUT(flash_mutex);
#endif
}

void ad_flash_skip_cache_flushing(uint32_t base, uint32_t size)
{
        /* cppcheck-suppress unsignedPositive */
        ASSERT_WARNING((base == AD_FLASH_ALWAYS_FLUSH_CACHE) || IS_REMAPPED_ADDRESS(base));

        no_cache_flush_base = base;
        no_cache_flush_end = base + size;
        if (no_cache_flush_end < no_cache_flush_base)
                no_cache_flush_end = 0;
}

#ifdef OS_PRESENT
ADAPTER_INIT(ad_flash_adapter, ad_flash_init)
#endif

#endif
