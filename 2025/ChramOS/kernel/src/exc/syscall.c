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

typedef struct mmap_args {
    size_t size;
    uint16_t ino;
    uint32_t offset;
} mmap_args_t;

#define REQUIRE_VALID_PTR(ptr, size) \
    do { \
        if (!pointer_in_vma((ptr), (size))) { \
            thread_kill(thread_get_current()); \
            return; \
        } \
    } while (0)

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

typedef void (*syscall_handler_t)(unative_t p1, unative_t p2, unative_t p3, unative_t p4);

static void sys_exit(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    int* retval = kmalloc(sizeof(int));
    if (retval == NULL) {
        thread_kill(thread_get_current());
        return;
    }
    *retval = (int)p1;
    thread_finish(retval);
}

static void sys_putchar(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    printer_putchar((char)p1);
}

static void sys_assert(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    assert(p1);
}

static void sys_proc_info_get(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    REQUIRE_VALID_PTR(p4, sizeof(bool));

    if (!pointer_in_vma(p1, sizeof(unative_t)) || !pointer_in_vma(p2, sizeof(size_t)) || !pointer_in_vma(p3, sizeof(size_t))) {
        *(bool*)p4 = false;
        return;
    }

    process_t* proc = thread_get_current()->owner_process;
    *(unative_t*)p1 = (unative_t)proc;
    *(size_t*)p2 = ++proc->tick_count;
    *(size_t*)p3 = thread_get_current()->as->size;
    *(bool*)p4 = true;
}

static void sys_spawn_process(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    REQUIRE_VALID_PTR(p2, sizeof(unative_t));
    REQUIRE_VALID_PTR(p3, sizeof(unative_t));
    errno_t err_spawn_process = 0;
    process_t* process = kmalloc(sizeof(process_t));
    err_spawn_process = process_spawn(&process, (uint16_t)p1);
    *(unative_t*)p2 = process->pid;
    *(unative_t*)p3 = err_spawn_process;
}

static void sys_lookup(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    REQUIRE_VALID_PTR(p1, 1);
    REQUIRE_VALID_PTR(p2, sizeof(uint16_t));
    REQUIRE_VALID_PTR(p3, sizeof(unative_t));

    errno_t err_lookup = minixfs_lookup(minixfs_get_current(), MINIX_ROOT_INO, (const char*)p1, (uint16_t*)p2);
    *(unative_t*)p3 = err_lookup;
}

static void sys_wait_process(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    REQUIRE_VALID_PTR(p2, sizeof(int));
    REQUIRE_VALID_PTR(p3, sizeof(unative_t));

    errno_t err_wait_process = 0;
    pid_t wanted_pid = (pid_t)p1;
    list_foreach(process_list, process_t, link, process) {
        if (process->pid == wanted_pid) {
            err_wait_process = process_join(process, (int*)p2);
            *(unative_t*)p3 = err_wait_process;
            return;
        }
    }

    *(unative_t*)p3 = EINVAL;
}

static void sys_mmap(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    REQUIRE_VALID_PTR(p1, sizeof(mmap_args_t));
    REQUIRE_VALID_PTR(p2, sizeof(uintptr_t));
    REQUIRE_VALID_PTR(p3, sizeof(unative_t));
    mmap_args_t* m_args = (mmap_args_t*)p1;

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
}

static void sys_mq_lookup(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    REQUIRE_VALID_PTR(p4, sizeof(unative_t));
    errno_t err = mq_lookup_or_create((fourcc_t)p1, (size_t)p2, (size_t)p3);
    *(unative_t*)p4 = err;
}

static void sys_mq_recv(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    REQUIRE_VALID_PTR(p2, p3);
    REQUIRE_VALID_PTR(p4, sizeof(unative_t));
    errno_t err = mq_recv((fourcc_t)p1, (void*)p2, (size_t*)p3);
    *(unative_t*)p4 = err;
}

static void sys_mq_recv_block(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    REQUIRE_VALID_PTR(p2, p3);
    REQUIRE_VALID_PTR(p4, sizeof(unative_t));
    errno_t err = mq_recv_blocking((fourcc_t)p1, (void*)p2, (size_t*)p3);
    *(unative_t*)p4 = err;
}

static void sys_mq_send(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    REQUIRE_VALID_PTR(p2, p3);
    REQUIRE_VALID_PTR(p4, sizeof(unative_t));
    errno_t err = mq_send((fourcc_t)p1, (const void*)p2, (size_t)p3);
    *(unative_t*)p4 = err;
}

static void sys_mq_destroy(unative_t p1, unative_t p2, unative_t p3, unative_t p4) {
    REQUIRE_VALID_PTR(p2, sizeof(unative_t));
    errno_t err = mq_destroy((fourcc_t)p1);
    *(unative_t*)p2 = err;
}

/** Handles a syscall.
 *
 * The function receives a pointer to an exception context, which
 * represents the snapshot of CPU and (some) CP0 registers at the
 * time of the syscall.
 */
void handle_syscall(exc_context_t* exc_context) {
    static const syscall_handler_t syscall_table[] = {
        [SYSCALL_EXIT] = sys_exit,
        [SYSCALL_PUTCHAR] = sys_putchar,
        [SYSCALL_ASSERT] = sys_assert,
        [SYSCALL_PROC_INFO_GET] = sys_proc_info_get,
        [SYSCALL_SPAWN_PROCESS] = sys_spawn_process,
        [SYSCALL_LOOKUP] = sys_lookup,
        [SYSCALL_WAIT_PROCESS] = sys_wait_process,
        [SYSCALL_MMAP] = sys_mmap,
        [SYSCALL_MQ_LOOKUP] = sys_mq_lookup,
        [SYSCALL_MQ_SEND] = sys_mq_send,
        [SYSCALL_MQ_RECV] = sys_mq_recv,
        [SYSCALL_MQ_RECV_BLOCK] = sys_mq_recv_block,
        [SYSCALL_MQ_DESTROY] = sys_mq_destroy,
    };

    syscall_t id = (syscall_t)exc_context->a0;

    if (id >= SYSCALL_LAST || syscall_table[id] == NULL) {
        panic("Unknown syscall");
    }

    syscall_table[id](exc_context->a1, exc_context->a2, exc_context->a3, exc_context->a4);

    // On success, shift EPC by 4 to resume execution of the interrupted
    // thread on the next instruction (we don't want to restart it).
    exc_context->sepc += 4;
}
