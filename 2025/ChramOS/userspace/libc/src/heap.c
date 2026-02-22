// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <np/list.h>
#include <np/proc.h>
#include <stdlib.h>

#define NULL_PAGE_OFFSET 2 * 4096

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

uintptr_t heap_start;
uintptr_t heap_end;
bool initialized = false;
list_t freelist;

static void heap_init(void) {
    heap_start = (uintptr_t)_app_end;

    np_proc_info_t info;
    bool valid = np_proc_info_get(&info);
    assert(valid);
    heap_end = info.virt_mem_size + NULL_PAGE_OFFSET;

    list_init(&freelist);
    freenode_header_t* node_header = (freenode_header_t*)heap_start;
    list_append(&freelist, &(node_header->link));
    node_header->size = heap_end - heap_start - sizeof(freenode_header_t);

    initialized = true;
}

#define MAGIC 0x12345678

/** Align given address up to the given power-of-two alignment. */
static inline uintptr_t uintptr_align_up(uintptr_t addr, unative_t align) {
    return (addr + align - 1) & ~(align - 1);
}

/** Align given address down to the given power-of-two alignment. */
static inline uintptr_t uintptr_align_down(uintptr_t addr, unative_t align) {
    return (addr) & ~(align - 1);
}

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

/** Allocate size bytes on heap.
 *
 * When size is zero, the behavior is implementation-defined with two options:
 * return NULL or return a unique pointer. In both of these cases, the returned
 * value can be later succesfully passed to free. Consult the C standard for
 * more details.
 *
 * @param size Amount of memory to allocate.
 * @return Pointer to allocated memory.
 * @retval NULL Out of memory.
 */
void* malloc(size_t size) {
    if (!initialized) {
        heap_init();
    }

    size = (size_t)uintptr_align_up(size, 32);
    list_foreach(freelist, freenode_header_t, link, node) {
        if (node->size >= size) {
            if (node->size - size > sizeof(*node)) {
                freenode_header_t* new_header = (freenode_header_t*)((char*)(node + 1) + size);
                new_header->size = (node->size) - size - sizeof(*new_header);
                link_init(&new_header->link);
                list_insert(&node->link, &new_header->link);
                list_remove(&node->link);

                alloc_header_t* alloc_header = (alloc_header_t*)node;
                alloc_header->size = size;
                alloc_header->magic = MAGIC;

                return (void*)(alloc_header + 1);
            } else {
                uint32_t node_size = node->size;
                list_remove(&node->link);
                alloc_header_t* alloc_header = (alloc_header_t*)node;
                alloc_header->size = node_size;
                alloc_header->magic = MAGIC;

                return (void*)(alloc_header + 1);
            }
        }
    }
    return NULL;
}

/** Free memory previously allocated by malloc.
 *
 * The ptr parameter must be either a pointer previously returned by a call
 * to malloc or NULL (in that case, no action is performed).
 *
 * @param ptr Pointer to memory to be freed.
 */
void free(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    if (!initialized) {
        assert(false);
        heap_init();
    }

    alloc_header_t* alloc_ptr = (alloc_header_t*)ptr;
    alloc_ptr--;
    assert(alloc_ptr->magic == MAGIC);
    uint32_t alloc_size = alloc_ptr->size;
    freenode_header_t* free_node_ptr = (freenode_header_t*)alloc_ptr;
    free_node_ptr->size = alloc_size;
    link_init(&free_node_ptr->link);

    link_t* prev_link = find_previous_node_link(free_node_ptr);

    if (prev_link == &freelist.head || prev_link == NULL)
        list_prepend(&freelist, &free_node_ptr->link);
    else
        list_insert(prev_link, &free_node_ptr->link);

    free_node_ptr = coalescing_left(prev_link, free_node_ptr);
    free_node_ptr = coalescing_right(free_node_ptr);
}
