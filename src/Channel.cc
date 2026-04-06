#include <poll.h>

#include "EventLoop.h"
#include "Channel.h"
#include "Logging.h"

const int Channel::kNonEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop*loop,int fd)
    :loop_(loop)
    ,fd_(fd)
    ,events_(0)
    ,revents_(0)
    ,index_(-1)
    ,tied_(false)
    ,eventHandling_(false)
    ,addedToLoop_(false)
{
}

Channel::~Channel()
{
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::remove()
{
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::handleEvent(TimeStamp recevieTime)
{
    std::shared_ptr<void> guard;
    if(tied_)
    {
        guard = tie_.lock();
        if(guard)
        {
            handleEventWithGuard(recevieTime);
        }
    }
    else
    {
        handleEventWithGuard(recevieTime);
    }
}

void Channel::handleEventWithGuard(TimeStamp recevieTime)
{
    eventHandling_ = true;
    if(revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if(readCallback_)
        {
            readCallback_(recevieTime);
        }
    }

    if(revents_ & POLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }

    eventHandling_ = false;
}