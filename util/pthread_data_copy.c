#include 	<unistd.h>
#include	<signal.h>
#include 	<pthread.h>

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>

#include	"pthread_data_copy.h"
#include "socket/VirtualPortMonitor.h"




int pthread_data_copy( char *argd,char *args,int *Length)
{
						int rc	=1;
						while(rc){
							rc	=	pthread_mutex_trylock(&public_data_seg_mutex_lock);
							usleep(10000);
						}

						memcpy(argd,args,*Length);

		        pthread_mutex_unlock(&public_data_seg_mutex_lock);
		        return 0;
}



/*

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *arry;
};

static	int pthread_data_copy_run_flag = 0;
static int sem_id = 0;

static int set_semvalue();
static void del_semvalue();
static int semaphore_p();
static int semaphore_v();

int pthread_data_copy( char *argd,char *args,int *Length)
{

    if(pthread_data_copy_run_flag < 1)
    {
    	 //�����ź���
       sem_id = semget((key_t)public_semaphore_protect_seg, 1, 0666 | IPC_CREAT);
        //�����һ�α����ã���ʼ���ź���
       if(!set_semvalue()){
            fprintf(stderr, "Failed to initialize semaphore\n");
            return	 -1;   
       }
       pthread_data_copy_run_flag	+=	1;
    }
         //�����ٽ���
  if(!semaphore_p())
            return	 -1;

	memcpy(argd,args,*Length);
	
 	if(!semaphore_v())
            return	 -1;
	pthread_data_copy_run_flag	-=	1;
   if(pthread_data_copy_run_flag < 1){
        //��������ǵ�һ�α����ã������˳�ǰɾ���ź���
      del_semvalue();
    }
			return	 0;
}

static int set_semvalue()
{
    //���ڳ�ʼ���ź�������ʹ���ź���ǰ����������
    union semun sem_union;

    sem_union.val = 1;
    if(semctl(sem_id, 0, SETVAL, sem_union) == -1)
        return 0;
    return 1;
}

static void del_semvalue()
{
    //ɾ���ź���
    union semun sem_union;

    if(semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p()
{
    //���ź�������1���������ȴ�P��sv��
    struct sembuf sem_b;
   sem_b.sem_num = 0;
    sem_b.sem_op = -1;//P()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_p failed\n");
        return 0;
    }
    return 1;
}

static int semaphore_v()
{
    //����һ���ͷŲ�������ʹ�ź�����Ϊ���ã��������ź�V��sv��
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;//V()
    sem_b.sem_flg = SEM_UNDO;
    if(semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_v failed\n");
        return 0;
    }
    return 1;
}

*/