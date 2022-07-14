#ifndef BAMBOO_BASE_CONDITION_H
#define BAMBOO_BASE_CONDITION_H
#include "Mutex.h"
#include <pthread.h>
#include <stdint.h>

namespace bamboo {

class Condtition {
    public:
    Condtition(Mutex& mutex) : mutex_(mutex) {
        pthread_cond_init(&cond_, NULL);
    }
    ~Condtition() {
        pthread_cond_destroy(&cond_);
    }

    void wait() {
        // MutexLockGuard lock(mutex_); //error deadlock
        pthread_cond_wait(&cond_, mutex_.get());
    }

    bool waitForSeconds(double seconds) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);

        const int64_t kNanoSecondsPerSecond = 1000000000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);

        abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
        abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

        return pthread_cond_timedwait(&cond_, mutex_.get(), &abstime);
    }

    void notify() {
        pthread_cond_signal(&cond_);
    }

    void notifyAll() {
        pthread_cond_broadcast(&cond_);
    }

    private:
        Mutex& mutex_;
        pthread_cond_t cond_;
};

}

#endif