#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include "Logging.h"
#include "CurrentThread.h"

LogLevel g_logLevel = LogLevel::DEBUG;
Logger::outPutFunc g_output = [](const char*msg, int len){
   size_t n = fwrite(msg,1,len,stdout); 
};

Logger::flushFunc g_flush = [](){
    fflush(stdout);
};

Logger::Logger(SourceFile file, int line)
    : impl_(LogLevel::INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
    : impl_(level, 0, file, line)
{
    impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level, bool toAbort)
    : impl_(level, errno, file, line)
{
}

Logger::~Logger()
{
    impl_.finish();
    const LogStream::Buffer& buf = impl_.stream_.buffer();
    g_output(buf.data(), buf.length());
    if (impl_.level_ == LogLevel::FATAL)
    {
        g_flush();
        abort();
    }
}

void Logger::setLogLevel(LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutPut(Logger::outPutFunc fn)
{
    g_output = fn;
}

void Logger::setFlush(Logger::flushFunc fn)
{
    g_flush = fn;
}

Logger::Impl::Impl(LogLevel level, int old_errno, const SourceFile& file, int line)
    : time_(TimeStamp::now())
    , stream_()
    , level_(level)
    , line_(line)
    , basename_(file)
{
    CurrentThread::tid();
    if (old_errno != 0)
    {
        stream_ << strerror(old_errno) << " (errno=" << old_errno << ") ";
    }
}


void Logger::Impl::finish()
{
    stream_ << " - " << basename_.data_ << ':' << line_ << '\n';
}
