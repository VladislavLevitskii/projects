// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug.h>
#include <drivers/csr.h>
#include <drivers/machine.h>
#include <exc.h>
#include <mm/as.h>
#include <mm/frame.h>
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
        } else {
            panic("Unhandled interrupt: scause=0x%x", exc_context->scause);
        }
    } else {
        // exception

        unative_t scause_exc = exc_context->scause & ~RV_INTERRUPT_BIT;

        if (scause_exc == EXC_CODE_INSTRUCTION_PAGE_FAULT || scause_exc == EXC_CODE_LOAD_PAGE_FAULT || scause_exc == EXC_CODE_STORE_PAGE_FAULT) {
            // page fault

            unative_t satp = current_thread->as->satp_val;
            unative_t stval = exc_context->stval;
            size_t vpn1 = VPN1_FROM_VA(stval);
            size_t vpn0 = VPN0_FROM_VA(stval);

            uintptr_t root_table_phys = ROOT_TABLE_PHYS_FROM_SATP(satp);
            uint32_t root_pte = PTE_AT_INDEX(root_table_phys, vpn1);

            if (!(root_pte & PTE_VALID)) {
                // we don't create l1 tables on demand
                thread_kill(current_thread);
            }

            uintptr_t l1_table_phys = PA_FROM_PTE(root_pte);
            uint32_t l1_pte = PTE_AT_INDEX(l1_table_phys, vpn0);

            assert(!(l1_pte & PTE_VALID));

            if (!(l1_pte & PTE_RSW0)) {
                // this page is not allowed to be allocated on demand
                thread_kill(current_thread);
            }

            uintptr_t frame_phys;
            errno_t err = frame_alloc(1, &frame_phys);
            if (err != EOK) {
                // out of memory
                thread_kill(current_thread);
            }

            uint32_t new_l1_pte = CREATE_PTE(frame_phys, l1_pte | PTE_VALID);
            PTE_AT_INDEX(l1_table_phys, vpn0) = new_l1_pte;

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
