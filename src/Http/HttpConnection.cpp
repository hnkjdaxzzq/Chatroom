#include "Log.h"
#include <HttpConnection.h>
#include <atomic>
#include <string>
#include <Connection.h>
#include <sys/types.h>


std::atomic<int> HttpConnection::httpConnetCount;

HttpConnection::HttpConnection(std::string &srcDir) : srcDir_(srcDir), isClose_(false) {

}

bool HttpConnection::isKeepAlive() const {
    return request_.IskeepAlive();
}

void ::HttpConnection::Close() {
    if(con_ != nullptr) {
        con_->deleteConnectionCallback(con_->getSocket());
    }
    isClose_ = true;
}

bool HttpConnection::init(Connection *con) {
    if(con == nullptr || con->isClosed()) {
        LOG_ERROR("can't get tcp connection");
        isClose_ = true;
        return false;
    }

    con_.reset(con);
    httpConnetCount++;
    con_->readBuffer.RetrieveAll();
    con_->writeBuffer.RetrieveAll();
    // 如果要打印新连接的用户数，会造成线程不安全，需要引入同步机制。
    // 但是在此处加锁会引起http连接对锁的争用，因此在此处不打印新连接的用户数
    LOG_DEBUG("Http Clinet[%d] in", con_->getFd()) ;    
    return true;
}

ssize_t HttpConnection::read(int *Errno) {
    return con_->readNonBlocking(Errno);
}

ssize_t HttpConnection::write(int *Errno) {
    return con_->writeNonBlocking(Errno);
}

int HttpConnection::ToWriteBytes() {
    return con_->writeBuffer.ReadableBytes();
}

bool HttpConnection::process() {
    if(con_->readBuffer.ReadableBytes() <=0 ) {
        return false;
    }
    else if (request_.parse(con_->readBuffer)) {
        LOG_DEBUG("fd[%d] HttpRequest parsed successfully get file %s", con_->getFd(),request_.getUri().c_str());
        response_.init(srcDir_, request_.getUri(), request_.IskeepAlive(), 200);
    } else {
        LOG_ERROR("fd[%d] HttpRequest parsed failed!!!", con_->getFd());
        LOG_ERROR("fd[%d] %s", con_->getFd(), con_->readBuffer.GetBufferToStr().c_str());
        response_.init(srcDir_, request_.getUri(), false, 400);
    }

    // 添加响应头
    response_.MakeResponse(con_->writeBuffer);

    // 添加请求文件
    if(response_.FileLen() > 0 && response_.File()) {
        con_->writeBuffer.Append(response_.File(), response_.FileLen());
        response_.UnmapFile();
    }
    LOG_DEBUG("filesize: %d", con_->writeBuffer.ReadableBytes());
    
    return true;
}