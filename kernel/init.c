#include <arch.h>
#include <driver/ps2.h>
#include <driver/vga.h>
#include <exc.h>
#include <intr.h>
#include <page.h>
#include <zjunix/bootmm.h>
#include <zjunix/buddy.h>
#include <zjunix/fs/fat.h>
<<<<<<< HEAD
//#include <zjunix/vfs/vfs.h>
#include <zjunix/fs/vfs.h>
#include <zjunix/fs/tree.h>
#include <zjunix/log.h>
#include <zjunix/pc.h>
#include <zjunix/slab.h>
#include <zjunix/vm.h>
=======
#include <zjunix/vfs/vfs.h>
#include <zjunix/log.h>
#include <zjunix/pc.h>
#include <zjunix/slab.h>
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
#include <zjunix/syscall.h>
#include <zjunix/time.h>
#include "../usr/ps.h"

// int init_done = 0;

void machine_info() {
    int row;
    int col;
<<<<<<< HEAD
    kernel_printf("%s\n", "Buguntu"/*"ZJUNIX V1.0"*/);
    row = cursor_row;
    col = cursor_col;
    cursor_row = 28;
    kernel_printf("%s", "Created by System Interest Group, Zhejiang University.");
    cursor_row = 29;
    cursor_col = 0;
    kernel_printf("%s", "Adapted by Rui Liu, Hongfu Liu and Jinglin Zhao.");
=======
    kernel_printf("\n%s\n", "412-UNIX V1.0");
    row = cursor_row;
    col = cursor_col;
    cursor_row = 29;
    kernel_printf("%s", "Created by Dorm 412 Block 32, Yuquan Campus, Zhejiang University.");
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    cursor_row = row;
    cursor_col = col;
    kernel_set_cursor();
}

#pragma GCC push_options
#pragma GCC optimize("O0")
void create_startup_process() {
    int res;

    res =  task_create("kernel_shell", (void*)ps, 0, 0, 0, 0);
    if (res != 0)
        kernel_printf("create startup process failed!\n");
    else
        kernel_printf("kernel shell created!\n");
<<<<<<< HEAD
    /* 
    unsigned int init_gp;
    asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
    //pc_create(2, system_time_proc, (unsigned int)kmalloc(4096) + 4096, init_gp, "time");
    task_create("time",(void*)system_time_proc,0,0x0,0,0);
    log(LOG_OK, "Timer init");
    */
}
#pragma GCC pop_options

void init_kernel() {/*
=======
}
#pragma GCC pop_options

void init_kernel() {
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    // init_done = 0;
    kernel_clear_screen(31);
    // Exception
    init_exception();
    // Page table
    init_pgtable();
    // Drivers
    init_vga();
    init_ps2();
    // Memory management
    log(LOG_START, "Memory Modules.");
    init_bootmm();
    log(LOG_OK, "Bootmem.");
    init_buddy();
    log(LOG_OK, "Buddy.");
    init_slab();
    log(LOG_OK, "Slab.");
<<<<<<< HEAD
    init_mmp();
    log(LOG_OK, "Memory Pool.");
=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    log(LOG_END, "Memory Modules.");
#ifndef VFS_DEBUG
    // File system
    log(LOG_START, "File System.");
    init_fs();
    log(LOG_END, "File System.");
#endif
<<<<<<< HEAD
    // Virtual File System
    log(LOG_START, "Virtual File System.");
    // new add vfs
    initial_vfs();
    log(LOG_OK, "Virtual File System.");
    log(LOG_END, "Virtual File System.");
    //log(LOG_START, "Virtual file System.");
    //if(!init_vfs())
    //    log(LOG_END, "Virtual file System.");
    //else
    //    log(LOG_FAIL, "Virtual file System.");
    
    // File Tree
    log(LOG_START, "File Tree.");
    // new add filetree
    init_filetree();
    log(LOG_OK, "File Tree.");
    log(LOG_END, "File Tree.");

=======
#ifdef VFS_DEBUG
    // Virtual file system
    log(LOG_START, "Virtual file System.");
    if(!init_vfs())
        log(LOG_END, "Virtual file System.");
    else
        log(LOG_FAIL, "Virtual file System.");
#endif
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    // System call
    log(LOG_START, "System Calls.");
    init_syscall();
    log(LOG_END, "System Calls.");

    // Process control
    log(LOG_START, "PID Module.");
    init_pid();
    log(LOG_END, "PID Module.");
<<<<<<< HEAD

=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    log(LOG_START, "Process Control Module.");
    init_pc();
    create_startup_process();
    log(LOG_END, "Process Control Module.");
    // Interrupts
    log(LOG_START, "Enable Interrupts.");
    init_interrupts();
    log(LOG_END, "Enable Interrupts.");
    // Init finished
    machine_info();
    // init_done = 1;
    *GPIO_SEG = 0x11223344;
    // Enter shell
    while (1)
<<<<<<< HEAD
        ;*/
        
    kernel_clear_screen(31);
    // Exception
    init_exception();
    // Page table
    init_pgtable();
    // Drivers
    init_vga();
    init_ps2();
    // Memory management
    log(LOG_START, "Memory Modules.");
    init_bootmm();
    log(LOG_OK, "Bootmem.");
    init_buddy();
    log(LOG_OK, "Buddy.");
    init_slab();
    log(LOG_OK, "Slab.");
    init_mmp();
    log(LOG_OK, "Memory Pool.");
    log(LOG_END, "Memory Modules.");
    // File system
    log(LOG_START, "File System.");
    init_fs();
    log(LOG_END, "File System.");
    // Virtual File System
    log(LOG_START, "Virtual File System.");
    // new add vfs
    initial_vfs();
    log(LOG_OK, "Virtual File System.");
    log(LOG_END, "Virtual File System.");
    // File Tree
    log(LOG_START, "File Tree.");
    // new add filetree
    init_filetree();
    log(LOG_OK, "File Tree.");
    log(LOG_END, "File Tree.");
    // System call
    log(LOG_START, "System Calls.");
    init_syscall();
    log(LOG_END, "System Calls.");
    
    // Process id
    log(LOG_START, "PID Module.");
    init_pid();
    log(LOG_END, "PID Module.");

    // Process control
    log(LOG_START, "Process Control Module.");
    init_pid();
    init_pc();
    create_startup_process();
    log(LOG_END, "Process Control Module.");
    // Interrupts
    log(LOG_START, "Enable Interrupts.");
    init_interrupts();
    log(LOG_END, "Enable Interrupts.");
    // Init finished
    machine_info();
    *GPIO_SEG = 0x11223344;
    // Enter shell
    while (1)
=======
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
        ;
}
