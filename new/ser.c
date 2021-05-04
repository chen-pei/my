/*
 * =====================================================================================
 *
 *       Filename:  ser.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/21/2020 07:28:13 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Fritz Mehner (mn), mehner@fh-swf.de
 *        Company:  FH Südwestfalen, Iserlohn
 *
 * =====================================================================================
 */

/*************************************************************************
	> File Name: ser.c
	> Author: xiao
	> Mail: www.1239012801@qq.com 
	> Created Time: Mon 21 Dec 2020 07:28:13 PM CST
 ************************************************************************/

#include<stdio.h>
#include"message.h"
#include"Mysql.h"
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<arpa/inet.h>
addr server;
void init_server(const char *host,short port)
{
	memset(&server,0,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr(host);
	server.sin_port=htons(port);
}
typedef void callback_t(int server_sock,addr from,Message msg);
void udp_recv(int sock,callback_t callback)
{
	addr peer;//存储发送信息的地址信息
	int addrlen;
	char buf[RECV_BUFSIZE];
	while(1)
	{
		addrlen=sizeof(peer);
		memset(&peer,0,sizeof(peer));
		memset(buf,0,RECV_BUFSIZE);
		int rd_size;
		rd_size=recvfrom(sock,buf,RECV_BUFSIZE,0,(struct sockaddr*)&peer,&addrlen);
		if(rd_size==-1)
		{
			perror("recvfrom");
			break;
		}
		else if(rd_size==0)
		{
			printf("EOF\n");
			continue;
		}
		Message msg=msg_unpack(buf,rd_size);
		if(msg.head.magic!=MSG_MAGIC || msg.body==NULL){
			printf("invalid message\n");
			continue;
		}
		callback(sock,peer,msg);
		continue;
	}

}
void on_message(int sock,addr from,Message msg)
{
	printf("RECV %u bytes TYPE:%s \n",msg.head.length,typemsg(msg.head.type));
	switch(msg.head.type)
	{
		case MTYPE_LOGIN:
			{
				char name[NAMELEN],passwd[NAMELEN];
				//对账号密码解包
				int jud=login_unpack(name,passwd,msg);
				if(jud==-1)//headtype为login 但是无信息
				{
					udp_send_text(sock,from,MTYPE_REPLY,"login error");
					break;
				}
				int ret=_mysql_login(name,passwd);
				if(ret==2)//未找到账号
				{
					udp_send_text(sock,from,MTYPE_NAMEERROR,"");
					break;
				}
				if(ret==1)//密码错误
				{
					udp_send_text(sock,from,MTYPE_PASSWDERROR,"");
					break;
				}
				if(ret==-1)
				{
					perror("database error");
					break;
				}
				if(ret==0)//登录成功
				{
					//更改数据库中的状态
					char sql[SQLLEN];
					char ip[IPLEN];
					int port;
					getIpAndPort(sock,from,ip,&port);
					printf("ip:%s port:%d\n",ip,port);
					snprintf(sql,SQLLEN,"update user set loginstatus='1',ip='%s',port='%d' where name='%s';",
							ip,port,name);
					exesql(sql);
					udp_send_text(sock,from,MTYPE_LOGINSUS,"succeed");

				}
			}
		case MTYPE_PING:
			udp_send_text(sock,from,MTYPE_PONG,NULL);
			break;
		case MTYPE_TEXT:
			printf("RECV:%s\n",msg.body);
			break;
		case MTYPE_LIST:
			{
				char *list;
				list=getthelist();
				printf("list:%s\n",list);
				udp_send_text(sock,from,MTYPE_REPLY,list);
				memset(list,0,sizeof(list));
				break;
			}
		case MTYPE_PUNCH:
			{
				char name[NAMELEN];
				memset(name,0,sizeof(name));
				memcpy(name,msg.body,NAMELEN);
				char ip[IPLEN],pip[IPLEN];
				int port,pport;
				char tuple[IPLEN+5],ptuple[IPLEN+5];
				int	ret=database_my(name,ip,&port);
				if(ret==-1)//未找到该账号
				{
					udp_send_text(sock,from,MTYPE_NAMEERROR,"");
					break;
				}
				snprintf(tuple,IPLEN+5,"%s:%d",ip,port);
				printf("tuple:%s\n",tuple);
				addr add=getaddr(ip,port);
				printf("add:%hd\n",ntohs(add.sin_port));
				getIpAndPort(sock,from,pip,&pport);
				snprintf(ptuple,IPLEN+5,"%s:%d",pip,pport);
				printf("ptuple%s\n",ptuple);
				udp_send_text(sock,add,MTYPE_PUNCH,ptuple);
				udp_send_text(sock,from,MTYPE_PUNCH,tuple);
			}
	}
}



int main(int argc,char **argv)
{
	if(argc!=2)
	{
		printf("Usage:%s <port>\n",argv[0]);
		return 1;
	}
	const char *host="0.0.0.0";
	short port=atoi(argv[1]);
	int ret;
	init_server(host,port);
	int sock=socket(AF_INET,SOCK_DGRAM,0);
	if(sock==-1)
	{
		perror("socket");
		exit(EXIT_FAILURE);

	}
	ret=bind(sock,(const struct sockaddr*)&server,sizeof(server));
	if(ret==-1)
	{
		perror("bind");
		exit(EXIT_FAILURE);

	}
	printf("the server start on %s %d sock:%d\n",host,port,sock);
	udp_recv(sock,on_message);
}


