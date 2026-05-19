// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#ifndef _LIBC_NP_FS_H
#define _LIBC_NP_FS_H

#include <np/types.h>

int fs_lookup(const char* path, uint16_t* out_ino);

#endif
