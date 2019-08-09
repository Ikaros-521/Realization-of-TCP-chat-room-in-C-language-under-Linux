#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

struct sockaddr_in addr = {};
int clifd_index = 0; // clifd的下标
char buf[1024] = {}; // 存储客户端发来的字符串
char str[1024] = {}; // 存储带clifd的回传信息
int clifd[30] = {}; // 存储clifd
int online_num = 0;	// 连接人数
int max_num = 10; // 最大人数

// 项目有bug，连接人数的限制控制不住，有待改进

void* start_read(void *arg) // 读取信息的子线程
{
//	printf("arg:%d\n",*(int*)arg);
	int clifd1 = *(int*)arg;
	printf("run_clifd:%d\n",clifd1);

	for(;;)
	{
//		printf("before read\n");

		int ret = read(clifd1,buf,sizeof(buf));
		printf("\nip:%s,port:%hu,size:%d\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),ret); // 获取相关信息
		printf("say:%s\n",buf);
		
		char id[10] = {};
		sprintf(id,"%d说:",clifd1);

		if(strlen(buf) != 0)
		{
			strcpy(str,id);
			strcat(str,buf);
		} // 存入str中

		if(0 == strcmp("quit",buf)) // 如果收到quit
		{
			online_num--; // 在线人数-1
			for(int i=0; i<clifd_index; i++)
			{
				if(clifd1 == clifd[i])
				{
					int *die = &clifd1;
					clifd[i] = 0;
					pthread_exit(die); // 终止线程
					break;
				}
			}
		}
		//usleep(1000);
	}
}

void* start_write(void *arg) // 写回的子线程
{
//	printf("arg:%d\n",*(int*)arg);

//	usleep(500);

	int clifd1 = *(int*)arg;

	printf("run_clifd:%d\n",clifd1);

	for(;;)
	{
		int flag = 0;
		for(int i=0; i<clifd_index; i++) // 因为读到quit的原因，clifd被置0
		{
			if(clifd1 == clifd[i])
			{
				break;
			}
			if(i == clifd_index-1)
			{
				int *die = &clifd1;
				flag = 1;
				pthread_exit(die); // 终止此写回的子线程
			}
		}
		if(flag == 1)
		{
			break;
		}

		if(strlen(str) == 0) // 空消息不写入
			continue;
		printf("before write\n");
		printf("str:%s\n",str);
		write(clifd1,str,strlen(str)+1);
		usleep(50000); // 最快的子线程等待其他子线程
		memset(str,0,1024); // 清空str
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
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6008);
	addr.sin_addr.s_addr = inet_addr("10.0.2.15");
	socklen_t len = sizeof(addr);

	printf("绑定socket与地址...\n");
	if(bind(sockfd,(struct sockaddr*)&addr,len))
	{
		perror("bind");
		return -1;
	}

	printf("设置监听...\n");
	if(listen(sockfd,2))
	{
		perror("listen");
		return -1;
	}


	printf("等待客户端连接...\n");
	for(;;)
	{
		if(online_num < max_num)
		{
			struct sockaddr_in addrcli = {};
			clifd[clifd_index] = accept(sockfd,(struct sockaddr*)&addrcli,&len);
		
			int flag = 0;
			for(int i=0; i<clifd_index; i++)
			{
				if(clifd[clifd_index] == clifd[i])
				{
					flag = 1;
					break;
				}
			}

			if(flag == 1)
			{
				clifd_index--;
				continue;
			}
			else
			{
				char link[50] = {};
				char link1[40] = "您已经成功连接";
				sprintf(link,"您的id是:%d,",clifd[clifd_index]);
				strcat(link,link1);
				write(clifd[clifd_index],link,strlen(link)+1);
				online_num++;
			}
		}
		else
		{
			continue;
		}


		if(0 > clifd[clifd_index])
		{
			perror("accept");
			continue;
		}

		printf("clifd:%d\n",clifd[clifd_index]);

		// 创建子线程
		pthread_t pid1,pid2;
		pthread_create(&pid1,NULL,start_read,&clifd[clifd_index]);
		pthread_create(&pid2,NULL,start_write,&clifd[clifd_index]);

		usleep(1000);

//		printf("clifd:%d\n",clifd[index]);

		clifd_index++; // 下标逐渐+1，这样写不是很合适

	}
	return 0;
}
