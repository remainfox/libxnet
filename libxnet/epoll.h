
#ifndef __XNET_EPOLL__
#define __XNET_EPOLL__


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

typedef void (*FUNC_CALLBACK)(int, int, void *);

/* 描述就绪文件描述符相关信息 */
struct event_data_s 
{    
	int efd;
	int fd;                                                 //要监听的文件描述符    
	int events;                                             //对应的监听事件   
	void *arg;                                              //泛型参数    
	FUNC_CALLBACK read_callback;       						//读回调函数  
	FUNC_CALLBACK write_callback;       					//写回调函数  
	int status;                                             //是否在监听:1->在红黑树上(监听), 0->不在(不监听)    
	char *buf;    
	int len;    
	long last_active;                                       //记录每次加入红黑树 g_efd 的时间值

	int live_statue;
	pthread_t pid;
	int thread_run_state;                                   //线程运行状态
};


#endif



