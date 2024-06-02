/**
 ****************************************************************************************
 *
 * @file sys_drbg.h
 *
 * @brief sys_drbg header file.
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
 * \addtogroup SYS_DRBG Deterministic Random Bit Generator
 *
 * \brief Deterministic random bit generator
 *
 * \{
 *
 */

#ifndef SYS_DRBG_H_
#define SYS_DRBG_H_


#if (MAIN_PROCESSOR_BUILD)

#if dg_configUSE_SYS_DRBG

#include <stdint.h>
#include "bsp_defaults.h"

/*
 * MACROS
 *****************************************************************************************
 */

/*
 * The length of the buffer which holds the random numbers.
 */
#define SYS_DRBG_BUFFER_LENGTH                  dg_configUSE_SYS_DRBG_BUFFER_LENGTH

/*
 * Threshold level (index) in the buffer which holds the random numbers. When the buffer index
 * reaches the threshold level or becomes greater than the threshold level a request for buffer
 * update is issued.
 */
#define SYS_DRBG_BUFFER_THRESHOLD               dg_configUSE_SYS_DRBG_BUFFER_THRESHOLD

#if (SYS_DRBG_BUFFER_THRESHOLD >= SYS_DRBG_BUFFER_LENGTH)
#error "The threshold must be less than the buffer length"
#endif

#if (SYS_DRBG_BUFFER_THRESHOLD <= 0)
#error "The threshold must be greater than zero"
#endif

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief sys drbg errors
 */
typedef enum {
        SYS_DRBG_ERROR_NONE             = 0,    /**< no error */
        SYS_DRBG_ERROR_BUFFER_EXHAUSTED = -1,   /**< buffer with random numbers has been exhausted */
} SYS_DRBG_ERROR;

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

#if (dg_configUSE_SYS_TRNG == 0)
/**
 * \brief Checks whether the DRBG module can use a random value from RAM as a seed or not.
 *
 * \return if true the sys_drbg_srand() can be called, otherwise not.
 */
bool sys_drbg_can_run(void);
#endif /* dg_configUSE_SYS_TRNG */

/**
 * \brief Set the seed for the random number generator function.
 */
void sys_drbg_srand(void);

#if defined(OS_PRESENT)
/**
 * \brief DRBG mutex and task creation.
 *
 * \note This function shall be called after the scheduler has been started.
 */
void sys_drbg_create_os_objects(void);
#endif /* OS_PRESENT */

/**
 * \brief Initializes the DRBG data structure. \sa sys_drbg_t
 */
void sys_drbg_init(void);

/**
 * \brief Reads a random number from the buffer which holds the random numbers.
 *
 * \note  When a random number is read from the buffer it is assumed to be consumed. The next time
 *        the sys_drbg_read_rand() will be called a new random number will be read from the buffer.
 *
 * \param[out] rand_number      The random number to be returned
 *
 * \return Error code. If 0 the random number was successfully read, otherwise not.
 */
SYS_DRBG_ERROR sys_drbg_read_rand(uint32_t *rand_number);

#if !defined(OS_PRESENT)
/**
 * \brief Updates the DRBG data structure. \sa sys_drbg_t
 */
void sys_drbg_update(void);
#endif /* OS_PRESENT */

/**
 * \brief Reads the current index value of the buffer.
 *
 * \return The current index value of the buffer
 */
uint32_t sys_drbg_read_index(void);

/**
 * \brief Reads the threshold level value.
 *
 * \note  If the buffer's current index is equal to the threshold level value or greater
 *        than the threshold level value, then a request for buffer update will be issued.
 *
 * \return The threshold level value
 */
uint32_t sys_drbg_read_threshold(void);

/**
 * \brief Reads the request value.
 *
 * \return Either 0 or 1. If the request value is 1, there is a pending request for buffer update.
 */
uint8_t sys_drbg_read_request(void);

#endif /* dg_configUSE_SYS_DRBG */

#endif /* MAIN_PROCESSOR_BUILD */


#endif /* SYS_DRBG_H_ */

/**
\}
\}
*/
