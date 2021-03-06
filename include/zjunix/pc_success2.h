#ifndef _ZJUNIX_PC_H
#define _ZJUNIX_PC_H

#include <zjunix/list.h>
#include <zjunix/pid.h>
#include <zjunix/vm.h>
#include <zjunix/fs/fat.h>

#define  KERNEL_STACK_SIZE  4096
#define  TASK_NAME_LEN   32

//进程状态
#define  S_UNINIT    0
#define  S_READY    1
#define  S_RUNNING  2
#define  S_WAITING  3
#define  S_TERMINAL 4

//进程时间片信息
#define  CNT_LV0    0xffffffff
#define  CNT_BASE   200

//调度链表信息
#define  SCHED_LV0   0  //forward process
#define  SCHED_LV1   1
#define  SCHED_LV2   2
#define  SCHED_LV3   3
#define  SCHED_LV4   4
#define  SCHED_LV5   5
#define  SCHED_LV6   6
#define  SCHED_LV7   7
#define  SCHED_LV8   8
#define  SCHED_LV_RANGE  8   
#define  SCHED_LV_MAX  SCHED_LV_RANGE
#define  SCHED_LV_MIN  1

//链表结构
struct pclist
{
	struct task_struct* pred;
	struct task_struct* succ;
	struct task_struct* what;
};

//寄存器信息结构，主要用于进程调度时的进程切换
struct regs_context 
{
    unsigned int epc;
    unsigned int at;
    unsigned int v0, v1;
    unsigned int a0, a1, a2, a3;
    unsigned int t0, t1, t2, t3, t4, t5, t6, t7;
    unsigned int s0, s1, s2, s3, s4, s5, s6, s7;
    unsigned int t8, t9;
    unsigned int hi, lo;
    unsigned int gp, sp, fp, ra;
};


struct task_struct {
    pid_t pid;              //进程pid号
    unsigned char name[TASK_NAME_LEN];  //进程名
    pid_t parent;           //父进程pid号
    int ASID;               //进程地址空间id号
    int state;              //进程状态

    unsigned int time_cnt;  //进程所拥有的时间片
    unsigned int sched_lv;  //用于多级反馈队列调度算法,当前为第几级队列   
	unsigned int tasktype; //进程类型

	unsigned int start_time;  //记录进程开始的时间
	unsigned int levelup; //记录进程每一次大调整的时候提升的优先度
	unsigned int runtime; //记录该进程跑多少个进程单位时间

    struct regs_context context;    //进程寄存器信息
    struct mm_struct *mm;           //进程地址空间结构指针
    FILE *task_files;               //进程打开文件指针

	struct pclist node;  //用于进程链表

    struct pclist wakeuplist;         //用于存储进程结束后具体会唤醒哪些进程
    struct pclist waitlist;          //用于存储进程需要等待哪一些进程完成
};

union task_union {
    struct task_struct task;        //进程控制块
    unsigned char kernel_stack[KERNEL_STACK_SIZE];  //进程的内核栈
};


typedef struct regs_context context;

extern struct task_struct *tasks[PID_NUM+1]; 
extern struct pclist schedhead;
extern struct pclist schedtail;
extern struct pclist sched_backtail[SCHED_LV_RANGE + 1];
extern struct pclist sched_backhead[SCHED_LV_RANGE + 1];
extern struct task_struct *current_task;

void task_files_delete(struct task_struct* task);
void remove_sched(struct task_struct *task);
void remove_task(struct task_struct *task);
void remove_exited(struct task_struct *task);


void add_sched(struct task_struct *task);
void add_sched_back(struct task_struct *task, int index);
void add_task(struct task_struct *task);
void add_exited(struct task_struct *task);

struct task_struct* find_in_tasks(pid_t pid);
struct task_struct* find_in_sched(pid_t pid);

int task_create(char *task_name, void(*entry)(unsigned int argc, void *args), 
                unsigned int argc, void *args, pid_t *retpid, int is_user);
int runprog(unsigned int argc, void *args);
static void copy_context(context* src, context* dest);
int exec_from_kernel(unsigned int argc, void *args, int is_wait, int is_user);

void init_pc();
void init_pc_list();
void pc_schedule(unsigned int status, unsigned int cause, context* pt_context);
int pc_kill(pid_t pid);

void waitpid(pid_t pid);
void add_wait(struct task_struct *task);
void wakeup_parent();
void print_wait();

/**************tlb****************/
void activate_mm(struct task_struct* task);
int runuserprog(char* progname);
int vmprog(unsigned int argc, void* args);
extern void enter_new_pc(unsigned int entry, unsigned int stack);

void pc_exit(int state);
struct task_struct* find_next_task();
struct task_struct* find_in_sched_back();
extern void switch_ex(struct regs_context* regs);
extern void switch_wa(struct regs_context* des, struct regs_context* src);
void clear_exited();

void print_exited();
void print_sched();     
void print_task();
void print_struct_task(struct task_struct* task);

#endif  // !_ZJUNIX_PC_H
