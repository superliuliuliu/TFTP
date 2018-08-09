#ifndef _TFTP_H_
#define _TFTP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>

/*
* TFTP报文中的操作码占两个字节  定义为short类型
*/
#define OPTCODE_RRQ (unsigned short)1
#define OPTCODE_WRQ (unsigned short)2
#define OPTCODE_DATA (unsigned short)3
#define OPTCODE_ACK (unsigned short)4
#define OPTCODE_ERROR (unsigned short)5


#define DATASIZE 512            // TFTP报文中的数据长度为512字节
#define MAX_RETRANSMISSION 3    // 发送报文的最大重传次数，超过该次数则认为报文不能够发送
#define MAX_TIME_WAIT 1000*1000 // 等待ACK确认报文的最大时间为1秒
#define DEFAULT_PORT (unsigned short) 7341     //客户端的默认监听端口号
#define MAX_PACKET_REQUEST 1024 //最大请求报文长度1024字节
#define MAX_THREAD_SIZE 16      //最大子线程数，对应最大允许连接的客户端数

/*
 *  TFTP报文数据结构
 *  其中采用union的原因是   union公用一块内存空间且大小均为两个字节
 *  三种不同的数据类型代表着三种不同的TFTP报文
 *  block代表DATA报文  指代块号
 *  errcode代表ERROR报文 指代差错码
 *  filename指代请求报文
 */
struct tftp_packet{
  unsigned short optcode;       //操作码
  union{
      unsigned short block;    //文件中的块号
      unsigned short errcode;  //差错码
      char filename[2];        //相关信息存储的内存地址其中包含文件名 模式以及块的大小 存储形式为"%s%c%s%c%c"
  };
  char data[DATASIZE];         //报文中用来存储数据的字符数组
};

// TFTP请求报文结构
struct tftp_request{
  int size;                    //请求报文的长度?
  struct sockaddr_in client;   //客户端的地址和端口信息
  struct tftp_packet packet;   //报文
};

// 线程记录结构体
struct thread_record {
  bool usable;                  //线程是否被使用
  pthread_t tid;                //线程ID
};

// 为了在pthread_create()中传递多个参数
struct deliever_para{
  struct tftp_request request;
  int thread_index;
};

//声明全局变量
extern int connect_counter;
extern char *list;
extern struct thread_record customer[MAX_THREAD_SIZE];
/*
*  服务器端调用的函数
*/
int send_packet(int sockfd, struct tftp_packet *packet, int size);
int send_ack(int sockfd, struct tftp_packet *packet, int size);
static void *thread_func(void *arg);
void file_download(struct tftp_request request, int sockfd);
void file_upload(struct tftp_request request, int sockfd);


#endif
