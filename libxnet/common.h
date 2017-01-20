
#ifndef __COMMON_XNET__
#define __COMMON_XNET__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <ifaddrs.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "queue.h"
#include "connect.h"
#include "epoll.h"

struct thread_pool_s
{
	int thread_number;	//线程池中的线程数
	int conn_number;	//请求连接的个数
	int max_resquests;	//请求队列中允许的最大请求数
	pthread_t* threads;	//描述线程池的数组，
	queue_t conn_head;  
	pthread_mutex_t locker;
	sem_t sem;
	int stop;

	int socket_fd;     //服务器的fd
};

typedef struct thread_pool_s thread_pool_t;



#endif


