# include "client.h"


//服务器端的相关信息
int sockfd;
struct sockaddr_in server;
socklen_t addr_len = sizeof(struct sockaddr_in);
int blocksize = DATASIZE;

/*
 * 客户端运行主程序
 */
int main(int argc, char **argv)
{
    char command[MAX_BUFFER_SIZE];         // 用来存储客户端输入的命令请求
    char *server_ip;                       // 用来存储用户输入的命令行参数
    unsigned short port = DEFAULT_PORT;    //

    //在客户端上打印使用提示信息
    printf("欢迎使用TFTP客户端！\n");
    printf("--------------------------------------------\n");
    printf("使用指南：\n");
    if (argc < 2){
		    printf("Usage:  %s [server ip] [port]\n",argv[0]);
		    printf("服务器端的默认端口号为7341,如果你已经设置请忽略！\n");
		    return 0;
	  }
    printf("你可以输入下列命令来使用本TFTP客户端:\n");
    printf("请注意!本程序暂时只支持上传下载100M以下大小的文件\n");
	  printf("-从服务器端下载文件:\n");
	  printf("get [remote_file]\n");
	  printf("-向服务器端上传文件:\n");
	  printf("put [local_file]\n");
    printf("-获取服务器端的文件目录\n");
    printf("list\n");
	  printf("-退出客户端程序:\n");
	  printf("quit\n");

    server_ip = argv[1];
    if (argc > 2)
    {
        port = (unsigned short)atoi(argv[2]);
    }
    printf("本客户端已连接到位于%s ：%d的TFTP服务器端\n",server_ip, port);

    //创建数据连接socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
		    printf("客户端数据传输socket创建失败.\n");
		    return 0;
	  }

	  // Initialize server address
	  server.sin_family = AF_INET;
	  server.sin_port = htons(port);
    //将字符形式的ip地址转换为网络字节序形式并存储到server中
    if (inet_pton(AF_INET, server_ip, &(server.sin_addr.s_addr)) != 1)
    {
        printf("服务器端地址信息设置出错！");
        return 0;
    }
    //开始处理用户输入的命令行
    char *buffer;
    char *arg;//用来存储输入的命令
    while (true)
    {
        printf(">>");
        memset(command, 0, MAX_BUFFER_SIZE);
        // 从输入流中读取数据到command数组中
        buffer = fgets(command, MAX_BUFFER_SIZE, stdin);
        if (buffer == NULL)
        {
            printf("读取命令失败！\n");
            return 0;
        }
        //利用strtok()切割输入的命令  获取具体的命令
        arg = strtok(buffer, " \t\n");
        if (arg == NULL)
        {
          continue;//结束本次while循环进行下一个while循环，而不是跳出while循环
        }
        // 进一步判断输入的命令是哪一个
        if (strcmp(arg, "get") == 0)
        {

            arg = strtok(NULL, " \t\n");
            if (arg == NULL)
            {
                printf("缺少文件名\n");
            }else
            {
                get_file(arg);
            }
        }
        else if (strcmp(arg, "put") == 0)
        {

            arg = strtok(NULL, " \t\n");
            if (arg == NULL)
            {
                printf("缺少文件名\n");
            }else
            {
                put_file(arg);
            }
        }
        else if (strcmp(arg, "list") == 0)
        {
            do_list();
        }
        else if (strcmp(arg, "quit") == 0)
        {
            break;
        }
        else
        {
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
void get_file(char *server_file)
{
    struct tftp_packet ack_packet, sed_packet, recv_packet;//sed_packet用来发送请求
    struct sockaddr_in client;

    // send the TFTP request
    sed_packet.optcode = htons(OPTCODE_RRQ);
    sprintf(sed_packet.filename, "%s%c%s%c%d%c", server_file, 0, "octet", 0, blocksize, 0);//将信息存储到packet中 以/-"filename"-"0"-"mode"-"0"-"blocksize"-"0"/
    sendto(sockfd, &sed_packet, sizeof(struct tftp_packet), 0, (struct sockaddr*)&server, addr_len);//发送这个报文到服务器端

    FILE *fp = fopen(server_file, "w");
    if (fp == NULL)
    {
        printf("本地文件：%s创建失败\n", server_file);
        return;
    }

    //接收数据 并发送ACK(停止等待机制、超时重传机制)
    int time_wait_counter = 0;
    int recv_size = 0;
    int write_size =0;
    unsigned short block = 1;//块号

    do{
        for(time_wait_counter = 0; time_wait_counter < MAX_RETRANSMISSION * MAX_TIME_WAIT; time_wait_counter += 10000)
        {
            recv_size = recvfrom(sockfd, &recv_packet, sizeof(struct tftp_packet), MSG_DONTWAIT,
                                (struct sockaddr *)&client,
                                &addr_len);
            if (recv_size > 0 && recv_size < 4)
            {
                printf("接收DATA报文出错！等待重传！\n");
            }
            if ((recv_size >= 4) && (recv_packet.optcode == htons(OPTCODE_DATA)) && (recv_packet.block == htons(block)))
            {
                printf("正在接收第%d个文件块...\n", block);
                write_size = fwrite(recv_packet.data, 1, recv_size - 4, fp);
                if (write_size == recv_size - 4)
                {
                    break;
                }
                else
                {
                    printf("数据写入文件失败，等待报文重传！\n");
                }
            }
            usleep(10000);
        }
        if (time_wait_counter >= MAX_TIME_WAIT * MAX_RETRANSMISSION)
        {
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
void put_file(char *local_file)
{
    struct tftp_packet  send_packet, recv_packet;
    struct sockaddr_in client;
    int time_wait_counter = 0;
    int recv_size = 0;
    //客户端在上传文件时发送的第一个包为 WRQ报文，用来发出上传文件请求 后续发出的报文为DATA报文
    send_packet.optcode = htons(OPTCODE_WRQ);

    sprintf(send_packet.filename, "%s%c%s%c%d%c", local_file, 0, "octet", 0, blocksize, 0);//将信息存储到packet中 以/-"filename"-"0"-"mode"-"0"-"blocksize"-"0"/
    sendto(sockfd, &send_packet, sizeof(struct tftp_packet), 0, (struct sockaddr*)&server, addr_len);//发送请求报文到服务器端
    //等待服务器端发来的第一个ACK 用来响应客户端的请求
    for (time_wait_counter = 0; time_wait_counter < MAX_TIME_WAIT; time_wait_counter += 20000)
    {
        recv_size = recvfrom(sockfd, &recv_packet, sizeof(struct tftp_packet), MSG_DONTWAIT,
                            (struct sockaddr *)&client,
                            &addr_len);

        if (recv_size > 0 && recv_size < 4)
        {
            printf("没有收到服务器端的响应报文，请等待重传！\n");
        }
        if ((recv_size >= 4) && (recv_packet.optcode == htons(OPTCODE_ACK)) && (recv_packet.block == htons(0)))
        {
            printf("收到服务服务器端发来的对应客户端上传请求的ACK报文！\n");
            break;
		    }
        usleep(20000);
    }
    if (time_wait_counter >= MAX_TIME_WAIT)
    {
        printf("接收请求的ACK报文超时！\n");
        return;
    }

    /*
     *开始上传文件
     *模式r对应的读取本地文件数据
     */
    FILE *fp = fopen(local_file, "rb");
	  if (fp == NULL)
    {
		    printf("您要上传的文件不存在，请检查文件名后重试！\n");
		    return;
	  }

    unsigned short block = 1; //块号
    // send_packet后续用来发送DATA包，所以需要覆盖操作码类型
    send_packet.optcode = htons(OPTCODE_DATA);
    int content_size = 0;
    int send_times = 0;
    do{
        //将原来存储的数据清空，防止传输数据出现错误
        memset(send_packet.data, 0, sizeof(send_packet.data));
        send_packet.block = htons(block);
        content_size = fread(send_packet.data, 1, blocksize, fp);   //从文件中读取数据到data_packet中
        //发送一个数据包 超时重传机制
        for (send_times = 0; send_times < MAX_RETRANSMISSION; send_times++)
        {
            sendto(sockfd, &send_packet, content_size + 4, 0, (struct sockaddr*)&client, addr_len);
            printf("正在上传第%d个文件块\n", block);
            //等待ack报文 确认服务器端收到了
            for (time_wait_counter = 0; time_wait_counter < MAX_TIME_WAIT; time_wait_counter += 20000 )
            {
                recv_size = recvfrom(sockfd, &recv_packet, sizeof(struct tftp_packet), MSG_DONTWAIT,
                                    (struct sockaddr *)&client,
                                    &addr_len);
                if (recv_size > 0 && recv_size < 4)
                {
                    printf("没有收到服务器端的确认报文，请等待重传！\n");
                }
                if ((recv_size >= 4) && (recv_packet.optcode == htons(OPTCODE_ACK)) && (recv_packet.block == htons(block)))
                {
                    break;
                }
                usleep(20000);
            }
            if (time_wait_counter < MAX_TIME_WAIT)
            {
                printf("收到第%d个文件块的ACK报文。\n",block);
                break;
            }
        }
        if (send_times >= MAX_RETRANSMISSION)
        {
            printf("第%d个文件块传送失败！\n",block);
            fclose(fp);
            return;
        }
        block++;
    }while(content_size == blocksize);

    printf("%s文件上传成功！\n", local_file);
    fclose(fp);
    return;
}

/**
 * do_list  用来接收并解析服务器端对list请求发回来的数据包
 */
void do_list()
{
    struct tftp_packet send_packet, ack_packet, recv_packet; //用来接收数据和发送ACK数据
    struct sockaddr_in client;
    int time_wait_counter = 0;
    int recv_size = 0;
    unsigned short block = 1;

    ack_packet.optcode = htons(OPTCODE_ACK);
    send_packet.optcode = htons(OPTCODE_LIST);
    //只发送一个操作码即可
    sendto(sockfd, &send_packet, sizeof(struct tftp_packet), 0, (struct sockaddr*)&server, addr_len);
    printf("文件类型\t大小\t文件名\n");

    //开始接收数据并发送ack报文
    do
    {
        for (time_wait_counter = 0; time_wait_counter < MAX_TIME_WAIT * MAX_RETRANSMISSION; time_wait_counter += 20000)
        {
            recv_size = recvfrom(sockfd, &recv_packet, sizeof(struct tftp_packet), MSG_DONTWAIT,
                                (struct sockaddr *)&client,
                                &addr_len);
            if (recv_size > 0 && recv_size < 4)
            {
                printf("接收DATA报文出错！等待重传！\n");
            }
            if ((recv_size >= 4) && (recv_packet.optcode == htons(OPTCODE_DATA)) && (recv_packet.block == htons(block)))
            {
                //data 报文的话将数据写到显示屏上
                fwrite(recv_packet.data, 1, recv_size - 4, stdout);
                break;
            }
            usleep(20000);
        }
        if (time_wait_counter >= MAX_TIME_WAIT * MAX_RETRANSMISSION)
        {
            printf("接收DATA报文超时！\n");
            return;//跳出while循环
        }
        //若未超时发送ack报文
        ack_packet.block = htons(block);
        sendto(sockfd, &ack_packet, sizeof(struct tftp_packet), 0, (struct sockaddr*)&client, addr_len);
        block++;
    }while(recv_size == blocksize + 4);
    return;
}
