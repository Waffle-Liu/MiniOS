#ifndef _ZJUNIX_BUDDY_H
#define _ZJUNIX_BUDDY_H

#include <zjunix/list.h>
#include <zjunix/lock.h>

#define BUDDY_RESERVED (1 << 31)
#define BUDDY_ALLOCED (1 << 30)
#define BUDDY_SLAB (1 << 29)
#define BUDDY_FREE 0

#define UINT unsigned int
#define UCHAR unsigned char

struct page
{
	UINT flag;       // the declaration of the usage of this page
    UINT reference;  // not used for now
    struct list_head list;   // double-way list
    void *virtual;           // slab cache address
    UINT bplevel;    // the order level of the page
    UINT slabp;      /* if the page is used by slab system,
                              * then slabp represents the base-addr of free space
                              */
};

#define PAGE_SHIFT 12
/*
 * order means the size of the set of pages, e.g. order = 1 -> 2^1
 * pages(consequent) are free In current system, we allow the max order to be
 * 4(2^4 consequent free pages)
 */
#define MAX_BUDDY_ORDER 4

struct freelist {
    UINT nr_free;               // Number of free blocks
    struct list_head free_head;         // Linked list
};

struct buddy_sys {
    UINT buddy_start_pfn, buddy_end_pfn;        // Physical frame number of buddy
    struct page *start_page;                            // Address of page array
    struct lock_t lock;                                 // Exclusive clock
    struct freelist freelist[MAX_BUDDY_ORDER + 1];      // Freelists for each order
};

#define _is_same_bpgroup(page, bage) (((*(page)).bplevel == (*(bage)).bplevel))
#define _is_same_bplevel(page, lval) ((*(page)).bplevel == (lval))
#define set_bplevel(page, lval) ((*(page)).bplevel = (lval))

#define set_flag(page, val) ((*(page)).flag = (val))
#define clean_flag(page, val) ((*(page)).flag = 0)
#define has_flag(page, val) ((*(page)).flag & val)

#define set_ref(page, val) ((*(page)).reference = (val))
#define inc_ref(page, val) ((*(page)).reference += (val))
#define dec_ref(page, val) ((*(page)).reference -= (val))

extern struct page *pages;
extern struct buddy_sys buddy;

// 内部函数
extern void __free_pages(struct page *page, unsigned int order);
extern struct page *__alloc_pages(unsigned int order);


// 外部接口
extern void free_pages(void *addr, unsigned int order);
extern void *alloc_pages(unsigned int nrPage);
extern void init_buddy();
extern void buddy_info();