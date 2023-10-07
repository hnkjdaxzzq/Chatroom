#pragma once
#include <sys/socket.h>
#include "util.h"

class InetAddress;
class Socket {
private:
    int fd;
public:
    Socket();
    Socket(int);
    ~Socket();

    void bind(InetAddress&);
    void listen();
    int accept(InetAddress&);

    void setnonblocking();

    void setnodelay();

    void setelegentclose();

    int getFd();

};
