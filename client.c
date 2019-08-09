#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

void* start_read(void* arg) // 读取信息
{
	int sockfd = *(int*)arg;
	char buf[1024] = {};
	for(;;)
	{
		read(sockfd,buf,sizeof(buf));
		if(strlen(buf) != 0)
		{
			printf("\n>%s\n",buf);
		}
	}
}

int main()
{
	printf("服务器创建socket...\n");
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(0 > sockfd)
	{
		perror("socket");
		return -1;
	}

	printf("准备地址...\n");
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6008);
	addr.sin_addr.s_addr = inet_addr("10.0.2.15");
	socklen_t len = sizeof(addr);

	printf("绑定连接服务器...\n");
	if(connect(sockfd,(struct sockaddr*)&addr,len))
	{
		perror("connect");
		return -1;
	}

	
	char link[50] = {};
	read(sockfd,link,sizeof(link));
//	printf("link:%s\n",link);
	if(strstr(link,"您已经成功连接")==NULL)
	{
		printf("连接人数已满，请稍后重试\n");
		return 0;
	}
	else
	{
		printf("link:%s\n",link);
	}

	// 创建读取子线程
	pthread_t pid;
	pthread_create(&pid,NULL,start_read,&sockfd);

	for(;;)
	{
		char buf[1024] = {};
		usleep(1000);
		//printf(">我说：");
		gets(buf);
		write(sockfd,buf,strlen(buf)+1);
		if(0 == strcmp("quit",buf))
		{
			printf("通信结束!\n");
			break;
		}
	}
	
	close(sockfd);
}
