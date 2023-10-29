#include "Epoll.h"
#include "Channel.h"
#include "util.h"
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <mutex>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>

const int MAX_EVENTS = 4096;

Epoll::Epoll() : epfd(-1), events(nullptr) {
    epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");
    events = new epoll_event[MAX_EVENTS];
    bzero(events, sizeof(*events) * MAX_EVENTS);
}

Epoll::~Epoll() {
    if(epfd != -1) {
        close(epfd);
        epfd = -1;
    }
    delete [] events;
}

std::vector<Channel*> Epoll::poll(int timeout) {
    std::vector<Channel*> activeChannels;
    int nfds = 0;
    {
        std::lock_guard<std::mutex> locker(mutx_);
        nfds = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    }
    // gdb调试的时候会在断点处插入一条中断指令,当程序执行到该断点处的时候会发送一个SIGTRAP信号,
    // 程序转去执行中断相应,进而gdb让程序停下来进行调试. 
    // 对于sem_wait\wait\read等会阻塞的函数在调试时,
    // 如果阻塞,都可能会收到调试器发送的信号,而返回非0值
    // 参考链接: https://blog.csdn.net/xidomlove/article/details/8274732
    errif(nfds == -1 && errno != EINTR, "epoll wait error");
    for(int i = 0; i < nfds; ++i) {
        Channel *ch = (Channel*)events[i].data.ptr;
        ch->setRevents(events[i].events);
        activeChannels.push_back(ch);
    }
    return activeChannels;
}

void Epoll::delChannel(Channel *channel) {
    int fd = channel->getFd();
    if(fd == -1)
        return;
    
    errif(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr) == -1, "epoll del error");
    close(fd);
}

void Epoll::updateChannel(Channel *channel) {
    int fd = channel->getFd();
    struct epoll_event ev;
    bzero(&ev, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->getEvents();
    if(!channel->getInEpoll()) {
        {
            std::lock_guard<std::mutex> locker(mutx_);
            errif(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1, "epoll add error");
        }
        channel->setInEpoll();
    } else {
        std::lock_guard<std::mutex> locker(mutx_);
        errif(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1, "epoll modify error");
    }
}