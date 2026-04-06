#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"
#include "Logging.h"

#include <sys/timerfd.h>
#include <unistd.h>

int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);

    if(timerfd < 0)
    {
    //    LOG_FATAL << "failed in timerfd_create"; 
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(TimeStamp when)
{
    int64_t microseconds = when.microSecondSinceEpoch() - TimeStamp::now().microSecondSinceEpoch();
     if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
      microseconds / TimeStamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
      (microseconds % TimeStamp::kMicroSecondsPerSecond) * 1000);
     return ts;
}

void readTimerfd(int timerfd,TimeStamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_DEBUG << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
    if (n != sizeof howmany)
    { 
        LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}

void resetTimerfd(int timerfd, TimeStamp expiration)
{
    // wake up loop by timerfd_settime()
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memZero(&newValue, sizeof newValue);
    memZero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        LOG_FATAL << "timerfd_settime()";
    }
}

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop)
    , timerfd_(createTimerfd())
    , timerfdChannel_(loop,timerfd_)
    , timers_()
    , callingExpiredTimer_(false)
{
    timerfdChannel_.setReadCallback(
        std::bind(&TimerQueue::handleRead,this)
    );
    timerfdChannel_.enableReading();
}

TimerId TimerQueue::addTimer(TimerCallback cb,TimeStamp when,double interval)
{
    Timer* timer = new Timer(std::move(cb),when,interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,this,timer));
    return TimerId(timer,timer->sequence());
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop,this,timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    bool earliestChanged = insert(timer);

    if(earliestChanged)
    {
        resetTimerfd(timerfd_,timer->expiration());
    }
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    ActiveTimer timer(timerId.timer_,timerId.sequence_);
    auto it = activeTimers_.find(timer);
    if(it != activeTimers_.end())
    {
        size_t n = timers_.erase(TimerQueue::Entry(it->first->expiration(),it->first));
        delete it->first;
        activeTimers_.erase(it);
    }
    else if(callingExpiredTimer_)
    {
        cancelingTimers_.insert(timer);
    }

}

void TimerQueue::handleRead()
{
    TimeStamp now(TimeStamp::now());
    readTimerfd(timerfd_,now);

    std::vector<TimerQueue::Entry> expired = getExpired(now);

    callingExpiredTimer_ = true;
    cancelingTimers_.clear();

    for(const TimerQueue::Entry& it : expired)
    {
        it.second->run();
    }
    callingExpiredTimer_ = false;
    reset(expired,now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(TimeStamp now)
{
    std::vector<TimerQueue::Entry> expired;
    TimerQueue::Entry entry(now,reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto end = timers_.lower_bound(entry);

    std::copy(timers_.begin(),end,back_inserter(expired));
    timers_.erase(timers_.begin(),end);

    for(const TimerQueue::Entry& it : expired)
    {
        ActiveTimer timer(it.second,it.second->sequence());
        size_t n = activeTimers_.erase(timer);
    }
    return expired;
}

void TimerQueue::reset(const std::vector<TimerQueue::Entry>& expired,TimeStamp now)
{
    TimeStamp nextExpire;

    for(const TimerQueue::Entry& it : expired)
    {
        ActiveTimer timer(it.second,it.second->sequence());
        if(it.second->repeat()
            && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it.second->restart(now);
            insert(it.second);
        }
        else
        {
            delete it.second;
        }

        if(!timers_.empty())
        {
            nextExpire = timers_.begin()->second->expiration();
        }

        if(nextExpire.valid())
        {
            resetTimerfd(timerfd_,nextExpire);
        }
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;
    TimeStamp when = timer->expiration();
    auto it = timers_.begin();
    if(it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }
    {
        std::pair<TimerList::iterator,bool> result
            = timers_.insert(TimerQueue::Entry(when,timer));
    }
    {
        std::pair<ActiveTimerSet::iterator,bool> result
            = activeTimers_.insert(ActiveTimer(timer,timer->sequence()));
    }
    return earliestChanged;
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
    for(const TimerQueue::Entry& timer : timers_)
    {
        delete timer.second;
    }
}

