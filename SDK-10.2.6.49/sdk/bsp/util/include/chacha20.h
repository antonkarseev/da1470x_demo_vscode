/**
 ****************************************************************************************
 *
 * @file chacha20.h
 *
 * @brief ChaCha20 random generator API.
 *
 * Copyright (C) 2021-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 * \addtogroup THIRD_PARTY
 * \{
 * \addtogroup THI_CHACHA_RAND CSP Random number generator
 *
 * \brief Random number generator
 *
 * \{
 *
 */

#ifndef CHACHA20_H_
#define CHACHA20_H_

#if dg_configUSE_SYS_DRBG

#if dg_configUSE_CHACHA20_RAND

#include <stdint.h>

/**
 ****************************************************************************************
 * \brief Set its argument as the seed for random numbers to be returned by
 *        csprng_get_next_uint32().
 *
 * \param[in] key seed value
 ****************************************************************************************
 */
void csprng_seed(const uint8_t key[16]);

/**
 ****************************************************************************************
 * \brief Get random number.
 *
 * \return Random number
 ****************************************************************************************
 */
uint32_t csprng_get_next_uint32(void);

#endif /* dg_configUSE_CHACHA20_RAND */

#endif /* dg_configUSE_SYS_DRBG */

#endif /* CHACHA20_H_ */

///@}
///@}
