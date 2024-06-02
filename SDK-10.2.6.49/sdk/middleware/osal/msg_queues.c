/**
 ****************************************************************************************
 *
 * @file msg_queues.c
 *
 * @brief Message queue API
 *
 * Copyright (C) 2015-2022 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifdef OS_PRESENT

#include <stdbool.h>
#include <string.h>
#include <sdk_defs.h>
#include <msg_queues.h>
#include <interrupts.h>

#if !defined(OS_FEATURE_SINGLE_STACK)
#if CONFIG_MSG_QUEUE_USE_ALLOCATORS
const content_allocator default_os_allocator = {
        .content_alloc = (MSG_ALLOC) MSG_QUEUE_MALLOC,
        .content_free = (MSG_FREE) MSG_QUEUE_FREE
};

#define QUEUE_ALLOC(queue, size) queue->allocator->content_alloc(size)
#define QUEUE_DEALLOCATOR(queue) queue->allocator->content_free

#else

#define QUEUE_ALLOC(queue, size) MSG_QUEUE_MALLOC(size)
#define QUEUE_DEALLOCATOR(queue) MSG_QUEUE_FREE

#endif
#endif

void msg_queue_create(msg_queue *queue, int queue_size, content_allocator *allocator)
{
#if defined(OS_FEATURE_SINGLE_STACK)
#pragma message "Revisit message queues implementation for single stack OSs." // XXX
        OS_ASSERT(0);
#else
        OS_QUEUE_CREATE(queue->queue, sizeof(msg), queue_size);
#if CONFIG_MSG_QUEUE_USE_ALLOCATORS
        queue->allocator = allocator;
#endif
#endif
}

void msg_queue_delete(msg_queue *queue)
{
#if defined(OS_FEATURE_SINGLE_STACK)
#pragma message "Revisit message queues implementation for single stack OSs." // XXX
        OS_ASSERT(0);
#else
        OS_QUEUE_DELETE(queue->queue);
#endif
}

int msg_queue_put(msg_queue *queue, msg *msg, OS_TICK_TIME timeout)
{
#if defined(OS_FEATURE_SINGLE_STACK)
#pragma message "Revisit message queues implementation for single stack OSs." // XXX
        return OS_QUEUE_FULL;
#else
        OS_BASE_TYPE ret = OS_QUEUE_FULL;

        if (in_interrupt()) {
                /* cppcheck-suppress unreadVariable */
                ret = OS_QUEUE_PUT_FROM_ISR(queue->queue, msg);
        } else {
                ret = OS_QUEUE_PUT(queue->queue, msg, timeout);
        }

        return ret;
#endif
}

int msg_queue_get(msg_queue *queue, msg *msg, OS_TICK_TIME timeout)
{
#if defined(OS_FEATURE_SINGLE_STACK)
#pragma message "Revisit message queues implementation for single stack OSs." // XXX
        return OS_QUEUE_EMPTY;
#else
        OS_BASE_TYPE ret = OS_QUEUE_EMPTY;

        if (in_interrupt()) {
                /* cppcheck-suppress unreadVariable */
                ret = OS_QUEUE_GET_FROM_ISR(queue->queue, msg);
        } else {
                ret = OS_QUEUE_GET(queue->queue, msg, timeout);
        }

        return ret;
#endif
}

void msg_init(msg *msg, MSG_ID id, MSG_TYPE type, void *buf, MSG_SIZE size, MSG_FREE free_cb)
{
#if defined(OS_FEATURE_SINGLE_STACK)
#pragma message "Revisit message queues implementation for single stack OSs." // XXX
        OS_ASSERT(0);
#else
        msg->id = id;
        msg->type = type;
        msg->data = buf;
        msg->size = size;
        msg->free_cb = free_cb;
#endif
}

void msg_release(msg *msg)
{
#if defined(OS_FEATURE_SINGLE_STACK)
#pragma message "Revisit message queues implementation for single stack OSs." // XXX
#else
        if (msg->free_cb) {
                msg->free_cb(msg->data);
                msg->free_cb = NULL;
        }
#endif
}

int msg_queue_init_msg(msg_queue *queue, msg *msg, MSG_ID id, MSG_TYPE type, MSG_SIZE size)
{
#if defined(OS_FEATURE_SINGLE_STACK)
#pragma message "Revisit message queues implementation for single stack OSs." // XXX
        return 0;
#else
        uint8_t *buf;

        buf = QUEUE_ALLOC(queue, size);
        if (buf == NULL) {
                return 0;
        }
        msg_init(msg, id, type, buf, size, QUEUE_DEALLOCATOR(queue));

        return 1;
#endif
}

int msg_queue_send(msg_queue *queue, MSG_ID id, MSG_TYPE type, void *buf, MSG_SIZE size,
                                                                        OS_TICK_TIME timeout)
{
#if defined(OS_FEATURE_SINGLE_STACK)
#pragma message "Revisit message queues implementation for single stack OSs." // XXX
        return OS_QUEUE_FULL;
#else
        msg msg;

        if (msg_queue_init_msg(queue, &msg, id, type, size) == 0) {
                return OS_QUEUE_FULL;
        }

        memcpy(msg.data, buf, size);
        if (msg_queue_put(queue, &msg, timeout) == OS_QUEUE_FULL) {
                msg_release(&msg);
                return OS_QUEUE_FULL;
        }

        return OS_QUEUE_OK;
#endif
}

int msq_queue_send_zero_copy(msg_queue *queue, MSG_ID id, MSG_TYPE type, void *buf,
                                        MSG_SIZE size, OS_TICK_TIME timeout, MSG_FREE free_cb)
{
#if defined(OS_FEATURE_SINGLE_STACK)
#pragma message "Revisit message queues implementation for single stack OSs." // XXX
        return OS_QUEUE_FULL;
#else
        msg msg;

        msg_init(&msg, id, type, buf, size, free_cb);

        if (msg_queue_put(queue, &msg, timeout) == OS_QUEUE_FULL) {
                msg_release(&msg);
                return OS_QUEUE_FULL;
        }

        return OS_QUEUE_OK;
#endif
}

#endif /* OS_PRESENT */
