#include "Connection.h"
#include "Log.h"
#include "Socket.h"
#include "Channel.h"
#include "Rio.h"
#include "util.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <functional>
#include <string>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>

const int READ_BUFFER = 2048;

Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock), channel(nullptr), closed(false) {
    channel = new Channel(loop, sock->getFd());
    // rio = new Rio(sock->getFd());
    sock->setnodelay();
    // std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    // channel->setReadCallback(cb);
    // channel->enableReading();
    // channel->useET();
}

ssize_t Connection::readNonBlocking(int *Errno) {
    ssize_t readBytes = 0;
    ssize_t readn = -1;
    while (true) 
    {
        readn = readBuffer.ReadFd(sock->getFd(), Errno);
        if(readn > 0 ) {
            readBytes += readn;
            continue;
        }
        
        if (readn == 0) {
            // EOF，客户端断开连接
            // 暂时先不进行后续处理，等将http模块调通后，加入定时器
            // LOG_DEBUG("Client [%d] closed connection", sock->getFd());
            closed = true;
            return 0;
        }

        if(readn < 0 && *Errno == EINTR) {
            // 系统正常中断，继续读取数据
            continue;
        } else if( readn < 0 && ( *Errno == EAGAIN || *Errno == EWOULDBLOCK)) {
            // 读取数据完成
            if(readBytes != 0)
                break;
            else
                return 1;
        } else if( readn < 0) {
            LOG_ERROR("read fd[%d] error, %s", sock->getFd(), strerror(*Errno));
            return readn;
        }
    }
    return readBytes;
}

ssize_t Connection::writeNonBlocking(int *Errno) {
    ssize_t writeBytes = 0;
    ssize_t writen = 0;
    while (true) {
        writen = writeBuffer.WriteFd(sock->getFd(), Errno);
        
        if( writen == 0) {
            //数据发送完毕
            break;
        }

        if( writen > 0) {
            writeBytes += writen;
            continue;
        }

        if( writen < 0 && *Errno == EINTR) {
            continue;
        }
        else if (writen < 0 && *Errno == EPIPE) {
            LOG_ERROR("write mesage to fd:%d that was already closed", sock->getFd());
            closed = true;
            return writen;
        }
        else if( writen < 0 && *Errno == EBADF) {
            LOG_ERROR("fd:%d invalid or can't write", sock->getFd());
            closed = true;
            return writen;
        }
        else if (writen < 0 && (*Errno == EAGAIN || *Errno == EWOULDBLOCK)) {
            // 写阻塞了
            break;
        }
    }
    return writeBytes;
}

Connection::~Connection() {
    delete channel;
    // delete sock;
    // delete rio;
}

bool Connection::isClosed() const {
    return closed;
}

int Connection::getFd() const {
    return sock->getFd();
}

Channel* Connection::getChannel() {
    return channel;
}

void Connection::setOneshot() {
    channel->setOneshot();
}

Socket* Connection::getSocket() const {
    return sock;
}

// ssize_t Connection::creadn(char *usrbuf, size_t n) {
//     return rio->rio_readn(usrbuf, n);
// }

// ssize_t Connection::creadnb(char *usrbuf, size_t n) {
//     return rio->rio_readnb(usrbuf, n);
// }

// ssize_t Connection::cwriten(const char *usrbuf, size_t n) {
//     return rio->rio_writen((void*)usrbuf, n);
// }

void Connection::Do(std::function<void (Connection*)> task) {
    if (! task) {
        task = [&](Connection*) {echo(sock->getFd());};
    }
    std::function<void()> callback = std::bind(task, this);
    // channel = new Channel(loop, sock->getFd());
    channel->setReadCallback(callback);
    channel->setOneshot();
    channel->useET();
    channel->enableReading();
}

void Connection::echo(int sockfd) {
    // char buf[READ_BUFFER];
    while(true) {    //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        // bzero(&buf, sizeof(buf));
        int Errno = 0; 
        ssize_t bytes_read = readBuffer.ReadFd(sockfd, &Errno);
        if(bytes_read > 0){
            // readBuffer.Append(buf);
            continue;
        } else if(bytes_read == -1 && Errno == EINTR){  //客户端正常中断、继续读取
            printf("continue reading");
            continue;
        } else if(bytes_read == -1 && ((Errno == EAGAIN) || (Errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
            std::string msg = readBuffer.RetrieveAllToStr();
            printf("message from client fd %d: %s\n", sockfd, msg.c_str());
            writeBuffer.Append(msg);
            errif(writeBuffer.WriteFd(sockfd, &Errno) == -1, "socket write error");
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