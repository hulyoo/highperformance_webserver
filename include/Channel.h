#pragma once

#include "Noncopyable.h"
#include "TimeStamp.h"
#include <functional>
#include <memory>

class EventLoop;
class Channel : noncopyable 
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(TimeStamp)>;

    Channel(EventLoop* loop,int fd);
    ~Channel();

    void setReadCallback(ReadEventCallback cb){readCallback_ = std::move(cb);}
    void setWriteCallback(EventCallback cb){writeCallback_ = std::move(cb);}
    void setCloseCallback(EventCallback cb){closeCallback_ = std::move(cb);}
    void handleEvent(TimeStamp receiveTime);

    void enableReading(){events_ |= kReadEvent;update();}
    void disableReading(){events_ &= ~kReadEvent;update();}
    void enableWriting(){events_ |= kWriteEvent;update();}
    void disableWriting(){events_ &= ~kWriteEvent;update();}
    void disableAll(){events_ = kNonEvent; update();}
    bool isWriting() const {return events_ & kWriteEvent;} 
    bool isReading() const {return events_ & kReadEvent;}

    int index() {return index_;}
    int fd() const {return fd_;}
    void set_index(int indx){index_ = indx;}

    void tie(const std::shared_ptr<void>& obj);

    EventLoop* ownerLoop(){return loop_;}
    void remove();

private:
    static std::string eventsToString(int fd,int ev);

    void update();
    void handleEventWithGuard(TimeStamp receiveTime);
private:
    static const int kNonEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_;
    int index_;
    
    std::weak_ptr<void> tie_;
    bool tied_;
    bool eventHandling_;
    bool addedToLoop_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;


};