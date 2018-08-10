#include "tftp.h"

/**
 * send_packet 发送一个TFTP报文 支持DATA报文、请求报文
 * @param  sockfd socket所对应的文件描述符
 * @param  packet 指向报文的指针
 * @param  size   最多能够发送的字节数
 * @return numbers of bytes send,-1 on error
 */
int send_packet(int sockfd, struct tftp_packet *packet, int size){
    struct tftp_packet recv_packet;
    int time_wait_counter = 0;//微妙级别
    int send_times = 0;
    int r_size = 0;
    /*
    * 规定一个报文最大的重传次数，当在规定时间内没有收到ACK时，进行重传，一个报文最多重传3次=MAX_RETRANSMISSION。
    * 当传送次数超过最大重传次数时，提示该报文发送失败。
    */
    for (send_times = 0; send_times < MAX_RETRANSMISSION; send_times++){
        printf("正在传送文件块：%d\n",ntohs(packet->block));
        if (send(sockfd, packet, size, 0) != size){
            printf("传送过程中有数据丢失,传送失败！");
            return -1;
        }
        //停止等待 接收ack报文 收到ack报文说明 packet发送成功
        for (time_wait_counter = 0; time_wait_counter < MAX_TIME_WAIT; time_wait_counter += 10000){
            r_size = recv(sockfd, &recv_packet, sizeof(struct tftp_packet), MSG_DONTWAIT);
            /*
             * 判断是否是ACK报文，
             * 是：跳出该停止等待机制
             * 不是：睡眠10000微妙，重复以上循环,继续等待ACK报文。
             */
            if ((r_size >= 4)&&(recv_packet.optcode == htons(OPTCODE_ACK))&&(recv_packet.block == packet->block)){
                break;
            }
            usleep(10000);
        }
        if (time_wait_counter < MAX_TIME_WAIT){
            break;
        }
    }
    if (send_times == MAX_RETRANSMISSION){
        printf("报文重传次数超出限制,请检查网络后重新尝试！\n");
        return -1;
    }
    return size;
}

/**
 * send_ack 向发送方返回一个确认报文
 * @param  sockfd  socket所对应的文件描述符
 * @param  packet  指向要发送报文指针
 * @param  size    最多能够发送的字节数
 * @return numbers of bytes send,-1 on error
 */
int send_ack(int sockfd, struct tftp_packet *packet, int size){
    if (send(sockfd, packet, size, 0) != size){
        printf("发送确认报文过程中丢失数据！");
        return -1;
    }
    return size;
}

/**
 * thread_func  线程start()函数,服务器端创建子线程用来处理客户端的请求。
 * @param  arg void*类型，即指向任何对象的指针都可以传递给它
 * @return void*与上面参数arg相似
 */
void *thread_func(void *arg){

    struct deliever_para *deliever = (struct deliever_para*)arg;
    int sockfd;
    int idx = deliever->thread_index;                      //使用的线程下标
    struct sockaddr_in server;
    pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALZER;                                  //互斥信号量给全局变量连接数加锁
    static socklen_t addr_len = sizeof(struct sockaddr_in);//this struct has a static length

    pthread_mutex_lock(&mtx);
    connect_counter++;
    printf("新的客户端与TFTP服务器端建立连接，当前连接的客户端数:%d\n", connect_counter);//设置全局变量来记录连接的客户端数 后面实现
    /*
    * 创建一个数据传输socket
    */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        printf("Socket 创建失败\n");
        return NULL;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = 0;    //设置数据传输socket端口号
    // 数据传输socket绑定server地址
    if (bind(sockfd, (struct sockaddr *)&server, addr_len) < 0){
        printf("数据传输socket绑定失败！\n");
        return NULL;
    }
    // 与客户端建立连接
    if (connect(sockfd, (struct sockaddr *)&(deliever->request.client), addr_len) < 0){
        printf("连接到客户端失败！\n");
        return NULL;
    }
    //判断客户端发出的请求
    switch(deliever->request.packet.optcode){
        case OPTCODE_RRQ:{
            printf("正在处理客户端：%d 的文件下载请求！\n",connect_counter);
            file_download(deliever->request, sockfd);
            customer[idx].usable = true;
            connect_counter--;
            pthread_mutex_unlock(&mtx);
            break;
        }
        case OPTCODE_WRQ:{
            printf("正在处理客户端：%d 的文件上传请求！\n",connect_counter);
            //此处应该有一个处理上传文件的函数
            file_upload(deliever->request, sockfd);
            customer[idx].usable = true;
            connect_counter--;
            pthread_mutex_unlock(&mtx);
            break;
        }
        /*case OPTCODE_END:{

        }*/
        default:{
            printf("错误的操作码\n！");
            customer[idx].usable = true;
            connect_counter--;
            pthread_mutex_unlock(&mtx);
            break;
        }
    }
    //数据传输完成后关闭用于数据传输的socket一起释放所分配的内存空间
    close(sockfd);
    return NULL;
}

/**
 * file_download    服务器端文件下载函数，对应客户端的RRQ请求  虚度去一个需求报文，发送多个DATA报文
 * @param  request  指向客户端发来请求报文的指针
 * @param  sockfd   文件描述符 对应服务器端在子线程中用于数据传输的socket
 * @return   暂时未定,函数功能框架确定后，再来确定返回值及其代表的含义
 */
void file_download(struct tftp_request request, int sockfd){
    struct tftp_packet s_packet;               //用于封装发送数据的报文结构
    char filepath[256];                          //用来存储文件路径
    /*
     *根据packet结构体的结构  确定报文传输数据块的大小
     *|-filename-'0'-mode-'0'-blocksize_str-|
     */
    char *filename = request.packet.filename;    //文件名指针
    char *mode = filename + strlen(filename) + 1; //文件模式指针
    char *blocksize_str = mode + strlen(mode) + 1;
    int blocksize = atoi(blocksize_str);         //文件的大小
    // 记录此刻系统时间,用于日志记录操作。
    time_t rawtime;
    struct tm *info;
    char buffer[80];
    //记录操作成功与否的布尔变量
    bool success = true;

    /*
     由于TFTP报文一次最多传送512字节的文件内容,当文件大于512字节时需要多次传送,
     有争议
    */
    if (blocksize <= 0 || blocksize > DATASIZE){
        blocksize = DATASIZE;
    }


     //初始化文件路径  linux下表示当前目录 "."
     memset(filepath, 0, sizeof(filepath));
     strcpy(filepath, list);
     if (filename[0] != '/'){
		     strcat(filepath, "/");
	   }
	   strcat(filepath, filename);
     printf("客户端:%d 请求下载文件：%s\n",connect_counter, filepath);

     FILE *fp = fopen(filepath, "rb");
	   if (fp == NULL){
		     printf("服务器端不存在您需要下载的文件，请检查文件名后重试！\n");
         // 发送错误报文？在客户端显示出错原因？  此处挖坑，后续补上
		     return;
	   }

     //开始封装发送数据报文
     int content_size = 0;//每次从文件中读取的数据数 最大512字节 当其不是512字节时表示文件传输完毕
     s_packet.optcode = htons(OPTCODE_DATA);
     unsigned short block = 1;
     do {
         memset(s_packet.data, 0, sizeof(s_packet.data));
         s_packet.block = htons(block);
         content_size = fread(s_packet.data, 1, blocksize, fp);
         // 当发包失败时会将错误打印到屏幕上  其中size 为数据部分+操作码(2字节)+块号(2字节)
         if (send_packet(sockfd, &s_packet, content_size + 4) == -1){
             fprintf(stderr, "服务器端发送第%d个文件块时出错.\n", block);
             success = false;
             break;
         }
         printf("发送第%d个文件块成功\n", block);
         block++;

     } while(content_size == DATASIZE);
     printf("文件发送完成！\n");

     // 记录服务器端操作到日志文件中
     FILE *fp_log = fopen("./server_log.txt", "a");
     if (fp_log == NULL){
		     printf("打开日志文件失败,请检查。\n");
		     return;
	   }
     time(&rawtime);
     info = localtime(&rawtime);

     if (success == true){
        fprintf(fp_log, "客户端:%d 下载文件:%s 时间:%s操作成功！\n", connect_counter, filename, asctime(info));
     }
     else{
        fprintf(fp_log, "客户端:%d 下载文件:%s 时间:%s操作失败:服务器发送文件块失败！\n", connect_counter, filename, asctime(info));
     }
     //关闭文件
     fclose(fp);
     fclose(fp_log);
     return;
}

/**
 * file_upload     服务器端文件上传函数，对应客户端的WRQ请求 接收并读取多个DATA报文 然后发送相应的ACK报文
 * @param  request 指向客户端发来请求报文的指针
 * @param  sockfd  文件描述符
 * @return  同上
 */
void file_upload(struct tftp_request request, int sockfd){
    struct tftp_packet ack_packet, recv_packet;
    char filepath[256];                          //用来存储文件路径
    /*
     *根据packet结构体的结构  确定报文传输数据块的大小
     *|-filename-'0'-mode-'0'-blocksize_str-|
     */
    char *filename = request.packet.filename;    //文件名指针
    char *mode = filename + strlen(filename) + 1; //文件模式指针
    char *blocksize_str = mode + strlen(mode) + 1;
    int blocksize = atoi(blocksize_str);         //文件的大小
    // 记录此刻系统时间,用于日志记录操作。
    time_t rawtime;
    struct tm *info;
    char buffer[80];
    // 记录操作成功与否的布尔变量
    bool success = true;

    /*
     由于TFTP报文一次最多传送512字节的文件内容,当文件大于512字节时需要多次传送,
     有争议
    */
    if (blocksize <= 0 || blocksize > DATASIZE){
        blocksize = DATASIZE;
    }
     //初始化文件路径  linux下表示当前目录 "."
     memset(filepath, 0, sizeof(filepath));
     strcpy(filepath, list);
     if (filename[0] != '/'){
		     strcat(filepath, "/");
	   }
	   strcat(filepath, filename);
     printf("客户端:%d 请求上传文件：%s\n",connect_counter, filepath);

     // 记录服务器端操作到服务器日志文件中
     FILE *fp_log = fopen("./server_log.txt", "a");
     if (fp_log == NULL){
		     printf("打开日志文件失败,请检查。\n");
		     return;
	   }
     time(&rawtime);
     info = localtime(&rawtime);
     //首先检测服务器端是否存在同名文件  以只读方式打开
     FILE *fp = fopen(filepath, "r");
	   if (fp != NULL){
		     fclose(fp);
		     printf("文件 %s 已经存在.\n", filepath);
         fprintf(fp_log, "客户端:%d 上传文件:%s 时间:%s 操作失败:服务器端已存在文件！\n", connect_counter, filename, asctime(info));
         fclose(fp_log);
         return;
	   }
     //创建新文件
     fp = fopen(filepath, "w");
     if (fp == NULL){
         printf("文件创建失败！");
         fclose(fp);
         fprintf(fp_log, "客户端:%d 上传文件:%s 时间:%s 操作失败：服务器端创建文件失败！\n", connect_counter, filename, asctime(info));
         fclose(fp_log);
         return;
     }

     //发送第一个ack报文  用来响应客户端的上传请求
     ack_packet.optcode = htons(OPTCODE_ACK);
     ack_packet.block = htons(0);
     if (send_ack(sockfd, &ack_packet, 4) == -1){
         fprintf(stderr, "服务器端发送响应请求ACK失败.\n");
         fprintf(fp_log, "客户端:%d   上传文件:%s   时间:%s  操作失败：服务器端发送响应请求ACK失败！\n", connect_counter, filename, asctime(info));
         fclose(fp_log);
         fclose(fp);
         return;
     }

     int recv_size = 0;
     int write_size = 0;
     unsigned short block = 1;
     int time_wait_counter = 0;
     /*
     接收DATA报文   关键点在于判断文件的传输结束
     当文件大于512字节，blocksize=512 文件会分为多个packet传送，最后一个recv_size < 512
    */
     do{
         /*
          等待接收一个packet  根据上面规定一个报文最多重发3次 以及超时重传的相关规定
         */
         for(time_wait_counter = 0; time_wait_counter < MAX_RETRANSMISSION * MAX_TIME_WAIT ; time_wait_counter += 20000){
             recv_size = recv(sockfd, &recv_packet, sizeof(struct tftp_packet), MSG_DONTWAIT);
             //DATA报文最小长度也要大于4字节
             if (recv_size > 0 && recv_size < 4){
                 printf("DATA报文传输错误,将重传！\n");
             }
             if ((recv_size >= 4) && (recv_packet.optcode = htons(OPTCODE_DATA)) && (recv_packet.block == htons(block))){
                 printf("正在接收第%d个文件块...\n", block);
                 write_size = fwrite(recv_packet.data, 1, recv_size - 4, fp);
                 if (write_size == recv_size - 4){
                     break;
                 }
                 else{
                     printf("报文写入文件失败，等待报文重传！\n");
                 }
             }
             usleep(20000);
         }
         if (time_wait_counter >= MAX_TIME_WAIT * MAX_RETRANSMISSION){
             printf("接收DATA报文超时！\n");
             success = false;
             break;//跳出while循环
         }
         //若未超时发送ACK报文
         ack_packet.block = htons(block);
         ack_packet.optcode = htons(OPTCODE_ACK);
         if (send_ack(sockfd, &ack_packet, 4) == -1){
             fprintf(stderr, "服务器端ACK失败.\n");
             fprintf(fp_log, "客户端:%d   上传文件:%s   时间:%s  操作失败：服务器端发送响应请求ACK失败！\n", connect_counter, filename, asctime(info));
             fclose(fp_log);
             fclose(fp);
             return;
         }
         block++;
     }while(recv_size == DATASIZE + 4);

     if (success == true){
        fprintf(fp_log, "客户端:%d   上传文件:%s   时间:%s  操作成功！\n", connect_counter, filename, asctime(info));
        printf("文件%s上传成功！", filename);
     }
     else{
        fprintf(fp_log, "客户端:%d   上传文件:%s   时间:%s  操作失败！\n", connect_counter, filename, asctime(info));
     }
     //关闭文件
     fclose(fp);
     fclose(fp_log);
     return;

}
