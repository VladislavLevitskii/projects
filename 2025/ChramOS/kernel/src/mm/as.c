// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <adt/bitmap.h>
#include <drivers/printer.h>
#include <drivers/sv32.h>
#include <exc.h>
#include <mm/as.h>
#include <mm/frame.h>
#include <mm/heap.h>
#include <mm/self.h>

#define PROBE_JUMP_SIZE 1024
#define PROBE_MAGIC_NUMBER 0x31415927

mutex_t as_thread_count_mutex;
mutex_t asid_allocation_mutex;
bitmap_t taken_asids_bitmap;
size_t free_asids_count = HARDWARE_ASIDS_MAX;
size_t last_asid = 0;
uint8_t asids_bitmap_data[HARDWARE_ASIDS_MAX / 8];

_Static_assert(HARDWARE_ASIDS_MAX % 8 == 0);

/** Initializes support for address spaces.
 *
 * Called once at system boot.
 */
void as_init(void) {
    mutex_init(&as_thread_count_mutex);
    mutex_init(&asid_allocation_mutex);
    bitmap_init(&taken_asids_bitmap, HARDWARE_ASIDS_MAX, asids_bitmap_data);
    bitmap_set(&taken_asids_bitmap, 0, 1);
    free_asids_count--;
}

void as_acquire(thread_t* thread, as_t* as) {
    mutex_lock(&as->as_mutex);

    as->thread_count++;
    thread->as = as;

    mutex_unlock(&as->as_mutex);
}

void as_release(thread_t* thread) {
    mutex_lock(&thread->as->as_mutex);

    as_t* as = thread->as;
    thread->as = NULL;

    as->thread_count--;
    if (as->thread_count == 0) {
        as_destroy(as);
        return;
    }

    mutex_unlock(&as->as_mutex);
}

static size_t asid_acquire() {
    mutex_lock(&asid_allocation_mutex);

    size_t asid;
    if (free_asids_count != 0) {
        errno_t err_find = bitmap_find_range_with_hint(&taken_asids_bitmap, 1, 0, (last_asid + 1) % HARDWARE_ASIDS_MAX, &asid);
        assert(err_find == EOK);

        bitmap_set(&taken_asids_bitmap, asid, 1);
        free_asids_count--;
        last_asid = asid;
    } else {
        asid = 0;
    }

    mutex_unlock(&asid_allocation_mutex);
    return asid;
}

static void asid_release(size_t asid) {
    mutex_lock(&asid_allocation_mutex);

    assert(bitmap_is_set(&taken_asids_bitmap, asid));
    if (asid != 0) {
        bitmap_set(&taken_asids_bitmap, asid, 0);
        free_asids_count++;
    }

    mutex_unlock(&asid_allocation_mutex);
    return;
}

static void identity_map_megapage_for_addr(uintptr_t addr_phys, uintptr_t root_table_phys, size_t flags) {
    addr_phys = uintptr_align_down(addr_phys, MEGAPAGE_SIZE);
    size_t l0_index = addr_phys / MEGAPAGE_SIZE;

    assert((PTE_AT_INDEX(root_table_phys, l0_index) & (PTE_VALID | PTE_RSW0)) == 0);
    PTE_AT_INDEX(root_table_phys, l0_index) = CREATE_MEGAPAGE_PTE(addr_phys, flags);
}

static void dealloc_page_and_user_frames(uintptr_t root_table_phys) {
    for (size_t i = 0; i < PTE_COUNT / 2; i++) {
        uint32_t root_pte = PTE_AT_INDEX(root_table_phys, i);
        if (!(root_pte & PTE_VALID))
            continue;

        assert(!(root_pte & (PTE_READ | PTE_WRITE | PTE_EXECUTE)));

        // there is l1 page

        uintptr_t l1_table_phys = PA_FROM_PTE(root_pte);
        for (size_t j = 0; j < PTE_COUNT; j++) {
            uint32_t l1_pte = PTE_AT_INDEX(l1_table_phys, j);

            if (!(l1_pte & PTE_VALID))
                continue;

            uintptr_t frame_phys = PA_FROM_PTE(l1_pte);

            errno_t frame_err = frame_free(1, frame_phys);
            assert(frame_err == EOK);
        }

        errno_t l1_err = frame_free(1, l1_table_phys);
        assert(l1_err == EOK);
    }

    errno_t root_err = frame_free(1, root_table_phys);
    assert(root_err == EOK);
}

/** Create new address space of given size.
 *
 * The address space must start at address (PAGE_SIZE * PAGE_NULL_COUNT).
 *
 * @param size Address space size (in bytes), aligned to PAGE_SIZE.
 * @param flags Flags (unused).
 * @return New address space.
 * @retval NULL Out of memory.
 */
as_t* as_create(size_t size, unsigned int flags) {
    uintptr_t root_table_phys;
    errno_t err_root = frame_alloc(1, &root_table_phys);
    if (err_root == ENOMEM)
        return NULL;

    // alloc userspace
    {
        for (size_t i = 0; i < PTE_COUNT; i++) {
            PTE_AT_INDEX(root_table_phys, i) = 0;
        }

        size_t l1_ptes_count = size_align_up(size, FRAME_SIZE) / FRAME_SIZE + PAGE_NULL_COUNT;
        size_t l0_ptes_count = size_align_up(l1_ptes_count, PTE_COUNT) / PTE_COUNT;

        if (size == 0) {
            l0_ptes_count = 0; // if no pages, skip loop
        }

        for (size_t i = 0; i < l0_ptes_count; i++) {
            uintptr_t l1_table_phys;
            errno_t err_l1 = frame_alloc(1, &l1_table_phys);
            if (err_l1 == ENOMEM) {
                dealloc_page_and_user_frames(root_table_phys);
                return NULL;
            }

            size_t l0_flags = PTE_VALID | PTE_USER;
            PTE_AT_INDEX(root_table_phys, i) = CREATE_PTE(l1_table_phys, l0_flags);

            for (size_t j = 0; j < PTE_COUNT; j++) {
                if (i == 0 && j < 2) {
                    // skip null pages

                    PTE_AT_INDEX(l1_table_phys, j) = 0;
                } else if (i == l0_ptes_count - 1 && i * PTE_COUNT + j >= l1_ptes_count) {
                    // beyond requested size

                    PTE_AT_INDEX(l1_table_phys, j) = 0;
                } else {
                    // on-demand frame allocation

                    size_t l1_flags = PTE_READ | PTE_WRITE | PTE_EXECUTE | PTE_USER | PTE_RSW0;
                    PTE_AT_INDEX(l1_table_phys, j) = l1_flags;
                }
            }
        }
    }

    // identity map available memory
    {
        uintptr_t kernel_start_aligned_phys = uintptr_align_down(KERNEL_ENTRYPOINT, MEGAPAGE_SIZE);
        size_t mem_size = (size_t)uintptr_align_up(mem_end - kernel_start_aligned_phys, MEGAPAGE_SIZE);
        size_t l0_megapages_count = mem_size / MEGAPAGE_SIZE;

        size_t start_kernel_pte_index = kernel_start_aligned_phys / MEGAPAGE_SIZE;
        for (size_t i = 0; i < l0_megapages_count; i++) {
            size_t l0_flags = PTE_VALID | PTE_READ | PTE_WRITE | PTE_EXECUTE | PTE_GLOBAL;

            uintptr_t addr_phys = (start_kernel_pte_index + i) * MEGAPAGE_SIZE;
            PTE_AT_INDEX(root_table_phys, start_kernel_pte_index + i) = CREATE_MEGAPAGE_PTE(addr_phys, l0_flags);
        }
    }

    // identity map printer
    {
        size_t flags_printer = PTE_VALID | PTE_READ | PTE_WRITE | PTE_GLOBAL | PTE_EXECUTE;
        identity_map_megapage_for_addr(PRINTER_ADDRESS, root_table_phys, flags_printer);
    }

    // identity map loader
    {
        size_t flags_loader = PTE_VALID | PTE_READ;
        identity_map_megapage_for_addr(LOADER_ADDRESS, root_table_phys, flags_loader);
    }

    // app image
    {
        size_t flags_app_image = PTE_VALID | PTE_READ;
        identity_map_megapage_for_addr(APP_IMAGE_ADDRESS, root_table_phys, flags_app_image);
    }

    uintptr_t asid = asid_acquire();
    uintptr_t out_satp = CREATE_SATP(root_table_phys, asid);

    as_t* result_virt = kmalloc(sizeof(as_t));
    if (result_virt == NULL) {
        dealloc_page_and_user_frames(root_table_phys);
        return NULL;
    }

    result_virt->size = size_align_up(size, FRAME_SIZE);
    result_virt->satp_val = out_satp;
    result_virt->thread_count = 0;
    mutex_init(&result_virt->as_mutex);
    return result_virt;
}

/** Get the size (in bytes) of a given address space. */
size_t as_get_size(as_t* as) {
    return as->size;
}

/** Destroy a given address space, freeing all the memory.
 *
 * It is not expected that a user will call this function directly: an address
 * space should be destroyed when its last thread exits. Therefore one should
 * keep track of owning threads (such API is not defined by the tests and is
 * implementation specific) and when the counter reaches zero, call this
 * function.
 *
 * Note that this function is called explicitly only in unit tests for
 * the as_t structure.
 */
void as_destroy(as_t* as) {
    assert(as != NULL);
    assert(as->thread_count == 0);

    uintptr_t root_table_addr_phys = ROOT_TABLE_PHYS_FROM_SATP(as->satp_val);
    uintptr_t asid = ASID_FROM_SATP(as->satp_val);

    asid_release(asid);

    dealloc_page_and_user_frames(root_table_addr_phys);

    kfree(as);

    // we do not support TLB shootdown per ASID, so we do a full shootdown
    sv32_tlb_shootdown();
}

/** Get physical address of root page table.
 *
 * The function should return complete physical address, i.e. not only PPN.
 *
 * @param as Address space to use.
 * @return Physical address of root page table.
 */
uintptr_t as_get_root_page_table(as_t* as) {
    return ROOT_TABLE_PHYS_FROM_SATP(as->satp_val);
}
