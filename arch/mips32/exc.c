#include "exc.h"

#include <driver/vga.h>
#include <zjunix/pc.h>
#include <zjunix/utils.h>
#include <zjunix/slab.h>
#include <zjunix/page.h>
#include <driver/ps2.h>

#pragma GCC push_options
#pragma GCC optimize("O0")

exc_fn exceptions[32];
int count = 0;
int count_2 = 0;

void tlb_refill(unsigned int bad_addr) {
    pgd_t* pgd;
    unsigned int pde_index, pte_index, pte_index_near;
    unsigned int pde, pte, pte_near;
    unsigned int pte_phy, pte_near_phy;
    unsigned int* pde_ptr, pte_ptr;
    unsigned int entry_lo0, entry_lo1;
    unsigned int entry_hi;


    
   #ifdef  TLB_DEBUG
    unsigned int entry_hi_test;
        asm volatile( 
            "mfc0  $t0, $10\n\t"
            "move  %0, $t0\n\t"
            : "=r"(entry_hi_test)
        );
    
    kernel_printf("tlb_refill: bad_addr = %x    entry_hi = %x \n", bad_addr, entry_hi_test);
    kernel_printf("%x  %d\n", current_task, current_task->pid);
   #endif
    if (current_task->mm == 0) {
        kernel_printf("tlb_refill: mm is null!!!  %d\n", current_task->pid);
        goto error_0;
    }

    pgd = current_task->mm->pgd;
    if (pgd == 0) {
        kernel_printf("tlb_refill: pgd == NULL\n");
        goto error_0;
    }

    bad_addr &= PAGE_MASK;
    
    //搜索bad_addr是否在vma中,如果不在任何vma中，报错
    //....... 
    //To be done
    
    pde_index = bad_addr >> PGD_SHIFT;
    pde = pgd[pde_index];
    pde &= PAGE_MASK;
    if (pde == 0) { //二级页表不存在
        pde = (unsigned int) kmalloc(PAGE_SIZE);

        #ifdef TLB_DEBUG
            kernel_printf("second page table not exist\n");
        #endif

        if (pde == 0) {
            kernel_printf("tlb_refill: alloc second page table failed!\n");
            goto error_0;
        }

        kernel_memset((void*)pde, 0, PAGE_SIZE);
        pgd[pde_index] = pde;
        pgd[pde_index] &= PAGE_MASK;
        pgd[pde_index] |= 0x0f; //attr
    }

#ifdef  VMA_DEBUG
    kernel_printf("tlb refill: %x\n", pde);
#endif
    pde_ptr = (unsigned int*)pde;
    pte_index = (bad_addr >> PAGE_SHIFT) & INDEX_MASK;
    pte = pde_ptr[pte_index];
    pte &= PAGE_MASK;
    if (pte == 0) {
        #ifdef TLB_DEBUG
            kernel_printf("page not exist\n");
        #endif

        pte = (unsigned int)kmalloc(PAGE_SIZE);  //要考虑物理地址？？？

        if (pte == 0) {
            kernel_printf("tlb_refill: alloc page failed!\n");
            goto error_0;
        }

        kernel_memset((void*)pte, 0, PAGE_SIZE);
        pde_ptr[pte_index] = pte;
        pde_ptr[pte_index] &= PAGE_MASK;
        pde_ptr[pte_index] |= 0x0f;
    }

    pte_index_near = pte_index ^ 0x01;
    pte_near = pde_ptr[pte_index_near];
    pte_near &= PAGE_MASK;


#ifdef  VMA_DEBUG
    kernel_printf("pte: %x pte_index: %x  pte_near_index: %x\n", pte, pte_index, pte_index_near);
#endif

    if (pte_near == 0) {  //附近项 为空
        #ifdef TLB_DEBUG
            kernel_printf("page near not exist\n");
        #endif

        pte_near = (unsigned int)kmalloc(PAGE_SIZE);

        if (pte_near == 0) {
            kernel_printf("tlb_refill: alloc pte_near failed!\n");
            goto error_0;
        }

        kernel_memset((void*)pte_near, 0, PAGE_SIZE);
        pde_ptr[pte_index_near] = pte_near;
        pde_ptr[pte_index_near] &= PAGE_MASK;
        pde_ptr[pte_index_near] |= 0x0f;
    } 

    //换成物理地址
    pte_phy = pte-0x80000000;
    pte_near_phy = pte_near-0x80000000;
    #ifdef TLB_DEBUG
    kernel_printf("pte: %x  %x\n", pte_phy, pte_near_phy);
    #endif
    //
    if (pte_index & 0x01 == 0) { //偶
        entry_lo0 = (pte_phy >> 12) << 6;
        entry_lo1 = (pte_near_phy >> 12) << 6;
    }
    else {
        entry_lo0 = (pte_near_phy >> 12) << 6;
        entry_lo1 = (pte_near >> 12) << 6;
    }
    entry_lo0 |= (3 << 3);   //cached ??
    entry_lo1 |= (3 << 3);   //cached ??
    entry_lo0 |= 0x06;      //D = 1, V = 1, G = 0
    entry_lo1 |= 0x06;

    entry_hi = (bad_addr & PAGE_MASK) & (~(1 << PAGE_SHIFT));
    entry_hi |= current_task->ASID;
    
    #ifdef TLB_DEBUG
        kernel_printf("pid: %d\n", current_task->pid);
        kernel_printf("tlb_refill: entry_hi: %x  entry_lo0: %x  entry_lo1: %x\n", entry_hi, entry_lo0, entry_lo1);
    #endif

    asm volatile (
        "move $t0, %0\n\t"
        "move $t1, %1\n\t"
        "move $t2, %2\n\t"
        "mtc0 $t0, $10\n\t"
        "mtc0 $zero, $5\n\t"
        "mtc0 $t1, $2\n\t"
        "mtc0 $t2, $3\n\t"
        "nop\n\t"
        "nop\n\t"
        "tlbwr\n\t"
        "nop\n\t"
        "nop\n\t"
        :
        : "r"(entry_hi),
          "r"(entry_lo0),
          "r"(entry_lo1)
    );


    kernel_printf("after refill\n");
    unsigned int* pgd_ = current_task->mm->pgd;
    unsigned int pde_, pte_;
    unsigned int* pde_ptr_;
    int i, j;
    count_2 ++;
    
    for (i = 0; i < 1024; i++) {
        pde_ = pgd_[i];
        pde_ &= PAGE_MASK;
       
        if (pde_ == 0)  //不存在二级页表
            continue;
        kernel_printf("pde: %x\n", pde_);
        pde_ptr_ = (unsigned int*)pde_;
        for (j = 0; j < 1024; j++) {
            pte_ = pde_ptr_[j];
            pte_ &= PAGE_MASK;
            if (pte_ != 0) {
                kernel_printf("\tpte: %x\n", pte_);
            }
        }
    }
    // if (count_2 == 4) {
    //     kernel_printf("")
    //     while(1)
    //         ;
    // }

    return;

error_0:
    while(1)
        ;
}

void do_exceptions(unsigned int status, unsigned int cause, context* pt_context, unsigned int bad_addr) {
    int index = cause >> 2;
    index &= 0x1f;
    
    #ifdef  TLB_DEBUG
    unsigned int count;
    #endif

    if (index == 2 || index == 3) {
        tlb_refill(bad_addr);
        #ifdef TLB_DEBUG
        kernel_printf("refill done\n");

        //count = 0x
       // kernel_getchar();
        #endif
        return ;
    }

    if (exceptions[index]) {
        exceptions[index](status, cause, pt_context);
    } else {
        struct task_struct* pcb;
        unsigned int badVaddr;
        asm volatile("mfc0 %0, $8\n\t" : "=r"(badVaddr));
        //modified by Ice
        pcb = current_task;
        kernel_printf("\nProcess %s exited due to exception cause=%x;\n", pcb->name, cause);
        kernel_printf("status=%x, EPC=%x, BadVaddr=%x\n", status, pcb->context.epc, badVaddr);
    //    pc_kill_syscall(status, cause, pt_context);
            //Done by Ice
        while (1)
            ;
    }
}

void register_exception_handler(int index, exc_fn fn) {
    index &= 31;
    exceptions[index] = fn;
}

void init_exception() {
    // status 0000 0000 0000 0000 0000 0000 0000 0000
    // cause 0000 0000 1000 0000 0000 0000 0000 0000
    asm volatile(
        "mtc0 $zero, $12\n\t"
        "li $t0, 0x800000\n\t"
        "mtc0 $t0, $13\n\t");
}

#pragma GCC pop_options