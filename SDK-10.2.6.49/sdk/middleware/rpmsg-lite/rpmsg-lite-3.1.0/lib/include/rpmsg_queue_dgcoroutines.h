/**
 ****************************************************************************************
 *
 * @file rpmsg_queue_dgcoroutines.h
 *
 * @brief RPMsg-Lite queue receive functions adapted to Dialog Co-Routines logic
 *
 * Copyright (C) 2021 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef RPMSG_QUEUE_DGCOROUTINES_H_
#define RPMSG_QUEUE_DGCOROUTINES_H_

#ifdef OS_DGCOROUTINES

#include "rpmsg_lite.h"
#include "rpmsg_queue.h"

//! @addtogroup rpmsg_queue
//! @{

typedef struct
{
    uint32_t src;
    void *data;
    uint32_t len;
} _rpmsg_queue_rx_cb_data_t;

/* RL_API_HAS_ZEROCOPY has to be enabled for RPMsg Queue to work */
#if defined(RL_API_HAS_ZEROCOPY) && (RL_API_HAS_ZEROCOPY == 1)

/*******************************************************************************
 * API
 ******************************************************************************/

/* Exported API functions */

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * \note Not to be called by the user
 */
__STATIC_INLINE int32_t rpmsg_queue_recv_PART1(struct rpmsg_lite_instance *rpmsg_lite_dev,
                                               rpmsg_queue_handle q,
                                               char *data)
{
        int32_t retval = RL_SUCCESS;

        if (rpmsg_lite_dev == RL_NULL)
        {
                return RL_ERR_PARAM;
        }
        if (q == RL_NULL)
        {
                return RL_ERR_PARAM;
        }
        if (data == RL_NULL)
        {
                return RL_ERR_PARAM;
        }

        return retval;
}

/**
 * \note Not to be called by the user
 */
__STATIC_INLINE int32_t rpmsg_queue_recv_PART11(struct rpmsg_lite_instance *rpmsg_lite_dev,
                                               rpmsg_queue_handle q,
                                               char **data)
{
        int32_t retval = RL_SUCCESS;

        if (rpmsg_lite_dev == RL_NULL)
        {
                return RL_ERR_PARAM;
        }
        if (q == RL_NULL)
        {
                return RL_ERR_PARAM;
        }
        if (data == RL_NULL)
        {
                return RL_ERR_PARAM;
        }

        return retval;
}

/**
 * \note Not to be called by the user
 */
__STATIC_INLINE int32_t rpmsg_queue_recv_PART2(struct rpmsg_lite_instance *rpmsg_lite_dev,
                                               uint32_t *src,
                                               char *data,
                                               uint32_t maxlen,
                                               uint32_t *len,
                                               _rpmsg_queue_rx_cb_data_t msg)
{
        int32_t retval = RL_SUCCESS;

        if (src != RL_NULL)
        {
                *src = msg.src;
        }
        if (len != RL_NULL)
        {
                *len = msg.len;
        }

        if (maxlen >= msg.len)
        {
                env_memcpy(data, msg.data, msg.len);
        }
        else
        {
                retval = RL_ERR_BUFF_SIZE;
        }

        /* Release used buffer. */
        return ((RL_SUCCESS == rpmsg_lite_release_rx_buffer(rpmsg_lite_dev, msg.data)) ? retval : RL_ERR_PARAM);
}

/**
 * \note Not to be called by the user
 */
__STATIC_INLINE int32_t rpmsg_queue_recv_PART3(uint32_t *src,
                                               char **data,
                                               uint32_t *len,
                                               _rpmsg_queue_rx_cb_data_t msg)
{
        if (src != RL_NULL)
        {
           *src = msg.src;
        }
        if (len != RL_NULL)
        {
           *len = msg.len;
        }

        *data = msg.data;

        return RL_SUCCESS;
}

/*!
 * @brief
 * blocking receive function - blocking version of the received function that can be called from an RTOS task.
 * The data is copied from the receive buffer into the user supplied buffer.
 *
 * This is the "receive with copy" version of the RPMsg receive function. This version is simple
 * to use but it requires copying data from shared memory into the user space buffer.
 * The user has no obligation or burden to manage the shared memory buffers.
 *
 * @param rpmsg_lite_dev    RPMsg-Lite instance
 * @param[in] q             RPMsg queue handle to listen on
 * @param[in] data          Pointer to the user buffer the received data are copied to
 * @param[out] len          Pointer to an int variable that will contain the number of bytes actually copied into the
 * buffer
 * @param[in] maxlen        Maximum number of bytes to copy (received buffer size)
 * @param[out] src          Pointer to address of the endpoint from which data is received
 * @param[in] timeout       Timeout, in milliseconds, to wait for a message. A value of 0 means don't wait (non-blocking
 * call).
 *                          A value of 0xffffffff means wait forever (blocking call).
 *
 * @return Status of function execution
 *
 * @see rpmsg_queue_recv_nocopy
 */
#define rpmsg_queue_recv(rpmsg_lite_dev,                                                        \
                         q,                                                                     \
                         src,                                                                   \
                         data,                                                                  \
                         maxlen,                                                                \
                         len,                                                                   \
                         timeout,                                                               \
                         retval)                                                                \
do {                                                                                            \
        /* Definitions copied from the initial rpmsg_queue_recv() function */                   \
        _rpmsg_queue_rx_cb_data_t msg_val  = {0};                                               \
        *(retval)                          = RL_SUCCESS;                                        \
                                                                                                \
        /* Call the first part of the initial rpmsg_queue_recv() function */                    \
        *(retval) = rpmsg_queue_recv_PART1(rpmsg_lite_dev, q, data);                            \
                                                                                                \
        if (*(retval) != RL_ERR_PARAM) {                                                        \
                int32_t ret;                                                                    \
                env_get_queue((void *)q, &msg_val, timeout, &ret);                              \
                if (0 != ret) {                                                                 \
                        *(retval) = rpmsg_queue_recv_PART2(rpmsg_lite_dev,                      \
                                                           src,                                 \
                                                           data,                                \
                                                           maxlen,                              \
                                                           len,                                 \
                                                           msg_val);                            \
                } else {                                                                        \
                        *(retval) = RL_ERR_NO_BUFF;                                             \
                }                                                                               \
        }                                                                                       \
} while (0)

/*!
 * @brief
 * blocking receive function - blocking version of the received function that can be called from an RTOS task.
 * The data is NOT copied into the user-app. buffer.
 *
 * This is the "zero-copy receive" version of the RPMsg receive function. No data is copied.
 * Only the pointer to the data is returned. This version is fast, but it requires the user to manage
 * buffer allocation. Specifically, the user must decide when a buffer is no longer in use and
 * make the appropriate API call to free it, see rpmsg_queue_nocopy_free().
 *
 * @param rpmsg_lite_dev    RPMsg Lite instance
 * @param[in] q             RPMsg queue handle to listen on
 * @param[out] data         Pointer to the RPMsg buffer of the shared memory where the received data is stored
 * @param[out] len          Pointer to an int variable that that will contain the number of valid bytes in the RPMsg
 * buffer
 * @param[out] src          Pointer to address of the endpoint from which data is received
 * @param[in] timeout       Timeout, in milliseconds, to wait for a message. A value of 0 means don't wait (non-blocking
 * call).
 *                          A value of 0xffffffff means wait forever (blocking call).
 *
 * @return Status of function execution.
 *
 * @see rpmsg_queue_nocopy_free
 * @see rpmsg_queue_recv
 */
#define rpmsg_queue_recv_nocopy(rpmsg_lite_dev,                                                 \
                                q,                                                              \
                                src,                                                            \
                                data,                                                           \
                                len,                                                            \
                                timeout,                                                        \
                                retval)                                                         \
do {                                                                                            \
        /* Definitions copied from the initial rpmsg_queue_recv() function */                   \
        _rpmsg_queue_rx_cb_data_t msg_val  = {0};                                               \
        *(retval)                          = RL_SUCCESS;                                        \
                                                                                                \
        /* Call the first part of the initial rpmsg_queue_recv() function */                    \
        *(retval) = rpmsg_queue_recv_PART11(rpmsg_lite_dev, q, data);                           \
                                                                                                \
        if (*(retval) != RL_ERR_PARAM) {                                                        \
                int32_t ret;                                                                    \
                env_get_queue((void *)q, &msg_val, timeout, &ret);                              \
                if (0 != ret) {                                                                 \
                        *(retval) = rpmsg_queue_recv_PART3(src,                                 \
                                                           data,                                \
                                                           len,                                 \
                                                           msg_val);                            \
                }                                                                               \
                if (*(retval) != RL_SUCCESS) {                                                  \
                        *(retval) = RL_ERR_NO_BUFF;                                             \
                }                                                                               \
                                                                                                \
        }                                                                                       \
} while (0)

//! @}

#if defined(__cplusplus)
}
#endif

#endif /* RL_API_HAS_ZEROCOPY */

#endif /* OS_DGCOROUTINES */

#endif /* RPMSG_QUEUE_DGCOROUTINES_H_ */
