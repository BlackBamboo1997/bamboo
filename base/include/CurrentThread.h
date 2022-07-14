#ifndef BAMBOO_BASE_CURRENTTHREAD_H
#define BAMBOO_BASE_CURRENTTHREAD_H

#include "Types.h"

namespace bamboo {

namespace currentthread {
    //__thread only support POD 
    extern __thread int t_cachedTid;
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char* t_threadName;

    void cacheTid();

    inline int tid() {
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }

    inline const char* tidString() {
        return t_tidString;
    }

    inline int tidStringLength() {
        return t_tidStringLength;
    }

    inline const char* threadName() {
        return t_threadName;
    }

    bool isMainThread();

    void sleepUsec(int64_t usec);

    string stackTrace(bool demangle);
}       // currentthread

}      // bamboo

#endif  //BAMBOO_BASE_CURRENTTHREAD_H