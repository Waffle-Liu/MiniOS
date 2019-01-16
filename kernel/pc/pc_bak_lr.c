#include <zjunix/pc.h>
#include <arch.h>
#include <intr.h>
#include <zjunix/syscall.h>
#include <zjunix/utils.h>
#include <zjunix/log.h>
#include <zjunix/slab.h>
#include <driver/vga.h>
#include <driver/ps2.h>
#include <zjunix/vfs/vfs.h>
#include <page.h>

//等待进程链表，用于存放处于等待状态的进程
struct pclist waittail;
struct pclist waithead;
//结束进程链表，用于存放处于结束状态的进程，在进程调度函数pc_schedule中清空
struct pclist exitedtail;
struct pclist exitedhead;
//进程数组，存放所有进程
struct task_struct *tasks[PID_NUM+1];

//调度链表，在多级反馈队列上进行部分修改
//前台队列, idle,INIT,RR调度算法
struct pclist schedtail;
struct pclist schedhead; 
//多级反馈队列，除最后一级使用FIFO，其余均使用RR
struct pclist sched_backtail[SCHED_LV_RANGE + 1]; 
struct pclist sched_backhead[SCHED_LV_RANGE + 1];

unsigned int sched_time_cnt[SCHED_LV_RANGE + 1]; //多级队列所具有的时间片数量
struct task_struct *current_task = 0;            //当前进程全局指针
unsigned int timecnt = 0; //记录当前已经经过了多少时间片
//无用变量
extern int count_2;

//激活task所指向进程的地址空间
void activate_mm(struct task_struct* task) {
    // init_pgtable();
    //在cp0中设置进程的ASID， 用于TLB匹配
    set_tlb_asid(task->ASID);
}


//等待子进程
//停止当前进程的运行，将其放入等待队列。并且通过调度算法从调度链表中选取下一个进程进行运行
void waitpid(pid_t pid) {
    int i;
    struct pclist *pos;
    struct task_struct *next, *torun, *towait;
    int is_exited;
    pid_t pid_val;

    pid_val = pid;
    disable_interrupts();

	//检查要等待的进程是否存在
	if (tasks[pid_val] == 0x0) 
	{
		enable_interrupts();
		return;
	}

    //检查要等待的进程是否已经退出
    is_exited = 0;
	if (tasks[pid_val]->state == S_TERMINAL)
	{
		is_exited = 1;
	}
	
	//等待的进程已经退出
    if (is_exited != 0) {     
        enable_interrupts();
        return ;
    }

	//占据pid的不是当前进程的子进程，也即表明子进程已经退出，或不存在
    if (tasks[pid_val]->parent != current_task->pid) 
	{    
        enable_interrupts();
        return ;
    }

    enable_interrupts();

	//进入异常模式 => 中断关闭
    asm volatile (      
        "mfc0  $t0, $12\n\t"
        "ori   $t0, $t0, 0x02\n\t"
        "mtc0  $t0, $12\n\t"
        "nop\n\t"
        "nop\n\t"
    );

    #ifdef PC_DEBUG
        kernel_printf("waitpid: current_task->state: %d, pid = %d", current_task->state, pid_val);
    #endif

    //调用调度算法选取下一个进程
    torun = find_next_task();

    //激活地址空间
    activate_mm(torun);

    //将当前进程从调度链表中移除，放入等待链表
    remove_sched(current_task);
	//state = -pid 表示进程正在等待进程号为pid的进程
	current_task->state = -(int)pid_val;
    add_wait(current_task);
    
    //加载新的进程的上下文信息
    towait = current_task;
    current_task = torun;
    switch_wa(&(torun->context), &(towait->context));

	
    #ifdef  PC_DEBUG
        kernel_printf("I'm here again\n");
    #endif
}

//唤醒父进程，将父进程重新放入调度链表
//进程退出pc_exit()函数中调用
void wakeup_parent() {
    struct task_struct *next;
    struct pclist *pos;
    int is_found;

	next = tasks[current_task->parent];
  
	//检测父进程是否存在，若不存在说明错误。
	if (next == 0x0)
	{
		kernel_printf("the pid %d does not has a parent,error!\n", current_task->pid);
		return;
	}

	//查找等待队列，看当前进程的父进程是否在等待自己
	is_found = 0;
	if (next->state == -(int)current_task->pid)
	{
		is_found = 1;
	}

    //如果父进程在等待当前进程，则将父进程放入调度链表，并将父进程状态设为S_READY状态
    if (is_found == 1) 
	{
       remove_sched(next);
	   next->state = S_READY;
       if (next->sched_lv == SCHED_LV0) 
	   {
           add_sched(next);
       }
       else 
	   {
		   add_sched_back(next, next->sched_lv); 
       }
       
    }
}

//将进程放入等待链表
void add_wait(struct task_struct *task) 
{
	//如果等待链表为空
	if (waittail.what == 0x0)
	{
		waithead.what = task;
		waithead.pred = 0x0;
		waithead.succ = 0x0;
		waittail.what = task;
		waittail.pred = 0x0;
		waittail.succ = 0x0;
		task->node.pred = 0x0;
		task->node.succ = 0x0;
	}
	else //等待列表非空
	{
		struct pclist temp;
		temp.what = task;
		temp.pred = waittail.what;
		temp.succ = 0x0;
		task->node.pred = temp.pred;
		task->node.succ = 0x0;
		waittail.succ = task;
		waittail.what->node.succ = task;
		waittail = temp;		
	}
	task->node.what = task;
}

//将进程加入tasks数组
void add_task(struct task_struct *task) {
	tasks[task->pid] = task;
}

//将进程放入结束链表
void add_exited(struct task_struct *task) {
	//如果结束链表为空
	if (exitedtail.what == 0x0)
	{
		exitedhead.what = task;
		exitedhead.pred = 0x0;
		exitedhead.succ = 0x0;
		exitedtail.what = task;
		exitedtail.pred = 0x0;
		exitedtail.succ = 0x0;
		task->node.pred = 0x0;
		task->node.succ = 0x0;
	}
	else //结束列表非空
	{
		struct pclist temp;
		temp.what = task;
		temp.pred = exitedtail.what;
		temp.succ = 0x0;
		task->node.pred = temp.pred;
		task->node.succ = 0x0;
		exitedtail.succ = task;
		exitedtail.what->node.succ = task;
		exitedtail = temp;
	}
	task->node.what = task;
}



//将进程放入前台调度链表
void add_sched(struct task_struct *task) {
	//如果前台调度链表为空
	if (schedtail.what == 0x0)
	{
		schedhead.what = task;
		schedhead.pred = 0x0;
		schedhead.succ = 0x0;
		schedtail.what = task;
		schedtail.pred = 0x0;
		schedtail.succ = 0x0;
		task->node.pred = 0x0;
		task->node.succ = 0x0;
	}
	else //前台调度列表非空
	{
		struct pclist temp;
		temp.what = task;
		temp.pred = schedtail.what;
		temp.succ = 0x0;
		task->node.pred = temp.pred;
		task->node.succ = 0x0;
		schedtail.succ = task;
		schedtail.what->node.succ = task;
		schedtail = temp;
	}
	task->node.what = task;
}

//将进程放入后台调度链表，index表示是第几级链表
void add_sched_back(struct task_struct *task, int index) {
    if (index > SCHED_LV_RANGE || index < SCHED_LV_MIN) {
        kernel_printf("add_sched_back: index out of range!\n");
        return ;
    }
	//如果这一级调度链表为空
	if (sched_backtail[index].what == 0x0)
	{
		sched_backhead[index].what = task;
		sched_backhead[index].pred = 0x0;
		sched_backhead[index].succ = 0x0;
		sched_backtail[index].what = task;
		sched_backtail[index].pred = 0x0;
		sched_backtail[index].succ = 0x0;
		task->node.pred = 0x0;
		task->node.succ = 0x0;
	}
	else //这一级调度列表非空
	{
		struct pclist temp;
		temp.what = task;
		temp.pred = sched_backtail[index].what;
		temp.succ = 0x0;
		task->node.pred = temp.pred;
		task->node.succ = 0x0;
		sched_backtail[index].succ = task;
		sched_backtail[index].what->node.succ = task;
		sched_backtail[index] = temp;
	}
	task->node.what = task;
}

//从进程链表中移除进程
//在清理结束进程的clear_exited()中调用
void remove_task(struct task_struct *task) {
	tasks[task->pid] = 0x0;
}

//从结束链表中移除进程
//在清理结束链表clear_exited()中调用
void remove_exited(struct task_struct *task) {
	//如果进程状态不对，意味着系统发生了错误
	if (task->state != S_TERMINAL)
	{
		kernel_printf("the pid %d is not exited!\n", task->pid);
		return;
	}
	//如果该进程是这个链表的唯一一个节点
	if (task->node.pred == 0x0 && task->node.succ==0x0)
	{
		exitedhead.what = 0x0;
		exitedhead.succ = 0x0;
		exitedtail.what = 0x0;
		exitedtail.pred = 0x0;
	}
	else 
	//如果是链表头
	if (task->node.pred == 0x0)
	{
		exitedhead.what = task->node.succ;
		exitedhead.pred = 0x0;
		exitedhead.what->node.pred = 0x0;
		exitedhead.succ = exitedhead.what->node.succ;
	}
	//如果是链表尾
	else
	if (task->node.succ == 0x0)
	{
		exitedtail.what = task->node.pred;
		exitedtail.succ = 0x0;
		exitedtail.what->node.succ = 0x0;
		exitedtail.pred = exitedtail.what->node.pred;
	}
	//如果有左右节点
	else
	{
		struct task_struct *taskleft = task->node.pred;
		struct task_struct *taskright = task->node.succ;
		taskleft->node.succ = taskright;
		taskright->node.pred = taskleft;
	}
	task->node.succ = 0x0;
	task->node.pred = 0x0;
	task->node.what = 0x0;	
}

//从调度链表中移除进程
void remove_sched(struct task_struct *task) 
{
	//如果该进程是这个链表的唯一一个节点
	if (task->node.pred == 0x0 && task->node.succ == 0x0) 
	{
		if (task->state < 0 || task->state == S_SLEEPING)
		{
			waithead.what = 0x0;
			waithead.succ = 0x0;
			waittail.what = 0x0;
			waittail.pred = 0x0;
		}
		if (task->state == S_READY)
		{
			if (task->sched_lv == SCHED_LV0)
			{
				schedhead.what = 0x0;
				schedhead.succ = 0x0;
				schedtail.what = 0x0;
				schedtail.pred = 0x0;
			}
			else
			{
				int index = task->sched_lv;
				sched_backhead[index].what = 0x0;
				sched_backhead[index].succ = 0x0;
				sched_backtail[index].what = 0x0;
				sched_backtail[index].pred = 0x0;
			}
		}
	}
	else
		//如果该进程是链表头
		if (task->node.pred == 0x0)
		{
			if (task->state < 0 || task->state == S_SLEEPING)
			{
				waithead.what = task->node.succ;
				waithead.pred = 0x0;
				waithead.what->node.pred = 0x0;
				waithead.succ = waithead.what->node.succ;
			}
			if (task->state == S_READY)
			{
				if (task->sched_lv == SCHED_LV0)
				{
					schedhead.what = task->node.succ;
					schedhead.pred = 0x0;
					schedhead.what->node.pred = 0x0;
					schedhead.succ = schedhead.what->node.succ;
				}
				else
				{
					int index = task->sched_lv;
					sched_backhead[index].what = task->node.succ;
					sched_backhead[index].pred = 0x0;
					sched_backhead[index].what->node.pred = 0x0;
					sched_backhead[index].succ = sched_backhead[index].what->node.succ;
				}
			}
		}
		else
			//如果该进程是链表尾
			if (task->node.succ == 0x0)
			{
				if (task->state < 0 || task->state == S_SLEEPING)
				{
					waittail.what = task->node.pred;
					waittail.succ = 0x0;
					waittail.what->node.succ = 0x0;
					waittail.pred = waittail.what->node.pred;
				}
				if (task->state == S_READY )
				{
					if (task->sched_lv == SCHED_LV0)
					{
						schedtail.what = task->node.pred;
						schedtail.succ = 0x0;
						schedtail.what->node.succ = 0x0;
						schedtail.pred = schedtail.what->node.pred;
					}
					else
					{
						int index = task->sched_lv;
						sched_backtail[index].what = task->node.pred;
						sched_backtail[index].succ = 0x0;
						sched_backtail[index].what->node.succ = 0x0;
						sched_backtail[index].pred = sched_backtail[index].what->node.pred;
					}
				}
			}
			//该进程有左右节点
			else
			{
				struct task_struct *taskleft = task->node.pred;
				struct task_struct *taskright = task->node.succ;
				taskleft->node.succ = taskright;
				taskright->node.pred = taskleft;
			}
	task->node.succ = 0x0;
	task->node.pred = 0x0;
	task->node.what = 0x0;
}

//根据进程号在进程链表中查找进程
//如果找到，则返回进程的task_struct结构
//否则返回0
struct task_struct* find_in_tasks(pid_t pid) {
	if (tasks[pid] == 0x0)
	{
		return 0;
	}
	else
	{
		return tasks[pid];
	}
}

//根据进程号在调度链表中查找进程
//如果找到，则返回进程的task_struct结构
//否则返回0
struct task_struct* find_in_sched(pid_t pid) {
	//进程不存在
	if (tasks[pid] == 0x0)
	{
		return 0;
	}
	//进程不在调度链表中
	if (tasks[pid]->state != S_READY)
	{
		return 0;
	}
	return tasks[pid];
}

void task_files_delete(struct task_struct* task) {
    //可能有问题，不知道如何查看文件是否打开
    fs_close(task->task_files);
    kfree(&(task->task_files));
}

//将src所指向的寄存器信息复制到dest所指向的地址处
static void copy_context(context* src, context* dest) {
    dest->epc = src->epc;
    dest->at = src->at;
    dest->v0 = src->v0;
    dest->v1 = src->v1;
    dest->a0 = src->a0;
    dest->a1 = src->a1;
    dest->a2 = src->a2;
    dest->a3 = src->a3;
    dest->t0 = src->t0;
    dest->t1 = src->t1;
    dest->t2 = src->t2;
    dest->t3 = src->t3;
    dest->t4 = src->t4;
    dest->t5 = src->t5;
    dest->t6 = src->t6;
    dest->t7 = src->t7;
    dest->s0 = src->s0;
    dest->s1 = src->s1;
    dest->s2 = src->s2;
    dest->s3 = src->s3;
    dest->s4 = src->s4;
    dest->s5 = src->s5;
    dest->s6 = src->s6;
    dest->s7 = src->s7;
    dest->t8 = src->t8;
    dest->t9 = src->t9;
    dest->hi = src->hi;
    dest->lo = src->lo;
    dest->gp = src->gp;
    dest->sp = src->sp;
    dest->fp = src->fp;
    dest->ra = src->ra;
}

//用于初始化所有的与进程有关的全局链表
//在init_pc()中调用
void init_pc_list() {
    int i;
    //等待链表
	waithead.what = 0x0;
	waithead.pred = 0x0;
	waithead.succ = 0x0;
	waittail.what = 0x0;
	waittail.pred = 0x0;
	waittail.succ = 0x0;
	//tasks数组
	for (i = 0; i <= PID_NUM; i++)
		tasks[i] = 0x0;
	//前台调度链表
	schedhead.what = 0x0;
	schedhead.pred = 0x0;
	schedhead.succ = 0x0;
	schedtail.what = 0x0;
	schedtail.pred = 0x0;
	schedtail.succ = 0x0;
    //结束链表
	exitedhead.what = 0x0;
	exitedhead.pred = 0x0;
	exitedhead.succ = 0x0;
	exitedtail.what = 0x0;
	exitedtail.pred = 0x0;
	exitedtail.succ = 0x0;
    //后台多级调度链表
    for (i = 0; i <= SCHED_LV_RANGE; i++) {
		sched_backhead[i].what = 0x0;
		sched_backhead[i].pred = 0x0;
		sched_backhead[i].succ = 0x0;
		sched_backtail[i].what = 0x0;
		sched_backtail[i].pred = 0x0;
		sched_backtail[i].succ = 0x0;
        sched_time_cnt[i] = CNT_BASE * i;  //根据调度链表的级数(i)来赋予调度链表的用于调度的时间片
    }

}

//初始化进程管理信息，创建idle进程
//在init_kernel()中调用
void init_pc() {
    struct task_struct *idle;

    //初始化进程管理有关的全局链表
    init_pc_list();

    //创建idle进程，idle进程的task_struct结构位于内核代码部分(0-16MB)的最后一个页
    idle = (struct task_struct*)(kernel_sp - KERNEL_STACK_SIZE);

    kernel_strcpy(idle->name, "idle");
    idle->pid = IDLE_PID;
    idle->parent = IDLE_PID;
    idle->ASID = idle->pid;   //ASID:  地址空间id
    idle->state = S_UNINIT;
    
    //当前寄存器中的内容也就是idle进程的context内容
    idle->mm = 0;
    idle->task_files = 0;
    idle->time_cnt = CNT_LV0;
	idle->sched_lv = SCHED_LV0;

    
    add_task(idle);      //加入进程数组
	add_sched(idle);     //加入ready链表
    idle->state = S_READY;
    
    current_task = idle;

    //注册进程调度函数，时钟中断触发
    register_interrupt_handler(7, pc_schedule);
    //设置cp0中的compare和count寄存器
    //当compare == count时，产生时钟中断（7号）
    asm volatile(
        "li $v0, 1000000\n\t"
        "mtc0 $v0, $11\n\t"
        "mtc0 $zero, $9");

}

//用命令创建进程
//新建进程的入口函数是公共的
//返回0表示执行成功，否则表示执行失败
int exec_from_kernel(unsigned int argc, void *args, int is_wait, int is_user) {
    int res; 
    pid_t new_pid;
    int status;

    if (is_user)
        //用户进程，新创建进程的入口为vmprog函数
        res = task_create(args, (void*)vmprog, argc, args, &new_pid, 1);
    else
        //内核进程，新创建进程的入口为runprog函数
        res = task_create(args, (void*)runprog, argc, args, &new_pid, 0);

    if (res != 0) {
        kernel_printf("exec_from_kernel: task created failed!\n");
        return 1;
    }

    //是否等待子进程
    if (is_wait) {
        waitpid(new_pid);
        //如果当前进程等待子进程，则在其被唤醒之后将从这里开始执行  
    }
    
    return 0;
}



//内核线程统一入口
//args指向真正的内核线程执行函数
//此处为了测试进程创建，让其输出测试提示信息
int runprog(unsigned int argc, void *args) {
    int count1, count2;


    kernel_printf("\n********This is runprog!*********\n");
    kernel_printf("current_task: %d\n", current_task->pid);
    kernel_printf("**********runprog end************\n");

     //用于等待子进程测试进程loop的创建
    if (kernel_strcmp(args, "loop") == 0) {
        while (1)
            ;
    }


    //循环计数使得新创建进程每隔一定的计数次数输出提示信息，表明进程被调度
    count1 = 0;
    count2 = 0;
    while(1) {
        count1 ++;
        if (count2 == current_task->runtime)
            break;
        if (count1== 10000000) {
//            kernel_printf("********This is runprog!*********\n");
            kernel_printf("\ncurrent_task: %d \n", current_task->pid);
//            kernel_printf("**********runprog end************\n");
            count1 = 0;
            count2 ++;
        }
    }
    //进程将要结束的提示信息
    kernel_printf("\ncurrent_task: %d  End.......\n", current_task->pid);

    //进程退出
    pc_exit(0);
    //error, 永远不会运行到这里
    kernel_printf("Error: runprog, pc_exit\n");
    while(1)
        ;
    return 0;                                                                                                                                                 
}

//创建新的进程
//task_name: 进程名
//entry: 进程的入口函数
//args: 进程的参数信息
//retpid: 用于返回新创建进程的pid
//is_user: 是否是创建用户进程
//创建成功返回0, 否则返回1
int task_create(char *task_name, void(*entry)(unsigned int argc, void *args), 
                unsigned int argc, void *args, pid_t *retpid, int is_user)              {
                
    pid_t newpid;            
    union task_union *new;
    int res;
    struct task_struct *cur = current_task;
    unsigned int index, pos;
    unsigned int init_gp;

    //分配pid号
    if (pid_alloc(&newpid)) {
        kernel_printf("do fork: pid allocated failed!\n");
        goto err_0;
    }

    //创建task_union结构
    new = (union task_union*) kmalloc(sizeof(union task_union));
    if (new == 0) {
        kernel_printf("do fork: task union allocated failed!\n");
        goto err_1;
    }
    
    //初始化task_struct结构信息
    new->task.pid = newpid;
    new->task.ASID = new->task.pid;
    new->task.parent = cur->pid;
    new->task.state = S_UNINIT;
	new->task.start_time = timecnt;

	//处理task_name中是否包含了其他信息，比如该进程所要运行的时间,进程的优先级等

	//默认task_name参数就只包含task_name
	kernel_strcpy(new->task.name, task_name);
	//默认3个进程时间单位
	int taskruntime = 3; 
	//后台调度链表，默认放入一级队列
	int schedlv = 1;

	//判断输入类型所用的flag
	int isruntimedeclared = 0;
	int isprioritydeclared = 0;

	//找到第一个空格
	int i;
	for (i = 0; i < strlen(task_name); i++)
		if (task_name[i] == ' ')
		{
			isruntimedeclared = 1;
			break;
		}

	//如果有第一个空格，就找第二个空格
	int i2;
	if (isruntimedeclared == 1)
	{
		for (i2 = i+1; i2 < strlen(task_name); i2++)
			if (task_name[i2] == ' ')
			{
				isprioritydeclared = 1;
				break;
			}
	}

	//如果没有第三个参数，i2设置为task_name长度，方便后面的处理
	if (isprioritydeclared == 0)
	{
		i2 = strlen(task_name);
	}

	//如果第一个参数非空，进程运行时间指定
	if (isruntimedeclared == 1)
	{
		char* true_task_name = (char*)kmalloc(sizeof(char)*(i + 1));
		int j;
		for (j = 0; j < i; j++)
			true_task_name[j] = task_name[j];
		true_task_name[i] = '\0';
		kernel_strcpy(new->task.name, true_task_name);
		taskruntime = 0;
		for (j = i + 1; j < i2; j++)
			taskruntime = taskruntime * 10 + task_name[j] - 48;
	}
	
	//如果第二个参数非空，进程运行优先级指定
	if (isprioritydeclared == 1)
	{
		int j;
		schedlv = 0;
		for (j = i2 + 1; j < strlen(task_name); j++)
			schedlv = schedlv * 10 + task_name[j] - 48;
		if (schedlv < SCHED_LV_MIN || schedlv > SCHED_LV_MAX)
		{
			kernel_printf("Error! Two small priority declared!");
			goto err_1;
		}
	}
	
	
	new->task.runtime = taskruntime;
	new->task.sched_lv = schedlv;

    /*拷贝文件*/
    new->task.task_files = 0;

    /*mm_struct*/
    if (is_user)
        //用户进程需要为其分配地址空间结构
        new->task.mm = mm_create();
    else
        new->task.mm = 0;
  

    //regs_context
    kernel_memset(&(new->task.context), 0, sizeof(struct regs_context));
    new->task.context.epc = (unsigned int)entry;    //设置新进程入口地址
    new->task.context.sp = (unsigned int)new + KERNEL_STACK_SIZE; //设置新进程内核栈
    asm volatile("la %0, _gp\n\t" : "=r"(init_gp)); 
    new->task.context.gp = init_gp;     
    //设置新进程参数
    new->task.context.a0 = argc;
    new->task.context.a1 = (unsigned int)args;

    //用于返回子进程pid
    if (retpid != 0)
        *retpid = newpid;

    //加入进程链表
    add_task(&(new->task));
    
    //加入调度链表
    if (new->task.pid == INIT_PID) {
        //前台调度链表
        new->task.time_cnt = CNT_LV0;
        new->task.sched_lv = SCHED_LV0;
        add_sched(&(new->task));
    }
    else {
        
        new->task.time_cnt = sched_time_cnt[new->task.sched_lv];
      //  add_sched_1(&(new->task));
        add_sched_back(&(new->task), new->task.sched_lv);
    }
    new->task.state = S_READY;
    #ifdef VMA_DEBUG
    kernel_printf("new task:%x  %d\n", new, new->task.pid);
    #endif
    return 0;

err_1:
    pid_free(newpid);
err_0:
    return 1;
}

//根据进程号杀死进程，将要杀死的进程从调度链表／等待链表中移除，放入结束链表
//返回0表示执行成功，否则表示执行失败
int pc_kill(pid_t pid) {
    int res;
    struct task_struct* task, *sched_node;

    //idle进程不能被杀死
    if (pid == IDLE_PID) {
        kernel_printf("You are killing idle process!!!\n");
        return 1;
    }

    //init进程不能被杀死 
    if (pid == INIT_PID) {
        kernel_printf("You are killing init process:kernel shell!!!\n");
        return 1;
    }

    //进程不能杀死自身
    if (pid == current_task->pid) {
        kernel_printf("You can't kill current task!\n");
        return 1;
    } 

    disable_interrupts();

    //通过进程id号检查要杀死的进程是否存在
    res = pid_check(pid);
    if (res == 0) {
        kernel_printf("pid not found!\n");
        goto error_0;
    }

    // 通过pid获取要被杀死的的进程的task_struct结构
    task = find_in_tasks(pid);
    
#ifdef  PC_DEBUG
    sched_node = find_in_sched(pid);
    if (sched_node == 0 || task == 0)
        goto error_1;

    if (task != sched_node) {
        kernel_printf("pc_kill: task != sched!!!!!!!BUG EXIST!!!!\n");
        goto error_2;
    }
#endif
	remove_sched(task);
    task->state = S_TERMINAL;
    add_exited(task);

    if (task->task_files != 0) {
        task_files_delete(task);
    }    
    
    if (task->mm != 0) {
        mm_delete(task->mm);
    }

    //释放pid
    pid_free(pid);
    enable_interrupts();
    return 0;

#ifdef  PC_DEBUG
error_2:
    while (1)
        ;
error_1:
    kernel_printf("pc_kill: task struct not found, There must be bug!!!\n");
#endif

error_0:   
    enable_interrupts();
    return 1;
}

// 根据进程号sleep进程，将要sleep的进程从调度链表中移除，放入等待链表
//返回0表示执行成功，否则表示执行失败
int pc_sleep(pid_t pid) {
	int res;
	struct task_struct* task, *sched_node;

	//idle进程不能被sleep
	if (pid == IDLE_PID) {
		kernel_printf("You are sleeping idle process!!!\n");
		return 1;
	}

	//init进程不能被sleep
	if (pid == INIT_PID) {
		kernel_printf("You are sleeping init process:kernel shell!!!\n");
		return 1;
	}

	//进程不能sleep自身
	if (pid == current_task->pid) {
		kernel_printf("You can't sleep current task!\n");
		return 1;
	}

	disable_interrupts();

	//通过进程id号检查要sleep的进程是否存在
	res = pid_check(pid);
	if (res == 0) {
		kernel_printf("pid not found!\n");
		goto error_0;
	}

	// 通过pid获取要被sleep的的进程的task_struct结构
	task = find_in_tasks(pid);

	//sleep的进程必须在调度队列当中
	if (task->state != S_READY)
	{
		kernel_printf("this task is not in sched list!\n");
		return 1;
	}

#ifdef  PC_DEBUG
	sched_node = find_in_sched(pid);
	if (sched_node == 0 || task == 0)
		goto error_1;

	if (task != sched_node) {
		kernel_printf("pc_sleep: task != sched!!!!!!!BUG EXIST!!!!\n");
		goto error_2;
	}
#endif
	remove_sched(task);
	task->state = S_SLEEPING;
	add_wait(task);

	enable_interrupts();
	return 0;

#ifdef  PC_DEBUG
	error_2 :
			while (1)
				;
		error_1:
			kernel_printf("pc_sleep: task struct not found, There must be bug!!!\n");
#endif

		error_0:
			enable_interrupts();
			return 1;
}

// 根据进程号wake进程，将要wake的进程从等待列表中移除，放入调度链表
//返回0表示执行成功，否则表示执行失败
int pc_wake(pid_t pid) {
	int res;
	struct task_struct* task, *sched_node;

	//idle进程不能被wake
	if (pid == IDLE_PID) {
		kernel_printf("You are waking idle process!!!\n");
		return 1;
	}

	//init进程不能被wake
	if (pid == INIT_PID) {
		kernel_printf("You are waking init process:kernel shell!!!\n");
		return 1;
	}

	//进程不能wake自身
	if (pid == current_task->pid) {
		kernel_printf("You can't wake current task!\n");
		return 1;
	}

	disable_interrupts();

	//通过进程id号检查要wake的进程是否存在
	res = pid_check(pid);
	if (res == 0) {
		kernel_printf("pid not found!\n");
		goto error_0;
	}

	// 通过pid获取要被wake的的进程的task_struct结构
	task = find_in_tasks(pid);

	//wake的进程状态必须是sleeping
	if (task->state != S_SLEEPING)
	{
		kernel_printf("this task is not sleeping!\n");
		return 1;
	}

#ifdef  PC_DEBUG
	sched_node = find_in_sched(pid);
	if (sched_node == 0 || task == 0)
		goto error_1;

	if (task != sched_node) {
		kernel_printf("pc_sleep: task != sched!!!!!!!BUG EXIST!!!!\n");
		goto error_2;
	}
#endif
	remove_sched(task);
	task->state = S_READY;
	if (task->sched_lv == SCHED_LV0)
	{
		add_sched(task);
	}
	else
	{
		add_sched_back(task, task->sched_lv);
	}

	enable_interrupts();
	return 0;

#ifdef  PC_DEBUG
	error_2 :
			while (1)
				;
		error_1:
			kernel_printf("pc_sleep: task struct not found, There must be bug!!!\n");
#endif

		error_0:
			enable_interrupts();
			return 1;
}


//清理结束链表
//在每次进行进程调度pc_schedule()时调用此函数
void clear_exited() 
{
    struct task_struct* next;
    
    #ifdef  PC_DEBUG
    int count = 0;  
    #endif
    
    //始终删除exited链表的第一个节点，直至exited为空
	next = exitedhead.what;
    while (next != 0x0) 
	{
        if (next->state != S_TERMINAL) 
		{
            kernel_printf("There is task_struct in exited list with state != TERMINAL.--%s", next->name);
            goto error_0;        
        }

        remove_exited(next);
        //在进程结束或者被杀死时并未将其从进程链表中移除，所以此处需要将其从进程链表中移除,也需要在这里真正的消除这个进程的task_struct结构
        remove_task(next);
		next = next->node.succ;
        #ifdef  PC_DEBUG
        kernel_printf("clear exited: count = %d\n", ++count);
        #endif

    }
    return;

error_0:
    while(1)
        ;
}

//进程退出，参数state目前并未使用，属于预留变量
void pc_exit(int state) 
{
    struct task_struct* cur;
    struct task_struct* next;

    //idle进程正在退出，报错，死机
    if (current_task->pid == IDLE_PID) {
        kernel_printf("!!!IDLE PROCESS IS EXITTING!\n");
        goto error_0;
    }

    //init进程正在退出，发出警告信息
    if (current_task->pid == INIT_PID) {
        kernel_printf("WARNING: init process(kernel shell) is exiting\n");
    }
    //清理task_struct中的一些信息
    cur = current_task;
    if (cur->task_files != 0) 
	{
        task_files_delete(current_task);
    }    
    if (cur->mm != 0) 
	{
        mm_delete(current_task->mm);
    }

	//进入异常模式 => 中断关闭
    asm volatile 
	(      
        "mfc0  $t0, $12\n\t"
        "ori   $t0, $t0, 0x02\n\t"
        "mtc0  $t0, $12\n\t"
        "nop\n\t"
        "nop\n\t"
    );
    

    //调用唤醒父进程函数 
    //唤醒父进程函数会检查当前进程的父进程是否在等待它，如果在等待，则唤醒父进程
    wakeup_parent();

    //！！！以下函数调用顺序不可改变
    
    //调用调度算法选择下一个要运行的进程
    next = find_next_task();

    if (next->mm != 0) 
	{
        //激活地址空间
        activate_mm(next);
    }

    remove_sched(cur);
	cur->state = S_TERMINAL;
    add_exited(cur);
	pid_free(cur->pid);
    current_task = next;

    //调用汇编代码,加载新的进程的上下文信息
    switch_ex(&(current_task->context));
    //never come here
    kernel_printf("Error: pc_exit\n");
    //
error_0:
    while(1)
        ;
}

//进程调度函数，由时钟中断触发
//参数pt_context指向当前进程的上下文信息
void pc_schedule(unsigned int status, unsigned int cause, context* pt_context) 
{
    struct task_struct *next;
    struct list_head *pos;

	timecnt = timecnt + 1; //每次时钟中断都会触发pc_schedule，这个时候时间片计数器+1

    //清理结束链表
    clear_exited();

    //调用调度算法，选取下一个要运行的进程
    next = find_next_task();
    if (next != current_task) 
	{
        //如果选取的进程不是当前进程
        if (next->mm != 0)
            //激活地址空间
            activate_mm(next);
        //将当前进程的上下文信息保存到其task_struct结构中的context变量
        copy_context(pt_context, &(current_task->context));
        current_task = next;
        //将新的要运行的进程的上下文信息保存到pt_context中，用于在中断退出时，将这些上下文信息加载到寄存器中
        copy_context(&(current_task->context), pt_context);
		
    }
    else 
	{
        //进到此处并不总是意味着是bug,如果只有idle进程，那么必须进入这里，否则就是bug
        kernel_printf("pc_schedule: next == current_task\n");
        while(1)
            ;
    }

    //将cp0中到count寄存器复位为0，结束时钟中断
    asm volatile("mtc0 $zero, $9\n\t");
}

//进程调度算法,返回选取的下一个要执行的进程的task_struct结构
//此处采用多级反馈队列调度算法， 但是在此算法基础上进行了部分修改
struct task_struct* find_next_task() 
{
    struct task_struct* next;
    int is_back;

    is_back = 0;
    next = (struct task_struct*)0;
    if (current_task->sched_lv == SCHED_LV0)    //当前在前台调度链表中的调度
        if (current_task->node.succ == 0x0)    //这个进程是目前sched队列中唯一的进程
            is_back = 1;
        else 
            next = current_task->node.succ;
    else 
	{
        if (current_task->sched_lv < SCHED_LV_MAX) 
		{
            current_task->time_cnt --;
            remove_sched(current_task);

			//时间片用完了就进入下一级优先队列
            if (current_task->time_cnt == 0) 
			{
                //debug
                kernel_printf("\npid %d drops from list %d\n",current_task->pid, current_task->sched_lv);
                //debug
                current_task->sched_lv ++;
                current_task->time_cnt = sched_time_cnt[current_task->sched_lv];
            }

			//如果长时间跑不完，为了防止饿死，就提高进程优先度
			if (timecnt - current_task->start_time > CNT_BASE * 40)
			{
				if (current_task->sched_lv > SCHED_LV_MIN)
				{
					kernel_printf("\npid %d upgrades from list %d\n", current_task->pid, current_task->sched_lv);
					current_task->sched_lv--;
					current_task->time_cnt = sched_time_cnt[current_task->sched_lv];
				}
				current_task->start_time = current_task->start_time + CNT_BASE * 40;
			}

            add_sched_back(current_task, current_task->sched_lv);
        }
		next = schedhead.what;
    }

    if (is_back == 1) 
	{
        next =find_in_sched_back();
		if ((unsigned int)next == 0)
		{
			next = schedhead.what;
		}
	}
    return next;
}

//遍历后台多级反馈队列，查找下一个要执行的进程
struct task_struct* find_in_sched_back() 
{
    int i;
    struct task_struct *next;

    for (i = 1; i <= SCHED_LV_RANGE; i++) 
	{
		struct task_struct* temp = sched_backhead[i].what;
        while (temp != 0x0)
		{
           if (temp->state == S_READY)
                return temp;
		   temp = temp->node.succ;
        }
    }
    return 0;

}

//输出调度链表信息
void print_sched() 
{
    struct task_struct *next;
    int i;
	//DEBUG
	kernel_printf("curent task:\n");
	print_struct_task(current_task);
    //输出前台调度链表
    kernel_printf("sched list:\n");
	next = schedhead.what;
	while (next != 0x0)
	{
        print_struct_task(next);
		next = next->node.succ;
    }
    
    //输出后台调度链表(多级反馈队列)
    for (i = 1; i <= SCHED_LV_RANGE; i++) 
	{
        kernel_printf("sched_back_lv%d: \n", i);
		next = sched_backhead[i].what;
		while (next != 0x0)
		{
            print_struct_task(next);
			next = next->node.succ;
        }
    }
}

//输出进程链表信息
void print_task() 
{
    struct task_struct *next;
    struct list_head *pos;
    int i;
    int count = 0;

    //debug
    // kernel_printf("current:\n");
    // print_struct_task(current_task);
    //debug

    kernel_printf("task list:\n");
	for (i=0; i<PID_NUM; i++)
		if (tasks[i] != 0x0)
		{
			print_struct_task(tasks[i]);
		}
}

//输出退出链表信息
void print_exited() 
{
    struct task_struct *next;
    int i;
    kernel_printf("exited list:\n");
	next = exitedhead.what;
    while (next != 0x0)
	{
        print_struct_task(next);
		next = next->node.succ;
    }        
}

//输出进程控制块-task_struct结构
void print_struct_task(struct task_struct* task) 
{
    kernel_printf("name:%s \t pid:%d \t  parent:%d \t", task->name, task->pid, task->parent);
    kernel_printf("state: %d\n", task->state);
} 

//输出等待链表信息
void print_wait() 
{
    struct task_struct* next;
    kernel_printf("wait list:\n");
	next = waithead.what;
    while (next != 0x0)
	{
        print_struct_task(next);
		next = next->node.succ;
    }
}

//此函数本设计用于加载用户程序，目前可直接忽略
int vmprog22(unsigned int argc, void *args) {
    char progname[128];
    int res;

    if (argc == 0 || args == 0)
        goto error_1;

    kernel_strcpy(progname, args);

    res = runuserprog(progname);
    //此处是不是不应该返回
    if (res != 0)
        goto error_2;

    //???  
    kernel_printf("runprog: runuserprog exits with no error");
    while(1)
        ;

error_2:
    kernel_printf("runprog: runuserprog error!\n");
error_1:
    kernel_printf("runprog: Error! Then Dead Loop!\n");
    while (1)
        ;
}

//用户进程统一内核入口
//args指向要加载的用户程序名
//目前用于测试TLB REFILL异常处理, 待后续扩展
int vmprog(unsigned int argc, void *args) {
    int i;
   
    unsigned int * test_addr = (unsigned int*)0;
    unsigned int test_val = 0, test_val2;
    *test_addr = 3;
    for (i = 0; i < 20; i++) {
        //重复访问用户空间虚拟地址,触发TLB miss异常,进行TLB refill处理操作
        test_addr = (unsigned int*)(i * 4);
        test_val += 3;
        *test_addr = test_val;
       kernel_printf("%x VMPROG WRITE:%x $$", (unsigned int)test_addr, test_val);
        test_val2 = *test_addr;
       kernel_printf("VMPROG READ: %x\n", test_val2);
    }

    #ifdef TLB_DEBUG
    kernel_getchar();
    #endif

    kernel_printf("go to here!\n");
    kernel_getchar();
    
    //进程退出
    pc_exit(0);
    return 0;
}


//加载外部用户程序，并以之替换为当前进程的执行内容
//由于目前仍进行到部分测试阶段,故该功能并未整合到操作系统中,并且函数内代码略显混乱
int runuserprog(char* progname) {
    // unsigned char buf[512];
    unsigned int entry, stackptr;
    unsigned int phy_entry;
    unsigned int stack_entry, heap_entry;
    int res, i;
    unsigned int size, npage, nbuf;
    struct mm_struct *newmm, *oldmm;

    #ifdef TLB_DEBUG
        unsigned int* debug_addr;
        unsigned int debug_val;
    #endif
    

    #ifdef VMA_DEBUG
    kernel_printf("runuser: pid %d\n", current_task->pid);
    #endif
//load user program
    // #ifdef TLB_DEBUG
    //     kernel_printf("progname: %s  %d\n", progname, sizeof(FILE));
    // #endif
    // current_task->task_files = kmalloc(sizeof(FILE));

    // if (current_task->task_files == 0) {
    //     kernel_printf("runuserprog: task files allocated failed!\n");
    //     goto error_0;
    // }
    // kernel_memset(current_task->task_files, 0, sizeof(FILE));
    
    // 调用VFS提供的打开接口
    struct file * file;
    file = vfs_open(progname, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(file)){
        if ( PTR_ERR(file) == -ENOENT )
            kernel_printf("File not found!\n");
            return PTR_ERR(file);
    }

    // 接下来读取文件数据区的内容到buf
    int base = 0;
    int file_size = file->f_dentry->d_inode->i_size;
    u8 *buf = (u8*) kmalloc (file_size + 1);
    if ( vfs_read(file, buf, file_size, &base) != file_size )
        return 1;
    vfs_close(file);

    
    // #ifdef VMA_DEBUG
    // kernel_printf("after read: %d  %s\n", current_task->pid, current_task->name);
    // #endif


    // res = fs_open(current_task->task_files, progname);
    // if (res != 0) {
    //     kernel_printf("runuserprog:File %s not exist\n", progname);
    //     goto error_1;
    // }

    //create mm
    newmm = mm_create();
    if (newmm == 0) {
        kernel_printf("runuserprog: mm_create create failed!\n");
        goto error_2;
    }
    //

    //load program
    // size = get_entry_filesize(current_task->task_files->entry.data);
    npage = file_size + (PAGE_SIZE - 1);
    npage >>= PAGE_SHIFT;
   phy_entry = (unsigned int) kmalloc(PAGE_SIZE * npage);
    
   // phy_entry = (unsigned int) kmalloc();

    if (phy_entry == 0) {
        kernel_printf("runuserprog: phy_entry allocated failed!\n");
        goto error_3;
    }


    



    //分配堆和栈
    stack_entry = (unsigned int) kmalloc(PAGE_SIZE * 2);
    if (stack_entry == 0) {
        kernel_printf("runuserprog: stack allocated failed!\n");
        goto error_3;
    }

    /*
    heap_entry = (unsigned int) kmalloc(PAGE_SIZE); //感觉这个暂时没用
    if (heap_entry == 0) {
        kernel_printf("runserprog: heap allocated failed!\n");
        goto error_4;
    }
    */

    res = do_mapping(newmm->pgd, USER_CODE_ENTRY, npage, phy_entry, 0x0f);
    if (res != 0) {
        kernel_printf("runuserprog:mapping for code segment failed!\n");
        goto error_5;
    }

    res = do_one_mapping(newmm->pgd, USER_STACK_ENTRY - PAGE_SIZE, stack_entry, 0x0f);
    if (res != 0) {
        kernel_printf("runuserprog:mapping for stack segment failed!\n");
        goto error_5;
    }
    // #ifdef VMA_DEBUG
    // kernel_printf("Map stack done\n");
    // #endif

    /*
    res = do_one_mapping(newmm->pgd, USER_HEAP_ENTRY, heap_entry, 0x0f);
    if (res != 0) {
        kernel_printf("runuserprog: mapping for heap segment failed!\n");
        goto error_5;
    }
    */

    oldmm = current_task->mm;
    current_task->mm = newmm;

    activate_mm(current_task);

    // init_pgtable();
    // #ifdef VMA_DEBUG
    // kernel_printf("after activate:%d  %s\n", current_task->pid, current_task->name);
    // #endif
 
   // if (oldmm != 0)
   //     mm_delete(oldmm);
    
    entry = USER_CODE_ENTRY;
    stackptr = USER_STACK_ENTRY - 32; //in case there is some special case

    // nbuf = npage * ((PAGE_SIZE + 511) / 512);
    // for (i = 0; i < nbuf; i++) {
    //     fs_read(current_task->task_files, buf, 512);
    //     kernel_memcpy((void*)(entry + i * 512), buf, 512);
    //     kernel_memcpy((void*)(phy_entry + i * 512), buf, 512);
    // }
    
    int CACHE_BLOCK_SIZE = 64;
    unsigned int n = file_size / CACHE_BLOCK_SIZE + 1;
    for (i = 0; i < n; i++) {
        // fs_read(current_task->task_files, buf, CACHE_BLOCK_SIZE);
        // kernel_memcpy((void*)(phy_entry + i * CACHE_BLOCK_SIZE), buf + i * CACHE_BLOCK_SIZE, CACHE_BLOCK_SIZE);
        // kernel_cache(phy_entry + i * CACHE_BLOCK_SIZE);
        kernel_memcpy((void*)(entry + i * CACHE_BLOCK_SIZE), buf + i * CACHE_BLOCK_SIZE, CACHE_BLOCK_SIZE);
        kernel_cache(entry + i * CACHE_BLOCK_SIZE);
    }

    kfree(buf);

   // fs_read(current_task->task_files, buf, 64);
   //     kernel_memcpy((void*)(entry + i * 512), buf, 512);
   // kernel_memcpy((void*)(phy_entry), buf, 64);
    //kernel_cache(phy_entry);
     
 
    #ifdef TLB_DEBUG
    
        kernel_printf("userspace-- %x  %x\n", entry, stackptr);
        kernel_printf("entry: %x\n", phy_entry);
        debug_addr = (unsigned int*)phy_entry;
        for (i = 0; i < 8; i++) {
            kernel_printf("%d: %x\n", i, *(debug_addr + i));
        }

        // kernel_getchar();
        debug_addr = (unsigned int*)entry;
        kernel_printf("user:\n");
        for (i = 0; i < 8; i++) {
            debug_val = *(debug_addr + i);
            kernel_printf("%x: %x $$$$$  \n", (debug_addr + i), debug_val);
            // *(debug_addr + i) = i * i;
            // debug_val = *(debug_addr + i);
            // kernel_printf("%x: %x\n", (debug_addr + i), debug_val);
        }

        // kernel_printf("entry again: %x\n", phy_entry);
        // debug_addr = (unsigned int*)phy_entry;
        // for (i = 0; i < 8; i++) {
        //     kernel_printf("%d: %x\n", i, *(debug_addr + i));
        // }
        // kernel_getchar();
        kernel_printf("count: %d\n", count_2);
  
    #endif

    // while(1)
    //     ;
   
    // asm volatile(
    //     "move  $t0, %0\n\t"
    //     "jalr  $t0\n\t"
    //     :
    //     :"r" (entry+4)
    // );
//    disable_interrupts();
    
    // int (*f)() = (int(*)())(entry);
    // res = f();
    // enable_interrupts();

    enter_new_pc(entry, stackptr);
    asm volatile(
        "move   $sp, %0\n\t"
        "addiu  $sp, $sp, 4096\n\t"
        :
        :"r" (current_task)
    );
    kernel_printf("return from user program!\n");

    // unsigned int* pgd = current_task->mm->pgd;
    // unsigned int pde, pte;
    // unsigned int* pde_ptr;
    // int j;

    // for (i = 0; i < 1024; i++) {
    //     pde = pgd[i];
    //     pde &= PAGE_MASK;
       
    //     if (pde == 0)  //不存在二级页表
    //         continue;
    //     kernel_printf("pde: %x\n", pde);
    //     pde_ptr = (unsigned int*)pde;
    //     for (j = 0; j < 1024; j++) {
    //         pte = pde_ptr[j];
    //         pte &= PAGE_MASK;
    //         if (pte != 0) {
    //             kernel_printf("\tpte: %x\n", pte);
    //         }
    //     }
    // }

    pc_exit(0);

error_5:
/*
    kfree(heap_entry);
*/
error_4:
    kfree((void*)stack_entry);
error_3:
    mm_delete(newmm);
error_2:
    // fs_close(current_task->task_files);
error_1:
    // kfree(current_task->task_files);
    // current_task->task_files = 0;
error_0:
    return 1;
};
