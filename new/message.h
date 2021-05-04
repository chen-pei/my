#ifndef MESSAGE_H
#define MESSAGE_H
#include<stdint.h>
#include"Mysql.h"
#include<netinet/in.h>
#define MSG_MAGIC 0x8964
#define MSG_MAGICLEN 2
#define MSG_TYPELEN 2
#define MSG_BODYLEN 4
#define MSG_HEADLEN MSG_MAGICLEN + MSG_TYPELEN + MSG_BODYLEN
#define SEND_BUFSIZE 1024
#define RECV_BUFSIZE 1024
#define IPLEN 20
typedef struct sockaddr_in addr;
typedef struct _MessageHead MessageHead;
typedef enum _MessageType
{
	MTYPE_LOGIN = 0,
	MTYPE_LOGOUT,
	MTYPE_LIST,
	MTYPE_PUNCH,//打洞
	MTYPE_PING,//心跳包
	MTYPE_PONG,
	MTYPE_REPLY,
	MTYPE_TEXT,
	MTYPE_NAMEERROR,
	MTYPE_PASSWDERROR,
	MTYPE_LOGINSUS,
	MTYPE_END
}MessageType;

struct _MessageHead {
	uint16_t magic;
	uint16_t type;
	uint32_t length;

}__attribute__((packed));

typedef struct _Message {
	MessageHead head;
	const char* body;
}Message;

char *typemsg(MessageType type);

int addr_equal(addr lp,addr rp);

int imsg_pack(Message msg,char *buf,unsigned int bufsize);//封包 封包之后存入msg

Message msg_unpack(const char* buf,unsigned int bufsize);//解包，返回一个Message

int udp_send_msg(int sock, addr peer, Message msg);

int udp_send_buf(int sock, addr peer, MessageType type,
	const char* buf, unsigned int len);
int udp_send_text(int sock,addr peer,
	MessageType type, const char* text);
int login_unpack(char name[NAMELEN],char passwd[PASSWDLEN],Message msg);

void getIpAndPort(int sock,addr peer,char ip[IPLEN],int *port);

void printfmsg(Message msg);

char *getthelist();

addr getaddr(char ip[IPLEN],int port);

addr ep_fromstring(const char *buf);

void cli_login(int sockfd,addr server,char name[NAMELEN]);

void printfhelp();
#endif // !MESSAGE_H
