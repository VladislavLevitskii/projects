// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _MM_HEAP_H
#define _MM_HEAP_H

#include <adt/list.h>
#include <types.h>

extern uintptr_t heap_start;
extern uintptr_t heap_end;
extern list_t freelist;

void heap_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif
