#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"
#include <vector>

EventLoop::EventLoop() : ep(nullptr), quit(false) {
    ep = new Epoll();
}

EventLoop::~EventLoop()
{
    delete ep;
}

void EventLoop::setLoop(std::function<void ()> loopCb) {
    loop_ = loopCb;
}

void EventLoop::loop() {
    if(loop_) {
        loop_();
        return;
    }
    while(!quit) {
        std::vector<Channel*> chs = ep->poll(0);
        for(auto it = chs.begin(); it != chs.end(); ++it) {
            (*it)->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel *ch) {
    ep->updateChannel(ch);
}

void EventLoop::delChannel(Channel *ch) {
    ep->delChannel(ch);
}