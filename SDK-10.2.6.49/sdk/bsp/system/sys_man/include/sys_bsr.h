/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_BSR Busy Status Register Driver Service
 * \{
 * \brief Busy Status Register (BSR) driver
 */

/**
 ****************************************************************************************
 *
 * @file sys_bsr.h
 *
 * @brief Busy Status Register (BSR) driver file.
 *
 * Copyright (C) 2017-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_BSR_H_
#define SYS_BSR_H_


#include "hw_bsr.h"

#if (dg_configBSR_IMPLEMENTATION == HW_BSR_IMPLEMENTATION)
#undef USE_SW_BSR
#define USE_SW_BSR              ( 0 )
#elif (dg_configBSR_IMPLEMENTATION == SW_BSR_IMPLEMENTATION)
#undef USE_SW_BSR
#define USE_SW_BSR              ( 1 )
#else
#error "Improper configuration value for mandatory dg_configBSR_IMPLEMENTATION!"
#endif /* dg_configUSE_HW_BSR */

/**
 * Legacy H/W and S/W BSR ID's
 */
#define SW_BSR_MASTER_NONE      HW_BSR_MASTER_NONE
#define SW_BSR_MASTER_SNC       HW_BSR_MASTER_SNC
#define SW_BSR_MASTER_SYSCPU    HW_BSR_MASTER_SYSCPU
#define SW_BSR_MASTER_CMAC      HW_BSR_MASTER_CMAC

/**
 * \brief Enumerations used when accessing the HW/SW BSR register.
 *        Zero (i.e. *_MASTER_NONE) indicates a free resource,
 *        whereas an non-zero value indicates the occupying CPU,
 *        currently restricting others from using it
 */
typedef enum {
        SYS_BSR_MASTER_NONE = HW_BSR_MASTER_NONE,
        SYS_BSR_MASTER_SNC = HW_BSR_MASTER_SNC,
        SYS_BSR_MASTER_SYSCPU = HW_BSR_MASTER_SYSCPU,
        SYS_BSR_MASTER_CMAC = HW_BSR_MASTER_CMAC,
        SYS_BSR_MASTER_NUM = HW_BSR_MASTER_NUM,
} SYS_BSR_MASTER_ID;

/**
 * \brief BSR position
 *
 * Indicates the resource position on SW or HW BSR that can be reserved by a processing unit for
 * use. Maximum supported peripheral ids are SYS_BSR_PERIPH_ID_MAX.
 */
#if USE_SW_BSR
typedef enum {
        SYS_BSR_PERIPH_ID_SNC = 0,
        SYS_BSR_PERIPH_ID_SPI1 = 1,
        SYS_BSR_PERIPH_ID_SPI2 = 2,
        SYS_BSR_PERIPH_ID_UART1 = 3,
        SYS_BSR_PERIPH_ID_UART2 = 4,
        SYS_BSR_PERIPH_ID_UART3 = 5,
        SYS_BSR_PERIPH_ID_I2C1 = 6,
        SYS_BSR_PERIPH_ID_I2C2 = 7,
        SYS_BSR_PERIPH_ID_RESERVED = 8,
        SYS_BSR_PERIPH_ID_GPADC = 9,
        SYS_BSR_PERIPH_ID_SDADC = 10,
#if defined(I2C3)
        SYS_BSR_PERIPH_ID_I2C3 = 11,
#endif
#if defined(I3C)
        SYS_BSR_PERIPH_ID_I3C = 12,
#endif
        SYS_BSR_PERIPH_ID_MAX = 16,
} SYS_BSR_PERIPH_ID;
#define SYS_BSR_SW_BSR_SIZE     (SYS_BSR_PERIPH_ID_MAX)
#else
typedef enum {
        SYS_BSR_PERIPH_ID_SW = HW_BSR_PERIPH_ID_SW,
        SYS_BSR_PERIPH_ID_PPL_ENABLE = HW_BSR_PERIPH_ID_PPL_ENABLE,
        SYS_BSR_PERIPH_ID_SNC = HW_BSR_PERIPH_ID_SNC,
        SYS_BSR_PERIPH_ID_SPI1 = HW_BSR_PERIPH_ID_SPI1,
        SYS_BSR_PERIPH_ID_SPI2 = HW_BSR_PERIPH_ID_SPI2,
        SYS_BSR_PERIPH_ID_UART1 = HW_BSR_PERIPH_ID_UART1,
        SYS_BSR_PERIPH_ID_UART2 = HW_BSR_PERIPH_ID_UART2,
        SYS_BSR_PERIPH_ID_UART3 = HW_BSR_PERIPH_ID_UART3,
        SYS_BSR_PERIPH_ID_I2C1 = HW_BSR_PERIPH_ID_I2C1,
        SYS_BSR_PERIPH_ID_I2C2 = HW_BSR_PERIPH_ID_I2C2,
        SYS_BSR_PERIPH_ID_RESERVED = HW_BSR_PERIPH_ID_RESERVED,
        SYS_BSR_PERIPH_ID_GPADC = HW_BSR_PERIPH_ID_GPADC,
        SYS_BSR_PERIPH_ID_SDADC = HW_BSR_PERIPH_ID_SDADC,
        SYS_BSR_PERIPH_ID_I2C3 = HW_BSR_PERIPH_ID_I2C3,
        SYS_BSR_PERIPH_ID_POWER_CTRL = HW_BSR_PERIPH_ID_POWER_CTRL,
        SYS_BSR_PERIPH_ID_WAKEUP_CONF = HW_BSR_PERIPH_ID_WAKEUP_CONF,
        SYS_BSR_PERIPH_ID_DRBG = HW_BSR_PERIPH_ID_DRBG,
        SYS_BSR_PERIPH_ID_MAILBOX = HW_BSR_PERIPH_ID_MAILBOX,
        SYS_BSR_PERIPH_ID_I3C = HW_BSR_PERIPH_ID_I3C,
        SYS_BSR_PERIPH_ID_MAX
} SYS_BSR_PERIPH_ID;

#define SYS_BSR_SW_BSR_SIZE     ((SYS_BSR_PERIPH_ID_MAX + 1) >> 1) //HW BSR peripheral ids are even
#endif


/**
 * \brief Initialize HW BSR and SW BSR
 *
 * \details It initializes SW and HW BSR and sends to SNC the BSR counter.
 *
 * \warning It must be called once during system initialization
 */
void sys_bsr_initialize(void);

/**
 * \brief Initializes the software busy status register.
 *
 * \note It shall be invoked as part of system initialization.
 */
void sys_bsr_init(void);

/**
 * \brief Acquire exclusive access to a specific peripheral.
 *        It is also used by other masters (SNC or CMAC). This function
 *        will block until access is granted.
 *
 * \param [in] bsr_master_id The id of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access must be granted.
 *                       Valid range is (0 - SYS_BSR_PERIPH_ID_MAX)
 */
void sys_bsr_acquire(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id);

/**
 * \brief Try once to acquire exclusive access to a specific peripheral.
 *        It is also used by other masters (SNC or CMAC).
 *
 * \param [in] bsr_master_id The id of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access must be granted.
 *                       Valid range is (0 - SYS_BSR_PERIPH_ID_MAX)
 */
bool sys_bsr_try_acquire(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id);

/**
 * \brief Checks if exclusive access to a specific peripheral has been acquired
 *        from a given master.
 *
 * \param [in] bsr_master_id The id of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access will be checked.
 *                       Valid range is (0 - SYS_BSR_PERIPH_ID_MAX).
 * \return true if peripheral exclusive access has been acquired from the specific master, else false.
 */
bool sys_bsr_acquired(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id);

/**
 * \brief Releases the exclusive access from a specific peripheral.
 *        It can be also used by other masters (SNC or CMAC).
 *
 * \param [in] bsr_master_id The id of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access must be released.
 *                       Valid range is (0 - SYS_BSR_PERIPH_ID_MAX).
 */
void sys_bsr_release(SYS_BSR_MASTER_ID bsr_master_id, SYS_BSR_PERIPH_ID periph_id);

#endif /* SYS_BSR_H_ */

/**
 \}
 \}
 */
