#include "EventLoopThreadPool.h"

#include "EventLoopThread.h"
#include "EventLoop.h"
using namespace bamboo;
using namespace bamboo::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *loop, const string &name)
    : started_(false),
      threadNum_(0),
      nextLoop_(0),
      basename_(name),
      baseLoop_(loop)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::startThreadPool()
{
    assert(!started_);
    baseLoop_->assertInLoopThread();
    started_ = true;
    eventLoopThreadPool_.resize(threadNum_);
    for (int i = 0; i < threadNum_; ++i)
    {
        char buf[basename_.size() + 32];
        const string &name = (basename_.size() == 0) ? "eventLoopThreadPool" : basename_;
        snprintf(buf, sizeof(buf), "%s%d", name.c_str(), i);
        eventLoopThreadPool_[i] = (std::unique_ptr<EventLoopThread>(new EventLoopThread(buf)));
        loops_.push_back(eventLoopThreadPool_[i]->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop *loop = baseLoop_;
    if (!loops_.empty())
    {
        loop = loops_[nextLoop_];
        ++nextLoop_;
        if (implicit_cast<size_t>(nextLoop_) >= loops_.size())
        {
            nextLoop_ = 0;
        }
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
{
    baseLoop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty())
    {
        return std::vector<EventLoop *>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}