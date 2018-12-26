#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>

#define MAX_CONTENT 1024

#define LOG_PATHNAME "./server.log"

//注意四字节对齐
//可以定义显式的char pad[x];(x=1,2,3)

//客户端颜色
struct Color
{
    unsigned char r,g,b,a;
};

//客户端设置
struct Config   //不使用int，不需要考虑字节序
{
    int oldnum; //显示旧消息的数目
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

//客户端
struct User
{
    char name[29];      //有尾零，用于标识结尾
    char password[33];  //有尾零，用于标识结尾
    Config config;
};

//转发的消息
struct Message
{
    char text[MAX_CONTENT];
};

//旧消息
struct OldMessage
{
    char timestamp[20];
    Message message;
};

void Log(const char *const str);

#endif