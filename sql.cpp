#include <iostream>	
#include <iomanip>	
#include <mysql.h>	
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <openssl/md5.h>
#include <cstring>
using namespace std;

#include "sql.h"

#define MD5_SECRET_LEN_16     (16)
#define MD5_BYTE_STRING_LEN   (4)

#define MYSQL_USERNAME "u1651965"
#define MYSQL_PASSWORD "u1651965"
#define MYSQL_DATABASE "db1651965"

string commonMd5Secret32(const std::string& src)
{
    MD5_CTX ctx;
 
    string md5String;
    unsigned char md[MD5_SECRET_LEN_16] = { 0 };
    char tmp[MD5_BYTE_STRING_LEN] = { 0 };
 
    MD5_Init( &ctx );
    MD5_Update( &ctx, src.c_str(), src.size() );
    MD5_Final( md, &ctx );
 
    for( int i = 0; i < 16; ++i )
    {
        memset( tmp, 0x00, sizeof( tmp ) );
        snprintf( tmp,sizeof(tmp) , "%02X", md[i] );
        md5String += tmp;
    }
    return md5String;
}

int Login(User &user)
{
    MYSQL     *mysql;   
    MYSQL_RES *result;   
    MYSQL_ROW  row;
    char MD5_PW [100];//���ܺ�����
    /* ��ʼ�� mysql ������ʧ�ܷ���NULL */
    if ((mysql = mysql_init(NULL))==NULL) {
    	cout << "mysql_init failed" << endl;
    	return ERR_IN;
    	}

    /* �������ݿ⣬ʧ�ܷ���NULL
       1��mysqldû����
       2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql,"localhost",MYSQL_USERNAME, MYSQL_PASSWORD,MYSQL_DATABASE,0, NULL, 0)==NULL) {
    	cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, "gbk"); 

    /* ���в�ѯ���ɹ�����0�����ɹ���0
       1����ѯ�ַ��������﷨����
       2����ѯ�����ڵ����ݱ� */
    if (mysql_query(mysql, "select * from user")) {
    	cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* ����ѯ����洢���������ִ����򷵻�NULL
       ע�⣺��ѯ���ΪNULL�����᷵��NULL */
    if ((result = mysql_store_result(mysql))==NULL) {
    	cout << "mysql_store_result failed" << endl;
    	return ERR_IN;
    	}


    /* ѭ����ȡ�������������ļ�¼
	   1�����ص���˳����selectָ������˳����ͬ����row[0]��ʼ
	   2���������ݿ�����ʲô���ͣ�C�ж��������ַ��������д�������б�Ҫ����Ҫ�Լ�����ת��
	   3�������Լ�����Ҫ��֯�����ʽ */

   /*   row[0] id
	row[1] name
	row[2] passwd
	row[3] oldnum
	row[4] never_online    */   
    
    sprintf(MD5_PW,"%s",commonMd5Secret32(user.password).c_str());

    for(int i=0;i<strlen(MD5_PW);i++)
    {
	if(MD5_PW[i]>=65)
		MD5_PW[i]+=32;
    }

    int p_pw;//�ж������Ƿ���ȷ

    while((row=mysql_fetch_row(result))!=NULL) {

	if(strcmp(row[1],user.name)==0&&strcmp(row[2],MD5_PW)==0)
	{
		user.config.oldnum=atoi(row[3]);
		user.config.color.a=atoi(row[5]);
		user.config.color.b=atoi(row[6]);
		user.config.color.g=atoi(row[7]);
		user.config.color.r=atoi(row[8]);
		if(atoi(row[4])==1)
		{
			SetConfig(user);		
			return SUC_FL;
		}
		SetConfig(user);
		return SUC;
	}
        }
    if(IsRegistered(user)!=1)
    return ERR_NE; 
    else 
    return ERR_WP;

    /* �ͷ�result */
    mysql_free_result(result);   

    /* �ر��������� */
    mysql_close(mysql);   
	
}

int ChangePassword(User &user, const char * const password)
{
    MYSQL     *mysql;   
    MYSQL_RES *result;   
    MYSQL_ROW  row;
    /* ��ʼ�� mysql ������ʧ�ܷ���NULL */
    if ((mysql = mysql_init(NULL))==NULL) {
    	cout << "mysql_init failed" << endl;
    	return ERR_IN;
    	}

    /* �������ݿ⣬ʧ�ܷ���NULL
       1��mysqldû����
       2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql,"localhost",MYSQL_USERNAME, MYSQL_PASSWORD,MYSQL_DATABASE,0, NULL, 0)==NULL) {
    	cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, "gbk"); 
    
    char buf[1000];

    if(strlen(password)<6||strlen(password)>32)
	return ERR_IP;

    if(IsRegistered(user)!=1)
	return ERR_NE;

    sprintf(buf,"update user set passwd=MD5(%s) where name='%s' ",password,user.name);

    if(mysql_query(mysql, buf))
        return ERR_IN;
    else
    {
	sprintf(buf,"update user set never_online=0 where name='%s'",user.name);
	mysql_query(mysql, buf);
	strcpy(user.password,password);
	return SUC;
    }
}

int SetConfig(User &user)
{
    MYSQL     *mysql;   
    MYSQL_RES *result;   
    MYSQL_ROW  row;
    char MD5_PW [100];//���ܺ�����
    /* ��ʼ�� mysql ������ʧ�ܷ���NULL */
    if ((mysql = mysql_init(NULL))==NULL) {
    	cout << "mysql_init failed" << endl;
    	return ERR_IN;
    	}

    /* �������ݿ⣬ʧ�ܷ���NULL
       1��mysqldû����
       2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql,"localhost",MYSQL_USERNAME, MYSQL_PASSWORD,MYSQL_DATABASE,0, NULL, 0)==NULL) {
    	cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, "gbk"); 

    /* ���в�ѯ���ɹ�����0�����ɹ���0
       1����ѯ�ַ��������﷨����
       2����ѯ�����ڵ����ݱ� */
    if (mysql_query(mysql, "select * from user")) {
    	cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* ����ѯ����洢���������ִ����򷵻�NULL
       ע�⣺��ѯ���ΪNULL�����᷵��NULL */
    if ((result = mysql_store_result(mysql))==NULL) {
    	cout << "mysql_store_result failed" << endl;
    	return ERR_IN;
    	}


    /* ѭ����ȡ�������������ļ�¼
	   1�����ص���˳����selectָ������˳����ͬ����row[0]��ʼ
	   2���������ݿ�����ʲô���ͣ�C�ж��������ַ��������д�������б�Ҫ����Ҫ�Լ�����ת��
	   3�������Լ�����Ҫ��֯�����ʽ */

   /*   row[0] id
	row[1] name
	row[2] passwd
	row[3] oldnum
	row[4] never_online    */   


    if(IsRegistered(user)!=1)
	return ERR_NE;


    char buf[1000];
    sprintf(MD5_PW,"%s",commonMd5Secret32(user.password).c_str());
    for(int i=0;i<strlen(MD5_PW);i++)
    {
	if(MD5_PW[i]>=65)
		MD5_PW[i]+=32;
    } 

    
    while((row=mysql_fetch_row(result))!=NULL) {

	if(strcmp(row[1],user.name)==0&&strcmp(row[2],MD5_PW)==0)
	{
		if(user.config.oldnum<0||int(user.config.color.a)<0||int(user.config.color.a)>255||int(user.config.color.b)<0||int(user.config.color.b)>255||
		int(user.config.color.g)<0||int(user.config.color.g)>255||int(user.config.color.r)<0||int(user.config.color.r)>255)
			return ERR_IC;
		

		sprintf(buf,"update user set oldnum=%d where name='%s'",user.config.oldnum,user.name);
		mysql_query(mysql, buf);
		sprintf(buf,"update user set a=%d where name='%s'",user.config.color.a,user.name);
		mysql_query(mysql, buf);
		sprintf(buf,"update user set r=%d where name='%s'",user.config.color.r,user.name);
		mysql_query(mysql, buf);
		sprintf(buf,"update user set g=%d where name='%s'",user.config.color.g,user.name);
		mysql_query(mysql, buf);
		sprintf(buf,"update user set b=%d where name='%s'",user.config.color.b,user.name);
		mysql_query(mysql, buf);
		return SUC;
	}
        }
}

int StoreMessage(const User &sender, const vector<User> &receivers,const Message &message)
{
    MYSQL     *mysql;   
    MYSQL_RES *result;   
    MYSQL_ROW  row;
	

    int s_id,r_id[100];    


    /* ��ʼ�� mysql ������ʧ�ܷ���NULL */
    if ((mysql = mysql_init(NULL))==NULL) {
    	cout << "mysql_init failed" << endl;
    	return ERR_IN;
    	}

    /* �������ݿ⣬ʧ�ܷ���NULL
       1��mysqldû����
       2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql,"localhost",MYSQL_USERNAME, MYSQL_PASSWORD,MYSQL_DATABASE,0, NULL, 0)==NULL) {
    	cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, "gbk"); 


    
    if (mysql_query(mysql, "select * from user")) {
    	cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* ����ѯ����洢���������ִ����򷵻�NULL
       ע�⣺��ѯ���ΪNULL�����᷵��NULL */
    if ((result = mysql_store_result(mysql))==NULL) {
    	cout << "mysql_store_result failed" << endl;
    	return ERR_IN;
    	}


    /* ѭ����ȡ�������������ļ�¼
	   1�����ص���˳����selectָ������˳����ͬ����row[0]��ʼ
	   2���������ݿ�����ʲô���ͣ�C�ж��������ַ��������д�������б�Ҫ����Ҫ�Լ�����ת��
	   3�������Լ�����Ҫ��֯�����ʽ */

   /*   row[0] id
	row[1] name
	row[2] passwd
	row[3] oldnum
	row[4] never_online    */   

    int ps_name=0;

    while((row=mysql_fetch_row(result))!=NULL) {
	if(strcmp(row[1],sender.name)==0)ps_name=1;

	if(strcmp(row[1],sender.name)==0)
	{
		s_id=atoi(row[0]);
		cout<<"s_id"<<s_id<<endl;
	}
	for(int i=0;i<receivers.size();i++)
	{  
		
		//else pr_name=0;
		if(strcmp(row[1],receivers[i].name)==0)
		{
			r_id[i]=atoi(row[0]);
			cout<<"r_id[i]"<<r_id[i]<<endl;
		}
	}
        }

    if(ps_name!=1)
	return ERR_IN;
    for(int i=0;i<receivers.size();i++)
    	if(IsRegistered(receivers[i])!=1)     
		return ERR_NE;

    /* �ͷ�result */
    mysql_free_result(result);   

    /* �ر��������� */
    mysql_close(mysql);   


   /* ��ʼ�� mysql ������ʧ�ܷ���NULL */
    if ((mysql = mysql_init(NULL))==NULL) {
    	cout << "mysql_init failed" << endl;
    	return ERR_IN;
    	}

    /* �������ݿ⣬ʧ�ܷ���NULL
       1��mysqldû����
       2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql,"localhost",MYSQL_USERNAME, MYSQL_PASSWORD,MYSQL_DATABASE,0, NULL, 0)==NULL) {
    	cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, "gbk"); 

    if (mysql_query(mysql, "select * from message")) {
    	cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* ����ѯ����洢���������ִ����򷵻�NULL
       ע�⣺��ѯ���ΪNULL�����᷵��NULL */
    if ((result = mysql_store_result(mysql))==NULL) {
    	cout << "mysql_store_result failed" << endl;
    	return ERR_IN;
    	}

   /*   row[0] sender_id
	row[1] receiver_id
	row[2] time
	row[3] text     */   
    char buf[1000];

    time_t curtime = time(NULL);
    tm *ptm = localtime(&curtime);
    char gettime[64];
    sprintf(gettime, "%d/%02d/%02d %02d:%02d:%02d", ptm->tm_year+1900, ptm->tm_mon+1,
    ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    
	for(int i=0;i<receivers.size();i++)
	{
		sprintf(buf,"insert into message values('%d','%d','%s','%s')",s_id,r_id[i],gettime,message.text);
		mysql_query(mysql, buf);
	}
    return SUC;
    /* �ͷ�result */
    mysql_free_result(result);   

    /* �ر��������� */
    mysql_close(mysql);   
}

int GetOldMessages(const User &user,vector<OldMessage> &messages)
{
    MYSQL     *mysql,*mysql2;   
    MYSQL_RES *result,*result2;   
    MYSQL_ROW  row,row2;
    char id_name[100][100];
    OldMessage m1;
    messages.push_back(m1);

    int id;    
    char buf[1000];

    if ((mysql = mysql_init(NULL))==NULL) {
    	cout << "mysql_init failed" << endl;
    	return ERR_IN;
    	}
    /* ��ʼ�� mysql ������ʧ�ܷ���NULL */
    if ((mysql2 = mysql_init(NULL))==NULL) {
    	cout << "mysql_init failed" << endl;
    	return ERR_IN;
    	}

    /* �������ݿ⣬ʧ�ܷ���NULL
       1��mysqldû����
       2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql2,"localhost",MYSQL_USERNAME, MYSQL_PASSWORD,MYSQL_DATABASE,0, NULL, 0)==NULL) {
    	cout << "mysql_real_connect failed(" << mysql_error(mysql2) << ")" << endl;
    	return ERR_IN;
    	}

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql2, "gbk"); 

    sprintf(buf,"select * from user");  
    
    if (mysql_query(mysql2, buf)) {
    	cout << "mysql_query failed(" << mysql_error(mysql2) << ")" << endl;
    	return ERR_IN;
    	}

    /* ����ѯ����洢���������ִ����򷵻�NULL
       ע�⣺��ѯ���ΪNULL�����᷵��NULL */
    if ((result2 = mysql_store_result(mysql2))==NULL) {
    	cout << "mysql_store_result failed" << endl;
    	return ERR_IN;
    	}


    /* ѭ����ȡ�������������ļ�¼
	   1�����ص���˳����selectָ������˳����ͬ����row[0]��ʼ
	   2���������ݿ�����ʲô���ͣ�C�ж��������ַ��������д�������б�Ҫ����Ҫ�Լ�����ת��
	   3�������Լ�����Ҫ��֯�����ʽ */





	int i=0;
	char text[1000];
	

    while((row2=mysql_fetch_row(result2))!=NULL ) {   
	if(strcmp(user.name,row2[1])==0)
		id=atoi(row2[0]);  
	strcpy(id_name[i],row2[1]);
	i++;
        }
    /* �ͷ�result */
    mysql_free_result(result2);   













    /* ��ʼ�� mysql ������ʧ�ܷ���NULL */



    /* �������ݿ⣬ʧ�ܷ���NULL
       1��mysqldû����
       2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql,"localhost",MYSQL_USERNAME, MYSQL_PASSWORD,MYSQL_DATABASE,0, NULL, 0)==NULL) {
    	cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

	 

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, "gbk"); 











   sprintf(buf,"select * from message where receiver_id=%d or sender_id=%d order by time desc",id,id);  
    
    if (mysql_query(mysql, buf)) {
    	cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* ����ѯ����洢���������ִ����򷵻�NULL
       ע�⣺��ѯ���ΪNULL�����᷵��NULL */
    if ((result = mysql_store_result(mysql))==NULL) {
    	cout << "mysql_store_result failed" << endl;
    	return ERR_IN;
    	}


    /* ѭ����ȡ�������������ļ�¼
	   1�����ص���˳����selectָ������˳����ͬ����row[0]��ʼ
	   2���������ݿ�����ʲô���ͣ�C�ж��������ַ��������д�������б�Ҫ����Ҫ�Լ�����ת��
	   3�������Լ�����Ҫ��֯�����ʽ */


   /*   row[0] sender_id
	row[1] receiver_id
	row[2] time
	row[3] text     */   
    i=0;

    while((row=mysql_fetch_row(result))!=NULL && i<user.config.oldnum-1) {  
	if(atoi(row[0])==id)
	{
	strcpy(messages[i].message.text,"me");
	strcat(messages[i].message.text,":");
	strcat(messages[i].message.text,row[3]);
	strcpy(messages[i].timestamp,row[2]);
	}
	else if(atoi(row[1])==id)
	{
	strcpy(messages[i].message.text,id_name[atoi(row[0])-1]);
	strcat(messages[i].message.text,":");
	strcat(messages[i].message.text,row[3]);
	strcpy(messages[i].timestamp,row[2]);
	}
	messages.push_back(messages[i]);
	i++;
        }

    /* �ͷ�result */
    mysql_free_result(result); 

    /* �ر��������� */
    mysql_close(mysql);   
    return SUC;
}

int IsRegistered(const User &user)
{
    MYSQL     *mysql;   
    MYSQL_RES *result;   
    MYSQL_ROW  row;
	
    int id=1;    
    char buf[1000];

    /* ��ʼ�� mysql ������ʧ�ܷ���NULL */
    if ((mysql = mysql_init(NULL))==NULL) {
    	cout << "mysql_init failed" << endl;
    	return ERR_IN;
    	}

    /* �������ݿ⣬ʧ�ܷ���NULL
       1��mysqldû����
       2��û��ָ�����Ƶ����ݿ���� */
    if (mysql_real_connect(mysql,"localhost",MYSQL_USERNAME, MYSQL_PASSWORD,MYSQL_DATABASE,0, NULL, 0)==NULL) {
    	cout << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* �����ַ���������������ַ����룬��ʹ/etc/my.cnf������Ҳ���� */
    mysql_set_character_set(mysql, "gbk"); 

    sprintf(buf,"select * from user",id,id);  
    
    if (mysql_query(mysql, buf)) {
    	cout << "mysql_query failed(" << mysql_error(mysql) << ")" << endl;
    	return ERR_IN;
    	}

    /* ����ѯ����洢���������ִ����򷵻�NULL
       ע�⣺��ѯ���ΪNULL�����᷵��NULL */
    if ((result = mysql_store_result(mysql))==NULL) {
    	cout << "mysql_store_result failed" << endl;
    	return ERR_IN;
    	}
    

      /*   row[0] id
	row[1] name
	row[2] passwd
	row[3] oldnum
	row[4] never_online    */   
    

    int p_name=0;//�ж��Ƿ�����û�


    while((row=mysql_fetch_row(result))!=NULL) {
	if(strcmp(row[1],user.name)==0)p_name=1;
        }
    if(p_name!=1)
    return ERR_NE; 
    else 
    return SUC;
}
