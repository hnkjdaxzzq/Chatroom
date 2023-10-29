#include "Server.h"
#include "Socket.h"
#include "Acceptor.h"
#include "Connection.h"
#include "EventLoop.h"
#include <functional>
#include <string>
#include <Log.h>

Server::Server(EventLoop *_loop, std::string listenAddr, std::function<void(Connection*)> contask) : mainReator(_loop), acceptor(nullptr), connectiontask(contask) {
    Log::Instance()->init(0, "./log", ".log", 1000);
    acceptor = new Acceptor(mainReator, 8888);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);
    if(! connectiontask) {
        LOG_DEBUG("Don't set connection task, use default echo server\n");
    }
    int size = 2; //std::thread::hardware_concurrency();
    thpool = new ThreadPool(size);
    for(auto i = 0; i < size; ++i) {
        subReactors.emplace_back(new EventLoop());
    }

    for(auto i = 0; i < size; ++i) {
        std::function<void()> sub_loop = std::bind(&EventLoop::loop, subReactors[i]);
        thpool->enqueue(sub_loop);
    }

}

Server::~Server() {
    delete acceptor;
}

void Server::newConnection(Socket *sock) {
    int random = sock->getFd() % subReactors.size();
    Connection *conn = new Connection(subReactors[random], sock);
    std::function<void(Socket*)> cb = std::bind(&Server::deleteConnection, this, std::placeholders::_1);
    conn->setDeleteConnectionCallback(cb);
    connections[sock->getFd()] = conn;
    conn->Do(connectiontask);
}

void Server::setConnectionTask(std::function<void (Connection*)> task) {
    connectiontask = task;
}

void Server::deleteConnection(Socket *sock) {
    Connection *conn = connections[sock->getFd()];
    connections.erase(sock->getFd());
    delete conn;
}