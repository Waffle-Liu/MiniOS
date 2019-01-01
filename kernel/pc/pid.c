#include <zjunix/pid.h>
#include <driver/vga.h>

//��ʼ��pidλͼ
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

//���pid�Ƿ��ѱ�����
//�ѱ������򷵻�1�����򷵻�0
int pid_check(pid_t pid) {

	//pid ������������pid��
	if (pid >= PID_NUM) {
		return 0;
	}

	//������Ӧ��pid_boolλ�Ƿ�Ϊtrue�����Ϊtrue���ʾ�ѷ���
	if (pid_bool[pid] == 1) {
		return 1;
	}
	else {
		return 0;
	}
}

//����pid
//����ɹ��򷵻�0�����򷵻�1
//ret����·����pid��
int pid_alloc(pid_t *ret) {

	//pid�ؿ��ˣ�û�ҵ�
	if (pid_pool_num <= 0)
		return 1;

	//�ҵ�������֮
	*ret = pid_pool[pid_pool_head];
	pid_bool[pid_pool[pid_pool_head]] = 1;
	pid_pool_head++;
	pid_pool_num--;
	if (pid_pool_head >= PID_NUM) {
		pid_pool_head -= PID_NUM;
	}
	return 0;
}

//�ͷ�����Ϊpid�Ľ���pid��
//�Ƿ�ɹ��򷵻�0�����򷵻�1
int pid_free(pid_t pid) {
	int res;

	//���ȼ��pid�Ƿ��ѱ�����
	res = pid_check(pid);
	if (res == 0)
		return 1;
	else {
		//���ͨ�����ͷ�֮
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
