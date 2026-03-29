#include "Timer.h"

std::atomic_int Timer::s_numCreated_;

void Timer::restart(TimeStamp now)
{
    if(repeat_)
    {
        expiration_ = addTime(now,interval_);
    }
    else
    {
        expiration_ =   TimeStamp::invalid();
    }
}