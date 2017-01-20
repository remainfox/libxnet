
#ifndef __XNET_CONNECT__
#define __XNET_CONNECT__


#include "queue.h"

#define   READ_BUFFER_SIZE    4096
#define   WRITE_BUFFER_SIZE   4096



struct client_conn_s {
	queue_t head;

	int sockfd;						//该HTTP连接的socket
	struct sockaddr_in address;		//对方的socket地址

	char read_buf[READ_BUFFER_SIZE];//读缓冲区
	int read_index;					//标识读缓冲中已经读入的客户端数据的最后一个字节的下一个位置


	char write_buf[WRITE_BUFFER_SIZE];				//想写缓冲区
	int write_index;				//写缓冲区待发送的字节数

	
	char* file_address;				//客户请求的目标文件被mmap到内存中的起始位置
	struct stat file_stat;			//目标文件的状态，通过它可以判断文件是否存在，是否为目录，是否可读，并获取文件大小等信息
	struct iovec iv[2];				//使用writev来执行写操作，所以定义下面两个成员，其中iv_count表示被写内存块的数量
	int iv_count;
};

typedef struct client_conn_s client_conn_t;


#endif


