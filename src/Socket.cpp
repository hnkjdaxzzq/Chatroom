#include "../include/Socket.h"
#include "../include/InetAddress.h"
#include <unistd.h>
#include <fcntl.h>

Socket::Socket() : fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create error");    
}

Socket::Socket(int _fd) : fd(_fd) {
    errif(fd == -1, "socket create error");
}

Socket::~Socket() {
    if(fd != -1) {
        close(fd);
        fd = -1;
    }
}

void Socket::bind(InetAddress& _addr) {
    struct sockaddr_in serv_addr;
    serv_addr = _addr.getAddr();
    errif(::bind(fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}

int Socket::getFd() {
    return fd;
}

void Socket::setnonblocking() {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int Socket::accept(InetAddress& _addr) {
    struct sockaddr_in clnt_addr;
    socklen_t clnt_len = sizeof(clnt_addr);
    bzero(&clnt_addr, sizeof(clnt_addr));
    int clntFd = ::accept(fd, (sockaddr*)&clnt_addr, &clnt_len);
    errif(clntFd == -1, "socket accept error");
    _addr.setAddr(clnt_addr);
    _addr.setAddrlen(clnt_len);
    return clntFd;
}