#include<cstdio>
#include <functional>
#include <strings.h>
#include<unistd.h>
#include"include/EventLoop.h"
#include"include/Server.h"
#include <Connection.h>
#include <util.h>
#include <HttpServer.h>

const int BUF_SIZE = 1024;

int main() {
    HttpServer server(8888, "./webapp");
    server.start();
    return 0;
}