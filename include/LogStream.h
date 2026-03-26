#pragma once

#include <functional>
#include <string>
#include <stdio.h>
#include <algorithm>

#include "Types.h"

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer
{
public:
    FixedBuffer()
        : cur_(data_)
    {
        setCookie(cookieStart);
    }

    ~FixedBuffer()
    {
        setCookie(cookieEnd);
    }

    void append(const char* buf,size_t len)
    {
        if((size_t)avail() > len)
        {
            memcpy(cur_,buf,len);
            cur_ += len;
        }
    }

    char* current(){return cur_;}
    int avail() const {return static_cast<int>(end() - cur_);}
    void add(size_t len){cur_ += len;}

    void bzero(){memZero(data_,sizeof data_);}
    void reset(){cur_ = data_;}

    const char* data() const {return data_;}
    int length() const {return static_cast<int>(cur_ - data_);}
    std::string toString() const{return string(data_,length());}

    void setCookie(void(*cookie)()){cookie_ = cookie;}
private:
    static void cookieStart();
    static void cookieEnd();

    const char* end() const {return data_ + sizeof data_;}
private:
    void (*cookie_)();
    char data_[SIZE];
    char* cur_;
};

template<int SIZE>
void FixedBuffer<SIZE>::cookieStart()
{

}

template<int SIZE>
void FixedBuffer<SIZE>::cookieEnd()
{

}
class LogStream
{
public:
    using self = LogStream;
    using Buffer = FixedBuffer<kSmallBuffer>;

    self &operator<<(bool v)
    {
        buffer_.append(v ? "1" : "0",1);
        return *this;
    }

    self &operator<<(const char* str)
    {
        if(str)
            buffer_.append(str,strlen(str));
        else
            buffer_.append("(null)",6);
        return *this;
    }

    self &operator<<(std::string& str)
    {
        buffer_.append(str.c_str(),str.length());
        return *this;
    }

    self &operator<<(int);

    void append(const char* data,int len){buffer_.append(data,len);}
    const Buffer& buffer(){return buffer_;}
    void resetBuffer(){buffer_.reset();}

private:
    template<typename T>
    void formatInteger(T);
private:
    Buffer buffer_;
    static const int kMaxnumericSize = 48;
};