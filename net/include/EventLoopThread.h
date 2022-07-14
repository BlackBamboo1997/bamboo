#ifndef BAMBOO_NET_EVENTLOOPTHREAD_H
#define BAMBOO_NET_EVENTLOOPTHREAD_H

#include "Thread.h"
#include "Mutex.h"
#include "Condition.h"

namespace bamboo {
    namespace net {
        class EventLoop;

        class EventLoopThread {
            public:

            EventLoopThread(const string& name);
            ~EventLoopThread();
            EventLoop* startLoop();

            private:
                void threadFunc();

                EventLoop* loop_;
                Thread thread_;
                bool exiting_;
                Mutex mutex_;
                Condtition cond_;
                // bool started_;
        };
    }
}


#endif