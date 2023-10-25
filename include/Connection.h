#pragma once
#include <cstddef>
#include <functional>
#include <Buffer.h>
#include <sys/types.h>

class EventLoop;
class Socket;
class Channel;
class Rio;

class Connection
{
private:
    EventLoop *loop;
    Socket *sock;
    Channel *channel;
    // Rio *rio; // 不用
    void echo(int sockfd);
    bool closed;
public:
    Buffer readBuffer;
    Buffer writeBuffer;
    std::function<void(Socket*)> deleteConnectionCallback;
    void setOneshot();

    Connection(EventLoop *_loop, Socket *_sock);
    ~Connection();

    bool isClosed() const ;
    int getFd() const ;
    Socket* getSocket() const ;
    Channel* getChannel() ;

    ssize_t readNonBlocking(int *Errno);
    ssize_t writeNonBlocking(int *Errno);

    // ssize_t creadn(char *usrbuf, size_t n);
    // ssize_t creadnb(char *usrbuf, size_t n);
    // ssize_t cwriten(const char *usrbuf, size_t n);

    void Do(std::function<void(Connection*)> task);
    void setDeleteConnectionCallback(std::function<void(Socket*)>);
};