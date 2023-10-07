#include "Acceptor.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

Acceptor::Acceptor(EventLoop *_loop, std::string listenAddr) : loop(_loop), sock(nullptr), acceptChannel(nullptr) {
    sock = new Socket();
    std::string::size_type pos =  listenAddr.find(":");
    if(pos == std::string::npos) {
        fprintf(stderr, "listenAddress set error: %s\n", listenAddr.c_str());
        std::exit(-1);
    }
    InetAddress addr = InetAddress(listenAddr.substr(0, pos).c_str(), static_cast<std::uint16_t>(std::stoul(listenAddr.substr(pos+1))));
    sock->bind(addr);
    sock->listen();
    sock->setnonblocking();
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
    fprintf(stderr, "New clint fd %d! IP: %s Port: %d\n", clnt_sock->getFd(), inet_ntoa(clnt_addr.getAddr().sin_addr), ntohs(clnt_addr.getAddr().sin_port));
    clnt_sock->setnonblocking();
    clnt_sock->setelegentclose();
    newConnectionCallback(clnt_sock);
}

void Acceptor::setNewConnectionCallback(std::function<void(Socket*)> _cb) {
    newConnectionCallback = _cb;
}