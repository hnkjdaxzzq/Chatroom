#pragma once
#include "util.h"
#include <sys/epoll.h>
#include <vector>

class Channel;

class Epoll
{
private:
    int epfd;
    struct epoll_event *events;
public:
    Epoll();
    ~Epoll();


    void delChannel(Channel*);
    void updateChannel(Channel*);

    std::vector<Channel*> poll(int timeout = -1);
};