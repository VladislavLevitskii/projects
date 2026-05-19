// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#include <np/fs.h>
#include <np/syscall.h>

/** Look up a file by path and return its inode number.
 *
 * Searches for a file in the root directory and returns its inode number.
 *
 * @param path File path (relative to root directory)
 * @return Inode number on success
 *         Negative error code on failure
 */
int fs_lookup(const char* path, uint16_t* out_ino) {
    volatile int err = -42;
    __SYSCALL3(SYSCALL_LOOKUP, (unative_t)path, (unative_t)out_ino, (unative_t)&err);
    return err;
}
