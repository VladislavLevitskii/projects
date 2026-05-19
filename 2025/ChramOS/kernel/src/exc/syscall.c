// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <drivers/machine.h>
#include <drivers/printer.h>
#include <drivers/timer.h>
#include <exc.h>
#include <ipc/mqueue.h>
#include <lib/print.h>
#include <mm/as.h>
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
    SYSCALL_SPAWN_PROCESS,
    SYSCALL_LOOKUP,
    SYSCALL_WAIT_PROCESS,
    SYSCALL_MMAP,
    SYSCALL_MQ_LOOKUP,
    SYSCALL_MQ_SEND,
    SYSCALL_MQ_RECV,
    SYSCALL_MQ_RECV_BLOCK,
    SYSCALL_MQ_DESTROY,
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
        break;

    case SYSCALL_SPAWN_PROCESS:
        errno_t err_spawn_process = 0;
        process_t* process = kmalloc(sizeof(process_t));
        err_spawn_process = process_spawn(&process, (uint16_t)p1);
        *(unative_t*)p2 = process->pid;
        *(unative_t*)p3 = err_spawn_process;
        break;

    case SYSCALL_LOOKUP:
        errno_t err_lookup = minixfs_lookup(minixfs_get_current(), MINIX_ROOT_INO, (const char*)p1, (uint16_t*)p2);
        *(unative_t*)p3 = err_lookup;
        break;

    case SYSCALL_WAIT_PROCESS:
        errno_t err_wait_process = 0;
        pid_t wanted_pid = (pid_t)p1;
        list_foreach(process_list, process_t, link, process) {
            if (process->pid == wanted_pid) {
                err_wait_process = process_join(process, (int*)p2);
                *(unative_t*)p3 = err_wait_process;
                goto end;
                break;
            }
        }

        // invalid PID
        *(unative_t*)p3 = EINVAL;
        break;

    case SYSCALL_MMAP:
        struct mmap_args {
            size_t size;
            uint16_t ino;
            uint32_t offset;
        }* m_args = (struct mmap_args*)p1;

        uintptr_t* out_addr = (uintptr_t*)p2;
        unative_t* out_err = (unative_t*)p3;

        minix_inode_t inode;
        errno_t err = minixfs_read_inode(minixfs_get_current(), m_args->ino, &inode);

        if (err == EOK) {
            uintptr_t addr = 0;
            err = as_mmap(thread_get_current()->as, &addr, m_args->size, &inode, m_args->offset);

            if (err == EOK) {
                *out_addr = addr;
            }
        }

        *out_err = (unative_t)err;
        break;

    case SYSCALL_MQ_LOOKUP: {
        errno_t err;
        err = mq_lookup_or_create((fourcc_t)p1, (size_t)p2, (size_t)p3);
        *(unative_t*)p4 = err;
    } break;
    case SYSCALL_MQ_RECV: {
        errno_t err;
        err = mq_recv((fourcc_t)p1, (void*)p2, (size_t*)p3);
        *(unative_t*)p4 = err;
    } break;
    case SYSCALL_MQ_RECV_BLOCK: {
        errno_t err;
        err = mq_recv_blocking((fourcc_t)p1, (void*)p2, (size_t*)p3);
        *(unative_t*)p4 = err;
    } break;
    case SYSCALL_MQ_SEND: {
        errno_t err;
        err = mq_send((fourcc_t)p1, (const void*)p2, (size_t)p3);
        *(unative_t*)p4 = err;
    } break;
    case SYSCALL_MQ_DESTROY: {
        errno_t err;
        err = mq_destroy((fourcc_t)p1);
        *(unative_t*)p2 = err;
    } break;

    default:
        panic("Unknown syscall");
        break;
    }

end:

    // On success, shift EPC by 4 to resume execution of the interrupted
    // thread on the next instruction (we don't want to restart it).
    exc_context->sepc += 4;
}
