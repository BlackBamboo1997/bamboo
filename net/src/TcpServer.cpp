#include "TcpServer.h"
#include "TcpConnection.h"
#include "SocketsOps.h"
#include "InetAddress.h"

#include <cassert>

using namespace bamboo;
using namespace bamboo::net;

TcpServer::TcpServer(EventLoop *loop, InetAddress listenAddr, const string& name)
    : started_(false),
      connectNum_(0),
      ipPort_(listenAddr.toIpPort()),
      name_(name),
      loop_(loop),
      acceptor_(new Accetpor(loop, listenAddr, false)),
      threadPools_(loop),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback)
{
    acceptor_->setNewConnCallBack(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (auto &item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::start()
{
    if (!started_)
    {
        started_ = true;
        threadPools_.startThreadPool();
        // assert(acceptor_->)
        loop_->runInLoop(std::bind(&Accetpor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int fd, InetAddress peerAddr)
{
    loop_->assertInLoopThread();
    EventLoop *loop = threadPools_.getNextLoop();
    char buf[64];
    int connNum = ++connectNum_;
    snprintf(buf, sizeof(buf), "%s%d", ipPort_.c_str(), connNum);
    string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_
             << "] - new connection [" << connName
             << "] from " << peerAddr.toIpPort();

    //why? has bug operator()
    auto local = sockets::getLocalAddr(fd);
    InetAddress localAddr(local);
    TcpConnectionPtr conn(new TcpConnection(loop, connName, fd, localAddr, peerAddr));
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setHighWaterMarkCallback(highWaterMarkCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));

    // assert(connections_.find(buf) == connections_.end());
    connections_[connName] = conn;
    // register to the owning thread
    loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
             << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    (void)n;
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
}