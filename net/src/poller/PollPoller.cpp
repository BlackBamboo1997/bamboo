#include "Logging.h"
#include "Types.h"
#include "PollPoller.h"
#include "Poller.h"
#include "Channel.h"

#include <poll.h>
#include <errno.h>
#include <cassert>

using namespace bamboo;
using namespace bamboo::net;

PollPoller::PollPoller(EventLoop *loop)
    : Poller(loop)
{
}

PollPoller::~PollPoller()
{
}

Timestamp PollPoller::poll(int timeoutMs, ChannelList *activeChannel)
{
    int n = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if (n > 0)
    {
        LOG_TRACE << n << " events happend";
        fillActiveChannels(n, activeChannel);
    }
    else if (n == 0)
    {
        LOG_TRACE << " nothing happend";
    }
    else
    {
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_SYSERR << "PollPoller::poll()";
        }
    }
    return now;
}

void PollPoller::removeChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd();
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    int backfd = pollfds_.back().fd;
    backfd = backfd >= 0 ? backfd : -backfd;
    std::swap(pollfds_[idx], pollfds_[pollfds_.size() - 1]);
    channels_[backfd]->setIndex(idx);
    channels_.erase(channel->fd());
    pollfds_.pop_back();
}

void PollPoller::updateChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->event();
    if (channel->index() < 0)
    { // new channel
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->event());
        pfd.revents = 0;
        channels_[pfd.fd] = channel;
        pollfds_.push_back(std::move(pfd));
        int idx = static_cast<int>(pollfds_.size() - 1);
        channel->setIndex(idx);
    }
    else
    { // existed channel
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(idx >= 0 && idx < static_cast<int>(pollfds_.size()));
        struct pollfd &pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
        pfd.fd = channel->fd();
        pfd.events = channel->event();
        pfd.revents = 0;
        if (channel->isNoneEvent())
        {
            // for quickly find, ignore this pollfd
            pfd.fd = -channel->fd() - 1;
        }
    }
}

void PollPoller::fillActiveChannels(int num, ChannelList *activeChannel)
{
    for (auto &pfd : pollfds_)
    {
        if (pfd.revents > 0)
        {
            Channel *channel = channels_[pfd.fd];
            channel->setRevent(pfd.revents);
            activeChannel->push_back(channel);
            if (--num <= 0)
                break;
        }
    }
}