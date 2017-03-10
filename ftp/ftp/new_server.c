#include"func.h"

int main(int argc, char *argv[])
{
	if(3 != argc)
	{
		printf("error argc\n");
		return -1;
	}
	int fd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == fd)
	{
		perror("socket");
		exit(-1);
	}

	struct sockaddr_in my_addr;
	bzero(&my_addr,sizeof(struct sockaddr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(atoi(argv[1]));
	my_addr.sin_addr.s_addr = inet_addr(argv[2]);
	int ret = bind(fd,(struct sockaddr*)&my_addr,sizeof(struct sockaddr));
	if(-1 == ret)
	{
		perror("bind");
		return -1;
	}

	ret = listen(fd,1);
	if(-1 == ret)
	{
		perror("listen");
		return -1;
	}

	struct sockaddr_in clientaddr;
	int addrlen = sizeof(struct sockaddr);
	bzero(&clientaddr,addrlen);
	char buf[1024];
	bzero(buf,1024);

	//制造子进程
	int pid;
	int fds[2];

	ret = socketpair(AF_LOCAL,SOCK_STREAM,0,fds);
	if(-1 ==ret)
	{
		perror("socketpair");
		return -1;
	}
	pid = fork();
	if(pid==0)
	{

		printf("making child\n");
		close(fds[1]);
		printf("i am here \n");
		while(1)
		{
			child_make(fds[0]);
		}		return 0;
	}
	if(-1 == pid)
	{
		printf("create failed\n");
		return -1;
	}
	close(fds[0]);

	int sfd;
	int i= 1;
	//循环接受数据
	while(1)
	{
		sfd = accept(fd,(struct sockaddr*)&clientaddr,&addrlen);
		if(-1 == sfd)
		{
			perror("accept");
			close(fd);
			return -1;
		}
		printf("%s success connect\nport:%d\n",inet_ntoa(clientaddr.sin_addr),clientaddr.sin_port);
		printf("client%d connected\n",i);
		send_msg(fds[1],sfd);
		printf("circet\n");
		i++;
	}
	close(fd);
	close(sfd);
}


void child_make(int fd)
{
	int sfd;
	int ret;
	pdoc pfile;
	pfile = (pdoc)malloc(sizeof(struct file));
	printf("in child now\n");
	recv_msg(fd,&sfd);
	while(1)
	{
		bzero(pfile,sizeof(struct file));
		printf("child made\n");
		ret = recv(sfd,&pfile->len,4,0);
		if(-1 == ret)
		{
			perror("recv");
			return ;
		}
		if(0 == ret)
		{
			break;
		}
		ret = recv(sfd,&pfile->buf,pfile->len,0);
		printf("pfile->buf = %s\n",pfile->buf);
		if(-1 == ret)
		{
			perror("recv");
			return ;
		}
		if(0 == memcmp("cd ",pfile->buf,3))
		{
			client_recv_cd(sfd, pfile);
		}
		else if(0 == memcmp("ls",pfile->buf,2))
		{
			client_recv_ls(pfile);
		}
		else if(0 == memcmp("pwd",pfile->buf,3))
		{
			client_recv_pwd(pfile);
		}
		else if(0 == memcmp("puts",pfile->buf,4))
		{
			client_recv_puts(sfd,pfile);
		}else if(0 == memcmp("gets",pfile->buf,4))
		{
			client_recv_gets(sfd,pfile);
		}
		else if(0 == memcmp("remove ",pfile->buf,7))
		{
			client_recv_remove(pfile);
		}
		ret = send(sfd,pfile,pfile->len+4,0);
		printf("%s\n",pfile->buf);
		if(-1 == ret)
		{
			perror("recv");
			return ;
		}
	}
	free(pfile);
}


void client_recv_cd(int fd,pdoc pfile)
{
	int ret;
	char buf[128];
	bzero(buf,128);
	sscanf(pfile->buf,"%*s%s",buf);
	printf("buf=%s\n",buf);
	bzero(pfile,sizeof(struct file));
	ret = chdir(buf);
	if(-1 == ret)
	{
		pfile->len = strlen("No such file or directory");
		memcpy(pfile->buf,"No such file or directory",pfile->len);
}	
	else
	{
		bzero(buf,128);
		getcwd(buf,128);
		printf("cd:buf=%s\n",buf);
		pfile->len = strlen(buf);
		memcpy(pfile->buf,buf,pfile->len);
		printf("cd:pfile->buf=%s\n",pfile->buf);
	}
}

void client_recv_ls(pdoc pfile)
{
	struct dirent *entry;
	struct stat statbuf;
	DIR*dp;
	char buf[128];
	bzero(buf,128);
	char buffer[1024];
	bzero(buffer,1024);
	bzero(pfile,sizeof(struct file));
	printf("i am here\n");
	getcwd(buf,128);
	printf("buf = %s\n",buf);
	if((dp = opendir(buf)) == NULL)
	{
		perror("opendir");
		return ;
	}
	while((entry = readdir(dp)) != NULL)
	{
		lstat(entry->d_name,&statbuf);
		if(S_ISDIR(statbuf.st_mode))
		{
			if ( strcmp(".",entry->d_name) == 0||strcmp("..",entry->d_name)==0)
				continue;
			sprintf(buffer,"%s",entry->d_name);
			strncat(pfile->buf,buffer,strlen(buffer));
			strncat(pfile->buf," ",1);
			bzero(buffer,1024);
		}
		else
		{
			sprintf(buffer,"%s",entry->d_name);
			strncat(pfile->buf,buffer,strlen(buffer));
			strncat(pfile->buf," ",1);
			bzero(buffer,1024);
		}
	}
	closedir(dp);
	pfile->len = strlen(pfile->buf);
	printf("ls:pfile->len=%d\n",pfile->len);
	printf("ls:pfile->buf=%s\n",pfile->buf);
	return;
}


void client_recv_pwd(pdoc pfile)
{
	char buf[128];
	bzero(buf,128);
	getcwd(buf,128);
	pfile->len = strlen(buf);
	memcpy(pfile->buf,buf,pfile->len);
}

void client_recv_puts(int sfd,pdoc pfile)
{
	int read = 0;
	int ret;
	int fd;
	char buf[128];
	bzero(buf,128);
	sscanf(pfile->buf,"%*s%s",buf);
	printf("puts:buf= %s\n",buf);
	bzero(pfile,sizeof(struct file));
	fd = open(buf,O_WRONLY|O_CREAT,0666);
	if(-1 == fd)
	{
		perror("open");
		pfile->len = strlen("fail to puts files");
		memcpy(pfile->buf,"fail to puts files",pfile->len);
		return ;
	}
	else
	{
		while(1)
		{
			ret = recv(sfd,&pfile->len,4,0);
			if(-1 == ret)
			{
				perror("recv");
				errorpfile(pfile);
				return ;
			}
			else if(0 == ret)	
			{
				break;
			}
			ret = recv(sfd, pfile->buf,pfile->len,0);
			if(-1 == ret)
			{
				perror("recv");
				errorpfile(pfile);
				return ;
			}
			if(pfile->len == 4 && memcmp(pfile->buf,&read,4)==0)
			{
				break;
			}
			write(fd, pfile->buf,pfile->len);
			printf("pfile->buf=%s\n",pfile->buf);
			bzero(pfile,sizeof(struct file));
		}
		close(fd);
	}
	client_recv_ls(pfile);
}

void errorpfile(pdoc pfile)
{
	pfile->len = strlen("error recv");
	memcpy(pfile->buf,"error recv",pfile->len);
}


void client_recv_gets(int fd,pdoc pfile)
{
	char fbuf[128];
	bzero(fbuf,128);
	int ret=0;
	sscanf(pfile->buf,"%*s%s",fbuf);
	printf("gets:fbuf=%s\n",fbuf);
	bzero(pfile,sizeof(struct file));
	int fdr;
	fdr = open(fbuf,O_RDONLY,0666);
	if(-1 == fdr)
	{
		pfile->len = strlen("no such files:gets failed");
		memcpy(pfile->buf,"no such files:gets failed",pfile->len);
		return ;
	}
	while(bzero(pfile,sizeof(struct file)),(pfile->len =read(fdr,pfile->buf,sizeof(pfile->buf)))>0)
	{
		send_poll(fd,pfile);
	}

	bzero(pfile,sizeof(struct file));
	pfile->len =sizeof(int);
	memcpy(pfile->buf,&ret,pfile->len);
	send(fd,pfile,pfile->len+4,0);
	close(fdr);
	client_recv_ls(pfile);
}


void send_poll(int fd,pdoc pfile)
{
	int size=0;
	int len = 0;
	int total = pfile->len+4;
	while(len < total)
	{
		size = send(fd,pfile+len,total-len,0);
		len+= size;
	}
}


void client_recv_remove(pdoc pfile)
{
	char fbuf[128];
	bzero(fbuf,128);
	char buf[128];
	bzero(buf,128);
	int ret=0;
	sscanf(pfile->buf,"%*s%s",fbuf);
	getcwd(buf,128);
	memcpy(buf,fbuf,strlen(fbuf));
	remove(fbuf);
	client_recv_ls(pfile);
}
