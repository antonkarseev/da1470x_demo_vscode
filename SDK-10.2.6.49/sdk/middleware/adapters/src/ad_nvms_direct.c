/**
 ****************************************************************************************
 *
 * @file ad_nvms_direct.c
 *
 * @brief NVMS direct access driver implementation
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configNVMS_ADAPTER

#include <string.h>
#include <osal.h>
#include <ad_flash.h>
#ifdef OS_PRESENT
#include <sys_power_mgr.h>
#endif
#include <ad_nvms.h>
#include <ad_nvms_direct.h>
#include <flash_partitions.h>

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef DIRECT_DRIVER_STRATEGY
#define DIRECT_DRIVER_STRATEGY          (DIRECT_DRIVER_STATIC_SECTOR_BUF)
#endif

#ifdef OS_PRESENT
static __RETAINED OS_MUTEX lock;
#endif

static int ad_nvms_direct_read(struct partition_t *part, uint32_t addr, uint8_t *buf,
                                                                                uint32_t size);
static int ad_nvms_direct_write(struct partition_t *part, uint32_t addr, const uint8_t *buf,
                                                                                uint32_t size);
static bool ad_nvms_direct_erase(struct partition_t *part, uint32_t addr, uint32_t size);
static size_t ad_nvms_direct_get_ptr(struct partition_t *part, uint32_t addr, uint32_t size,
                                                                                const void **ptr);
static bool ad_nvms_direct_bind(struct partition_t *part);
static void ad_nvms_direct_flush(struct partition_t *part, bool free_mem);

const partition_driver_t ad_nvms_direct_driver = {
        .bind     = ad_nvms_direct_bind,
        .read     = ad_nvms_direct_read,
        .write    = ad_nvms_direct_write,
        .erase    = ad_nvms_direct_erase,
        .get_ptr  = ad_nvms_direct_get_ptr,
        .get_size = NULL,
        .flush    = ad_nvms_direct_flush,
};

static int ad_nvms_direct_ro_write(struct partition_t *part, uint32_t addr, const uint8_t *buf,
                                                                                uint32_t size);
static bool ad_nvms_direct_ro_erase(struct partition_t *part, uint32_t addr, uint32_t size);

const partition_driver_t ad_nvms_direct_ro_driver = {
        .bind     = ad_nvms_direct_bind,
        .read     = ad_nvms_direct_read,
        .write    = ad_nvms_direct_ro_write,
        .erase    = ad_nvms_direct_ro_erase,
        .get_ptr  = ad_nvms_direct_get_ptr,
        .get_size = NULL,
        .flush    = NULL,
};

void ad_nvms_direct_init(void)
{
#ifdef OS_PRESENT
        if (OS_MUTEX_CREATE_SUCCESS != OS_MUTEX_CREATE(lock)) {
                OS_ASSERT(0);
        }
#endif
}

#if DIRECT_DRIVER_STRATEGY == DIRECT_DRIVER_DYNAMIC_SECTOR_BUF

/*
 * Allocate memory for sector
 */
void *ad_nvms_direct_sector_get(void)
{
        void *p = OS_MALLOC_NORET(AD_FLASH_MAX_SECTOR_SIZE);
        if (p) {
                pm_sleep_mode_request(pm_mode_active);
        }
        return p;
}

/*
 * Release sector memory
 */
void ad_nvms_direct_sector_release(void *p)
{
        if (p) {
                pm_sleep_mode_release(pm_mode_active);
                OS_FREE(p);
        }
}

#elif DIRECT_DRIVER_STRATEGY == DIRECT_DRIVER_STATIC_SECTOR_BUF

static uint8_t flash_sector[AD_FLASH_MAX_SECTOR_SIZE];

void *ad_nvms_direct_sector_get(void)
{
#ifdef OS_PRESENT
        pm_sleep_mode_request(pm_mode_active);
#endif
        return flash_sector;
}

void ad_nvms_direct_sector_release(void *p)
{
#ifdef OS_PRESENT
        pm_sleep_mode_release(pm_mode_active);
#endif
}

#elif DIRECT_DRIVER_STRATEGY == DIRECT_DRIVER_NO_SECTOR_BUF

void *ad_nvms_direct_sector_get(void)
{
        return NULL;
}

void ad_nvms_direct_sector_release(void *p)
{
}

#else
#error Define direct flash driver strategy
#endif

__STATIC_INLINE uint32_t part_addr(const struct partition_t *part, uint32_t addr)
{
        return part->data.start_address + addr;
}

__STATIC_INLINE void part_lock(struct partition_t *part)
{
#ifdef OS_PRESENT
        OS_MUTEX_GET(lock, OS_MUTEX_FOREVER);
#endif
}

__STATIC_INLINE void part_unlock(struct partition_t *part)
{
#ifdef OS_PRESENT
        OS_MUTEX_PUT(lock);
#endif
}

/*
 * Structure and related function to cache flash sector
 */
typedef struct {
        uint32_t flash_address;
        uint8_t *buf;
        bool in_use;
} cached_sector;

static __RETAINED cached_sector sector_buff = {
        .flash_address = 0,
        .buf           = NULL,
        .in_use        = false
};

__STATIC_INLINE int alloc_sector(cached_sector *sec)
{
        if (sec->buf == NULL) {
                sec->buf = ad_nvms_direct_sector_get();
                if (sec->buf == NULL) {
                        return -1;
                }
        }

        return 0;
}

__STATIC_INLINE void dealloc_sector(cached_sector *sec)
{
        if (sec->buf) {
                ad_nvms_direct_sector_release(sec->buf);
        }

        sec->buf = NULL;
}

__STATIC_INLINE void read_sector(cached_sector *sec, uint32_t flash_addr)
{
        ad_flash_read(flash_addr, sec->buf, AD_FLASH_GET_SECTOR_SIZE(flash_addr));
        sec->flash_address = flash_addr;
        sec->in_use = true;
}

__STATIC_INLINE void flush_sector(cached_sector *sec, bool erase_cache)
{
        if (sec->in_use) {
                /* Flush sector from RAM buffer to flash memory*/
                ad_flash_erase_region(sec->flash_address, AD_FLASH_GET_SECTOR_SIZE(sec->flash_address));
                ad_flash_write(sec->flash_address, sec->buf, AD_FLASH_GET_SECTOR_SIZE(sec->flash_address));
                if (erase_cache) {
                        /*
                         * No real erase is needed - in next read cycle the old data
                         * will be overwritten by new. Only set casched_sector as unused.
                         */
                        sec->flash_address = 0;
                        sec->in_use = false;
                }
        }
}

static int ad_nvms_direct_read(struct partition_t *part, uint32_t addr, uint8_t *buf,
                                                                                uint32_t size)
{
        uint32_t read_address = part_addr(part, addr);

        if (addr > part->data.size - 1) {
                size = 0;
        } else if (addr + size > part->data.size ) {
                size = part->data.size - addr;
        }

        if (size == 0) {
                return 0;
        }

        part_lock(part);

        size_t len = ad_flash_read(read_address, buf, size);

        part_unlock(part);

#if dg_configNVMS_FLASH_CACHE
        if (sector_buff.in_use) {
                /* Find common part of buffers */
                uint32_t start = MAX(sector_buff.flash_address, read_address);
                uint32_t end = MIN(sector_buff.flash_address + AD_FLASH_GET_SECTOR_SIZE(sector_buff.flash_address),
                                                                read_address + size);

                if (start < end) {
                        /*
                         * Common part exist. Overwrite data read from flash with data
                         * from cached sector
                         */
                        uint32_t partition_offset = start - read_address;
                        uint32_t cache_offset = start - sector_buff.flash_address;

                        memmove(buf + partition_offset, sector_buff.buf + cache_offset,
                                                                                end - start);
                }
        }
#endif /* dg_configNVMS_FLASH_CACHE */

        return len;
}

static int ad_nvms_direct_write(struct partition_t *part, uint32_t addr, const uint8_t *buf,
                                                                                uint32_t size)
{
        int off;
        uint32_t chunk_size;
        uint32_t sector_start;
        uint32_t sector_offset;
        uint32_t sector_size;
        int written = 0;
        size_t w;

        /* Make sure write is not outside partition */
        if (addr > part->data.size - 1) {
                size = 0;
        } else if (addr + size > part->data.size) {
                size = part->data.size - addr;
        }

        if (size == 0) {
                return 0;
        }


        sector_size = AD_FLASH_GET_SECTOR_SIZE(part->data.start_address);

        part_lock(part);

        while (written < size) {
                sector_start = addr & ~(sector_size - 1);
                sector_offset = addr - sector_start;
                chunk_size = sector_size - sector_offset;

                if (chunk_size > size - written) {
                        chunk_size = size - written;
                }

#if dg_configNVMS_FLASH_CACHE
                if (sector_buff.in_use && part_addr(part, sector_start)
                            == sector_buff.flash_address) {
                        /*
                         * This sector is buffered in RAM - only modify data in RAM
                         * without content checking
                         */
                        memmove(sector_buff.buf + sector_offset, buf, chunk_size);
                        goto advance;
                }

                /* Sector in RAM is no more needed - the current sector is different */
                if (sector_buff.in_use) {
                        flush_sector(&sector_buff, true);
                }
#endif /* dg_configNVMS_FLASH_CACHE */

                off = ad_flash_update_possible(part_addr(part, addr), buf, chunk_size);

                /* No write needed in this sector, same data */
                if (off == (int) chunk_size) {
                        goto advance;
                }

                /* Write without erase possible */
                if (off >= 0) {
                        w = off + ad_flash_write(part_addr(part, addr + off),
                                                                buf + off, chunk_size - off);
                        /* check that the intended content-size is actually written in flash */
                        OS_ASSERT(w == chunk_size);
                        goto advance;
                }

                /* If entire sector is to be written, no need to read old data */
                if (addr == sector_start && chunk_size == sector_size) {
                        ad_flash_erase_region(part_addr(part, sector_start), sector_size);
                        ad_flash_write(part_addr(part, sector_start), buf, sector_size);
                } else {
                        /*
                         * The sector modification is needed. Get this sector to RAM,
                         * modify and keep it to the next write cycle.
                         */

                        /* Alloc sector buffer to read old content to RAM */
                        if (sector_buff.buf == NULL) {
                                if (alloc_sector(&sector_buff) == -1) {
                                        break;
                                }
                        }

                        read_sector(&sector_buff, part_addr(part, sector_start));

                        /* Modify */
                        memmove(sector_buff.buf + addr - sector_start, buf, chunk_size);
#if !dg_configNVMS_FLASH_CACHE
                        /* Erase and write back to flash */
                        flush_sector(&sector_buff, true);
#endif /* dg_configNVMS_FLASH_CACHE */
                }
advance:
                written += chunk_size;
                buf += chunk_size;
                addr += chunk_size;
        }

#if !dg_configNVMS_FLASH_CACHE
        dealloc_sector(&sector_buff);
#endif /* dg_configNVMS_FLASH_CACHE */

        part_unlock(part);

        return written;
}

static bool ad_nvms_direct_erase(struct partition_t *part, uint32_t addr, uint32_t size)
{
        bool result = false;

        /* Make sure write is not outside partition */
        if (addr > part->data.size) {
                size = 0;
        } else if (addr + size > part->data.size) {
                size = part->data.size - addr;
        }

        if (size != 0) {
                result = ad_flash_erase_region(part_addr(part, addr), size);
        }

        return result;
}

static size_t ad_nvms_direct_get_ptr(struct partition_t *part, uint32_t addr, uint32_t size,
                                                                                const void **ptr)
{
        /* Make sure address is not outside partition */
        if (addr > part->data.size - 1) {
                size = 0;
                *ptr = NULL;
        } else {
                if (addr + size > part->data.size) {
                        size = part->data.size - addr;
                }
                *ptr = ad_flash_get_ptr(part_addr(part, addr));
        }
        return size;
}

static int ad_nvms_direct_ro_write(struct partition_t *part, uint32_t addr, const uint8_t *buf,
                                                                                uint32_t size)
{
        return -1;
}

static bool ad_nvms_direct_ro_erase(struct partition_t *part, uint32_t addr, uint32_t size)
{
        return false;
}

static bool ad_nvms_direct_bind(struct partition_t *part)
{
        bool ret = false;

        switch (part->data.type) {
        case NVMS_FIRMWARE_PART:
        case NVMS_PARAM_PART:
        case NVMS_BIN_PART:
        case NVMS_LOG_PART:
                part->driver = &ad_nvms_direct_driver;
                ret = true;
                break;
        case NVMS_PARTITION_TABLE:
                part->driver = &ad_nvms_direct_ro_driver;
                ret = true;
                break;
        default:
                if ((part->data.flags & PARTITION_FLAG_VES) == 0) {
                        if (part->data.flags & PARTITION_FLAG_READ_ONLY) {
                                part->driver = &ad_nvms_direct_ro_driver;
                        } else {
                                part->driver = &ad_nvms_direct_driver;
                        }
                        ret = true;
                        break;
                }
                break;
        }
        return ret;
}

static void ad_nvms_direct_flush(struct partition_t *part, bool free_mem)
{
#if dg_configNVMS_FLASH_CACHE
        flush_sector(&sector_buff, free_mem);

        if (free_mem) {
                dealloc_sector(&sector_buff);
        }
#endif /* dg_configNVMS_FLASH_CACHE */
}

#endif /* dg_configNVMS_ADAPTER */
