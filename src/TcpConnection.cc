#include "TcpConnection.h"
#include "Logging.h"
#include "Channel.h"
#include "Socket.h"
#include "SocketOops.h"
#include "EventLoop.h"

#include <errno.h>

void defaultConnectionCallback(const TcpConnectionPtr& conn)
{
    
}

void defaultMessageCallback(const TcpConnectionPtr&,Buffer* buf,TimeStamp)
{
    buf->retrieveAll();
}
TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string &nameArg,
                             int sockfd,
                             const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(loop),
      name_(nameArg),
      state_(kConnecting),
      reading_(true),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr)
{
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this));
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
              << " fd=" << sockfd;
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
}

bool TcpConnection::getTcpInfo(struct tcp_info *tcpi) const
{
    return socket_->getTcpInfo(tcpi);
}

std::string TcpConnection::getTcpInfoString() const
{
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof buf);
    return buf;
}

void TcpConnection::send(const void *data, int len)
{
    send(StringPiece(static_cast<const char*>(data),len));
}

void TcpConnection::send(const StringPiece& message)
{
    if(state_ == kConnected)
    {
        if(loop_->IsInLoopThread())
	{
	    sendInLoop(message);
	}
	else
	{
	    void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
	    loop_->runInLoop(std::bind(fp,this,message.as_string()));
	}
    }
}

void TcpConnection::send(Buffer *buf)
{
    if (state_ == kConnected)
    {
        if (loop_->IsInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
            std::bind(fp,this,buf->retrieveAllAsString());
        }
    }
}

void TcpConnection::sendInLoop(const StringPiece& message)
{
    sendInLoop(message.data(),message.size());
}

void TcpConnection::sendInLoop(const void* data,size_t len)
{
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if(state_ == kDisconnected)
    {
        LOG_WARN <<"disconnected,give up writing";
	return;
    }
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = sockets::write(channel_->fd(),data,len);
	if(nwrote >= 0)
	{
	    remaining = len - nwrote;
	    if(remaining == 0 && writeCompleteCallback_)
	    {
	    	loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
	    }
	}
	else
	{
	    nwrote = 0;
	    if(errno != EWOULDBLOCK)
	    {
	        LOG_ERROR <<"TcpConnection::sendInLoop";
	        if(errno == EPIPE || errno == ECONNRESET)
		{
		    faultError = true;
		}	
	    }
	}
    }

    if(!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();
	//TODO
	outputBuffer_.append(static_cast<const char*>(data)+nwrote,remaining);
	if(!channel_->isWriting())
	{
	    channel_->enableWriting();
	}
    }
}

void TcpConnection::shutdown()
{
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop()
{
    if (!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceClose()
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseWithDelay(double seconds)
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
	std::weak_ptr<TcpConnection>wp(shared_from_this());
        loop_->runAfter(seconds, [wp](){
	    if(auto sp = wp.lock())
	    {
	        sp->forceClose();
	    }
		});
    }
}
void TcpConnection::forceCloseInLoop()
{
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        handleClose();
    }
}
const char *TcpConnection::stateToString() const
{
    switch (state_)
    {
    case kDisconnected:
        return "kDisconnected";
    case kConnecting:
        return "kConnecting";
    case kConnected:
        return "kConnected";
    case kDisconnecting:
        return "kDisconnecting";
    default:
        return "unknown state";
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop()
{
    if (!reading_ || !channel_->isReading())
    {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::stopRead()
{
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop()
{
    if (reading_ || channel_->isReading())
    {
        channel_->disableReading();
        reading_ = false;
    }
}

void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::handleRead(TimeStamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_ERROR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting())
    {
        ssize_t n = sockets::write(channel_->fd(),
                                   outputBuffer_.peek(),
                                   outputBuffer_.readableBytes());
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_ERROR << "TcpConnection::handleWrite";
            // if (state_ == kDisconnecting)
            // {
            //   shutdownInLoop();
            // }
        }
    }
    else
    {
        LOG_INFO << "Connection fd = " << channel_->fd()
                 << " is down, no more writing";
    }
}

void TcpConnection::handleClose()
{
    LOG_INFO << "fd = " << channel_->fd() << " state = " << stateToString();
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    closeCallback_(guardThis);
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError [" << name_
              << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}
