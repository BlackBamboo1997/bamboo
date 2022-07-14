#include "Channel.h"
#include "EventLoop.h"
#include "Logging.h"

#include <poll.h>
#include <cassert>
#include <sstream>

using namespace bamboo;
using namespace bamboo::net;

const int Channel::kNoneEvent = 0;
const int Channel::kEnableRead = POLLIN | POLLPRI;
const int Channel::kEnableWrite = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), fd_(fd), event_(0), revent_(0), index_(-1),
      logHup_(true), tied_(false), eventHandling_(false), addedtoLoop_(false)
{
}

Channel::~Channel()
{
    assert(!eventHandling_);
    assert(!addedtoLoop_);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_)
    {
        auto guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::remove()
{
    assert(isNoneEvent());
    addedtoLoop_ = false;
    loop_->removeChannel(this);
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    addedtoLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    eventHandling_ = true;
    LOG_TRACE << reventsToString();
    if ((revent_ & POLLHUP) && !(revent_ & POLLIN))
    {
        if (logHup_)
        {
            LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
        }
        if (closeCallBack_)
            closeCallBack_();
    }

    if (revent_ & POLLNVAL)
    {
        LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
    }

    if (revent_ & (POLLERR | POLLNVAL))
    {
        if (errorCallBack_)
            errorCallBack_();
    }
    if (revent_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (readCallBack_)
            readCallBack_(receiveTime);
    }
    if (revent_ & POLLOUT)
    {
        if (writeCallBack_)
            writeCallBack_();
    }
    eventHandling_ = false;
}

string Channel::eventsToString() const
{
    return eventsToString(fd_, event_);
}

string Channel::reventsToString() const
{
    return eventsToString(fd_, revent_);
}

string Channel::eventsToString(int fd, int ev)
{
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & POLLIN)
        oss << "IN ";
    if (ev & POLLPRI)
        oss << "PRI ";
    if (ev & POLLOUT)
        oss << "OUT ";
    if (ev & POLLHUP)
        oss << "HUP ";
    if (ev & POLLRDHUP)
        oss << "RDHUP ";
    if (ev & POLLERR)
        oss << "ERR ";
    if (ev & POLLNVAL)
        oss << "NVAL ";

    return oss.str();
}