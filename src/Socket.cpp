#include "../include/Socket.h"
#include "../include/InetAddress.h"

Socket::Socket() : fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create error");    
}

Socket::Socket(int _fd) : fd(_fd) {
    errif(fd == -1, "socket create error");
}

void Socket::bind(InetAddress& _addr) {
    struct sockaddr_in serv_addr;
    serv_addr = _addr.getAddr();
    errif(::bind(fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}

int Socket::accept(InetAddress& _addr) {
    struct sockaddr_in clnt_addr;
    socklen_t clnt_len;
    int clntFd = ::accept(fd, (sockaddr*)&clnt_addr, &clnt_len);
    errif(clntFd == -1, "socket accept error");
    _addr.setAddr(clnt_addr);
    _addr.setAddrlen(clnt_len);
    return clntFd;
}