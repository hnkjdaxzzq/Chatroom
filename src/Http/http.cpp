#include "Http.h"
#include <Connection.h>
#include "util.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstdio>
#include <memory>
#include <strings.h>
#include <sys/types.h>
#include <sys/uio.h>

void Http::read(Connection *con) {
    static char buf[2048];
    while(true) {
        bzero(&buf, sizeof(buf));
        ssize_t byte_read = con->creadn(buf, sizeof(buf));
        if(byte_read > 0) {
            con->readBuffer.Append(buf);
        } else if(byte_read == -1 && errno == EINTR) {
            printf("continue reading");
            continue;
        } else if(byte_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            printf("message from client fd %d: length %ld\n", con->getFd(), con->readBuffer.size());
        } else if(byte_read == 0) {
            con->deleteConnectionCallback(con->getSocket());
            printf("EOF, client fd %d disconnected\n", con->getFd());
            break;
        }
    }
}

void Http::process(Connection *con) {
    read(con);
    request_ = std::make_unique<HttpRequest>();
    printf("\n%s\n", con->readBuffer.c_str());
    try {
        request_->parse(con->readBuffer.getBufText());
        printf("parse httpRequest successful form fd:%d\n", con->getFd());
        response_ = std::make_unique<HttpResponse>(srcDir_, request_->getUrl(), request_->IskeepAlive(), 200);
        response_->MakeResponse(con->writeBuffer);

    } catch (const char* errmesg) {
        printf("%s", errmesg);
        response_ = std::make_unique<HttpResponse>(srcDir_, request_->getUrl(), request_->IskeepAlive(), 400);
    }

    /* 响应头 */
    iov_[0].iov_base = (char*)con->writeBuffer.c_str();
    iov_[0].iov_len = con->writeBuffer.size();
    ioCnt_ = 1;

    /* 文件 */
    if(response_->FileLen() > 0 && response_->File()) {
        iov_[1].iov_base = response_->File();
        iov_[1].iov_len = response_->FileLen();
        ioCnt_ = 2;
    }

    int write_byte = writev(con->getFd(), iov_, ioCnt_);
    if(write_byte < 0) {
        printf("Failed to send response to fd:%d\n", con->getFd());
        printf("Close connection fd:%d\n", con->getFd());
        con->deleteConnectionCallback(con->getSocket());
    } else {
        printf("Send response to fd:%d successful\n", con->getFd());
        response_->UnmapFile();
        if(!request_->IskeepAlive())
            con->deleteConnectionCallback(con->getSocket());
    }


}