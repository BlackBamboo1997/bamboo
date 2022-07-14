#ifndef BAMBOO_BASE_MUTEX_H
#define BAMBOO_BASE_MUTEX_H

#include "CurrentThread.h"

#include <pthread.h>
#include <cassert>

namespace bamboo
{
    
class Mutex
{
public:
    Mutex(): holder(0) {
        pthread_mutex_init(&mutex_, NULL);
    }
    ~Mutex() {
        assert(holder == 0);
        pthread_mutex_destroy(&mutex_);
    }

    void lock() {
        pthread_mutex_lock(&mutex_);
        holder = currentthread::tid();
    }
    void unlock() {
        holder = 0;
        pthread_mutex_unlock(&mutex_);
    }

    bool isLockedByThread() {
        return holder == currentthread::tid();
    }

    pthread_mutex_t* get() {
        return &mutex_;
    }
private:
    pthread_mutex_t mutex_;
    int holder;
};

class MutexLockGuard {
    public:
        MutexLockGuard(Mutex& mutex) : mutex_(mutex) {
            mutex_.lock();
        }

        ~MutexLockGuard() {
            mutex_.unlock();
        }
    private:
        Mutex& mutex_;
};

}   // bamboo


#endif  // BAMBOO_BASE_MUTEX_H