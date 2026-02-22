// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <adt/list.h>
#include <debug.h>
#include <drivers/timer.h>
#include <exc.h>
#include <lib/print.h>
#include <proc/scheduler.h>
#include <proc/thread.h>

list_t scheduler_list;
thread_t* current_thread = NULL;

#ifndef KERNEL_SCHEDULER_QUANTUM
/** Scheduling quantum default. */
#define KERNEL_SCHEDULER_QUANTUM 2000
#endif

/** Initialize support for scheduling.
 *
 * Called once at system boot.
 */
void scheduler_init(void) {
    list_init(&scheduler_list);
}

/** Marks given thread as ready to be executed.
 *
 * It is expected that this thread would be added at the end
 * of the queue to run in round-robin fashion.
 *
 * @param thread Thread to make runnable.
 */
void scheduler_add_ready_thread(thread_t* thread) {
    bool saved = interrupts_disable();
    thread->state = RUNNING;
    list_append(&scheduler_list, &thread->scheduler_link);
    interrupts_restore(saved);
}

/** Removes given thread from scheduling.
 *
 * Expected to be called when thread is suspended, terminates execution
 * etc.
 *
 * @param thread Thread to remove from the queue.
 */
void scheduler_remove_thread(thread_t* thread) {
    bool saved = interrupts_disable();
    list_remove(&thread->scheduler_link);
    interrupts_restore(saved);
}

/** Switch to next thread in the queue. */
void scheduler_schedule_next(void) {
    bool saved = interrupts_disable();
    thread_t* new_thread;

    if (current_thread == NULL) {
        new_thread = list_item(scheduler_list.head.next, thread_t, scheduler_link);
    } else if (current_thread->state != RUNNING) {
        new_thread = list_item(scheduler_list.head.next, thread_t, scheduler_link);
        list_rotate(&scheduler_list);
    } else {
        list_rotate(&scheduler_list);
        new_thread = list_item(scheduler_list.head.next, thread_t, scheduler_link);
    }

    timer_interrupt_after(KERNEL_SCHEDULER_QUANTUM);
    interrupts_restore(saved);
    thread_switch_to(new_thread);
}
