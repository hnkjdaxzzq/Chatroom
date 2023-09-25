#pragma once

#include <string>
#include "Http.h"
#include <Connection.h>
class HttpServer {
public:
    HttpServer(const std::string& srcDir ) : srcDir_(srcDir) {

    };
    void process(Connection* con);
private:
    std::string srcDir_;
};