// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#include <np/mqueue.h>
#include <np/syscall.h>

/** Create or open a named message queue.
 *
 * Creates a new fixed-size-message queue with the given name, or opens the
 * existing queue with that name. Queues are intended to be used as MPSC queues:
 * multiple senders and one receiver.
 *
 * @param name Queue name.
 * @param max_msgs Maximum number of messages in queue.
 * @param msg_size Fixed message size in bytes.
 * @return 0 on success, error code on failure.
 */
int mq_lookup_or_create(fourcc_t name, int max_msgs, int msg_size) {
    volatile int err = -42;
    __SYSCALL4(SYSCALL_MQ_LOOKUP, (unative_t)name, (unative_t)max_msgs, (unative_t)msg_size, (unative_t)&err);
    return err;
}

/** Send a message to a queue (non-blocking).
 *
 * Copies one message into the kernel queue buffer. The message size must exactly
 * match the queue's configured message size. If the queue is full, the call
 * returns an error instead of blocking.
 *
 * @param name Queue name.
 * @param msg Pointer to message data.
 * @param size Size of message; must equal the queue's message size.
 * @return 0 on success, error code on failure.
 */
int mq_send(fourcc_t name, const void* msg, int size) {
    volatile int err = -42;
    __SYSCALL4(SYSCALL_MQ_SEND, (unative_t)name, (unative_t)msg, (unative_t)size, (unative_t)&err);
    return err;
}

/** Receive a message from a queue (non-blocking).
 *
 * Copies the next message from the queue into msg. If the queue is empty, the
 * call returns an error instead of blocking. The buffer must be large enough for
 * the queue's fixed message size.
 *
 * @param name Queue name.
 * @param msg Buffer to receive message data.
 * @param size In: size of message buffer. Out: number of bytes written.
 * @return 0 on success, error code on failure.
 */
int mq_recv(fourcc_t name, void* msg, size_t* size) {
    volatile int err = -42;
    __SYSCALL4(SYSCALL_MQ_RECV, (unative_t)name, (unative_t)msg, (unative_t)size, (unative_t)&err);
    return err;
}

/** Receive a message from a queue, blocking while the queue is empty.
 *
 * If the queue is empty, suspends the calling receiver until a sender enqueues a
 * message. Only one waiting receiver per queue is supported.
 *
 * @param name Queue name.
 * @param msg Buffer to receive message data.
 * @param size In: size of message buffer. Out: number of bytes written.
 * @return 0 on success, error code on failure.
 */
int mq_recv_blocking(fourcc_t name, void* msg, size_t* size) {
    volatile int err = -42;
    __SYSCALL4(SYSCALL_MQ_RECV_BLOCK, (unative_t)name, (unative_t)msg, (unative_t)size, (unative_t)&err);
    return err;
}

/** Destroy a message queue.
 *
 * Frees the queue storage. The caller must ensure no process is still using the
 * queue and no receiver is blocked on it.
 *
 * @param name Queue name.
 * @return 0 on success, error code on failure.
 */
int mq_destroy(fourcc_t name) {
    volatile int err = -42;
    __SYSCALL2(SYSCALL_MQ_DESTROY, (unative_t)name, (unative_t)&err);
    return err;
}
