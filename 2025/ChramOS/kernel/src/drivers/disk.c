// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#include <adt/list.h>
#include <drivers/csr.h>
#include <drivers/disk.h>
#include <errno.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/heap.h>
#include <proc/scheduler.h>

list_t waiting_queue_disks;

/** Initialize the disk driver.
 *
 * Called once at system boot.
 */
void disk_init() {
    // enables interrupts
    list_init(&waiting_queue_disks);
    csr_set(sie, INTERRUPT_BIT);
}

/**
 * Get the disk currently being used by the system.
 */
disk_t* disk_get_current(void) {
    static disk_t* current_disk = NULL;

    if (current_disk != NULL) {
        return current_disk;
    }

    current_disk = kmalloc(sizeof(disk_t));
    if (current_disk == NULL) {
        return NULL;
    }

    current_disk->registers = (disk_registers_t*)DISK_ADDRESS;

    uint64_t disk_size = current_disk->registers->size_disk_hi;
    current_disk->disk_size = (disk_size << 32) | current_disk->registers->size_disk_lo;

    return current_disk;
}

/**
 * Get the size of the disk in bytes.
 */
uint64_t disk_get_size(disk_t* disk) {
    return disk->disk_size;
}

/** Read a sector from the disk into the provided buffer.
 *
 * This function should block until the read operation is complete. The buffer
 * must be at least DISK_SECTOR_SIZE bytes long.
 *
 * @param disk Disk to read from.
 * @param sector Sector number to read.
 * @param buffer Buffer to read the data into.
 * @return Error code.
 */
errno_t disk_read_sector(disk_t* disk, uint32_t sector, void* buffer) {
    bool saved = interrupts_disable();

    assert(sector < disk_get_size(disk) / DISK_SECTOR_SIZE);

    list_remove(&thread_get_current()->scheduler_link);
    list_append(&waiting_queue_disks, &thread_get_current()->scheduler_link);

    disk->registers->sector = sector;
    disk->registers->dma_addr_lo = (uint32_t)buffer;
    disk->registers->dma_addr_hi = 0;

    disk->registers->status_command = COMMAND_READ;

    interrupts_restore(saved);
    scheduler_schedule_next();

    return EOK;
}

/** Write a sector to the disk from the provided buffer.
 *
 * This function should block until the write operation is complete. The buffer
 * must be at least DISK_SECTOR_SIZE bytes long.
 *
 * @param disk Disk to write to.
 * @param sector Sector number to write.
 * @param buffer Buffer containing the data to write.
 * @return Error code.
 */
errno_t disk_write_sector(disk_t* disk, uint32_t sector, const void* buffer) {
    bool saved = interrupts_disable();
    list_remove(&thread_get_current()->scheduler_link);
    list_append(&waiting_queue_disks, &thread_get_current()->scheduler_link);

    disk->registers->sector = sector;
    disk->registers->dma_addr_lo = (uint32_t)buffer;
    disk->registers->dma_addr_hi = 0;

    disk->registers->status_command = COMMAND_WRITE;

    interrupts_restore(saved);
    scheduler_schedule_next();
    return EOK;
}