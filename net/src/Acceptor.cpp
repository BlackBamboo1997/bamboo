#include "Acceptor.h"
#include "SocketsOps.h"
#include "InetAddress.h"
#include "EventLoop.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>

using namespace bamboo;
using namespace bamboo::net;

Accetpor::Accetpor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
      accpetChannel_(loop, acceptSocket_.fd()),
      listenning_(false),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idleFd_ > 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bind(listenAddr);
    accpetChannel_.setReadCallBack(std::bind(&Accetpor::handRead, this));
}

Accetpor::~Accetpor()
{
    accpetChannel_.disableAll();
    accpetChannel_.remove();
    ::close(idleFd_);
}

void Accetpor::listen()
{
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    accpetChannel_.enableReading();
}

void Accetpor::handRead()
{
    loop_->assertInLoopThread();

    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
        if (newConnCallBack_)
        {
            newConnCallBack_(connfd, peerAddr);
        }
        else
        {
            sockets::close(connfd);
        }
    }
    else
    {
        LOG_SYSERR << "in Acceptor::handleRead";
        if (errno == EMFILE)    // Too many open files
        {
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}