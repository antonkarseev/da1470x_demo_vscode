/**
 * \addtogroup BSP_MEMORY_DEFAULTS
 * \{
 *
 * \addtogroup MEMORY_LAYOUT_SETTINGS Memory Layout Configuration Settings
 *
 * \brief Memory Layout Configuration Settings
 * \{
 *
 */
/**
 ****************************************************************************************
 *
 * @file bsp_memory_defaults_da1470x.h
 *
 * @brief Board Support Package. Device-specific system configuration default values.
 *
 * Copyright (C) 2020-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef BSP_MEMORY_DEFAULTS_DA1470X_H_
#define BSP_MEMORY_DEFAULTS_DA1470X_H_



#define __RAM9_SIZE                                     (0x20000)
#define __RAM10_SIZE                                    (0x30000)


#ifdef CONFIG_USE_BLE

#include "../../interfaces/ble/stack/da14700/include/cmac_memory.h"

# define CMAC_AREA_BYTES \
        (CMAC_AREA_SIZE > 0 ?  (CMAC_AREA_SIZE * 1024) :  0)

#else

# define CMAC_AREA_BYTES                                (0)

#endif  /* CONFIG_USE_BLE */

#if (CMAC_AREA_BYTES > __RAM10_SIZE)

# define __RAM9_AVAILABLE_BYTES \
        ((CMAC_AREA_BYTES - __RAM10_SIZE) > __RAM9_SIZE \
                ?  0  : __RAM9_SIZE - (CMAC_AREA_BYTES - __RAM10_SIZE))
# define __RAM10_AVAILABLE_BYTES                        (0)

#else

# define __RAM9_AVAILABLE_BYTES                         __RAM9_SIZE
# define __RAM10_AVAILABLE_BYTES                        (__RAM10_SIZE - CMAC_AREA_BYTES)

#endif


#ifdef dg_config_RETAINED_UNINIT_SECTION_SIZE
#error "dg_config_RETAINED_UNINIT_SECTION_SIZE is deprecated! "\
       "Please use dg_configRETAINED_UNINIT_SECTION_SIZE instead (no underscore)!"
#endif

/**
 * \brief Size of the RETAINED_RAM_UNINIT section, in bytes.
 *
 * This section is not initialized during startup by either the bootloader or
 * the application. It can be therefore used to maintain debug or other relevant
 * information that will not be lost after reset. It should be guaranteed that
 * both the bootloader (if any) and the application are using the same value for
 * this option (or otherwise the booloader can corrupt the contents of the section).
 * To use this section for a specific variable, use the __RETAINED_UNINIT attribute.
 */
#ifndef dg_configRETAINED_UNINIT_SECTION_SIZE
#define dg_configRETAINED_UNINIT_SECTION_SIZE           (128)
#endif

/**
 * \brief Code size in QSPI projects for DA1470x.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_CODE_SIZE_AA
#define dg_configQSPI_CODE_SIZE_AA                      (384 * 1024) /* Includes CMI and SNC */
#endif

/**
 * \brief Maximum size (in bytes) of image in the QSPI flash.
 *
 * The image in the QSPI flash contains the text (code + const data) and any other initialized data.
 *
 * \note This size should not be larger than the flash partition where the image is stored.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_MAX_IMAGE_SIZE
#define dg_configQSPI_MAX_IMAGE_SIZE                    ( IMAGE_PARTITION_SIZE )
#endif

#if dg_configQSPI_MAX_IMAGE_SIZE < dg_configQSPI_CODE_SIZE_AA
#error "dg_configQSPI_MAX_IMAGE_SIZE cannot be smaller than dg_configQSPI_CODE_SIZE_AA"
#endif

/**
 * \brief RAM-block size in cached mode for DA1470x.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_CACHED_RAM_SIZE_AA
#define dg_configQSPI_CACHED_RAM_SIZE_AA                (384 * 1024)
#endif

/**
 * \brief Code and RAM size in RAM projects for DA1470x.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configRAM_CODE_SIZE_AA
/*
 * We prefer to fit the code in RAM3 (256KiB), because it goes through AHB CPUC
 */
#define dg_configRAM_CODE_SIZE_AA                       (256 * 1024)
#endif

/**
 * \brief Retention memory configuration.
 *
 * 28 bits field; each couple of bits controls how the relevant memory block will behave when PD_MEM is UP or DOWN.
 * -  bits 0-1   : SYSRAM0
 * -  bits 2-3   : SYSRAM1
 * -  bits 4-5   : SYSRAM2
 * -  bits 6-7   : SYSRAM3
 * -  bits 8-9   : SYSRAM4
 * -  bits 10-11 : SYSRAM5
 * -  bits 11-13 : SYSRAM6
 * -  bits 14-15 : SYSRAM7
 * -  bits 16-17 : SYSRAM8
 * -  bits 18-19 : SYSRAM9
 * -  bits 20-21 : SYSRAM10
 * -  bits 22-23 : SYSRAM11
 * -  bits 24-25 : SYSRAM12
 * -  bits 26-27 : SYSRAM13
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 */
#ifndef dg_configMEM_RETENTION_MODE
#define dg_configMEM_RETENTION_MODE                     (0)
#endif

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_OQSPI_FLASH)
# if (dg_configEXEC_MODE == MODE_IS_CACHED)
#  define CODE_SIZE                                     dg_configQSPI_CODE_SIZE_AA
#  define RAM_SIZE                                      dg_configQSPI_CACHED_RAM_SIZE_AA
# else // MIRRORED
#  error "OQSPI mirrored mode is not supported!"
# endif
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
# if (dg_configEXEC_MODE == MODE_IS_CACHED)
#  pragma message "RAM cached mode is not supported! Resetting to RAM (mirrored) mode!"
#  undef dg_configEXEC_MODE
#  define dg_configEXEC_MODE                            MODE_IS_RAM
# endif
# define CODE_SIZE                                      dg_configRAM_CODE_SIZE_AA
/*
 * All code (and data) should fit in RAM3-RAM7 cells (1MiB)
 */
#define MAX_CODE_SIZE                                   (1024 * 1024)
# if (CODE_SIZE) > MAX_CODE_SIZE
#  error "The used CODE_SIZE value exceeds the available amount of RAM!"
# endif
#else
# error "Unknown configuration..."
#endif /* dg_configCODE_LOCATION */

/**
 * \brief Output binary can be loaded by SEGGER FLASH Loader.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 */
#ifndef dg_configUSE_SEGGER_FLASH_LOADER
# define dg_configUSE_SEGGER_FLASH_LOADER               (0)
#endif

#if (dg_configUSE_SEGGER_FLASH_LOADER == 1)
# if dg_configCODE_LOCATION != NON_VOLATILE_IS_OQSPI_FLASH || dg_configEXEC_MODE != MODE_IS_CACHED
#  error "dg_configUSE_SEGGER_FLASH_LOADER can be used only for cached OQSPI build!"
# endif

# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_OQSPI_FLASH) && (dg_configOQSPI_FLASH_AUTODETECT == 1)
# error "Segger Flash Loader cannot make use of the auto-detection mechanism for OQSPI devices"
# endif
#endif

/**
 * Enable storing variables to external RAM (PSRAM)
 *
 * User must provide:
 *     dg_configEXTERNAL_RAM_BASE: External RAM region base address
 *     dg_configEXTERNAL_RAM_SIZE: External RAM region size
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configSTORE_VARIABLES_TO_EXTERNAL_RAM
# define dg_configSTORE_VARIABLES_TO_EXTERNAL_RAM       (0)
#endif

/**
 * Enable (full or partial) use of RAM9 for MAIN PROCESSOR storage, instead of
 * leaving it up for CMAC usage only.
 *
 * \note RAM9 is clocked with a 32MHz clock and the MAIN PROCESSOR has access to
 *       it via a bridge, so access times to RAM9 are much slower than to RAM
 *       cells RAM1-RAM8. Moreover, CMAC has absolute access priority to RAM9
 *       (i.e. the MAIN PROCESSOR may be stalled so that CMAC gets zero-wait-state
 *       access to RAM9).
 *
 * The boundaries of the area to be used by the MAIN PROCESSOR are defined with:
 *
 *     dg_configUSE_CMAC_RAM9_BASE: offset in RAM9 where the area allocated to the
 *                                  MAIN PROCESSOR starts (valid only if
 *                                  dg_configUSE_CMAC_RAM9_SIZE > 0)
 *     dg_configUSE_CMAC_RAM9_SIZE: size (in bytes, up to 128KiB) of the area of
 *                                  RAM9 that is allocated to the MAIN PROCESSOR
 *                                  The special value of -1 is interpreted as
 *                                  "allocate all space that is not used by
 *                                  CMAC". In that case, the value of
 *                                  dg_configUSE_CMAC_RAM9_BASE is not
 *                                  effective.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configUSE_CMAC_RAM9_BASE
# define dg_configUSE_CMAC_RAM9_BASE                    (0)
#endif

#ifndef dg_configUSE_CMAC_RAM9_SIZE
# define dg_configUSE_CMAC_RAM9_SIZE                    (0)
#endif

#if (dg_configUSE_CMAC_RAM9_SIZE == -1)

# define __RAM9_BASE_FOR_MAIN_PROC                      (__RAM9_SIZE - __RAM9_AVAILABLE_BYTES)
# define __RAM9_SIZE_FOR_MAIN_PROC                      __RAM9_AVAILABLE_BYTES

#else

# if (dg_configUSE_CMAC_RAM9_BASE < 0  ||  dg_configUSE_CMAC_RAM9_BASE >= __RAM9_SIZE)
#   error "dg_configUSE_CMAC_RAM9_BASE can take values only in the range [0, 0x20000)"
# endif

# if (dg_configUSE_CMAC_RAM9_SIZE < 0  ||  dg_configUSE_CMAC_RAM9_SIZE > __RAM9_SIZE)
#   error "dg_configUSE_CMAC_RAM9_SIZE can take values only in the range [0, 0x20000]"
# endif

# if (dg_configUSE_CMAC_RAM9_SIZE > 0) && \
        (dg_configUSE_CMAC_RAM9_BASE + dg_configUSE_CMAC_RAM9_SIZE > __RAM9_SIZE)
#   error "dg_configUSE_CMAC_RAM9_BASE and dg_configUSE_CMAC_RAM9_SIZE define an area that overflows RAM9"
# endif


# ifdef CONFIG_USE_BLE

#   if (CMAC_AREA_BYTES > __RAM10_SIZE)

/* CMAC uses all of RAM10 and (a part of) RAM9 */

#     if (dg_configUSE_CMAC_RAM9_SIZE > 0) && (dg_configUSE_CMAC_RAM9_BASE < (CMAC_AREA_BYTES - __RAM10_SIZE))
#       error "dg_configUSE_CMAC_RAM9_BASE overlaps with RAM used by CMAC"
#     endif

#   endif  /* CMAC_AREA_BYTES */

# endif  /* CONFIG_USE_BLE */


# define __RAM9_BASE_FOR_MAIN_PROC                      dg_configUSE_CMAC_RAM9_BASE
# define __RAM9_SIZE_FOR_MAIN_PROC                      dg_configUSE_CMAC_RAM9_SIZE

#endif

/**
 * Enable (full or partial) use of RAM10 for MAIN PROCESSOR storage, instead of
 * leaving it up for CMAC usage only.
 *
 * \note RAM10 is clocked with a 32MHz clock and the MAIN PROCESSOR has access to
 *       it via a bridge, so access times to RAM10 are much slower than to RAM
 *       cells RAM1-RAM8. Moreover, CMAC has absolute access priority to RAM10
 *       (i.e. the MAIN PROCESSOR may be stalled so that CMAC gets zero-wait-state
 *       access to RAM10).
 *
 * The boundaries of the area to be used by the MAIN PROCESSOR are defined with:
 *
 *     dg_configUSE_CMAC_RAM10_BASE: offset in RAM10 where the area allocated to the
 *                                   MAIN PROCESSOR starts (valid only if
 *                                   dg_configUSE_CMAC_RAM10_SIZE > 0)
 *     dg_configUSE_CMAC_RAM10_SIZE: size (in bytes, up to 192KiB) of the area of
 *                                   RAM10 that is allocated to the MAIN PROCESSOR
 *                                   The special value of -1 is interpreted as
 *                                   "allocate all space that is not used by
 *                                   CMAC". In that case, the value of
 *                                   dg_configUSE_CMAC_RAM10_BASE is not
 *                                   effective.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */

#if (dg_configRF_ENABLE_RECALIBRATION == 1) || (dg_configUSE_SYS_TRNG == 1) || (dg_configUSE_SYS_DRBG == 1)
#ifndef dg_configUSE_CMAC_RAM10_SIZE
#define dg_configUSE_CMAC_RAM10_SIZE                    (-1)
#endif
#endif

#ifndef dg_configUSE_CMAC_RAM10_BASE
# define dg_configUSE_CMAC_RAM10_BASE                   (0)
#endif

#ifndef dg_configUSE_CMAC_RAM10_SIZE
# define dg_configUSE_CMAC_RAM10_SIZE                   (0)
#endif

#if (dg_configUSE_CMAC_RAM10_SIZE == -1)

# define __RAM10_BASE_FOR_MAIN_PROC                     (__RAM10_SIZE - __RAM10_AVAILABLE_BYTES)
# define __RAM10_SIZE_FOR_MAIN_PROC                     __RAM10_AVAILABLE_BYTES

#else

# if (dg_configUSE_CMAC_RAM10_BASE < 0  ||  dg_configUSE_CMAC_RAM10_BASE >= __RAM10_SIZE)
#   error "dg_configUSE_CMAC_RAM10_BASE can take values only in the range [0, 0x30000)"
# endif

# if (dg_configUSE_CMAC_RAM10_SIZE < 0  ||  dg_configUSE_CMAC_RAM10_SIZE > __RAM10_SIZE)
#   error "dg_configUSE_CMAC_RAM10_SIZE can take values only in the range [0, 0x30000]"
# endif

# if (dg_configUSE_CMAC_RAM10_SIZE > 0) && \
        (dg_configUSE_CMAC_RAM10_BASE + dg_configUSE_CMAC_RAM10_SIZE > __RAM10_SIZE)
#   error "dg_configUSE_CMAC_RAM10_BASE and dg_configUSE_CMAC_RAM10_SIZE define an area that overflows RAM10"
# endif

# ifdef CONFIG_USE_BLE

#   if (dg_configUSE_CMAC_RAM10_SIZE > 0) && (dg_configUSE_CMAC_RAM10_BASE < CMAC_AREA_BYTES)
#     error "dg_configUSE_CMAC_RAM10_BASE overlaps with RAM used by CMAC"
#   endif

# endif

# define __RAM10_BASE_FOR_MAIN_PROC                     dg_configUSE_CMAC_RAM10_BASE
# define __RAM10_SIZE_FOR_MAIN_PROC                     dg_configUSE_CMAC_RAM10_SIZE

#endif

/* fall-back values for linker scripts */
#ifndef __RAM9_SIZE_FOR_MAIN_PROC
#define __RAM9_SIZE_FOR_MAIN_PROC                       (0)
#endif
#ifndef __RAM10_SIZE_FOR_MAIN_PROC
#define __RAM10_SIZE_FOR_MAIN_PROC                      (0)
#endif

#if (dg_configSTORE_VARIABLES_TO_EXTERNAL_RAM == 1)

# if (dg_configUSE_HW_QSPI2 != 1)
#  error "dg_configSTORE_VARIABLES_TO_EXTERNAL_RAM can be used only when a PSRAM is present!"
# endif

# ifndef dg_configEXTERNAL_RAM_BASE
#   error "dg_configEXTERNAL_RAM_BASE is missing. Please provide external RAM region base address!"
# endif

# ifndef dg_configEXTERNAL_RAM_SIZE
#   error "dg_configEXTERNAL_RAM_SIZE is missing.  Please provide external RAM region size!"
# endif

#endif /* dg_configSTORE_VARIABLES_TO_EXTERNAL_RAM == 1 */


#endif /* BSP_MEMORY_DEFAULTS_DA1470X_H_ */

/**
 * \}
 * \}
 */
