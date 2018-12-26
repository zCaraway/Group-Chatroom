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

list<Connection> conl;  //connection list��ʹ����������ɾ��Ч�ʸ��ߣ�

//���������
void MakeSendPackage(list<Connection>::iterator &iter,unsigned char type,unsigned char subtype,unsigned short length,const char *content)
{
    Package p;
    p.type=type;
    p.subtype=subtype;
    p.length=htons(length);
    if(content)
        memcpy(p.content,content,length-4);
    int writelen=write(iter->accfd,(char *)&p,length>4+MAX_CONTENT?4+MAX_CONTENT:length);
    printf("��fd%d��������Ϊ%02X/%02X��%dB���ģ�",iter->accfd,type,subtype,length);
    if(writelen==-1)
    {
        printf("д�����%d\n",errno);
    }
    else
    {
        printf("д��%dB\n",writelen);
    }
}

//�Ͽ�����
//�ͻ�����������ʱֱ�ӵ���EndConnection
//����Ƿ���������ر����ӣ�ִ���ض��������ٵ���EndConnection
void EndConnection(list<Connection>::iterator &iter)
{
    State s=iter->state;
    iter=conl.erase(iter);
    --iter; //�������Ӻ������һ��ѭ������++iter������Ҫ��--������

    //�����������һ�������û������������������û��������߸��±���
    if(s==ONLINE||s==SFILE)
    {
        list<Connection>::iterator it;
        State ss;
        for(it=conl.begin();it!=conl.end();++it)
        {
            ss=it->state;
            if(ss==LOGIN||ss==FORCEPW)    //��������״̬�����Ӳ��÷����߸��±���
                continue;
            MakeSendPackage(it,NOS,NOS_OF,4,NULL);
        }
    }
}

//���ߺ���
//�����ͬ����¼��������
//���Ͷ�Ӧ��LRP���޳�ͻ���г�ͻ��¼��
//���͵�ǰ�����û���Ϣ
//���͵�ǰ�û�����
//���͹�ȥ����Ϣ
//��������Ϣ���͸����������û�
void GoOnline(list<Connection>::iterator &iter)
{
	list<Connection>::iterator it;
    char buffer[1024];
    /********** ��ͬ����¼ **********/
    bool kick=false;
	for (it = conl.begin(); it != conl.end(); ++it)
	{
		if (it != iter)
		{
			if (!strcmp(iter->user.name, it->user.name) && it->state != LOGIN && it->state != FORCEPW)  //��ͬ��(ֻ������1��)
			{
				MakeSendPackage(it, KK, 0x00, 4, NULL);//�����߷���
				EndConnection(it);//ǿ������
				kick=true;
                break;
			}
		}
	}
    if(kick)
    {
        sprintf(buffer,"��һ���ն˵�¼���û�%s��ǿ������\n",iter->user.name);
        Log(buffer);
    }
    if(1)
        printf("���ߺ���-��ͬ����¼���:%s\n",kick?"kicked":"not kicked");
    /********** �����˵����߸��� **********/
    //���߻ظ�
    //if(it==conl.end())    //��������жϣ���Ϊ����ߵ����һ�����ӣ�it�ͻ�ָ��conl.end()
    MakeSendPackage(iter, LRP, kick?LRP_KK:LRP_NC, 4, NULL);
    //���������û�������״̬
    for(it=conl.begin();it!=conl.end();++it)
    {
        if(it==iter||it->state==LOGIN||it->state==FORCEPW)
            continue;
        MakeSendPackage(iter,NOS,NOS_ON,32,it->user.name);
    }
    if(1)
        printf("���ߺ���-�����˵����߸������:%d�����������û�\n",conl.size()-1);
    /********** ���͵�ǰ�û����� **********/
	Config c = iter->user.config;
	c.hton();
	MakeSendPackage(iter, CC, 0x00, 4 + sizeof(Config), (char *)&c);
    if(1)
        printf("���ߺ���-���͵�ǰ�û��������:%d,%02X%02X%02X%02X\n",
                iter->user.config.oldnum,
                iter->user.config.color.r,
                iter->user.config.color.g,
                iter->user.config.color.b,
                iter->user.config.color.a);
    /********** ������ʷ��Ϣ **********/

    /*
    //������Ϣ�ͽ�����Ϣ�ֱ�ʱ�����򣬲������߼�
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
        printf("���ߺ���-������ʷ��Ϣ���:%d��\n",num);
    /********** �����˵��û�״̬���� **********/
    for (it = conl.begin(); it != conl.end(); ++it)
	{
		if (it==iter || it->state == LOGIN || it->state == FORCEPW) //û��������ɵĲ��÷�
			continue;
		MakeSendPackage(it, NOS, NOS_ON, 32, iter->user.name);
	}
    if(1)
        printf("���ߺ���-�������û����߸������\n");
	
    iter->state = ONLINE;
}

//�ĸ�״̬�հ�ʱ�Ĵ������������ܿ������͵İ���������������

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
            printf("����LRQ���ģ�username=%s,password=%s\n",iter->user.name,iter->user.password);
        if(result==SUC)
        {
            GoOnline(iter);
            sprintf(buffer,"�û�%s��½�ɹ�\n",iter->user.name);
            Log(buffer);
        }
        else if(result==SUC_FL)
        {
            MakeSendPackage(iter,FL,0x00,4,NULL);
            iter->state=FORCEPW;
            sprintf(buffer,"�û�%s��һ�ε�¼��Ҫ�����\n",iter->user.name);
            Log(buffer);
        }
        else if(result==ERR_NE)
        {
            MakeSendPackage(iter,LRP,LRP_NE,4,NULL);
            sprintf(buffer,"�����¼���û�%s������\n",iter->user.name);
            Log(buffer);
            EndConnection(iter);
        }
        else if(result==ERR_WP)
        {
            MakeSendPackage(iter,LRP,LRP_WP,4,NULL);
            sprintf(buffer,"�û�%s�������:%s\n",iter->user.name,iter->user.password);
            Log(buffer);
            EndConnection(iter);
        }
        else if(result==ERR_IN)
        {
            //�ڲ�����
            sprintf(buffer,"�û�%s��¼ʱ�������ڲ�����\n",iter->user.name);
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
            //�ڲ����󣺸ո�ͨ����֤���û�������
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
	if (pack.type == NP)//�޸�����
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
	else if (pack.type == NC)//�޸�����
	{
		memcpy(&(iter->user.config),pack.content,sizeof(Config));
        iter->user.config.ntoh();
        int result = SetConfig(iter->user);
		if (result == SUC)
		{
			MakeSendPackage(iter, NCR, NCR_SU, 4, NULL);
            sprintf(buffer,"�û�%s���óɹ���%d,%02X%02X%02X%02X\n",
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
            sprintf(buffer,"�û�%s���ò��Ϸ���%d,%02X%02X%02X%02X\n",
                iter->user.name,
                iter->user.config.oldnum,
                iter->user.config.color.r,
                iter->user.config.color.g,
                iter->user.config.color.b,
                iter->user.config.color.a);
		}
	}
	else if (pack.type == SM)//ת����Ϣ
	{
		User user;
        vector<User> receivers;
        int p,q;    //p����ָ���,q���ҷָ���
        list<Connection>::iterator it;
        char buffer[1024];
        bool all=false;

        //������������
        for(p=0,q=1;q<pack.length;++q)
        {
            //����ʱ��
            if(memcmp(pack.content,"@all",3)==0)
            {
                    all=true;
                    break;
            }
            //ԭ�ж�����
            if(pack.content[q]=='@'||pack.content[q]==':')
            {
                //���ڵ�@�����Ϸ����������أ��Թ�
                if(q-1==p)
                {
                    //MakeSendPackage(iter,TMR,TMR_WS,4,NULL);
                    p=q;
                    continue;
                }
                //��������all��
                if(q-p==4&&memcmp(pack.content,"all",3)==0)
                {
                    for(it=conl.begin();it!=conl.end();++it)
                    {
                        memcpy(user.name,it->user.name,sizeof(user.name));  //������������
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
                //���һ���ָ���
                if(pack.content[q]==':')
                    break;
                //��ָ�������
                p=q;
            }
        }

        //���ݽ�������ж��Ƿ��﷨����
        if((pack.content[q]!=':'||receivers.size()==0)&&!all)
        {
            MakeSendPackage(iter,TMR,TMR_WS,4,NULL);
            return;
        }

        //ȥ���ظ��Ľ�����//////////
        //����/////////////////////

        //����ʱ��
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
                sprintf(buffer,"�û�%s��%s������Ϣ��%s\n",iter->user.name,"���������û�",m.text);
                Log(buffer);
            }
            return;
        }
        

        //�ӱ���������ȡ����Ϣ����
        Message m;
        memcpy(m.text,pack.content+q+1,pack.length-4-(q+1));

        //�����ת����Ϣ�����˺Ͷ���
        //�ȱ�����Ϣ�����ݿ⣬˳���֪�û��Ƿ����
        int result=StoreMessage(iter->user,receivers,m);
        int i;
        if(receivers.size()==1)
        {
            if(result==SUC)
            {
                //�жϽ��շ��Ƿ�����
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
            sprintf(buffer,"�û�%s��%s������Ϣ��%s\n",iter->user.name,all?"���������û�":receivers[0].name,m.text);
            Log(buffer);
        }
        else
        {
            MakeSendPackage(iter,TMR,TMR_SU,4,NULL);
            for(i=0;i<receivers.size();++i)
            {
                if(result==SUC)
                {
                    //�жϽ��շ��Ƿ�����
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
            sprintf(buffer,"�û�%s��",iter->user.name);
            for(i=0;i<receivers.size();++i)
            {
                strcat(buffer,receivers[i].name);
                if(i==receivers.size()-1)
                    break;
                strcat(buffer,",");
            }
            strcat(buffer,"��������Ϣ");
            strcat(buffer,m.text);
            strcat(buffer,"\n");
            Log(buffer);
        }

        
	}
	else if (pack.type == PF)//���ͷ�׼�������ļ�
	{
        //���շ��û�������
        User u;
        strcpy(u.name,pack.content+4);

        sprintf(buffer,"�û�%s׼����%s����%dB���ļ�%s\n",iter->user.name,pack.content+4,ntohl(*(int *)pack.content),pack.content+4+28);
        Log(buffer);

        int result=IsRegistered(u);
        if(result==SUC)
        {
            //�û�������
            list<Connection>::iterator it;
            for(it=conl.begin();it!=conl.end();++it)
            {
                if(it!=iter&&strcmp(pack.content+4,it->user.name)==0)
                    break;
            }
            //�û�������
            if(it==conl.end())
            {
                MakeSendPackage(iter,PFR,PFR_NO,4,NULL);
                return;
            }
            //׼��ת���ļ�
            char buf[4+28+256];
            memcpy(buf,pack.content,4+28+256);
            memcpy(buf+4,iter->user.name,28);
            MakeSendPackage(it,PT,0x00,4+4+28+256,buf);
            //�������Լ�¼�Է�
            strcpy(iter->other.receiver.name,it->user.name);
            strcpy(it->other.sender.name,iter->user.name);
            
        }
        else if(result==ERR_NE)
        {
            MakeSendPackage(iter,PFR,PFR_NE,4,NULL);
            printf("������%s������\n",pack.content+4);
            return;
        }
        else if(result==ERR_IN)
        {
            MakeSendPackage(iter,PFR,PFR_IE,4,NULL);
            return;
        }
	}
    else if (pack.type == PTR)//���շ�׼�������ļ�
    {
        list<Connection>::iterator it;
        for(it=conl.begin();it!=conl.end();++it)
        {
            if(it!=iter&&strcmp(iter->other.sender.name,it->user.name)==0)
                break;
        }
        if(it==conl.end())
        {
            //û�ҵ����ͷ���˵���Ѿ�����
            printf("�û�%s�����ļ�������δ�ҵ����ͷ�%s\n",iter->user.name,iter->other.receiver.name);
            return;
        }
        //�����ļ��շ�״̬
        if(pack.subtype==PTR_OK)
        {
            MakeSendPackage(it,PFR,PFR_CO,4,NULL);
            it->state=SFILE;
            sprintf(buffer,"�û�%s����������%s�ķ����ļ�����\n",iter->user.name,it->user.name);
            Log(buffer);
        }
        else if(pack.subtype==PTR_NO)
        {
            MakeSendPackage(it,PFR,PFR_DE,4,NULL);
            sprintf(buffer,"�û�%s�ܾ�������%s�ķ����ļ�����\n",iter->user.name,it->user.name);
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
    //û�ҵ����շ�
    if(it==conl.end())
    {
        iter->state=ONLINE;
        printf("�û�%s�����ļ�������δ�ҵ����շ�%s\n",iter->user.name,iter->other.receiver.name);
        //�����ش�
        /*
        strcpy(iter->filename,content+32);
        FILE *fp=fopen(iter->filename,"w");
        if(fp==NULL)
        {
            sprintf(buffer,"�û�%s׼�������ļ������ļ�%s��ʧ��",iter->user.name,pack.content+4);
            Log(buffer);
            MakeSendPackage(iter,PFR,PFR_IE,4,NULL);
            return;
        }*/
        //û�ҵ����շ�
        //��׷�ӷ�ʽ���ļ���д������
        //����յ�SSF��֪ͨ���ͷ��Ѿ�����
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
        printf("�û�%s��%s�����ļ�����\n",iter->user.name,it->user.name);
        //Ŀǰû�з���SFR_FA�ĵط�
    }
}

//����ͻ��˵��ܺ���
void ProcessClients(int listenfd)
{
	int nonblock = 1;
	struct sockaddr_in their_addr;	//�Է���ַ��Ϣ
	socklen_t sin_size=sizeof(struct sockaddr_in);
	fd_set fds;			//�ļ�����������
	int maxfd;			//����select�����fd
	int accfd;			//�������ӵ�fd
    list<Connection>::iterator iter;    //���������
    int i;

    while(1)
    {
        //listenfd���������ӵ�fd����fds
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
            printf("/----------������Ŀ��%2d----------\\\n",conl.size());
            printf("�û���%14s״̬\n","");
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
            //��������
			if(FD_ISSET(listenfd,&fds))
			{
				accfd=accept(listenfd,(struct sockaddr*)&their_addr,&sin_size);
                if(accfd==-1)
                {
                    if(errno!=11)
                        printf("���ӽ���ʧ�ܣ�%d\n",errno);
                }
                else
                {
                    printf("�ɹ�����fd%d��\n",accfd);
                    Connection con;
                    ioctl (accfd, FIONBIO, &nonblock);
                    con.accfd=accfd;
                    con.state=LOGIN;
                    conl.push_back(con);
				}
			}

			//�пͻ��˷���
            //������Ϊ��,iter==conl.begin(),iter==conl.end(),++iter==iter,--iter==iter
			for(iter=conl.begin();iter!=conl.end();++iter)
				if(FD_ISSET(iter->accfd,&fds))
				{
                    Package p;
                    //��ȡ����ͷ
                    int readsize=read(iter->accfd,(char *)&p,4);
                    //��select�е���û�����ݣ������ѶϿ�
                    if(readsize==0)
                    {
                        EndConnection(iter);
                        continue;
                    }
                    else if(readsize==-1)
                    {
                        printf("��fd%d��ȡ��ͷ����%d\n",iter->accfd,errno);
                        continue;
                    }
                    //��������ͷ
                    p.length=ntohs(p.length);       //ת������
                    printf("��fd%d��ȡ%dB��ͷ������������Ϊ%d��%02X/%02X�౨��\n",iter->accfd,readsize,p.type,p.subtype,p.length);
                    if(p.length>MAX_CONTENT+4||p.length<4)
                    {
                        printf("���ĳ����쳣���ر����ӣ�\n",p.length);
                        EndConnection(iter);
                        continue;
                    }//���ȴ���ʱ�����ڻ����������ݲ��ͻ��������������ô����
                    
                    //��ȡ��������
                    //���ڱ��ĺ�С�����ķֿ������Բ�������Ŀǰ����һ��select�ڶ�ȡ�걨��
                    //������ֱ��ı��ֿ�����������Ҫ���select��ȡ��
                    int total=4;
                    int once;
                    while(total<p.length)
                    {
                        once=read(iter->accfd,p.content,p.length-total);
                        if(once<0)
                        {
                            printf("fd%d����read����-1��%d\n",iter->accfd,errno);
                            break;
                        }
                        else if(once==0)
                        {
                            printf("fd%d����read����0\n",iter->accfd);
                            break;
                        }
                        else
                        {
                            printf("��fd%d��ȡ%dB�ı�������.\n",iter->accfd,once);
                            total+=once;
                        }
                    }

                    //�������ӵ�״̬������������в���
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