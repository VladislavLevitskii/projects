// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <adt/bitmap.h>
#include <adt/bits.h>
#include <debug.h>
#include <exc.h>
#include <mm/frame.h>
#include <mm/heap.h>
#include <mm/self.h>
#include <types.h>

#define PROBE_JUMP_SIZE 1024
#define PROBE_MAGIC_NUMBER 0x31415927

static uintptr_t detect_end_of_available_memory() {
    volatile uint32_t* pointer = (volatile uint32_t*)kernel_get_image_end();
    while (true) {
        *pointer = PROBE_MAGIC_NUMBER;
        if (*pointer != PROBE_MAGIC_NUMBER) {
            pointer -= PROBE_JUMP_SIZE;
            break;
        }
        pointer += PROBE_JUMP_SIZE;
    }
    return (uintptr_t)pointer;
}

bitmap_t bitmap;
uintptr_t frames_start;
uintptr_t mem_end;

/**
 * Initializes frame allocator.
 *
 * Called once at system boot.
 */
void frame_init(void) {
    uintptr_t mem_start = (uintptr_t)kernel_get_image_end();
    mem_end = detect_end_of_available_memory();

    uint8_t* storage = (uint8_t*)mem_start;

    size_t max_frame_count = (mem_end - mem_start) / FRAME_SIZE;

    frames_start = uintptr_align_up((size_t)storage + BITMAP_GET_STORAGE_SIZE(max_frame_count), FRAME_SIZE);
    uintptr_t frames_end = uintptr_align_down(mem_end, FRAME_SIZE);

    bitmap_init(&bitmap, (frames_end - frames_start) / FRAME_SIZE, storage);
}

static errno_t frame_alloc_unsafe(size_t count, uintptr_t* phys) {
    assert(count > 0);

    size_t start_index;

    errno_t errno = bitmap_find_range(&bitmap, count, BITMAP_FREE, &start_index);
    if (errno == ENOENT) {
        *phys = (uintptr_t)NULL;
        return ENOMEM;
    }
    bitmap_fill_range(&bitmap, start_index, count);

    *phys = index_to_physical_address(start_index);

    return EOK;
}

/**
 * Allocate continuous sequence of physical frames.
 *
 * The allocated frames can be returned by frame_free.
 *
 * Note that frame_free must be given the number of frames being freed.
 * This means that the frame allocator does not need to record the number
 * of frames allocated (only their allocation state: allocated or free).
 *
 * The provided tests will always call this function with count being a power
 * of two (and at least 1). But the implementation may choose to not restrict
 * the caller with this requirement. The restriction is meant to allow
 * implementing a class of frame allocators which require it.
 *
 * @param count How many frames to allocate.
 * @param phys Output-value pointer to the physical address of the
 * first frame in sequence.
 * @return Error code.
 * @retval EOK Frames allocated.
 * @retval ENOMEM Not enough memory.
 */
errno_t frame_alloc(size_t count, uintptr_t* phys) {
    bool saved = interrupts_disable();
    errno_t result = frame_alloc_unsafe(count, phys);
    interrupts_restore(saved);
    return result;
}

size_t physical_address_to_index(uintptr_t phys) {
    return ((uintptr_t)phys - frames_start) / FRAME_SIZE;
}

uintptr_t index_to_physical_address(size_t index) {
    return frames_start + index * FRAME_SIZE;
}

static errno_t frame_free_unsafe(size_t count, uintptr_t phys) {
    assert(count > 0);

    if (uintptr_align_up(phys, FRAME_SIZE) != phys) {
        return ENOENT;
    }
    if (uintptr_align_down(phys, FRAME_SIZE) != phys) {
        return ENOENT;
    }

    size_t index = physical_address_to_index(phys);

    if (count + index > bitmap.length) {
        return ENOENT;
    }
    if (!bitmap_check_range_is(&bitmap, index, count, BITMAP_ALLOCATED)) {
        return EBUSY;
    }

    bitmap_clear_range(&bitmap, index, count);
    return EOK;
}

/**
 * Free continuous sequence of physical frames.
 *
 * The returned frames were previously allocated by frame_alloc.
 *
 * @param count How many frames to free.
 * @param phys Physical address of the first frame in sequence.
 * @return Error code.
 * @retval EOK Frames freed.
 * @retval ENOENT Invalid frame address.
 * @retval EBUSY Some frames were not allocated (double free).
 */
errno_t frame_free(size_t count, uintptr_t phys) {
    bool saved = interrupts_disable();
    errno_t result = frame_free_unsafe(count, phys);
    interrupts_restore(saved);
    return result;
}
