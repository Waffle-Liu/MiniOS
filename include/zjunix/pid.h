#ifndef  _ZJUNIX_PID_H
#define  _ZJUNIX_PID_H


#define  PID_NUM  256  //��������
#define  IDLE_PID  0            //idle ����
#define  INIT_PID  1            //for kernel shell
#define  PID_MIN   1

typedef unsigned int pid_t;
int pid_pool[PID_NUM]; //pid��
int pid_pool_head;
int pid_pool_tail;
int pid_pool_num;
int pid_bool[PID_NUM]; //pid״̬��ѯ�� 0��ʾpid��û���ã�1��ʾpid�ű�����

void init_pid();
int pid_alloc(pid_t *ret);
int pid_free(pid_t num);
int pid_check(pid_t pid);

#endif