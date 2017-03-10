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
void child_make(int);
void send_client(int);
void send_poll(int, pdoc );
void  client_recv_cd(int,pdoc);
void client_recv_ls(pdoc);
void  client_recv_pwd(pdoc);
void client_recv_puts(int ,pdoc);
void client_recv_gets(int ,pdoc);
void client_recv_remove(pdoc);
void errorpfile(pdoc);
