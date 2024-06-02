/**
\addtogroup PLA_DRI_PER_OTHER
\{
\addtogroup HW_BSR BSR driver
\{
\brief BSR LLD
*/

/**
****************************************************************************************
*
* @file hw_bsr.h
*
* @brief BSR LLD header file.
*
* Copyright (C) 2021-2022 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef HW_BSR_H_
#define HW_BSR_H_


#include "sdk_defs.h"
#include <stdbool.h>

/* \brief Specifies the master mask used for accessing the HW and SW BSR */
#define HW_BSR_MASTER_MASK     (0x3)

#define HW_BSR_MAX_HW_PERIPH_IDS (42)

/*
 * \brief Enumerations used when accessing the HW BSR register.
 *        When 0, indicates that resources (for which race condition may occur) are available
 *        for use. Otherwise, the value indicates the processing unit that controls and
 *        keeps busy the resource, restricting other processing units from accessing it.
 */
typedef enum {
        HW_BSR_MASTER_NONE = 0,
        HW_BSR_MASTER_SNC = 1,
        HW_BSR_MASTER_SYSCPU = 2,
        HW_BSR_MASTER_CMAC = 3,
        HW_BSR_MASTER_NUM,
} HW_BSR_MASTER_ID;

#define HW_BSR_SW_POS                   (0)     /*!< HW BSR position for general purpose SW locking  */
#define HW_BSR_PLL_ENABLE_POS           (2)     /*!< HW BSR position for PLL settings locking  */
#define HW_BSR_POWER_CTRL_POS           (28)    /*!< HW BSR position for power settings locking  */
#define HW_BSR_WAKEUP_CONFIG_POS        (30)    /*!< HW BSR position for wake up settings locking  */
#define HW_BSR_DRBG_POS                 (32)    /*!< HW BSR position for DRBG locking  */
#define HW_BSR_MAILBOX_POS              (34)    /*!< HW BSR position for SNC M33 mailbox locking  */
#define HW_BSR_I3C_POS                  (36)    /*!< HW BSR position for I3C locking  */

/*
 * \brief HW BSR position
 *
 * Indicates the resource (position or BSR bit) that can be reserved by a processing unit for
 * use. The enumeration defines a convention on resources that may be contented. For example,
 * position '0' is reserved for SW purposes. Such an approach deviating from the one defined in
 * the device family datasheet is not restricted (BSR[0:1] corresponds there to UART peripheral).
 * Every peripheral id should be even. Maximum supported peripheral ids are HW_BSR_MAX_HW_PERIPH_IDS.
 */
typedef enum {
        HW_BSR_PERIPH_ID_SW = HW_BSR_SW_POS, //0
        HW_BSR_PERIPH_ID_PPL_ENABLE = HW_BSR_PLL_ENABLE_POS, //2
        HW_BSR_PERIPH_ID_SNC = 4,
        HW_BSR_PERIPH_ID_SPI1 = 6,
        HW_BSR_PERIPH_ID_SPI2 = 8,
        HW_BSR_PERIPH_ID_UART1 = 10,
        HW_BSR_PERIPH_ID_UART2 = 12,
        HW_BSR_PERIPH_ID_UART3 = 14,
        HW_BSR_PERIPH_ID_I2C1 = 16,
        HW_BSR_PERIPH_ID_I2C2 = 18,
        HW_BSR_PERIPH_ID_RESERVED = 20,
        HW_BSR_PERIPH_ID_GPADC = 22,
        HW_BSR_PERIPH_ID_SDADC = 24,
        HW_BSR_PERIPH_ID_I2C3 = 26,
        HW_BSR_PERIPH_ID_POWER_CTRL = HW_BSR_POWER_CTRL_POS, //28
        HW_BSR_PERIPH_ID_WAKEUP_CONF = HW_BSR_WAKEUP_CONFIG_POS, //30
        HW_BSR_PERIPH_ID_DRBG = HW_BSR_DRBG_POS, //32
        HW_BSR_PERIPH_ID_MAILBOX = HW_BSR_MAILBOX_POS, //34
        HW_BSR_PERIPH_ID_I3C = HW_BSR_I3C_POS, // 36
        HW_BSR_PERIPH_ID_MAX
} HW_BSR_PERIPH_ID;

/**
 * \brief Initializes BSR status register.
 *
 * \note It shall be invoked as part of system initialization.
 */
void hw_bsr_init(void);

/**
 * \brief  Try to lock a BSR entry
 *
 * \param [in] bsr_master_id The id of the relevant master
 * \param [in] per_id The position of the entry in BSR.
 *
 * \note  It is possible to lock for the same master
 *        the same peripheral multiple times.
 *
 * \return true if the BSR entry has been acquired, else false.
 *
 */
__RETAINED_HOT_CODE bool hw_bsr_try_lock(HW_BSR_MASTER_ID bsr_master_id, HW_BSR_PERIPH_ID per_id);

/**
 * \brief  Unlock a BSR entry
 *
 * \param [in] bsr_master_id The id of the relevant master
 * \param [in] per_id The position of the entry in BSR.
 *
 */
__RETAINED_HOT_CODE void hw_bsr_unlock(HW_BSR_MASTER_ID bsr_master_id, HW_BSR_PERIPH_ID per_id);

/**
 * \brief  Check if BSR entry is locked by a master
 *
 * \param [in] bsr_master_id The id of the relevant master
 * \param [in] per_id The position of the entry in BSR.
 *
 */
bool hw_bsr_is_locked(HW_BSR_MASTER_ID bsr_master_id, HW_BSR_PERIPH_ID per_id);

#endif /* HW_BSR_H_ */

/**
\}
\}
*/

