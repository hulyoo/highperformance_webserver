#pragma once

#include <string>
#include <atomic>
#include <memory>
#include <vector>
#include <condition_variable>
#include <mutex>

#include "Thread.h"
#include "LogStream.h"

class AsyncLogging
{
public:
    AsyncLogging(const std::string& baseName,off_t rollSize,int flushInterval = 3);
    ~AsyncLogging()
    {
        if(running_)
        {
            stop();
        }
    }

    void start()
    {
        running_ = true;
        thread_.start();
    }


    void stop()
    {
        running_ = false;
        cond_.notify_all();
        thread_.join();
    }

    void append(const char* logline,int len);
private:
    void threadFunc();

private:
    using Buffer = FixedBuffer<kLargeBuffer>;
    using BufferVector = std::vector<std::unique_ptr<Buffer>>;
    using BufferPtr = std::unique_ptr<Buffer>;

    const int flushInterval_;
    std::atomic_bool running_;
    const std::string baseName_;
    const off_t rollSize_;

    std::mutex mutex_;
    std::condition_variable cond_;
    Thread thread_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
};