/**
 ****************************************************************************************
 *
 * @file dialog_semphr.h
 *
 * @brief Extensions to FreeRTOS semphr.h
 *
 * Copyright (C) 2020-2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DIALOG_SEMAPHORE_H_
#define DIALOG_SEMAPHORE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Obtain semaphore
 *
 * This function is used for the calling co-routine to obtain a semaphore. It checks whether
 * the semaphore is available, and if it is, it obtains it. Otherwise co-routine blocks and waits
 * for the semaphore to become available. The co-routine does not consume any CPU time while it
 * is in the Blocked state.
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary(),
 * xSemaphoreCreateMutex() or xSemaphoreCreateCounting().
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xSemaphore handle of the semaphore to be obtained
 * \param [in] xTicksToWait maximum amount of time in ticks the co-routine should wait in the
 * Blocked state for the semaphore to become available
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if semaphore
 * was successfully obtained, otherwise pdFAIL
 *
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateMutex
 * \sa xSemaphoreCreateCounting
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrSEMAPHORE_TAKE(xHandle, xSemaphore, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xSemaphoreDgCRTake((xSemaphore), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xSemaphoreDgCRTake((xSemaphore), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrSEMAPHORE_TAKE(xHandle, xSemaphore, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xSemaphoreDgCRTake((xSemaphore), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xSemaphoreDgCRTake((xSemaphore), 0); \
                } \
        } while (0)
#endif

/**
 * \brief Wait for semaphore to become available
 *
 * This function is used for the calling co-routine to wait for a semaphore to become available.
 * It checks whether the semaphore is available, and if not, the co-routine blocks and waits for
 * the semaphore to become available. The co-routine does not consume any CPU time while it
 * is in the Blocked state. The semaphore is not obtained by the co-routine.
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary(),
 * xSemaphoreCreateMutex() or xSemaphoreCreateCounting().
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xSemaphore handle of the semaphore to be checked
 * \param [in] xTicksToWait maximum amount of time in ticks the co-routine should wait in the
 * Blocked state for the semaphore to become available
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if semaphore
 * became available, otherwise pdFAIL
 *
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateMutex
 * \sa xSemaphoreCreateCounting
 */
#define dgcrSEMAPHORE_CHECK(xHandle, xSemaphore, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xSemaphoreDgCRCheck((xSemaphore), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xSemaphoreDgCRCheck((xSemaphore), 0); \
                } \
        } while (0)

#if (configUSE_RECURSIVE_MUTEXES == 1)
/**
 * \brief Obtain recursively mutex type semaphore
 *
 * This function is used for the calling co-routine to recursively obtain a mutex type semaphore.
 * It checks whether the mutex is available, and if it is, it obtains it and updates the owner
 * co-routine of the mutex. Otherwise, if it has previously been obtained by the same co-routine,
 * it just increases the corresponding counter measuring the times the mutex has been obtained,
 * else the co-routine blocks and waits for the mutex to become available. The co-routine does not
 * consume any CPU time while it is in the Blocked state.
 *
 * Since a mutex used recursively can be 'taken' repeatedly by the owner, it does not become
 * available again until the owner has released the mutex for each successful 'take' request.
 *
 * The mutex must have previously been created using a call to xSemaphoreCreateRecursiveMutex().
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xMutex handle of the mutex being obtained
 * \param [in] xTicksToWait maximum amount of time in ticks the co-routine should wait in the
 * Blocked state for the mutex to be obtained
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if mutex
 * was obtained, otherwise pdFAIL
 *
 * \sa xSemaphoreCreateRecursiveMutex
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrSEMAPHORE_TAKE_RECURSIVE(xHandle, xMutex, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xSemaphoreDgCRTakeRecursive((xMutex), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xSemaphoreDgCRTakeRecursive((xMutex), 0); \
                } \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE1((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrSEMAPHORE_TAKE_RECURSIVE(xHandle, xMutex, xTicksToWait, pxResult) \
        do { \
                *(pxResult) = xSemaphoreDgCRTakeRecursive((xMutex), (xTicksToWait)); \
                if (*(pxResult) == errQUEUE_BLOCKED) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = xSemaphoreDgCRTakeRecursive((xMutex), 0); \
                } \
        } while (0)
#endif
#endif /* configUSE_RECURSIVE_MUTEXES */

/**
 * \brief Release semaphore
 *
 * This function is used for the calling co-routine to release a semaphore that has previously been
 * obtained.
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary(),
 * xSemaphoreCreateMutex() or xSemaphoreCreateCounting().
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine with higher priority.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xSemaphore handle of the semaphore to be released
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if mutex
 * was released, otherwise pdFAIL
 *
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateMutex
 * \sa xSemaphoreCreateCounting
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrSEMAPHORE_GIVE(xHandle, xSemaphore, pxResult) \
        do { \
                *(pxResult) = xSemaphoreDgCRGive((xSemaphore)); \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrSEMAPHORE_GIVE(xHandle, xSemaphore, pxResult) \
        do { \
                *(pxResult) = xSemaphoreDgCRGive((xSemaphore)); \
        } while (0)
#endif

#if (configUSE_RECURSIVE_MUTEXES == 1)
/**
 * \brief Release recursively mutex type semaphore
 *
 * This function is used for the calling co-routine to recursively release a mutex type semaphore
 * that has previously been recursively obtained. Since a mutex used recursively can be 'taken'
 * repeatedly be the owner, it does not become available again until the owner has released the
 * mutex for each successful 'take' request.
 *
 * The mutex must have previously been created using a call to xSemaphoreCreateRecursiveMutex().
 *
 * This function must not be called from an interrupt service routine (ISR).
 * It can only be called from the co-routine function itself - not from within a function called
 * by the co-routine function. This is because co-routines do not maintain their own stack, and
 * the calling co-routine may yield its execution in favor of another co-routine with higher priority.
 *
 * \param [in] xHandle handle of the calling co-routine, which is the xHandle parameter of the
 * co-routine function
 * \param [in] xMutex handle of the mutex being released
 * \param [out] pxResult pointer to store the result of this function, that is, pdPASS if mutex
 * was released, otherwise pdFAIL
 *
 * \sa xSemaphoreCreateRecursiveMutex
 */
#if (configUSE_PREEMPTION == 1)
#define dgcrSEMAPHORE_GIVE_RECURSIVE(xHandle, xMutex, pxResult) \
        do { \
                *(pxResult) = xSemaphoreDgCRGiveRecursive((xMutex)); \
                if (*(pxResult) == errQUEUE_YIELD) { \
                        crSET_STATE0((xHandle)); \
                        *(pxResult) = pdPASS; \
                } \
        } while (0)
#else
#define dgcrSEMAPHORE_GIVE_RECURSIVE(xHandle, xMutex, pxResult) \
        do { \
                *(pxResult) = xSemaphoreDgCRGiveRecursive((xMutex)); \
        } while (0)
#endif
#endif /* configUSE_RECURSIVE_MUTEXES */

/**
 * \brief Obtain semaphore from ISR
 *
 * This function is used for obtaining a semaphore from interrupt service routine (ISR). It checks
 * whether the semaphore is available, and if it is, it obtains it.
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary()
 * or xSemaphoreCreateCounting().
 *
 * \param [in] xSemaphore handle of the semaphore to be obtained
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by obtaining the semaphore
 *
 * \return pdPASS if the semaphore was obtained, otherwise pdFAIL
 *
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateCounting
 */
#define dgcrSEMAPHORE_TAKE_FROM_ISR(xSemaphore, pxCoRoutineWoken) \
        xSemaphoreDgCRTakeFromISR((xSemaphore), (pxCoRoutineWoken))

/**
 * \brief Check whether semaphore is available
 *
 * This function is used for checking from interrupt service routine (ISR) whether a semaphore is
 * available. The semaphore is not obtained.
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary()
 * or xSemaphoreCreateCounting().
 *
 * \param [in] xSemaphore handle of the semaphore to be checked
 *
 * \return pdPASS if the semaphore is available, otherwise pdFAIL
 *
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateCounting
 */
#define dgcrSEMAPHORE_CHECK_FROM_ISR(xSemaphore) \
        xSemaphoreDgCRCheckFromISR((xSemaphore))

/**
 * \brief Release semaphore from ISR
 *
 * This function is used for releasing from interrupt service routine (ISR) a semaphore that has
 * previously been obtained.
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary()
 * or xSemaphoreCreateCounting().
 *
 * \param [in] xSemaphore handle of the semaphore to be released
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by releasing the semaphore
 *
 * \return pdPASS if the semaphore was released, otherwise pdFAIL
 *
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateCounting
 */
#define dgcrSEMAPHORE_GIVE_FROM_ISR(xSemaphore, pxCoRoutineWoken) \
        xSemaphoreDgCRGiveFromISR((xSemaphore), (pxCoRoutineWoken))

/**
 * \brief Obtain semaphore
 *
 * This function is used for the calling co-routine to obtain a semaphore. It checks whether
 * the semaphore is available, and if it is, it obtains it. Otherwise it indicates whether the
 * co-routine needs to block and wait for it to become available.
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary(),
 * xSemaphoreCreateMutex() or xSemaphoreCreateCounting().
 *
 * \param [in] xSemaphore handle of the semaphore to be obtained
 * \param [in] xBlockTime time in ticks to wait for the semaphore to become available
 *
 * \return pdPASS if the semaphore was obtained, otherwise an error code as defined within ProjDefs.h
 *
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateMutex
 * \sa xSemaphoreCreateCounting
 */
#define xSemaphoreDgCRTake(xSemaphore, xBlockTime) \
        xQueueDgCRSemaphoreTake((xSemaphore), (xBlockTime))

/**
 * \brief Wait for semaphore to become available
 *
 * This function is used for the calling co-routine to check whether a semaphore is available.
 * It indicates whether it is available or the co-routine needs to block and wait for it to
 * become available. The semaphore is not obtained by the co-routine.
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary(),
 * xSemaphoreCreateMutex() or xSemaphoreCreateCounting().
 *
 * \param [in] xSemaphore handle of the semaphore to be checked
 * \param [in] xBlockTime time in ticks to wait for the semaphore to become available
 *
 * \return pdPASS if the semaphore became available, otherwise an error code as defined within ProjDefs.h
 *
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateMutex
 * \sa xSemaphoreCreateCounting
 */
#define xSemaphoreDgCRCheck(xSemaphore, xBlockTime) \
        xQueueDgCRSemaphoreCheck((xSemaphore), (xBlockTime))

/**
 * \brief Obtain recursively mutex type semaphore
 *
 * This function is used for the calling co-routine to recursively obtain a mutex type semaphore.
 * It checks whether the mutex is available, and if it is, it obtains it and updates the owner
 * co-routine of the mutex. Otherwise, if it has previously been obtained by the same co-routine,
 * it just increases the corresponding counter measuring the times the mutex has been obtained,
 * else it indicates that the co-routine needs to block and wait for the mutex to become available.
 *
 * Since a mutex used recursively can be 'taken' repeatedly by the owner, it does not become
 * available again until the owner has called xSemaphoreDgCRGiveRecursive() for each successful
 * 'take' request.
 *
 * The mutex must have previously been created using a call to xSemaphoreCreateRecursiveMutex().
 *
 * \param [in] xMutex handle of the mutex being obtained
 * \param [in] xBlockTime time in ticks to wait for the mutex to be obtained
 *
 * \return pdPASS if the mutex was obtained, otherwise an error code as defined within ProjDefs.h
 *
 * \sa xSemaphoreDgCRGiveRecursive
 * \sa xSemaphoreCreateRecursiveMutex
 */
#define xSemaphoreDgCRTakeRecursive(xMutex, xBlockTime) \
        xQueueDgCRTakeMutexRecursive((xMutex), (xBlockTime))

/**
 * \brief Release semaphore
 *
 * This function is used for the calling co-routine to release a semaphore that has previously been
 * obtained by calling xSemaphoreDgCRTake().
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary(),
 * xSemaphoreCreateMutex() or xSemaphoreCreateCounting().
 *
 * \param [in] xSemaphore handle of the semaphore to be released
 *
 * \return pdPASS if the semaphore was released, otherwise an error code as defined within ProjDefs.h
 *
 * \sa xSemaphoreDgCRTake
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateMutex
 * \sa xSemaphoreCreateCounting
 */
#define xSemaphoreDgCRGive(xSemaphore) xQueueDgCRSend((xSemaphore), NULL, semGIVE_BLOCK_TIME)

/**
 * \brief Release recursively mutex type semaphore
 *
 * This function is used for the calling co-routine to recursively release a mutex type semaphore
 * that has previously been recursively obtained by calling xSemaphoreDgCRTakeRecursive().
 * Since a mutex used recursively can be 'taken' repeatedly be the owner, it does not become
 * available again until the owner has called xSemaphoreDgCRGiveRecursive() for each successful
 * 'take' request.
 *
 * The mutex must have previously been created using a call to xSemaphoreCreateRecursiveMutex().
 *
 * \param [in] xMutex handle of the mutex being released
 *
 * \return pdPASS if the mutex was released or its count was decremented, otherwise pdFAIL
 *
 * \sa xSemaphoreDgCRTakeRecursive
 * \sa xSemaphoreCreateRecursiveMutex
 */
#define xSemaphoreDgCRGiveRecursive(xMutex) xQueueDgCRGiveMutexRecursive((xMutex))

/**
 * \brief Obtain semaphore from ISR
 *
 * This function is used for obtaining a semaphore from interrupt service routine (ISR). It checks
 * whether the semaphore is available, and if it is, it obtains it.
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary()
 * or xSemaphoreCreateCounting().
 *
 * \param [in] xSemaphore handle of the semaphore to be obtained
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by obtaining the semaphore
 *
 * \return pdPASS if the semaphore was obtained, otherwise pdFAIL
 *
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateCounting
 */
#define xSemaphoreDgCRTakeFromISR(xSemaphore, pxCoRoutineWoken) \
        xQueueDgCRReceiveFromISR((xSemaphore), NULL, (pxCoRoutineWoken))

/**
 * \brief Check whether semaphore is available
 *
 * This function is used for checking from interrupt service routine (ISR) whether a semaphore is
 * available. The semaphore is not obtained.
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary()
 * or xSemaphoreCreateCounting().
 *
 * \param [in] xSemaphore handle of the semaphore to be checked
 *
 * \return pdPASS if the semaphore is available, otherwise pdFAIL
 *
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateCounting
 */
#define xSemaphoreDgCRCheckFromISR(xSemaphore) xQueueDgCRPeekFromISR((xSemaphore), NULL)

/**
 * \brief Release semaphore from ISR
 *
 * This function is used for releasing from interrupt service routine (ISR) a semaphore that has
 * previously been obtained by calling xSemaphoreDgCRTake() or xSemaphoreDgCRTakeFromISR().
 *
 * The semaphore must have previously been created using a call to xSemaphoreCreateBinary()
 * or xSemaphoreCreateCounting().
 *
 * \param [in] xSemaphore handle of the semaphore to be released
 * \param [in,out] pxCoRoutineWoken indication whether a co-routine has already been unblocked
 * from the current ISR by releasing the semaphore
 *
 * \return pdPASS if the semaphore was released, otherwise pdFAIL
 *
 * \sa xSemaphoreDgCRTake
 * \sa xSemaphoreDgCRTakeFromISR
 * \sa xSemaphoreCreateBinary
 * \sa xSemaphoreCreateCounting
 */
#define xSemaphoreDgCRGiveFromISR(xSemaphore, pxCoRoutineWoken) \
        xQueueDgCRSendFromISR((xSemaphore), NULL, (pxCoRoutineWoken))

#ifdef __cplusplus
}
#endif

#endif /* DIALOG_SEMAPHORE_H_ */
