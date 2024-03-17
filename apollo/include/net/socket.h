#ifndef __APOLLO_SOCKET_H__
#define __APOLLO_SOCKET_H__

namespace apollo
{
class InetAddress;

//套接字封装类
class Socket
{
public:
    explicit Socket(int sockfd):sockfd_(sockfd){}
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    ~Socket();

    //返回socket描述符
    int fd()const { return sockfd_;}

    //绑定IP地址和端口号
    void bindAddress(const InetAddress& localAddr);

    //开启套接字监听
    void listen();

    //接收客户端连接
    int accept(InetAddress* peerAddr);

    //关闭写端
    void shutdownWrite();

    //设置TCP Nagle算法的开启与关闭
    void setTcpNoDelay(bool on);

    //设置复用IP地址选项的开启与关闭
    void setReuseAddr(bool on);

    //设置复用端口号选项的开启与关闭
    void setReusePort(bool on);

    //设置TCP保活选项的开启与关闭
    void setKeepAlive(bool on);


private:
    const int sockfd_;

};
}

#endif