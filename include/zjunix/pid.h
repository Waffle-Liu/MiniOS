#ifndef  _ZJUNIX_PID_H
#define  _ZJUNIX_PID_H


#define  PID_NUM  256  //最大进程数
#define  IDLE_PID  0            //idle 进程
#define  INIT_PID  1            //for kernel shell
#define  PID_MIN   1

typedef unsigned int pid_t;
int pid_pool[PID_NUM]; //pid池
int pid_pool_head;
int pid_pool_tail;
int pid_pool_num;
int pid_bool[PID_NUM]; //pid状态查询表 0表示pid号没被用，1表示pid号被用了

void init_pid();
int pid_alloc(pid_t *ret);
int pid_free(pid_t num);
int pid_check(pid_t pid);

#endif