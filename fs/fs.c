#include "fs.h"
#include "inode.h"
#include "super_block.h"
#include "dir.h"
#include "stdio-kernel.h"

struct partition *cur_part;

static bool mount_partition(struct list_elem *pelem, int arg) {
    char *part_name = (char *)arg;
    struct partition *part = elem2entry(struct partition, part_tag, pelem);
    if (!strcmp(part->name, part_name)) {
        // found target partition
        cur_part = part;
        struct disk *hd = cur_part->my_disk;

        struct super_block *sb_buf = (struct super_block *)sys_malloc(SECTOR_SIZE);

        cur_part->sb = (struct super_block *)sys_malloc(sizeof(struct super_block));
        if (cur_part->sb == NULL) {
            PANIC("alloc memory failed!");
        }

        memset(sb_buf, 0, SECTOR_SIZE);
        ide_read(hd, cur_part->start_lba + 1, sb_buf, 1);

        memcpy(cur_part->sb, sb_buf, sizeof(struct super_block));

        // read block bitmap into memory
        cur_part->block_bitmap.bits = (uint8_t *)sys_malloc(sb_buf->block_bitmap_sects * SECTOR_SIZE);
        if (cur_part->block_bitmap.bits == NULL) {
            PANIC("alloc memory failed!");
        }
        cur_part->block_bitmap.btmp_bytes_len = sb_buf->block_bitmap_sects * SECTOR_SIZE;
        ide_read(hd, sb_buf->block_bitmap_lba, cur_part->block_bitmap.bits, sb_buf->block_bitmap_sects);

        // read inode bitmap into memory
        cur_part->inode_bitmap.bits = (uint8_t *)sys_malloc(sb_buf->inode_bitmap_sects * SECTOR_SIZE);
        if (cur_part->inode_bitmap.bits == NULL) {
            PANIC("alloc memory failed!");
        }
        cur_part->inode_bitmap.btmp_bytes_len = sb_buf->inode_bitmap_sects * SECTOR_SIZE;
        ide_read(hd, sb_buf->inode_bitmap_lba, cur_part->inode_bitmap.bits, sb_buf->inode_bitmap_sects);

        list_init(&cur_part->open_inodes);
        printk("mount %s done!\n", part->name);

        return true;
    }
    return false;
}

static void partition_format(struct partition *part) {
    uint32_t boot_sector_sects = 1;
    uint32_t super_block_sects = 1;
    uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILES_PER_PART, BITS_PER_SECTOR);
    uint32_t inode_table_sects = DIV_ROUND_UP(((sizeof(struct inode) * MAX_FILES_PER_PART)), SECTOR_SIZE);
    uint32_t used_sects = boot_sector_sects + super_block_sects + inode_bitmap_sects + inode_table_sects;
    uint32_t free_sects = part->sec_cnt - used_sects;

    // block bitmap sector
    uint32_t block_bitmap_sects;
    // 1 block = 1 sector. free_sects is total free blocks
    block_bitmap_sects = DIV_ROUND_UP(free_sects, BITS_PER_SECTOR);
    uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects;
    block_bitmap_sects = DIV_ROUND_UP(block_bitmap_bit_len, BITS_PER_SECTOR);

    // init super block
    struct super_block sb;
    sb.magic = 0x19590318;
    sb.sec_cnt = part->sec_cnt;
    sb.inode_cnt = MAX_FILES_PER_PART;
    sb.part_lba_base = part->start_lba;

    sb.block_bitmap_lba = sb.part_lba_base + 2; // 0: MBR, 1: super block
    sb.block_bitmap_sects = block_bitmap_sects;

    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
    sb.inode_bitmap_sects = inode_bitmap_sects;

    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
    sb.inode_table_sects = inode_table_sects;

    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;
    sb.root_inode_no = 0;
    sb.dir_entry_size = sizeof(struct dir_entry);

    printk("%s info:\n", part->name);
    printk("   magic:0x%x\n   part_lba_base:0x%x\n   all_sectors:0x%x\n   "
           "inode_cnt:0x%x\n   "
           "block_bitmap_lba:0x%x\n   block_bitmap_sectors:0x%x\n   "
           "inode_bitmap_lba:0x%x\n   inode_bitmap_sectors:0x%x\n   "
           "inode_table_lba:0x%x\n   inode_table_sectors:0x%x\n   "
           "data_start_lba:0x%x\n", 
            sb.magic, sb.part_lba_base, sb.sec_cnt, sb.inode_cnt, 
            sb.block_bitmap_lba, sb.block_bitmap_sects, 
            sb.inode_bitmap_lba, sb.inode_bitmap_sects, 
            sb.inode_table_lba, sb.inode_table_sects, 
            sb.data_start_lba);

    struct disk *hd = part->my_disk;
    // step 1: write super block to 1st sector in this partition
    ide_write(hd, part->start_lba + 1, &sb, 1);
    printk("    super_block_lba:0x%x\n", part->start_lba + 1);

    uint32_t buf_size = (sb.block_bitmap_sects >= sb.inode_bitmap_sects ? sb.block_bitmap_sects : sb.inode_bitmap_sects);
    buf_size = (buf_size >= sb.inode_table_sects ? buf_size : sb.inode_table_sects) * SECTOR_SIZE;
    uint8_t *buf = (uint8_t *)sys_malloc(buf_size);

    // step 2: initialize block bitmap and write it to sb.block_bitmap_lba
    buf[0] |= 0x01;
    uint32_t block_bitmap_last_byte = block_bitmap_bit_len / 8;
    uint8_t block_bitmap_last_bit = block_bitmap_bit_len % 8;
    uint32_t last_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE);

    memset(&buf[block_bitmap_last_byte], 0xff, last_size);

    uint8_t bit_idx = 0;
    while (bit_idx <= block_bitmap_last_bit) {
        buf[block_bitmap_last_byte] &= !(1 << bit_idx++);
    }
    ide_write(hd, sb.block_bitmap_lba, buf, sb.block_bitmap_sects);

    // step 3: initialize inode bitmap and write it to sb.block_bitmap_lba
    memset(buf, 0, buf_size);
    buf[0] |= 0x1;
    
    ide_write(hd, sb.inode_bitmap_lba, buf, sb.inode_bitmap_sects);

    // step 4: initialize inode table and write it to sb.inode_table_lba

    // initialize 0th inode entry (root inode)
    memset(buf, 0, buf_size);
    struct inode *i = (struct inode *)buf;
    i->i_size = sb.dir_entry_size * 2;
    i->i_no = 0;
    i->i_sectors[0] = sb.data_start_lba;
    ide_write(hd, sb.inode_table_lba, buf, sb.inode_table_sects);

    // step 5: initialize root dir and write to sb.data_start_lba
    memset(buf, 0, buf_size);
    struct dir_entry *p_de = (struct dir_entry *)buf;

    memcpy(p_de->filename, ".", 1);
    p_de->i_no = 0;
    p_de->f_type = FT_DIRECTORY;
    p_de++;

    memcpy(p_de->filename, "..", 2);
    p_de->i_no = 0;
    p_de->f_type = FT_DIRECTORY;

    ide_write(hd, sb.data_start_lba, buf, 1);

    printk("    root_dir_lba:0x%x\n", sb.data_start_lba);
    printk("%s format done\n", part->name);
    sys_free(buf);
}

void filesys_init(void) {
    uint8_t channel_no = 0, dev_no, part_idx = 0;

    struct super_block *sb_buf = (struct super_block *)sys_malloc(SECTOR_SIZE);

    if (sb_buf == NULL) {
        PANIC("alloc memory failed!");
    }
    printk("searching filesystem......\n");
    while (channel_no < channel_cnt) {
        dev_no = 0;
        while (dev_no < 2) {
            if (dev_no == 0) { // skip hd60M.image
                dev_no++;
                continue;
            }
            struct disk *hd = &channels[channel_no].devices[dev_no];
            struct partition *part = hd->prim_parts;
            while (part_idx < 12) {
                if (part_idx == 4) {
                    part = hd->logic_parts;
                }

                if (part->sec_cnt != 0) {
                    memset(sb_buf, 0, SECTOR_SIZE);

                    ide_read(hd, part->start_lba + 1, sb_buf, 1);

                    if (sb_buf->magic == 0x19590318) {
                        printk("%s has filesystem\n", part->name);
                    } else {
                        printk("formatting %s`s partition %s......\n", hd->name, part->name);
                        partition_format(part);
                    }
                }
                part_idx++;
                part++;
            }
            dev_no++;
        }
        channel_no++;
    }
    sys_free(sb_buf);

    char default_part[8] = "sdb1";
    list_traversal(&partition_list, mount_partition, (int)default_part);
}