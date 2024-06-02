/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_MPU Memory Protection Unit Low Level Driver
 * \{
 * \brief MPU Driver
 */

/**
 ****************************************************************************************
 *
 * @file hw_mpu.h
 *
 * @brief Definition of API for the Memory Protection Unit (MPU) Low Level Driver.
 *
 * The MPU is an optional ARM CM33 feature supported in DA14yyx SoC families that
 * enables protecting loosely defined regions of system RAM memory through enforcing
 * privilege and access rules per region. All MPU LLD terminology in based on the ARM
 * CM33 nomenclature.
 *
 * Copyright (C) 2017-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_MPU_H_
#define HW_MPU_H_


#if dg_configUSE_HW_MPU

#include <stdbool.h>
#include "sdk_defs.h"

#define MPU_END_ADDRESS_MASK (0x1F)     /* Due to the 32-byte alignment of MPU-protected segments
                                         * described in the ARM M33 documentation,
                                         * all End Addresses must be ORed with this Mask */

/**
 * \brief Region Definitions
 *
 * The MPU divides the memory map into a number of eight regions. Each region has a defined
 * memory type and memory attributes that determine the behavior of accesses to the region.
 * A background (or default) region numbered as -1 exists with the same access attributes as
 * the generic memory map, but is accessible from privileged software only.
 */
typedef enum {
        HW_MPU_REGION_NONE = MPU_REGION_NONE,   /**< MPU Protection Omitted */
        HW_MPU_REGION_0 = MPU_REGION_0,         /**< MPU region 0 */
        HW_MPU_REGION_1 = MPU_REGION_1,         /**< MPU region 1 */
        HW_MPU_REGION_2 = MPU_REGION_2,         /**< MPU region 2 */
        HW_MPU_REGION_3 = MPU_REGION_3,         /**< MPU region 3 */
        HW_MPU_REGION_4 = MPU_REGION_4,         /**< MPU region 4 */
        HW_MPU_REGION_5 = MPU_REGION_5,         /**< MPU region 5 */
        HW_MPU_REGION_6 = MPU_REGION_6,         /**< MPU region 6 */
        HW_MPU_REGION_7 = MPU_REGION_7,         /**< MPU region 7 */
} HW_MPU_REGION_NUM;

/**
 * \brief Executable Region
 *
 * Attribute regarding the code execution from a particular region. The XN (eXecute Never) flag
 * must be zero and there must be read access for the privilege level in order to execute code
 * from the region, otherwise a memory manage (MemManage) fault is generated.
 */
typedef enum {
        HW_MPU_XN_FALSE = 0x00,         /**< Executable region */
        HW_MPU_XN_TRUE = 0x01,          /**< Execute never region */
} HW_MPU_XN;

/**
 * \brief Region Read/Write or Read Only
 *
 * Attribute regarding the access permission (AP) of a particular region with respect to privilege
 * level and read/write capabilities.
 */
typedef enum {
        HW_MPU_APH_RO_RW = 0x00,         /**< Read/write */
        HW_MPU_APH_RO_RO = 0x01,         /**< Read-only */
} HW_MPU_APH_RO;

/**
 * \brief Privileged or Non-Privileged access
 *
 * Attribute to allow an application the privilege of accessing CPU features such as memory, I/O,
 * enable/disable interrupts, setup the NVIC, etc. By system design it can be imperative to restrict
 * an application by defining accordingly the MPU settings for the corresponding region.
 */
typedef enum {
        HW_MPU_APL_NP_PRIVRW = 0x00,     /**< Privileged code only */
        HW_MPU_APL_NP_RW = 0x01,         /**< Any privilege level */
} HW_MPU_APL_NP;

/**
 * \brief Access Permissions
 *
 * Attribute regarding the access permission (AP) of a particular region with respect to privilege
 * level and read/write capabilities. Depending on the privilege configuration an application can
 * access or not CPU features such as memory, I/O, enable/disable interrupts, setup the NVIC, etc.
 * By system design it can be imperative to restrict an application by defining accordingly the
 * MPU settings for the corresponding region.
 * This enumerator is the superposition of HW_MPU_APH_RO and HW_MPU_APL_NP and is provided for
 * cases where the combined AP section is required.
 */
typedef enum {
        HW_MPU_AP_PRIVRW = ARM_MPU_AP_(HW_MPU_APH_RO_RW, HW_MPU_APL_NP_PRIVRW), /**< Read/write by privileged code only */
        HW_MPU_AP_RW = ARM_MPU_AP_(HW_MPU_APH_RO_RW, HW_MPU_APL_NP_RW),         /**< Read/write by any privilege level */
        HW_MPU_AP_PRIVRO = ARM_MPU_AP_(HW_MPU_APH_RO_RO, HW_MPU_APL_NP_PRIVRW), /**< Read-only by privileged code only */
        HW_MPU_AP_RO = ARM_MPU_AP_(HW_MPU_APH_RO_RO, HW_MPU_APL_NP_RW),         /**< Read-only by any privilege level */
} HW_MPU_AP;

/**
 * \brief Shareability
 *
 * Attribute regarding the Shareability status (SH) of a particular region. In our case (ARMv8-M33) the options
 * regarding Shareability are:
 *
 *              [Non-shareable] - This represents memory accessible only by a single processor or other agent,
 *              so memory accesses never need to be synchronized with other processors.
 *              [Inner Shareable] - This represents a shareability domain that can be shared by multiple processors,
 *              but not necessarily all of the agents in the system. A system might have multiple Inner Shareable
 *              domains. An operation that affects one Inner Shareable domain does not affect other Inner Shareable
 *              domains in the system.
 *              [Outer Shareable] - An outer shareable (OSH) domain re-order is shared by multiple agents and can
 *              consist of one or more inner shareable domains. An operation that affects an outer shareable domain
 *              also implicitly affects all inner shareable domains inside it. However, it does not otherwise behave
 *              as an inner shareable operation.
 *
 * CAUTION: The value of HW_MPU_SH must ALWAYS be other-than 0x01. A value of 0x01 will lead to UNPREDICTABLE behavior
 * according to ARMv8 MPU documentation.
 * The most common Shareability status is Non-Shareable.
 */
typedef enum {
        HW_MPU_SH_NS = ARM_MPU_SH_NON,          /**< Non-Shareable */
        HW_MPU_SH_OS = ARM_MPU_SH_OUTER,        /**< Outer Shareable */
        HW_MPU_SH_IS = ARM_MPU_SH_INNER,        /**< Inner Shareable */
} HW_MPU_SH;

/**
 * \brief Memory Type
 *
 * Attribute regarding the memory type of a particular region. According to ARM CM33 nomenclature
 * two memory types are defined: device memory pertains to a memory-mapped region for a peripheral,
 * while normal memory is instead relevant to CPU use. The following enumerator sums up the two most
 * commonly deployed attribute setups:
 * 0x00 - Device Memory,  non-Gathering, non-Re-Ordering, non-Early-Write-Acknowledgement (nGnRnE).
 * 0x44 - Inner Memory normal and non-Cacheable, Outer Memory normal and non-Cacheable.
 */
typedef enum {
        HW_MPU_ATTR_DEVICE = 0x00,      /**< Device Memory, nGnRnE */
        HW_MPU_ATTR_NORMAL = 0x44,      /**< Normal memory, Outer non-cacheable, Inner non-cacheable */
} HW_MPU_ATTR;

/**
 * \brief Memory Region Configuration
 */
typedef struct {
        uint32_t start_addr;            /**< MPU region start address. Address will be rounded to previous 32-byte multiple */
        uint32_t end_addr;              /**< MPU region end address. Address will be rounded to next 32-byte multiple minus 1 */
        HW_MPU_AP access_permissions;   /**< MPU region access permissions */
        HW_MPU_SH shareability;         /**< MPU region Shareability status */
        HW_MPU_XN execute_never;        /**< Defines whether code can be executed from this region */
        HW_MPU_ATTR attributes;         /**< MPU region's memory attributes */
} mpu_region_config;

/**
 * \brief Enables/Disables the operation of MPU during hard fault, NMI, and FAULTMASK handlers.
 * \param [in] hfnmiena Controls (enable/disable) operation of MPU during HardFault and NMI handlers.
 *
 *              When disabled, MPU is disabled during HardFault and NMI handlers, regardless of the
 *              value of the ENABLE bit.
 *              When enabled, the MPU is enabled during HardFault and NMI handlers.
 *
 */
__STATIC_FORCEINLINE void hw_mpu_hardfault_nmi_handlers_enable(bool hfnmiena)
{
        REG_SETF(MPU, CTRL, HFNMIENA, hfnmiena);
}

/**
 * \brief Enables/Disables Privileged Background Access
 * \param [in] privdefena Controls (enable/disable) privileged access to the background region.
 *
 *              When disabled, any access to the background region will cause a memory manage fault.
 *              When enabled, privileged accesses to the background region are allowed.
 *
 *              In handler mode, execution is always privileged. In thread mode
 *              privilege level can be set using the 'nPRIV' field of the control register.
 *              For manipulating nPRIV, check __set_CONTROL() and __get_CONTROL() CMSIS API calls.
 *              Hard fault and NMI handlers always operate with MPU disabled, accessing the
 *              default memory map as normal. The same can be true when FAULTMASK is set to 1,
 *              effectively masking Hard Fault exceptions by raising the current priority level to -1.
 *              FAULTMASK can only be set in privileged mode except from within NMI and HardFault
 *              Handlers (in which cases lockup state will be entered).
 *
 */
__STATIC_FORCEINLINE void hw_mpu_privileged_background_access_enable(bool privdefena)
{
        REG_SETF(MPU, CTRL, PRIVDEFENA, privdefena);
}

/**
 * \brief Initializes the MPU by disabling its operation during faults, defining the background region
 *              privilege access and finally by enabling the actual HW block.
 * \param [in] privdefena Controls (enable/disable) privileged access to the background region.
 *
 *              When disabled, any access to the background region will cause a memory manage fault.
 *              When enabled, privileged accesses to the background region are allowed.
 *
 *              In handler mode, execution is always privileged. In thread mode
 *              privilege level can be set using the 'nPRIV' field of the control register.
 *              For manipulating nPRIV, check __set_CONTROL() and __get_CONTROL() CMSIS API calls.
 *              Hard fault and NMI handlers always operate with MPU disabled, accessing the
 *              default memory map as normal. The same can be true when FAULTMASK is set to 1,
 *              effectively masking Hard Fault exceptions by raising the current priority level to -1.
 *              FAULTMASK can only be set in privileged mode except from within NMI and HardFault
 *              Handlers (in which cases lockup state will be entered).
 */
__STATIC_FORCEINLINE void hw_mpu_enable(bool privdefena)
{
        uint32_t MPU_Control = (privdefena ? REG_MSK(MPU, CTRL, PRIVDEFENA) : 0);
        ARM_MPU_Enable(MPU_Control);
}

/**
 * \brief Disables the MPU
 *
 */
__STATIC_FORCEINLINE void hw_mpu_disable(void)
{
        ARM_MPU_Disable();
        __ISB();
}

/**
 * \brief Checks if MPU is enabled
 *
 * \return true if enabled, false otherwise
 */
__STATIC_FORCEINLINE bool hw_mpu_is_enabled(void)
{
        return (!!REG_GETF(MPU, CTRL, ENABLE));
}

/**
 * \brief Configures an MPU region.
 * Region's start and end addresses will be aligned to 32 byte boundary. The start address
 * is logically ANDed with 0xFFFFFFE0 whereas the end address is logically ORed with 0x1F.
 *
 * The following accesses will generate a hard fault:
 * - An access to an address that matches in more than one region.
 * - An access that does not match all the access conditions for that region.
 * - An access to the background region, depending on the privilege mode and the value of
 *      the 'privdefena' parameter when MPU is enabled.
 *
 * \param [in] region_num Region number
 * \param [in] cfg Region configuration. When cfg is NULL the particular region is disabled.
 *
 * \note The regions intended for protection will be rounded to increments of 32 bytes in any case.
 *       This is a result of the fact that the 5 low bits of RLAR and RBAR registers are reserved
 *       for other purposes. The first two assertions serve as a reminder of that detail.
*/
__STATIC_FORCEINLINE void hw_mpu_config_region(HW_MPU_REGION_NUM region_num, mpu_region_config *cfg)
{
        ASSERT_ERROR(region_num > HW_MPU_REGION_NONE && region_num <= HW_MPU_REGION_7);
        if (!cfg) {
                GLOBAL_INT_DISABLE();

                ARM_MPU_ClrRegion(region_num);

                GLOBAL_INT_RESTORE();
        } else {
                /* The following assertions check whether the start and end addresses of the region intended
                 * to be protected comply with the 32-byte alignment rule as described in the ARM M33 MPU
                 * documentation. */
                ASSERT_WARNING((cfg->start_addr & MPU_END_ADDRESS_MASK) == 0);
                ASSERT_WARNING((cfg->end_addr & MPU_END_ADDRESS_MASK) == 0x1F);
                /* The following assertion checks whether the value of the shareability is other-than 0x01
                 * which will lead to UNPREDICTABLE behavior according to ARMv8 MPU documentation. */
                ASSERT_ERROR(cfg->shareability != 0x01);

                GLOBAL_INT_DISABLE();
                /* Each of the eight M33 MPU regions is configured via a specific 8 bit set in the 32-bit MAIR0 and
                 * MAIR1 registers. The lower four regions are catered by MAIR0 and the upper four by MAIR1
                 * respectively. Please refer to the ARM M33 MPU documentation for a more detailed description. */
                ARM_MPU_SetMemAttr(region_num, cfg->attributes);
                ARM_MPU_SetRegion(region_num,
                        ARM_MPU_RBAR(cfg->start_addr, cfg->shareability, ((cfg->access_permissions >> 1) & 0x01), (cfg->access_permissions & 0x01), cfg->execute_never),
                        ARM_MPU_RLAR(cfg->end_addr, region_num));
                GLOBAL_INT_RESTORE();
        }
}

#endif /* dg_configUSE_HW_MPU */


#endif /* HW_MPU_H_ */

/**
 * \}
 * \}
 */
