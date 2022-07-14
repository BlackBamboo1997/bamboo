#ifndef BAMBOO_BASE_THREAD_H
#define BAMBOO_BASE_THREAD_H

#include "Types.h"
#include "LatchCutDown.h"

#include <pthread.h>
#include <functional>
#include <memory>
#include <atomic>

namespace bamboo
{

    class Thread
    {
    public:
        typedef std::function<void()> ThreadFunc;
        Thread(ThreadFunc func, const string &name = string());
        ~Thread();

        void start();
        int join();

        bool started() { return started_; }
        pid_t tid() const { return tid_; }
        const string &name() const { return name_; }

        static int numCreated() { return numCreated_; }

    private:
        void setDefaultName();
        // thread status
        bool started_;
        bool joined_;
        // thread identification
        pthread_t pthreadId_;
        pid_t tid_;
        string name_;
        // thread context
        ThreadFunc func_;
        LatchCutDown latch_;

        static std::atomic<int> numCreated_;
    };

}

#endif