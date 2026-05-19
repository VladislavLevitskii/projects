// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#ifndef _IPC_MQUEUE_H
#define _IPC_MQUEUE_H

#include <adt/list.h>
#include <errno.h>
#include <lib/fourcc.h>
#include <proc/mutex.h>
#include <proc/sem.h>
#include <types.h>

#define MAX_QUEUES 16

typedef struct {
    fourcc_t name;
    size_t max_msgs;
    size_t msg_size;

    link_t link;

    list_t message_list;

    mutex_t lock;
    sem_t sem;
} mqueue_t;

typedef struct {
    link_t link;

    const void* message;
} message_t;

void mqueue_init(void);
errno_t mq_lookup_or_create(fourcc_t name, size_t max_msgs, size_t msg_size);
errno_t mq_send(fourcc_t name, const void* msg, size_t size);
errno_t mq_recv(fourcc_t name, void* buf, size_t* size);
errno_t mq_destroy(fourcc_t name);

errno_t mq_recv_blocking(fourcc_t name, void* buf, size_t* size);

#endif
