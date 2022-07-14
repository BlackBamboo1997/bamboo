#include "EPollPoller.h"
#include "Logging.h"
#include "Channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace bamboo;
using namespace bamboo::net;

static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace
{
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize)
{
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannel)
{
    int n = ::epoll_wait(epollfd_, &*events_.begin(), events_.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    int savedErrno = errno;
    if (n > 0)
    {
        LOG_TRACE << " events happened";
        fillActiveChannel(n, activeChannel);
        if (static_cast<size_t>(n) == events_.size())
        {
            events_.resize(2 * events_.size());
        }
    }
    else if (n == 0)
    {
        LOG_TRACE << "nothing happened";
    }
    else
    {
        if (savedErrno != EINTR)
        {
            errno = savedErrno;
            LOG_SYSERR << "EPollPoller::poll()";
        }
    }
    return now;
}

void EPollPoller::fillActiveChannel(int n, ChannelList *activeChannel)
{
    for (int i = 0; i < n; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->setRevent(events_[i].events);
        activeChannel->push_back(channel);
    }
}

void EPollPoller::removeChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());

    int idx = channel->index();
    assert(idx == kAdded || idx == kDeleted);
    size_t n = channels_.erase(fd); (void)n;
    assert(n == 1);

    if (idx == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew); // why set new?
}

void EPollPoller::updateChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    const int idx = channel->index();
    LOG_TRACE << "fd = " << channel->fd()
              << " events = " << channel->event() << " index = " << idx;
    if (idx == kNew || idx == kDeleted)
    {
        int fd = channel->fd();
        if (idx == kNew) // new channel
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else // delete
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else // exist
    {
        int fd = channel->fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(idx == kAdded);
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::update(int op, Channel *channel)
{
    struct epoll_event efd;
    ::bzero(&efd, sizeof efd);
    efd.events = channel->event();
    efd.data.ptr = channel;

    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operationToString(op)
              << " fd = " << fd << " event = { " << channel->event() << " }";
    if (::epoll_ctl(epollfd_, op, channel->fd(), &efd) < 0)
    {
        if (op == EPOLL_CTL_DEL)
        {
            LOG_SYSERR << "epoll_ctl op = " << operationToString(op) << " fd = " << fd;
        }
        else
        {
            LOG_SYSFATAL << "epoll_ctl op = " << operationToString(op) << " fd = " << fd;
        }
    }
}

const char *EPollPoller::operationToString(int op)
{
    switch (op)
    {
    case EPOLL_CTL_ADD:
        return "ADD";
    case EPOLL_CTL_DEL:
        return "DEL";
    case EPOLL_CTL_MOD:
        return "MOD";
    default:
        assert(false && "ERROR op");
        return "Unknown Operation";
    }
}