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
    Rio *rio;
    Buffer readBuffer;
    std::function<void(Socket*)> deleteConnectionCallback;
public:
    Connection(EventLoop *_loop, Socket *_sock);
    ~Connection();

    ssize_t creadn(char *usrbuf, size_t n);
    ssize_t creadnb(char *usrbuf, size_t n);
    ssize_t cwriten(char *usrbuf, size_t n);

    void Do(std::function<void(Connection*)> task);
    void echo(int sockfd);
    void setDeleteConnectionCallback(std::function<void(Socket*)>);
};