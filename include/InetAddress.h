#pragma once
#include<arpa/inet.h>
#include<cstring>

class InetAddress {
private:
    struct sockaddr_in addr;
    socklen_t addr_len;
public:
    InetAddress() : addr_len(sizeof(addr)) {
        bzero(&addr, sizeof(addr));
    }

    InetAddress(const char* ip, uint16_t port);

    inline sockaddr_in getAddr() {
        return addr;
    }

    inline void setAddr(struct sockaddr_in _addr) {
        addr = _addr;
    }

    inline void setAddrlen(socklen_t len) {
        addr_len = len;
    }

    inline socklen_t getAddrlen() {
        return addr_len;
    }
};