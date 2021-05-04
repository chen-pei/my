/*
 * =====================================================================================
 *
 *       Filename:  Mysql.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/21/2020 06:27:17 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Fritz Mehner (mn), mehner@fh-swf.de
 *        Company:  FH Südwestfalen, Iserlohn
 *
 * =====================================================================================
 */

/*************************************************************************
	> File Name: Mysql.h
	> Author: xiao
	> Mail: www.1239012801@qq.com 
	> Created Time: Mon 21 Dec 2020 06:27:17 PM CST
 *********************************************************************/
#ifndef _MYSQL_H_
#define _MYSQL_H_
#define NAMELEN 16
#define PASSWDLEN 16
#define SQLLEN 256
#include<mysql.h>
int _mysql_login(const char name[NAMELEN],const char passwd[PASSWDLEN]);//初始化mysql
int database_my(const char name[NAMELEN], char ip[20],int *port);
void exesql(const char sql[SQLLEN]);//执行:一句sql语句

#endif
