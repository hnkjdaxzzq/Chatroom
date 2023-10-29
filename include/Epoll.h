#pragma once
#include "util.h"
#include <mutex>
#include <sys/epoll.h>
#include <vector>

class Channel;

class Epoll
{
private:
    int epfd;
    struct epoll_event *events;
    std::mutex mutx_;
public:
    Epoll();
    ~Epoll();


    void delChannel(Channel*);
    void updateChannel(Channel*);

    std::vector<Channel*> poll(int timeout = -1);
};