#pragma once

#include <string>
#include <netinet/in.h>
#include "SocketOops.h"
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
class InetAddress
{
public:
    explicit InetAddress(uint16_t port = 0,bool loopbackOnly = false,bool ipv6 = false);
    
    explicit InetAddress(const struct sockaddr_in& addr)
        : addr_(addr)
    {}
   
    sa_family_t family() const {return addr_.sin_family;}
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t port() const;
    uint16_t portNetEndian() const { return addr_.sin_port; }
    const struct sockaddr* getSockAddr() const {return sockets::sockaddr_cast(&addr6_);}
private:
    union
    {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};