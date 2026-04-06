#pragma once

#include <atomic>
#include <functional>
#include <vector>
#include <mutex>

#include "TimerId.h"
#include "CurrentThread.h"
#include "TimeStamp.h"
#include "Callbacks.h"


class Channel;
class Poller;
class TimerQueue;

class EventLoop
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();

    void quit();

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    size_t queueSize() const;

    TimerId runAt(TimeStamp time,TimerCallback cb);
    TimerId runAfter(double delay,TimerCallback cb);
    TimerId runEvery(double interval,TimerCallback cb);

    void cancel(TimerId timerId);

    void wakeup();

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);


    bool IsInLoopThread() const {return threadId_ == CurrentThread::tid();}
    bool eventHandling() const {return eventHandling_;}
    

private:
    using ChannelList = std::vector<Channel*>;
    void handleRead();  // waked up
    void doPendingFunctors();
private: 
    bool looping_;
    std::atomic_bool quit_;
    bool eventHandling_;
    bool callingPendingFunctors_;
    int64_t iteration_;
    const pid_t threadId_;
    TimeStamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;

    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    //  这里为什么要用mutable？因为在const函数中加锁
    mutable std::mutex mutex_;

    std::vector<Functor> pendingFunctors_;
};