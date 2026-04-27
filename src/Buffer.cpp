
#include <sys/uio.h>
#include <errno.h>
#include <iostream>
#include "Buffer.h"


size_t Buffer::readFd(int fd,int* savedErrno)
{
    char extrabuf[65536];
    iovec vec[2];
    const size_t writeable = writeableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1;
    const ssize_t   n = readv(fd,vec,iovcnt);
    if(n < 0)
    {
        *savedErrno = errno;
    }
    else if(n <= writeable)
    {
       writerIndex_ += n; 
    }
    else
    {
        writerIndex_ = buffer_.size();
        append(extrabuf,n - writeable);

    }

    return n;
}

// int main()
// {

// 	Buffer buffer;

//     std::string str = "dsfsfd";
//     buffer.append(str.c_str(),str.length());
//     std::string strOut = buffer.retrieveAllAsString();
//     std::cout<<strOut<<std::endl;

//     std::string str1 = "dsgdfhgfds213";
//     buffer.append(str1.c_str(),str1.length());
//     std::string strOut1 = buffer.retrieveAllAsString();
//     std::cout<<strOut1<<std::endl;

//    }
