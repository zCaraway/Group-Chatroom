#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>
#include <cstdio>
#include <list>
#include <cstring>
#include <unistd.h>
using namespace std;

#include "connection.h"
#include "sql.h"

list<Connection> conl;  //connection list（使用链表，插入删除效率更高）

//造包并发包
void MakeSendPackage(list<Connection>::iterator &iter,unsigned char type,unsigned char subtype,unsigned short length,const char *content)
{
    Package p;
    p.type=type;
    p.subtype=subtype;
    p.length=htons(length);
    if(content)
        memcpy(p.content,content,length-4);
    int writelen=write(iter->accfd,(char *)&p,length>4+MAX_CONTENT?4+MAX_CONTENT:length);
    printf("向fd%d发送类型为%02X/%02X的%dB报文：",iter->accfd,type,subtype,length);
    if(writelen==-1)
    {
        printf("写入错误：%d\n",errno);
    }
    else
    {
        printf("写入%dB\n",writelen);
    }
}

//断开连接
//客户端主动下线时直接调用EndConnection
//如果是服务端主动关闭连接，执行特定操作后再调用EndConnection
void EndConnection(list<Connection>::iterator &iter)
{
    State s=iter->state;
    iter=conl.erase(iter);
    --iter; //结束连接后进入下一次循环，会++iter，所以要先--抵消掉

    //如果断连的是一个在线用户，给其他所有在线用户发送在线更新报文
    if(s==ONLINE||s==SFILE)
    {
        list<Connection>::iterator it;
        State ss;
        for(it=conl.begin();it!=conl.end();++it)
        {
            ss=it->state;
            if(ss==LOGIN||ss==FORCEPW)    //不是在线状态的连接不用发在线更新报文
                continue;
            MakeSendPackage(it,NOS,NOS_OF,4,NULL);
        }
    }
}

//上线函数
//如果有同名登录，踢下线
//发送对应的LRP（无冲突或有冲突登录）
//发送当前在线用户信息
//发送当前用户设置
//发送过去的消息
//将上线信息发送给其他在线用户
void GoOnline(list<Connection>::iterator &iter)
{
	list<Connection>::iterator it;
    char buffer[1024];
    /********** 踢同名登录 **********/
    bool kick=false;
	for (it = conl.begin(); it != conl.end(); ++it)
	{
		if (it != iter)
		{
			if (!strcmp(iter->user.name, it->user.name) && it->state != LOGIN && it->state != FORCEPW)  //有同名(只可能有1个)
			{
				MakeSendPackage(it, KK, 0x00, 4, NULL);//向被踢者发送
				EndConnection(it);//强制下线
				kick=true;
                break;
			}
		}
	}
    if(kick)
    {
        sprintf(buffer,"另一个终端登录的用户%s被强制下线\n",iter->user.name);
        Log(buffer);
    }
    if(1)
        printf("上线函数-踢同名登录完成:%s\n",kick?"kicked":"not kicked");
    /********** 给本人的在线更新 **********/
    //上线回复
    //if(it==conl.end())    //不能如此判断，因为如果踢掉最后一个连接，it就会指向conl.end()
    MakeSendPackage(iter, LRP, kick?LRP_KK:LRP_NC, 4, NULL);
    //发送所有用户的在线状态
    for(it=conl.begin();it!=conl.end();++it)
    {
        if(it==iter||it->state==LOGIN||it->state==FORCEPW)
            continue;
        MakeSendPackage(iter,NOS,NOS_ON,32,it->user.name);
    }
    if(1)
        printf("上线函数-给本人的在线更新完成:%d个其他在线用户\n",conl.size()-1);
    /********** 发送当前用户设置 **********/
	Config c = iter->user.config;
	c.hton();
	MakeSendPackage(iter, CC, 0x00, 4 + sizeof(Config), (char *)&c);
    if(1)
        printf("上线函数-发送当前用户设置完成:%d,%02X%02X%02X%02X\n",
                iter->user.config.oldnum,
                iter->user.config.color.r,
                iter->user.config.color.g,
                iter->user.config.color.b,
                iter->user.config.color.a);
    /********** 发送历史消息 **********/

    /*
    //发送消息和接收消息分别按时间排序，并不合逻辑
	vector<OldMessage> sm,rm;

	GetOldMessages(iter->user,sm,rm);
    int i;
    Message m;
	for (i=sm.size()-1;i>=0;--i)
	{
        strcpy(m.text,sm[i].timestamp);
        strcpy(m.text+20,"Me:");
        strcat(m.text+20,sm[i].message.text);
		MakeSendPackage(iter, OMS, 0x00, 4+20+strlen(m.text)+1,(char *)&m);
	}
    for (i=rm.size();i>=0;--i)
	{
        strcpy(m.text,rm[i].timestamp);
        strcpy(m.text+20,rm[i].message.text);
		MakeSendPackage(iter, OMO, 0x00, 4+20+strlen(m.text)+1,(char *)&m);
	}
    */
    
    vector<OldMessage> om;
    Message m;
    int i;
    
    GetOldMessages(iter->user,om);
    
    int num=iter->user.config.oldnum>om.size()?om.size():iter->user.config.oldnum;

    for(i=num-1;i>=0;--i)
    {
        strcpy(m.text,om[i].timestamp);
        strcpy(m.text+20,om[i].message.text);
        if(!memcmp(om[i].message.text,"Me",2))
            MakeSendPackage(iter, OMS, 0x00, 4+20+strlen(m.text)+1,(char *)&m);
        else
            MakeSendPackage(iter, OMO, 0x00, 4+20+strlen(m.text)+1,(char *)&m);
    }
    
    if(1)
        printf("上线函数-发送历史消息完成:%d条\n",num);
    /********** 给别人的用户状态更新 **********/
    for (it = conl.begin(); it != conl.end(); ++it)
	{
		if (it==iter || it->state == LOGIN || it->state == FORCEPW) //没有上线完成的不用发
			continue;
		MakeSendPackage(it, NOS, NOS_ON, 32, iter->user.name);
	}
    if(1)
        printf("上线函数-给其他用户在线更新完成\n");
	
    iter->state = ONLINE;
}

//四个状态收包时的处理函数（仅接受可能类型的包，其它包丢弃）

void PLogin(list<Connection>::iterator &iter,const Package &pack)
{
    if(pack.type==LRQ)
    {
        memcpy(iter->user.name,pack.content,28);
        iter->user.name[28]=0;
        memcpy(iter->user.password,pack.content+28,32);
        iter->user.password[32]=0;
        int result=Login(iter->user);
        char buffer[1024];
        if(1)
            printf("处理LRQ报文：username=%s,password=%s\n",iter->user.name,iter->user.password);
        if(result==SUC)
        {
            GoOnline(iter);
            sprintf(buffer,"用户%s登陆成功\n",iter->user.name);
            Log(buffer);
        }
        else if(result==SUC_FL)
        {
            MakeSendPackage(iter,FL,0x00,4,NULL);
            iter->state=FORCEPW;
            sprintf(buffer,"用户%s第一次登录，要求改密\n",iter->user.name);
            Log(buffer);
        }
        else if(result==ERR_NE)
        {
            MakeSendPackage(iter,LRP,LRP_NE,4,NULL);
            sprintf(buffer,"请求登录的用户%s不存在\n",iter->user.name);
            Log(buffer);
            EndConnection(iter);
        }
        else if(result==ERR_WP)
        {
            MakeSendPackage(iter,LRP,LRP_WP,4,NULL);
            sprintf(buffer,"用户%s密码错误:%s\n",iter->user.name,iter->user.password);
            Log(buffer);
            EndConnection(iter);
        }
        else if(result==ERR_IN)
        {
            //内部错误
            sprintf(buffer,"用户%s登录时服务器内部错误\n",iter->user.name);
            Log(buffer);
            EndConnection(iter);
        }
    }
}

void PForcepw(list<Connection>::iterator &iter,const Package &pack)
{
    if(pack.type==NP)
    {
        int result=ChangePassword(iter->user,pack.content);
        if(result==SUC)
        {
            GoOnline(iter);
        }
        else if(result==ERR_NE)
        {
            //内部错误：刚刚通过验证的用户不存在
            EndConnection(iter);
        }
        else if(result==ERR_IP)
        {
            MakeSendPackage(iter,LRP,LRP_IP,4,NULL);
            EndConnection(iter);
        }
    }
}

void POnline(list<Connection>::iterator &iter, const Package &pack)
{
    char buffer[1024];
	if (pack.type == NP)//修改密码
	{
		int result = ChangePassword(iter->user, pack.content);
		if (result == SUC)
		{
			MakeSendPackage(iter, NPR, NPR_SU, 4, NULL);
		}
		else if (result == ERR_NE)
		{
            MakeSendPackage(iter,NPR,NPR_IE,4,NULL);
		}
        else if (result == ERR_IP)
		{
			MakeSendPackage(iter, NPR, NPR_FA, 4, NULL);
		}
	}
	else if (pack.type == NC)//修改设置
	{
		memcpy(&(iter->user.config),pack.content,sizeof(Config));
        iter->user.config.ntoh();
        int result = SetConfig(iter->user);
		if (result == SUC)
		{
			MakeSendPackage(iter, NCR, NCR_SU, 4, NULL);
            sprintf(buffer,"用户%s设置成功：%d,%02X%02X%02X%02X\n",
                iter->user.name,
                iter->user.config.oldnum,
                iter->user.config.color.r,
                iter->user.config.color.g,
                iter->user.config.color.b,
                iter->user.config.color.a);
            Log(buffer);
		}
        else if(result == ERR_NE)
        {
            MakeSendPackage(iter,NCR,NCR_IE,4,NULL);
        }
		else if (result == ERR_IC)
		{
			MakeSendPackage(iter, NCR, NCR_FA, 4, NULL);
            sprintf(buffer,"用户%s设置不合法：%d,%02X%02X%02X%02X\n",
                iter->user.name,
                iter->user.config.oldnum,
                iter->user.config.color.r,
                iter->user.config.color.g,
                iter->user.config.color.b,
                iter->user.config.color.a);
		}
	}
	else if (pack.type == SM)//转发消息
	{
		User user;
        vector<User> receivers;
        int p,q;    //p：左分隔符,q：右分隔符
        list<Connection>::iterator it;
        char buffer[1024];
        bool all=false;

        //解析报文内容
        for(p=0,q=1;q<pack.length;++q)
        {
            //验收时加
            if(memcmp(pack.content,"@all",3)==0)
            {
                    all=true;
                    break;
            }
            //原判断流程
            if(pack.content[q]=='@'||pack.content[q]==':')
            {
                //相邻的@：不合法，但不严重，略过
                if(q-1==p)
                {
                    //MakeSendPackage(iter,TMR,TMR_WS,4,NULL);
                    p=q;
                    continue;
                }
                //接收者是all吗？
                if(q-p==4&&memcmp(pack.content,"all",3)==0)
                {
                    for(it=conl.begin();it!=conl.end();++it)
                    {
                        memcpy(user.name,it->user.name,sizeof(user.name));  //拷贝整个数组
                        receivers.push_back(user);
                    }
                    all=true;
                }
                else
                {
                    memcpy(user.name,pack.content+p+1,q-p-1);
                    user.name[q-p-1]=0;
                    receivers.push_back(user);
                }
                //最后一个分隔符
                if(pack.content[q]==':')
                    break;
                //左分隔符右移
                p=q;
            }
        }

        //根据解析结果判断是否语法错误
        if((pack.content[q]!=':'||receivers.size()==0)&&!all)
        {
            MakeSendPackage(iter,TMR,TMR_WS,4,NULL);
            return;
        }

        //去除重复的接收者//////////
        //待做/////////////////////

        //验收时加
        if(all)
        {
            Message m;
            memcpy(m.text,pack.content+5,pack.length-4-5+1);
            
            MakeSendPackage(iter,TMR,TMR_SU,4,NULL);
            for(it=conl.begin();it!=conl.end();++it)
            {
                if(strcmp(it->user.name,iter->user.name)==0)
                    continue;
                receivers.clear();
                receivers.push_back(it->user);
                int result=StoreMessage(iter->user,receivers,m);
                if(SUC)
                {
                    char text[MAX_CONTENT]={0};
                    strcpy(text,iter->user.name);
                    strcat(text,":");
                    strcat(text,m.text);
                    MakeSendPackage(it,TM,0x00,4+strlen(text)+1,text);
                }
                sprintf(buffer,"用户%s向%s发送消息：%s\n",iter->user.name,"所有在线用户",m.text);
                Log(buffer);
            }
            return;
        }
        

        //从报文正文中取得消息正文
        Message m;
        memcpy(m.text,pack.content+q+1,pack.length-4-(q+1));

        //分情况转发消息：单人和多人
        //先保存消息到数据库，顺便得知用户是否存在
        int result=StoreMessage(iter->user,receivers,m);
        int i;
        if(receivers.size()==1)
        {
            if(result==SUC)
            {
                //判断接收方是否在线
                for(it=conl.begin();it!=conl.end();++it)
                    if(!strcmp(receivers[0].name,it->user.name))
                        break;
                
                if(it!=conl.end())
                {
                    char text[MAX_CONTENT]={0};
                    strcpy(text,iter->user.name);
                    strcat(text,":");
                    strcat(text,m.text);
                    MakeSendPackage(it,TM,0x00,4+strlen(text)+1,text);
                    MakeSendPackage(iter,TMR,TMR_SU,4,NULL);
                }
                else
                {
                    MakeSendPackage(iter,TMR,TMR_SO,4,NULL);
                }
            }
            else if(result==ERR_NE)
            {
                MakeSendPackage(iter,TMR,TMR_NE,4,NULL);
            }
            else if(result==ERR_IN)
            {
                MakeSendPackage(iter,TMR,TMR_IE,4,NULL);
            }
            sprintf(buffer,"用户%s向%s发送消息：%s\n",iter->user.name,all?"所有在线用户":receivers[0].name,m.text);
            Log(buffer);
        }
        else
        {
            MakeSendPackage(iter,TMR,TMR_SU,4,NULL);
            for(i=0;i<receivers.size();++i)
            {
                if(result==SUC)
                {
                    //判断接收方是否在线
                    for(it=conl.begin();it!=conl.end();++it)
                        if(!strcmp(receivers[i].name,it->user.name))
                            break;
                    
                    if(it!=conl.end())
                    {
                        char text[MAX_CONTENT]={0};
                        strcpy(text,iter->user.name);
                        strcat(text,":");
                        strcat(text,m.text);
                        MakeSendPackage(it,TM,0x00,4+strlen(text)+1,text);
                    }
                    else
                    {
                        MakeSendPackage(iter,TMR,TMR_SO,4,NULL);
                    }
                }
                else if(result==ERR_NE)
                {
                    MakeSendPackage(iter,TMR,TMR_NE,4,NULL);
                }
                else if(result==ERR_IN)
                {
                    MakeSendPackage(iter,TMR,TMR_IE,4,NULL);
                }
            }
            sprintf(buffer,"用户%s向",iter->user.name);
            for(i=0;i<receivers.size();++i)
            {
                strcat(buffer,receivers[i].name);
                if(i==receivers.size()-1)
                    break;
                strcat(buffer,",");
            }
            strcat(buffer,"发送了消息");
            strcat(buffer,m.text);
            strcat(buffer,"\n");
            Log(buffer);
        }

        
	}
	else if (pack.type == PF)//发送方准备发送文件
	{
        //接收方用户存在吗？
        User u;
        strcpy(u.name,pack.content+4);

        sprintf(buffer,"用户%s准备向%s发送%dB的文件%s\n",iter->user.name,pack.content+4,ntohl(*(int *)pack.content),pack.content+4+28);
        Log(buffer);

        int result=IsRegistered(u);
        if(result==SUC)
        {
            //用户在线吗？
            list<Connection>::iterator it;
            for(it=conl.begin();it!=conl.end();++it)
            {
                if(it!=iter&&strcmp(pack.content+4,it->user.name)==0)
                    break;
            }
            //用户不在线
            if(it==conl.end())
            {
                MakeSendPackage(iter,PFR,PFR_NO,4,NULL);
                return;
            }
            //准备转发文件
            char buf[4+28+256];
            memcpy(buf,pack.content,4+28+256);
            memcpy(buf+4,iter->user.name,28);
            MakeSendPackage(it,PT,0x00,4+4+28+256,buf);
            //两方各自记录对方
            strcpy(iter->other.receiver.name,it->user.name);
            strcpy(it->other.sender.name,iter->user.name);
            
        }
        else if(result==ERR_NE)
        {
            MakeSendPackage(iter,PFR,PFR_NE,4,NULL);
            printf("接受者%s不存在\n",pack.content+4);
            return;
        }
        else if(result==ERR_IN)
        {
            MakeSendPackage(iter,PFR,PFR_IE,4,NULL);
            return;
        }
	}
    else if (pack.type == PTR)//接收方准备接收文件
    {
        list<Connection>::iterator it;
        for(it=conl.begin();it!=conl.end();++it)
        {
            if(it!=iter&&strcmp(iter->other.sender.name,it->user.name)==0)
                break;
        }
        if(it==conl.end())
        {
            //没找到发送方，说明已经下线
            printf("用户%s接收文件过程中未找到发送方%s\n",iter->user.name,iter->other.receiver.name);
            return;
        }
        //进入文件收发状态
        if(pack.subtype==PTR_OK)
        {
            MakeSendPackage(it,PFR,PFR_CO,4,NULL);
            it->state=SFILE;
            sprintf(buffer,"用户%s接受了来自%s的发送文件请求\n",iter->user.name,it->user.name);
            Log(buffer);
        }
        else if(pack.subtype==PTR_NO)
        {
            MakeSendPackage(it,PFR,PFR_DE,4,NULL);
            sprintf(buffer,"用户%s拒绝了来自%s的发送文件请求\n",iter->user.name,it->user.name);
            Log(buffer);
        }
        
    }
}

void PSfile(list<Connection>::iterator &iter,const Package &pack)
{
    char buffer[1024];
    list<Connection>::iterator it;
    for(it=conl.begin();it!=conl.end();++it)
    {
        if(it!=iter&&strcmp(iter->other.receiver.name,it->user.name)==0)
            break;
    }
    //没找到接收方
    if(it==conl.end())
    {
        iter->state=ONLINE;
        printf("用户%s发送文件过程中未找到接收方%s\n",iter->user.name,iter->other.receiver.name);
        //断线重传
        /*
        strcpy(iter->filename,content+32);
        FILE *fp=fopen(iter->filename,"w");
        if(fp==NULL)
        {
            sprintf(buffer,"用户%s准备传送文件，但文件%s打开失败",iter->user.name,pack.content+4);
            Log(buffer);
            MakeSendPackage(iter,PFR,PFR_IE,4,NULL);
            return;
        }*/
        //没找到接收方
        //以追加方式打开文件，写入内容
        //如果收到SSF，通知发送方已经保存
        return;
    }

    if(pack.type==SF)
    {
        MakeSendPackage(it,TF,0x00,4+MAX_CONTENT,pack.content);
        printf(pack.content);
    }
    else if(pack.type==SSF)
    {
        MakeSendPackage(it,STF,0x00,4,NULL);
        MakeSendPackage(iter,SFR,SFR_SU,4,NULL);
        iter->state=ONLINE;
        printf("用户%s向%s发送文件结束\n",iter->user.name,it->user.name);
        //目前没有发送SFR_FA的地方
    }
}

//处理客户端的总函数
void ProcessClients(int listenfd)
{
	int nonblock = 1;
	struct sockaddr_in their_addr;	//对方地址信息
	socklen_t sin_size=sizeof(struct sockaddr_in);
	fd_set fds;			//文件描述符集合
	int maxfd;			//用于select的最大fd
	int accfd;			//接收连接的fd
    list<Connection>::iterator iter;    //链表迭代器
    int i;

    while(1)
    {
        //listenfd和所有连接的fd加入fds
        FD_ZERO(&fds);
		FD_SET(listenfd,&fds);
		maxfd=listenfd;
		for(iter=conl.begin();iter!=conl.end();++iter)
		{
            FD_SET(iter->accfd,&fds);
            if(iter->accfd>maxfd)
                maxfd=iter->accfd;
		}

        if(1)
        {
            printf("/----------连接数目：%2d----------\\\n",conl.size());
            printf("用户名%14s状态\n","");
            for(iter=conl.begin();iter!=conl.end();++iter)
            {
                if(iter->state!=LOGIN)
                {
                    printf("%-20s",iter->user.name);
                    printf("%d\n",iter->state);
                }
            }
            printf("\\--------------------------------/\n");
        }

        if(select(maxfd+1,&fds,NULL,NULL,NULL)>0)
        {
            //有新连接
			if(FD_ISSET(listenfd,&fds))
			{
				accfd=accept(listenfd,(struct sockaddr*)&their_addr,&sin_size);
                if(accfd==-1)
                {
                    if(errno!=11)
                        printf("连接建立失败：%d\n",errno);
                }
                else
                {
                    printf("成功连接fd%d！\n",accfd);
                    Connection con;
                    ioctl (accfd, FIONBIO, &nonblock);
                    con.accfd=accfd;
                    con.state=LOGIN;
                    conl.push_back(con);
				}
			}

			//有客户端发包
            //当链表为空,iter==conl.begin(),iter==conl.end(),++iter==iter,--iter==iter
			for(iter=conl.begin();iter!=conl.end();++iter)
				if(FD_ISSET(iter->accfd,&fds))
				{
                    Package p;
                    //读取报文头
                    int readsize=read(iter->accfd,(char *)&p,4);
                    //被select中但是没有内容：连接已断开
                    if(readsize==0)
                    {
                        EndConnection(iter);
                        continue;
                    }
                    else if(readsize==-1)
                    {
                        printf("从fd%d读取报头出错：%d\n",iter->accfd,errno);
                        continue;
                    }
                    //解析报文头
                    p.length=ntohs(p.length);       //转主机序
                    printf("从fd%d读取%dB报头，解析：长度为%d的%02X/%02X类报文\n",iter->accfd,readsize,p.type,p.subtype,p.length);
                    if(p.length>MAX_CONTENT+4||p.length<4)
                    {
                        printf("报文长度异常，关闭连接！\n",p.length);
                        EndConnection(iter);
                        continue;
                    }//长度错误时，留在缓冲区的内容不就会解析出错了吗？怎么处理？
                    
                    //读取报文正文
                    //由于报文很小，报文分开可能性不大。所以目前是在一次select内读取完报文
                    //如果出现报文被分开来，可能需要多次select读取了
                    int total=4;
                    int once;
                    while(total<p.length)
                    {
                        once=read(iter->accfd,p.content,p.length-total);
                        if(once<0)
                        {
                            printf("fd%d出现read返回-1：%d\n",iter->accfd,errno);
                            break;
                        }
                        else if(once==0)
                        {
                            printf("fd%d出现read返回0\n",iter->accfd);
                            break;
                        }
                        else
                        {
                            printf("从fd%d读取%dB的报文正文.\n",iter->accfd,once);
                            total+=once;
                        }
                    }

                    //根据连接的状态解析报文与进行操作
                    switch(iter->state)
                    {
                    case LOGIN:
                        PLogin(iter,p);
                        break;
                    case FORCEPW:
                        PForcepw(iter,p);
                        break;
                    case ONLINE:
                        POnline(iter,p);
                        break;
                    case SFILE:
                        PSfile(iter,p);
                        break;
                    }
				}
        }//end of if(select)ee
    }//end of while(1)
}