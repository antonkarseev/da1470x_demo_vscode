//--------------------------------------------------------------------------
// Project: D/AVE
// File:    dave_memory_da1470x.c (%version: 1 %)
//          created Tue Mar 24 13:29:09 2020 by markus.hertel
//
// Description:
//  %date_modified: Tue Mar 24 13:29:09 2020 %  (%derived_by:  by markus.hertel %)
//
// Changes:
//  2020-06-17 MHe  started
//
// Copyright by TES Electronic Solutions GmbH, www.tes-dst.com. All rights reserved.
//--------------------------------------------------------------------------
//
/* Copyright (c) 2020-2022 Modified by Dialog Semiconductor */

#include <stdlib.h>
#include <string.h>
#include <dave_base.h>
#include <sdk_defs.h>
#include <dave_base_da1470x.h>
#include <dave_d0lib.h>
#include "osal.h"

//--------------------------------------------------------------------------
// Allocate system memory
//
__WEAK void* d1_allocmem(unsigned int size)
{
        unsigned int *ptr = OS_MALLOC((size_t)size + sizeof(unsigned int));
        if (ptr) {
                *ptr = size;
                ++ptr;
        }
        return (void *)ptr;
}

//--------------------------------------------------------------------------
// Release system memory.
//
__WEAK void d1_freemem(void *ptr)
{
        OS_FREE(((unsigned int *)ptr) - 1);
}

//--------------------------------------------------------------------------
// Returns the size of the given memory block
//
__WEAK unsigned int d1_memsize(void *ptr)
{
        return *(((unsigned int *)ptr) - 1);
}


//--------------------------------------------------------------------------
// Allocate video memory
//
__WEAK void* d1_allocvidmem(d1_device *handle, int memtype, unsigned int size)
{
        void *ptr;

        // unused arguments
        (void)handle;

        // feed all requests directly to standard heap
        ptr = OS_MALLOC((size_t)size);

        if (!ptr)
                return NULL;

        // flush possible old data out of cache
        if (!d1_cacheblockflush(handle, memtype, ptr, size)) {
                OS_FREE(ptr);
                return NULL;
        }

        return ptr;
}

//--------------------------------------------------------------------------
// Release video memory
//
__WEAK void d1_freevidmem(d1_device *handle, int memtype, void *ptr)
{
        // unused arguments
        (void)handle;

        // feed all requests directly to standard heap
        OS_FREE(ptr);
}

//--------------------------------------------------------------------------
// Get current memory status
//
__WEAK int d1_queryvidmem(d1_device *handle, int memtype, int query)
{
        return 0;
}

//--------------------------------------------------------------------------
// Return hints about systems memory architecture
//
__WEAK int d1_queryarchitecture(d1_device *handle)
{
        // unused arguments
        (void)handle;

        return d1_ma_mapped;
}

//--------------------------------------------------------------------------
// Map video memory for direct CPU access
//
__WEAK void* d1_mapvidmem(d1_device *handle, void *ptr, int flags)
{
        // unused arguments
        (void)handle;
        (void)flags;

        // map memory into uncached area (test)
        return ptr;
}

//--------------------------------------------------------------------------
// Release memory mapping
//
__WEAK int d1_unmapvidmem(d1_device *handle, void *ptr)
{
        // unused arguments
        (void)handle;
        (void)ptr;

        // no unmapping necessary
        return 1;
}

//--------------------------------------------------------------------------
// Map CPU accessible address of a video memory block back to video memory address
//
__WEAK void* d1_maptovidmem(d1_device *handle, void *ptr)
{
        uint32_t phy_addr = black_orca_phy_addr((uint32_t)ptr);

        // unused arguments
        (void)handle;

        if (IS_OQSPIC_ADDRESS(phy_addr)) {
                phy_addr += MEMORY_OQSPIC_S_BASE - MEMORY_OQSPIC_BASE;
        }

        return (void*)phy_addr;
}

//--------------------------------------------------------------------------
// Map already allocated video memory address to an address for direct CPU access
//
__WEAK void* d1_mapfromvidmem(d1_device *handle, void *ptr)
{
        // unused arguments
        (void)handle;

        // map memory into uncached area (test)
        return ptr;
}

//--------------------------------------------------------------------------
// Copy data to video memory
//
__WEAK int d1_copytovidmem(d1_device *handle, void *dst, const void *src, unsigned int size, int flags)
{
        // unused arguments
        (void)handle;
        (void)flags;

        // can use direct memcpy
        OPT_MEMCPY(dst, src, size);

        return 1;
}

//--------------------------------------------------------------------------
// Copy data from video memory
//
__WEAK int d1_copyfromvidmem(d1_device *handle, void *dst, const void *src, unsigned int size, int flags)
{
        // unused arguments
        (void)handle;
        (void)flags;

        // can use direct memcpy
        OPT_MEMCPY(dst, src, size);

        return 1;
}

//--------------------------------------------------------------------------
// Flush CPU data caches
//
__WEAK int d1_cacheflush(d1_device *handle, int memtype)
{
        // unused arguments
        (void)handle;
        (void)memtype;

        return 1;
}

//--------------------------------------------------------------------------
// Flush part of CPU data caches
//
__WEAK int d1_cacheblockflush(d1_device *handle, int memtype, const void *ptr, unsigned int size)
{
        // unused arguments
        (void)handle;
        (void)memtype;
        (void)ptr;
        (void)size;

        return 1;
}
