#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H

#include "stdint.h"
#include "bitmap.h"
#include "stdint.h"
#include "print.h"
#include "string.h"
#include "global.h"
#include "debug.h"
#include "list.h"

enum pool_flags {
    PF_KERNEL = 1,
    PF_USER = 2
};

#define	 PG_P_1	  1	// 页表项或页目录项存在属性位
#define	 PG_P_0	  0	// 页表项或页目录项存在属性位
#define	 PG_RW_R  0	// R/W 属性位值, 读/执行
#define	 PG_RW_W  2	// R/W 属性位值, 读/写/执行
#define	 PG_US_S  0	// U/S 属性位值, 系统级
#define	 PG_US_U  4	// U/S 属性位值, 用户级

struct virtual_addr {
    struct bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

struct mem_block {
    struct list_elem free_elem;
};

struct mem_block_desc {
    uint32_t block_size;
    uint32_t blocks_per_arena;
    struct list free_list;
};

#define DESC_CNT 7 // total mem block desc number

extern struct pool kernel_pool, user_pool;
void mem_init(void);
void *get_kernel_pages(uint32_t pg_cnt);
void *malloc_page(enum pool_flags pf, uint32_t pg_cnt);
void malloc_init(void);
uint32_t *pte_ptr(uint32_t vaddr);
uint32_t *pde_ptr(uint32_t vaddr);
uint32_t addr_v2p(uint32_t vaddr);
void *get_a_page(enum pool_flags pf, uint32_t vaddr);
void *get_user_pages(uint32_t pg_cnt);
void block_desc_init(struct mem_block_desc* desc_array);
void *sys_malloc(uint32_t size);
void mfree_page(enum pool_flags pf, void *_vaddr, uint32_t pg_cnt);
void pfree(uint32_t pg_phy_addr);
void sys_free(void *ptr);

#endif