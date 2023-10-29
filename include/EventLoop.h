#pragma once

#include <functional>
class Epoll;
class Channel;

class EventLoop
{
private:
    Epoll *ep;
    bool quit;
    std::function<void()> loop_;
public:
    EventLoop();
    ~EventLoop();

    void setLoop(std::function<void()>);
    void loop();
    void updateChannel(Channel*);
    void delChannel(Channel*);
};