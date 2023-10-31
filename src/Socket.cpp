#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <Log.h>

Socket::Socket() : fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create error");    
}

Socket::Socket(int _fd) : fd(_fd) {
    fd = _fd;
    errif(fd == -1, "socket create error");
}

Socket::~Socket() {
    LOG_DEBUG("fd[%d] closed", fd);
    if(fd != -1) {
        shutdown(fd, SHUT_WR);
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

void Socket::setnodelay() {
    int enable = 1;
    int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
    if(ret < 0) {
        LOG_ERROR("fd[%d] Close Nagle failed! %s", fd, strerror(errno));
    }
}

void Socket::setelegentclose() {
    struct linger optLinger = { 0 };
    /* 优雅关闭: 直到所剩数据发送完毕或超时 */
    optLinger.l_onoff = 1;
    optLinger.l_linger = 1;
    int ret = setsockopt(fd, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        LOG_ERROR("fd[%d] Init linger error! %s", fd, strerror(errno));
    }
    // errif(ret < 0, "Init linger error!");
}

int Socket::accept(InetAddress& _addr) {
    struct sockaddr_in clnt_addr;
    socklen_t clnt_len = sizeof(clnt_addr);
    bzero(&clnt_addr, sizeof(clnt_addr));
    int clntFd = -1;
    while (true) {
        clntFd = ::accept(fd, (sockaddr*)&clnt_addr, &clnt_len);
        if(clntFd > 0)
            break;

        if(clntFd < 0 && (errno == EMFILE || errno == ENFILE || errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) ) {
            continue;
        } else {
            LOG_ERROR("%s ", strerror(errno));
            exit(1);
        } 
    }
    // errif(clntFd < 0, "socket accept error");
    _addr.setAddr(clnt_addr);
    _addr.setAddrlen(clnt_len);
    return clntFd;
}