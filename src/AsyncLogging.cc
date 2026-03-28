#include "AsyncLogging.h"
#include "LogFile.h"
#include "TimeStamp.h"

AsyncLogging::AsyncLogging(const std::string& baseName,off_t rollSize,int flushInterval)
    : baseName_(baseName)
    , rollSize_(rollSize)
    , flushInterval_(flushInterval)
    , running_(false)
    , thread_(std::bind(&AsyncLogging::threadFunc,this),"Logging")
    , currentBuffer_(new Buffer)
    , nextBuffer_(new Buffer)
    , buffers_()
{
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline,int len)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if(currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline,len);
    }
    else
    {
        buffers_.push_back(std::move(currentBuffer_));
        if(nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline,len);
        cond_.notify_all();
    }
}

void AsyncLogging::threadFunc()
{
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    LogFile output(baseName_,rollSize_,false);
    newBuffer1->bzero();
    newBuffer2->bzero();

    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    while(running_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if(buffers_.empty())
            {
                cond_.wait_for(lock,std::chrono::seconds(flushInterval_));
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if(!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        if(buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf,sizeof buf,"Dropped log at %s,%zd larger buffers \n",
                TimeStamp::now().toFormatString().c_str(),
            buffersToWrite.size() - 2);

            fputs(buf,stderr);
            buffersToWrite.erase(buffersToWrite.begin() + 2,buffersToWrite.end());
        }

        for(const auto& buffer : buffersToWrite)
        {
            output.append(buffer->data(),buffer->length());
        }

        if(buffersToWrite.size() > 2)
        {
            buffersToWrite.resize(2);
        }

        if(!newBuffer1)
        {
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if(!newBuffer2)
        {
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}


