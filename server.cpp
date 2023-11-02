#include <cstdint>
#include<cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include <strings.h>
#include <thread>
#include<unistd.h>
#include"include/EventLoop.h"
#include"include/Server.h"
#include <Connection.h>
#include <util.h>
#include <HttpServer.h>

int main(int argc, char *argv[]) {
    int opt;

    uint16_t port = 8888;
    std::string webDir("./webapp");
    int threadnums = std::thread::hardware_concurrency();
    int loglevel = 0;
    std::string logpath("./log");

    while ((opt = getopt(argc, argv, "p:d:t:l:v:")) != -1) {
        switch (opt) {
            case 'p':
                port = static_cast<uint16_t>(strtoul(optarg, NULL, 10));
                break;
            case 'd':
                webDir = optarg;
                break;
            case 't':
                threadnums = atoi(optarg);
                break;
            case 'l':
                logpath = optarg;
                break;
            case 'v':
                loglevel = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "UnKnown option: -%c\n", optopt);
                return 1;
        }
    }

    HttpServer server(port, webDir, threadnums, loglevel, logpath);
    server.start();
    return 0;
}