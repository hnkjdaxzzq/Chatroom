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
    std::function<void(Connection*)> connectiontask;
public:
    Server(EventLoop*, std::function<void(Connection*)> contask = nullptr);
    ~Server();

    void setConnectionTask(std::function<void(Connection*)> task);
    void handleReadEvent(int);
    void newConnection(Socket *sock);
    void deleteConnection(Socket *sock);
};