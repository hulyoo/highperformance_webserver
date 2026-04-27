#pragma once

#include "Types.h"
#include "Noncopyable.h"
#include "TcpConnection.h"
#include <map>
#include <functional>
#include <string>
#include <memory>
#include <atomic>

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    enum Option
    {
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop *loop,
              const InetAddress &listenAddr,
              const std::string &namArg,
              Option option = kNoReusePort);
    ~TcpServer();

    const std::string &ipPort() const { return ipPort_; }
    const std::string &name() const { return name_; }
    EventLoop *getLoop() const { return loop_; }

    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
    std::shared_ptr<EventLoopThreadPool> threadPool()
    {
        return threadPool_;
    }

    void start();

    void setConnectionCallback(const ConnectionCallback &cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback &cb)
    {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb)
    {
        writeCompleteCallback_ = cb;
    }

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
    EventLoop *loop_;
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    ThreadInitCallback threadInitCallback_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    std::atomic_int started_;
    int nextConnId_;
    ConnectionMap connections_;
};
