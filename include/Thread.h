#pragma once

#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <thread>

class Thread
{
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc,const std::string& name = std::string());
    ~Thread();

    void start();
    void join();
    bool started() const { return started_;}
    pid_t tid() const {return tid_;}
    const std::string& name() const {return name_;}
private:
    void setDefaultName();
private:
    std::unique_ptr<std::thread> thread_;
    pid_t tid_;
    bool started_;
    bool joined_;
    ThreadFunc func_;
    std::string name_;
    static std::atomic_int numCreated_;
};