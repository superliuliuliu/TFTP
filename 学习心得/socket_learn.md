### socket编程之send、recv函数与sendto、recvfrom

#### socket的运作


#### sendto()和recvfrom()
标准定义如下
~~~
#include<sys/socket.h>

ssize_t recvfrom(int sockfd, void *buffer, size_t length, int flags
        struct sockaddr *src_addr, socklen_t addrlen);
            Return numbers of bytes received, 0 on EOF,-1 on error.
ssize_t sendto(int sockfd,const void *buffer, size_t length, int flags
        struct sockaddr *dest_addr, socklen_t addrlen);
            Return numbers of bytes send,-1 on error.
~~~
上述函数中sockfd代表socket的文件描述符、buffer参数用来提供发送或者接收数据的内存缓冲区地址、length则表示最多能够发送或者接收的字节数。返回值代表实际接收或者发送的字节数。flags则是一个特殊的位掩码，用来设置socket特定的I/O属性，当无需使用特性是，flags的值设置为0.

对于recvfrom函数，src_addr和addrlen参数会用来返回发送数据报的远程socket地址和该src_addr指针指向结构体的大小。
对于sendto函数，src_addr和addrlen参数则指定了报文要发送到的socket地址信息。

#### send()和recv()
~~~
int send(int fd,const void *buffer, size_t length, int flags);
         return numbers of bytes send,-1 on error.
int recv(int fd,const void *buffer, size_t length, int flags);
         return numbers of bytes recieved, 0 on EOF, -1 on error.
~~~

recv()和send()系统调用可以在已经连接的套接字上执行I/O操作。
flags选项：

#### bind()
