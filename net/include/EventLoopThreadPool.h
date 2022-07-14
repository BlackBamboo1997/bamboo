#ifndef BAMBOO_NET_EVENTLOOPTHREADPOOL_H
#define BAMBOO_NET_EVENTLOOPTHREADPOOL_H

#include "Types.h"

#include <vector>
#include <memory>

namespace bamboo
{
    namespace net
    {
        class EventLoop;
        class EventLoopThread;

        class EventLoopThreadPool
        {
        public:
            EventLoopThreadPool(EventLoop *loop, const string &name = string());
            ~EventLoopThreadPool();

            void setThreadNum(int num) { threadNum_ = num; }
            void startThreadPool();

            EventLoop *getNextLoop();
            std::vector<EventLoop *> getAllLoops();

            bool started() const { return started_; }
            const string &name() const { return basename_; }

        private:
            bool started_;
            int threadNum_;
            int nextLoop_;
            const string basename_;
            EventLoop *baseLoop_;
            std::vector<std::unique_ptr<EventLoopThread>> eventLoopThreadPool_;
            std::vector<EventLoop *> loops_;
        };
    }
}

#endif