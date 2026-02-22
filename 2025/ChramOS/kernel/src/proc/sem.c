// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <proc/sem.h>

/** Initializes given semaphore.
 *
 * @param sem Semaphore to initialize.
 * @param value Initial semaphore value (1 effectively creates mutex).
 * @return Error code.
 * @retval EOK Semaphore was successfully initialized.
 */
errno_t sem_init(sem_t* sem, int value) {
    list_init(&sem->queue_list);
    sem->value = value;

    return EOK;
}

/** Destroys given semaphore.
 *
 * The function must panic if there are threads waiting for this semaphore.
 *
 * @param sem Semaphore to destroy.
 */
void sem_destroy(sem_t* sem) {
    if (sem->value < 0)
        panic("Cannot destroy semaphore when it has waiting threads!");
}

/** Get current value of the semaphore.
 *
 * @param sem Semaphore to query.
 * @return Current semaphore value.
 */
int sem_get_value(sem_t* sem) {
    return sem->value;
}

static void sem_wait_unsafe(sem_t* sem) {
    if (--(sem->value) >= 0) {
        return;
    }

    list_append(&sem->queue_list, &current_thread->locks_link);

    thread_suspend();
}

/** Locks (downs) the semaphore.
 *
 * Decrement the value of this semaphore. If the new value would be negative,
 * block and wait for someone to call sem_post() first.
 *
 * @param sem Semaphore to be locked.
 */
void sem_wait(sem_t* sem) {
    assert(sem != NULL);
    bool saved = interrupts_disable();
    sem_wait_unsafe(sem);
    interrupts_restore(saved);
}

static void sem_post_unsafe(sem_t* sem) {
    if (sem->value++ >= 0) {
        return;
    }

    link_t* next_link = list_pop(&sem->queue_list);
    thread_t* new_thread = list_item(next_link, thread_t, locks_link);
    thread_wakeup(new_thread);
}

/** Unlocks (ups/signals) the sempahore.
 *
 * Increment the value of this semaphore or wake-up one of blocked threads
 * inside sem_wait().
 *
 * @param sem Semaphore to be unlocked.
 */
void sem_post(sem_t* sem) {
    assert(sem != NULL);
    bool saved = interrupts_disable();
    sem_post_unsafe(sem);
    interrupts_restore(saved);
}

/** Try to lock the semaphore without waiting.
 *
 * If the call to sem_wait() would block, do nothing and return EBUSY.
 *
 * @param sem Semaphore to be locked.
 * @return Error code.
 * @retval EOK Semaphore was successfully locked.
 * @retval EBUSY Semaphore has value of 0 and locking would block.
 */
errno_t sem_trywait(sem_t* sem) {
    assert(sem != NULL);
    bool saved = interrupts_disable();

    if (sem->value > 0) {
        sem_wait(sem);
        interrupts_restore(saved);
        return EOK;
    }

    interrupts_restore(saved);
    return EBUSY;
}
