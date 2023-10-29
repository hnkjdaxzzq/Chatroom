#include "Channel.h"
#include "EventLoop.h"
#include <unistd.h>
#include <sys/epoll.h>
#include <cstdint>

Channel::Channel(EventLoop *_loop, int _fd) : loop(_loop), fd(_fd), events(0), revents(0), inEpoll(false) {

}

Channel::~Channel() {
    if(fd != -1) {
        close(fd);
        fd = -1;
    }
}

void Channel::delChannel() {
    if(inEpoll)
        loop->delChannel(this);
}

void Channel::handleEvent() {
    if(revents & (EPOLLIN | EPOLLPRI)) {
        readcallback();
    }
    if(revents & (EPOLLOUT)) {
        writecallback();
    }
}

void Channel::update() {
    loop->updateChannel(this);
}

void Channel::enableReading() {
    events |= EPOLLIN | EPOLLPRI ;
    loop->updateChannel(this);
}

void Channel::enableWriting() {
    events |= EPOLLOUT ;
    loop->updateChannel(this);
}

void Channel::useET() {
    events |= EPOLLET;
    // loop->updateChannel(this);
}

void Channel::setOneshot() {
    events |= EPOLLONESHOT;
    // loop->updateChannel(this);
}

int Channel::getFd() {
    return fd;
}

uint32_t Channel::getEvents() {
    return events;
}

uint32_t Channel::getRevents() {
    return revents;
}

bool Channel::getInEpoll() {
    return inEpoll;
}

void Channel::setInEpoll() {
    inEpoll = true;
}

void Channel::setEvents(uint32_t _ev) {
    events = _ev;
}

void Channel::setRevents(uint32_t _ev) {
    revents = _ev;
}

void Channel::setReadCallback(std::function<void()> _cb) {
    readcallback = _cb;
}

void Channel::setWriteCallback(std::function<void()> _cb) {
    writecallback = _cb;
}