// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#ifndef _DRIVERS_DISK_H
#define _DRIVERS_DISK_H

#include <errno.h>
#include <proc/thread.h>
#include <types.h>

/** Sector size is hard-coded by MSIM. */
#define DISK_SECTOR_SIZE 512

/** Disk registers virtual address (map 1:1). */
#define DISK_ADDRESS 0x90001000

#define COMMAND_READ 1
#define COMMAND_WRITE (1 << 1)
#define STATUS_PENDING (1 << 2)
#define STATUS_ERROR (1 << 3)
#define INTERRUPT_BIT (1 << 9)

extern list_t waiting_queue_disks;

/**
 * This structure represents the memory-mapped registers of the disk controller.
 *
 * For more information, please refer to the MSIM `ddisk` device documentation.
 *
 * https://msim.readthedocs.io/en/latest/reference/devices.html#block-device-ddisk
 */
typedef struct {
    volatile uint32_t dma_addr_lo;
    volatile uint32_t sector;
    volatile uint32_t status_command;
    volatile uint32_t size_disk_lo;
    volatile uint32_t dma_addr_hi;
    volatile uint32_t reserved;
    volatile uint32_t size_disk_hi;
} disk_registers_t;

/** Information about a disk. */
typedef struct {
    disk_registers_t* registers;
    uint64_t disk_size;
} disk_t;

void disk_init(void);

disk_t* disk_get_current(void);
uint64_t disk_get_size(disk_t* disk);

errno_t disk_read_sector(disk_t* disk, uint32_t sector, void* buffer);
errno_t disk_write_sector(disk_t* disk, uint32_t sector, const void* buffer);

#endif
