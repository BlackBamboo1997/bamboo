#ifndef BAMBOO_BASE_LATCHCUTDOWN_H
#define BAMBOO_BASE_LATCHCUTDOWN_H

#include "Mutex.h"
#include "Condition.h"

namespace bamboo
{
    class LatchCutDown
    {
    public:
        explicit LatchCutDown(int count) : count_(count), mutex_(), cond_(mutex_) {}

        void wait()
        {
            MutexLockGuard lock(mutex_);
            while (count_ > 0)
            {
                cond_.wait();
            }
        }

        void cutdown()
        {
            MutexLockGuard lock(mutex_);
            --count_;
            if (count_ == 0)
            {
                cond_.notifyAll();
            }
        }

        int getCount() const    // for const, mutex is mutable
        {
            MutexLockGuard lock(mutex_);
            return count_;
        }

    private:
        int count_;
        mutable Mutex mutex_;
        Condtition cond_;
    };
}   //bamboo

#endif  //  BAMBOO_BASE_LATCHCUTDOWN_H