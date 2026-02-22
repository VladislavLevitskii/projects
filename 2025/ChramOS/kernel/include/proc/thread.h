// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#ifndef _PROC_THREAD_H
#define _PROC_THREAD_H

#include <adt/list.h>
#include <errno.h>
#include <proc/context.h>
#include <types.h>

typedef struct process process_t;

/** Thread stack size.
 *
 * Set quite liberally because stack overflows are notoriously
 * difficult to debug (and difficult to detect too).
 */
#define THREAD_STACK_SIZE 4096

/** Max length of a thread name (excluding the terminating zero). */
#define THREAD_NAME_MAX_LENGTH 31

/* Forward declaration to prevent cyclic header inclusions. */
typedef struct as as_t;

/** Thread entry function (similar to pthreads). */
typedef void* (*thread_entry_func_t)(void*);

/** Information about any existing thread. */
typedef enum {
    RUNNING = 0,
    SUSPENDED = 1,
    KILLED = 2,
    FINISHED = 3,
    DEFAULT = 4
} thread_state_t;

typedef struct thread thread_t;

struct thread {
    char name[THREAD_NAME_MAX_LENGTH + 1];
    thread_entry_func_t entry_func;

    context_t cpu_context;
    link_t scheduler_link;
    link_t locks_link;

    thread_state_t state;
    thread_t* joinee;

    void* func_parameter;
    void* func_retval;
    void* stack_base;
    void* stack_tip;

    as_t* as;
    process_t* owner_process; // NULL for kernel threads without associated process
};

void threads_init(void);

errno_t thread_create(thread_t** thread, thread_entry_func_t entry, void* data, unsigned int flags, const char* name);
errno_t thread_create_new_as(thread_t** thread, thread_entry_func_t entry, void* data, unsigned int flags, const char* name, size_t as_size);
errno_t thread_create_for_process(thread_t** thread, thread_entry_func_t entry, void* data, unsigned int flags, const char* name, size_t as_size, process_t* process);

thread_t* thread_get_current(void);
void thread_yield(void);
void thread_suspend(void);
void thread_finish(void* retval) __attribute__((noreturn));
bool thread_has_finished(thread_t* thread);

errno_t thread_wakeup(thread_t* thread);
errno_t thread_join(thread_t* thread, void** retval);
void thread_switch_to(thread_t* thread);

as_t* thread_get_as(thread_t* thread);
errno_t thread_kill(thread_t* thread);

#endif
