// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <drivers/machine.h>
#include <drivers/printer.h>
#include <drivers/timer.h>
#include <exc.h>
#include <lib/print.h>
#include <mm/heap.h>
#include <proc/process.h>
#include <proc/thread.h>

static void handle_proc_info_get(unative_t* id, size_t* total_tics, size_t* virt_mem_size, bool* valid) {
    process_t* proc = thread_get_current()->owner_process;

    if (id == NULL || (uintptr_t)virt_mem_size >= (PAGE_NULL_COUNT * PAGE_SIZE) + proc->image_size || (uintptr_t)id < PAGE_NULL_COUNT * PAGE_SIZE) {
        // invalid pointers || points out of the application memory region || points to null page region
        *valid = false;
        return;
    }
    *id = (unative_t)proc;
    *total_tics = ++proc->tick_count;
    *virt_mem_size = thread_get_current()->as->size;
    *valid = true;
}

/** Available system calls.
 *
 * Must be kept up-to-date with userspace list.
 */
typedef enum {
    SYSCALL_EXIT,
    SYSCALL_PUTCHAR,
    SYSCALL_ASSERT,
    SYSCALL_PROC_INFO_GET,
    SYSCALL_LAST
} syscall_t;

/** Handles a syscall.
 *
 * The function receives a pointer to an exception context, which
 * represents the snapshot of CPU and (some) CP0 registers at the
 * time of the syscall.
 */
void handle_syscall(exc_context_t* exc_context) {
    syscall_t id = (syscall_t)exc_context->a0;
    unative_t p1 = exc_context->a1;
    unative_t p2 = exc_context->a2;
    unative_t p3 = exc_context->a3;
    unative_t p4 = exc_context->a4;

    switch (id) {
    case SYSCALL_EXIT:
        int* retval = kmalloc(sizeof(int));
        if (retval == NULL) {
            // if we cannot allocate memory for retval, just kill the thread (we have more problems anyway)
            thread_kill(thread_get_current());
        }

        *retval = (int)p1;
        thread_finish(retval);
        break;
    case SYSCALL_PUTCHAR:
        printer_putchar((char)p1);
        break;
    case SYSCALL_ASSERT:
        assert(p1);
        break;
    case SYSCALL_PROC_INFO_GET:
        handle_proc_info_get((unative_t*)p1, (size_t*)p2, (size_t*)p3, (bool*)p4);
    default:
        break;
    }

    // On success, shift EPC by 4 to resume execution of the interrupted
    // thread on the next instruction (we don't want to restart it).
    exc_context->sepc += 4;
}
