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

static vma_t* find_vma_for_address(as_t* as, unative_t fault_addr) {
    list_foreach(as->vma_list, vma_t, link, vma) {
        if (fault_addr >= vma->start && fault_addr < vma->start + vma->size) {
            return vma;
        }
    }
    return NULL;
}

static uintptr_t get_or_allocate_l1_table(uintptr_t root_table_phys, size_t vpn1) {
    uint32_t root_pte = PTE_AT_INDEX(root_table_phys, vpn1);

    if (!(root_pte & PTE_VALID)) {
        uintptr_t new_l1_phys;
        if (frame_alloc(1, &new_l1_phys) != EOK) {
            return 0;
        }

        empty_buffer((void*)new_l1_phys, FRAME_SIZE);

        PTE_AT_INDEX(root_table_phys, vpn1) = CREATE_PTE(new_l1_phys, PTE_VALID | PTE_USER);
        root_pte = PTE_AT_INDEX(root_table_phys, vpn1);
    }

    return PA_FROM_PTE(root_pte);
}

static uintptr_t allocate_and_fill_frame(vma_t* valid_vma, unative_t fault_addr) {
    uintptr_t frame_phys = 0;
    if (frame_alloc(1, &frame_phys) != EOK) {
        return 0;
    }

    empty_buffer((void*)frame_phys, FRAME_SIZE);

    if (valid_vma->disk_backed) {
        uintptr_t faulting_page_va = fault_addr & ~(FRAME_SIZE - 1);
        size_t offset_ve_vma = faulting_page_va - valid_vma->start;
        size_t file_offset = valid_vma->offset + offset_ve_vma;

        size_t bytes_to_read = FRAME_SIZE;
        if (file_offset >= valid_vma->ino.i_size) {
            bytes_to_read = 0;
        } else if (file_offset + bytes_to_read > valid_vma->ino.i_size) {
            bytes_to_read = valid_vma->ino.i_size - file_offset;
        }

        void* kernel_dst_ptr = (void*)frame_phys;
        size_t actually_read = 0;

        if (bytes_to_read > 0) {
            minixfs_read_file(minixfs_get_current(),
                    &valid_vma->ino,
                    kernel_dst_ptr,
                    file_offset,
                    bytes_to_read,
                    &actually_read);
        }

        if (actually_read < FRAME_SIZE) {
            empty_buffer((void*)((uintptr_t)kernel_dst_ptr + actually_read), FRAME_SIZE - actually_read);
        }
    }

    return frame_phys;
}

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
            bool saved = interrupts_disable();
            if (!list_is_empty(&waiting_queue_disks)) {
                link_t* first_thread = list_pop(&waiting_queue_disks);
                list_append(&scheduler_list, first_thread);
                thread_t* first_t = list_item(first_thread, thread_t, scheduler_link);
                first_t->state = RUNNING;
            }

            interrupts_restore(saved);
        } else {
            panic("Unhandled interrupt: scause=0x%x", exc_context->scause);
        }
    } else {
        // exception

        unative_t scause_exc = exc_context->scause & ~RV_INTERRUPT_BIT;

        if (scause_exc == EXC_CODE_LOAD_PAGE_FAULT || scause_exc == EXC_CODE_STORE_PAGE_FAULT || scause_exc == EXC_CODE_INSTRUCTION_PAGE_FAULT) {
            // page fault
            unative_t satp = current_thread->as->satp_val;
            unative_t stval = exc_context->stval;

            unative_t fault_addr = stval;
            if (scause_exc == EXC_CODE_INSTRUCTION_PAGE_FAULT && stval == 0) {
                // RISC will store the address in sepc
                fault_addr = exc_context->sepc;
            }

            uintptr_t root_table_phys = ROOT_TABLE_PHYS_FROM_SATP(satp);
            size_t vpn1 = VPN1_FROM_VA(fault_addr);

            vma_t* valid_vma = find_vma_for_address(current_thread->as, fault_addr);
            if (valid_vma == NULL) {
                thread_kill(current_thread);
                return;
            }

            uintptr_t l1_table_phys = get_or_allocate_l1_table(root_table_phys, vpn1);
            if (l1_table_phys == 0) {
                thread_kill(current_thread);
                return;
            }

            uintptr_t frame_phys = allocate_and_fill_frame(valid_vma, fault_addr);
            if (frame_phys == 0) {
                thread_kill(current_thread);
                return;
            }

            size_t vpn0 = VPN0_FROM_VA(fault_addr);
            uint32_t new_l1_pte = CREATE_PTE(frame_phys, PTE_VALID | PTE_USER | PTE_READ | PTE_WRITE | PTE_EXECUTE);
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
