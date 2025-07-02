#include "ext2/ext2.h"

#include "libc_kernel/string.h"
#include "heap/heap.h"
#include "block_device/block_device.h"

#define EXT2_SUPERBLOCK_LBA 2
#define EXT2_SUPERBLOCK_SECTORS 2
#define EXT2_MAX_BLOCK_SIZE 4096

#define EXT2_ROOT_INODE 2
#define EXT2_PATH_SEPERATOR "/"

static uint32_t block_to_lba(ext2_fs_t* fs, uint32_t block_num);
static void read_block_group_desc(ext2_fs_t* fs);
static bool read_inode(ext2_fs_t* fs, size_t inode_index, inode_t* out_inode);
static size_t read_dir_entry_list(ext2_fs_t* fs, inode_t* inode, dir_entry_t** dir_entries);
static void read_dir_entry(ext2_fs_t* fs, inode_t* inode, uint16_t start_offset, dir_entry_t* dir_entry);
static bool resolve_path(ext2_fs_t* fs, char* path, inode_t* inode);

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

uint8_t* read_file(ext2_fs_t* fs, char* path, size_t* out_size) {
    inode_t file_inode;
    if (!resolve_path(fs, path, &file_inode)) {
        return NULL;
    }

    bool is_file = (file_inode.mode & 0xF000) == EXT2_S_IFREG;
    if (!is_file) {
        return NULL;
    }

    *out_size = file_inode.size;

    uint8_t* buffer = (uint8_t*) kmalloc(*out_size);
    if (!buffer) {
        return NULL;
    }

    size_t total_bytes = *out_size;
    size_t total_blocks = (total_bytes + fs->block_size - 1) / fs->block_size;
    size_t read_blocks = 0;

    // Direct blocks
    for (; read_blocks < total_blocks && read_blocks < EXT2_DIRECT_BLOCKS; read_blocks++) {
        fs->device->read_sectors(
            block_to_lba(fs, file_inode.direct_blocks[read_blocks]),
            fs->block_size / BLOCK_SECTOR_SIZE,
            buffer + read_blocks * fs->block_size,
            fs->device->driver_data
        );
    }

    if (read_blocks == total_blocks) return buffer;

    // Single indirect
    uint32_t* indirect_block = (uint32_t*) kmalloc(fs->block_size);
    fs->device->read_sectors(
        block_to_lba(fs, file_inode.single_indirect_block),
        fs->block_size / BLOCK_SECTOR_SIZE,
        indirect_block,
        fs->device->driver_data
    );

    size_t indirect_count = fs->block_size / sizeof(uint32_t);
    for (size_t i = 0; read_blocks < total_blocks && i < indirect_count; i++, read_blocks++) {
        fs->device->read_sectors(
            block_to_lba(fs, indirect_block[i]),
            fs->block_size / BLOCK_SECTOR_SIZE,
            buffer + read_blocks * fs->block_size,
            fs->device->driver_data
        );
    }
    kfree(indirect_block);

    if (read_blocks == total_blocks) {
        return buffer;
    }

    // Double indirect
    uint32_t* dbl_indirect = (uint32_t*) kmalloc(fs->block_size);
    fs->device->read_sectors(
        block_to_lba(fs, file_inode.double_indirect_block),
        fs->block_size / BLOCK_SECTOR_SIZE,
        dbl_indirect,
        fs->device->driver_data
    );

    for (size_t i = 0; read_blocks < total_blocks && i < indirect_count; i++) {
        uint32_t* indirect = (uint32_t*) kmalloc(fs->block_size);
        fs->device->read_sectors(
            block_to_lba(fs, dbl_indirect[i]),
            fs->block_size / BLOCK_SECTOR_SIZE,
            indirect,
            fs->device->driver_data
        );
        for (size_t j = 0; read_blocks < total_blocks && j < indirect_count; j++, read_blocks++) {
            fs->device->read_sectors(
                block_to_lba(fs, indirect[j]),
                fs->block_size / BLOCK_SECTOR_SIZE,
                buffer + read_blocks * fs->block_size,
                fs->device->driver_data
            );
        }
        kfree(indirect);
    }
    kfree(dbl_indirect);

    if (read_blocks == total_blocks) {
        return buffer;
    }

    // Triple indirect
    uint32_t* tpl_indirect = (uint32_t*) kmalloc(fs->block_size);
    fs->device->read_sectors(
        block_to_lba(fs, file_inode.triple_indirect_block),
        fs->block_size / BLOCK_SECTOR_SIZE,
        tpl_indirect,
        fs->device->driver_data
    );

    for (size_t i = 0; read_blocks < total_blocks && i < indirect_count; i++) {
        uint32_t* dbl = (uint32_t*) kmalloc(fs->block_size);
        fs->device->read_sectors(
            block_to_lba(fs, tpl_indirect[i]),
            fs->block_size / BLOCK_SECTOR_SIZE,
            dbl,
            fs->device->driver_data
        );

        for (size_t j = 0; read_blocks < total_blocks && j < indirect_count; j++) {
            uint32_t* indirect = (uint32_t*) kmalloc(fs->block_size);
            fs->device->read_sectors(
                block_to_lba(fs, dbl[j]),
                fs->block_size / BLOCK_SECTOR_SIZE,
                indirect,
                fs->device->driver_data
            );

            for (size_t k = 0; read_blocks < total_blocks && k < indirect_count; k++, read_blocks++) {
                fs->device->read_sectors(
                    block_to_lba(fs, indirect[k]),
                    fs->block_size / BLOCK_SECTOR_SIZE,
                    buffer + read_blocks * fs->block_size,
                    fs->device->driver_data
                );
            }

            kfree(indirect);
        }
        kfree(dbl);
    }

    kfree(tpl_indirect);

    return buffer;
}

static uint32_t block_to_lba(ext2_fs_t* fs, uint32_t block_num) {
    uint32_t sectors_per_block = fs->block_size / BLOCK_SECTOR_SIZE;

    if (fs->block_size == 1024) {
        // block 0 = boot, block 1 = superblock at sector 2
        return (block_num + 1) * sectors_per_block;
    } else {
        return block_num * sectors_per_block;
    }
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
    uint8_t* block = kmalloc(fs->block_size);

    fs->device->read_sectors(lba, block_size / BLOCK_SECTOR_SIZE, block, fs->device->driver_data);
    kmemcpy(out_inode, block + offset_in_block, sizeof(inode_t));
    kfree(block);

    return true;
}

static size_t read_dir_entry_list(ext2_fs_t* fs, inode_t* inode, dir_entry_t** dir_entries) {
    if ((inode->mode & 0xF000) != EXT2_S_IFDIR) {
        return 0;
    }

    *dir_entries = (dir_entry_t*) kmalloc(inode->size);

    uint32_t inode_offset = 0;
    size_t dir_count = 0;

    while (inode_offset < inode->size) {
        dir_entry_t dir_entry;
        read_dir_entry(fs, inode, inode_offset, &dir_entry);

        kmemcpy((uint8_t*) *dir_entries + inode_offset, &dir_entry, dir_entry.rec_len);

        inode_offset += dir_entry.rec_len;
        dir_count++;
    }

    return dir_count;
}

static void read_dir_entry(ext2_fs_t* fs, inode_t* inode, uint16_t offset, dir_entry_t* dir_entry) {
    uint32_t block_index = offset / fs->block_size;
    uint32_t offset_in_block = offset % fs->block_size;

    uint32_t lba = block_to_lba(fs, inode->direct_blocks[block_index]);
    uint8_t* block = kmalloc(fs->block_size);

    fs->device->read_sectors(lba, fs->block_size / BLOCK_SECTOR_SIZE, block, fs->device->driver_data);
    
    dir_entry_t* entry_in_block = (dir_entry_t*) (block + offset_in_block);

    kmemcpy(dir_entry, entry_in_block, entry_in_block->rec_len);
    kfree(block);
}

static bool resolve_path(ext2_fs_t *fs, char *path, inode_t *out_inode) {
    // check if the path is absolute (starts with the line seperator)
    char* current_path_dir = kstrtok(path, EXT2_PATH_SEPERATOR);
    if (kstrcmp(current_path_dir, "") != 0) {
        return false;
    }

    // read root inode first
    inode_t current_inode;
    if (!read_inode(fs, EXT2_ROOT_INODE, &current_inode)) {
        return false;
    }

    bool found = false;
    while (current_path_dir) {
        current_path_dir = kstrtok(NULL, EXT2_PATH_SEPERATOR);
        if (current_path_dir == NULL) {
            return false;
        }
        
        dir_entry_t* dir_entry_list;
        size_t dir_count = read_dir_entry_list(fs, &current_inode, &dir_entry_list);

        for (size_t i = 0; i < dir_count; i++) {
            char* dir_name_str = kmalloc(dir_entry_list[i].name_len);
            kmemcpy(dir_name_str, dir_entry_list[i].name, dir_entry_list[i].name_len);
            
            // found the dir, switch the inode
            if (kstrcmp(dir_name_str, current_path_dir) == 0) {
                read_inode(fs, dir_entry_list[i].inode, &current_inode);
                found = true;
            }

            kfree(dir_name_str);

            if (found) {
                break;
            }
        }

        kfree(dir_entry_list);

        if (!found) {
            return false;
        }
    }

    kmemcpy(out_inode, &current_inode, sizeof(inode_t));

    return true;
}