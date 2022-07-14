#include "Poller.h"
#include "Channel.h"
#include "PollPoller.h"
#include "EPollPoller.h"

using namespace bamboo;
using namespace bamboo::net;

Poller::Poller(EventLoop *loop)
    : loop_(loop)
{
}

Poller::~Poller() = default;

bool Poller::hasChannel(Channel* channel) const {
    assertInLoopThread();
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if (::getenv("BAMBOO_USE_POLL")) {
        return new PollPoller(loop);
    }
    else {
        return new EPollPoller(loop);
    }
}