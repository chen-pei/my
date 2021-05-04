#MAKEFILE

cli:
	gcc cli.c Mysql.h Mysql.c message.h message.c -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient -lpthread -o cli
ser:
	gcc ser.c Mysql.h Mysql.c message.h message.c -I/usr/include/mysql -L/usr/lib/mysql -lmysqlclient -o ser