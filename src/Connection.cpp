#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include "Rio.h"
#include "util.h"
#include <functional>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>

const int READ_BUFFER = 2048;

Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock), channel(nullptr) {
    // channel = new Channel(loop, sock->getFd());
    rio = new Rio(sock->getFd());
    // std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    // channel->setReadCallback(cb);
    // channel->enableReading();
    // channel->useET();
}

Connection::~Connection() {
    delete channel;
    delete sock;
    delete rio;
}

ssize_t Connection::creadn(char *usrbuf, size_t n) {
    return rio->rio_readn(usrbuf, n);
}

ssize_t Connection::creadnb(char *usrbuf, size_t n) {
    return rio->rio_readnb(usrbuf, n);
}

ssize_t Connection::cwriten(char *usrbuf, size_t n) {
    return rio->rio_writen(usrbuf, n);
}

void Connection::Do(std::function<void (Connection*)> task) {
    if (! task) {
        task = [&](Connection*) {echo(sock->getFd());};
    }
    std::function<void()> callback = std::bind(task, this);
    channel = new Channel(loop, sock->getFd());
    channel->setReadCallback(callback);
    channel->enableReading();
    channel->useET();
}

void Connection::echo(int sockfd) {
    char buf[READ_BUFFER];
    while(true) {    //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        bzero(&buf, sizeof(buf));
        ssize_t bytes_read = rio->rio_readnb(buf, sizeof(buf));
        if(bytes_read > 0){
            readBuffer.Append(buf);
        } else if(bytes_read == -1 && errno == EINTR){  //客户端正常中断、继续读取
            printf("continue reading");
            continue;
        } else if(bytes_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
            printf("message from client fd %d: %s\n", sockfd, readBuffer.c_str());
            errif(rio->rio_writen((void *)readBuffer.c_str(), sizeof(buf)) == -1, "socket write error");
            readBuffer.clear();
            break;
        } else if(bytes_read == 0){  //EOF，客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd);
            // close(sockfd);   //关闭socket会自动将文件描述符从epoll树上移除
            deleteConnectionCallback(sock);
            break;
        }
    } 
}

void Connection::setDeleteConnectionCallback(std::function<void(Socket*)> _cb) {
    deleteConnectionCallback = _cb;
}