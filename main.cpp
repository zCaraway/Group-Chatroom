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
/******************** ������ж� ********************/
    if(argc<2)
    {
        printf("���������Ŀ���㣡\n");
        return -1;
    }

/******************** ���ػ����� ********************/
    pid_t pid;

    //��һ��fork
    if((pid=fork())<0)
	{
		perror("fork");
		exit(1);
	}
	else if(pid>0)
		exit(0);
	
    //�ڶ���fork
	if ((pid = fork()) > 0)
        exit(0);

	//chdir("/");
	umask(0);
    
/******************** ׼���׽��� ********************/
    int listenfd;
    short port=atoi(argv[1]);   //����Ķ˿ںţ�������
    struct sockaddr_in my_addr;	//������ַ��Ϣ�ṹ��
	int reuseaddr=1;			//�˿�����
    int nonblock=1;

	//�����������׽���	
	if((listenfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		printf("�׽��ִ���ʧ�ܣ�%d\n",errno);
		return -1;
	}
	if(ioctl(listenfd, FIONBIO, &nonblock)==-1)
	{
		printf("�趨����/������ʧ�ܣ�%d\n",errno);
		close(listenfd);
		return -1;
	}
	if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void*)&reuseaddr,sizeof(int))==-1)
	{
		printf("�����׽��ֶ˿�����ʧ�ܣ�%d\n",errno);
		close(listenfd);
		return -1;
	}
	//���÷���˵�ַ�ṹ��
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(port);
	my_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	bzero(&(my_addr.sin_zero),8);
	//�󶨵�ַ�ṹ���socket
	if(bind(listenfd,(struct sockaddr*)&my_addr,sizeof(struct sockaddr))<0)
	{
		printf("�׽��ְ�ʧ�ܣ�%d\n",errno);
		close(listenfd);
		return -1;
	}

/******************** ���������� ********************/
    listen(listenfd,BACKLOG);	//��������
    printf("��ʼ����%d�˿�...\n",port);

    ProcessClients(listenfd);

    close(listenfd);
    return 0;
}