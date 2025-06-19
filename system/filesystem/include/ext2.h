#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>

#define EXT2_DIRECT_BLOCKS 12
#define SUPERBLOCK_SIZE 1024

typedef struct superblock_t {
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t superuser_blocks;
    uint32_t free_blocks;
    uint32_t free_inodes;
    uint32_t superblock_index;
    uint32_t log_2_block_size;
    uint32_t log_2_fragment_size;
    uint32_t blocks_per_group;
    uint32_t fragments_per_group;
    uint32_t inodes_per_group;
    uint32_t last_mount_time;
    uint32_t last_written_time;

    uint16_t mounts_since_last_check;
    uint16_t mounts_allowed_before_check;
    uint16_t signature;
    uint16_t fs_state;
    uint16_t error_detected_action;
    uint16_t minor_version;
    
    uint32_t last_check;
    uint32_t interval;
    uint32_t os_id;
    uint32_t major_version;
    
    uint16_t uid;
    uint16_t gid;

    // Extended features
    uint32_t first_inode;
    
    uint16_t inode_size;
    uint16_t superblock_group;
    
    uint32_t optional_feature;
    uint32_t required_feature;
    uint32_t readonly_feature;

    char fs_id[16];
    char vol_name[16];
    char last_mount_path[64];

    uint32_t compression_method;

    uint8_t file_pre_alloc_blocks;
    uint8_t dir_pre_alloc_blocks;
    
    uint16_t unused;

    char journal_id[16];

    uint32_t journal_inode;
    uint32_t journal_device;
    uint32_t orphan_head;
    // Extended features end

    char padding[SUPERBLOCK_SIZE - 236];
} __attribute__((packed)) superblock_t;

typedef struct inode_t {
    uint16_t permissions;
    uint16_t uid;

    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;

    uint16_t gid;
    uint16_t hard_links;
    
    uint32_t disk_sectors;
    uint32_t flags;
    uint32_t os_specific_value_1;
    uint32_t direct_blocks[EXT2_DIRECT_BLOCKS];
    uint32_t single_indirect_block;
    uint32_t double_indirect_block;
    uint32_t triple_indirect_block;
    uint32_t generation_number;
    uint32_t file_acl;
    union {
        uint32_t dir_acl;
        uint32_t file_size;
    };
    uint32_t block_fragment_addr;

    char os_specific_value_2[12];
} __attribute__((packed)) inode_t;

typedef struct dir_entry_t {
    uint32_t inode;
    uint16_t size;
    uint8_t name_len;
    uint8_t type;
    char name[];
} __attribute__((packed)) dir_entry_t;

typedef struct block_group_t {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;

    uint16_t free_blocks;
    uint16_t free_inodes;
    uint16_t dir_count;
    
    char unused[14];
} __attribute__((packed)) block_group_t;

#endif // FILESYSTEM_H
