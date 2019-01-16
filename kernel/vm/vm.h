#ifndef  _ZJUNIX_VM_H
#define  _ZJUNIX_VM_H

#include <zjunix/page.h>
#include <zjunix/rbtree.h>

#define  USER_CODE_ENTRY   0x00100000
#define  USER_DATA_ENTRY   0x01000000
#define  USER_DATA_END
#define  USER_BRK_ENTRY     0x10000000
#define  USER_STACK_ENTRY   0x80000000
#define  USER_DEFAULT_ATTR     0x0f

#define  MAX_POOL_SIZE 5
#define  MAX_POOLLIST_SIZE 5
#define  LARGE_MM_LIST 255

#define  MP_OK 1
#define  MP_MALLOC_FAIL 0
// extern struct page;

static unsigned int mmpsize[MAX_POOL_SIZE] = {16, 64, 256, 1024, 2048};

struct mm_struct;

struct mm_struct {
    struct vm_area_struct *mmap;        // VMA list
    struct vm_area_struct *mmap_cache;  // Latest used VMA
    struct rb_root mm_rb;

    int map_count;                  // VMA count

    pgd_t *pgd;                     // Page table entry

    unsigned int start_code, end_code;
    unsigned int start_data, end_data;
    unsigned int start_brk, brk;
    unsigned int start_stack;
};

struct vm_area_struct {
    struct mm_struct *vm_mm;
    unsigned long vm_start, vm_end;
    struct vm_area_struct *vm_prev, *vm_next;

    struct rb_node vm_rb;
    //struct vm_operations_struct *vm_ops;
};

// struct vm_operations_struct
// {
//     void (*open)(struct vm_area_struct * area);
//     void (*close)(struct vm_area_struct * area);
//     void (*nopage)(int error_code,
//                struct vm_area_struct * area, unsigned long address);
//     void (*wppage)(struct vm_area_struct * area, unsigned long address);
//     int (*share)(struct vm_area_struct * from, struct vm_area_struct * to, unsigned long address);
//     int (*unmap)(struct vm_area_struct *area, unsigned long, size_t);        
// };

struct mm_pool_node
{   
    unsigned int index;
    unsigned char *data;
    struct mm_pool_node *next;
};

struct mm_pool_list
{
    unsigned int total_cnt;
    unsigned int used_cnt;
    unsigned int block_size;

    struct mm_pool_node *free_header;
    struct mm_pool_node *used_header;
};

struct mm_pool
{  
    struct mm_pool_list mmp_list[MAX_POOL_SIZE];
};

extern struct mm_pool mmp;

void mm_delete(struct mm_struct* mm);
struct mm_struct* mm_create();

extern void set_tlb_asid(unsigned int asid);


unsigned long do_map(unsigned long addr, unsigned long len, unsigned long flags);
int do_unmap(unsigned long addr, unsigned long len);
int is_in_vma(unsigned long addr);



/*************VMA*************/
struct vm_area_struct* find_vma(struct mm_struct* mm, unsigned long addr);
unsigned long get_unmapped_area(unsigned long addr, unsigned long len, unsigned long flags);
struct vm_area_struct* find_vma_intersection(struct mm_struct* mm, unsigned long start_addr, unsigned long end_addr);
struct vm_area_struct* find_vma_prev(struct mm_struct* mm, unsigned long addr, struct vm_area_struct** prev);
struct vm_area_struct* find_vma_prepare(struct mm_struct* mm, unsigned long addr);
int insert_vm_struct(struct mm_struct* mm, struct vm_area_struct* area);
int find_vma_links(struct mm_struct *mm, unsigned long addr, unsigned long end, struct vm_area_struct **pprev, struct rb_node ***rb_link, struct rb_node **rb_parent)
void vma_link(struct mm_struct *mm, struct vm_area_struct *vma, struct vm_area_struct *prev, struct rb_node **rb_link, struct rb_node *rb_parent);
void vma_link_list(struct mm_struct *mm, struct vm_area_struct *vma, struct vm_area_struct *prev, struct rb_node *rb_parent);
void vma_link_rb(struct mm_struct *mm, struct vm_area_struct *vma, struct rb_node **rb_link, struct rb_node *rb_parent);
void vma_rb_insert(struct vm_area_struct *vma, struct rb_root *root);

void exit_mmap(struct mm_struct* mm);
void pgd_delete(pgd_t* pgd);

extern unsigned int init_mmp();
extern struct mm_pool_node *mmp_malloc(unsigned int size);
extern void mmp_free(struct mm_pool_node *free_node);
extern void del_pool();

#endif
