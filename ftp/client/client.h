#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<signal.h>
#include<sys/time.h>
#include<sys/select.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/uio.h>
#include<sys/epoll.h>
#include<dirent.h>

typedef struct file{
	int len;
	char buf[1000];
}doc, *pdoc;

void send_msg(int , int );
void recv_msg(int, int*);
void send_client(int);
void send_poll(int, pdoc );
int recv_cd(pdoc ,int);
int dir_match(pdoc);
void server_recv(int,pdoc);
void server_recv_puts(int , pdoc);
