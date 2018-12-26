#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace std;

#define BACKLOG 5

void ProcessClients(int listenfd);
int main(int argc, char *argv[])
{
/******************** 输入的判断 ********************/
    if(argc<2)
    {
        printf("输入参数数目不足！\n");
        return -1;
    }

/******************** 创守护进程 ********************/
    pid_t pid;

    //第一次fork
    if((pid=fork())<0)
	{
		perror("fork");
		exit(1);
	}
	else if(pid>0)
		exit(0);
	
    //第二次fork
	if ((pid = fork()) > 0)
        exit(0);

	//chdir("/");
	umask(0);
    
/******************** 准备套接字 ********************/
    int listenfd;
    short port=atoi(argv[1]);   //输入的端口号（主机序）
    struct sockaddr_in my_addr;	//本方地址信息结构体
	int reuseaddr=1;			//端口重用
    int nonblock=1;

	//创建和设置套接字	
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		printf("套接字创建失败：%d\n",errno);
		return -1;
	}
	if(ioctl(listenfd, FIONBIO, &nonblock)==-1)
	{
		printf("设定阻塞/非阻塞失败：%d\n",errno);
		close(listenfd);
		return -1;
	}
	if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&reuseaddr,sizeof(int))==-1)
	{
		printf("设置套接字端口重用失败：%d\n",errno);
		close(listenfd);
		return -1;
	}
	//设置服务端地址结构体
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(port);
	my_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	bzero(&(my_addr.sin_zero),8);
	//绑定地址结构体和socket
	if(bind(listenfd,(struct sockaddr*)&my_addr,sizeof(struct sockaddr))<0)
	{
		printf("套接字绑定失败：%d\n",errno);
		close(listenfd);
		return -1;
	}

/******************** 监听与连接 ********************/
    listen(listenfd,BACKLOG);	//开启监听
    printf("开始监听%d端口...\n",port);

    ProcessClients(listenfd);

    close(listenfd);
    return 0;
}
