#pragma once

#include "EventLoop.h"
#include "TimeStamp.h"

#include <map>
#include <vector>

class Channel;
class EventLoop;

class Poller
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller();

    virtual TimeStamp Poll(int timeoutMs,ChannelList* activeChannels) = 0;

    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;
    virtual bool hasChannel(Channel* channel) const;

    static Poller* newDefaultPoller(EventLoop* loop);



protected:
    using ChannelMap = std::map<int,Channel*>; 
    ChannelMap channels_;
private:
    EventLoop* loop_;
};