
#include "common.h"


#define  PORT              8001
#define  MAX_CLIENT        512     
#define  MAX_EVENTS        1024                                    //监听上限数

#define  READ_SIZE         2


typedef struct headStruct{
	char identify[4];
	int seq;
	int len;
	unsigned int timeStamp;
}headStruct;

static char data1[1024*1024];
static char data2[1024*1024];
static int len1;
static int len2;


void* SendThread(void *arg);

int 
xnet_set_nonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}


int 
xnet_init_socket(int port)
{

	int listen_fd     = 0;
	int ret           = 0;
	int flag          = 1;
	
	do
	{
		struct sockaddr_in  severAddr;
		ret = listen_fd = socket(AF_INET, SOCK_STREAM, 0);
		if(listen_fd < 0){
			printf("socket err!\n");
			break;
		}

		memset(&severAddr, 0, sizeof(struct sockaddr_in));
		severAddr.sin_family = AF_INET;
		severAddr.sin_port = htons(port);
		severAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		
		if(-1 == (ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)))){
			printf("REUSE IO ERROR\n");
			break;
		}

		xnet_set_nonblocking(listen_fd);
		
		ret = bind(listen_fd, (struct sockaddr*)&severAddr, sizeof(struct sockaddr));
		if(ret < 0){
			printf("bind sock err!\n");
			close(listen_fd);
			break;
		}
		
		ret = listen(listen_fd, 10);
		if(ret < 0){
			printf("listen sock err!\n");
			close(listen_fd);
			break;
		}
		
		return listen_fd;

	}while(0);

	return ret;
	
}



static thread_pool_t * 
xnet_new_threadpool(int listen_fd)
{
	int ret = 0;
	thread_pool_t * pool = (thread_pool_t *)xnet_create_threadpool(10,MAX_CLIENT);
	if (pool == NULL){
		printf("create thread pool is failed.");
		return NULL;
	}

	pool->socket_fd = listen_fd;
	
	return pool;
}

static void * 
connection_cb(int fd,int event,void *arg)
{
	//printf("new clients,fd:%d\n",fd);
}

static void *
recv_cb(int fd,int event,void *arg)
{
	struct event_data_s *ev = (struct event_data_s *)arg;

	char buf[2048];
	memset(buf,0,sizeof(buf));
	
	xnet_recv_data(ev,buf,sizeof(buf));

	//printf("recieve clients,fd:%d,content:%s\n",fd,buf);

}


static void 
xnet_init_new_connect(int efd, int connect_fd,
			FUNC_CALLBACK read_callback,FUNC_CALLBACK write_callback)
{

	struct event_data_s *edata = malloc(sizeof(struct event_data_s));
	if(NULL == edata){
		printf("edata malloc failed\n");
		return;
	}

	int reuse = 1;
	setsockopt(connect_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	xnet_set_nonblocking(connect_fd);

	eventset(edata,efd,connect_fd, read_callback,write_callback, edata);
	eventadd(EPOLLIN | EPOLLET | EPOLLOUT, edata);    // listen fd   epoll in  

	edata->live_statue = 1;
	edata->thread_run_state = 0;

	printf("new clients,fd:%d\n",connect_fd);

	if(edata->live_statue && !edata->thread_run_state){
		printf("start send thread , client fd : %d\n",edata->fd);
		pthread_create(&edata->pid,NULL,SendThread,edata);
	}
	
	
}

static int 
xnet_close_client(struct event_data_s *ev)
{
	if(!ev)
		return -1;
	
	if(ev->live_statue == 0)
		return -1;
	
	if(ev->fd < 0)
		return -1;
	
	ev->live_statue = 0;
	//pthread_cancel(ev->pid);
	//pthread_join(ev->pid,NULL);

	eventdel(ev->efd, ev);

	close(ev->fd);
		
	printf("clients closed:%d\n",ev->fd);

	free(ev);
	
	ev = NULL;
			
	return 0;
}



void* 
SendThread(void *arg)
{
	pthread_detach(pthread_self());

	struct event_data_s *ev = (struct event_data_s *)arg;

	char buf[] = "hello libxnet\n";

	ev->thread_run_state = 1;

	int pic_type = 0;

	headStruct header;
	memset(&header,0,sizeof(header));
	header.seq = 2;
	header.timeStamp = 5;
	
	while(1){

		if(!ev->live_statue){
			goto exit;
		}
		
		if(pic_type){
			header.len = len1;
			xnet_send_data(ev,(char *)&header,sizeof(header));
			xnet_send_data(ev, data1, len1);
		}
		else{
			header.len = len2;
			xnet_send_data(ev,(char *)&header,sizeof(header));
			xnet_send_data(ev, data2, len2);
		}
		
		pic_type = !pic_type;
		
		usleep(40*1000);
	}

exit:
	ev->thread_run_state = 0;

	printf("client send thread quit:%d",ev->fd);
	
}


static void *
write_cb(int fd,int event,void *arg)
{
	//printf(" clients is write enable,fd:%d\n",fd);

	
	struct event_data_s *ev = (struct event_data_s *)arg;
	/*
	char szText[] = "hello libxnet ";
	xnet_send_data(ev,szText,sizeof(szText));
	

	if(ev->live_statue && !ev->thread_run_state){
		printf("start send thread , client fd : %d\n",ev->fd);
		pthread_create(&ev->pid,NULL,SendThread,arg);
	}
	*/
	
}


int
xnet_send_data(struct event_data_s *ev, char *buf, int size)
{
	if(ev->fd < 0 || NULL == buf){
		return -1;
	}
	int writen = 0;
	int write_len = 0;
	
	do
	{
		writen = send(ev->fd, buf + write_len, size - write_len, 0);
		if(writen == -1){

			if(errno == EAGAIN){           // send all data success 
				break; 
			}
			else if(errno == EINTR){       // signal internal 
				continue;
			}
			else if(errno == ECONNRESET){  // clients closed
				printf("[send data] client closed \n");
				goto closed;  
			}else{
				goto closed;  
			}
		}
		if(0 == writen){
			goto closed;
		}

		write_len += writen;
		
	}while (write_len < size);
	
	return 0;
	
closed:
	//printf("[%s %d] clients is closed,fd:%d,errno:%d\n",__FUNCTION__,__LINE__,ev->fd,errno);
	xnet_close_client(ev);
	return -1;
	
}


int 
xnet_recv_data(struct event_data_s *ev, char *buf, int size)
{
	if(ev->fd < 0 || NULL == buf){
		return -1;
	}
	
	int read_len = 0;
	int readn    = 0;
	
	do
	{
		readn = recv(ev->fd,buf + read_len,READ_SIZE,0);

		if( -1 == readn ){
			if (errno == EAGAIN || errno == EWOULDBLOCK){
				break;
			}
			goto closed;
		}
		
		if (0 == readn ){
			goto closed;
		}
		
		read_len += readn;

		if(read_len > size)break;
		
	}while(readn > 0);

	return 0;

closed:
	//printf("[%s %d] clients is closed,fd:%d,errno:%d\n",__FUNCTION__,__LINE__,ev->fd,errno);
	xnet_close_client(ev);
	return -1;

}


int 
main(int argc,char *argv[])
{

	if(argc < 3){
		printf("need input jpg param\n");
		return 0;
	}
	
	FILE*fp1 = fopen(argv[1],"rb");
	len1 = fread(data1,1,1000000,fp1);
	fclose(fp1);
	FILE*fp2 = fopen(argv[2],"rb");
	len2 = fread(data2,1,1000000,fp2);
	fclose(fp2);

	struct epoll_event events[MAX_EVENTS];	
	struct sockaddr_in client_address;
	socklen_t client_addr_len = sizeof(client_address);

	int listen_fd = xnet_init_socket(PORT);
	if(listen_fd < 0){
		printf("listen socket create failed\n");
		return -1;
	}

	/*
	thread_pool_t *pool = xnet_new_threadpool(listen_fd);
	if(NULL == pool){
		printf("create thread pool failed\n");
		return -1;
	}
	*/

	int efd = epoll_create(MAX_EVENTS);
	if(efd < 0){
		printf("epoll create falied\n");
		return -1;
	}
	
	struct event_data_s *edata = malloc(sizeof(struct event_data_s));
	if(edata == NULL){
		printf("epoll create falied\n");
		return -1;
	}
	
	eventset(edata,efd,listen_fd,connection_cb,NULL, edata);

	eventadd(EPOLLIN, edata);    // listen fd   epoll in  
	
	while (1)
	{
		int nReady = epoll_wait(efd, events, MAX_EVENTS, 200);
		int i = 0;

		for (; i < nReady; i++){
			
			struct event_data_s *ev = (struct event_data_s*)events[i].data.ptr;  
			if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) {           //读就绪事件

				if (ev->fd == listen_fd){
					int conn_fd = accept(listen_fd, (struct sockaddr*)&client_address,&client_addr_len);
					if (conn_fd < 0){
						printf("errno is : %d\n",errno);
						continue;
					}
					//insert new fd to epool and set noblocking
					xnet_init_new_connect(efd,conn_fd,recv_cb,write_cb);

					//call connection_cb here
					if(ev->read_callback)
				    	ev->read_callback(ev->fd, events[i].events, ev->arg);
					
				}else{
					//handle connect fd read event
					if(ev->fd > 0){
						if(ev->read_callback)
							ev->read_callback(ev->fd, events[i].events, ev->arg);
					}
				}
            }
			else if((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)){

				if(ev->fd > 0){
					if(ev->write_callback)
						ev->write_callback(ev->fd, events[i].events, ev->arg);
				}
			}

		}
	}

	close(efd);
	close(listen_fd);
	//destroy_thread_pool(pool);
	return 0;
	
		

}

