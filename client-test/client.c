#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>

#define ADDR	"127.0.0.1"
#define MAX_EVENTS	65535


static int port=8000;
int idx=0;

static int connect_to_server(int port)
{
	struct sockaddr_in srvaddr;
	int sockfd=0;
	int res=0;
	char sendbuf[1024]="hello epoll server\n";
	
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	
	srvaddr.sin_family=AF_INET;
	srvaddr.sin_port=htons(port);
	srvaddr.sin_addr.s_addr=inet_addr(ADDR);
	
	res=connect(sockfd,(struct sockaddr*)&srvaddr,sizeof(srvaddr));
	if(res==-1)
	{
		perror("connect error.\n");
		return -1;
	}
	else
	{
		res=write(sockfd,(char*)sendbuf,strlen(sendbuf));
		if(res<0)
		{
			perror("send error.\n");
			return -1;
		}
		printf("connect index:%d.\n",++idx);
	}
	close(sockfd);
	return 0;
}

int main(int argc,char* argv[])
{
	int i=0;
	
	for(i=0;i<MAX_EVENTS;++i)
	{
		usleep(100000);
		connect_to_server(port);
	}
	
	while(1)
	{
		sleep(1);
	}
	return 0;
}











