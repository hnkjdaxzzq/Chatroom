#include "../include/Server.h"
#include "../include/Socket.h"
#include "../include/Acceptor.h"
#include "../include/Connection.h"
#include "../include/EventLoop.h"
#include <functional>

Server::Server(EventLoop *_loop) : mainReator(_loop), acceptor(nullptr) {
    acceptor = new Acceptor(mainReator);
    std::function<void(Socket*)> cb = std::bind(&Server::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(cb);

    int size = std::thread::hardware_concurrency();
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
}

void Server::deleteConnection(Socket *sock) {
    Connection *conn = connections[sock->getFd()];
    connections.erase(sock->getFd());
    delete conn;
}