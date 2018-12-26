#ifndef SQL_H
#define SQL_H

#include <vector>
#include "common.h"

#define SUC_FL  2   //first login
#define SUC     1
#define ERR_IN  0   //internal error
#define ERR_NE  -1  //user not exist
#define ERR_WP  -2  //wrong password
#define ERR_IP  -3  //illegal password
#define ERR_IC  -4  //illegal config

//用user.name和user.password进行数据库匹配，并将设置填入user.config
//成功：返回SUC
//成功，但是是第一次登录：返回SUC_FL
//无此用户：返回ERR_NE
//密码错误：返回ERR_WP
int Login(User &user);

//更改user对应用户的密码
//成功：返回SUC
//无此用户：返回ERR_NE
//密码不合法：返回ERR_IP
int ChangePassword(User &user, const char *const password);

//根据user.name把user.config存入数据库
//成功：返回SUC
//无此用户：返回ERR_NE
//设置不合法：返回ERR_IC
int SetConfig(User &user);

//将发送的消息存回数据库
//注意，@all发送的消息，是给所有[在线]用户的
//成功：返回SUC
//接收方用户不存在：返回ERR_NE
//发送方不存在：返回ERR_IN
int StoreMessage(const User &sender, const vector<User> &receivers, const Message &message);

//根据user.name获取此用户发的、此用户收到过的前user.config.oldnum条消息，并按时间顺序push_back进messages
//成功：返回SUC
int GetOldMessages(const User &user, vector<OldMessage> &messages);

//根据user.name判断是否已注册
//已注册：返回SUC
//未注册：返回ERR_NE
int IsRegistered(const User &user);

#endif