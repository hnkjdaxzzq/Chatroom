#include "Http.h"
#include <Connection.h>
#include "util.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
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
            fprintf(stderr, "continue reading");
            continue;
        } else if(byte_read == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            if(strlen(buf) != 0)
                con->readBuffer.Append(buf);
            fprintf(stderr, "read() message from client fd %d: length %ld, errno: %d\n", con->getFd(), con->readBuffer.size(), errno);
            // perror("why can't read the data");
            break;
        } else if(byte_read == 0) {
            con->deleteConnectionCallback(con->getSocket());
            fprintf(stderr, "EOF, client fd %d disconnected\n", con->getFd());
            break;
        }
    }
}

void Http::process(Connection *con) {
    read(con);
    request_ = std::make_unique<HttpRequest>();
    // printf("\n%s\n", con->readBuffer.c_str());
    try {
        request_->parse(con->readBuffer.getBufText());
        con->readBuffer.clear();
        fprintf(stderr, "fd:%d ,process() path:%s\n", con->getFd(),request_->getUrl().c_str());
        fprintf(stderr, "process() parse httpRequest successful form fd:%d\n", con->getFd());
        response_ = std::make_unique<HttpResponse>(srcDir_, request_->getUrl(), request_->IskeepAlive(), 200);
        response_->MakeResponse(con->writeBuffer);

    } catch (const char* errmesg) {
        fprintf(stderr, "%s\n", errmesg);
        response_ = std::make_unique<HttpResponse>(srcDir_, request_->getUrl(), request_->IskeepAlive(), 400);
        response_->MakeResponse(con->writeBuffer);
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
        fprintf(stderr, "Failed to send response to fd:%d\n", con->getFd());
        fprintf(stderr, "Close connection fd:%d\n", con->getFd());
        con->deleteConnectionCallback(con->getSocket());
    } else {
        fprintf(stderr, "Send response %s size:%d to fd:%d successful\n\n", (srcDir_ + request_->getUrl()).c_str(), write_byte, con->getFd());
        
        if(!request_->IskeepAlive())
            con->deleteConnectionCallback(con->getSocket());
    }
    response_->UnmapFile();

}