#include "Logging.h"

#include "EventLoop.h"
#include "Channel.h"
#include "Poller.h"
#include "SocketsOps.h"

#include <cassert>
#include <signal.h> //for ignore sigpipe
#include <sys/eventfd.h>
#include <unistd.h>

using namespace bamboo;
using namespace bamboo::net;

namespace
{
    __thread EventLoop *t_loopInThisThread = 0;

    const int kPollTimeMs = 10000;

    // for wakeupfd base Linux 2.6.27
    int createEventfd()
    {
        int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (evtfd < 0)
        {
            LOG_SYSERR << "Failed in eventfd";
            abort();
        }
        return evtfd;
    }
#pragma GCC diagnostic ignored "-Wold-style-cast"
    class IgnoreSigPipe
    {
    public:
        IgnoreSigPipe()
        {
            ::signal(SIGPIPE, SIG_IGN);
            // LOG_TRACE << "Ignore SIGPIPE";
        }
    };
#pragma GCC diagnostic error "-Wold-style-cast"

    IgnoreSigPipe initObj;
} // namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false),
      eventHandling_(false),
      pendingHandling_(false),
      quit_(false),
      poller_(Poller::newDefaultPoller(this)),
      iteration_(0),
      // timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(nullptr),
      tid_(currentthread::tid())
{
    LOG_DEBUG << "EventLoop created " << this << " in thread " << tid_;
    if (t_loopInThisThread)
    {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                  << " exists in this thread " << tid_;
    }
    else
    {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallBack(
        std::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    LOG_DEBUG << "EventLoop " << this << " of thread " << tid_
              << " destructs in thread " << currentthread::tid();
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    if (!quit_)
    {
        LOG_TRACE << "EventLoop " << this << " start looping";
    }
    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        ++iteration_;
        // if (Logger::logLevel() <= Logger::TRACE)
        // {
        //     printActiveChannels();
        // }
        eventHandling_ = true;
        for (auto &channel : activeChannels_)
        {
            currentActiveChannel_ = channel;
            channel->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = nullptr;
        eventHandling_ = false;
        doPendingFunctors();
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit()
{
  quit_ = true;
  if (!isInLoopThread())
  {
    wakeup();
  }
}

void EventLoop::wakeup()
{
  uint64_t one = 1;
  // no less than 8 bytes
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::runInLoop(Func cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Func cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    if (!isInLoopThread() || pendingHandling_)
    {
        wakeup();
    }
}

bool EventLoop::isInLoopThread() const
{
    return tid_ == currentthread::tid();
}

void EventLoop::assertInLoopThread()
{
    if (!isInLoopThread())
    {
        abortNotInLoopThread();
    }
}

void EventLoop::doPendingFunctors()
{
    FuncList execFuncList;
    {
        MutexLockGuard lock(mutex_);
        execFuncList.swap(pendingFunctors_);
        // swap(execFuncList, pendingFunctors_);
    }

    for (const auto &func : execFuncList)
    {
        func();
    }
}

size_t EventLoop::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return pendingFunctors_.size();
}

void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << tid_
              << ", current thread id = " << currentthread::tid();
}

void EventLoop::handleRead()
{
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}