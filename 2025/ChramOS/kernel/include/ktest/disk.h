// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#ifndef _KTEST_DISK_H
#define _KTEST_DISK_H

#include <debug.h>
#include <drivers/disk.h>
#include <proc/thread.h>

static inline uint8_t ktest_disk_value_at(uint32_t sector, uint32_t offset) {
    /*
     * We combine the two values and then we repeatedly mix higher bits
     * into the lower ones and combine them with the lower bis again.
     *
     * Constants are somewhat inspired by MurmurHash as offered by ChatGPT.
     *
     * Emperically tested to spread the values relatively uniformly
     * across the whole range.
     */
    uint32_t x = (sector << 9) ^ offset;
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    x ^= (sector * 0x9e3779b1U);

    return (uint8_t)(x & 0xFF);
}

/*
 * Writes pseudorandom data to a sector and then verifies they were read back.
 *
 * - buffer must be at least DISK_SECTOR_SIZE big
 * - the function is static inline only not to be reported as unsed
 */
static inline void ktest_write_and_read_back_sector(disk_t* disk, uint32_t sector, uint8_t* buffer) {
    for (size_t offset = 0; offset < DISK_SECTOR_SIZE; offset++) {
        buffer[offset] = ktest_disk_value_at(sector, offset);
    }

    errno_t err = disk_write_sector(disk, sector, buffer);
    ktest_assert_errno(err, "disk_write_sector");

    for (size_t offset = 0; offset < DISK_SECTOR_SIZE; offset++) {
        buffer[offset] = ~buffer[offset];
    }

    err = disk_read_sector(disk, sector, buffer);
    ktest_assert_errno(err, "disk_read_sector");

    for (size_t offset = 0; offset < DISK_SECTOR_SIZE; offset++) {
        uint8_t expected = ktest_disk_value_at(sector, offset);
        ktest_assert(buffer[offset] == expected,
                "Sector %u: mismatch at byte %u: expected 0x%02x, got 0x%02x",
                sector, offset,
                expected, buffer[offset]);
    }
}

#endif
