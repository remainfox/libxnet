
#include <stdio.h>
#include "epoll.h"




/*将结构体 myevent_s 成员变量 初始化*/
void eventset(struct event_data_s *ev, int efd,int fd, FUNC_CALLBACK read_callback,
				FUNC_CALLBACK write_callback, void *arg)
{    
	ev->efd = efd;
	ev->fd = fd;    
	ev->read_callback  = read_callback;   
	ev->write_callback = write_callback;
	ev->events = 0;    
	ev->arg = arg;   
	ev->status = 0; 
	ev->live_statue = 0;
	ev->last_active = time(NULL);                       
	//调用eventset函数的时间    
	return;
}


/* 向 epoll监听的红黑树 添加一个 文件描述符 */
void eventadd(int events, struct event_data_s *ev)
{
    struct epoll_event epv = {0, {0}};
    int op;
    epv.data.ptr = ev;
    epv.events = ev->events = events ;       //EPOLLIN 或 EPOLLOUT

    if (ev->status == 1) {                                          //已经在红黑树 g_efd 里
        op = EPOLL_CTL_MOD;                                         //修改其属性
    } else {                                						//不在红黑树里
        op = EPOLL_CTL_ADD;                 						//将其加入红黑树 g_efd, 并将status置1
        ev->status = 1;
    }
	
    if (epoll_ctl(ev->efd, op, ev->fd, &epv) < 0)  {
        //printf("event add failed [fd=%d], events[%d],errno:%d\n", ev->fd, events,errno);
	}
    else{
        //printf("event add OK [fd=%d]\n", ev->fd, op);
	}

    return ;
}


void eventdel(int efd, struct event_data_s *ev)
{
    struct epoll_event epv = {0, {0}};

    if (ev->status != 1)                                        //不在红黑树上
        return ;

    epv.data.ptr = ev;
    ev->status = 0;                                             //修改状态
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);                //从红黑树 efd 上将 ev->fd 摘除

    return ;
}



