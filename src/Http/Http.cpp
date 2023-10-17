#include "Http.h"
#include <Connection.h>
#include "util.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <chrono>
#include <strings.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <Log.h>

void Http::read(Connection *con) {
    static char buf[2048];
    // auto startTime = std::chrono::high_resolution_clock::now();
    
    // while(true) {
    //     // size_t headEnd = con->readBuffer.getBufText().find("\r\n\r\n");
    //     // if(headEnd != std::string::npos) {
    //     //     break;
    //     // }
    //     bzero(&buf, sizeof(buf));
    //     ssize_t byte_read = con->creadn(buf, sizeof(buf));
    //     if(byte_read > 0) {
    //         con->readBuffer.Append(buf);
            
    //     } else if(byte_read == -1 && errno == EINTR) {
    //         fprintf(stderr, "continue reading");
    //         continue;
    //     } else if(byte_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
    //         if(strlen(buf) != 0)
    //             con->readBuffer.Append(buf);
    //         fprintf(stderr, "read() message from client fd %d: length %ld, errno: %d\n", con->getFd(), con->readBuffer.size(), errno);
    //         // auto currentTime = std::chrono::high_resolution_clock::now();
    //         // auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
    //         // if (elapsedTime.count() > 5) {
    //         //     break;
    //         // }
    //         // perror("why can't read the data");
    //         break;
    //     } else if(byte_read == 0) {
    //         con->deleteConnectionCallback(con->getSocket());
    //         fprintf(stderr, "EOF, client fd %d disconnected\n", con->getFd());
    //         break;
    //     }
    // }
     while(true) {    //由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        // bzero(&buf, sizeof(buf));
        int Errno = 0; 
        ssize_t bytes_read = con->readBuffer.ReadFd(con->getFd(), &Errno);
        if(bytes_read > 0){
            // readBuffer.Append(buf);
            continue;
        } else if(bytes_read == -1 && Errno == EINTR){  //客户端正常中断、继续读取
            printf("continue reading");
            continue;
        } else if(bytes_read == -1 && ((Errno == EAGAIN) || (Errno == EWOULDBLOCK))){//非阻塞IO，这个条件表示数据全部读取完毕
            // std::string msg = readBuffer.RetrieveAllToStr();
            LOG_DEBUG("read() message from client fd %d: length %ld, errno: %d\n", con->getFd(), con->readBuffer.ReadableBytes(), Errno);
            // writeBuffer.Append(msg);
            // errif(writeBuffer.WriteFd(sockfd, &Errno) == -1, "socket write error");
            break;
        } else if(bytes_read == 0){  //EOF，客户端断开连接
            LOG_DEBUG("EOF, client fd %d disconnected\n", con->getFd());
            // close(sockfd);   //关闭socket会自动将文件描述符从epoll树上移除
            con->deleteConnectionCallback(con->getSocket());
            break;
        }
    } 
}

void Http::process(Connection *con) {
    read(con);
    request_ = std::make_unique<HttpRequest>();
    // printf("\n%s\n", con->readBuffer.c_str());
    try {
        request_->parse(con->readBuffer.RetrieveAllToStr());
        // con->readBuffer.clear();
        LOG_DEBUG("fd:%d ,process() path:%s\n", con->getFd(),request_->getUrl().c_str());
        LOG_DEBUG("process() parse httpRequest successful form fd:%d\n", con->getFd());
        response_ = std::make_unique<HttpResponse>(srcDir_, request_->getUrl(), request_->IskeepAlive(), 200);
        response_->MakeResponse(con->writeBuffer);

    } catch (const char* errmesg) {
        LOG_DEBUG("%s\n", errmesg);
        response_ = std::make_unique<HttpResponse>(srcDir_, request_->getUrl(), request_->IskeepAlive(), 400);
        response_->MakeResponse(con->writeBuffer);
    }

    /* 响应头 */
    iov_[0].iov_base = const_cast<char*>(con->writeBuffer.Peek());
    iov_[0].iov_len = con->writeBuffer.ReadableBytes();
    ioCnt_ = 1;

    /* 文件 */
    if(response_->FileLen() > 0 && response_->File()) {
        iov_[1].iov_base = response_->File();
        iov_[1].iov_len = response_->FileLen();
        ioCnt_ = 2;
    }

    int write_byte = writev(con->getFd(), iov_, ioCnt_);
    response_->UnmapFile();
    if(write_byte < 0) {
        LOG_DEBUG("Failed to send response to fd:%d\n", con->getFd());
        LOG_DEBUG("Close connection fd:%d\n", con->getFd());
        // con->deleteConnectionCallback(con->getSocket());
    } else {
        LOG_DEBUG("Send response %s size:%d to fd:%d successful\n\n", (srcDir_ + request_->getUrl()).c_str(), write_byte, con->getFd());
        // if(!request_->IskeepAlive())
        //     con->deleteConnectionCallback(con->getSocket());
    }
    con->deleteConnectionCallback(con->getSocket());
}