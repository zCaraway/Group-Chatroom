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

//��user.name��user.password�������ݿ�ƥ�䣬������������user.config
//�ɹ�������SUC
//�ɹ��������ǵ�һ�ε�¼������SUC_FL
//�޴��û�������ERR_NE
//������󣺷���ERR_WP
int Login(User &user);

//����user��Ӧ�û�������
//�ɹ�������SUC
//�޴��û�������ERR_NE
//���벻�Ϸ�������ERR_IP
int ChangePassword(User &user, const char *const password);

//����user.name��user.config�������ݿ�
//�ɹ�������SUC
//�޴��û�������ERR_NE
//���ò��Ϸ�������ERR_IC
int SetConfig(User &user);

//�����͵���Ϣ������ݿ�
//ע�⣬@all���͵���Ϣ���Ǹ�����[����]�û���
//�ɹ�������SUC
//���շ��û������ڣ�����ERR_NE
//���ͷ������ڣ�����ERR_IN
int StoreMessage(const User &sender, const vector<User> &receivers, const Message &message);

//����user.name��ȡ���û����ġ����û��յ�����ǰuser.config.oldnum����Ϣ������ʱ��˳��push_back��messages
//�ɹ�������SUC
int GetOldMessages(const User &user, vector<OldMessage> &messages);

//����user.name�ж��Ƿ���ע��
//��ע�᣺����SUC
//δע�᣺����ERR_NE
int IsRegistered(const User &user);

#endif