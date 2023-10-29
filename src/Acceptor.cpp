#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <Log.h>

Acceptor::Acceptor(EventLoop *_loop, uint16_t port) : loop(_loop), sock(nullptr), acceptChannel(nullptr) {
    sock = new Socket();
    InetAddress addr = InetAddress("0.0.0.0", port);
    sock->bind(addr);
    sock->listen();
    // sock->setnonblocking();
    acceptChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Acceptor::acceptConnection, this);
    acceptChannel->setReadCallback(cb);
    acceptChannel->enableReading();
}

Acceptor::~Acceptor() {
    delete sock;
    delete acceptChannel;
}

void Acceptor::acceptConnection() {
    InetAddress clnt_addr;
    Socket *clnt_sock = new Socket(sock->accept(clnt_addr));
    LOG_INFO("New TCP clint fd %d! IP: %s Port: %d", clnt_sock->getFd(), inet_ntoa(clnt_addr.getAddr().sin_addr), ntohs(clnt_addr.getAddr().sin_port));
    clnt_sock->setnonblocking();
    clnt_sock->setelegentclose();
    newConnectionCallback(clnt_sock);
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb) {
    newConnectionCallback = _cb;
}