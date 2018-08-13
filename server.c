# include "tftp.h"

//全局变量
int connect_counter = 0;       //全局变量：记录连接的客户端数目
char *list = ".";              //目录变量  初始化为当前目录"."
struct thread_record customer[MAX_THREAD_SIZE];   //子线程数组
/*
 * 服务器端运行主函数
 *
 *
 */
int main(int argc, char **argv)
{
    bool enough = true;         //子线程是否足够可用
    unsigned short port = DEFAULT_PORT;
    printf("--------------------------------------------\n");
    printf("欢迎使用TFTP服务器端！\n");
    printf("使用指南：\n");
    printf("Usage:  %s [port]  ", argv[0]);
    printf("(可用端口号:1024-65535)\n");
    if (argc > 1)
    {
        port = (unsigned short)atoi(argv[1]);
    }
    printf("如果您未设置端口号，则服务器端的端口号默认初始值为7341。\n");
    int sockfd;
	  struct sockaddr_in server;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("服务器端监听socket创建失败，请检查后重新运行！\n");
        return -1;
    }
    server.sin_family = AF_INET;
	  server.sin_addr.s_addr = INADDR_ANY;
	  server.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("监听socket绑定失败!,请检查您输入的端口号是否符合要求。\n");
        return -1;
    }

    printf("服务器开始运行！\nlocalhost  %d.\n", port);

    //用于线程创建函数中的参数传递  该参数包含请求报文与线程id来标志时哪一个线程
    struct deliever_para *deliever;
    //struct tftp_request *request;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    //对线程数组进行初始化
    int i;
    for(i = 0; i < MAX_THREAD_SIZE; i++)
    {
        customer[i].usable = true;
    }
    //监听进程一直在运行
    while(true)
    {
        printf("当前与服务器端建立的连接数为:%d\n", connect_counter);
        // 指向结构体的指针需要用malloc分配内存
        deliever = (struct deliever_para *)malloc(sizeof(struct deliever_para));
        memset(deliever, 0, sizeof(struct deliever_para));
        deliever->request.size = recvfrom(
				sockfd, &(deliever->request.packet), MAX_PACKET_REQUEST, 0,
				(struct sockaddr *) &(deliever->request.client),
				&addr_len);
        deliever->request.packet.optcode = ntohs(deliever->request.packet.optcode);//转码 便于线程的switch函数*/
        printf("成功接收客户端的请求报文！\n");
        /*
         * 遍历线程数组，寻找可用线程（线程是否可用的标志usable）
         * 找到一个可用线程后对其进行修改、记录并跳出遍历
         * 若线程数组中的线程均不可用，表示服务器端连接的客户端数达到了饱和，则不进行工作
         */
        int idx;
        void *res;
        for (idx = 0; idx < MAX_THREAD_SIZE; idx++)
        {
            if (customer[idx].usable == true)
            {
                customer[idx].usable = false;
                deliever->thread_index = idx;//将可用线程下标记录下来，传递到线程start函数中，便于在结束任务后终结线程
                break;
            }
            else
            {
                enough = false;
            }
        }
        if (enough == true)
        {
            //创建线程
            pthread_create(&customer[idx].tid, NULL, thread_func, deliever);
            //连接已终止的线程
            pthread_join(customer[idx].tid, &res);
        }
        else
        {
            printf("当前服务器忙，请稍后重试！\n");
        }

    }
    return 0;
}
