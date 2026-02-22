// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <abi.h>
#include <adt/align.h>
#include <debug/code.h>
#include <drivers/sv32.h>
#include <exc.h>
#include <mm/as.h>
#include <mm/frame.h>
#include <mm/heap.h>
#include <proc/context.h>
#include <proc/scheduler.h>
#include <proc/thread.h>

#define STACK_FRAME_COUNT (THREAD_STACK_SIZE / FRAME_SIZE)
_Static_assert(THREAD_STACK_SIZE % FRAME_SIZE == 0);

/** Initialize support for threading.
 *
 * Called once at system boot.
 */
void threads_init(void) {
}

static void thread_start() {
    void* retval = current_thread->entry_func(current_thread->func_parameter);
    thread_finish(retval);
}

static errno_t thread_create_base(thread_t** thread_out, thread_entry_func_t entry, void* data, unsigned int flags, const char* name, as_t* address_space) {
    thread_t* new_thread = kmalloc(sizeof(thread_t));
    if (new_thread == NULL) {
        thread_t mock;
        as_acquire(&mock, address_space);
        as_release(&mock);
        return ENOMEM;
    }

    as_acquire(new_thread, address_space);

    // allocate stack and thread structure
    uintptr_t thread_stack_start;
    errno_t err = frame_alloc(STACK_FRAME_COUNT, &thread_stack_start);
    if (err == ENOMEM) {
        kfree(new_thread);
        as_release(new_thread);
        return ENOMEM;
    }

    // initialize thread structure
    int i = 0;
    for (; i < THREAD_NAME_MAX_LENGTH && name[i] != '\0'; i++)
        new_thread->name[i] = name[i];
    new_thread->name[i] = '\0';
    new_thread->entry_func = entry;

    _Static_assert(THREAD_STACK_SIZE % (ABI_STACK_ALIGNMENT * 8) == 0);
    uintptr_t thread_stack_end = thread_stack_start + THREAD_STACK_SIZE;

    new_thread->cpu_context.sp = thread_stack_end;
    new_thread->cpu_context.ra = (unative_t)thread_start;
    new_thread->cpu_context.sstatus = CSR_SSTATUS_SUM; // permit Supervisor User Memory access
    new_thread->cpu_context.sstatus |= CSR_SSTATUS_SIE; // enable interrupts

    new_thread->stack_base = (void*)thread_stack_start;
    new_thread->stack_tip = (void*)(thread_stack_start + THREAD_STACK_SIZE);
    new_thread->as = address_space;

    new_thread->state = DEFAULT;
    new_thread->joinee = NULL;

    new_thread->func_parameter = data;
    new_thread->func_retval = NULL;

    new_thread->owner_process = NULL;

    link_init(&new_thread->scheduler_link);
    link_init(&new_thread->locks_link);

    *thread_out = new_thread;
    return EOK;
}

/** Create a new thread.
 *
 * The thread is automatically placed into the queue of ready threads.
 *
 * This function allocates space for both stack and the thread_t structure
 * (hence the double <code>**</code> in <code>thread_out</code>).
 *
 * This thread will use the same address space as the current one.
 *
 * @param thread_out Where to place the initialized thread_t structure.
 * @param entry Thread entry function.
 * @param data Data for the entry function.
 * @param flags Flags (unused).
 * @param name Thread name (for debugging purposes).
 * @return Error code.
 * @retval EOK Thread was created and started (added to ready queue).
 * @retval ENOMEM Not enough memory to complete the operation.
 * @retval EINVAL Invalid flags (unused).
 */
errno_t thread_create(thread_t** thread_out, thread_entry_func_t entry, void* data, unsigned int flags, const char* name) {
    assert(current_thread != NULL);
    as_t* current_thread_as = current_thread->as;
    errno_t error = thread_create_base(thread_out, entry, data, flags, name, current_thread_as);
    if (error != EOK) {
        return error;
    }

    scheduler_add_ready_thread(*thread_out);
    return EOK;
}

/** Create a new thread with new address space.
 *
 * The thread is automatically placed into the queue of ready threads.
 *
 * This function allocates space for both stack and the thread_t structure
 * (hence the double <code>**</code> in <code>thread_out</code>).
 *
 * @param thread_out Where to place the initialized thread_t structure.
 * @param entry Thread entry function.
 * @param data Data for the entry function.
 * @param flags Flags (unused).
 * @param name Thread name (for debugging purposes).
 * @param as_size Address space size, aligned at page size (0 is correct though not very useful).
 * @return Error code.
 * @retval EOK Thread was created and started (added to ready queue).
 * @retval ENOMEM Not enough memory to complete the operation.
 * @retval EINVAL Invalid flags (unused).
 */
errno_t thread_create_new_as(thread_t** thread_out, thread_entry_func_t entry, void* data, unsigned int flags, const char* name, size_t as_size) {
    as_t* as_addr_phys = as_create(as_size, 0);
    if (as_addr_phys == NULL) {
        return ENOMEM;
    }
    errno_t error = thread_create_base(thread_out, entry, data, flags, name, as_addr_phys);
    if (error != EOK) {
        return error;
    }

    scheduler_add_ready_thread(*thread_out);
    return EOK;
}

/** Create a new thread with new address space for given process.
 *
 * The <code>owner_process</code> pointer of the created thread is set to the given process.
 *
 * The thread is automatically placed into the queue of ready threads.
 *
 * @param thread_out Where to place the initialized thread_t structure.
 * @param entry Thread entry function.
 * @param data Data for the entry function.
 * @param flags Flags (unused).
 * @param name Thread name (for debugging purposes).
 * @param as_size Address space size, aligned at page size (0 is correct though not very useful).
 * @param owner_process Process owning the thread.
 * @return Error code.
 * @retval EOK Thread was created and started (added to ready queue).
 * @retval ENOMEM Not enough memory to complete the operation.
 * @retval EINVAL Invalid flags (unused).
 */
errno_t thread_create_for_process(thread_t** thread_out, thread_entry_func_t entry, void* data, unsigned int flags, const char* name, size_t as_size, process_t* owner_process) {
    as_t* as_addr_phys = as_create(as_size, 0);
    if (as_addr_phys == NULL) {
        return ENOMEM;
    }
    errno_t error = thread_create_base(thread_out, entry, data, flags, name, as_addr_phys);
    if (error != EOK) {
        return error;
    }

    (*thread_out)->owner_process = owner_process;
    scheduler_add_ready_thread(*thread_out);
    return EOK;
}

/** Return information about currently executing thread.
 *
 * @retval NULL When no thread was started yet.
 */
thread_t* thread_get_current(void) {
    return current_thread;
}

/** Yield the processor. */
void thread_yield(void) {
    scheduler_schedule_next();
}

/** Current thread stops execution and is not scheduled until woken up. */
void thread_suspend(void) {
    current_thread->state = SUSPENDED;
    scheduler_remove_thread(current_thread);
    thread_yield();
}

/** Terminate currently running thread.
 *
 * Thread can (normally) terminate in two ways: by returning from the entry
 * function or by calling this function. The parameter to this function then
 * has the same meaning as the return value from the entry function.
 *
 * Note that this function never returns.
 *
 * @param retval Data to return in thread_join.
 */
void thread_finish(void* retval) {
    current_thread->state = FINISHED;
    current_thread->func_retval = retval;

    if (current_thread->joinee != NULL)
        thread_wakeup(current_thread->joinee);

    scheduler_remove_thread(current_thread);
    thread_yield();

    while (1) {
    }
}

/** Tells if a thread finished its execution.
 *
 * Thread finished the execution either when it returned from the entry
 * function, called thread_finish() or was already killed.
 *
 * In other words, when this function returns true, thread_join() should
 * return immediately (it is not required but it is a natural consequence
 * of the above).
 *
 * This function may not be called when thread_join() was already called
 * on the thread in question (because the thread pointer by that time will
 * not be valid).
 *
 * @param thread Thread in question.
 */
bool thread_has_finished(thread_t* thread) {
    return (thread->state == FINISHED || thread->state == KILLED);
}

/** Wakes-up existing thread.
 *
 * Note that waking-up a running (or ready) thread has no effect (i.e. the
 * function shall not count wake-ups and suspends).
 *
 * Note that waking-up a thread does not mean that it will immediatelly start
 * executing.
 *
 * @param thread Thread to wake-up.
 * @return Error code.
 * @retval EOK Thread was woken-up (or was already ready/running).
 * @retval EEXITED Thread already finished its execution.
 */
errno_t thread_wakeup(thread_t* thread) {
    if (thread->state == FINISHED || thread->state == KILLED) {
        return EEXITED;
    } else if (thread->state == SUSPENDED) {
        thread->state = RUNNING;
        scheduler_add_ready_thread(thread);
    }
    return EOK;
}

/** Joins another thread (waits for it to terminate).
 *
 * Calling thread_join blocks until the thread we are joining
 * (i.e. the parameter) terminates (either by being killed, finishing its
 * entry function or calling thread_finish). It is not allowed to join the
 * same thread from two places, where possible (see below) this should be
 * detected and signalled via EBUSY return value.
 *
 * After thread_join returns, all pointers to the joined thread (i.e., to the
 * thread_t structure given as parameter) are considered invalid and should
 * not be used (including, but not limited to, calls to thread_finish,
 * thread_join or thread_kill).
 *
 * Note that thread_create is an allocation function (like kmalloc) while
 * thread_join is somewhat a deallocation function (like kfree). As with kfree,
 * it does not make sense to call thread_join twice or use the pointer in any
 * way after return as it is no longer valid.
 *
 * Implementation note: thread_join must release all memory allocated for the
 * joined thread (assuming a successful join). While it is possible to release
 * the thread stack earlier, the rest of the structure must remain valid until
 * a call to thread_join. For example, thread_kill can release the stack of
 * the killed thread because the thread no longer executes but releasing the
 * stack inside thread_finish is not directly possible as the clean-up function
 * still needs to use the stack (it would be possible to delegate the job to
 * a special clean-up thread so that the stack is released as soon as possible
 * but postponing the stack release to thread_join is a typical choice).
 *
 * Note that <code>retval</code> could be <code>NULL</code> if the caller
 * is not interested in the returned value.
 *
 * @param thread Thread to wait for.
 * @param retval Where to place the value returned from thread_finish.
 * @return Error code.
 * @retval EOK Thread was joined.
 * @retval EBUSY Some other thread is already joining this one.
 * @retval EINVAL The thread to join is invalid (including NULL and self).
 * @retval EKILLED Thread was killed.
 */
errno_t thread_join(thread_t* thread, void** retval) {
    if (thread == NULL || thread == current_thread)
        return EINVAL;

    if (thread->joinee != NULL)
        return EBUSY;

    thread->joinee = current_thread;

    if (thread->state == RUNNING || thread->state == SUSPENDED)
        thread_suspend();

    assert(thread->state != RUNNING);
    assert(thread->state != SUSPENDED);

    void* ret_value = thread->func_retval;
    thread_state_t thread_state = thread->state;

    as_release(thread);

    frame_free(STACK_FRAME_COUNT, (uintptr_t)thread->stack_base);
    kfree(thread);

    if (thread_state == KILLED)
        return EKILLED;

    if (retval != NULL)
        *retval = ret_value;

    return EOK;
}

static void switch_context(context_t* this_context, context_t* next_context, unative_t satp_value) {
    sv32_set_satp(satp_value);

    if (ASID_FROM_SATP(satp_value) == 0) {
        // if shared ASID, do TLB shootdown
        // we do not support TLB shootdown per ASID, so we do a full shootdown
        // documentation states that if we reuse ASIDs, we should do TLB shootdown after the context switch,
        //   but as this is identity mapped kernel code, we can do it before as well (there is not a good place
        //   in the code to do it after the context switch)

        sv32_tlb_shootdown();
    }

    cpu_switch_context(this_context, next_context);
}

/** Switch CPU context to a different thread.
 *
 * Note that this function must work even if there is no current thread
 * (i.e. for the very first context switch in the system).
 *
 * @param thread Thread to switch to.
 */
void thread_switch_to(thread_t* thread) {
    if (current_thread == NULL) {
        context_t dump_context;
        current_thread = thread;
        switch_context(&dump_context, &thread->cpu_context, thread->as->satp_val);
    } else {
        thread_t* prev_thread = current_thread;
        current_thread = thread;
        switch_context(&prev_thread->cpu_context, &thread->cpu_context, thread->as->satp_val);
    }
}

/** Get address space of given thread. */
as_t* thread_get_as(thread_t* thread) {
    return thread->as;
}

/** Kills given thread.
 *
 * Note that this function shall work for any existing thread, including
 * currently running one.
 *
 * Joining a killed thread results in EKILLED return value from thread_join.
 *
 * @param thread Thread to kill.
 * @return Error code.
 * @retval EOK Thread was killed.
 * @retval EEXITED Thread already finished its execution.
 */
errno_t thread_kill(thread_t* thread) {
    assert(thread != NULL);
    if (thread->state == FINISHED)
        return EEXITED;

    thread->state = KILLED;
    if (thread->joinee != NULL)
        thread_wakeup(thread->joinee);

    scheduler_remove_thread(thread);
    scheduler_schedule_next();

    return EOK;
}