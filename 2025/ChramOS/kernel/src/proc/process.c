// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/code.h>
#include <errno.h>
#include <mm/as.h>
#include <mm/frame.h>
#include <mm/heap.h>
#include <proc/process.h>
#include <proc/userspace.h>

static void copy_bytes(uintptr_t from_addr, uintptr_t to_addr, size_t size) {
    size_t four_bytes_count = size / 4;
    for (size_t i = 0; i < four_bytes_count; i++) {
        *((uint32_t*)to_addr + i) = *((uint32_t*)from_addr + i);
    }

    size_t bytes_copied = four_bytes_count * 4;
    from_addr += bytes_copied;
    to_addr += bytes_copied;
    size -= bytes_copied;

    for (size_t i = 0; i < size; i++) {
        *((int8_t*)to_addr + i) = *((int8_t*)from_addr + i);
    }
}

static void* process_entry_func(process_t* data) {
    copy_bytes(PROCESS_IMAGE_START + APP_CODE_START, APP_CODE_START, data->image_size - APP_CODE_START);
    cpu_jump_to_userspace(APP_STACK_TOP, APP_CODE_START);
    return NULL;
}

/** Create new userspace process.
 *
 * @param process_out Where to place the initialized process_t structure.
 * @param image_location Virtual address (in kernel segment) where is the image of the raw application binary.
 * @param image_size Size of the application binary image.
 * @param process_memory_size Amount of virtual memory to give to the application (at least image_size).
 * @return Error code.
 * @retval EOK Process was created and its main thread started.
 * @retval ENOMEM Not enough memory to complete the operation.
 * @retval EINVAL Invalid call (unaligned size etc.).
 */
errno_t process_create(process_t** process_out, uintptr_t image_location, size_t image_size, size_t process_memory_size) {
    if (image_size > process_memory_size || process_memory_size % PAGE_SIZE != 0)
        return EINVAL;

    *process_out = kmalloc(sizeof(process_t));
    if (*process_out == NULL)
        return ENOMEM;

    (*process_out)->image_size = image_size;
    (*process_out)->image_start = APP_CODE_START;
    (*process_out)->tick_count = 0;

    errno_t error = thread_create_for_process(&((*process_out)->user_thread), (thread_entry_func_t)process_entry_func, *process_out, 0, "User process", process_memory_size + APP_STACK_SIZE, *process_out);
    assert(error != EINVAL);
    if (error == ENOMEM) {
        kfree(*process_out);
        return error;
    }
    assert(error == EOK);

    return EOK;
}

/** Wait for termination of another process.
 *
 * @param process Process to wait for.
 * @param exit_status Where to place the process exit status (return value from main).
 * @return Error code.
 * @retval EOK Joined successfully.
 * @retval EBUSY Some other thread is already joining this process.
 * @retval EKILLED Process was killed.
 * @retval EINVAL Invalid process.
 */
errno_t process_join(process_t* process, int* exit_status) {
    if (process == NULL)
        return EINVAL;

    void* retval = NULL;
    errno_t error = thread_join(process->user_thread, &retval);
    if (error == EBUSY || error == EINVAL)
        return error;
    if (error == EKILLED) {
        assert(retval == NULL);
        kfree(process);
        return error;
    }

    assert(retval != NULL);
    *exit_status = *(int*)retval;
    kfree(retval);
    kfree(process);

    return EOK;
}
