#ifndef _CLIENT_H_
#define _CLIENT_H_


#include "tftp.h"

//客户端接收输入命令的最大长度
#define MAX_BUFFER_SIZE 1024


//客户端利用的主要函数声明
void get_file(char *server_file);
void put_file(char *local_file);
void do_list();

#endif
