#ifndef BAMBOO_BASE_ASYNCLOGGING_H
#define BAMBOO_BASE_ASYNCLOGGING_H

#include "Mutex.h"
#include "Condition.h"
#include "Thread.h"
#include "LogStream.h"
#include "LatchCutDown.h"

#include <functional>
#include <memory>
#include <vector>
#include <atomic>

namespace bamboo {

//choose producer consumer or buffers
class AsyncLogging {
public:
    AsyncLogging(const string& name, int rollSize, double flushInterval = 3);
    ~AsyncLogging();

    void start();
    void stop();
    //check your basename, it should be in plain alpha. then set path.
    void setLogPath(const string& path);
    // void setLogBasename(const string& basename);

    void append(const char* str, int len);

private:
    typedef detail::FixedBuffer<detail::kbigBuffer> Buffer;
    // typedef std::function<void ()> ThreadFunc;
    typedef std::unique_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferPtrList;
    typedef std::unique_ptr<Thread> ThreadPtr;

    void runInThread();

    string path_;
    string basename_;
    int rollSize_;
    double flushInterval_;
    std::atomic<bool> running_;

    Mutex mutex_;
    Condtition cond_;
    LatchCutDown latch_;

    ThreadPtr thread_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferPtrList buffers_;
};

}

#endif