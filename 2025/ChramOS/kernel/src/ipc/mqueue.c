// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#include <exc.h>
#include <fs/minix.h>
#include <ipc/mqueue.h>
#include <lib/mm.h>
#include <mm/heap.h>
#include <proc/mutex.h>
#include <proc/sem.h>

list_t queue_list;
mutex_t queue_list_lock;

/** Initialize message queues
 *
 * This function should be called once during kernel initialization.
 */
void mqueue_init(void) {
    list_init(&queue_list);
    mutex_init(&queue_list_lock);
}

/** Create or open a named message queue.
 *
 * If a queue with the given name already exists, the call succeeds and leaves
 * the existing queue unchanged. The implementation does not track ownership;
 * by convention, the queue is used as an MPSC queue with a single receiver.
 *
 * @param name     Queue name (fourcc).
 * @param max_msgs Capacity in number of messages.
 * @param msg_size Fixed size of each message slot in bytes.
 * @return Error code.
 * @retval EOK    Queue created or already exists.
 * @retval EINVAL Invalid parameters.
 * @retval ENOMEM Allocation failed.
 */
errno_t mq_lookup_or_create(fourcc_t name, size_t max_msgs, size_t msg_size) {
    mutex_lock(&queue_list_lock);

    list_foreach(queue_list, mqueue_t, link, queue) {
        if (name == queue->name) {
            mutex_unlock(&queue_list_lock);
            return EOK;
        }
    }

    mqueue_t* new_queue = kmalloc(sizeof(mqueue_t));
    if (new_queue == NULL) {
        mutex_unlock(&queue_list_lock);
        return ENOMEM;
    }

    new_queue->max_msgs = max_msgs;
    new_queue->msg_size = msg_size;
    new_queue->name = name;
    list_init(&new_queue->message_list);

    mutex_init(&new_queue->lock);
    sem_init(&new_queue->sem, 0);

    list_append(&queue_list, &new_queue->link);

    mutex_unlock(&queue_list_lock);
    return EOK;
}

/** Send a message to a queue (non-blocking).
 *
 * @param name Name of the queue.
 * @param msg  Message data; must point to exactly size bytes.
 * @param size Size of msg; must equal the queue's msg_size.
 * @return Error code.
 * @retval EOK    Message enqueued.
 * @retval EBUSY  Queue is full.
 */
errno_t mq_send(fourcc_t name, const void* msg, size_t size) {
    mutex_lock(&queue_list_lock);
    mqueue_t* active_queue = NULL;
    list_foreach(queue_list, mqueue_t, link, queue) {
        if (name == queue->name) {
            active_queue = queue;
            break;
        }
    }
    mutex_unlock(&queue_list_lock);

    if (!active_queue) {
        return EINVAL;
    }

    assert(size == active_queue->msg_size);

    mutex_lock(&active_queue->lock);

    if (list_get_size(&active_queue->message_list) == active_queue->max_msgs) {
        mutex_unlock(&active_queue->lock);
        return EBUSY;
    }

    message_t* new_message = kmalloc(sizeof(message_t));
    if (new_message == NULL) {
        mutex_unlock(&active_queue->lock);
        return ENOMEM;
    }

    new_message->message = kmalloc(active_queue->msg_size);
    if (new_message->message == NULL) {
        kfree(new_message);
        mutex_unlock(&active_queue->lock);
        return ENOMEM;
    }

    mem_copy((uintptr_t)msg, (uintptr_t)new_message->message, active_queue->msg_size);

    list_append(&active_queue->message_list, &new_message->link);

    mutex_unlock(&active_queue->lock);
    sem_post(&active_queue->sem);

    return EOK;
}

/** Receive a message from a queue (non-blocking).
 *
 * @param name Name of the queue.
 * @param buf  Buffer to copy the message into; must be at least msg_size bytes.
 * @param size In: size of message buffer. Out: number of bytes written.
 * @return Error code.
 * @retval EOK    Message dequeued.
 * @retval EBUSY  Queue is empty.
 */
errno_t mq_recv(fourcc_t name, void* buf, size_t* size) {
    mutex_lock(&queue_list_lock);
    mqueue_t* active_queue = NULL;
    list_foreach(queue_list, mqueue_t, link, queue) {
        if (name == queue->name) {
            active_queue = queue;
            break;
        }
    }
    mutex_unlock(&queue_list_lock);

    if (!active_queue) {
        return EINVAL;
    }

    errno_t err = sem_trywait(&active_queue->sem);
    if (err != EOK) {
        return EBUSY;
    }

    mutex_lock(&active_queue->lock);

    if (list_is_empty(&active_queue->message_list)) {
        mutex_unlock(&active_queue->lock);
        return EBUSY;
    }

    link_t* first_message_link = list_pop(&active_queue->message_list);
    message_t* message = list_item(first_message_link, message_t, link);

    mutex_unlock(&active_queue->lock);

    mem_copy((uintptr_t)message->message, (uintptr_t)buf, active_queue->msg_size);
    *size = active_queue->msg_size;

    kfree((void*)message->message);
    kfree(message);

    return EOK;
}

/** Receive a message from a queue, blocking while the queue is empty.
 *
 * @param name Name of the queue.
 * @param buf  Buffer to copy the message into; must be at least msg_size bytes.
 * @param size In: size of message buffer. Out: number of bytes written.
 * @return Error code.
 * @retval EOK    Message dequeued.
 * @retval EINVAL Queue not found or invalid output pointer.
 */
errno_t mq_recv_blocking(fourcc_t name, void* buf, size_t* size) {
    mutex_lock(&queue_list_lock);
    mqueue_t* active_queue = NULL;
    list_foreach(queue_list, mqueue_t, link, queue) {
        if (name == queue->name) {
            active_queue = queue;
            break;
        }
    }
    mutex_unlock(&queue_list_lock);

    if (!active_queue) {
        return EINVAL;
    }

    sem_wait(&active_queue->sem);

    mutex_lock(&active_queue->lock);
    link_t* first_message_link = list_pop(&active_queue->message_list);
    message_t* message = list_item(first_message_link, message_t, link);
    mutex_unlock(&active_queue->lock);

    mem_copy((uintptr_t)message->message, (uintptr_t)buf, active_queue->msg_size);
    *size = active_queue->msg_size;

    kfree((void*)message->message);
    kfree(message);

    return EOK;
}

/** Destroy a message queue.
 *
 * @param name Name of the queue
 * @return Error code
 * @retval EOK    Queue destroyed
 * @retval EINVAL Queue not found
 */
errno_t mq_destroy(fourcc_t name) {
    mutex_lock(&queue_list_lock);
    mqueue_t* deleting_queue = NULL;
    list_foreach(queue_list, mqueue_t, link, queue) {
        if (name == queue->name) {
            deleting_queue = queue;
            break;
        }
    }

    if (!deleting_queue) {
        mutex_unlock(&queue_list_lock);
        return EINVAL;
    }

    list_remove(&deleting_queue->link);
    mutex_unlock(&queue_list_lock);

    while (!list_is_empty(&deleting_queue->message_list)) {
        link_t* lnk = list_pop(&deleting_queue->message_list);
        message_t* msg_to_delete = list_item(lnk, message_t, link);

        kfree((void*)msg_to_delete->message);
        kfree(msg_to_delete);
    }

    kfree(deleting_queue);

    return EOK;
}
