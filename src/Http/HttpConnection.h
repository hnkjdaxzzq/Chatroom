#pragma once

#include <atomic>
#include <bits/types/struct_iovec.h>
#include <memory>
#include <string>
#include <sys/types.h>
#include <HttpRequest.h>
#include <HttpResponse.h>

class Connection;
class HttpRequest;
class HttpResponse;

class HttpConnection {
public:
    HttpConnection (std::string &srcDir);

    bool init(Connection* con);

    void Close();

    ssize_t read(int *Errno);
    ssize_t write(int *Errno);

    bool process();

    bool isKeepAlive() const ;

    static std::atomic<int> httpConnetCount;

    int ToWriteBytes();
    
    std::unique_ptr<Connection> con_;
private:
    HttpRequest request_;
    HttpResponse response_;
    std::string srcDir_;

    bool isClose_;

};