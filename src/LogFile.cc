#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include "LogFile.h"
#include "Logging.h"

AppendFile::AppendFile(StringArg filename)
    :fp_(::fopen(filename.c_str(),"ae"))
    ,writtenBytes_(0)
{
    ::setbuffer(fp_,buffer_,sizeof buffer_);
}

AppendFile::~AppendFile()
{
    ::fclose(fp_);
}

void AppendFile::append(const char* logline,int len)
{
    size_t written = 0;

    while(written != len)
    {
        size_t remain = len - written;
        size_t n = this->write(logline + written,remain);
        if(n != remain)
        {
            int err = ferror(fp_);
            if(err)
            {
                break;
            }
        }
        written += n;
    }
    writtenBytes_ = written;
}

void AppendFile::flush()
{
    ::fflush(fp_);
}

size_t AppendFile::write(const char* logline,size_t len)
{
    return ::fwrite_unlocked(logline,1,len,fp_);
}

LogFile::LogFile(const std::string& basename,
                off_t rollSize,
                bool threadSafe,
                int flushInterval,
                int checkEveryM)
            : basename_(basename)
            , rollSize_(rollSize)
            , flushInterval_(flushInterval)
            , checkEveryM_(checkEveryM)
            , count_(0)
            , mutex_(threadSafe ? std::make_unique<std::mutex>() : NULL)
            , startOfPeriod_(0)
            , lastRoll_(0)
            , lastFlush_(0)
{
    rollFile();
}

void LogFile::append(const char* logline,size_t len)
{
    if(mutex_)
    {
        std::unique_lock<std::mutex> lock(*mutex_);
        append_unlocked(logline,len);
    }
    else
    {
        append_unlocked(logline,len);
    }
}

void LogFile::flush()
{
    if(mutex_)
    {
        std::unique_lock<std::mutex>(*mutex_);
        file_->flush();
    }
    else
    {
        file_->flush();
    }
}

void LogFile::append_unlocked(const char* logline,int len)
{
    file_->append(logline,len);

    if(file_->writtenBytes() > rollSize_)
    {
        rollFile();
    }
    else
    {
        ++count_;
        if(count_ >= checkEveryM_)
        {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerseconds_ * kRollPerseconds_;
            if(thisPeriod_ != startOfPeriod_)
            {
                rollFile();
            }
            else if(now - lastFlush_ > flushInterval_)
            {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

bool LogFile::rollFile()
{
    time_t now = 0;
    std::string filename = getLogFileName(basename_,&now);
    time_t start = now / kRollPerseconds_ * kRollPerseconds_; 

    if(now > lastRoll_)
    {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendFile(filename));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string& basename,time_t* now)
{
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now,&tm);
    strftime(timebuf,sizeof timebuf,">%Y%m%d-%H%M%S",&tm);
    filename += timebuf;

    // char pidbuf[32];

    // filename += pidbuf;
    filename += ".log";

    return filename;
}