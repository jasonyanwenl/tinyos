#ifndef __FS_SUPER_BLOCK_H
#define __FS_SUPER_BLOCK_H

#include "stdint.h"

struct super_block {
    uint32_t magic;                     // file system type label
    uint32_t sec_cnt;                   // totoal sectors in this partition
    uint32_t inode_cnt;                 // total inodes in this partition
    uint32_t part_lba_base;             // start lba in this partition

    uint32_t block_bitmap_lba;          // start lba
    uint32_t block_bitmap_sects;        // total sectors

    uint32_t inode_bitmap_lba;          // start lba
    uint32_t inode_bitmap_sects;        // total sectors

    uint32_t inode_table_lba;           // start lba
    uint32_t inode_table_sects;         // total sectors

    uint32_t data_start_lba;            // start lba for data area
    uint32_t root_inode_no;             // inode idx for root inode
    uint32_t dir_entry_size;

    uint8_t pad[460];                   // padding to fit 512 bytes for a sector
} __attribute__ ((packed));

#endif