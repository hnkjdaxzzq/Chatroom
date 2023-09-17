#pragma once
#include <functional>
#include <Buffer.h>

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

    void echo(int sockfd);
    void setDeleteConnectionCallback(std::function<void(Socket*)>);
};