#pragma once

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
public:
    Server(EventLoop*);
    ~Server();

    void handleReadEvent(int);
    void newConnection(Socket *sock);
    void deleteConnection(Socket *sock);
};