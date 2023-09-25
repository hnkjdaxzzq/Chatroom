
#include "HttpServer.h"
#include <Connection.h>
#include <memory>
#include "Http.h"

void HttpServer::process(Connection * con) {
    std::unique_ptr<Http> http_server = std::make_unique<Http>(srcDir_);
    http_server->process(con);
}