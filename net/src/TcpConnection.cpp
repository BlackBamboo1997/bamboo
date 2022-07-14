#include "TcpConnection.h"
#include "Channel.h"
#include "SocketsOps.h"
#include "Socket.h"

#include "Logging.h"

#include <errno.h>

using namespace bamboo;
using namespace bamboo::net;

// namespace bamboo
// {

//     namespace net
//     {

//         void defaultConnectionCallback(const TcpConnectionPtr &conn)
//         {
//             LOG_TRACE << conn->localAddress().toIpPort() << " -> "
//                       << conn->peerAddress().toIpPort() << " is "
//                       << (conn->connected() ? "UP" : "DOWN");
//         }

//         void defaultMessageCallback(const TcpConnectionPtr &conn,
//                                     Buffer *buffer,
//                                     Timestamp timestamp)
//         {
//             buffer->retrieveAll();
//         }
//     }
// }

void bamboo::net::defaultConnectionCallback(const TcpConnectionPtr &conn)
{
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
              << conn->peerAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void bamboo::net::defaultMessageCallback(const TcpConnectionPtr &conn,
                                         Buffer *buffer,
                                         Timestamp timestamp)
{
    buffer->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop *loop, const string &name, int sockfd, InetAddress localAddr, InetAddress peerAddr)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      reading_(false),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      channel_(new Channel(loop, sockfd)),
      socket_(new Socket(sockfd)),
      highWaterMark_(60 * 1024 * 1024)
{
    channel_->setReadCallBack(std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallBack(std::bind(&TcpConnection::handleWirte, this));
    channel_->setErrorCallBack(std::bind(&TcpConnection::handleError, this));
    channel_->setCloseCallBack(std::bind(&TcpConnection::handleClose, this));
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
              << " fd=" << sockfd;

    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at " << this
              << " fd=" << channel_->fd()
              << " state=" << stateToString();
    assert(state_ == kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info *tcpi) const
{
    return socket_->getTcpInfo(tcpi);
}

string TcpConnection::getTcpInfoString() const
{
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof buf);
    return buf;
}

void TcpConnection::send(const string &data)
{
    send(data.c_str(), data.size());
}

void TcpConnection::send(Buffer *data)
{
    send(data->peek(), data->readableBytes());
}

void TcpConnection::send(const void *data, size_t len)
{
    send(static_cast<const char *>(data), len);
}

void TcpConnection::send(const char *data, size_t len)
{
    loop_->runInLoop(std::bind<void (TcpConnection::*)(const char *, size_t)>(&TcpConnection::sendInLoop,
                               shared_from_this(),
                               data,
                               len));
}

void TcpConnection::send(const std::shared_ptr<string> data)
{
    loop_->runInLoop(std::bind<void (TcpConnection::*)(const std::shared_ptr<string>)>(&TcpConnection::sendInLoop,
                               shared_from_this(),
                               data));
}

void TcpConnection::stopRead()
{
    // remove enable
    loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, shared_from_this()));
}

void TcpConnection::startRead()
{
    // register enable
    loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, shared_from_this()));
}

void TcpConnection::shutdown()
{
    // close write
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, shared_from_this()));
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

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    assert(reading_ == false);
    reading_ = true;
    setState(kConnected);
    //绑定TcpConnection对象
    channel_->tie(shared_from_this());
    //注册可读事件
    channel_->enableReading();
    //日志登记
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::sendInLoop(const std::shared_ptr<string> strPtr)
{
    sendInLoop(strPtr->c_str(), strPtr->size());
}

//跨线程调用会出现bug : data数据丢失, data's life cycle
void TcpConnection::sendInLoop(const char *data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remain = len;
    bool faultError = false;
    if (state_ == kDisconnected)
    {
        LOG_WARN << "disconnected, give up writing";
        return;
    }
    // direct write
    if (!channel_->isWritting() && outputBuffer_.readableBytes() == 0)
    {
        LOG_TRACE << "direct write";
        nwrote = sockets::write(socket_->fd(), data, len);
        if (nwrote >= 0)
        {
            remain = len - nwrote;
            if (remain == 0 && writeCompleteCallback_)
            {
                // writeCompleteCallback_(shared_from_this());
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }

    if (!faultError && remain > 0)
    {
        size_t previousLen = outputBuffer_.readableBytes();
        LOG_TRACE << "previous readableBytes " << previousLen;
        if (previousLen + remain >= highWaterMark_ && previousLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), previousLen + remain));
        }
        outputBuffer_.append(static_cast<const char *>(data) + nwrote, remain);
        // have you registered readable events?
        if (!channel_->isWritting())
        {
            channel_->enableWritting();
        }
    }
}

void TcpConnection::stopReadInLoop()
{
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading())
    {
        channel_->disableReading();
        reading_ = false;
    }
}

void TcpConnection::startReadInLoop()
{
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading())
    {
        channel_->enableReading();
        reading_ = true;
    }
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWritting())
    {
        // we are not writing
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if (state_ == kConnected || state_ == kDisconnecting)
    {
        // as if we received 0 byte in handleRead();
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

void TcpConnection::handleRead(Timestamp receiveTime)
{
    loop_->assertInLoopThread();
    // read in buffer
    int savederrno = 0;
    ssize_t n = inputBuffer_.readFd(socket_->fd(), savederrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        LOG_WARN << "TcpConnection::handRead" << " read 0 bytes";
        handleClose();
    }
    else
    {
        errno = savederrno;
        LOG_SYSERR << "TcpConnection::handRead";
        handleError();
    }
}

void TcpConnection::handleWirte()
{
    loop_->assertInLoopThread();
    if (channel_->isWritting())
    {
        LOG_TRACE << "send data : " << string(outputBuffer_.peek(), outputBuffer_.readableBytes());
        ssize_t n = sockets::write(socket_->fd(),
                                   outputBuffer_.peek(),
                                   outputBuffer_.readableBytes());
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWritting();
                if (writeCompleteCallback_)
                {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_,
                                                 shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
    }
    else
    {
        LOG_TRACE << "Connection fd = " << channel_->fd()
                  << " is down, no more writing";
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    // must be the last line
    closeCallback_(guardThis);
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError [" << name_
              << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}