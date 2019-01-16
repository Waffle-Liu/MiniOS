#include <zjunix/pid.h>
#include <driver/vga.h>

//初始化pid位图
void init_pid() {
	int i;
	for (i = 1; i < PID_NUM; i++)
		pid_bool[i] = 0;
	pid_bool[0] = 1;
	pid_pool_head = 0;
	pid_pool_tail = 0;
	pid_pool_num = 0;
	for (i = 1; i < PID_NUM; i++)
	{
		pid_pool[pid_pool_tail] = i;
		pid_pool_tail++;
		pid_pool_num++;
	}
}

//检查pid是否已被分配
//已被分配则返回1，否则返回0
int pid_check(pid_t pid) {

	//pid 大于允许的最大pid号
	if (pid >= PID_NUM) {
		return 0;
	}

	//查找相应的pid_bool位是否为true，如果为true则表示已分配
	if (pid_bool[pid] == 1) {
		return 1;
	}
	else {
		return 0;
	}
}

//分配pid
//分配成功则返回0，否则返回1
//ret存放新分配的pid号
int pid_alloc(pid_t *ret) {

	//pid池空了，没找到
	if (pid_pool_num <= 0)
		return 1;

	//找到，分配之
	*ret = pid_pool[pid_pool_head];
	pid_bool[pid_pool[pid_pool_head]] = 1;
	pid_pool_head++;
	pid_pool_num--;
	if (pid_pool_head >= PID_NUM) {
		pid_pool_head -= PID_NUM;
	}
	return 0;
}

//释放数字为pid的进程pid号
//是否成功则返回0，否则返回1
int pid_free(pid_t pid) {
	int res;

	//首先检查pid是否已被分配
	res = pid_check(pid);
	if (res == 0)
		return 1;
	else {
		//检查通过，释放之
		pid_pool[pid_pool_tail] = pid;
		pid_pool_tail++;
		if (pid_pool_tail >= PID_NUM) {
			pid_pool_tail -= PID_NUM;
		}
		pid_pool_num--;
		pid_bool[pid] = 0;
		return 0;
	}
}
