#include"message.h"
#include<string.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdint.h>
int addr_equal(addr lp,addr rp)
{
	return ((lp.sin_family==rp.sin_family)
			&&(lp.sin_addr.s_addr==rp.sin_addr.s_addr)
			&&(lp.sin_port==rp.sin_port));
}

/*
* 在使用该函数之前需要有一个已经填好头部信息的Mesage
该函数主要是对msg的信息转变为网络字节序，存入buf中，将其发送出去，bufzise为buf的长度
不能小于msg的头和body长度之和 不然放不下
*/
int msg_pack(Message msg, char* buf, unsigned int bufsize)
{
	if (bufsize < MSG_HEADLEN + msg.head.length) {
		printf("buf too small");
		return 0;
	}

	int16_t m_magic = htons(msg.head.magic);
	int16_t m_type = htons(msg.head.type);
	int32_t m_length = htonl(msg.head.length);

	int index = 0;
	memcpy(buf+index,&m_magic,MSG_MAGICLEN);
	index += MSG_MAGICLEN;
	memcpy(buf + index, &m_type, MSG_TYPELEN);
	index += MSG_TYPELEN;
	memcpy(buf + index, &m_length, MSG_BODYLEN);
	index += MSG_BODYLEN;
	memcpy(buf + index, msg.body, msg.head.length);
	index +=msg.head.length;

	return index;
}

char *typemsg(MessageType type)
{
	switch(type)
	{
		case MTYPE_LOGIN:
			return "MTYPE_LOGIN";
		case MTYPE_LOGOUT:
			return "MTYPE_LOGOUT";
		case MTYPE_LIST:
			return "MTYPE_LIST";
		case MTYPE_PUNCH:
			return "MTYPE_PUNCH";
		case MTYPE_PING:
			return"MTYPE_PING";
		case MTYPE_PONG:
			return "MTYPE_PONG";
		case MTYPE_REPLY:
			return "MTYPE_REPLY";
		case MTYPE_TEXT:
			return "MTYPE_TEXT";
		case MTYPE_NAMEERROR:
			return "MTYPE_NAMEERROR";
		case MTYPE_PASSWDERROR:
			return "MTYPE_PASSWDERROR";
		case MTYPE_LOGINSUS:
			return "MTYPE_LOGINSUS";
		case MTYPE_END:
			return "MTYPE_END";
		default:
			return "Unknow";
	}

}



/*

*/
Message msg_unpack(const char* buf, unsigned int buflen)
{
	Message m;
	memset(&m, 0, sizeof(m));
	if (buflen < MSG_HEADLEN)//小于头的长度 放不下
	{
		return m;
	}
	int index = 0;
	//uint16_t*为两个字节的指针 解析magic
	m.head.magic = ntohs(*(uint16_t*)(buf + index));
	index += sizeof(uint16_t);
	if (m.head.magic != MSG_MAGIC)//不符合该报文格式 不用解析
	{
		return m;
	}
	//解析类型
	m.head.type = ntohs(*(uint16_t*)(buf + index));
	index += sizeof(uint16_t);
	//解析长度
	m.head.length = ntohl(*(uint32_t*)(buf + index));
	index += sizeof(uint32_t);
	if (index + m.head.length > buflen) {
		printf("message declared body size(%d) is larger than what's received (%d), truncating\n",
			m.head.length, buflen - MSG_HEADLEN);
		m.head.length = buflen - index;
	}
	//解析body
	m.body = buf + index;
	return m;
}
int udp_send_msg(int sock, addr peer, Message msg)
{

	char buf[SEND_BUFSIZE] = { 0 };
	int wt_size = msg_pack(msg, buf, SEND_BUFSIZE);
	return sendto(sock, buf, wt_size, MSG_DONTWAIT, (struct sockaddr*)&peer,sizeof(peer));
}
/*传入一个要发送的信息buf，对buf进行封装成发送的Message*/
int udp_send_buf(int sock, addr peer, MessageType type,const char* buf, unsigned int len)
{
	Message m;
	m.head.magic = MSG_MAGIC;
	m.head.type = type;
	m.head.length = len;
	m.body = buf;
	return udp_send_msg(sock, peer, m);
}


int udp_send_text(int sock, addr peer,MessageType type, const char* text)
{
	unsigned int len = (text == NULL ? 0 : strlen(text));
	return udp_send_buf(sock, peer, type, text, len);
}
int login_unpack(char name[NAMELEN],char passwd[PASSWDLEN],Message msg)
{

	//因为msg中的信息不可更改 strtok需要改变msg
	if(msg.head.length==0)
	{
		return -1;
	}
	char buf[msg.head.length+1];
	memcpy(buf,msg.body,msg.head.length);
	char *token;

	token=strtok(buf,",");
	strncpy(name,token,sizeof(token));
	token=strtok(NULL,",");
	strncpy(passwd,token,sizeof(token));
	return 0;
}
void getIpAndPort(int sock,addr peer,char ip[IPLEN],int *port)
{
	int peerlen=sizeof(peer);
	getpeername(sock,(struct sockaddr*)&peer,&peerlen);
	char *p=inet_ntoa(peer.sin_addr);
	memcpy(ip,p,IPLEN);
	*port=ntohs(peer.sin_port);
}

void printfmsg(Message msg)
{
	printf("magic:%hu\ntype:%hu\nlength:%u\nbody:%s\n",msg.head.magic,
			msg.head.type,msg.head.length,msg.body);

}

char *getthelist()
{
	static char list[SEND_BUFSIZE]={0};
	MYSQL *mysql;
	MYSQL_RES *res=NULL;
	mysql=mysql_init(NULL);
	if(mysql_real_connect(mysql,"localhost","root","123456","users",3306,NULL,0))
	{
		char sql[SQLLEN]="select name from users.user where loginstatus=1;";
		mysql_query(mysql,sql);
		res=mysql_store_result(mysql);
		MYSQL_ROW myrow=mysql_fetch_row(res);
		char *q=list;
		for(char **p=myrow;p!=NULL;p=mysql_fetch_row(res))
		{
			for(int i=0;i<strlen(p[0]);i++)
			{
				*q++=p[0][i];
			}
			*q++=' ';
		}
	}
	return list;
}

addr getaddr(char ip[IPLEN],int port)
{
	static addr add;
	add.sin_family=AF_INET;
	add.sin_addr.s_addr=inet_addr(ip);
	add.sin_port=htons(port);
	return add;
}
addr ep_fromstring(const char *buf)
{
	char tuple[IPLEN+5];
	char *host=NULL;
	char *port=NULL;
	sprintf(tuple,"%s",buf);
	host=strtok(tuple,":");
	port=strtok(NULL,":");
	if(host==NULL || port==NULL)
	{
		host="255.255.255.255";
		port="0";
	}
	addr add;
	memset(&add,0,sizeof(add));
	add.sin_family=AF_INET;
	add.sin_addr.s_addr=inet_addr(host);
	add.sin_port=htons(atoi(port));
	return add;
}

void cli_login(int sockfd,addr server,char name[NAMELEN])
{
	char _name[NAMELEN],_passwd[PASSWDLEN];
	char tuple[NAMELEN+PASSWDLEN];
	printf("NAME:\n");
	scanf("%s",_name);
	printf("PASSWD:\n");
	scanf("%s",_passwd);
	snprintf(tuple,NAMELEN+PASSWDLEN,"%s,%s",_name,_passwd);
	udp_send_text(sockfd,server,MTYPE_LOGIN,tuple);
	strcpy(name,_name);
}

void printfhelp()
{
	printf("--------------------------------------------\n");
	printf("This is a prograrm for p2p punch\n");
	printf("You can login use 123 456 789\n");
	printf("The server host is 120.26.187.136,port is 5000\n");
	printf("login:Login\n");
	printf("list: Get the online accunt\n");
	printf("punch: Punch a user,you can talk with him.format:punch xxx\n");
	printf("send:Send message to you have punched.format:send xxx\n");
	printf("quit:Quit the program\n");
	printf("--------------------------------------------\n");

}
