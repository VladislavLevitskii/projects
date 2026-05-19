// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 Charles University

#ifndef _FS_MINIX_H
#define _FS_MINIX_H

#include <adt/bitmap.h>
#include <drivers/disk.h>
#include <errno.h>
#include <types.h>

#define MINIX_MAGIC 0x137F //  MinixFS v1 magic number
#define MINIX_BLOCK_SIZE (2 * (DISK_SECTOR_SIZE)) // 1KB blocks (2 sectors)
#define MINIX_NAME_LEN 14 // Max filename length

#define MINIX_ROOT_INO 1 // Root directory inode number

//=======
// dummy Minix
#define NUMBER_INODES 360
#define NUMBER_ZONES 4096
#define FIRST_DATA_ZONE 27
#define LOG_ZONE_SIZE 0
#define MAX_SIZE 0x04000000
//=======

#define NUMBER_DIRECT_ZONES 7

#define SIZE_BOOT 2
#define SIZE_SB 2

#define SB_DISK_SECTOR 2

#define NUMBER_OF_SECTORS_IN_DATA_ZONE 2

#define NUMBER_ENTRIES_ZONE ((MINIX_BLOCK_SIZE) / sizeof(uint16_t))

#define DIRECT_BOUNDARY (NUMBER_DIRECT_ZONES) * (MINIX_BLOCK_SIZE) // 7KB
#define INDIRECT_BOUNDARY ((NUMBER_ENTRIES_ZONE) * (MINIX_BLOCK_SIZE)) + (DIRECT_BOUNDARY) // 519KB
#define MAX_SIZE_FILE (DIRECT_BOUNDARY) + (INDIRECT_BOUNDARY) + ((NUMBER_ENTRIES_ZONE) * (NUMBER_ENTRIES_ZONE) * (MINIX_BLOCK_SIZE)) // 7KB + 519KB + 256MB

#define BM_SIZE_BLOCKS(number_inodes) (((number_inodes) + ((MINIX_BLOCK_SIZE) * 8 - 1)) / ((MINIX_BLOCK_SIZE) * 8))

#define INODE_TABLE_OFFSET_SECTOR(number_inodes, number_zones) ((SIZE_BOOT) + (SIZE_SB) + (BM_SIZE_BLOCKS(number_inodes)) * 2 + (BM_SIZE_BLOCKS(number_zones)) * 2)
#define INODE_TABLE_SIZE_BYTES(number_inodes) ((MINIX_BLOCK_SIZE) * (((number_inodes) + (MINIX_BLOCK_SIZE) / sizeof(minix_inode_t) - 1) / ((MINIX_BLOCK_SIZE) / sizeof(minix_inode_t))))

typedef struct {
    uint16_t s_ninodes; // Number of inodes
    uint16_t s_nzones; // Number of data zones
    uint16_t s_imap_blocks; // Inode bitmap blocks
    uint16_t s_zmap_blocks; // Zone bitmap blocks
    uint16_t s_firstdatazone; // First data zone
    uint16_t s_log_zone_size; // log2(block_size/1024)
    uint32_t s_max_size; // Maximum file size
    uint16_t s_magic; // Magic number
    uint16_t s_state; // Filesystem state
} minix_superblock_t;

typedef struct {
    uint16_t i_mode; // File mode (type + permissions)
    uint16_t i_uid; // User ID (unused)
    uint32_t i_size; // File size in bytes
    uint32_t i_time; // Modification time (unused)
    uint8_t i_gid; // Group ID (unused)
    uint8_t i_nlinks; // Number of links
    uint16_t i_zone[9]; // Data zone pointers
                        // [0-6]: direct zones
                        // [7]: indirect zone
                        // [8]: double indirect zone
} minix_inode_t;

typedef struct {
    uint16_t inode; // Inode number (0 = unused entry)
    char name[MINIX_NAME_LEN]; // Filename (not null-terminated if exactly 14 chars)
} minix_dirent_t;

// File mode bits
#define MINIX_S_IFMT 0170000 // File type mask
#define MINIX_S_IFREG 0100000 // Regular file
#define MINIX_S_IFDIR 0040000 // Directory
#define MINIX_S_IFCHR 0020000 // Character device
#define MINIX_S_IFBLK 0060000 // Block device

// Permission bits
#define MINIX_S_ISUID 0004000 // Set UID
#define MINIX_S_ISGID 0002000 // Set GID
#define MINIX_S_ISVTX 0001000 // Sticky bit
#define MINIX_S_IRWXU 0000700 // User rwx
#define MINIX_S_IRUSR 0000400 // User read
#define MINIX_S_IWUSR 0000200 // User write
#define MINIX_S_IXUSR 0000100 // User execute
#define MINIX_S_IRWXG 0000070 // Group rwx
#define MINIX_S_IRGRP 0000040 // Group read
#define MINIX_S_IWGRP 0000020 // Group write
#define MINIX_S_IXGRP 0000010 // Group execute
#define MINIX_S_IRWXO 0000007 // Other rwx
#define MINIX_S_IROTH 0000004 // Other read
#define MINIX_S_IWOTH 0000002 // Other write
#define MINIX_S_IXOTH 0000001 // Other execute

#define MINIX_ISREG(m) (((m) & MINIX_S_IFMT) == MINIX_S_IFREG)
#define MINIX_ISDIR(m) (((m) & MINIX_S_IFMT) == MINIX_S_IFDIR)
#define MINIX_ISCHR(m) (((m) & MINIX_S_IFMT) == MINIX_S_IFCHR)
#define MINIX_ISBLK(m) (((m) & MINIX_S_IFMT) == MINIX_S_IFBLK)

typedef struct {
    disk_t* disk; // Disk device
    minix_superblock_t sb; // Cached superblock
    bitmap_t imap; // Inode bitmap
    bitmap_t zmap; // Zone bitmap
    uint8_t* imap_data; // Inode bitmap storage
    uint8_t* zmap_data; // Zone bitmap storage
    bool mounted;
} minixfs_t;

extern minixfs_t* minixfs;

bool find_dirent_ino(void* data_dir, size_t dir_size, const char* name, uint16_t* result_ino);
uint16_t calculate_dir_zone_block(uint16_t dir_ino, minixfs_t* fs);
void print_bytes_char(void* buf, size_t size);
void empty_buffer(void* buffer, size_t size);
uint16_t get_number_zone(minix_inode_t* inode, size_t* bytes_read, minixfs_t* fs);
uint16_t uint_16_min(uint16_t n1, uint16_t n2);
void copy_bytes(void* from_addr, void* to_addr, size_t size);
errno_t read_zone(void* buffer, uint16_t zone_number, disk_t* disk);
errno_t check_overflow(uint32_t number1, uint32_t number2);
errno_t write_zone(const void* buffer, uint16_t zone_number, disk_t* disk);
uint16_t alloc_zone(minixfs_t* fs);
uint16_t get_or_alloc_number_zone(minix_inode_t* inode, size_t offset, minixfs_t* fs);

errno_t create_sb(disk_t* disk);
errno_t set_bms(disk_t* disk);

errno_t minixfs_init(disk_t* disk);
minixfs_t* minixfs_get_current(void);

errno_t minixfs_mount(minixfs_t* fs);
void minixfs_unmount(minixfs_t* fs);

errno_t minixfs_format(disk_t* disk);

errno_t minixfs_lookup(minixfs_t* fs, uint16_t dir_ino, const char* name,
        uint16_t* result_ino);

errno_t minixfs_read_inode(minixfs_t* fs, uint16_t ino, minix_inode_t* inode);

errno_t minixfs_read_file(minixfs_t* fs, minix_inode_t* inode, void* buffer,
        uint32_t offset, uint32_t size, size_t* bytes_read);

// --- Extension functions ---

errno_t minixfs_write_inode(minixfs_t* fs, uint16_t ino, const minix_inode_t* inode);

errno_t minixfs_create_file(minixfs_t* fs, uint16_t dir_ino, const char* name,
        uint16_t mode, uint16_t* new_ino);
errno_t minixfs_write_file(minixfs_t* fs, minix_inode_t* inode,
        const void* buffer, uint32_t offset, uint32_t size, size_t* bytes_written);

errno_t minixfs_mkdir(minixfs_t* fs, uint16_t dir_ino, const char* name,
        uint16_t mode, uint16_t* new_ino);

#endif // _FS_MINIX_H
