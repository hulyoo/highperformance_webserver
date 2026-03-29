#pragma once

#include <atomic>
#include <functional>
#include <vector>
#include "CurrentThread.h"
#include "TimeStamp.h"

class Channel;
class Poller;

class EventLoop
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();

    void quit();

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    

private:
};