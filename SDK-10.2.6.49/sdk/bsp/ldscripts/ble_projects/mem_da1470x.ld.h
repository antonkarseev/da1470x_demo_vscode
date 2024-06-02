/*
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

/*
 * Linker script to configure memory regions.
 */

#define CODE_IS_IN_RAM          (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
#define CODE_IS_IN_FLASH \
        (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) || \
        (dg_configCODE_LOCATION == NON_VOLATILE_IS_OQSPI_FLASH)

/*
 * Interrupt vector remmaping overhead.
 * This is required only for FLASH builds, where the IVT is
 * copied at RAM0 to improve performance.
 * For RAM builds this overhead is not needed, but it still exists so that
 * NMI and HardFault info blocks are always at the same address.
 */
#define IVT_OVERHEAD            0x200

/*
 * Common regions for all cases.
 */
MEMORY
{
        /*
         * Region for initialization stuff.
         * The Interrupt Vector Table is copied at the beginning of this
         * area only when doing XiP from flash.
         * This region has no attribute so that input sections can be put
         * here only explicitly.
         */
        IVT             : ORIGIN = 0x0F000000 + IVT_OVERHEAD, LENGTH = 8K - IVT_OVERHEAD

#ifdef CONFIG_USE_SNC
        /*
         * Region for SNC code/data.
         * This region has no attribute so that input sections can be put
         * here only explicitly.
         */
        SNC             : ORIGIN = 0x20000000, LENGTH = 64K
#endif

        /*
         * Region for exchanging data among the CPUs.
         * This region has no attribute so that input sections can be put
         * here only explicitly.
         */
        SHARED          : ORIGIN = 0x20110000, LENGTH = 128K

#if (dg_configSTORE_VARIABLES_TO_EXTERNAL_RAM == 1)
        /*
         * External RAM region located in PSRAM
         * This region has no attribute so that input sections can be put
         * here only explicitly.
         */
        EXTERNAL_RAM    : ORIGIN = dg_configEXTERNAL_RAM_BASE, LENGTH = dg_configEXTERNAL_RAM_SIZE
#endif
}


/*
 * Special handling for CMAC memories
 */

#if (__RAM9_SIZE_FOR_MAIN_PROC > 0)
#       define CMAC_MEM2_ORG                    (0x20130000 + __RAM9_BASE_FOR_MAIN_PROC)
#       define CMAC_MEM2_LEN                    (__RAM9_SIZE_FOR_MAIN_PROC)

        MEMORY
        {
                /*
                 * Main region of CMAC memory to be allocated to the MAIN PROCESSOR
                 * This region has no attribute so that input sections can be put
                 * here only explicitly.
                 */
                CMAC2           : ORIGIN = CMAC_MEM2_ORG, LENGTH = CMAC_MEM2_LEN
        }
#endif

#if (__RAM10_SIZE_FOR_MAIN_PROC > 0)
#       define CMAC_MEM1_ORG                    (0x20150000 + __RAM10_BASE_FOR_MAIN_PROC)
#       define CMAC_MEM1_LEN                    (__RAM10_SIZE_FOR_MAIN_PROC)

        MEMORY
        {
                /*
                 * Extra region of CMAC memory to be allocated to the MAIN PROCESSOR
                 * (if region CMAC2 is not enough).
                 * This region has no attribute so that input sections can be put
                 * here only explicitly.
                 */
                CMAC            : ORIGIN = CMAC_MEM1_ORG, LENGTH = CMAC_MEM1_LEN
        }
#endif


#if CODE_IS_IN_RAM

        /*
         * Extra regions used in RAM projects.
         */
        MEMORY
        {
                /*
                 * Region mainly for code (connected to AHB CPUC).
                 * It is remapped to 0
                 */
                RAMC (x)        : ORIGIN = 0x00000000, LENGTH = 256K

                /*
                 * Region mainly for data (connected to AHB CPUS)
                 */
                RAMS (rwx)      : ORIGIN = 0x20050000, LENGTH = 768K
        }

#elif CODE_IS_IN_FLASH

#       ifdef CONFIG_USE_SNC
                /* reserve first 64K for SNC */
#               define RAMC_ORG                 0x10010000
#               define RAMS_ORG                 0x20010000
#               define SNC_MEM_LEN              0
#       else
#               define RAMC_ORG                 0x10000000
#               define RAMS_ORG                 0x20000000
#               define SNC_MEM_LEN              64K
#       endif

#       define RAMS_LEN                         (1024K + SNC_MEM_LEN)

        /*
         * Extra regions used in flash projects.
         */
        MEMORY
        {
                /*
                 * Region mainly for code that should be in RAM (connected to AHB CPUC)
                 * This region has no attribute so that input sections can be put
                 * here only explicitly.
                 */
                RAMC            : ORIGIN = RAMC_ORG, LENGTH = 256K

                /*
                 * Region for data (connected to AHB CPUS)
                 */
                RAMS (w)        : ORIGIN = RAMS_ORG, LENGTH = RAMS_LEN

                /*
                 * Region for code and read-only data in FLASH, assuming that
                 * FLASH is remapped to 0.
                 */
                FLASH (rx)      : ORIGIN = 0x00000000, LENGTH = CODE_SIZE
        }

#else

        #error "Unknown code location type..."

#endif


