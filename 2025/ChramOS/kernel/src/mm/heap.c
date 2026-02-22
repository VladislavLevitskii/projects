// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <adt/list.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/frame.h>
#include <mm/heap.h>
#include <mm/self.h>

#define PROBE_JUMP_SIZE 1024 // with uint32_t 4 KiB
#define PROBE_MAGIC_NUMBER 0x31415927
#define ALGINMEN_UP_VALUE 4
#define MAGIC 0x12345678

// #define BUMP_POINTER_IMPLEMENTATION

uintptr_t heap_start;
uintptr_t heap_end;

#ifdef BUMP_POINTER_IMPLEMENTATION
#define INIT_HEAP_FRAMES 128
uintptr_t bump_pointer;

#else
#define INIT_HEAP_FRAMES 1

list_t freelist;

typedef struct {
    uint32_t size;
    link_t link;
} freenode_header_t;

typedef struct {
    uint32_t magic;
    uint32_t size;
    uint32_t padding;
} alloc_header_t;

_Static_assert(sizeof(freenode_header_t) == sizeof(alloc_header_t));
#endif

/** Heap initialization.
 *
 * Called once during system boot.
 */
void heap_init(void) {
    errno_t errno = frame_alloc(INIT_HEAP_FRAMES, &heap_start);

    if (errno == ENOMEM)
        panic("cannot allocate frames for heap");

    heap_end = heap_start + INIT_HEAP_FRAMES * FRAME_SIZE;

#ifdef BUMP_POINTER_IMPLEMENTATION
    bump_pointer = heap_start;
#else
    list_init(&freelist);

    freenode_header_t* node_header = (freenode_header_t*)heap_start;
    list_append(&freelist, &(node_header->link));
    node_header->size = heap_end - heap_start - sizeof(freenode_header_t);
#endif
}

#ifndef BUMP_POINTER_IMPLEMENTATION
static link_t* find_previous_node_link(freenode_header_t* fp) {
    list_foreach(freelist, freenode_header_t, link, node) {
        if (node > fp) {
            return &freelist.head;
        } else if (node->link.next == &freelist.head) {
            return &node->link;
        } else if (node->link.next > &fp->link) {
            return &node->link;
        }
    }
    return NULL;
}

static freenode_header_t* coalescing_left(link_t* prev_link, freenode_header_t* fp) {
    if (prev_link != &freelist.head && prev_link != NULL) {
        freenode_header_t* prev_node = list_item(prev_link, freenode_header_t, link);
        uint32_t addr_diff = (uint32_t)fp - (uint32_t)prev_node;
        uint32_t block_size = sizeof(freenode_header_t) + prev_node->size;
        if (addr_diff == block_size) {
            prev_node->size += sizeof(freenode_header_t) + fp->size;
            list_remove(&fp->link);
            fp = prev_node;
        }
    }
    return fp;
}

static freenode_header_t* coalescing_right(freenode_header_t* fp) {
    if (fp->link.next != &freelist.head) {
        freenode_header_t* next_node = list_item(fp->link.next, freenode_header_t, link);
        uint32_t addr_diff = (uint32_t)next_node - (uint32_t)fp;
        uint32_t block_size = sizeof(freenode_header_t) + fp->size;
        if (addr_diff == block_size) {
            fp->size += sizeof(freenode_header_t) + next_node->size;
            list_remove(&next_node->link);
        }
    }
    return fp;
}

/**
 * Inserts a free block into the free list and coalesces it with adjacent free blocks if possible.
 */
static freenode_header_t* insert_free_block(freenode_header_t* free_node_ptr) {
    link_t* prev_link = find_previous_node_link(free_node_ptr);

    if (prev_link == &freelist.head || prev_link == NULL) {
        list_prepend(&freelist, &free_node_ptr->link);
    } else {
        list_insert(prev_link, &free_node_ptr->link);
    }

    free_node_ptr = coalescing_left(prev_link, free_node_ptr);

    free_node_ptr = coalescing_right(free_node_ptr);
    return free_node_ptr;
}

static void release_frame_both_boundaries(freenode_header_t* free_block) {
    size_t frames_start = physical_address_to_index((uintptr_t)free_block + FRAME_SIZE - 1); // align up
    size_t frames_end = physical_address_to_index(((uintptr_t)(free_block + 1)) + free_block->size); // align down

    if (frames_start >= frames_end) {
        assert(false); // this should never happen
        return;
    }

    size_t frames_count = frames_end - frames_start;

    // remove the whole free block
    list_remove(&free_block->link);

    errno_t errno = frame_free(frames_count, index_to_physical_address(frames_start));
    assert(errno == EOK);
}

static void release_frame_end_boundary(freenode_header_t* free_block) {
    uintptr_t header_end = (uintptr_t)(free_block + 1);
    uintptr_t free_block_end = header_end + free_block->size;
    size_t prev_free_block_size = free_block->size;

    size_t frames_start = physical_address_to_index(header_end + FRAME_SIZE - 1); // align up
    size_t frames_end = physical_address_to_index(free_block_end); // align down
    uintptr_t frame_start_addr = index_to_physical_address(frames_start);

    if (frames_start >= frames_end) {
        return;
    }

    size_t frames_count = frames_end - frames_start;

    // keep the starting part in the original free block, do not add ending part
    free_block->size = frame_start_addr - header_end;

    assert(prev_free_block_size == free_block->size + frames_count * FRAME_SIZE);

    errno_t errno = frame_free(frames_count, index_to_physical_address(frames_start));
    assert(errno == EOK);
}

static void release_frame_start_boundary(freenode_header_t* free_block) {
    uintptr_t free_block_almost_end = (uintptr_t)(free_block + 1) + free_block->size - sizeof(freenode_header_t);
    size_t prev_free_block_size = free_block->size;

    size_t frames_start = physical_address_to_index((uintptr_t)(free_block) + FRAME_SIZE - 1); // align up
    size_t frames_end = physical_address_to_index(free_block_almost_end); // align down
    uintptr_t frame_end_addr = index_to_physical_address(frames_end);

    if (frames_start >= frames_end) {
        return;
    }

    size_t frames_count = frames_end - frames_start;

    // create a new free block for the ending part
    freenode_header_t* new_free_node = (freenode_header_t*)frame_end_addr;
    new_free_node->size = free_block_almost_end - frame_end_addr;
    link_init(&new_free_node->link);
    list_insert(&free_block->link, &new_free_node->link);

    // remove the the original free block, do not add starting part
    list_remove(&free_block->link);

    assert(prev_free_block_size + sizeof(freenode_header_t) == frames_count * FRAME_SIZE + sizeof(freenode_header_t) + new_free_node->size);

    errno_t errno = frame_free(frames_count, index_to_physical_address(frames_start));
    assert(errno == EOK);
}

static void release_frame_no_boundary(freenode_header_t* free_block) {
    uintptr_t header_end = (uintptr_t)(free_block + 1);
    uintptr_t free_block_almost_end = header_end + free_block->size - sizeof(freenode_header_t);
    size_t prev_free_block_size = free_block->size;

    size_t frames_start = physical_address_to_index(header_end + FRAME_SIZE - 1); // align up
    size_t frames_end = physical_address_to_index(free_block_almost_end); // align down
    uintptr_t frame_start_addr = index_to_physical_address(frames_start);
    uintptr_t frame_end_addr = index_to_physical_address(frames_end);

    if (frames_start >= frames_end) {
        return;
    }

    size_t frames_count = frames_end - frames_start;

    // keep the starting part in the original free block
    free_block->size = frame_start_addr - header_end;

    // create a new free block for the ending part
    freenode_header_t* new_free_node = (freenode_header_t*)frame_end_addr;
    new_free_node->size = free_block_almost_end - frame_end_addr;
    link_init(&new_free_node->link);
    list_insert(&free_block->link, &new_free_node->link);

    assert(prev_free_block_size == free_block->size + frames_count * FRAME_SIZE + sizeof(freenode_header_t) + new_free_node->size);

    errno_t errno = frame_free(frames_count, frame_start_addr);
    assert(errno == EOK);
}

static void release_extra_heap_frames(freenode_header_t* free_block) {
    bool starting_at_frame_boundary = ((uintptr_t)free_block) % FRAME_SIZE == 0;
    bool ending_at_frame_boundary = ((uintptr_t)(free_block + 1) + free_block->size) % FRAME_SIZE == 0;

    if (ending_at_frame_boundary && starting_at_frame_boundary) {
        // both starting and ending at frame boundary
        release_frame_both_boundaries(free_block);
    } else if (ending_at_frame_boundary) {
        // ending at exactly frame boundary
        release_frame_end_boundary(free_block);
    } else if (starting_at_frame_boundary) {
        // starting at exactly frame boundary
        release_frame_start_boundary(free_block);
    } else {
        // neither starting nor ending at frame boundary
        release_frame_no_boundary(free_block);
    }
}

/**
 * Uses a free block to satisfy an allocation request, splitting it if necessary.
 * @param node The free block to use.
 * @param size The size of memory to allocate.
 * @param out_new_free_node Output parameter for the new free block if the original block is split
 * (NULL if not split). May be NULL if the caller is not interested in the new free block.
 * @return Pointer to the allocated memory.
 */
static void* use_free_block(freenode_header_t* node, size_t size, freenode_header_t** out_new_free_node) {
    assert(node->size >= size);

    if (node->size - size > sizeof(freenode_header_t)) {
        freenode_header_t* new_header = (freenode_header_t*)((char*)(node + 1) + size);
        new_header->size = (node->size) - size - sizeof(*new_header);
        link_init(&new_header->link);
        list_insert(&node->link, &new_header->link);
        list_remove(&node->link);

        alloc_header_t* alloc_header = (alloc_header_t*)node;
        alloc_header->size = size;
        alloc_header->magic = MAGIC;

        if (out_new_free_node != NULL)
            *out_new_free_node = new_header;
        return (void*)(alloc_header + 1);
    } else {
        uint32_t node_size = node->size;
        list_remove(&node->link);
        alloc_header_t* alloc_header = (alloc_header_t*)node;
        alloc_header->size = node_size;
        alloc_header->magic = MAGIC;

        if (out_new_free_node != NULL)
            *out_new_free_node = NULL;
        return (void*)(alloc_header + 1);
    }
}
#endif

static void* kmalloc_unsafe(size_t size) {
#ifdef BUMP_POINTER_IMPLEMENTATION
    uintptr_t temp = bump_pointer;
    size = (size_t)uintptr_align_up(size, ALGINMEN_UP_VALUE);
    bump_pointer += size;
    if (bump_pointer > heap_end) {
        bump_pointer -= size;
        return NULL;
    }
    return (void*)temp;
#else
    size = (size_t)uintptr_align_up(size, 32);
    list_foreach(freelist, freenode_header_t, link, node) {
        if (node->size >= size) {
            return use_free_block(node, size, NULL);
        }
    }

    uintptr_t new_node_addr;

    size_t needed_frames_count = (size + 2 * sizeof(freenode_header_t) + FRAME_SIZE - 1) / FRAME_SIZE;
    errno_t errno = frame_alloc(needed_frames_count, &new_node_addr);

    if (errno == ENOMEM)
        return NULL;

    freenode_header_t* free_node_ptr = (freenode_header_t*)new_node_addr;
    free_node_ptr->size = needed_frames_count * FRAME_SIZE - sizeof(freenode_header_t);
    link_init(&free_node_ptr->link);
    free_node_ptr = insert_free_block(free_node_ptr);

    assert(free_node_ptr->size >= size);

    freenode_header_t* remaining_free_node;
    void* return_value = use_free_block(free_node_ptr, size, &remaining_free_node);

    release_extra_heap_frames(remaining_free_node);

    return return_value;
#endif
}

/** Allocate size bytes on heap.
 *
 * The behavior when size is zero is not defined and none of the provided
 * tests will ever call kmalloc with 0 as the parameter. The implementation may
 * choose to return NULL or a unique pointer for such call (i.e., follow the
 * C standard). Note that causing a kernel panic or not checking for such
 * case at all is also a valid option.
 *
 * @param size Amount of memory to allocate.
 * @return Pointer to allocated memory.
 * @retval NULL Out of memory.
 */
void* kmalloc(size_t size) {
    bool saved = interrupts_disable();
    void* result = kmalloc_unsafe(size);
    interrupts_restore(saved);
    return result;
}

static void kfree_unsafe(void* ptr) {
#ifndef BUMP_POINTER_IMPLEMENTATION
    assert(ptr != NULL);

    alloc_header_t* alloc_ptr = (alloc_header_t*)ptr;
    alloc_ptr--;
    assert(alloc_ptr->magic == MAGIC);
    uint32_t alloc_size = alloc_ptr->size;
    freenode_header_t* free_node_ptr = (freenode_header_t*)alloc_ptr;
    free_node_ptr->size = alloc_size;
    link_init(&free_node_ptr->link);

    freenode_header_t* free_block = insert_free_block(free_node_ptr);
    release_extra_heap_frames(free_block);
#endif
}

/** Free memory previously allocated by kmalloc.
 *
 * The behavior when ptr is NULL is not defined and none of the provided
 * tests will ever call kfree with NULL as the parameter. Note that accepting
 * NULL as a parameter follows the C standard behavior, i.e. everything that
 * malloc can return can be passed to free. In the context of a kernel,
 * aborting the execution on receiving NULL is also an option as that might
 * signal an abnormal situation in a different module.
 *
 * @param ptr Pointer to memory to be freed.
 */
void kfree(void* ptr) {
    bool saved = interrupts_disable();
    kfree_unsafe(ptr);
    interrupts_restore(saved);
    return;
}
