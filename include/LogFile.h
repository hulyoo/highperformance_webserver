#pragma once

#include <string>
#include <memory>
#include <mutex>

class StringArg
{
public:
    StringArg(const char* str)
        :str_(str)
    {}
    StringArg(const std::string& str)
        :str_(str.c_str())
    {}

    const char* c_str() {return str_;}
private:
    const char* str_;
};

class AppendFile
{
public:
    explicit AppendFile(StringArg filename);
    ~AppendFile();

    void append(const char* logline,int len);
    void flush();

    off_t writtenBytes()const {return writtenBytes_;}
private:
    size_t write(const char* logline,size_t len);

    FILE* fp_;
    char buffer_[60*1024];
    off_t writtenBytes_;
};


class LogFile
{
public:
    LogFile(const std::string& basename,off_t rollSize,bool threadSafe = true,int flushInterval = 3,int checkEveryM = 1024);
    ~LogFile() = default;

    void append(const char* logline,size_t len);
    void flush();
    bool rollFile();
private:
    void append_unlocked(const char* logline,int len);
    static std::string getLogFileName(const std::string& basename,time_t* now);
private:
    const std::string basename_;
    const off_t rollSize_;
    const int flushInterval_;
    const int checkEveryM_;

    int count_;

    std::unique_ptr<std::mutex> mutex_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    std::unique_ptr<AppendFile> file_;

    const static int kRollPerseconds_ = 60*60*24;
};