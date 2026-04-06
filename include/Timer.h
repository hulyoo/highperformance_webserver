#pragma once

#include <atomic>

#include "TimeStamp.h"
#include "Noncopyable.h"
#include "Callbacks.h"


class Timer : noncopyable
{
public:
    Timer(TimerCallback cb,TimeStamp when,double interval)
        : callback_(std::move(cb))
        , expiration_(when)
        , interval_(interval)
        , repeat_(interval > 0.0)
        , sequence_(s_numCreated_++)
    {}
    ~Timer() = default;

    void run() const
    {
        callback_();
    }

    TimeStamp expiration() const {return expiration_;}
    bool repeat() const {return repeat_;}

    int64_t sequence() const {return sequence_;}
    
    void restart(TimeStamp now); 

private:
    const TimerCallback callback_;
    TimeStamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;
    static std::atomic_int s_numCreated_;
};

