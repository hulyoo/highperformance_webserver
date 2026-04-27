#pragma once

#include <vector>

static const size_t kPrepend = 8;
static const size_t kInitialSize = 1024;

class Buffer
{
public:
    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kPrepend + kInitialSize)
        , readerIndex_(kPrepend)
        , writerIndex_(kPrepend)
        {}
    
    size_t writeableBytes() const {return buffer_.size() - writerIndex_;} 
    size_t readableBytes() const {return writerIndex_ - readerIndex_;}
    size_t prependableBytes() const {return readerIndex_;}

    const char* peek() const {return begin() + readerIndex_;}


    void retrieve(size_t len)
    {
        if((len < readableBytes()))
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }
    void retrieveAll()
    {
        readerIndex_ = kPrepend;
        writerIndex_ = kPrepend;
    }

    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(),len);
        retrieve(len);
        return result;
    }

    void append(const char* data,size_t len)
    {
        std::copy(data,data + len,beginWrite());
        writerIndex_ += len;
    }
    
    char* beginWrite(){return begin() + writerIndex_;}
    const char* beginWrite() const {return begin() + writerIndex_;}

    size_t readFd(int fd,int* savedErrno);
private:
    char* begin(){return &*buffer_.begin();}
    const char* begin() const {return &*buffer_.begin();}

    void makeSpace(size_t len)
    {
        //TODO 这么做是为了什么？ 
        if(writeableBytes() + prependableBytes() < len + kPrepend)
        {
            buffer_.resize(len + writerIndex_);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                    begin() + writerIndex_,
                    begin() + kPrepend);

            readerIndex_ = kPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }
private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

};
