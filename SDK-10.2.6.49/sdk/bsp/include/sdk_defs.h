/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup BSP_DEFINITIONS SDK Platform Definitions
 *
 * \brief Platform definitions
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sdk_defs.h
 *
 * @brief Central include header file with platform definitions.
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef __SDK_DEFS_H__
#define __SDK_DEFS_H__

#include <stddef.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef __GNUC__
#  define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)
/* assert gcc version is at least 4.9.3 */
#  if GCC_VERSION < 40903
#    error "Please use gcc version 4.9.3 or newer!"
#  endif
#endif

#     include "DA1470x-00.h"



#include "system_DA1470x.h"                         /* DA1470x System                                                      */

/************************
 * Memory map
 ************************/

/**
 * \brief Remapped device base address.
 */
#define MEMORY_REMAPPED_BASE    0x00000000UL
#define MEMORY_REMAPPED_END     0x00800000UL

/**
 * \brief Remapped device memory size.
 */
#define MEMORY_REMAPPED_SIZE    (MEMORY_REMAPPED_END - MEMORY_REMAPPED_BASE)

/**
 * \brief ROM base address.
 */
#define MEMORY_ROM_BASE         0x0F020000UL
#define MEMORY_ROM_END          (MEMORY_ROM_BASE + 0x10000UL)

/**
 * \brief ROM memory size.
 */
#define MEMORY_ROM_SIZE         (MEMORY_ROM_END - MEMORY_ROM_BASE)

/**
 * \brief OTP memory base address.
 */
#define MEMORY_OTP_BASE         0x10080000UL
#define MEMORY_OTP_END          0x10090000UL
 /* Peripheral remap */
#define MEMORY_OTP_BASE_P       0x30080000UL
#define MEMORY_OTP_END_P        0x30090000UL

/**
 * \brief OTP memory size.
 */
#define MEMORY_OTP_SIZE         (MEMORY_OTP_END - MEMORY_OTP_BASE)

/**
 * \brief SYSTEM RAM base address.
 */
#define MEMORY_SYSRAM_BASE      (0x20000000UL)
#define MEMORY_SYSRAM3_BASE     (0x20010000UL)
#define MEMORY_SYSRAM8_BASE     (0x20110000UL)
#define MEMORY_SYSRAM9_BASE     (0x20130000UL)
#define MEMORY_SYSRAM10_BASE    (0x20150000UL)
#define MEMORY_SYSRAM11_BASE    (0x20180000UL)
#define MEMORY_SYSRAM_END       (0x20180000UL)

/**
 * \brief SYSTEM RAM size.
 */
#define MEMORY_SYSRAM_SIZE      (MEMORY_SYSRAM_END - MEMORY_SYSRAM_BASE)

/**
 * \brief CACHE RAM base address.
 */
#define MEMORY_CACHERAM_BASE    0x10068000UL
#define MEMORY_CACHERAM_END     0x1006A000UL

/**
 * \brief CACHE RAM size.
 */
#define MEMORY_CACHERAM_SIZE    (MEMORY_CACHERAM_END - MEMORY_CACHERAM_BASE)

/**
 * \brief QSPI Flash base address.
 */

 /*
  * OQSPIC (octa/quad-SPI) AHB-C(ode) bus (cached). Accesses through this bus
  * are restricted according to CACHE_FLASH_REG.
  */
#define MEMORY_OQSPIC_BASE      (0x18000000UL)
#define MEMORY_OQSPIC_END       (MEMORY_OQSPIC_BASE + 0x8000000UL)
 /*
  * OQSPIC (octa-SPI) AHB-S(ystem) bus (Not cached). Accesses through this bus
  * are not affected by CACHE_FLASH_REG.
  */
#define MEMORY_OQSPIC_S_BASE    (0x38000000UL)
#define MEMORY_OQSPIC_S_END     (MEMORY_OQSPIC_S_BASE + 0x8000000UL)

/*
 * QSPIC (typically for a second external flash, not cached, non-XiP)
 */
#define MEMORY_QSPIC_BASE       (0x48000000UL)
#define MEMORY_QSPIC_END        (MEMORY_QSPIC_BASE + 0x8000000UL)

/*
 * QSPIC2 (typically for an external PSRAM, cached through DCACHE)
 */
#define MEMORY_QSPIC2_BASE       (0x28000000UL)
#define MEMORY_QSPIC2_END        (MEMORY_QSPIC2_BASE + 0x8000000UL)


/*
* DCACHE controller
*/
#define MEMORY_DCACHE_BASE       (0x30104000UL)
#define MEMORY_DCACHE_SIZE       (0x4000UL)
#define MEMORY_DCACHE_END        (MEMORY_DCACHE_BASE + MEMORY_DCACHE_SIZE)


/**
 * \brief OQSPIC memory size.
 */
#define MEMORY_OQSPIC_SIZE      (MEMORY_OQSPIC_S_END - MEMORY_OQSPIC_S_BASE)

/**
 * \brief QSPIC memory size.
 */

#define MEMORY_QSPIC_SIZE       (MEMORY_QSPIC_END - MEMORY_QSPIC_BASE)

/**
 * \brief QSPIC2 memory size.
 */
#define MEMORY_QSPIC2_SIZE      (MEMORY_QSPIC2_END - MEMORY_QSPIC2_BASE)

/*
 * Primary product header location
 */
#if defined(NVMS_PRODUCT_HEADER_PART_START)
/*
 * Product header location according to the partition table definitions
 */
#define PRIMARY_PRODUCT_HEADER_BASE     (NVMS_PRODUCT_HEADER_PART_START)
#else
 /*
  * User-defined location of the product headers.
  */
#define PRIMARY_PRODUCT_HEADER_BASE     (0x0000)
#endif

/*
 * Backup product header location
 */
#define BACKUP_PRODUCT_HEADER_BASE      (PRIMARY_PRODUCT_HEADER_BASE + 0x1000)


/**
 * \brief TRNG FIFO address
 */



/**
 * \brief OTP User Data Encryption Keys memory base address
 */
#define MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_START         0x00000A00UL
#define MEMORY_OTP_USER_DATA_KEYS_PAYLOAD_END           0x00000B00UL
#define MEMORY_OTP_USER_DATA_KEYS_INDEX_START           0x000009C0UL
#define MEMORY_OTP_USER_DATA_KEYS_INDEX_END             0x000009E0UL


#define WITHIN_RANGE(_a, _s, _e)        (((uint32_t)(_a) >= (uint32_t)(_s)) && ((uint32_t)(_a) < (uint32_t)(_e)))

/**
 * \brief Address is in the remapped memory region
 */
#define IS_REMAPPED_ADDRESS(_a)         WITHIN_RANGE((_a), MEMORY_REMAPPED_BASE, MEMORY_REMAPPED_END)

/**
 * \brief Address is in the ROM region
 */
#define IS_ROM_ADDRESS(_a)              WITHIN_RANGE((_a), MEMORY_ROM_BASE, MEMORY_ROM_END)

/**
 * \brief Address is in the OTP memory region
 */
#define IS_OTP_ADDRESS(_a)              WITHIN_RANGE((_a), MEMORY_OTP_BASE, MEMORY_OTP_END) || \
                                        WITHIN_RANGE((_a), MEMORY_OTP_BASE_P, MEMORY_OTP_END_P )
/**
 * \brief Address is in the SYSTEM RAM region
 */
#define IS_SYSRAM_ADDRESS(_a)           WITHIN_RANGE((_a), MEMORY_SYSRAM_BASE, MEMORY_SYSRAM_END)

/**
 * \brief Address is in the CACHE RAM region
 */
#define IS_CACHERAM_ADDRESS(_a)         WITHIN_RANGE((_a), MEMORY_CACHERAM_BASE, MEMORY_CACHERAM_END)



/**
 * \brief Address is in the octa-SPI Flash memory region
 */
#define IS_OQSPIC_ADDRESS(_a)           WITHIN_RANGE((_a), MEMORY_OQSPIC_BASE, MEMORY_OQSPIC_END)

/**
* \brief Address is in the octa-SPI AHB-S(ystem) memory region
*/
#define IS_OQSPIC_S_ADDRESS(_a)         WITHIN_RANGE((_a), MEMORY_OQSPIC_S_BASE, MEMORY_OQSPIC_S_END)

/**
* \brief Address is in the QSPI AHB-S(ystem) memory region
*/
#define IS_QSPIC_ADDRESS(_a)            WITHIN_RANGE((_a), MEMORY_QSPIC_BASE, MEMORY_QSPIC_END)

/**
* \brief Address is in the QSPI2 AHB-S(ystem) memory region
*/
#define IS_QSPIC2_ADDRESS(_a)           WITHIN_RANGE((_a), MEMORY_QSPIC2_BASE, MEMORY_QSPIC2_END)

/**
 * \brief Address is in OQSPI_MEM1 virtual memory region
 */
#define IS_OQSPI_MEM1_VIRTUAL_ADDRESS(_a)       WITHIN_RANGE((_a), OQSPI_MEM1_VIRTUAL_BASE_ADDR,                        \
                                                                  (OQSPI_MEM1_VIRTUAL_BASE_ADDR + MEMORY_OQSPIC_SIZE))
/**
 * \brief Address is in QSPI_MEM1 virtual memory region
 */
#define IS_QSPI_MEM1_VIRTUAL_ADDRESS(_a)        WITHIN_RANGE((_a), QSPI_MEM1_VIRTUAL_BASE_ADDR,                         \
                                                                  (QSPI_MEM1_VIRTUAL_BASE_ADDR + MEMORY_QSPIC_SIZE))
/**
 * \brief Address is in QSPI_MEM2 virtual memory region
 */
#define IS_QSPI_MEM2_VIRTUAL_ADDRESS(_a)        WITHIN_RANGE((_a), QSPI_MEM2_VIRTUAL_BASE_ADDR,                         \
                                                                  (QSPI_MEM2_VIRTUAL_BASE_ADDR + MEMORY_QSPIC2_SIZE))
/**
 * \brief Check if the current build has cached flash configuration enabled
 */
#define IS_CACHED_FLASH                         ((dg_configCODE_LOCATION == NON_VOLATILE_IS_OQSPI_FLASH) && \
                                                 (dg_configEXEC_MODE == MODE_IS_CACHED))

 /**
 * \brief The Sector Size of the OQSPI and QSPI flash memories
 */
 #define FLASH_SECTOR_SIZE              (0x1000)


 /**
  * \brief The base address for accessing the Flash memory connected to OQSPI
  *
  * The base address is used in oqspi_automode. Automode is using a single zero-based address region
  * for accessing the Flash devices connected to OQSPI and both QSPI controllers (OQSPIC, QSPIC and QSPIC2).
  * The defined address sub-regions are:
  *    Address region 1: OQSPI_MEM1_VIRTUAL_BASE_ADDR..QSPI_MEM1_VIRTUAL_BASE_ADDR-1
  *    Address region 2: QSPI_MEM1_VIRTUAL_BASE_ADDR..QSPI_MEM2_VIRTUAL_BASE_ADDR-1
  *    Address region 3: starting at QSPI_MEM2_VIRTUAL_BASE_ADDR
  * When Flash address is in region 1 then the device connected to OQSPIC is accessed.
  * When Flash address is in region 2 then the device connected to QSPIC is accessed.
  * When Flash address is in region 3 then the device connected to QSPIC2 is accessed.
  * The maximum region size handled by each QSPI controller in automode is 128MBytes.
  * The default value of each region size is 0x8000000, allowing 128MBytes region
  * for each controller.
  */
 #ifndef OQSPI_MEM1_VIRTUAL_BASE_ADDR
 #define OQSPI_MEM1_VIRTUAL_BASE_ADDR                (0x00000000)
 #endif

 /**
  * \brief The base address for accessing the Flash memory connected to QSPIC
  *
  * The base address is used in qspi_automode.
  */
 #ifndef QSPI_MEM1_VIRTUAL_BASE_ADDR
 #define QSPI_MEM1_VIRTUAL_BASE_ADDR                 (OQSPI_MEM1_VIRTUAL_BASE_ADDR + MEMORY_OQSPIC_SIZE)
 #endif

 /**
  * \brief The base address for accessing the Flash memory connected to QSPIC2
  *
  * The base address is used in qspi_automode.
  */
 #ifndef QSPI_MEM2_VIRTUAL_BASE_ADDR
 #define QSPI_MEM2_VIRTUAL_BASE_ADDR                 (QSPI_MEM1_VIRTUAL_BASE_ADDR + MEMORY_QSPIC_SIZE)
 #endif


/**
 * \brief Zero-initialized data retained memory attribute
 */
#define __RETAINED                      __attribute__((section("retention_mem_zi")))    // RetRAM0
#define __RETAINED_1                    __attribute__((section("retention_mem_1_zi")))  // RetRAM1

/**
 * \brief Initialized data retained memory attribute
 */
#define __RETAINED_RW                   __attribute__((section("retention_mem_init")))

/**
 * \brief Uninitialized data retained memory attribute. Used for variables that should
 * not be initialized during startup.
 */
#define __RETAINED_UNINIT               __attribute__((section("retention_mem_uninit")))

 /**
  * \brief Constant data retained memory attribute
  */
#define __RETAINED_CONST_INIT           __attribute__((section("retention_mem_const")))

/**
 * \brief Zero-initialized data retained memory attribute visible by CMAC and SNC
 */
#define __RETAINED_SHARED               __attribute__((section("retention_mem_shared_zi")))

/**
 * \brief Text retained memory attribute
 */
# if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_OQSPI_FLASH) && (dg_configEXEC_MODE == MODE_IS_CACHED))
# define __RETAINED_CODE                __attribute__((section("text_retained"))) __attribute__((noinline)) __attribute__((optimize ("no-tree-switch-conversion")))
# else
# define __RETAINED_CODE
# endif

# if dg_configRETAIN_HOT_CODE
# define __RETAINED_HOT_CODE             __RETAINED_CODE
# else
# define __RETAINED_HOT_CODE
# endif


# if (dg_configSTORE_VARIABLES_TO_EXTERNAL_RAM == 1)
/**
  * \brief Uninitialized data external memory attribute.
  *        Used for uninitialized variables which should be placed in external RAM (PSRAM).
  */
# define __EXTERNAL_MEM_UNINIT          __attribute__((section("external_mem_uninit")))
#endif

#if (__RAM9_SIZE_FOR_MAIN_PROC > 0)
/**
 * \brief Attribute for data to be placed in RAM9
 * (which has slower access times for the MAIN PROCESSOR)
 */
# define __IN_CMAC_MEM2                 __attribute__((section("m33_data_in_ram9")))

/**
 * \brief Attribute for uninitialized data to be placed in RAM9
 * (which has slower access times for the MAIN PROCESSOR)
 */
# define __IN_CMAC_MEM2_UNINIT          __attribute__((section("m33_uninit_data_in_ram9")))
#endif

#if (__RAM10_SIZE_FOR_MAIN_PROC > 0)
/**
 * \brief Attribute for data to be placed in RAM10
 * (which has slower access times for the MAIN PROCESSOR)
 */
# define __IN_CMAC_MEM1                 __attribute__((section("m33_data_in_ram10")))

/**
 * \brief Attribute for uninitialized data to be placed in RAM10
 * (which has slower access times for the MAIN PROCESSOR)
 */
# define __IN_CMAC_MEM1_UNINIT          __attribute__((section("m33_uninit_data_in_ram10")))
#endif



/**
 * \brief Attribute to silence warnings about unused parameters/variables/function
 */
#define __UNUSED                __attribute__((unused))

/**
 * \brief Attribute to tell the compiler to consider a symbol as externally visible (for LTO)
 */
#define __LTO_EXT               __attribute__((externally_visible))

 /**
  * \brief Macro to silence warnings about unused function arguments
  *
  * \param[in] x         The variable name
  */
#define UNUSED_ARG(x)          (void)(x)

typedef void (*assertion_func_t)(void* args);
extern assertion_func_t assert_warning_func;
extern assertion_func_t assert_error_func;
void assertion_functions_set_to_uninit(void);
void assertion_functions_set_to_init(void);

/* Push registers on stack */
#define __PUSH_SCRATCH_REGISTERS()                                                                      \
        do {                                                                                            \
                __ASM volatile (                                                                        \
                        "  .syntax unified              \n"     /* Prevent non-unified syntax */        \
                        "   push {r0, r1, r2, r3}       \n"     /* Push scratch registers on stack. */  \
                        :                                       /* Output operands: None */             \
                        :                                       /* Input operands: None */              \
                        :                                       /* Clobbers: None */                    \
                );                                                                                      \
        } while (0)

/* Get the value of the stack pointer */
#define __get_SP()                                                                              \
({                                                                                              \
  uint32_t ret;                                                                                 \
                                                                                                \
  __ASM volatile (                                                                              \
          "  .syntax unified    \n"     /* Prevent non-unified syntax */                        \
          "   mov %[ret], sp    \n"     /* Move SP value on a general purpose register . */     \
          :   [ret]"=r" (ret)           /* Output operands: Store the SP value on ret */        \
          :                             /* Input operands: None */                              \
          :                             /* Clobbers: None */                                    \
  );                                                                                            \
  ret;                                                                                          \
})

/**
 * \brief Assert as warning macro
 *
 * \note Active only while in development mode
 */
 #define ASSERT_WARNING(a) if (!(a)) {                                                          \
                                 if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {                \
                                         __PUSH_SCRATCH_REGISTERS();                            \
                                         assert_warning_func((void *)__get_SP());               \
                                 }                                                              \
                           }



/**
 * \brief Assert as error macro
 *
 */
#define ASSERT_ERROR(a) if (!(a)) {                                                            \
                                if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {                \
                                        __PUSH_SCRATCH_REGISTERS();                            \
                                        assert_warning_func((void *)__get_SP());               \
                                } else {                                                       \
                                        assert_error_func(0);                                  \
                                }                                                              \
                        }


/**
 * \brief Macro to disable all interrupts
 *
 * This macro must always be used with GLOBAL_INT_RESTORE(). E.g.
 *
 * \code{.c}
 * GLOBAL_INT_DISABLE();
 *  ... code to be executed with interrupts disabled ...
 * GLOBAL_INT_RESTORE();
 * \endcode
 *
 * \sa GLOBAL_INT_RESTORE
 */
#define GLOBAL_INT_DISABLE()                                                            \
        do {                                                                            \
                unsigned int __l_irq_rest;                                              \
                __ASM volatile ("mrs   %0, primask  \n\t"                               \
                              "mov   r1, $1     \n\t"                                   \
                              "msr   primask, r1  \n\t"                                 \
                              : "=r" (__l_irq_rest)                                     \
                              :                                                         \
                              : "r1"                                                    \
                              );                                                        \
               DBG_CONFIGURE_HIGH(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);

/**
 * \brief Macro to restore all interrupts
 *
 * This macro must always be used after GLOBAL_INT_DISABLE(). E.g.
 *
 * \code{.c}
 * GLOBAL_INT_DISABLE();
 *  ... code to be executed with interrupts disabled ...
 * GLOBAL_INT_RESTORE();
 * \endcode
 *
 * \sa GLOBAL_INT_DISABLE
 */
#define GLOBAL_INT_RESTORE()                                                            \
                if (__l_irq_rest == 0) {                                                \
                        DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);   \
                }                                                                       \
                __ASM volatile ("msr   primask, %0  \n\t"                               \
                                                          :                             \
                                                          : "r" (__l_irq_rest)          \
                                                          :                             \
                                                          );                            \
        } while (0)

#define containingoffset(address, type, field) ((type*)((uint8*)(address)-(size_t)(&((type*)0)->field)))

/**
 * \brief Macro the minimum of two values
 *
 * \param[in] a         First value
 * \param[in] b         Second value
 */
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/**
 * \brief Macro the maximum of two values
 *
 * \param[in] a         First value
 * \param[in] b         Second value
 */
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

/**
 * \brief Macro to swap the bytes of a 16-bit variable
 *
 * \param[in] a         The 16-bit variable
 */
#if defined(__GNUC__)
#define SWAP16(a) __builtin_bswap16(a)
#else
#define SWAP16(a) ((a<<8) | (a>>8))
#endif

/**
 * \brief Macro to swap the bytes of a 32-bit variable
 *
 * \param[in] a         The 32-bit variable
 */
#if defined(__GNUC__)
#define SWAP32(a) __builtin_bswap32(a)
#else
#define SWAP32(a) ((a>>24 & 0xff) | (a>>8 & 0xff00) | (a<<8 & 0xff0000) | (a<<24 & 0xff000000))
#endif

#if defined(__GNUC__)
#define DEPRECATED __attribute__((deprecated))
#else
#pragma message "Deprecated macro must be implemented for this compiler"
#define DEPRECATED
#endif

#if defined(__GNUC__)
#define DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#else
#pragma message "Deprecated macro must be implemented for this compiler"
#define DEPRECATED_MSG(msg)
#endif


DEPRECATED_MSG ("Consider using ASSERT_WARNING instead.")
__STATIC_FORCEINLINE void ASSERT_WARNING_UNINIT(bool a)
{
        ASSERT_WARNING(a)
}

 DEPRECATED_MSG ("Consider using ASSERT_ERROR instead.")
 __STATIC_FORCEINLINE void ASSERT_ERROR_UNINIT(bool a)
 {
         ASSERT_ERROR(a)
 }

#if defined(__GNUC__)
#define DEPRECATED_LITERAL_MACRO(macro, msg) \
        DEPRECATED_MSG(msg) static const uint32_t macro = 0;
#else
#pragma message "Deprecated literal macro must be implemented for this compiler"
#define DEPRECATED_LITERAL_MACRO(macro, msg)
#endif

#if defined(__GNUC__)
#define DEPRECATED_MACRO(macro, msg) DEPRECATED_MSG(msg) __STATIC_INLINE void macro(void) {}
#else
#pragma message "Deprecated macro must be implemented for this compiler"
#define DEPRECATED_MACRO(macro, msg)
#endif

/* The following exist in ROM code */
void __aeabi_memcpy(void *dest, const void *src, size_t n);
void __aeabi_memmove(void *dest, const void *src, size_t n);
void __aeabi_memset(void *dest, size_t n, int c);

/**
 * \brief Optimized memcpy
 */
#define OPT_MEMCPY      __aeabi_memcpy

/**
 * \brief Optimized memmove
 */
#define OPT_MEMMOVE     __aeabi_memmove

/**
 * \brief Optimized memset
 */
#define OPT_MEMSET(s, c, n)      __aeabi_memset(s, n, c)

/**
 * \brief Access register field mask.
 *
 * Returns a register field mask (aimed to be used with local variables).
 * e.g.
 * \code
 * uint16_t tmp;
 *
 * tmp = CRG_TOP->SYS_STAT_REG;
 *
 * if (tmp & REG_MSK(CRG_TOP, SYS_STAT_REG, XTAL16_TRIM_READY)) {
 * ...
 * \endcode
 */
#define REG_MSK(base, reg, field) \
        (base ## _ ## reg ## _ ## field ## _Msk)

/**
 * \brief Access register field position.
 *
 * Returns a register field position (aimed to be used with local variables).
 */
#define REG_POS(base, reg, field) \
        (base ## _ ## reg ## _ ## field ## _Pos)

/**
 * \brief Access register field value.
 *
 * Returns a register field value (aimed to be used with local variables).
 * e.g.
 * \code
 * uint16_t tmp;
 * int counter;
 * tmp = CRG_TOP->TRIM_CTRL_REG;
 * counter = REG_GET_FIELD(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N, tmp);
 * ...
 * \endcode
 */
#define REG_GET_FIELD(base, reg, field, var) \
        ((var & (base ## _ ## reg ## _ ## field ## _Msk)) >> \
                (base ## _ ## reg ## _ ## field ## _Pos))

/**
 * \brief Set register field value.
 *
 * Sets a register field value (aimed to be used with local variables).
 * e.g.
 * \code
 * uint16_t tmp;
 *
 * tmp = CRG_TOP->TRIM_CTRL_REG;
 * REG_SET_FIELD(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N, tmp, 10);
 * REG_SET_FIELD(CRG_TOP, TRIM_CTRL_REG, XTAL_TRIM_SELECT, tmp, 2);
 * CRG_TOP->TRIM_CTRL_REG = tmp;
 * ...
 * \endcode
 */
#define REG_SET_FIELD(base, reg, field, var, val) \
        var = ((var & ~((base ## _ ## reg ## _ ## field ## _Msk))) | \
                (((val) << (base ## _ ## reg ## _ ## field ## _Pos)) & \
                (base ## _ ## reg ## _ ## field ## _Msk)))

/**
 * \brief Set memory field value.
 *
 * Sets a memory field value using a mask (aimed to be used with local variables).
 * e.g.
 * \code
 * uint32_t tmp = *(volatile uint32_t *)0x50000000;
 *
 * RAW_SET_FIELD(tmp, 0x1UL, 1);
 * ...
 * \endcode
 */
#define RAW_SET_FIELD(mem, mask, val) \
        mem = ((mem & ~((mask))) | (((val) << (__builtin_ctz(mask))) & (mask)))

/**
 * \brief Clear register field value.
 *
 * Clears a register field value (aimed to be used with local variables).
 * e.g.
 * \code
 * uint16_t tmp;
 *
 * tmp = CRG_TOP->TRIM_CTRL_REG;
 * REG_CLR_FIELD(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N, tmp);
 * REG_CLR_FIELD(CRG_TOP, TRIM_CTRL_REG, XTAL_TRIM_SELECT, tmp);
 * CRG_TOP->TRIM_CTRL_REG = tmp;
 * ...
 * \endcode
 */
#define REG_CLR_FIELD(base, reg, field, var) \
        var &= ~(base ## _ ## reg ## _ ## field ## _Msk)

/**
 * \brief Get the address of a register value by index (provided a register interval)
 *
 * \note The register interval should be an exact multiple of the register's base size. For example,
 * if the register size is 32-bit, then the interval should be 0x4, 0x8, etc. Otherwise, the result
 * will be undefined. The interval value must be in bytes. The index value (0,1,2...) is multiplied by
 * the interval value (in bytes) to find the actual offset of the register.
 *
 * Returns a register address value by index
 */
#define REG_GET_ADDR_INDEXED(base, reg, interval, index) \
        ((&base->reg) + (((intptr_t) index) * ((interval) / sizeof(base->reg))))

/**
 * \brief Return the value of a register field by index (provided a register interval).
 *
 * e.g.
 * \code
 * uint16_t val;
 * uint16_t index = 2
 *
 * val = REG_GETF_INDEXED(FTDF, FTDF_LONG_ADDR_0_0_REG, REG_EXP_SA_L, 0x10, index)
 *
 * ...
 * \endcode
 *
 * \note The register interval should be an exact multiple of the register's base size. For example,
 * if the register size is 32-bit, then the interval should be 0x4, 0x8, etc. Otherwise, the result
 * will be undefined. The interval value must be in bytes. The index value (0,1,2...) is multiplied by
 * the interval value (in bytes) to find the actual offset of the register.
 *
 */
#define REG_GETF_INDEXED(base, reg, field, interval, index) \
        (((*REG_GET_ADDR_INDEXED(base, reg, interval, index)) & \
            (base ## _ ## reg ## _ ## field ## _Msk)) >> (base ## _ ## reg ## _ ## field ## _Pos))

/**
 * \brief Return the value of a register field.
 *
 * e.g.
 * \code
 * uint32_t val;
 *
 * val = REG_GETF(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N);
 * ...
 * \endcode
 */
#define REG_GETF(base, reg, field) \
        (((base->reg) & (base##_##reg##_##field##_Msk)) >> (base##_##reg##_##field##_Pos))

/**
 * \brief Return the value of a memory field using a mask.
 *
 * e.g.
 * \code
 * uint32_t val;
 *
 * val = RAW_GETF(0x50000000, 0x1UL);
 * ...
 * \endcode
 */
#define RAW_GETF(addr, mask) \
        ((*(volatile uint32_t *)(addr) & (mask)) >> (__builtin_ctz(mask)))

/**
 * \brief Set the value of a register field.
 *
 * e.g.
 * \code
 *
 * REG_SETF(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N, new_value);
 * ...
 * \endcode
 */
#define REG_SETF(base, reg, field, new_val) \
                base->reg = ((base->reg & ~(base##_##reg##_##field##_Msk)) | \
                ((base##_##reg##_##field##_Msk) & ((new_val) << (base##_##reg##_##field##_Pos))))

/**
 * \brief Set the value of a memory field.
 *
 * e.g.
 * \code
 *
 * RAW_SETF(0x50000000, 0x1UL, 1);
 * ...
 * \endcode
 */
#define RAW_SETF(addr, mask, val) \
                (*(volatile uint32_t *)(addr)) = ((*(volatile uint32_t *)(addr) & ~(mask)) | \
                ((mask) & ((val) << __builtin_ctz(mask))))

/**
 * \brief Set a bit of a register.
 *
 * e.g.
 * \code
 *
 * REG_SET_BIT(CRG_TOP, CLK_TMR_REG, TMR1_ENABLE);
 * ...
 * \endcode
 */
#define REG_SET_BIT(base, reg, field) \
        do { \
                base->reg |= (1 << (base##_##reg##_##field##_Pos)); \
         } while (0)

/**
 * \brief Clear a bit of a register.
 *
 * e.g.
 * \code
 *
 * REG_CLR_BIT(CRG_TOP, CLK_TMR_REG, TMR1_ENABLE);
 * ...
 * \endcode
 */
#define REG_CLR_BIT(base, reg, field) \
        do { \
                base->reg &= ~(base##_##reg##_##field##_Msk); \
         } while (0)

/**
 * \brief Sets register bits, indicated by the mask, to a value.
 *
 * e.g.
 * \code
 * REG_SET_MASKED(RFCU_POWER, RF_CNTRL_TIMER_5_REG, 0xFF00, 0x1818);
 * \endcode
 */
#define REG_SET_MASKED(base, reg, mask, value) \
        do { \
                base->reg = (base->reg & ~(mask)) | ((value) & (mask)); \
        } while (0)

/**
 * \brief Sets memory bits, indicated by the mask, to a value.
 *
 * e.g.
 * \code
 * RAW_SET_MASKED(0x50000000, 0xFF00, 0x1818);
 * \endcode
 */
#define RAW_SET_MASKED(addr, mask, value) \
        do { \
                (*(volatile uint32_t *)(addr)) = ((*(volatile uint32_t *)(addr)) & ~(mask)) | ((value) & (mask)); \
        } while (0)

/**
 * \brief Sets 16-bit wide register bits, indicated by the field, to a value v.
 */
#define BITS16(base, reg, field, v) \
        ((uint16) (((uint16) (v) << (base ## _ ## reg ## _ ## field ## _Pos)) & \
                (base ## _ ## reg ## _ ## field ## _Msk)))

/**
 * \brief Sets 32-bit wide register bits, indicated by the field, to a value v.
 */
#define BITS32(base, reg, field, v) \
        ((uint32) (((uint32) (v) << (base ## _ ## reg ## _ ## field ## _Pos)) & \
                (base ## _ ## reg ## _ ## field ## _Msk)))

/**
 * \brief Reads 16-bit wide register bits, indicated by the field, to a variable v.
 */
#define GETBITS16(base, reg, v, field) \
        ((uint16) (((uint16) (v)) & (base ## _ ## reg ## _ ## field ## _Msk)) >> \
                (base ## _ ## reg ## _ ## field ## _Pos))

/**
 * \brief Reads 32-bit wide register bits, indicated by the field, to a variable v.
 */
#define GETBITS32(base, reg, v, field) \
        ((uint32) (((uint32) (v)) & (base ## _ ## reg ## _ ## field ## _Msk)) >> \
                (base ## _ ## reg ## _ ## field ## _Pos))

/**
 * \brief Macro to enable the debugger
 *
 */
#define ENABLE_DEBUGGER \
        do { \
                REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, DEBUGGER_ENABLE); \
        } while (0)

/**
 * \brief Macro to enable the CMAC debugger
 *
 */
#define ENABLE_CMAC_DEBUGGER \
        do { \
                CRG_TOP->SYS_CTRL_REG |= (1 << (6UL)); \
        } while (0)

/**
 * \brief Macro to enable the SNC debugger
 *
 */
#define ENABLE_SNC_DEBUGGER \
        do { \
                CRG_TOP->SYS_CTRL_REG |= (1 << (5UL)); \
        } while (0)

/**
 * \brief Macro to disable the debugger
 *
 */
#define DISABLE_DEBUGGER \
        do { \
                REG_CLR_BIT(CRG_TOP, SYS_CTRL_REG, DEBUGGER_ENABLE); \
        } while (0)

/**
 * \brief Macro to disable the CMAC debugger
 *
 */
#define DISABLE_CMAC_DEBUGGER \
        do { \
                CRG_TOP->SYS_CTRL_REG &= ~(0x40UL); \
        } while (0)

/**
 * \brief Macro to disable the SNC debugger
 *
 */
#define DISABLE_SNC_DEBUGGER \
        do { \
                CRG_TOP->SYS_CTRL_REG &= ~(0x20UL); \
        } while (0)

/**
 * \brief Macro to trigger a software reset
 *
 */
#define SWRESET \
        do { \
                REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, SW_RESET); \
        } while (0)

#define BIT0        0x01
#define BIT1        0x02
#define BIT2        0x04
#define BIT3        0x08
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80

#define BIT8      0x0100
#define BIT9      0x0200
#define BIT10     0x0400
#define BIT11     0x0800
#define BIT12     0x1000
#define BIT13     0x2000
#define BIT14     0x4000
#define BIT15     0x8000

#define BIT16 0x00010000
#define BIT17 0x00020000
#define BIT18 0x00040000
#define BIT19 0x00080000
#define BIT20 0x00100000
#define BIT21 0x00200000
#define BIT22 0x00400000
#define BIT23 0x00800000

#define BIT24 0x01000000
#define BIT25 0x02000000
#define BIT26 0x04000000
#define BIT27 0x08000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

typedef unsigned char      uint8;   //  8 bits
typedef char               int8;    //  8 bits
typedef unsigned short     uint16;  // 16 bits
typedef short              int16;   // 16 bits
typedef unsigned long      uint32;  // 32 bits
typedef long               int32;   // 32 bits
typedef unsigned long long uint64;  // 64 bits
typedef long long          int64;   // 64 bits

/* See also "Data Types" on pag. 21 of the (Doulos) Cortex-M0 / SoC 1.0 training documentation. */
typedef unsigned char      BYTE;     //  8 bits = Byte
typedef unsigned short     HWORD;    // 16 bits = Halfword
typedef unsigned long      WORD;     // 32 bits = Word
typedef long long          DWORD;    // 64 bits = Doubleword

/**
 * \brief Macro to get the number of elements in an array
 *
 * \param[in] array     The array
 */
#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

/**
 * \brief Macro to convert time in nsec to clock cycles
 *
 * \param[in] nsec              Time in nsec
 * \param[in] clk_freq_hz       Clock Frequency in Hz
 */
#define NSEC_TO_CLK_CYCLES(nsec, clk_freq_hz)   ((((nsec) * ((clk_freq_hz) / 10000)) + 99999) / 100000)


#ifdef __cplusplus
}
#endif

#endif  /* __SDK_DEFS_H__ */

/**
 * \}
 * \}
 */
