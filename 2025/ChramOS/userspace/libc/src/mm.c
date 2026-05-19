// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#include <np/mm.h>
#include <np/syscall.h>

/** Map a file into memory.
 *
 * Maps a file into memory using the file's inode number and offset.
 *
 * @param size Size of the memory region to map
 * @param ino Inode number of the file
 * @param offset Offset within the file to map from
 * @return Pointer to the mapped memory region on success
 */
void* mmap(size_t size, uint16_t ino, uint32_t offset) {
    void* addr = NULL;
    volatile int err = -42;

    struct {
        size_t size;
        uint16_t ino;
        uint32_t offset;
    } args = { size, ino, offset };

    __SYSCALL3(SYSCALL_MMAP, (unative_t)&args, (unative_t)&addr, (unative_t)&err);

    if (err != 0) {
        return NULL;
    }
    return addr;
}
