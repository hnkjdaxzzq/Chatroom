#pragma once

#include <functional>
#include <map>
#include <vector>
#include "ThreadPool.h"
class EventLoop;
class Socket;
class Acceptor;
class Connection;

class Server
{
private:
    EventLoop *mainReator;
    Acceptor *acceptor;
    std::map<int, Connection*> connections;
    std::vector<EventLoop*> subReactors;
    ThreadPool *thpool;
    std::function<void()> connectiontask;
public:
    Server(EventLoop*, std::function<void()> contask = nullptr);
    ~Server();

    void setConnectionTask(std::function<void()> task);
    void handleReadEvent(int);
    void newConnection(Socket *sock);
    void deleteConnection(Socket *sock);
};