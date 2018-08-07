## socket之get/setsockopt()函数理解
最近在做的实训项目是搭建TFTP服务器和TFTP客户端，实现文件上传和文件下载功能。在做的时候学习到了一些关于socket编程的知识，在此记录下来，加深我的理解。
### 创建socket
~~~
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
         Return fd on success, or -1 on error.
~~~
上述函数用来创建一个socket。

其中参数domain 指定了socket的通信domain可选的值有
~~~
AF_UNIX(内核进程间信息传递)
AF_INET(IPV4网络通信协议 不同主机间通信)
AF_INET6(IPV6网络通信协议 不同主机间进行通信)。
~~~
参数 type指定了socket的类型
~~~
SOCK_DGRAM ：数据报socket TCP协议
SOCK_STEAM : 流socket    UDP协议
~~~
参数protocol常常被指定为0



### 函数定义
~~~
#include <sys/types.h>
#include <sys/socket.h>

int getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);
                 Returns -1 on error,or 1 on success
~~~
参数的定义：

sock：将要被设置选项或者取得选项套接字对应的文件描述符。

level：选项所在的协议层

optname：选项名

optval：对于getsockopt()，指向返回选项值的缓冲。对于setsockopt()，指向包含新选项值的缓冲。

optlen：对于getsockopt()，作为入口参数时，选项值的最大长度。作为出口参数时，选项值的实际长度。对于setsockopt()，指代选项的长度。

### 功能描述
顾名思义，从这两个函数的名字中我们大致可以了解这些的函数实现的功能，即获取或者设置与套接字所关联的选项。选项可能位于多层协议之中，当操作套接字选项时，必须指出选项位于的层数和选项的名称。当操作套接字层的选型时，需将协议层指定为SOL_SOCKET。

### 参数的详细说明
1.level 指定控制套接字的层次，可以取三种值:

SOL_SOCKET:通用套接字选项

IPPROTO_IP:IP选项

IPPROTO_TCP:TCP选项

2.optname：选项名,指定对套接字的控制方式。

![pict](https://myblogohoto.oss-cn-beijing.aliyuncs.com/blogphoto/pict.png?x-oss-process=style/BLOG-LGY)

#### SO_REUSERADDR参数的含义
在学习过程中看到别人的代码使用了SO_REYSERADDR参数不是很了解其作用，于是在网上收集其资料，在这里整理一下。(后续可能会更新其他参数的用法和作用)

在Linux操作系统中之中，我们知道一个端口在释放两分钟之后才能被重新使用（这一点涉及到LINUX中的TIME_WAIT）,在此SO_REUSERADDR参数的作用就是让端口在释放后就能立即重新使用。
SO_REUSERADDR用于TCP套接字处于TIME_WIAT状态的socket，server端往往会在在调用bind()（将套接字绑定到地址上）前设置SO_REUSERADDR选项。

SO_REUSERADDR选项提供以下四种功能：
1.SO_REUSEADDR允许启动一个监听服务器并捆绑其众所周知端口，即使以前建立的将此端口用做他们的本地端口的连接仍存在。这通常是重启监听服务器时出现，若不设置此选项，则bind时将出错。

2.SO_REUSEADDR允许在同一端口上启动同一服务器的多个实例，只要每个实例捆绑一个不同的本地IP地址即可。对于TCP，我们根本不可能启动捆绑相同IP地址和相同端口号的多个服务器。

3.SO_REUSEADDR允许单个进程捆绑同一端口到多个套接口上，只要每个捆绑指定不同的本地IP地址即可。这一般不用于TCP服务器。

4.SO_REUSEADDR允许完全重复的捆绑：当一个IP地址和端口绑定到某个套接口上时，还允许此IP地址和端口捆绑到另一个套接口上。一般来说，这个特性仅在支持多播的系统上才有，而且只对UDP套接口而言（TCP不支持多播）。

SO_REUSEPORT选项有如下语义：

1.此选项允许完全重复捆绑，但仅在想捆绑相同IP地址和端口的套接口都指定了此套接口选项才行。

2.如果被捆绑的IP地址是一个多播地址，则SO_REUSEADDR和SO_REUSEPORT等效。

SO_REUSERADDR选项的使用建议：
1.在TCP服务器中，在调用bind()将套接字绑定地址前，前设置SO_REUSERADDR选项.
2.当编写一个可以在同一时刻运行多次的多播应用程序时，设置SO_REUSERADDR选项,并将本多播程序的地址作为本地IP与其绑定。
