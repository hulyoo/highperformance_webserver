#pragma once

#include <set>
#include <vector>

#include "TimeStamp.h"
#include "Channel.h"
#include "Callbacks.h"

class EventLoop;
class Timer;
class TimerId;
class TimerQueue
{
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    void cancel(TimerId timerId);

private:
    using Entry = std::pair<TimeStamp,Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*,int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);

    void handleRead();
    std::vector<Entry> getExpired(TimeStamp now);

    void reset(const std::vector<Entry>& expired,TimeStamp now);

    bool insert(Timer* timer);

private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;

    ActiveTimerSet activeTimers_;
    bool callingExpiredTimer_;
    ActiveTimerSet cancelingTimers_;
};