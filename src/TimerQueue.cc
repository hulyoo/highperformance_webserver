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

