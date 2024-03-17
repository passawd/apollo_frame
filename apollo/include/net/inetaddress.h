#ifndef __APOLLO_INET_ADDRESS_H__
#define __APOLLO_INET_ADDRESS_H__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

namespace apollo
{
class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0, const std::string& ip = "127.0.0.1");

    explicit InetAddress(const sockaddr_in& addr)
        : addr_(addr) { }

    //将IP地址转化为字符串
    std::string toIp() const;


    //将IP地址和端口号转化为字符串形式
    std::string toIpPort() const;

    //获取端口号
    uint16_t toPort() const;

    //Get the Sock Addr object
    const sockaddr_in* getSockAddr() const { return &addr_; }

    //设置IP地址和端口号
    void setSockAddr(const sockaddr_in& addr) { addr_ = addr; }


private:
    sockaddr_in addr_;

};


}
#endif