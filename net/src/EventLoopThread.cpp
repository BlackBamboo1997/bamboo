#include "EventLoopThread.h"

#include "EventLoop.h"

#include <cassert>
using namespace bamboo;
using namespace bamboo::net;

EventLoopThread::EventLoopThread(const string& name = string())
    : loop_(nullptr),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name),
    exiting_(false),
    mutex_(),
    cond_(mutex_)
{
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop*  EventLoopThread::startLoop() {
    assert(!exiting_);

    thread_.start();

    EventLoop* loop = nullptr;
    {
        MutexLockGuard lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait();
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    // if () {

    // }
    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }
    loop.loop();
    MutexLockGuard lock(mutex_);
    loop_ = nullptr;
}