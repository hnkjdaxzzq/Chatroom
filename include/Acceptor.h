#pragma once
#include <cstdint>
#include <functional>
#include <string>

class EventLoop;
class Socket;
class Channel;

class Acceptor
{
private:
    EventLoop *loop;
    Socket *sock;
    Channel *acceptChannel;
    std::function<void(Socket*)> newConnectionCallback;
public:
    Acceptor(EventLoop *_loop, std::uint16_t port);
    ~Acceptor();
    void acceptConnection();
    void setNewConnectionCallback(std::function<void(Socket*)>);
};