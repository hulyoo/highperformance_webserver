#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "EpollPoller.h"
#include "Channel.h"
#include "Logging.h"

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

EpollPoller::EpollPoller(EventLoop* loop)
    :Poller(loop)
    ,epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    ,events_(kInitEventListSize)
    {
        if(epollfd_ < 0)
        {
            LOG_FATAL<<"EpollPoller::EpollPoller";
        }
    }


EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}

TimeStamp EpollPoller::Poll(int timeoutMs,ChannelList* activeChannel)
{
    int numEvents = ::epoll_wait(epollfd_,
            &*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int savedErrno = errno;
    TimeStamp now(TimeStamp::now());
    if(numEvents > 0)
    {
        fillActiveChannel(numEvents,activeChannel);
        if(implicit_cast<size_t>(numEvents) == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_DEBUG<<"nothing hanppened";
    }
    else
    {
        if (savedErrno != EINTR)
        {   
            errno = savedErrno;
            LOG_ERROR << "EPollPoller::poll()";
        }
    }
    return now;
}


