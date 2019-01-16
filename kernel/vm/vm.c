#include <zjunix/vm.h>
#include <zjunix/slab.h>
#include <zjunix/utils.h>
#include <zjunix/pc.h>
#include <zjunix/rbtree.h>
#include <driver/vga.h>
#include <arch.h>

static unsigned int mmpsize[MAX_POOL_SIZE] = {16, 64, 256, 1024, 2048};

struct mm_pool mmp;

/************mm_struct************/
struct mm_struct *mm_create()
{
    struct mm_struct* mm;

    mm = kmalloc(sizeof(*mm));
#ifdef VMA_DEBUG
    kernel_printf("mm_create: %x\n", mm);
#endif //VMA_DEBUG
    if (mm) {
        kernel_printf("mm_create: %x\n", mm);
        kernel_memset(mm, 0, sizeof(*mm));
        mm->pgd = kmalloc(PAGE_SIZE); 
        if (mm->pgd) {
            kernel_memset(mm->pgd, 0, PAGE_SIZE);
            return mm;
        }
#ifdef VMA_DEBUG
        kernel_printf("mm_create fail\n");
#endif //VMA_DEBUG
        kfree(mm);
    }
    return 0;
}

void mm_delete(struct mm_struct* mm)
{
#ifdef VMA_DEBUG
    kernel_printf("mm_delete: pgd %x\n", mm->pgd);
#endif //VMA_DEBUG
    pgd_delete(mm->pgd);
    exit_mmap(mm);
#ifdef VMA_DEBUG
    kernel_printf("exit_mmap done\n");
#endif //VMA_DEBUG
    kfree(mm);
}

void pgd_delete(pgd_t* pgd)
{
    int i,j;
    unsigned int pde, pte;
    unsigned int* pde_ptr;

    for (i = 0; i < 1024; i++) {
        pde = pgd[i];
        pde &= PAGE_MASK;
       
        if (pde == 0)  //不存在二级页表
            continue;
#ifdef VMA_DEBUG
        kernel_printf("Delete pde: %x\n", pde);
#endif //VMA_DEBUG 
        pde_ptr = (unsigned int*)pde;
        for (j = 0; j < 1024; j++) {
            pte = pde_ptr[j];
            pte &= PAGE_MASK;
            if (pte != 0) {
#ifdef VMA_DEBUG
                kernel_printf("Delete pte: %x\n", pte);
#endif //VMA_DEBUG 
                kfree((void*)pte);
            }
        }
        kfree((void*)pde_ptr);
    }  
    kfree(pgd);
#ifdef VMA_DEBUG
    kernel_printf("pgd_delete done\n");
#endif //VMA_DEBUG  
    return ;
}



/*************VMA*************/
// Find the first vma with ending address greater than addr
// struct vm_area_struct* find_vma(struct mm_struct* mm, unsigned long addr)
// {
//     struct vm_area_struct* vma = 0;

//     if(mm)
//     {
//         vma = mm->mmap_cache;
//         if(vma && vma->vm_end>addr && vma->vm_start<=addr)
//             return vma;
//         vma = mm->mmap;
//         while(vma) 
//         {
//             if (vma->vm_end > addr) 
//             {
//                 mm->mmap_cache = vma;
//                 break;
//             }
//             vma = vma->vm_next;
//         }
//     }
//     return vma;
// }

/* Look up the first VMA which satisfies  addr < vm_end,  NULL if none. */
struct vm_area_struct *find_vma(struct mm_struct *mm, unsigned long addr)
{
    struct rb_node *rb_node;
    struct vm_area_struct *vma;
    
    if (mm)
    {
        rb_node = mm->mm_rb.rb_node;
        vma = mm->mmap_cache;
        if(vma && vma->vm_end>addr && vma->vm_start<=addr)
            return vma;

        while (rb_node) 
        {
            struct vm_area_struct *tmp;

            tmp = rb_entry(rb_node, struct vm_area_struct, vm_rb);

            if (tmp->vm_end > addr) 
            {
                vma = tmp;
                if (tmp->vm_start <= addr)
                    break;
                rb_node = rb_node->rb_left;
            } 
            else
                rb_node = rb_node->rb_right;
        }
    }
    return vma;
}


/*
 * Same as find_vma, but also return a pointer to the previous VMA in *pprev.
 */
struct vm_area_struct *find_vma_prev(struct mm_struct *mm, unsigned long addr, struct vm_area_struct **pprev)
{
    struct vm_area_struct *vma;

    vma = find_vma(mm, addr);
    if (vma) 
    {
        *pprev = vma->vm_prev;
    } 
    else 
    {
        struct rb_node *rb_node = mm->mm_rb.rb_node;
        *pprev = 0;
        while (rb_node) 
        {
            *pprev = rb_entry(rb_node, struct vm_area_struct, vm_rb);
            rb_node = rb_node->rb_right;
        }
    }
    return vma;
}

// Find the first vma overlapped with start_addr~end_addr
// struct vm_area_struct *find_vma_intersection(struct mm_struct* mm, unsigned long start_addr, unsigned long end_addr)
// {
//     if(start_addr > end_addr)
//         return 0;
//     struct vm_area_struct* vma = find_vma(mm, start_addr);
//     return vma;
// }

// Return the first vma with ending address greater than addr, recording the previous vma
// struct vm_area_struct* find_vma_and_prev(struct mm_struct* mm, unsigned long addr, struct vm_area_struct** prev)
// {
//     struct vm_area_struct* vma = 0;
//     *prev = 0;
//     if(mm) 
//     {
//         vma = mm->mmap;
//         while(vma) 
//         {
//             if(vma->vm_end > addr)
//             {
//                 mm->mmap_cache = vma;
//                 break;
//             }
//             *prev = vma;
//             vma = vma->vm_next;
//         }
//     }
//     return vma;
// }

// Get unmapped area starting after addr        目前省略了file, pgoff
unsigned long get_unmapped_area(unsigned long addr, unsigned long len, unsigned long flags)
{
    struct vm_area_struct* vma;
    struct mm_struct* mm = current_task->mm;         //全局变量，当前线程对应的task_struct

    addr = UPPER_ALLIGN(addr, PAGE_SIZE);   // Allign to page size
    if(addr+len > KERNEL_ENTRY) return -1;
    if(addr) 
    {
        for(vma=find_vma(mm, addr); ; vma=vma->vm_next) 
        {
            if (addr+len > KERNEL_ENTRY) return -1;
            if (!vma || addr+len<=vma->vm_start) 
            {
                return addr;
            }
            addr = vma->vm_end;
        }
    }

    return 0;
}

// Find vma preceding addr, assisting insertion
struct vm_area_struct* find_vma_prepare(struct mm_struct* mm, unsigned long addr)
{
    struct vm_area_struct* vma = mm->mmap;
    struct vm_area_struct* prev = 0;
    while (vma) {
        if(addr < vma->vm_start) break;
        prev = vma;
        vma = vma->vm_next;
    }
    return prev;
}

// Insert vma to the linked list
// void insert_vm_area_struct(struct mm_struct* mm, struct vm_area_struct* area)
// {
//     struct vm_area_struct* vma = find_vma_prepare(mm, area->vm_start);
// #ifdef VMA_DEBUG
//     kernel_printf("Insert: %x  %x", vma, area->vm_start);
// #endif //VMA_DEBUG
//     if (!vma) {
// #ifdef VMA_DEBUG
//         kernel_printf("mmap init\n");
// #endif //VMA_DEBUG
//         area->vm_next = 0;
//         mm->mmap = area;
//     }
//     else {
//         area->vm_next = vma->vm_next;
//         vma->vm_next = area;
//     }
//     mm->map_count ++;
// }

// Insert vma into rbtree
int insert_vm_struct(struct mm_struct *mm, struct vm_area_struct *vma)
{
    struct vm_area_struct *prev;
    struct rb_node **rb_link, *rb_parent;

    if (find_vma_links(mm, vma->vm_start, vma->vm_end, &prev, &rb_link, &rb_parent))
        return -12;

    vma_link(mm, vma, prev, rb_link, rb_parent);
    return 0;
}

int find_vma_links(struct mm_struct *mm, unsigned long addr, unsigned long end, struct vm_area_struct **pprev, struct rb_node ***rb_link, struct rb_node **rb_parent)
{
    struct rb_node **__rb_link, *__rb_parent, *rb_prev;

    __rb_link = &mm->mm_rb.rb_node;
    rb_prev = __rb_parent = 0;

    while (*__rb_link) {
        struct vm_area_struct *vma_tmp;

        __rb_parent = *__rb_link;
        vma_tmp = rb_entry(__rb_parent, struct vm_area_struct, vm_rb);

        if (vma_tmp->vm_end > addr) {
            /* Fail if an existing vma overlaps the area */
            if (vma_tmp->vm_start < end)
                return -12;
            __rb_link = &__rb_parent->rb_left;
        } else {
            rb_prev = __rb_parent;
            __rb_link = &__rb_parent->rb_right;
        }
    }

    *pprev = 0;
    if (rb_prev)
        *pprev = rb_entry(rb_prev, struct vm_area_struct, vm_rb);
    *rb_link = __rb_link;
    *rb_parent = __rb_parent;
    return 0;
}

void vma_link(struct mm_struct *mm, struct vm_area_struct *vma, struct vm_area_struct *prev, struct rb_node **rb_link, struct rb_node *rb_parent)
{
    vma_link_list(mm, vma, prev, rb_parent);
    vma_link_rb(mm, vma, rb_link, rb_parent);
}

void vma_link_list(struct mm_struct *mm, struct vm_area_struct *vma, struct vm_area_struct *prev, struct rb_node *rb_parent)
{
    struct vm_area_struct *next;

    vma->vm_prev = prev;
    if (prev) {
        next = prev->vm_next;
        prev->vm_next = vma;
    } else {
        mm->mmap = vma;
        if (rb_parent)
            next = rb_entry(rb_parent, struct vm_area_struct, vm_rb);
        else
            next = 0;
    }
    vma->vm_next = next;
    if (next)
        next->vm_prev = vma;
}

void vma_link_rb(struct mm_struct *mm, struct vm_area_struct *vma, struct rb_node **rb_link, struct rb_node *rb_parent)
{
    rb_link_node(&vma->vm_rb, rb_parent, rb_link);
    vma_rb_insert(vma, &mm->mm_rb);
}

void vma_rb_insert(struct vm_area_struct *vma, struct rb_root *root)
{
    rb_insert_color(&vma->vm_rb, root);
}

// Mapping a region
unsigned long do_map(unsigned long addr, unsigned long len, unsigned long flags)
{
    struct mm_struct* mm = current_task->mm;
    struct vm_area_struct *vma, *prev;
    if (!len) return addr;
    addr = get_unmapped_area(addr, len, flags);
    vma = kmalloc(sizeof(struct vm_area_struct));
    if (!vma) return -1;

    vma->vm_mm = mm;
    vma->vm_start = addr;
    vma->vm_end = UPPER_ALLIGN(addr+len, PAGE_SIZE);
#ifdef VMA_DEBUG
    kernel_printf("Do map: %x  %x\n", vma->vm_start, vma->vm_end);
#endif //VMA_DEBUG
    insert_vm_struct(mm, vma);
    return addr;
}


int do_unmap(unsigned long addr, unsigned long len)
{
    struct mm_struct* mm = current_task->mm;
    struct vm_area_struct *vma, *prev;
    if (addr>KERNEL_ENTRY || len+addr>KERNEL_ENTRY) return -1;  // Bad addr
    vma = find_vma_prev(mm, addr, &prev);
    if (!vma || vma->vm_start>=addr+len) return 0;      // It has not been mapped
#ifdef VMA_DEBUG
    kernel_printf("do_unmap. %x %x\n", addr, vma->vm_start);
#endif //VMA_DEBUG
    // VMA Length match
    if (vma->vm_end >= addr+len) {
#ifdef VMA_DEBUG
        kernel_printf("Length match\n");
#endif //VMA_DEBUG
        if (!prev) {
            mm->mmap = vma->vm_next;
        }
        else {
            prev->vm_next = vma->vm_next;
        }
        kfree(vma);
        mm->map_count --;
#ifdef VMA_DEBUG
        kernel_printf("Unmap done. %d vma(s) left\n", mm->map_count);
#endif //VMA_DEBUG
        return 0;
    }
    
    // Length mismatch
#ifdef VMA_DEBUG
    kernel_printf("Length match");
#endif //VMA_DEBUG
    return 1;
}


// Free all the vmas
void exit_mmap(struct mm_struct* mm)
{
    struct vm_area_struct* vmap = mm->mmap;
    mm->mmap = mm->mmap_cache = 0;
    while (vmap) {
        struct vm_area_struct* next = vmap->vm_next;
        kfree(vmap);
        mm->map_count--;
        vmap = next;
    }
    if (mm->map_count) {
        kernel_printf("exit mmap bug! %d vma left", mm->map_count);
        //BUG
        while(1)    ;
    }
}


int is_in_vma(unsigned long addr)
{
    struct mm_struct* mm = current_task->mm;
    struct vm_area_struct *vma = find_vma(mm, addr);
    if (!vma || vma->vm_start>addr) return 0;
    else return 1;
}

// initialize the memory pool
unsigned int init_mmp()
{
    unsigned int i, j;
    struct mm_pool_node *cur_node = 0;


    for(i=0; i<MAX_POOL_SIZE; i++)
    {
        mmp.mmp_list[i].block_size = mmpsize[i];
        mmp.mmp_list[i].used_cnt = 0;
        mmp.mmp_list[i].total_cnt = MAX_POOLLIST_SIZE;
        mmp.mmp_list[i].free_header = (struct mm_pool_node*)kmalloc(sizeof(struct mm_pool_node));
        mmp.mmp_list[i].used_header = (struct mm_pool_node*)kmalloc(sizeof(struct mm_pool_node));
        for(j=0; j<MAX_POOLLIST_SIZE; j++)
        {
            struct mm_pool_node *new_node = 0;
            new_node = (struct mm_pool_node*)kmalloc(sizeof(*new_node));
            new_node->index = i;
            new_node->data = 0;
            // new_node->data = (unsigned char*)kmalloc(mmpsize[i]);
            // kernel_memset(new_node->data, 0, mmpsize[i]);
            if(new_node == 0 || new_node->data == 0)
                return MP_MALLOC_FAIL;
            new_node->next = 0; 
            if(j == 0)
            {
                mmp.mmp_list[i].free_header = new_node;
                cur_node = mmp.mmp_list[i].free_header;
            }
            else
            {
                cur_node->next = new_node;
                cur_node = cur_node->next;
            }
        }
    } 
    return MP_OK;
}


unsigned char *mmp_malloc(unsigned int size)
{
    unsigned int i;
    struct mm_pool_node *node = 0;
    // large memory
    if(size > mmp.mmp_list[MAX_POOLLIST_SIZE-1].block_size)
    {
#ifdef VMA_DEBUG
        kernel_printf("malloc size[%d] so big ,need new from stack!\n" , size);
#endif
a:      node = (struct mm_pool_node*)kmalloc(sizeof(struct mm_pool_node));
        node->index = LARGE_MM_LIST;
        node->data = (unsigned char*)kmalloc(size);
        kernel_memset(node->data, 0, size);
        if(node == 0 || node->data == 0)
            return 0;
        node->next = 0;
        return node->data;
    }

    for(i=0; i<MAX_POOL_SIZE; i++)
    {
        if(size>mmp.mmp_list[i].block_size)
            continue;
        if(mmp.mmp_list[i].total_cnt - mmp.mmp_list[i].used_cnt == 0)
        {
#ifdef VMA_DEBUG
            kernel_printf("warning!!!!  size[%d]pool use up!!!! \n" , mmp.mmp_list[i].block_size);
#endif
            continue;
        }
        node = mmp.mmp_list[i].free_header;
        node->data = (unsigned char*)kmalloc(mmpsize[i]);
        kernel_memset(node->data, 0, mmpsize[i]);
        mmp.mmp_list[i].free_header = mmp.mmp_list[i].free_header->next;
        mmp.mmp_list[i].used_cnt++;
        node->next = mmp.mmp_list[i].used_header;
        mmp.mmp_list[i].used_header = node;
        return node->data;
    }
#ifdef VMA_DEBUG
    kernel_printf("warning! all of pool used up! \n");
#endif
    goto a; 
}


void mmp_free(struct mm_pool_node *free_node)
{
    if(free_node->index == LARGE_MM_LIST)
    {
        kfree(free_node->data);
        kfree(free_node);
        free_node = 0;
        return;
    }
    struct mm_pool_node *cur_node = mmp.mmp_list[free_node->index].used_header;
    struct mm_pool_node *pre_node = 0;
    while(cur_node!=0)
    {
        if(cur_node!=free_node)
        {
            pre_node = cur_node;
            cur_node = cur_node->next;
            continue;
        }
        if(pre_node==0)
            mmp.mmp_list[free_node->index].used_header = mmp.mmp_list[free_node->index].used_header->next;
        else 
            pre_node->next = cur_node->next;

        mmp.mmp_list[free_node->index].used_cnt--;
        cur_node->next = mmp.mmp_list[free_node->index].free_header;
        mmp.mmp_list[free_node->index].free_header = cur_node;
        break;
    } 
    return;
}


void del_pool()
{
    unsigned int i;
    struct mm_pool_node *cur_node = 0;
    struct mm_pool_node *del_node = 0;
    if (mmp.mmp_list!=0)
        return ;
    for(i=0; i<MAX_POOL_SIZE; i++)
    {
        cur_node = mmp.mmp_list[i].used_header;
        while(cur_node!=0)
        {
            del_node = cur_node;
            cur_node = cur_node->next;
            kfree(del_node->data);
            kfree(del_node);
        }
        cur_node = mmp.mmp_list[i].free_header;
        while(cur_node!=0)
        {
            del_node = cur_node;
            cur_node = cur_node->next;
            kfree(del_node->data);
            kfree(del_node);
        }    
    }
    kfree(&mmp);
    return ;
}

void mmp_info(){
    unsigned int i;
    kernel_printf("memory pool: \n");
    for(i=0; i<MAX_POOL_SIZE; i++)
    {
        kernel_printf("\tno:%x size:%x freelist:%x \n", i, mmp.mmp_list[i].block_size, mmp.mmp_list[i].free_header);
    }
}
