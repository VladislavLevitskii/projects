// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#ifndef _LIB_MEMCOPY_H
#define _LIB_MEMCOPY_H

void mem_copy(uintptr_t from_addr, uintptr_t to_addr, size_t size);

void mem_copy(uintptr_t from_addr, uintptr_t to_addr, size_t size) {
    size_t four_bytes_count = size / 4;
    for (size_t i = 0; i < four_bytes_count; i++) {
        *((uint32_t*)to_addr + i) = *((uint32_t*)from_addr + i);
    }

    size_t bytes_copied = four_bytes_count * 4;
    from_addr += bytes_copied;
    to_addr += bytes_copied;
    size -= bytes_copied;

    for (size_t i = 0; i < size; i++) {
        *((int8_t*)to_addr + i) = *((int8_t*)from_addr + i);
    }
}

#endif