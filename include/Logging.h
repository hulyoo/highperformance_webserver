#pragma once

#include <cstring>
#include <functional>
#include "LogStream.h"
#include "TimeStamp.h"

enum class LogLevel
{
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5,
    NUM_LOG_LEVELS,
};

class Logger
{
public:
    class SourceFile
    {
    public:
        template<int N>
        SourceFile(const char(&arr)[N])
            : data_(arr)
            , size_(N - 1)
        {
            const char* slash = strrchr(data_,'/');
            if(slash)
            {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char* filename)
            : data_(filename)
        {
            const char* slash = strrchr(filename,'/');
            if(slash)
            {
                data_ = slash + 1;
            }
            size_ = static_cast<int>(strlen(data_));
        }

    const char* data_;
    int size_;
    };
    Logger(SourceFile file,int line);
    Logger(SourceFile file,int line,LogLevel level);
    Logger(SourceFile file,int line,LogLevel level,const char* func);
    Logger(SourceFile file,int line,LogLevel level,bool toAbort);
    ~Logger();

    LogStream& stream(){return impl_.stream_;}

    static LogLevel loglevel();
    static void setLogLevel(LogLevel level);

    using outPutFunc = std::function<void(const char* msg,int len)>;
    using flushFunc = std::function<void()>;
    static void setOutPut(outPutFunc);
    static void setFlush(flushFunc);

private:
    class Impl
    {
    public:
        Impl(LogLevel level,int old_errno,const SourceFile& file,int line);

        void finish();
    public:
        TimeStamp time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    };
private:
    Impl impl_;
};

extern LogLevel g_logLevel;

inline LogLevel Logger::loglevel()
{
    return g_logLevel;
}

#define LOG_INFO if(Logger::loglevel() <= LogLevel::INFO) \
    Logger(__FILE,__LINE__,LogLevel::INFO,__func__).stream()
#define LOG_DEBUG if (Logger::loglevel() <= LogLevel::DEBUG) \
    Logger(__FILE__,__LINE__,LogLevel::DEBUG,__func__).stream()
#define LOG_WARN Logger(__FILE__,__LINE__,LogLevel::WARN,__func__).stream()
#define LOG_ERROR Logger(__FILE__,__LINE__,LogLevel::ERROR,__func__).stream()
#define LOG_FATAL Logger(__FILE__,__LINE__,LogLevel::FATAL,__func__).stream()

