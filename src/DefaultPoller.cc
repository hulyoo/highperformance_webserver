#include <stdlib.h>

#include "Poller.h"
#include "EpollPoller.h"

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        // return new PollPoller(loop);
    }
    else
    {
        // TODO
        // return new EpollPoller(loop);
    }
}