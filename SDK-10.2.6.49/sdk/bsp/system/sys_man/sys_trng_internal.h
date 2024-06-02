/**
 ****************************************************************************************
 *
 * @file sys_trng_internal.h
 *
 * @brief sys_trng internal header file.
 *
 * Copyright (C) 2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_TRNG_INTERNAL_H_
#define SYS_TRNG_INTERNAL_H_

#if (MAIN_PROCESSOR_BUILD)

/**
 * \brief Get a pointer to the TRNG seed calculated by sys_trng_init_seed()
 *
 * \return A pointer to the TRNG seed
 */
const uint8_t *sys_trng_get_seed(void);


#endif /* DEVICE_FAMILY */


#endif /* SYS_TRNG_INTERNAL_H_ */
