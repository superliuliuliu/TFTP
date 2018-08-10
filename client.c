# include "client.h"


//服务器端的相关信息
int sockfd;
struct sockaddr_in server;
socklen_t addr_len = sizeof(struct sockaddr_in);
int blocksize = DATASIZE;

/*
 * 客户端运行主程序
 */
int main(int argc, char **argv){
    char command[MAX_BUFFER_SIZE];         // 用来存储客户端输入的命令请求
    char *server_ip;                       // 用来存储用户输入的命令行参数
    unsigned short port = DEFAULT_PORT;    //

    //在客户端上打印使用提示信息
    printf("欢迎使用TFTP客户端！\n");
    printf("--------------------------------------------\n");
    printf("使用指南：\n");
    if (argc < 2){
		    printf("Usage:  %s [server ip] [port]\n",argv[0]);
		    printf("服务器端的默认端口号为7341,,如果你已经设置请忽略！\n");
		    return 0;
	  }
    printf("你可以输入下列命令来使用本TFTP客户端:\n");
	  printf("-从服务器端下载文件:\n");
	  printf("get [remote_file]\n");
	  printf("-向服务器端上传文件:\n");
	  printf("put [local_file]\n");
	  printf("-退出客户端程序:\n");
	  printf("quit\n");

    server_ip = argv[1];
    if (argc > 2){
        port = (unsigned short)atoi(argv[2]);
    }
    printf("本客户端已连接到位于%s ：%d的TFTP服务器端\n",server_ip, port);

    //创建数据连接socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
		    printf("客户端数据传输socket创建失败.\n");
		    return 0;
	  }

	  // Initialize server address
	  server.sin_family = AF_INET;
	  server.sin_port = htons(port);
    //将字符形式的ip地址转换为网络字节序形式并存储到server中
    if (inet_pton(AF_INET, server_ip, &(server.sin_addr.s_addr)) != 1){
        printf("服务器端地址信息设置出错！");
        return 0;
    }
    //开始处理用户输入的命令行
    char *buffer;
    char *arg;//用来存储输入的命令
    while (true){
        printf(">>");
        memset(command, 0, MAX_BUFFER_SIZE);
        // 从输入流中读取数据到command数组中
        buffer = fgets(command, MAX_BUFFER_SIZE, stdin);
        if (buffer == NULL){
            printf("读取命令失败！\n");
            return 0;
        }
        //利用strtok()切割输入的命令  获取具体的命令
        arg = strtok(buffer, " \t\n");
        if (arg == NULL){
          continue;//结束本次while循环进行下一个while循环，而不是跳出while循环
        }
        // 进一步判断输入的命令是哪一个
        if (strcmp(arg, "get") == 0){

            arg = strtok(NULL, " \t\n");
            if (arg == NULL){
                printf("缺少文件名\n");
            }else{
                get_file(arg);
            }
        }
        else if (strcmp(arg, "put") == 0){

            arg = strtok(NULL, " \t\n");
            if (arg == NULL){
                printf("缺少文件名\n");
            }else{
                put_file(arg);
            }
        }
        else if (strcmp(arg, "quit") == 0){
            break;
        }
        else{
            printf("未知的命令，请检查后重新输入！");
        }
    }
    return 0;
}

/**
 * get_file  客户端从服务器端下载远端文件
 * 客户端会发送一个请求报文与多个ACK报文
 * @param server_file 要下载远端文件的文件名
 */
void get_file(char *server_file){
    struct tftp_packet ack_packet, sed_packet, recv_packet;//sed_packet用来发送请求
    struct sockaddr_in client;

    // send the TFTP request
    sed_packet.optcode = htons(OPTCODE_RRQ);
    sprintf(sed_packet.filename, "%s%c%s%c%d%c", server_file, 0, "octet", 0, blocksize, 0);//将信息存储到packet中 以/-"filename"-"0"-"mode"-"0"-"blocksize"-"0"/
    sendto(sockfd, &sed_packet, sizeof(struct tftp_packet), 0, (struct sockaddr*)&server, addr_len);//发送这个报文到服务器端

    FILE *fp = fopen(server_file, "w");
    if (fp == NULL) {
        printf("本地文件：%s创建失败\n", server_file);
        return;
    }

    //接收数据 并发送ACK(停止等待机制、超时重传机制)
    int time_wait_counter = 0;
    int recv_size = 0;
    int write_size =0;
    unsigned short block = 1;//块号

    do{
        for(time_wait_counter = 0; time_wait_counter < MAX_RETRANSMISSION * MAX_TIME_WAIT; time_wait_counter += 10000){
            recv_size = recvfrom(sockfd, &recv_packet, sizeof(struct tftp_packet), MSG_DONTWAIT,
                                (struct sockaddr *)&client,
                                &addr_len);
            if (recv_size > 0 && recv_size < 4){
                printf("接收DATA报文出错！等待重传！\n");
            }
            if ((recv_size >= 4) && (recv_packet.optcode == htons(OPTCODE_DATA)) && (recv_packet.block == htons(block))){
                printf("正在接收第%d个文件块...\n", block);
                write_size = fwrite(recv_packet.data, 1, recv_size - 4, fp);
                if (write_size == recv_size - 4){
                    break;
                }
                else{
                    printf("数据写入文件失败，等待报文重传！\n");
                }
            }
            usleep(10000);
        }
        if (time_wait_counter >= MAX_TIME_WAIT * MAX_RETRANSMISSION){
            printf("接收DATA报文超时！\n");
            return;//跳出while循环
        }
        //若未超时发送ack报文
        ack_packet.block = htons(block);
        ack_packet.optcode = htons(OPTCODE_ACK);
        sendto(sockfd, &ack_packet, sizeof(struct tftp_packet), 0, (struct sockaddr*)&client, addr_len);
        /*if (send_ack(sockfd, &ack_packet, 4) == -1){
            fprintf(stderr, "客户端发送ACK失败.\n");
            fclose(fp);
            return;
        }*/
        block++;
    }while(recv_size == blocksize + 4);
    printf("文件下载完成！\n");

    fclose(fp);
    return;
}

/**
 * put_file  客户端向服务器端上传本地文件
 * 客户端会发送一个请求报文和多个DATA报文
 * @param local_file 要上传本地文件名
 */
void put_file(char *local_file){




    return;
}
