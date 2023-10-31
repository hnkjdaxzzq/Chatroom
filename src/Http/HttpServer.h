#pragma once

#include <mutex>
#include <string>
#include "Acceptor.h"
#include "EventLoop.h"
#include "Http.h"
#include <Connection.h>
#include <thread>
#include <unordered_map>
#include <HttpConnection.h>
#include <vector>
#include <ThreadPool.h>
class HttpServer {
public:
    HttpServer(uint16_t port, const std::string& srcDir, int threadSize = std::thread::hardware_concurrency());
    void newConnection(Socket *sock);
    void deleteConnection(Socket *sock);
    void process(Connection* con);

    void DealRead_(HttpConnection* con);
    void DealWrite_(HttpConnection* con);

    void start();
private:
    uint16_t port_;
    std::string srcDir_;
    int threadSize_;
    bool isClose_;

    
    
    EventLoop *mainReactor;
    std::vector<EventLoop*> subReactors;
    ThreadPool *thpool;

    Acceptor *acceptor;
    std::unordered_map<int, HttpConnection*> users_;
    std::mutex mtx_;

    bool init();
};