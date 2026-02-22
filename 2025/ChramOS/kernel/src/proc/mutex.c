// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <proc/mutex.h>

/** Initializes given mutex.
 *
 * @param mutex Mutex to initialize.
 * @return Error code.
 * @retval EOK Mutex was successfully initialized.
 */
errno_t mutex_init(mutex_t* mutex) {
    list_init(&mutex->queue_list);
    mutex->locked = false;
    mutex->owning_thread = NULL;
    return EOK;
}

/** Destroys given mutex.
 *
 * The function must panic if the mutex is still locked when being destroyed.
 *
 * @param mutex Mutex to destroy.
 */
void mutex_destroy(mutex_t* mutex) {
    if (mutex->locked)
        panic("Cannot destroy mutex when it is locked!");
}

static void mutex_lock_unsafe(mutex_t* mutex) {
    if (!mutex->locked) {
        mutex->locked = true;
        mutex->owning_thread = current_thread;
        return;
    }

    list_append(&mutex->queue_list, &current_thread->locks_link);

    thread_suspend();

    // the unlocking thread kept it locked for us
    assert(mutex->locked);
}

/** Locks the mutex.
 *
 * Note that when this function returns, the mutex must be locked.
 *
 * @param mutex Mutex to be locked.
 */
void mutex_lock(mutex_t* mutex) {
    assert(mutex != NULL);
    bool saved = interrupts_disable();
    mutex_lock_unsafe(mutex);
    interrupts_restore(saved);
}

static void mutex_unlock_unsafe(mutex_t* mutex) {
    if (current_thread != mutex->owning_thread)
        panic("Cannot unlock mutex you do not own!");

    if (list_is_empty(&mutex->queue_list)) {
        mutex->locked = false;
        mutex->owning_thread = NULL;
        return;
    }

    link_t* next_link = list_pop(&mutex->queue_list);
    thread_t* new_thread = list_item(next_link, thread_t, locks_link);
    thread_wakeup(new_thread);
    mutex->owning_thread = new_thread;

    // keep locked
}

/** Unlocks the mutex.
 *
 * Note that when this function returns, the mutex might be already locked
 * by a different thread.
 *
 * This function shall panic if the mutex is unlocked by a different thread
 * than the one that locked it.
 *
 * @param mutex Mutex to be unlocked.
 */
void mutex_unlock(mutex_t* mutex) {
    assert(mutex != NULL);
    bool saved = interrupts_disable();
    mutex_unlock_unsafe(mutex);
    interrupts_restore(saved);
}

/** Try to lock the mutex without waiting.
 *
 * If the mutex is already locked, do nothing and return EBUSY.
 *
 * @param mutex Mutex to be locked.
 * @return Error code.
 * @retval EOK Mutex was successfully locked.
 * @retval EBUSY Mutex is currently locked by a different thread.
 */
errno_t mutex_trylock(mutex_t* mutex) {
    assert(mutex != NULL);
    bool saved = interrupts_disable();

    if (!mutex->locked) {
        mutex_lock(mutex);
        interrupts_restore(saved);
        return EOK;
    }

    interrupts_restore(saved);
    return EBUSY;
}
