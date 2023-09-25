#pragma once
#include<functional>
#include <cstdint>

class EventLoop;

class Channel
{
private:
    EventLoop *loop;
    int fd;
    uint32_t events;
    uint32_t revents;
    bool inEpoll;
    std::function<void()> readcallback;
    std::function<void()> writecallback;
public:
    Channel(EventLoop *_loop, int _fd);
    ~Channel();

    void handleEvent();
    void enableReading();

    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();
    bool getInEpoll();
    void setInEpoll();
    void useET();

    void setRevents(uint32_t);
    void setReadCallback(std::function<void()>);
};