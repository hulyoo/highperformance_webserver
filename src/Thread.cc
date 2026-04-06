#include "Thread.h"
#include "CurrentThread.h"

#include <sys/syscall.h>
#include <unistd.h>
#include <semaphore.h>


std::atomic_int Thread::numCreated_ = 0;

pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}


Thread::Thread(ThreadFunc func,const std::string& name)
    : func_(func)
    , name_(name)
    , started_(false)
    , joined_(false)
    , tid_(0)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_ && !joined_)
    {
        thread_->detach();
    }
}


void Thread::start()
{
    started_ = true;
    sem_t sem;
    sem_init(&sem,false,0);
    thread_ = std::make_unique<std::thread>([&]{
        sem_post(&sem);
        func_();
    });
    sem_wait(&sem);
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty())
    {
        char buf[32];
        snprintf(buf,sizeof buf,"Thread%n",num);
        name_ = buf;
    }
}

void CurrentThread::cacheTid()
{
    if(t_cacheTid == 0)
    {
        t_cacheTid = gettid();
        t_tidStringLength = snprintf(t_tidString,sizeof t_tidString,"%5d",t_cacheTid);
    }
}