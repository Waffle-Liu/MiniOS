#include "ps.h"
#include <driver/ps2.h>
#include <driver/sd.h>
#include <driver/vga.h>
#include <zjunix/bootmm.h>
#include <zjunix/buddy.h>
#include <zjunix/fs/fat.h>
<<<<<<< HEAD
#include <zjunix/fs/vfs.h>
#include <zjunix/fs/tree.h>
#include <zjunix/slab.h>
#include <zjunix/vm.h>
#include <zjunix/time.h>
#include <zjunix/utils.h>
#include <zjunix/log.h>
#include <zjunix/pc.h>
#include <driver/vga.h>
#include "../kernel/fs/fat/fat.h"
=======
#include <zjunix/vfs/vfs.h>
#include <zjunix/slab.h>
#include <zjunix/time.h>
#include <zjunix/utils.h>
#include <driver/vga.h>
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
#include "../usr/ls.h"
#include "exec.h"
#include "myvi.h"

<<<<<<< HEAD

char ps_buffer[64];
int ps_buffer_index;
int ps_buffer_base;
int ps_buffer_ins;
=======
char ps_buffer[64];
int ps_buffer_index;
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab

struct lock_t lk;

extern struct dentry *pwd_dentry;

void test_proc() {
    unsigned int timestamp;
    unsigned int currTime;
    unsigned int data;
    asm volatile("mfc0 %0, $9, 6\n\t" : "=r"(timestamp));
    data = timestamp & 0xff;
    while (1) {
        asm volatile("mfc0 %0, $9, 6\n\t" : "=r"(currTime));
        if (currTime - timestamp > 100000000) {
            timestamp += 100000000;
            *((unsigned int *)0xbfc09018) = data;
        }
    }
}
<<<<<<< HEAD
static int strlen(char *s){
    int i=0;
    while(s[i++]);
    return i-1;
}

void fs_ld(u8 *path) 
{
    FILE cat_file;
    int nowpc;
    u32 i,inst,opcode,rest;

    /* Open */
    u8 tmp[128];
    fs_fullpath(path,tmp,128);
    if (0 != fs_open(&cat_file, tmp)) {
        log(LOG_FAIL, "File %s open failed", path);
        return ;
    }
    /* Read */
    u32 file_size = get_entry_filesize(cat_file.entry.data);
    /* apply for the memory space */
    u8 *buf = (u8 *)kmalloc( (file_size + 1));
    /* read the content of file by using buffer */
    fs_read(&cat_file, buf, file_size);
    asm volatile(
        "jal next_label\n\t"
        "next_label:\n\t"
        :"=$ra"(nowpc)
        :
        :"31"
    );
    int delta=nowpc-(int)(void*)buf;
    for(i=0;i<file_size;i+=4){
        inst=*((u32 *)(buf+i));
        opcode=inst>>26;
        if(opcode!=0x02&&opcode!=0x03)
            continue;
        rest=inst<<6>>6;
        if(rest+delta>=(1<<26))
            goto fail;
        rest+=delta;
        inst=(opcode<<26)+rest;
    }
    kernel_printf("F2\n", buf);
    asm volatile(
        "addi $8,%0,0\n\t"
        "la $ra,ret_label\n\t"
        "jr $8\n\t"
        "ret_label:\n\t"
        :
        :"r"((int)buf)
        :"8","31"
    );
    kernel_printf("F2\n", buf);
    buf[file_size] = 0;
    /* output the content of file */
    kernel_printf("run finished\n", buf);
exit:
    /* close the file */
    fs_close(&cat_file);
    kfree(buf);
    return ;
fail:
    kernel_printf("fail to relocate\n", buf);
}
=======

>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
void testMem() {
    int i;
    int total = 100;
    //int sizeArr[200];
    int *addrArr[200];
	int size = 100;
    //for (i=0; i<total; i++) sizeArr[i] = 100;
    
    // for (i=10; i<100; i++) sizeArr[i] = 2<<12;
    // for (i=100; i<total; i++) sizeArr[i] = 4<<12;
    for (i=0; i<total; i++) addrArr[i] = kmalloc(size);
    kernel_printf("Allocate %d blocks sized %d byte\n", total, size);
    // bootmap_info("bootmm");
    buddy_info();
    // kernel_getkey();
    
    for (i=0; i<total; i++) kfree(addrArr[i]);
    kernel_printf("Deallocate\n");
    // bootmap_info("bootmm");
    buddy_info();
}

void testMem2() {
//     static int i;
//     static int total = 10;
//     static int sizeArr[200];
//     static int *addrArr[200];
//     for (i=0; i<total; i++) sizeArr[i] = 4096;
    
//     // for (i=10; i<100; i++) sizeArr[i] = 2<<12;
//     // for (i=100; i<total; i++) sizeArr[i] = 4<<12;
//     for (i=0; i<total; i++) addrArr[i] = kmalloc(sizeArr[i]);
//     kernel_printf("Allocate %d blocks sized %d byte\n", 20, 4096);
//     // bootmap_info("bootmm");
//     buddy_info();
//     // kernel_getkey();
    
//     for (i=0; i<total; i++) kfree(addrArr[i]);
//     kernel_printf("Deallocate\n");
//     // bootmap_info("bootmm");
//     buddy_info();
}

void test_sync() {
    int i, j;
    lockup(&lk);
    for (i=0; i<10; i++) {
        kernel_printf("%d%d%d\n", i, i, i);
        //kernel_getchar();
        for (j=0; j<100000; j++){

        }
    }
    unlock(&lk);
    while (1) {

    }
}

int sync_demo_create() {
    // init_lock(&lk);
    // int asid = pc_peek();
    // if (asid < 0) {
    //     kernel_puts("Failed to allocate pid.\n", 0xfff, 0);
    //     return 1;
    // }
    // unsigned int init_gp;
    // asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
    // pc_create(asid, test_sync, (unsigned int)kmalloc(4096), init_gp, "sync1");
    
    // asid = pc_peek();
    // if (asid < 0) {
    //     kernel_puts("Failed to allocate pid.\n", 0xfff, 0);
    //     return 1;
    // }
    // asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
    // pc_create(asid, test_sync, (unsigned int)kmalloc(4096), init_gp, "sync2");
    // return 0;
}



// int proc_demo_create() {
//     int asid = pc_peek();
//     if (asid < 0) {
//         kernel_puts("Failed to allocate pid.\n", 0xfff, 0);
//         return 1;
//     }
//     unsigned int init_gp;
//     asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
//     pc_create(asid, test_proc, (unsigned int)kmalloc(4096), init_gp, "test");
//     return 0;
// }
<<<<<<< HEAD
void ps() {
    kernel_printf("Press any key to enter shell.\n");
    kernel_getchar();
    char comm[8][16];
    int comm_cur=0,comm_sz=0;
    char c;
    int i;
    ps_buffer_index = 0;
    ps_buffer_base=0;
    ps_buffer_ins=0;
    for(i=0;i<64;i++)
        ps_buffer[i] = 0;
    kernel_clear_screen(31);
    // kernel_puts("PS>", VGA_WHITE, VGA_BLACK);
    // kernel_puts("PowerShell\n", VGA_WHITE, VGA_BLACK);
    ps_buffer_base+=kernel_puts("PS", VGA_GREEN, VGA_BLACK);
    ps_buffer_base+=kernel_puts(relative_path, VGA_YELLOW, VGA_BLACK);
    ps_buffer_base+=kernel_puts("\032", VGA_WHITE, VGA_BLACK);
    while (1) {
        c = kernel_getchar();
        if (c == '\n') {
            while(ps_buffer_ins<ps_buffer_index){
                cursor_col++;
                ps_buffer_ins++;
            }
            kernel_set_cursor();
=======

void ps() {
    kernel_printf("Press any key to enter shell.\n");
    kernel_getchar();
    char c;
    ps_buffer_index = 0;
    ps_buffer[0] = 0;
    kernel_clear_screen(31);
    // kernel_puts("PS>", VGA_WHITE, VGA_BLACK);
    // kernel_puts("PowerShell\n", VGA_WHITE, VGA_BLACK);
    kernel_puts("PS", VGA_GREEN, VGA_BLACK);
    kernel_puts(":", VGA_WHITE, VGA_BLACK);
    kernel_puts(pwd_dentry->d_name.name, VGA_YELLOW, VGA_BLACK);
    kernel_puts(">", VGA_WHITE, VGA_BLACK);
    while (1) {
        c = kernel_getchar();
        if (c == '\n') {
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
            ps_buffer[ps_buffer_index] = 0;
            if (kernel_strcmp(ps_buffer, "exit") == 0) {
                ps_buffer_index = 0;
                ps_buffer[0] = 0;
                kernel_printf("\nPowerShell exit.\n");
<<<<<<< HEAD
            }
            else{
                if(ps_buffer_index>0){
                    for(i=0;i<16;i++)
                        comm[comm_sz%8][i]=ps_buffer[i];
                    for(i=0;i<16;i++)
                        comm[(comm_sz+1)%8][i]='\0';
                    ++comm_sz;
                }
                
                comm_cur==comm_sz;
                parse_cmd();
            }
            ps_buffer_index = 0;
            for(i=0;i<64;i++)
                ps_buffer[i]='\0';
            // kernel_puts("PS>", VGA_WHITE, VGA_BLACK);
            // kernel_puts("PowerShell\n", VGA_WHITE, VGA_BLACK);
            ps_buffer_base=0;
            ps_buffer_ins=0;
            ps_buffer_base+=kernel_puts("PS", VGA_GREEN, VGA_BLACK);
            ps_buffer_base+=kernel_puts(relative_path, VGA_YELLOW, VGA_BLACK);
            ps_buffer_base+=kernel_puts("\032", VGA_WHITE, VGA_BLACK);
            for(i=0;i<64;i++)
                ps_buffer[i]='\0';
        }
        else if (c == 0x08) {//backspace
            if (ps_buffer_ins) {
                for(i=ps_buffer_ins;i<ps_buffer_index;i++){
                    ps_buffer[i-1]=ps_buffer[i]; 
                    kernel_putchar_at(ps_buffer[i-1],0xfff,0,cursor_row,ps_buffer_base+i-1);
                }
                ps_buffer[--ps_buffer_index]='\0';
                kernel_putchar_at(' ',0xfff,0,cursor_row,ps_buffer_base+ps_buffer_index);
                ps_buffer_ins--;
                //kernel_putchar_at(' ', 0xfff, 0, cursor_row, cursor_col - 1);
                cursor_col--;
                kernel_set_cursor();
            }
        }
        else if (c==0x10){//delete
            if(ps_buffer_ins<ps_buffer_index){
                for(i=ps_buffer_ins;i<ps_buffer_index;i++){
                    ps_buffer[i]=ps_buffer[i+1]; 
                    kernel_putchar_at(ps_buffer[i],0xfff,0,cursor_row,ps_buffer_base+i);
                }
                ps_buffer[--ps_buffer_index]='\0';
                kernel_putchar_at(' ',0xfff,0,cursor_row,ps_buffer_base+ps_buffer_index);
            }
        }
        else if (c == 0x18) {//u arr
            comm_cur--;
            //kernel_printf("%d %d\n",comm_cur,comm_sz);
            if(comm_cur>=0&&comm_cur>comm_sz-64&&comm_cur<comm_sz){
                for(i=0;i<16;i++)
                    ps_buffer[i]=comm[comm_cur%8][i];
                ps_buffer[i]='\0';
                for(i=0;i<64&&ps_buffer[i]!='\0';i++);
                ps_buffer_index=i;
                ps_buffer_ins=i;
                for(i=0;i<ps_buffer_index;i++)
                    kernel_putchar_at(ps_buffer[i],0xfff,0,cursor_row,ps_buffer_base+i);
                cursor_col=ps_buffer_base+i;
                kernel_set_cursor();
                while(i<64)
                    kernel_putchar_at(' ',0xfff,0,cursor_row,ps_buffer_base+i++);
            }
            else
                comm_cur++;
        }
        else if (c == 0x19) {//d arr
            comm_cur++;
            //kernel_printf("%d %d\n",comm_cur,comm_sz);
            if(comm_cur<=comm_sz){
                for(i=0;i<16;i++)
                    ps_buffer[i]=comm[comm_cur%8][i];
                ps_buffer[i]='\0';
                for(i=0;i<64&&ps_buffer[i]!='\0';i++);
                ps_buffer_index=i;
                ps_buffer_ins=i;
                for(i=0;i<ps_buffer_index;i++)
                    kernel_putchar_at(ps_buffer[i],0xfff,0,cursor_row,ps_buffer_base+i);
                cursor_col=ps_buffer_base+i;
                kernel_set_cursor();
                while(i<64)
                    kernel_putchar_at(' ',0xfff,0,cursor_row,ps_buffer_base+i++);
            }
            else
                comm_cur--;
        }
        else if (c == 0x1a) {//r arr
            if(ps_buffer_ins<ps_buffer_index){
                ps_buffer_ins++;
                cursor_col++;
                kernel_set_cursor();
            }
        }
        else if (c == 0x1b) {//l arr
            if(ps_buffer_ins){
                ps_buffer_ins--;
                cursor_col--;
                kernel_set_cursor();
            }
        }
        else {
            if (ps_buffer_index < 63) {
                for(i=ps_buffer_index;i>ps_buffer_ins;i--){
                    ps_buffer[i]=ps_buffer[i-1];
                    kernel_putchar_at(ps_buffer[i],0xfff,0,cursor_row,ps_buffer_base+i);
                }
                kernel_putchar_at(c,0xfff,0,cursor_row,ps_buffer_base+ps_buffer_ins);
                ps_buffer_index++;
                ps_buffer[ps_buffer_ins++] = c;
                cursor_col++;
                kernel_set_cursor();
=======
            } else
                parse_cmd();
            ps_buffer_index = 0;
            // kernel_puts("PS>", VGA_WHITE, VGA_BLACK);
            // kernel_puts("PowerShell\n", VGA_WHITE, VGA_BLACK);
            kernel_puts("PS", VGA_GREEN, VGA_BLACK);
            kernel_puts(":", VGA_WHITE, VGA_BLACK);
            kernel_puts(pwd_dentry->d_name.name, VGA_YELLOW, VGA_BLACK);
            kernel_puts(">", VGA_WHITE, VGA_BLACK);
        } else if (c == 0x08) {
            if (ps_buffer_index) {
                ps_buffer_index--;
                kernel_putchar_at(' ', 0xfff, 0, cursor_row, cursor_col - 1);
                cursor_col--;
                kernel_set_cursor();
            }
        } else {
            if (ps_buffer_index < 63) {
                ps_buffer[ps_buffer_index++] = c;
                kernel_putchar(c, 0xfff, 0);
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
            }
        }
    }
}

void parse_cmd() {
    unsigned int result = 0;
    char dir[32];
    char c;
    kernel_putchar('\n', 0, 0);
    char sd_buffer[8192];
<<<<<<< HEAD
    int i = 0,j,v;
    int len; /* The length of string */
    char *param; /* The parameter of instruction */
    char *src; /* The source of cp and mv instruction */
    char *dest; /* The destination of cp and mv instruction */
    char *parent;
    struct filetree * fd;
    struct filetree * p;
    for(i=1;i<63;i++)
        if(ps_buffer[i]==' '&&ps_buffer[i-1]==' ')
            for(j=i+1;j<63;j++)
                ps_buffer[j-1]=ps_buffer[j];
=======
    int i = 0;
    char *param;
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    for (i = 0; i < 63; i++) {
        if (ps_buffer[i] == ' ') {
            ps_buffer[i] = 0;
            break;
        }
    }
    if (i == 63){
        ps_buffer[62] = 0;
        param = ps_buffer + 62;
    }
    else
        param = ps_buffer + i + 1;
    if (ps_buffer[0] == 0) {
        return;
    } else if (kernel_strcmp(ps_buffer, "clear") == 0) {
        kernel_clear_screen(31);
    } else if (kernel_strcmp(ps_buffer, "echo") == 0) {
        kernel_printf("%s\n", param);
    } else if (kernel_strcmp(ps_buffer, "gettime") == 0) {
        char buf[10];
        get_time(buf, sizeof(buf));
        kernel_printf("%s\n", buf);
    } else if (kernel_strcmp(ps_buffer, "sdwi") == 0) {
        for (i = 0; i < 512; i++)
            sd_buffer[i] = i;
        sd_write_block(sd_buffer, 23128, 1);
<<<<<<< HEAD
        kernel_puts("sdwi 23128\n", 0xfff, 0);
    } else if (kernel_strcmp(ps_buffer, "sdr") == 0) {
        for (i = 0; i < 512; i++)
            sd_buffer[i] = 0;
        v=0;
        i=0;
        while('0'<=param[i]&&param[i]<='9')
            v=(v<<3)+(v<<1)+param[i++]-'0';
        i = sd_read_block(sd_buffer, v, 1);
        //i = sd_read_block(sd_buffer, 23128, 1);

        kernel_printf("read_result: %x\n", i);
        for (i = 0; i < 512; i++) {
            kernel_printf("%x%x ", sd_buffer[i]>>4,sd_buffer[i]&0xf);
            if(i&&(i&0xf)==0)
                kernel_printf("\n");
=======
        kernel_puts("sdwi\n", 0xfff, 0);
    } else if (kernel_strcmp(ps_buffer, "sdr") == 0) {
        for (i = 0; i < 512; i++)
            sd_buffer[i] = 0;

        i = sd_read_block(sd_buffer, 23128, 1);

        kernel_printf("read_result: %d\n", i);
        for (i = 0; i < 512; i++) {
            kernel_printf("%d ", sd_buffer[i]);
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
        }
        kernel_putchar('\n', 0xfff, 0);
    } else if (kernel_strcmp(ps_buffer, "sdwz") == 0) {
        for (i = 0; i < 512; i++) {
            sd_buffer[i] = 0;
        }
        sd_write_block(sd_buffer, 23128, 1);
        kernel_puts("sdwz\n", 0xfff, 0);
<<<<<<< HEAD
    } else if (kernel_strcmp(ps_buffer, "bootmmif") == 0) {
        bootmap_info("bootmm");
    } else if (kernel_strcmp(ps_buffer, "buddyif") == 0) {
        buddy_info();
    } else if (kernel_strcmp(ps_buffer, "slabif") == 0) {
        slab_info();
    } else if (kernel_strcmp(ps_buffer, "mmpif") == 0) {
        mmp_info();
=======
    } else if (kernel_strcmp(ps_buffer, "mminfo") == 0) {
        bootmap_info("bootmm");
        buddy_info();
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    } else if (kernel_strcmp(ps_buffer, "mmtest") == 0) {
        kernel_printf("kmalloc : %x, size = 1KB\n", kmalloc(1024));
    } else if (kernel_strcmp(ps_buffer, "ps") == 0) {
    //    result = print_proc();
        result = 0;
        disable_interrupts();
  //      print_task();
        print_sched();
        // print_exited();
        // print_wait();
        enable_interrupts();
        kernel_printf("ps return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "kill") == 0) {
        int pid = param[0] - '0';
        kernel_printf("Killing process %d\n", pid);
        result = pc_kill(pid);
        kernel_printf("kill return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "time") == 0) {
        unsigned int init_gp;
        asm volatile("la %0, _gp\n\t" : "=r"(init_gp));
//       pc_create(2, system_time_proc, (unsigned int)kmalloc(4096), init_gp, "time");
    }
    // else if (kernel_strcmp(ps_buffer, "proc") == 0) {
    //     result = proc_demo_create();
    //     kernel_printf("proc return with %d\n", result);
    // } 
    else if (kernel_strcmp(ps_buffer, "vi") == 0) {
        result = myvi(param);
        kernel_printf("vi return with %d\n", result);
    }
    else if (kernel_strcmp(ps_buffer, "mt") == 0) {
        testMem();
        kernel_printf("Memory test return with 0\n");
    }
    else if (kernel_strcmp(ps_buffer, "mt2") == 0) {
        testMem2();
        kernel_printf("Memory test2 return with 0\n");
    }
    
    else if (kernel_strcmp(ps_buffer, "execk") == 0) {
        //debug
        kernel_printf("Enter execk\n");
        //debug
        result = exec_from_kernel(1, (void*)param, 0, 0);
        kernel_printf("execk return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "execk2") == 0) {
        result = exec_from_kernel(1, (void*)param, 1, 0);
        kernel_printf(ps_buffer, "execk2 return with %d\n");
    } else if (kernel_strcmp(ps_buffer, "vm") == 0) {
        struct mm_struct* mm = mm_create();
        kernel_printf("Create succeed. %x\n", mm);
        mm_delete(mm);        
    } else if (kernel_strcmp(ps_buffer, "execk3") == 0) {
        result = exec_from_kernel(1, (void*)param, 0, 1);
        kernel_printf(ps_buffer, "execk3 return with %d\n",result);
    } else if (kernel_strcmp(ps_buffer, "ey") == 0) {
        result = exec_from_kernel(1, "/seg.bin", 0, 1);
        kernel_printf(ps_buffer, "execk3 return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "es") == 0) {
        result = exec_from_kernel(1, "/syscall.bin", 0, 1);
        kernel_printf(ps_buffer, "execk3 return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "tlb") == 0) {
        // result = tlb_test();
        // kernel_printf(ps_buffer, "tlb_test return with %d\n", result);
    } else if (kernel_strcmp(ps_buffer, "sync") == 0) {
        result = sync_demo_create();
        kernel_printf("proc return with %d\n", result);
    }
<<<<<<< HEAD
    
    else if (kernel_strcmp(ps_buffer, "send") == 0) {
		kernel_printf("sending message\n");
		result = pc_send((void*)param);
		kernel_printf("sending return with %d\n", result);
	}
	else if (kernel_strcmp(ps_buffer, "receive") == 0) {
		int pid = param[0] - '0';
		kernel_printf("Receiving message process %d\n", pid);
		result = pc_receive(pid);
		kernel_printf("receiving return with %d\n", result);
	}
	else if (kernel_strcmp(ps_buffer, "sleep") == 0) {
		int pid = param[0] - '0';
		kernel_printf("Sleep process %d\n", pid);
		result = pc_sleep(pid);
		kernel_printf("sleep return with %d\n", result);
	}
	else if (kernel_strcmp(ps_buffer, "wake") == 0) {
		int pid = param[0] - '0';
		kernel_printf("Wake process %d\n", pid);
		result = pc_wake(pid);
		kernel_printf("wake return with %d\n", result);
	}
    else if (kernel_strcmp(ps_buffer, "shift") == 0) {
		int pid = param[0] - '0';
		kernel_printf("Shift process %d\n", pid);
		result = pc_shift(pid);
		kernel_printf("shift return with %d\n", result);
	}

    else if (kernel_strcmp(ps_buffer, "cat") == 0) {
        result = vfsfile->fat32_file->cat(param);
        kernel_printf("  cat return with %d\n", result);
    }
    else if(kernel_strcmp(ps_buffer, "ldbin") == 0){
        fs_ld(param);
    }
    else if (kernel_strcmp(ps_buffer, "ls") == 0) {
        result = ls(param);
        kernel_printf("  ls return with %d\n", result);
    }
    else if (kernel_strcmp(ps_buffer, "cd") == 0) {
        cd(param);
    }
    else if(kernel_strcmp(ps_buffer, "rename") == 0){
        for(i = 0; i < 63; i++) {
            if(param[i] == ' ') {
                param[i] = 0;
                break;
            }
        }
        /* cut the source and destination */
        if (i == 63) {
            src = param;
        }
        else {
            /* get the string of src and the string of dest */
            src = param;
            dest = param + i + 1;
        }
        result = vfsfile->fat32_file->rename(src,dest);
        kernel_printf("  rename return with %d\n", result);
    }
    else if (kernel_strcmp(ps_buffer, "rm") == 0) {
        /* remove a file which exists under current path */
        result = vfsfile->fat32_file->rm(param);
        if(result == 0)
        {
            /* delete the node on the tree */
            deleteNode(param);
        }
        kernel_printf("  rm return with %d\n", result);
    }
    else if (kernel_strcmp(ps_buffer, "rmdir") == 0) {
        /* add rmdir instruction */
        result = vfsfile->fat32_file->rmdir(param);
        if(result == 0)
        {
            deleteNode(param);
        }
        kernel_printf("  rmdir return with %d\n", result);
    }
    else if (kernel_strcmp(ps_buffer, "mv") == 0) {
        /* add mv instruction */
        for (i = 0; i < 63; i++) {
            if(param[i] == ' ') {
                param[i] = 0;
                break;
            }
        }
        /* cut the source and destination */
        if (i == 63) {
            src = param;
        }
        else {
            src = param;
            dest = param + i + 1;
        }
        /* mv the file from source to destination */
        result = fs_mv(src, dest);
        /* if move instruction is right then we change the node of file tree */
        if(result == 0) {
            len = strlen(dest);
            p = init_treenode(dest);
            for(i = len - 1; i >=0; i--) {
                if(dest[i] == '/') {
                    dest[i] = 0;
                    break;
                }
            }
            if(i == 0) {
                parent = "/";
            }
            else {
                parent = dest;
            }
            if(parent[0] == 0) {
                parent[0] = '/';
                parent[1] = 0;
            }
            fd = findNode(parent);
            becomeChild(fd,p);
            deleteNode(src);
        }
        kernel_printf("  mv return with %d\n", result);
    }
    else if (kernel_strcmp(ps_buffer, "cp") == 0) {
        /* add cp instruction */
        for(i = 0; i < 63; i++) {
            if(param[i] == ' ') {
                param[i] = 0;
                break;
            }
        }
        /* cut the source and destination */
        if (i == 63) {
            src = param;
        }
        else {
            /* get the string of src and the string of dest */
            src = param;
            dest = param + i + 1;
        }
        /* cp the file from source to destination */
        result = fs_cp(src, dest);
        /* modify the file tree */
        if(result == 0) {
            len = strlen(dest);
            p = init_treenode(dest);
            for(i = len - 1; i >=0; i--) {
                if(dest[i] == '/') {
                    dest[i] = 0;
                    break;
                }
            }
            if(i == 0) {
                parent = "/";
            }
            else {
                parent = dest;
            }
            if(parent[0] == 0) {
                parent[0] = '/';
                parent[1] = 0;
            }
            fd = findNode(parent);
            becomeChild(fd,p);
        }
        kernel_printf("  cp return with %d\n", result);
    }
    else if(kernel_strcmp(ps_buffer, "mkdir") == 0){
        /* make a new directory */
        result = vfsfile->fat32_file->mkdir(param);
        // add in tree
        if(result == 0)
        {
            len = strlen(param);
            p = init_treenode(param);
            for(i = len - 1; i >= 0; i--)
            {
                if(param[i] == '/')
                {
                    param[i] = 0;
                    break;
                }
            }
            if(i == 0)
            {
                parent = "/";
            }
            else
            {
                parent = param;
            }
            fd = findNode(parent);
            becomeChild(fd,p);
        }
        kernel_printf("  mkdir return with %d\n", result);
    }/*
    else if (kernel_strcmp(ps_buffer, "mmp") == 0) {
        init_mmp();
    }*/
    else if (kernel_strcmp(ps_buffer, "mal") == 0) {
        v=0;
        i=0;
        while('0'<=param[i]&&param[i]<='9')
            v=(v<<3)+(v<<1)+param[i++]-'0';
        unsigned int res = (unsigned int)mmp_malloc(v);
        kernel_printf("mmp malloc return with %x\n", res);
    }
    else if (kernel_strcmp(ps_buffer, "vmpro") == 0) {
        vmprog(0, 0);
        kernel_printf("tlb test");
=======

    else if (kernel_strcmp(ps_buffer, "cat") == 0) {
        result = vfs_cat(param);
    }
    else if (kernel_strcmp(ps_buffer, "rm") == 0) {
        result = vfs_rm(param);
    }
    else if (kernel_strcmp(ps_buffer, "ls") == 0) {
        result = vfs_ls(param);
    }
    else if (kernel_strcmp(ps_buffer, "cd") == 0) {
        result = vfs_cd(param);
>>>>>>> f4e0b061d017001174f96bd5938c7dee3d0569ab
    }

    else {
        kernel_puts(ps_buffer, 0xfff, 0);
        kernel_puts(": command not found\n", 0xfff, 0);
    }
}
