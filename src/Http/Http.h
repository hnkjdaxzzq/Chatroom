#pragma once
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Connection.h"
#include <bits/types/struct_iovec.h>
#include <memory>
#include <string>


class Http {
public:
    Http() = default;
    Http(std::string srcDir) : srcDir_(srcDir) {}
    void read(Connection* con);
    void write(Connection* con);
    void process(Connection* con);

private:
    std::unique_ptr<HttpRequest> request_;
    std::unique_ptr<HttpResponse> response_;
    std::string srcDir_;
    int ioCnt_;
    struct iovec iov_[2];

};