#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>

#define MAX_CONTENT 1024

#define LOG_PATHNAME "./server.log"

//ע�����ֽڶ���
//���Զ�����ʽ��char pad[x];(x=1,2,3)

//�ͻ�����ɫ
struct Color
{
    unsigned char r,g,b,a;
};

//�ͻ�������
struct Config   //��ʹ��int������Ҫ�����ֽ���
{
    int oldnum; //��ʾ����Ϣ����Ŀ
    Color color;
    
    void hton()
    {
        oldnum=htonl(oldnum);
    }
    void ntoh()
    {
        oldnum=ntohl(oldnum);
    }
};

//�ͻ���
struct User
{
    char name[29];      //��β�㣬���ڱ�ʶ��β
    char password[33];  //��β�㣬���ڱ�ʶ��β
    Config config;
};

//ת������Ϣ
struct Message
{
    char text[MAX_CONTENT];
};

//����Ϣ
struct OldMessage
{
    char timestamp[20];
    Message message;
};

void Log(const char *const str);

#endif