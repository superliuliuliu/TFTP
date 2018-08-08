# include "client.h"


//服务器端的相关信息
int sockfd;
struct sockaddr_in server;
socklen_t addr_len = sizeof(struct sockaddr_in);

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
		    printf("\nUsage:  %s [server ip] [port]\n",argv[0]);
		    printf("服务器端的默认端口号为7341,,如果你已经设置请忽略！\n");
		    return 0;
	  }
    printf("你可以输入下列命令来使用本TFTP客户端:\n");
	  printf("  -从服务器端下载文件:\n");
	  printf("    get [remote_file]\n");
	  printf("  -向服务器端上传文件:\n");
	  printf("    put [local_file]\n");
	  printf("  -退出客户端程序:\n");
	  printf("    quit\n");

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
    char *;
    while (true){
        printf("<<");
        memset(command, 0, MAX_BUFFER_SIZE);
        // 从输入流中读取数据到command数组中
        buffer = fgets(command, MAX_BUFFER_SIZE, stdin);
        if (buffer == NULL){
            printf("读取命令失败！\n");
            return 0;
        }
        //strtok()切割输入的命令  获取具体的命令



    }
    return 0;
}

/**
 * get_file  客户端从服务器端下载远端文件
 * @param server_file [description]
 */
void get_file(char *server_file){

}

/**
 * put_file  客户端向服务器端上传本地文件
 * @param local_file [description]
 */
void put_file(char *local_file){

}
