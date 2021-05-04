/*
 * =====================================================================================
 *
 *       Filename:  Mysql.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/21/2020 06:38:09 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Fritz Mehner (mn), mehner@fh-swf.de
 *        Company:  FH Südwestfalen, Iserlohn
 *
 * =====================================================================================
 */

/*************************************************************************
	> File Name: Mysql.c
	> Author: xiao
	> Mail: www.1239012801@qq.com 
	> Created Time: Mon 21 Dec 2020 06:38:09 PM CST
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include"Mysql.h"

int database_my(const char name[NAMELEN], char ip[20],int *port)
{
	int ret;
	char sql[SQLLEN];
	MYSQL *mysql;
	MYSQL_RES *res=NULL;
	mysql=mysql_init(NULL);
	if(mysql_real_connect(mysql,"localhost","root","123456","users",3306,NULL,0))
	{
		//连接数据库成功
		snprintf(sql,SQLLEN,"select *from users.user where name='%s' and loginstatus='1';",name);

	}
	ret=mysql_query(mysql,sql);
	res=mysql_store_result(mysql);
	MYSQL_ROW row=mysql_fetch_row(res);
	if(row==NULL)//账号未登录或未找到账号
	{
		mysql_free_result(res);
		return -1;
	
	}
	strcpy(ip,row[3]);
	*port=atoi(row[4]);
	mysql_free_result(res);
	return 0;

}
//返回2未查找到账号 返回1密码错误 返回0登录成功 返回-1数据库连接失败
int _mysql_login(const char name[NAMELEN],const char passwd[PASSWDLEN])
{
	int ret;
	char sql[SQLLEN];
	MYSQL *mysql;
	MYSQL_RES *res=NULL;
	mysql=mysql_init(NULL);
	if(mysql_real_connect(mysql,"localhost","root","123456","users",3306,NULL,0))
	{
		snprintf(sql,SQLLEN,"select *from users.user where name='%s';",name);
		ret=mysql_query(mysql,sql);
		res=mysql_store_result(mysql);
		if(!mysql_fetch_row(res))
		{
			mysql_free_result(res);
			return 2;
		}
		mysql_free_result(res);
		memset(sql,0,sizeof(sql));
		snprintf(sql,SQLLEN,"select *from users.user where name='%s'and passwd='%s';",name,passwd);
		ret=mysql_query(mysql,sql);
		res=mysql_store_result(mysql);
		if(!mysql_fetch_row(res))
		{
			mysql_free_result(res);
			return 1;
		}
		mysql_free_result(res);
		return 0;
	}
	else
		return -1;

}
void exesql(const char sql[SQLLEN])
{
	MYSQL *mysql;
	MYSQL_RES *res=NULL;
	mysql=mysql_init(NULL);
	if(mysql_real_connect(mysql,"localhost","root","123456","users",3306,NULL,0))
	{
		int ret=mysql_query(mysql,sql);
		res=mysql_store_result(mysql);
		mysql_free_result(res);
	}

}



