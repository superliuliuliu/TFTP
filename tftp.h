#ifndef _TFTP_H_
#define _TFTP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>

/*
* TFTP报文中的操作码占两个字节  定义为short类型
*/
#define OPTCODE_RRQ (short)1
#define OPTCODE_WRQ (short)2
#define OPTCODE_DATA (short)3
#define OPTCODE_ACK (short)4
#define OPTCODE_ERROR (short)5


#define DATASIZE 512         // TFTP报文中的数据长度为512字节
#define MAX_RETRANSMISSION 3 // 发送报文的最大重传次数，超过该次数则认为报文不能够发送
#define MAX_TIEM_WAIT 1000*1000 // 等待ACK确认报文的最大时间为1秒


/*
 *  TFTP报文数据结构
 *  其中采用union的原因是   union公用一块内存空间且大小均为两个字节
 *  三种不同的数据类型代表着三种不同的TFTP报文
 *  block代表DATA报文  指代块号
 *  errcode代表ERROR报文 指代差错码
 *  filename指代请求报文
 */
struct tftp_packet{
  unsigned short optcode;    //操作码
  union{
      unsigned short block;  //文件中的块号
      unsigned short errcode;//差错码
      char filename[2];      //相关信息存储的内存地址其中包含文件名 模式以及块的大小 存储形式为"%s%c%s%c%c"
  };
  char data[DATASIZE];       //报文中用来存储数据的字符数组
};

// TFTP请求报文结构
struct tftp_request{
  int size;                 //请求报文的长度
  struct sockaddr_in client;//客户端的地址和端口信息
  struct tftp_packet packet;//报文
};

//全局变量
int connect_counter = 0;    //全局变量：记录连接的客户端数目
char *list = ".";          //目录变量  初始化为当前目录"."

/*
*  该模块包括以下子功能
*   1.报文发送功能
*   2.服务器端处理客户端请求线程功能
*/
int send_packet(int sockfd, struct tftp_packet *packet, int size);
int send_ack(int sockfd, struct tftp_packet *packet, int size);
static void *thread_func(void *arg);
void file_download(struct tftp_request *request, int sockfd);
void file_upload(struct tftp_request *request, int sockfd);

#endif
