// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#include <drivers/disk.h>
#include <fs/minix.h>
#include <mm/heap.h>

// global pointers to allocated structure of Filesystem instance
minixfs_t* minixfs;

/**
 * Initialize a MinixFS structure associated with a disk.
 *
 * Called once at system boot.
 */
errno_t minixfs_init(disk_t* disk) {
    if (minixfs == NULL) {
        minixfs = kmalloc(sizeof(minixfs_t));
        CHECK_NULL(minixfs);
    }

    void* buffer_sb = kmalloc(DISK_SECTOR_SIZE);
    CHECK_NULL(buffer_sb);
    disk_read_sector(disk, SB_DISK_SECTOR, buffer_sb);
    minixfs->sb = *(minix_superblock_t*)buffer_sb;
    kfree(buffer_sb);

    minixfs->disk = disk;

    minixfs->imap.length = minixfs->sb.s_ninodes;
    minixfs->zmap.length = minixfs->sb.s_nzones;

    minixfs->imap_data = (uint8_t*)kmalloc(minixfs->sb.s_ninodes / 8);
    minixfs->zmap_data = (uint8_t*)kmalloc(minixfs->sb.s_nzones / 8);

    // inodes bm
    size_t number_of_blocks_imap = BM_SIZE_BLOCKS(minixfs->sb.s_ninodes);
    void* buffer_imap = kmalloc(number_of_blocks_imap * MINIX_BLOCK_SIZE);

    for (size_t i = 0; i < number_of_blocks_imap * NUMBER_OF_SECTORS_IN_DATA_ZONE; ++i) {
        disk_read_sector(disk, SIZE_BOOT + SIZE_SB + i, (void*)((char*)buffer_imap + i * DISK_SECTOR_SIZE));
    }

    copy_bytes(buffer_imap, minixfs->imap_data, minixfs->sb.s_ninodes / 8);
    kfree(buffer_imap);

    // zones mb
    size_t number_of_blocks_zmap = BM_SIZE_BLOCKS(minixfs->sb.s_nzones);
    void* buffer_zmap = kmalloc(number_of_blocks_zmap * MINIX_BLOCK_SIZE);

    for (size_t i = 0; i < number_of_blocks_zmap * NUMBER_OF_SECTORS_IN_DATA_ZONE; ++i) {
        disk_read_sector(disk, SIZE_BOOT + SIZE_SB + BM_SIZE_BLOCKS(minixfs->sb.s_ninodes) * NUMBER_OF_SECTORS_IN_DATA_ZONE + i, (void*)((char*)buffer_zmap + i * DISK_SECTOR_SIZE));
    }

    copy_bytes(buffer_zmap, minixfs->zmap_data, minixfs->sb.s_nzones / 8);
    kfree(buffer_zmap);

    minixfs->imap.data = minixfs->imap_data;
    minixfs->imap.length = minixfs->sb.s_ninodes;

    minixfs->zmap.data = minixfs->zmap_data;
    minixfs->zmap.length = minixfs->sb.s_nzones;

    return EOK;
}

/**
 * Get current file system instance used by the system.
 */
minixfs_t* minixfs_get_current(void) {
    if (minixfs == NULL) {
        minixfs_init(disk_get_current());
    }
    return minixfs;
}

/**
 * Mount an existing MinixFS filesystem from disk.
 */
errno_t minixfs_mount(minixfs_t* fs) {
    if (fs->sb.s_magic != MINIX_MAGIC) {
        return EINVAL;
    }

    fs->mounted = true;
    return EOK;
}

/**
 * Unmount the MinixFS filesystem and release any associated resources.
 */
void minixfs_unmount(minixfs_t* fs) {
    fs->mounted = false;

    size_t imap_bloks = BM_SIZE_BLOCKS(fs->sb.s_ninodes);
    void* imap_tmp = kmalloc(imap_bloks * MINIX_BLOCK_SIZE);
    if (imap_tmp == NULL) {
        return;
    }

    empty_buffer(imap_tmp, imap_bloks * MINIX_BLOCK_SIZE);
    copy_bytes(fs->imap_data, imap_tmp, fs->sb.s_ninodes / 8);
    for (size_t i = 0; i < imap_bloks * NUMBER_OF_SECTORS_IN_DATA_ZONE; ++i) {
        disk_write_sector(fs->disk, SIZE_BOOT + SIZE_SB + i, (void*)((char*)imap_tmp + i * DISK_SECTOR_SIZE));
    }
    kfree(imap_tmp);

    size_t zmap_bloks = BM_SIZE_BLOCKS(fs->sb.s_nzones);
    void* zmap_tmp = kmalloc(zmap_bloks * MINIX_BLOCK_SIZE);
    if (zmap_tmp == NULL) {
        return;
    }
    empty_buffer(zmap_tmp, zmap_bloks * MINIX_BLOCK_SIZE);
    copy_bytes(fs->zmap_data, zmap_tmp, fs->sb.s_nzones / 8);
    size_t zmap_start_sector = SIZE_BOOT + SIZE_SB + (imap_bloks * NUMBER_OF_SECTORS_IN_DATA_ZONE);
    for (size_t i = 0; i < zmap_bloks * NUMBER_OF_SECTORS_IN_DATA_ZONE; ++i) {
        disk_write_sector(fs->disk, zmap_start_sector + i, (void*)((char*)zmap_tmp + i * DISK_SECTOR_SIZE));
    }
    kfree(zmap_tmp);

    kfree((void*)fs->imap_data);
    kfree((void*)fs->zmap_data);
    kfree((void*)fs);
    return;
}

/**
 * This function is a part of the creating dummy minix fs in the disk
 * @param disk disk where to create file system instance
 * @return error (e.g. ENOMEM or EOK)
 */
errno_t create_sb(disk_t* disk) {
    void* buffer = kmalloc(DISK_SECTOR_SIZE);
    empty_buffer(buffer, DISK_SECTOR_SIZE);
    if (buffer == NULL) {
        return ENOMEM;
    }

    minix_superblock_t superblock_to_write = {
        .s_ninodes = NUMBER_INODES,
        .s_nzones = NUMBER_ZONES,
        .s_imap_blocks = BM_SIZE_BLOCKS(NUMBER_INODES),
        .s_zmap_blocks = BM_SIZE_BLOCKS(NUMBER_ZONES),
        .s_firstdatazone = FIRST_DATA_ZONE,
        .s_log_zone_size = LOG_ZONE_SIZE,
        .s_max_size = MAX_SIZE,
        .s_magic = MINIX_MAGIC,
        .s_state = 1
    };

    *(minix_superblock_t*)buffer = superblock_to_write;

    disk_write_sector(disk, SB_DISK_SECTOR, buffer);

    kfree(buffer);

    return EOK;
}

/**
 * This function creates bitmaps for new instance of Minix file system, uses defined macros
 *
 * @param disk sisk where to write bitmaps
 * @return error (ENOMEM or EOK)
 */
errno_t set_bms(disk_t* disk) {
    bitmap_t bm_inodes;
    bitmap_t bm_zones;

    uint8_t bm_inodes_storage[NUMBER_INODES / 8];
    uint8_t bm_zones_storage[NUMBER_ZONES / 8];

    bitmap_init(&bm_inodes, NUMBER_INODES, bm_inodes_storage);
    bitmap_init(&bm_zones, NUMBER_ZONES, bm_zones_storage);

    bitmap_set(&bm_inodes, 0, 1);
    bitmap_set(&bm_zones, 0, 1);

    // bitmap for inodes
    size_t bytes_needed_for_bm_inodes = (((NUMBER_INODES / 8) + MINIX_BLOCK_SIZE - 1) / MINIX_BLOCK_SIZE) * MINIX_BLOCK_SIZE;
    void* buffer_inodes = kmalloc(bytes_needed_for_bm_inodes);
    CHECK_NULL(buffer_inodes);

    for (int i = 0; i < NUMBER_INODES / 8; ++i) {
        *((uint8_t*)buffer_inodes + i) = bm_inodes_storage[i];
    }

    for (size_t i = 0; i < BM_SIZE_BLOCKS(NUMBER_INODES) * NUMBER_OF_SECTORS_IN_DATA_ZONE; i += DISK_SECTOR_SIZE) {
        disk_write_sector(disk, SIZE_BOOT + SIZE_SB + i, (void*)((char*)buffer_inodes + i));
    }

    kfree(buffer_inodes);

    // bitmap for zones
    size_t bytes_needed_for_bm_zones = (((NUMBER_ZONES / 8) + MINIX_BLOCK_SIZE - 1) / MINIX_BLOCK_SIZE) * MINIX_BLOCK_SIZE;
    void* buffer_zones = kmalloc(bytes_needed_for_bm_zones);
    CHECK_NULL(buffer_zones);

    for (int i = 0; i < NUMBER_INODES / 8; ++i) {
        *((uint8_t*)buffer_zones + i) = bm_zones_storage[i];
    }

    for (size_t i = 0; i < BM_SIZE_BLOCKS(NUMBER_INODES) * NUMBER_OF_SECTORS_IN_DATA_ZONE; i += DISK_SECTOR_SIZE) {
        disk_write_sector(disk, SIZE_BOOT + SIZE_SB + BM_SIZE_BLOCKS(NUMBER_INODES) * NUMBER_OF_SECTORS_IN_DATA_ZONE + i, (void*)((char*)buffer_zones + i));
    }

    kfree(buffer_zones);

    return EOK;
}

/**
 * Format the given disk with a new MinixFS filesystem.
 *
 * This should write all the control blocks and fill the superblock with
 * appropriate values. After this call, the disk should be ready to be mounted
 * and used.
 */
errno_t minixfs_format(disk_t* disk) {

    errno_t err = create_sb(disk);
    if (err != EOK) {
        return err;
    }

    err = set_bms(disk);

    if (err != EOK) {
        return err;
    }

    // root inode to inode table
    {
        minix_inode_t root_dir_inode = {
            .i_nlinks = 2,
            .i_size = 32,
            .i_mode = MINIX_S_IFDIR,
            .i_zone[0] = FIRST_DATA_ZONE
        };

        void* buffer = kmalloc(DISK_SECTOR_SIZE);
        CHECK_NULL(buffer);

        *(minix_inode_t*)buffer = root_dir_inode;

        disk_write_sector(disk, INODE_TABLE_OFFSET_SECTOR(NUMBER_INODES, NUMBER_ZONES), buffer);
        kfree(buffer);
    }

    // zone entry
    {
        minix_dirent_t dir_entry_dot = {
            .inode = 1,
            .name = "."
        };

        minix_dirent_t dir_entry_dot_2 = {
            .inode = 1,
            .name = ".."
        };

        void* buffer = kmalloc(DISK_SECTOR_SIZE);
        empty_buffer(buffer, DISK_SECTOR_SIZE);
        CHECK_NULL(buffer);

        *(minix_dirent_t*)buffer = dir_entry_dot;
        *(minix_dirent_t*)((uint8_t*)buffer + sizeof(minix_dirent_t)) = dir_entry_dot_2;

        disk_write_sector(disk, FIRST_DATA_ZONE * NUMBER_OF_SECTORS_IN_DATA_ZONE, buffer);

        kfree(buffer);
    }

    return EOK;
}

/** Compare two MINIX file names
 *
 * @param str1 First string.
 * @param str2 Second string.
 * @return TRUE if same, else FALSE
 */
static bool name_files_comp_minix(const char* str1, const char* str2) {
    for (int i = 0; i < MINIX_NAME_LEN; ++i) {
        if (str1[i] != str2[i]) {
            return false;
        }
        if (str1[i] == '\0') {
            return true;
        }
    }
    return true;
}

/** Finds name in the directory and writes its inode into result_ino
 *
 * @param data_dir buffer with dir_inode.
 * @param name name to compare.
 * @param result_ino result to write.
 * @return TRUE if found, else FALSE
 */
bool find_dirent_ino(void* data_dir, size_t dir_size, const char* name, uint16_t* result_ino) {
    for (size_t i = 0; i < dir_size; i += sizeof(minix_dirent_t)) {
        minix_dirent_t* dir = (void*)((char*)(data_dir) + i);

        if (dir->inode != 0 && name_files_comp_minix(dir->name, name)) {
            *result_ino = dir->inode;
            return true;
        }
    }
    return false;
}

/** Calculates zone of provided inode of directory.
 * Suppose only first zone has appropriate zone block number.
 *
 * @param dir_ino Inode of the directory.
 * @param inode_table Inode table.
 * @return Zone block number with directory (uint16_t).
 */
uint16_t calculate_dir_zone_block(uint16_t dir_ino, minixfs_t* fs) {
    minix_inode_t dir;
    minixfs_read_inode(fs, dir_ino, &dir);
    return dir.i_zone[0];
}

/** Look up a name in a directory and return the inode number of the corresponding entry.
 *
 * @param fs Filesystem instance.
 * @param dir_ino Inode number of the directory to search in.
 * @param name Name of the entry to look up (null-terminated string if applicable).
 * @param result_ino Output parameter to receive the inode number of the found entry.
 */
errno_t minixfs_lookup(minixfs_t* fs, uint16_t dir_ino, const char* name, uint16_t* result_ino) {
    if (fs->mounted == false) {
        return ENOENT;
    }

    minix_inode_t dir;
    minixfs_read_inode(fs, dir_ino, &dir);

    void* data_dir = kmalloc(dir.i_size);
    CHECK_NULL(data_dir);

    size_t bytes_read = 0;
    minixfs_read_file(fs, &dir, data_dir, 0, dir.i_size, &bytes_read);
    bool found = find_dirent_ino(data_dir, dir.i_size, name, result_ino);

    kfree(data_dir);
    if (!found)
        return ENOENT;
    return EOK;
}

/**
 * Read the inode with the given number from disk into the provided structure.
 */
errno_t minixfs_read_inode(minixfs_t* fs, uint16_t ino, minix_inode_t* inode) {
    assert(ino > 0 && ino < fs->sb.s_ninodes);

    if (fs->mounted == false) {
        return ENOENT;
    }

    size_t ino_offset = (ino - 1) * sizeof(minix_inode_t);
    size_t ino_sector_offset = ino_offset / DISK_SECTOR_SIZE;
    ino_offset %= DISK_SECTOR_SIZE;

    void* inode_sector_buffer = kmalloc(DISK_SECTOR_SIZE);
    CHECK_NULL(inode_sector_buffer);

    disk_read_sector(fs->disk, INODE_TABLE_OFFSET_SECTOR(fs->sb.s_ninodes, fs->sb.s_nzones) + ino_sector_offset, inode_sector_buffer);
    *inode = *(minix_inode_t*)((char*)inode_sector_buffer + ino_offset);

    kfree(inode_sector_buffer);
    return EOK;
}

/**
 * This part stolen from process.c and reworked to support bytes copy and volatile.
 * @param from_addr Address from.
 * @param to_addr Address to.
 * @param size Size in bytes.
 */
void copy_bytes(void* from_addr, void* to_addr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        ((volatile uint8_t*)to_addr)[i] = ((volatile uint8_t*)from_addr)[i];
    }
}

/**
 * Function returnes minimum of two uint16_t numbers
 * @param n1 First number.
 * @param n2 Second number.
 * @return uint16_t minimum.
 */
inline uint16_t uint_16_min(uint16_t n1, uint16_t n2) {
    return (n1 < n2) ? n1 : n2;
}

/**
 * Debug function to print whatever there is in some address,
 * used for dumping buffers for disk_read_sector.
 * Prints chars.
 * @note To print uint values just change %c to %u and change size to
 * size / sizeof(uint_...), be careful it won't print every byte if you do so.
 *
 * @param buf void* to read values from.
 * @param size how many bytes to read
 */
void print_bytes_char(void* buf, size_t size) {
    printkl("\n================");
    printkl("================");
    printkl("Stored values: ");
    printkl("================");
    for (size_t i = 0; i < size; ++i) {
        printk("%c", *((char*)buf + i));
    }
    printkl("================");
    printkl("================");
}

/**
 * Used to clear buffer after kmalloc called.
 * @note Be careful to use size from kmalloc.
 * If you insert bigger size, it will clear some unrelated values.
 *
 * @param buffer Which buffer to clear.
 * @param size How many bytes to clear.
 */
void empty_buffer(void* buffer, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        *((char*)buffer + i) = 0;
    }
}

/**
 * This function reads disk from the given zone_number to buffer.
 * @param buffer Buffer to store values
 * @param zone_number Minix zone to read
 * @param disk From which disk to read
 *
 * @note This function does not check if the buffer is big enough.
 */
errno_t read_zone(void* buffer, uint16_t zone_number, disk_t* disk) {
    for (int i = 0; i < NUMBER_OF_SECTORS_IN_DATA_ZONE; ++i) {
        disk_read_sector(disk, zone_number * NUMBER_OF_SECTORS_IN_DATA_ZONE + i, (void*)((uint8_t*)buffer + (DISK_SECTOR_SIZE * i)));
    }
    return EOK;
}

uint16_t alloc_zone(minixfs_t* fs) {
    size_t free_zone_idx = 0;

    if (bitmap_find_range(&fs->zmap, 1, 0, &free_zone_idx) != EOK) {
        return 0;
    }
    if (free_zone_idx == 0) {
        return 0;
    }

    bitmap_set(&fs->zmap, free_zone_idx, 1);

    uint16_t actual_zone = (uint16_t)(free_zone_idx + fs->sb.s_firstdatazone - 1);

    void* empty_buf = kmalloc(MINIX_BLOCK_SIZE);
    if (empty_buf != NULL) {
        empty_buffer(empty_buf, MINIX_BLOCK_SIZE);
        write_zone(empty_buf, actual_zone, fs->disk);
        kfree(empty_buf);
    }

    return actual_zone;
}

static uint16_t resolve_zone(minix_inode_t* inode, size_t offset, minixfs_t* fs, bool alloc) {
    if (offset < DIRECT_BOUNDARY) {
        size_t idx = offset / MINIX_BLOCK_SIZE;
        if (inode->i_zone[idx] == 0 && alloc) {
            inode->i_zone[idx] = alloc_zone(fs);
        }
        return inode->i_zone[idx];
    }

    if (offset < INDIRECT_BOUNDARY) {
        if (inode->i_zone[NUMBER_DIRECT_ZONES] == 0) {
            if (!alloc)
                return 0;
            inode->i_zone[NUMBER_DIRECT_ZONES] = alloc_zone(fs);
        }
        uint16_t ind_zone = inode->i_zone[NUMBER_DIRECT_ZONES];
        if (ind_zone == 0)
            return 0;

        uint16_t* zones = kmalloc(MINIX_BLOCK_SIZE);
        if (zones == NULL)
            return 0;

        read_zone(zones, ind_zone, fs->disk);

        size_t idx = (offset - DIRECT_BOUNDARY) / MINIX_BLOCK_SIZE;
        if (zones[idx] == 0 && alloc) {
            zones[idx] = alloc_zone(fs);
            write_zone(zones, ind_zone, fs->disk);
        }
        uint16_t result = zones[idx];
        kfree(zones);
        return result;
    }

    // Double indirect
    if (inode->i_zone[NUMBER_DIRECT_ZONES + 1] == 0) {
        if (!alloc)
            return 0;
        inode->i_zone[NUMBER_DIRECT_ZONES + 1] = alloc_zone(fs);
    }
    uint16_t d_ind_zone = inode->i_zone[NUMBER_DIRECT_ZONES + 1];
    if (d_ind_zone == 0)
        return 0;

    uint16_t* zones1 = kmalloc(MINIX_BLOCK_SIZE);
    if (zones1 == NULL)
        return 0;

    read_zone(zones1, d_ind_zone, fs->disk);

    size_t rel_offset = offset - INDIRECT_BOUNDARY;
    size_t idx1 = rel_offset / (NUMBER_ENTRIES_ZONE * MINIX_BLOCK_SIZE);

    if (zones1[idx1] == 0) {
        if (!alloc) {
            kfree(zones1);
            return 0;
        }
        zones1[idx1] = alloc_zone(fs);
        write_zone(zones1, d_ind_zone, fs->disk);
    }
    uint16_t s_ind_zone = zones1[idx1];
    kfree(zones1);

    if (s_ind_zone == 0)
        return 0;

    uint16_t* zones2 = kmalloc(MINIX_BLOCK_SIZE);
    if (zones2 == NULL)
        return 0;

    read_zone(zones2, s_ind_zone, fs->disk);

    size_t idx2 = (rel_offset % (NUMBER_ENTRIES_ZONE * MINIX_BLOCK_SIZE)) / MINIX_BLOCK_SIZE;
    if (zones2[idx2] == 0 && alloc) {
        zones2[idx2] = alloc_zone(fs);
        write_zone(zones2, s_ind_zone, fs->disk);
    }
    uint16_t result = zones2[idx2];
    kfree(zones2);

    return result;
}

/**
 * This function checks if the file is not too big.
 *
 * @param number1 First number
 * @param number2 Second number
 *
 * @return EOK if there is no overflow, ENOMEM if there is.
 *
 * @note ENOMEM because this function primarily used for checking offset and size of the read_file function.
 */
errno_t check_overflow(uint32_t number1, uint32_t number2) {
    uint32_t result = number1 + number2;
    if (result < number1 || result > MAX_SIZE_FILE) {
        return ENOMEM;
    }
    return EOK;
}

/** Read data from a file represented by the given inode into the provided buffer.
 *
 * @param fs Filesystem instance.
 * @param inode Inode of the file to read from.
 * @param buffer Buffer to read the data into.
 * @param offset Byte offset in the file to start reading from.
 * @param size Number of bytes to read.
 *
 * @note Also because of the size bytes_read (32b size_t) the function can not
 * read files bigger than 7KB + 512KB + 256MB (potentionally).
 * This includes offset. E.g. if you want to read only 20B with offset
 * 1GB this function will return ENOENT. Minix does not support bigger files
 */
errno_t minixfs_read_file(minixfs_t* fs, minix_inode_t* inode, void* buffer, uint32_t offset, uint32_t size, size_t* bytes_read) {
    if (fs->mounted == false)
        return ENOENT;
    if (check_overflow(offset, size) != EOK)
        return ENOENT;
    assert(size + offset < fs->sb.s_max_size);

    if (offset >= inode->i_size) {
        if (bytes_read)
            *bytes_read = 0;
        return EOK;
    }

    if (offset + size > inode->i_size) {
        size = inode->i_size - offset;
    }

    size_t tmp_size = size;
    uint32_t tmp_offset = offset;

    size_t current_pos = offset;

    size_t bytes_really_read = 0;
    bool offset_used = false;

    if (bytes_read) {
        *bytes_read = 0;
    }

    while (true) {
        uint16_t zone = resolve_zone(inode, current_pos, fs, false);

        size_t bytes_to_copy = uint_16_min(tmp_size, MINIX_BLOCK_SIZE - (tmp_offset % MINIX_BLOCK_SIZE));

        if (zone == 0) {
            empty_buffer((void*)((uint8_t*)buffer + bytes_really_read), bytes_to_copy);
        } else {
            void* zone_buffer = kmalloc(MINIX_BLOCK_SIZE);
            CHECK_NULL(zone_buffer);

            read_zone(zone_buffer, zone, fs->disk);

            void* copy_from = zone_buffer;
            if (!offset_used) {
                copy_from = (void*)((uint8_t*)zone_buffer + (tmp_offset % MINIX_BLOCK_SIZE));
                offset_used = true;
                tmp_offset = 0;
            }

            copy_bytes(copy_from, (void*)((uint8_t*)buffer + bytes_really_read), bytes_to_copy);

            kfree(zone_buffer);
        }

        current_pos += bytes_to_copy;
        bytes_really_read += bytes_to_copy;
        tmp_size -= bytes_to_copy;

        if (tmp_size == 0) {
            if (bytes_read)
                *bytes_read = bytes_really_read;
            return EOK;
        }
    }

    return ENOENT;
}

/*
 * Methods below are part of an extra points task, so you can choose to
 * implement them after finishing the main part. They are not required for basic
 * filesystem functionality, but they will allow you to create directories and
 * delete files, which can be useful for testing and usability.
 */

/**
 * Write the given inode structure to disk at the location corresponding to the
 * given inode number.
 */
errno_t minixfs_write_inode(minixfs_t* fs, uint16_t ino, const minix_inode_t* inode) {
    assert(ino > 0 && ino < fs->sb.s_ninodes);

    if (fs->mounted == false) {
        return ENOENT;
    }

    size_t ino_offset = (ino - 1) * sizeof(minix_inode_t);
    size_t ino_sector_offset = ino_offset / DISK_SECTOR_SIZE;
    ino_offset %= DISK_SECTOR_SIZE;

    void* inode_sector_buffer = kmalloc(DISK_SECTOR_SIZE);
    CHECK_NULL(inode_sector_buffer);

    disk_read_sector(fs->disk, INODE_TABLE_OFFSET_SECTOR(fs->sb.s_ninodes, fs->sb.s_nzones) + ino_sector_offset, inode_sector_buffer);

    *(minix_inode_t*)((char*)inode_sector_buffer + ino_offset) = *inode;

    disk_write_sector(fs->disk, INODE_TABLE_OFFSET_SECTOR(fs->sb.s_ninodes, fs->sb.s_nzones) + ino_sector_offset, inode_sector_buffer);

    kfree(inode_sector_buffer);
    return EOK;
}

/** Create a new file with the given name and mode in the specified directory.
 *
 * @param fs Filesystem instance.
 * @param dir_ino Number of the directory to create the file in.
 * @param name Name of the new file (null-terminated string if applicable).
 * @param mode File mode (permissions and type).
 * @param new_ino Output parameter to receive the inode number of the newly created file.
 */
errno_t minixfs_create_file(minixfs_t* fs, uint16_t dir_ino, const char* name, uint16_t mode, uint16_t* new_ino) {
    size_t free_index = 0;
    bitmap_find_range(&fs->imap, 1, 0, &free_index);
    bitmap_set(&fs->imap, free_index, 1);
    uint16_t actual_ino = (uint16_t)(free_index + 1);
    *new_ino = actual_ino;

    minix_inode_t new_inode_file = {
        .i_gid = 0,
        .i_mode = mode,
        .i_nlinks = 1,
        .i_size = 0,
        .i_time = 0,
        .i_uid = 0,
        .i_zone[0] = 0,
        .i_zone[1] = 0,
        .i_zone[2] = 0,
        .i_zone[3] = 0,
        .i_zone[4] = 0,
        .i_zone[5] = 0,
        .i_zone[6] = 0,
        .i_zone[7] = 0,
        .i_zone[8] = 0
    };
    minixfs_write_inode(fs, actual_ino, &new_inode_file);

    minix_inode_t dir_inode;
    minixfs_read_inode(fs, dir_ino, &dir_inode);

    void* data_dir = kmalloc(dir_inode.i_size + sizeof(minix_dirent_t));
    CHECK_NULL(data_dir);

    size_t bytes_read = 0;
    minixfs_read_file(fs, &dir_inode, data_dir, 0, dir_inode.i_size, &bytes_read);

    minix_dirent_t new_entry = {
        .inode = actual_ino
    };

    for (size_t i = 0; i < MINIX_NAME_LEN; ++i) {
        new_entry.name[i] = name[i];
        if (new_entry.name[i] == '\0') {
            break;
        }
    }

    *(minix_dirent_t*)((char*)data_dir + dir_inode.i_size) = new_entry;

    size_t bw = 0;
    minixfs_write_file(fs, &dir_inode, data_dir, 0, dir_inode.i_size + sizeof(minix_dirent_t), &bw);
    dir_inode.i_size += sizeof(minix_dirent_t);
    minixfs_write_inode(fs, dir_ino, &dir_inode);

    kfree(data_dir);

    return EOK;
}

/**
 * Writes a data block to the disk.
 *
 * @param buffer pointer to the data to be written (MINIX_BLOCK_SIZE)
 * @param zone_number zone number on the disk
 * @param disk pointer to the disk device
 * @return EOK if success.
 */
errno_t write_zone(const void* buffer, uint16_t zone_number, disk_t* disk) {
    for (int i = 0; i < NUMBER_OF_SECTORS_IN_DATA_ZONE; ++i) {
        disk_write_sector(disk, zone_number * NUMBER_OF_SECTORS_IN_DATA_ZONE + i, (void*)((uint8_t*)buffer + (DISK_SECTOR_SIZE * i)));
    }
    return EOK;
}

/** Write data to a file represented by the given inode from the provided buffer.
 *
 * @param fs Filesystem instance.
 * @param inode Inode of the file to write to.
 * @param buffer Buffer containing the data to write.
 * @param offset Byte offset in the file to start writing to.
 * @param size Number of bytes to write.
 */
errno_t minixfs_write_file(minixfs_t* fs, minix_inode_t* inode, const void* buffer, uint32_t offset, uint32_t size, size_t* bytes_written) {
    if (fs->mounted == false) {
        return ENOENT;
    }

    if (check_overflow(offset, size) != EOK) {
        return ENOMEM;
    }

    assert(size + offset < fs->sb.s_max_size);

    size_t tmp_size = size;
    uint32_t tmp_offset = offset;
    size_t current_pos = offset;
    *bytes_written = 0;

    bool offset_used = false;

    while (tmp_size > 0) {
        uint16_t zone = resolve_zone(inode, current_pos, fs, true);
        if (zone == 0) {
            return ENOMEM;
        }

        void* zone_buffer = kmalloc(MINIX_BLOCK_SIZE);
        CHECK_NULL(zone_buffer);

        size_t bytes_to_copy = uint_16_min(tmp_size, MINIX_BLOCK_SIZE - (tmp_offset % MINIX_BLOCK_SIZE));

        if (bytes_to_copy < MINIX_BLOCK_SIZE && (tmp_offset % MINIX_BLOCK_SIZE != 0 || tmp_size < MINIX_BLOCK_SIZE)) {
            read_zone(zone_buffer, zone, fs->disk);
        }

        void* copy_to = zone_buffer;

        if (!offset_used) {
            copy_to = (void*)((uint8_t*)zone_buffer + (tmp_offset % MINIX_BLOCK_SIZE));
            offset_used = true;
            tmp_offset = 0;
        }

        copy_bytes((void*)((uint8_t*)buffer + *bytes_written), copy_to, bytes_to_copy);

        write_zone(zone_buffer, zone, fs->disk);

        *bytes_written += bytes_to_copy;
        current_pos += bytes_to_copy;
        tmp_size -= bytes_to_copy;

        kfree(zone_buffer);
    }

    if (offset + *bytes_written > inode->i_size) {
        inode->i_size = offset + *bytes_written;
    }

    return EOK;
}

/** Create a new directory with the given name and mode in the specified directory.
 *
 * @param fs Filesystem instance.
 * @param dir_ino Number of the directory to create the new directory in.
 * @param name Name of the new directory (null-terminated string if applicable).
 * @param mode Directory mode (permissions and type).
 * @param new_ino Output parameter to receive the inode number of the newly created directory.
 */
errno_t minixfs_mkdir(minixfs_t* fs, uint16_t dir_ino, const char* name, uint16_t mode, uint16_t* new_ino) {
    if (fs->mounted == false)
        return ENOENT;

    size_t free_index = 0;
    if (bitmap_find_range(&fs->imap, 1, 0, &free_index) != EOK) {
        return ENOMEM;
    }

    bitmap_set(&fs->imap, free_index, 1);
    uint16_t actual_ino = (uint16_t)(free_index + 1);
    *new_ino = actual_ino;

    minix_inode_t new_dir_inode = {
        .i_gid = 0,
        .i_mode = mode,
        .i_nlinks = 2,
        .i_size = 0,
        .i_time = 0,
        .i_uid = 0,
        .i_zone[0] = 0,
        .i_zone[1] = 0,
        .i_zone[2] = 0,
        .i_zone[3] = 0,
        .i_zone[4] = 0,
        .i_zone[5] = 0,
        .i_zone[6] = 0,
        .i_zone[7] = 0,
        .i_zone[8] = 0
    };

    minix_dirent_t dot_entries[2];
    empty_buffer(dot_entries, sizeof(dot_entries));

    dot_entries[0].inode = actual_ino;
    dot_entries[0].name[0] = '.';

    dot_entries[1].inode = dir_ino;
    dot_entries[1].name[0] = '.';
    dot_entries[1].name[1] = '.';

    size_t written = 0;
    errno_t err = minixfs_write_file(fs, &new_dir_inode, dot_entries, 0, sizeof(dot_entries), &written);
    if (err != EOK)
        return err;

    minixfs_write_inode(fs, actual_ino, &new_dir_inode);

    minix_inode_t parent_inode;
    minixfs_read_inode(fs, dir_ino, &parent_inode);

    minix_dirent_t new_entry;
    empty_buffer(&new_entry, sizeof(minix_dirent_t));
    new_entry.inode = actual_ino;

    for (size_t i = 0; i < MINIX_NAME_LEN; ++i) {
        new_entry.name[i] = name[i];
        if (name[i] == '\0')
            break;
    }

    err = minixfs_write_file(fs, &parent_inode, &new_entry, parent_inode.i_size, sizeof(minix_dirent_t), &written);
    if (err != EOK)
        return err;

    parent_inode.i_nlinks += 1;
    minixfs_write_inode(fs, dir_ino, &parent_inode);

    return EOK;
}