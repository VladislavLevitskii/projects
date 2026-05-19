// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#ifndef _LIBC_NP_MM_H
#define _LIBC_NP_MM_H

#include <np/types.h>
#include <stddef.h>

void* mmap(size_t size, uint16_t ino, uint32_t offset);

#endif
