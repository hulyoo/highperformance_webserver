#pragma once
#include <stdint.h>

class Timer;

class TimerId
{
public:
    TimerId(Timer* timer,int64_t seq)
        :timer_(timer)
        ,sequence_(seq)
    {}

friend class TimerQueue;
private:
    Timer* timer_;
    int64_t sequence_;
};  