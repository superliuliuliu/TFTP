# include "tftp.h"


/*
 *服务器端运行主函数
 * 主进程：一直运行的进程   创建一个监听socket
 *
 */
int main(int argc, char **argv){
    unsigned short port = DEFAULT_PORT;
    printf("欢迎使用TFTP服务器端！\n");
    printf("--------------------------------------------\n");
    printf("使用指南：\n");
    printf("\nUsage:  %s [port]\n",argv[0]);
    printf("可用端口号:1024-65535\n");
    if (argc > 1){
        port = (unsigned short)atoi(argv[1]);
    }
    printf("\n如果你未设置服务器端的端口号,端口号默认为7341。\n");
    int sockfd;
    pthread_t t_id;               //线程ID
	  struct sockaddr_in server;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
        printf("服务器端监听socket创建失败！请检查后重新运行！\n");
        return -1;
    }
    server.sin_family = AF_INET;
	  server.sin_addr.s_addr = INADDR_ANY;
	  server.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
        printf("监听socket绑定失败！\n");
        return -1;
    }

    printf("服务器开始运行: localhost:%d.\n", port);

    //接收request 创建线程处理请求
    struct tftp_request *request;
    int addr_len = sizeof(struct sockaddr_in);
    //监听进程一直在运行
    while(true){
        printf("此时与服务器端建立连接数为:%d", connect_counter);
        // 指向结构体的指针需要用malloc分配内存
        request = (struct tftpx_request *)malloc(sizeof(struct tftpx_request));
		    memset(request, 0, sizeof(struct tftpx_request));
        request->size = recvfrom(
				sockfd, &(request->packet), MAX_PACKET_REQUEST, 0,
				(struct sockaddr *) &(request->client),
				&addr_len);
        request->packet.optcode = ntohs(request->packet.cmd);//转码 便于线程的switch函数
        //判断request->size?
        printf("请求报文成功接收！\n");
        //创建线程
        

    }
    return 0;
}
