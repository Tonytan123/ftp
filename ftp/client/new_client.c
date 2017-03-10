#include"client.h"

int main(int argc, char *argv[])
{
	if(3 != argc)
	{
		printf("error argc");
		return -1;
	}
	int fd = socket(AF_INET,SOCK_STREAM,0);
	int i = 0;
	struct sockaddr_in seraddr;
	bzero(&seraddr,sizeof(struct sockaddr_in));
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(atoi(argv[1]));
	seraddr.sin_addr.s_addr = inet_addr(argv[2]);
	int ret = connect(fd, (struct sockaddr*)&seraddr,sizeof(struct sockaddr));
	if(-1 == ret)
	{
		perror("connect");
		return -1;
	}

	pdoc dfile;
	dfile = (pdoc)malloc(sizeof(struct file));
	bzero(dfile, sizeof(doc));
	char buf[128];
	char recvbuf[1024];
	bzero(recvbuf,1024);
	while(bzero(buf, 128), (ret = read(0, buf,sizeof(buf))>0))
	{
		dfile->len = strlen(buf)-1;
		memcpy(dfile->buf,buf,dfile->len);
		printf("dfile->buf=%s\n",dfile->buf);
		printf("dfile->len=%d\n",dfile->len);
		if(memcmp("cd ", dfile->buf,3) == 0)
		{		
			send(fd,dfile,dfile->len+4,0);
			ret = recv_cd(dfile,fd);
			if(-1 == ret)			{
				break;
			}
		}
		else if (memcmp("ls",dfile->buf,2) == 0&&dfile->len == 2)
		{
			send(fd,dfile,dfile->len+4,0);
			ret = recv_cd(dfile,fd);
			if(-1 == ret)
			{
				break;
			}
		}
		else if (memcmp("pwd",dfile->buf,3) == 0&&dfile->len == 3)
		{
			send(fd,dfile,dfile->len+4,0);
			ret = recv_cd(dfile,fd);
			if(-1 == ret)
			{
				break;
			}
		}
		else if (memcmp("puts ",dfile->buf,5) == 0)
		{
			ret = dir_match(dfile);
			if(0 == ret)
			{
				send(fd,dfile,dfile->len+4,0);
				server_recv(fd,dfile);
				recv_cd(dfile,fd);
			}

		}
		else if(memcmp("gets ",dfile->buf,5) == 0)
		{
			send(fd,dfile,dfile->len+4,0);
			server_recv_puts(fd,dfile);
			recv_cd(dfile,fd);
		}
		else if(memcmp("remove ",dfile->buf,7) == 0)
		{
			send(fd,dfile,dfile->len+4,0);
			ret = recv_cd(dfile,fd);
			if(-1 == ret)
			{
				break;
			}
		}
		else
		{
			printf("no command \'%s\'found\n",dfile->buf);
		}
		bzero(dfile,sizeof(struct file));
	}
	close(fd);
}

int recv_cd(pdoc dfile,int fd)
{
	bzero(dfile,sizeof(struct file));
	int ret = recv(fd,&dfile->len,4,0);
	if(ret == 0)
		return -1;
	else if(-1 == ret)
	{
		perror("recv");
		return -1;
	}
	ret =recv(fd, dfile->buf, dfile->len,0 );
	if(ret>0)
	{
		printf("%s\n",dfile->buf);
	}
	return 0;
}

int dir_match(pdoc dfile)
{
	char fbuf[128];
	bzero(fbuf,128);
	sscanf(dfile->buf,"%*s%s",fbuf);

	struct dirent *entry;
	struct stat statbuf;
	DIR*dp;
	char buf[128];
	bzero(buf,128);
	char buffer[1024];
	bzero(buffer,1024);

	getcwd(buf,128);
	if((dp = opendir(buf)) == NULL)
	{
		perror("opendir");
		return -1;
	}
	while((entry = readdir(dp)) != NULL)
	{
		lstat(entry->d_name,&statbuf);
		if(S_ISDIR(statbuf.st_mode))
		{
			if ( strcmp(".",entry->d_name) == 0||strcmp("..",entry->d_name)==0)
				continue;
			sprintf(buffer,"%s",entry->d_name);
			if (memcmp(fbuf,buffer,sizeof(fbuf)) == 0)
			{
				printf("the file is a directory\n");
				return 1;
			}
			bzero(buffer,1024);
		}
		else
		{
			sprintf(buffer,"%s",entry->d_name);
			if (memcmp(fbuf,buffer,sizeof(fbuf)) == 0)
			{
				return 0;
			}
			bzero(buffer,1024);
		}
	}
	closedir(dp);
	printf("no files in this in this dirctory\n");
	return -1;
}


void server_recv(int fd,pdoc pfile)
{
	char fbuf[128];
	bzero(fbuf,128);
	int ret=0;
	sscanf(pfile->buf,"%*s%s",fbuf);
	int fdr;
	fdr = open(fbuf,O_RDONLY|O_CREAT,0666);
	while(bzero(pfile,sizeof(struct file)),(pfile->len =read(fdr,pfile->buf,sizeof(pfile->buf)))>0)
	{
		send_poll(fd,pfile);
	}
	close(fdr);

	bzero(pfile,sizeof(struct file));
	pfile->len = sizeof(int);
	memcmp(pfile->buf,&ret,pfile->len+4);
	send(fd,pfile,pfile->len+4,0);
}


void send_poll(int fd,pdoc pfile)
{
	int size =0;
	int len = 0;
	int total = pfile->len+4;
	while(len < total)
	{
		size = send(fd,pfile+len,total-len,0);
		len+= size;
	}
}
void server_recv_puts(int sfd,pdoc pfile)
{
	int read = 0;
	int ret;
	int fd;
	char buf[128];
	bzero(buf,128);
	sscanf(pfile->buf,"%*s%s",buf);
	bzero(pfile,sizeof(struct file));
	fd = open(buf,O_WRONLY|O_CREAT,0666);
	if(-1 == fd)
	{
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
				return ;
			}	
			ret = recv(sfd, pfile->buf,pfile->len,0);
			if(-1 == ret)
			{
				perror("recv");
				return ;
			}
			if(pfile->len == 4 && memcmp(pfile->buf,&read,4)==0)
			{
				break;
			}
			write(fd, pfile->buf,pfile->len);
			printf("gets:pfile->buf=%s",pfile->buf);
			bzero(pfile,sizeof(struct file));
		}
		close(fd);
	}

}
