// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Charles University

#include <debug/code.h>
#include <errno.h>
#include <mm/as.h>
#include <mm/frame.h>
#include <mm/heap.h>
#include <proc/process.h>
#include <proc/userspace.h>

list_t process_list;

void process_init(void) {
    list_init(&process_list);
}

static void mem_copy(uintptr_t from_addr, uintptr_t to_addr, size_t size) {
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
    mem_copy(data->image_start, APP_CODE_START, data->image_size - APP_CODE_START);
    cpu_jump_to_userspace(APP_STACK_TOP, APP_CODE_START);
    return NULL;
}

static void* process_entry_func_disk(process_t* data) {
    mutex_lock(&data->init_lock);
    mutex_unlock(&data->init_lock);
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
    if (image_size > process_memory_size || process_memory_size % PAGE_SIZE != 0) {
        return EINVAL;
    }

    *process_out = kmalloc(sizeof(process_t));
    if (*process_out == NULL) {
        return ENOMEM;
    }

    (*process_out)->image_size = image_size;
    (*process_out)->tick_count = 0;

    errno_t error;

    if (image_location != 0) {
        // for previous tests if we have binary somewhere in RAM
        // maybe it is good to just disable every test that use this method

        (*process_out)->image_start = APP_CODE_START + image_location;

        error = thread_create_for_process(&((*process_out)->user_thread), (thread_entry_func_t)process_entry_func, *process_out, 0, "User process", process_memory_size + APP_STACK_SIZE, *process_out);

        if (error != EOK) {
            kfree(*process_out);
            return error;
        }

        link_t* first_link = (*process_out)->user_thread->as->vma_list.head.next;
        vma_t* vma = list_item(first_link, vma_t, link);

        uintptr_t old_start = vma->start;
        vma->start = PAGE_NULL_COUNT * FRAME_SIZE;
        vma->size += (old_start - vma->start);

        for (size_t offset = 0; offset < vma->size; offset += FRAME_SIZE) {
            uintptr_t va = vma->start + offset;

            uintptr_t root_table = ROOT_TABLE_PHYS_FROM_SATP((*process_out)->user_thread->as->satp_val);
            size_t vpn1 = VPN1_FROM_VA(va);
            size_t vpn0 = VPN0_FROM_VA(va);

            uint32_t root_pte = PTE_AT_INDEX(root_table, vpn1);
            if (!(root_pte & PTE_VALID)) {
                uintptr_t new_l1_phys;
                errno_t err = frame_alloc(1, &new_l1_phys);
                if (err == ENOMEM) {
                    return ENOMEM;
                }
                empty_buffer((void*)new_l1_phys, FRAME_SIZE);
                PTE_AT_INDEX(root_table, vpn1) = CREATE_PTE(new_l1_phys, PTE_VALID | PTE_USER);
                root_pte = PTE_AT_INDEX(root_table, vpn1);
            }

            uintptr_t l1_table_phys = PA_FROM_PTE(root_pte);

            if (!(PTE_AT_INDEX(l1_table_phys, vpn0) & PTE_VALID)) {
                uintptr_t frame_phys;
                errno_t err = frame_alloc(1, &frame_phys);
                if (err == ENOMEM) {
                    return ENOMEM;
                }
                empty_buffer((void*)frame_phys, FRAME_SIZE);
                PTE_AT_INDEX(l1_table_phys, vpn0) = CREATE_PTE(frame_phys, PTE_VALID | PTE_USER | PTE_READ | PTE_WRITE | PTE_EXECUTE);
            }
        }

    } else {
        // disk binary
        (*process_out)->image_start = 0;
        error = thread_create_for_process(&((*process_out)->user_thread), (thread_entry_func_t)process_entry_func_disk, *process_out, 0, "User process", process_memory_size + APP_STACK_SIZE, *process_out);
        mutex_init(&(*process_out)->init_lock);
        mutex_lock(&(*process_out)->init_lock);
    }

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

    list_remove(&process->link);

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

/* Uses simple counter, doesn't mind the overflow */
pid_t set_pid(void) {
    static int counter = -1;
    return ++counter;
}

/** Spawn a new userspace process from a file on disk.
 *
 * @param process Where to place the initialized process_t structure.
 * @param ino Inode number of the executable file.
 * @return Error code.
 * @retval EOK Process was spawned successfully.
 * @retval ENOENT File not found, is not executable or not a regular file.
 * @retval ENOMEM Not enough memory.
 * @retval EBUSY Filesystem I/O error.
 */
errno_t process_spawn(process_t** process, uint16_t ino) {
    minix_inode_t* inode = kmalloc(sizeof(minix_inode_t));
    minixfs_read_inode(minixfs_get_current(), ino, inode);

    size_t number_pages = inode->i_size / PAGE_SIZE + 1;
    errno_t err = process_create(process, 0, inode->i_size, number_pages * PAGE_SIZE + APP_STACK_SIZE);

    if (err == EOK) {
        (*process)->ino = *inode;
        (*process)->pid = set_pid();

        link_t* first_link = (*process)->user_thread->as->vma_list.head.next;
        vma_t* first_vma = list_item(first_link, vma_t, link);

        first_vma->disk_backed = true;
        first_vma->ino = *inode;

        first_vma->start = APP_STACK_SIZE;
        first_vma->offset = APP_STACK_SIZE;

        list_append(&process_list, &((*process)->link));
        mutex_unlock(&(*process)->init_lock);
    }

    return err;
}

/** Get a process by its PID.
 *
 * @param pid Process ID to look up.
 * @return Process pointer, or NULL if not found.
 */
process_t* process_get_by_pid(pid_t pid) {
    list_foreach(process_list, process_t, link, process) {
        if (process->pid == pid) {
            return process;
        }
    }
    return NULL;
}
