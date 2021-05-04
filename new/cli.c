/*
 * =====================================================================================
 *
 *       Filename:  cli.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/23/2020 07:08:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Fritz Mehner (mn), mehner@fh-swf.de
*        Company:  FH Südwestfalen, Iserlohn
 *
 * =====================================================================================
 */
/*************************************************************************
	> File Name: cli.c
	> Author: xiao
	> Mail: www.1239012801@qq.com 
	> Created Time: Wed 23 Dec 2020 07:08:15 PM CST
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/select.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include"message.h"
#include<string.h>
static int sockfd;
static int login_status=0;
addr server;
addr cpeer;
void init_ser()
{
	memset(&server,0,sizeof (server));
	char ip[IPLEN];
	short port;
	printf("please input ip:\n");
	scanf("%s",ip);
	printf("please input port:\n");
	scanf("%hd",&port);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=inet_addr(ip);
	server.sin_port=htons(port);
}

void on_message(addr from,Message msg)
{
	if(addr_equal(server,from))
	{
		switch(msg.head.type){
			case MTYPE_PUNCH:
				{
					printf("recv from ser:\n");
					addr peer=ep_fromstring(msg.body);
					for(int i=0;i<3;i++)//d向peer发送3次打洞
						udp_send_text(sockfd,peer,MTYPE_PUNCH,NULL);
				}
				break;
			case MTYPE_REPLY:
				printf("%s\n",msg.body);
				break;
			case MTYPE_NAMEERROR:
				printf("name error\n");
				login_status=0;
				break;
			case MTYPE_PASSWDERROR:
				printf("passwd error\n");
				login_status=0;
				break;
			case MTYPE_LOGINSUS:
				printf("login succeed\n");
				login_status=1;
				break;
			default:
				break;
		}

	}
	//from peer
	memset(&cpeer,0,sizeof(cpeer));
	memcpy(&cpeer,&from,sizeof(from));
	switch(msg.head.type)
	{
		case MTYPE_TEXT:
			printf("peer:%s\n",msg.body);
			break;
		case MTYPE_REPLY:
			printf("peer repiled,you can talk now\n");
			break;
		case MTYPE_PUNCH:
			udp_send_text(sockfd,from,MTYPE_TEXT,"I SEE YOU");
			break;
		case MTYPE_PING:
			udp_send_text(sockfd,from,MTYPE_PONG,NULL);
			break;
		default:
			break;
	}
}
void *keepalive_loop()
{
	Message ping;
	ping.head.magic=MSG_MAGIC;
	ping.head.type=MTYPE_PING;
	ping.head.length=0;
	ping.body=NULL;
	while(1)
	{
		udp_send_msg(sockfd,server,ping);
		udp_send_msg(sockfd,cpeer,ping);
		sleep(5);
	}
	return NULL;
}
void talk()
{
	char line[SEND_BUFSIZE];
	while(1)
	{
		scanf("%s",line);
		if(strncmp(line,"quit",4)==0)
			break;
		udp_send_text(sockfd,cpeer,MTYPE_TEXT,line);
	}
}

void *console_loop()
{
	char name[NAMELEN];
	char *line=NULL;
	size_t len;
	ssize_t read;
	while(fprintf(stdout,">>>")&&(read=getline(&line,&len,stdin))!=-1){
		if(login_status==0)
		{
			printf("loginstatus:no\n");
		}else{
			printf("loginstatus:%s on line\n",name);
		}
		if(read==1) continue;
		char *cmd=strtok(line," ");
		if(strncmp(cmd,"login",5)==0)
		{
			//登录
			cli_login(sockfd,server,name);
		}else if(strncmp(cmd,"list",4)==0){
			//获取list
			printf("lllist\n");
			udp_send_text(sockfd,server,MTYPE_LIST,"");
		}else if(strncmp(cmd,"punch",5)==0){
			//打洞 传入一个名字
			char *name=strtok(NULL,"\n");
			udp_send_text(sockfd,server,MTYPE_PUNCH,name);
		}else if(strncmp(cmd,"send",4)==0){
			//发送消息
			char *buf=strtok(NULL,"\n");
			udp_send_text(sockfd,cpeer,MTYPE_TEXT,buf);
		}else if(strncmp(cmd,"quit",4)==0){
			//退出
			break;
		}else if(strncmp(cmd,"help",4)==0){
			//打印帮助信息
			printfhelp();
		}else if(strncmp(cmd,"logout",4)==0){
			//登出
		}else if(strncmp(cmd,"talk",4)==0)
		{
			talk();
		}
		else{
			printf("Unknow cmd:\n");
		}

	}
	free(line);
	return NULL;
}

void *receive_loop()
{
	addr peer;
	socklen_t addrlen;
	char buf[RECV_BUFSIZE];
	int nfds;
	fd_set readfds;
	struct timeval timeout;
	nfds=sockfd+1;

	while(1)
	{
		FD_ZERO(&readfds);
		FD_SET(sockfd,&readfds);
		timeout.tv_sec=1;
		timeout.tv_usec=0;
		int ret=select(nfds,&readfds,NULL,NULL,&timeout);
		if(ret==0)
		{
			//超时
			continue;
		}
		else if(ret==-1)
		{
			perror("select");
			continue;
		}
		addrlen=sizeof(peer);
		memset(&peer,0,addrlen);
		memset(buf,0,RECV_BUFSIZE);
		int rd_size=recvfrom(sockfd,buf,RECV_BUFSIZE,0,(struct sockaddr*)&peer,&addrlen);
		if(rd_size==-1)
		{
			perror("recvfrom");
			continue;
		}else if(rd_size==0)
		{
			printf("EOF\n");
			continue;
		}
		Message msg=msg_unpack(buf,rd_size);
		if(msg.head.magic!=MSG_MAGIC || msg.body==NULL)
		{
			printf("invaild message\n");
			continue;
		}
		on_message(peer,msg);
	}
	return NULL;


}

int main()
{
	int ret;
	init_ser();
	sockfd=socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd==-1)
	{
		printf("sock error\n");
	}
	pthread_t keepalive_pid,receive_pid,console_pid;
	ret=pthread_create(&keepalive_pid,NULL,&keepalive_loop,NULL);
	if(ret!=0){ return 0;}
	ret=pthread_create(&receive_pid,NULL,&receive_loop,NULL);
	if(ret!=0){return 0;}
	ret=pthread_create(&console_pid,NULL,&console_loop,NULL);
	if(ret!=0){return 0;}
	pthread_join(console_pid,NULL);
	pthread_join(receive_pid,NULL);
	pthread_join(keepalive_pid,NULL);
	return 0;

}
