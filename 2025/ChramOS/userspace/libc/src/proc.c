// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <np/proc.h>
#include <np/syscall.h>

/** Get information about current process.
 *
 * Note that id can be simply address of the kernel structure, the only
 * requirement is that it shall not change between calls.
 *
 * virt_mem_size shall be the total amount of virtual memory this process
 * has available (i.e. first invalid address is virt_mem_size + 2*4096).
 *
 * total_ticks shall represent accounting for the process, however we will
 * accept a solution that only increments the counter with every call to
 * this function.
 *
 * @param info Where to store the retrieved information.
 * @return true if the call was successful, false otherwise.
 */
bool np_proc_info_get(np_proc_info_t* info) {
    volatile bool valid = false;
    __SYSCALL4(SYSCALL_PROC_INFO_GET, (unative_t)&info->id, (unative_t)&info->total_ticks, (unative_t)&info->virt_mem_size, (unative_t)&valid);
    return valid;
}

/** Spawn a new process from an inode.
 *
 * Creates a new process by loading the specified executable file
 * from a filesystem using its inode number.
 *
 * @param ino Inode number of an executable file
 * @return Process ID on success
 *         Negative error code on failure
 */
int process_spawn(uint16_t ino, pid_t* out_pid) {
    volatile int err = -42;
    __SYSCALL3(SYSCALL_SPAWN_PROCESS, (unative_t)ino, (unative_t)out_pid, (unative_t)&err);
    return err;
}

/** Wait for a process to finish.
 *
 * Waits for the specified process to terminate and returns its exit status.
 * This will block until the process exits.
 *
 * @param pid Process ID returned by spawn()
 * @return Exit status of the process on success
 *         Negative error code on failure
 */
int process_wait(pid_t pid, int* exit_code) {
    volatile int err = -42;
    __SYSCALL3(SYSCALL_WAIT_PROCESS, pid, (unative_t)exit_code, (unative_t)&err);
    return err;
}
