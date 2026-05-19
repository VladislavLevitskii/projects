// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug.h>
#include <debug/code.h>
#include <drivers/disk.h>
#include <drivers/machine.h>
#include <ipc/mqueue.h>
#include <ktest.h>
#include <lib/print.h>
#include <main.h>
#include <mm/as.h>
#include <mm/frame.h>
#include <mm/heap.h>
#include <proc/process.h>
#include <proc/scheduler.h>
#include <proc/thread.h>

/** Kernel init thread.
 *
 * This is first real thread running. When this thread terminates,
 * the whole system shuts down.
 *
 * When the code is compiled to run kernel test, we execute only
 * that test and terminate.
 */
static void* init_thread(void* ignored) {
#ifdef KERNEL_TEST
    kernel_test();
#else
    printk("%s: Hello from kernel!\n", thread_get_current()->name);

    minixfs_mount(minixfs_get_current());

    uint16_t ino;
    errno_t err = minixfs_lookup(minixfs_get_current(), MINIX_ROOT_INO, INIT_PROCESS_NAME, &ino);
    panic_if(err != EOK, "Binary was not found (%d: %s)", err, errno_as_str(err));

    process_t* app;
    err = process_spawn(&app, ino);
    panic_if(err != EOK, "userspace application launch failed (%d: %s)", err, errno_as_str(err));

    int exit_status;
    err = process_join(app, &exit_status);
    if (err == EKILLED) {
        printk("\nUser application forcefully terminated.\n");
    } else {
        panic_if(err != EOK, "waiting for userspace application failed (%d: %s)", err, errno_as_str(err));
    }
    if (exit_status != 0) {
        printk("\nUser application failed.\n");
    }
#endif
    printk("\nHalt.\n");
    machine_halt();

    return NULL;
}

/** This is kernel C-entry point.
 *
 * The kernel boostrap code written in assembly jumps here. Note that this
 * function runs on special stack and does not represent a real thread (yet).
 */
void kernel_main(void) {
    frame_init();
    heap_init();
    as_init();
    scheduler_init();
    threads_init();
    disk_init();
    process_init();
    mqueue_init();

    thread_t* main_thread;
    errno_t err = thread_create_new_as(&main_thread, init_thread, NULL, 0, "[INIT]", 0);
    panic_if(err != EOK, "init thread creation failed (%d: %s)", err, errno_as_str(err));

    // Switch to the first thread.
    scheduler_schedule_next();

    // We are not a real thread here so we should never return here.
    panic("unexpected return to kernel_main");
}
