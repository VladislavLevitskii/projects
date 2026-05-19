// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#ifndef _LIBC_MQUEUE_H
#define _LIBC_MQUEUE_H

#include <np/types.h>
#include <stddef.h>

int mq_lookup_or_create(fourcc_t name, int max_msgs, int msg_size);
int mq_send(fourcc_t name, const void* msg, int size);
int mq_recv(fourcc_t name, void* msg, size_t* size);
int mq_destroy(fourcc_t name);

int mq_recv_blocking(fourcc_t name, void* msg, size_t* size);

#endif
