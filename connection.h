#ifndef CONNECTION_H
#define CONNECTION_H

#include "common.h"

//服务端可能收到的包的主类型
#define LRQ 0x00    //LOGIN_QUEST
#define NP 0x10     //NEW_PASSWORD
#define NC 0x20     //NEW_CONFIG
#define SM 0x30     //SEND_MESSAGE
#define PF 0x40     //PREPARE_FILE
#define PTR 0x41    //PREPARE_TRANSMIT_FILE
#define SF 0x42     //SEND_FILE
#define SSF 0x43    //STOP_SEND_FILE

//服务端可能收到的包的子类型
#define PTR_OK 0x00 //OK
#define PTR_NO 0xFF //NO

//服务端可能发送的包的主类型
#define LRP 0x80    //LOGIN_REPLY
#define KK 0x81     //KICK
#define FL 0x90     //FIRST_LOGIN
#define NPR 0x91    //NEW_PASSWORD_RESULT
#define CC 0xA0     //CURRENT_CONFIG
#define NCR 0xA1    //NEW_CONFIG_RESULT
#define TM 0xB0     //TRANSMIT_MESSAGE
#define TMR 0xB1    //TRANSMIT_MESSAGE_RESULT
#define OMO 0xB2    //OLD_MESSAGE_OTHERS
#define OMS 0xB3    //OLD_MESSAGE_SELF
#define PFR 0xC0    //PREPARE_FILE_REPLY
#define PT 0xC1     //PREPARE_TRANSMIT_FILE
#define TF 0xC2     //TRANSMIT_FILE
#define STF 0xC3    //STOP_TRANSMIT_FILE
#define SFR 0xC4    //SEND_FILE_REPLY
#define NOS 0xD0    //NEW_ONLINE_STATE

//服务端可能发送的包的子类型
#define LRP_NC 0x00 //NO_CONFLICT
#define LRP_KK 0x01 //KICK
#define LRP_NE 0xFF //NOT_EXIST
#define LRP_WP 0xFE //WRONG_PASSWORD
#define LRP_IP 0xFD //ILLEGAL_PASSWORD
#define LRP_IE 0xFC //INTERNAL_ERROR

#define NPR_SU 0x00 //SUCCESS
#define NPR_FA 0xFF //FAIL
#define NPR_IE 0xFE //INTERNAL_ERROR

#define NCR_SU 0x00 //SUCCESS
#define NCR_FA 0xFF //FAIL
#define NCR_IE 0xFE //INTERNAL_ERROR

#define TMR_SU 0x00 //SUCCESS
#define TMR_SP 0x01 //SUCCESS_PARTLY
#define TMR_SO 0x02 //SUCCESS_OFFLINE
#define TMR_WS 0xFF //WRONG_SYNTAX
#define TMR_NE 0xFE //NOT_EXIST
#define TMR_NO 0xFD //NOT_ONLINE
#define TMR_AF 0xFC //ALL_FAIL
#define TMR_IE 0xFB //INTERNAL_ERROR

#define PFR_CO 0X00 //CORRECT
#define PFR_NE 0xFF //NOT_EXIST
#define PFR_NO 0xFE //NOT_ONLINE
#define PFR_IE 0xFD //INTERNAL_ERROR
#define PFR_DE 0xFC //DENIED

#define SFR_SU 0X00 //SUCCESS
#define SFR_FA 0xFF //FAIL

#define NOS_ON 0x00 //ONLINE
#define NOS_OF 0xFF //OFFLINE

//连接的状态
enum State
{
    LOGIN,      //等待登录
    FORCEPW,    //等待强制改密的密码
    ONLINE,     //正常登录状态
    SFILE,      //发送文件（仅发送方）
};

//与客户端的连接
struct Connection
{
    int accfd;
    State state;
    User user;
    union
    {
        User sender;
        User receiver;
    }other;
};

//数据包
struct Package
{
    //0x80被当成int型了，应该用unsigned char
    unsigned char type;
    unsigned char subtype;
    unsigned short length;   //记得与网络序转换
    char content[MAX_CONTENT];
};

#endif