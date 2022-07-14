#ifndef BAMBOO_BASE_THREADPOOL_H
#define BAMBOO_BASE_THREADPOOL_H

#include "Thread.h"
#include "Mutex.h"
#include "Condition.h"

#include <memory>
#include <vector>
#include <queue>

namespace bamboo
{

    class ThreadPool
    {
    public:
        typedef std::unique_ptr<Thread> ThreadPtr;
        typedef std::function<void()> Task;
        typedef std::function<void()> ThreadCallBack;

        ThreadPool(const string &name = string("ThreadPool"))
            : basename_(name), threadNum(0), maxTaskNum_(0), mutex_(), notEmpty_(mutex_), notFull_(mutex_), running_(false)
        {
        }

        ~ThreadPool();

        void setThreadNum(int num) { threadNum = num; }
        void setMaxTaskNum(int num) { maxTaskNum_ = num; }
        void setThreadInitCallback(const ThreadCallBack &cb)
        {
            threadInitCallback_ = cb;
        }

        void start();
        void stop();

        const string &name() const
        {
            return basename_;
        }

        int queueSize() const
        {
            MutexLockGuard lock(mutex_);
            return queueTask_.size();
        }

        void push(Task task);

        Task take();

    private:
        void runInThread();
        bool isFull() const
        {
            // assert(mutex_.isLockedByThread());
            return maxTaskNum_ != 0 && queueTask_.size() >= maxTaskNum_;
        }

        ThreadCallBack threadInitCallback_;
        string basename_;
        int threadNum;
        int maxTaskNum_;

        mutable Mutex mutex_;
        Condtition notEmpty_;
        Condtition notFull_;
        bool running_;

        std::vector<ThreadPtr> threads_;
        std::queue<Task> queueTask_;
    };

} // bamboo

#endif