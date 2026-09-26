// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <adt/list.h>
#include <debug.h>
#include <drivers/csr.h>
#include <drivers/disk.h>
#include <drivers/machine.h>
#include <drivers/sv32.h>
#include <exc.h>
#include <fs/minix.h>
#include <mm/as.h>
#include <mm/frame.h>
#include <proc/process.h>
#include <proc/scheduler.h>

/** Handles general exception.
 *
 * The function receives a pointer to an exception context, which
 * represents a snapshot of CPU and (some) CSRs at the
 * time of the exception occurring.
 */
void handle_exception_general(exc_context_t* exc_context) {
    if (exc_context->scause & RV_INTERRUPT_BIT) {
        // interrupt
        if (exc_context->scause == EXC_CODE_STI) {
            // timer interrupt
            scheduler_schedule_next();
        } else if (exc_context->scause == EXC_CODE_DISK) {
            disk_get_current()->registers->status_command = STATUS_PENDING;
            if (!list_is_empty(&waiting_queue_disks)) {
                link_t* first_thread = list_pop(&waiting_queue_disks);
                list_append(&scheduler_list, first_thread);
                thread_t* first_t = list_item(first_thread, thread_t, scheduler_link);
                first_t->state = RUNNING;
            }
        } else {
            panic("Unhandled interrupt: scause=0x%x", exc_context->scause);
        }
    } else {
        // exception

        unative_t scause_exc = exc_context->scause & ~RV_INTERRUPT_BIT;

        if (scause_exc == EXC_CODE_LOAD_PAGE_FAULT || scause_exc == EXC_CODE_STORE_PAGE_FAULT || scause_exc == EXC_CODE_INSTRUCTION_PAGE_FAULT) {
            // page fault
            unative_t stval = exc_context->stval;

            unative_t fault_addr = stval;
            if (scause_exc == EXC_CODE_INSTRUCTION_PAGE_FAULT && stval == 0) {
                // RISC will store the address in sepc
                fault_addr = exc_context->sepc;
            }

            errno_t err = as_handle_page_fault(current_thread->as, fault_addr);

            if (err != EOK) {
                thread_kill(current_thread);
                return;
            }

        } else if (scause_exc == EXC_CODE_ECALL_U) {
            // user syscall
            handle_syscall(exc_context);
        } else if (scause_exc == EXC_CODE_ILLEGAL_INSTRUCTION) {
            // illegal instruction
            thread_kill(current_thread);
        } else {
            // unhandled exception
            panic("GG, unhandled exception: scause=0x%x", exc_context->scause);
        }
    }
}
