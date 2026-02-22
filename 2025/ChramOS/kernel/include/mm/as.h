// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _MM_AS_H
#define _MM_AS_H

#include <errno.h>
#include <proc/mutex.h>
#include <types.h>

/*
 * Setup amount of ASIDS that are actually supported by the hardware.
 *
 * We use a a simplification here: MSIM supports full 9 bits of ASIDs and
 * do not require our kernel to detect that.
 *
 * To speed up some of the tests we set this constant from tester.py so that
 * the implementation uses less values and thus more quickly recycles the
 * ASID numbers.
 */
#ifndef HARDWARE_ASIDS_MAX
#define HARDWARE_ASIDS_MAX 512
#endif

#define PAGE_SIZE 4096
#define PTE_SIZE 4
#define PTE_COUNT ((FRAME_SIZE) / (PTE_SIZE))
#define MEGAPAGE_SIZE ((PAGE_SIZE) * (PTE_COUNT))

#define PAGE_NULL_COUNT 2
#define VPN1_FROM_VA(va) ((va) >> 22)
#define VPN0_FROM_VA(va) (((va) >> 12) & 0x3FF)

#define ROOT_TABLE_PHYS_FROM_SATP(satp) ((satp) << 12)
#define CREATE_SATP(root_table_phys, asid) (((root_table_phys) >> 12) | (1 << 31) | ((asid) << 22))
#define ASID_FROM_SATP(satp) (((satp) >> 22) & 0x1FF)

#define PA_FROM_PTE(pte) (((pte) & 0xFFFFFC00) << 2)
#define CREATE_PTE(pa, flags) ((((pa) >> 12) << 10) | ((flags) & 0x3FF))
#define CREATE_MEGAPAGE_PTE(pa, flags) ((((pa) >> 22) << 20) | ((flags) & 0x3FF))

#define PTE_VALID (1 << 0)
#define PTE_READ (1 << 1)
#define PTE_WRITE (1 << 2)
#define PTE_EXECUTE (1 << 3)
#define PTE_USER (1 << 4)
#define PTE_GLOBAL (1 << 5)
#define PTE_ACCESSED (1 << 6)
#define PTE_DIRTY (1 << 7)
#define PTE_RSW0 (1 << 8)
#define PTE_RSW1 (1 << 9)

// for m01 tests
#define LOADER_ADDRESS (0xF0000000)

#define APP_IMAGE_ADDRESS (0xB8000000)

#define PTE_AT_INDEX(frame_addr_phys, index) (*(uint32_t*)((frame_addr_phys) + ((index) * PTE_SIZE)))

#define VAS_START (PAGE_NULL_COUNT * PAGE_SIZE)

typedef struct as {
    size_t size;
    unative_t satp_val;
    size_t thread_count;
    mutex_t as_mutex;
} as_t;

void as_init(void);
as_t* as_create(size_t size, unsigned int flags);
size_t as_get_size(as_t* as);
void as_destroy(as_t* as);
uintptr_t as_get_root_page_table(as_t* as);

void as_acquire(thread_t* thread, as_t* as);
void as_release(thread_t* thread);

#endif
