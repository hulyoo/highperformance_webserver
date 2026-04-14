#pragma once

#include <vector>
#include "Poller.h"

struct epoll_event;

class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop* loop);
    virtual ~EpollPoller() override;

    virtual TimeStamp Poll(int timeoutMs,ChannelList* activeChannels);
    virtual void updateChannel(Channel* channel) override;
    virtual void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;
    static const char* operationToString(int op);

    void fillActiveChannel(int numEvents,ChannelList* activeChannels) const;
    void update(int operation,Channel* channel);

    using EventList = std::vector<struct epoll_event>;
    int epollfd_;
    EventList events_;
};