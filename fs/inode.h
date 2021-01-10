#ifndef __FS_INODE_H
#define __FS_INODE_H

#include "stdint.h"
#include "list.h"

struct inode {
    uint32_t i_no;          // inode idx
    uint32_t i_size;
    uint32_t i_open_cnts;   // times have been opened
    bool write_deny;        // write cannot be parallel
    uint32_t i_sectors[13]; // [0-11] for direct block, [12] for indirect block
    struct list_elem inode_tag;
};

#endif