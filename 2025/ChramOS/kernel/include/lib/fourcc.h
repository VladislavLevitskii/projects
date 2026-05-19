// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#ifndef _LIB_FOURCC_H
#define _LIB_FOURCC_H

#include <types.h>

typedef uint32_t fourcc_t;
#define FOURCC(s) \
    ({ \
        _Static_assert(sizeof(s) == 5, "4CC string must be exactly 4 characters"); \
        ((fourcc_t)(s[0]) | ((fourcc_t)(s[1]) << 8) | ((fourcc_t)(s[2]) << 16) | ((fourcc_t)(s[3]) << 24)); \
    })

#endif
