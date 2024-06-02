/**
 ****************************************************************************************
 *
 * @file sys_trng_v2.h
 *
 * @brief sys_trng_v2 header file.
 *
 * @note  It supports the following devices:
 *        - DA1470X
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_TRNG True Random Number Generator
 *
 * \brief Random number generation
 *
 * \{
 *
 */

#ifndef SYS_TRNG_V2_H_
#define SYS_TRNG_V2_H_


#if dg_configUSE_SYS_TRNG

#include <stdint.h>
#include "sdk_defs.h"
#include "iid_platform.h"
#include "iid_irng.h"
#include "iid_aes_types.h"
#include "iid_return_codes.h"

/*
 * MACROS
 *****************************************************************************************
 */

 /**
 * \brief The size of TRNG seed in bytes.
 */
#define SYS_TRNG_SEED_SIZE                              (IRNG_RANDOM_SEED_SIZE_BYTES)

 /**
 * \brief The number of memory blocks passed as entropy source. The size of the block is 16 bytes,
 *        as defined by IRNG_BLOCK_SIZE_BYTES.
 */
#define SYS_TRNG_MEMORY_BLOCKS                          (IRNG_MINIMUM_SRAM_PUF_BLOCKS + 4)

#if (SYS_TRNG_MEMORY_BLOCKS < IRNG_MINIMUM_SRAM_PUF_BLOCKS)
#error "The number of SYS_TRNG_MEMORY_BLOCKS must be equal or greater than IRNG_MINIMUM_SRAM_PUF_BLOCKS"
#endif

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief SYS_TRNG error codes
 */
typedef enum {
        SYS_TRNG_ERROR_NONE                     = IID_SUCCESS,
        SYS_TRNG_ERROR_NOT_ALLOWED              = IID_NOT_ALLOWED,
        SYS_TRNG_ERROR_INVALID_PARAMETERS       = IID_INVALID_PARAMETERS,
        SYS_TRNG_ERROR_INVALID_SRAM_PUF_DATA    = IID_ERROR_SRAM_PUF_DATA,
        SYS_TRNG_ERROR_INSUFFICIENT_SRAM_BLOCKS = IID_ERROR_INSUFFICIENT_SRAM_BLOCKS,
        SYS_TRNG_ERROR_AES_TIMEOUT              = IID_ERROR_AES_TIMEOUT,
        SYS_TRNG_ERROR_AES_FAILED               = IID_ERROR_AES_FAILED,
} SYS_TRNG_ERROR;

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

/**
 * \brief Checks whether the TRNG module can generate a seed or not.
 *
 * \return if true the sys_trng_init() can be called, otherwise not.
 */
bool sys_trng_can_run(void);

/**
 * \brief Runs a software algorithm which generates a random seed. Random memory data are used
 *        to feed the software algorithm.
 *
 * \note  It must be ensured that random memory data (it is assumed that a RAM cell contains
 *        random values when it powers-up) are fed to the software algorithm, otherwise the
 *        sys_trng_init() will return an error code. The sys_trng_can_run() assures that the
 *        software algorithm will be fed with random data.
 *
 * \return error code. If 0 the seed was successfully generated, otherwise not. \sa SYS_TRNG_ERROR
 */
SYS_TRNG_ERROR sys_trng_init(void);

/**
 * \brief Sets the trng_id value. The trng_id randomness defines if the sys_trng_init()
 *        can run or not.
 *
 * \param[in] value       The trng_id value to be set
 */
void sys_trng_set_trng_id(uint32_t value);

#endif /* dg_configUSE_SYS_TRNG */


#endif /* SYS_TRNG_V2_H_ */

/**
\}
\}
*/
