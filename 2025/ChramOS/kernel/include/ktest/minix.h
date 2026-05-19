// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#ifndef _KTEST_MINIX_H
#define _KTEST_MINIX_H

#include <drivers/disk.h>
#include <fs/minix.h>
#include <ktest.h>

static inline minixfs_t* ktest_minixfs_format_and_mount(disk_t* disk) {
    errno_t err = minixfs_format(disk);
    ktest_assert_errno(err, "minixfs_format");

    err = minixfs_init(disk);
    ktest_assert_errno(err, "minixfs_init");

    minixfs_t* fs = minixfs_get_current();

    err = minixfs_mount(fs);
    ktest_assert_errno(err, "minixfs_mount");

    return fs;
}

#endif // _KTEST_MINIX_H
