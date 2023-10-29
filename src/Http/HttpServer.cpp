
#include "HttpServer.h"
#include <Connection.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include "Acceptor.h"
#include "EventLoop.h"
#include "Http.h"
#include <Log.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <thread>
#include <Socket.h>
#include <Channel.h>
#include <unistd.h>

void HttpServer::process(Connection * con) {
    std::unique_ptr<Http> http_server = std::make_unique<Http>(srcDir_);
    http_server->process(con);
}

HttpServer::HttpServer(uint16_t port, const std::string &srcDir, int threadSize ) : 
                port_(port), srcDir_(srcDir), threadSize_(threadSize) {

}

bool HttpServer::init() {
    mainReactor = new EventLoop();
    acceptor = new Acceptor(mainReactor, port_);
    std::function<void(Socket*)> connectCb = std::bind(&HttpServer::newConnection, this, std::placeholders::_1);
    acceptor->setNewConnectionCallback(connectCb);

    thpool = new ThreadPool(threadSize_);
    for (auto i = 0; i < threadSize_; ++i) {
        subReactors.emplace_back(new EventLoop());
    }

    for(auto i = 0; i < threadSize_; ++i) {
        std::function<void()> sub_loop = std::bind(&EventLoop::loop, subReactors[i]);
        thpool->enqueue(sub_loop);
    }
    return true;
}

void HttpServer::start() {
    init();
    Log::Instance()->init(0);
    LOG_INFO("=========== HttpServer started ============");
    LOG_INFO("Port:%d", port_);
    LOG_INFO("srcDur: %s", srcDir_.c_str());
    LOG_INFO("ThreadPool num: %d", threadSize_);
    LOG_INFO("===========================================");
    mainReactor->loop();
}

void HttpServer::DealRead_(HttpConnection *con) {
    ssize_t ret = -1;
    int Errno = 0;
    ret = con->read(&Errno);
    if(ret == 0) { // 如果客户端断开连接，服务端也关闭连接
        int delFd = con->con_->getFd();
        LOG_INFO("client fd: %d closed connection", delFd);
        // if(con->con_->readBuffer.ReadableBytes() == 0) {
        //     con->Close();
        //     delete con;
        // }
        // return;
    }
    
    if(ret < 0 && ! (Errno == EAGAIN || Errno == EWOULDBLOCK)) {
        // 读取socket内容出错，关闭客户端连接
        con->Close();
        return;
    }
    
    Channel *chan = con->con_->getChannel();
    if(con->con_->readBuffer.ReadableBytes() > 0 && con->process() == true) {
        // 如果读缓冲区还有数据，则需要注册写事件
        chan->setEvents(EPOLLET | EPOLLOUT | EPOLLONESHOT);
        chan->update();
    }

    if(con->con_->readBuffer.ReadableBytes() > 0 && con->process() == false) {
        // 如果缓冲区有数据，但是数据解析失败，则可能数据每收全，继续注册读事件
        chan->setEvents(EPOLLET | EPOLLIN | EPOLLONESHOT | EPOLLPRI);
        chan->update();    
    }

}

void HttpServer::DealWrite_(HttpConnection *con) {
    ssize_t ret = -1;
    int Erron = 0;
    ret = con->write(&Erron);
    if(ret >= 0 && con->ToWriteBytes() == 0 && con->isKeepAlive() == false && con->con_->isClosed()) {
        // HttpResponse传输完成，且客户端不期待保持连接，服务端直接关闭连接
        con->Close();
        delete con;
        return;
    }

    if(ret >= 0 && con->ToWriteBytes() == 0 && con->isKeepAlive() && ! con->con_->isClosed()) {
        // HttpResponse传输完成，且客户端希望保持连接，继续监听可读事件
        Channel *chan = con->con_->getChannel();
        chan->setEvents(EPOLLET | EPOLLIN | EPOLLONESHOT | EPOLLPRI);
        chan->update();    
        return;
    }


    if(con->ToWriteBytes() != 0 && ret >= 0) {
        // 如果数据没传输完成且传输过程中没出错，继续监听可写事件
        Channel *chan = con->con_->getChannel();
        chan->setEvents(EPOLLET | EPOLLOUT | EPOLLONESHOT);
        chan->update();

    } else if(ret < 0 && !(Erron == EAGAIN || Erron == EWOULDBLOCK) ) {
        // 如果传输过程出错
        LOG_ERROR("HttpResponse send failed");
        con->Close();
        delete con;
    }
}

void HttpServer::newConnection(Socket *sock) {
    int random = sock->getFd() % subReactors.size();
    Connection* con = new Connection(subReactors[random], sock);
    if(con == nullptr) {
        LOG_ERROR("Memory not enough!!!");
        close(sock->getFd());
        delete sock;
        return;
    }
    HttpConnection *httpcon = new HttpConnection(srcDir_);
    if(httpcon->init(con) == false) {
        LOG_ERROR("fd:%d http connection create failed", sock->getFd());
        return;
    }

    users_[sock->getFd()] = httpcon;

    std::function<void(Socket*)> delConCb = std::bind(&HttpServer::deleteConnection, this, std::placeholders::_1);
    httpcon->con_->setDeleteConnectionCallback(delConCb);

    std::function<void()> readcb = std::bind(&HttpServer::DealRead_, this, httpcon);
    std::function<void()> writecb = std::bind(&HttpServer::DealWrite_, this, httpcon);


    Channel *chan = httpcon->con_->getChannel();
    chan->setReadCallback(readcb);
    chan->setWriteCallback(writecb);
    chan->setEvents(EPOLLET | EPOLLIN | EPOLLONESHOT | EPOLLPRI);
    chan->update();
}

void HttpServer::deleteConnection(Socket *sock) {
    int delfd = sock->getFd();
    if(users_.count(delfd)) {
        HttpConnection *httpcon = users_[delfd];
        users_.erase(delfd);
        httpcon->con_->getChannel()->delChannel();
        // delete sock;
    }
}
