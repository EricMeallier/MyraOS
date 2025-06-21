#include "ext2/ext2.h"

#include "libc_kernel/string.h"
#include "heap/heap.h"
#include "block_device/block_device.h"

#define EXT2_SUPERBLOCK_LBA 2
#define EXT2_SUPERBLOCK_SECTORS 2

static void read_block_group_desc(ext2_fs_t* fs);
static bool read_inode(ext2_fs_t* fs, size_t inode_index, inode_t* out_inode);

static uint32_t block_to_lba(ext2_fs_t* fs, uint32_t block_num) {
    uint32_t sectors_per_block = fs->block_size / BLOCK_SECTOR_SIZE;

    if (fs->block_size == 1024) {
        // block 0 = boot, block 1 = superblock at sector 2
        return (block_num + 1) * sectors_per_block;
    } else {
        return block_num * sectors_per_block;
    }
}

bool ext2_mount(ext2_fs_t* fs, block_device_t* device) {
    fs->superblock = kmalloc(sizeof(superblock_t));
    fs->device = device;

    // Read the superblock sectors
    uint8_t superblock[EXT2_SUPERBLOCK_SIZE];

    device->read_sectors(EXT2_SUPERBLOCK_LBA, EXT2_SUPERBLOCK_SECTORS, superblock, device->driver_data);

    kmemcpy(fs->superblock, superblock, sizeof(superblock_t));

    // Verify ext2 signature
    if (fs->superblock->signature != EXT2_SUPERBLOCK_SIGNATURE) {
        return false;
    }

    fs->block_size = (1024 << fs->superblock->log_2_block_size);
    read_block_group_desc(fs);

    return true;
}

static void read_block_group_desc(ext2_fs_t* fs) {
    fs->total_groups = fs->superblock->total_blocks / fs->superblock->blocks_per_group;

    uint32_t desc_size = fs->total_groups * sizeof(block_group_desc_t);
    uint32_t total_blocks = (desc_size + BLOCK_SECTOR_SIZE - 1) / BLOCK_SECTOR_SIZE;

    fs->group_desc = kmalloc(desc_size);

    uint32_t lba = (fs->block_size == 1024) ? 2 : 1;
    fs->device->read_sectors(lba, total_blocks, fs->group_desc, fs->device->driver_data);
}

static bool read_inode(ext2_fs_t* fs, size_t inode_index, inode_t* out_inode) {
    if (inode_index == 0 || inode_index > fs->superblock->total_inodes) {
        return false;
    }

    uint32_t inodes_per_group = fs->superblock->inodes_per_group;
    uint32_t block_size = fs->block_size;

    uint32_t group_index = (inode_index - 1) / inodes_per_group;
    uint32_t index_in_group = (inode_index - 1) % inodes_per_group;

    block_group_desc_t* group_desc = &fs->group_desc[group_index];
    uint32_t inode_table_block = group_desc->inode_table;

    uint32_t inode_offset = index_in_group * fs->superblock->inode_size;
    uint32_t block_offset = inode_table_block + inode_offset / block_size;
    uint32_t offset_in_block = inode_offset % block_size;

    uint32_t lba = block_to_lba(fs, block_offset);
    uint8_t block[block_size];

    fs->device->read_sectors(lba, block_size, block, fs->device->driver_data);
    kmemcpy(out_inode, block + offset_in_block, sizeof(inode_t));

    return true;
}