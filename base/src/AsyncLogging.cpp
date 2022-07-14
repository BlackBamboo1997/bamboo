#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"

#include <cassert>
#include <stdio.h>

#define DEBUG 0

using namespace bamboo;

AsyncLogging::AsyncLogging(const string &name, int rollSize, double flushInterval)
    : path_(), basename_(name), rollSize_(rollSize), flushInterval_(flushInterval),
      running_(false), mutex_(), cond_(mutex_), latch_(1),
      thread_(new Thread(std::bind(&AsyncLogging::runInThread, this), "Logging")),
      currentBuffer_(new Buffer), nextBuffer_(new Buffer), buffers_()
{
    assert(basename_.find('/') == string::npos);
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(12);
}

AsyncLogging::~AsyncLogging()
{
    if (running_)
    {
        stop();
    }
}

void AsyncLogging::start()
{
    running_ = true;
    thread_->start();
    latch_.wait();
}

void AsyncLogging::stop()
{
    running_ = false;
    cond_.notify();
    thread_->join();
}

void AsyncLogging::setLogPath(const string &path)
{
    path_ = path;
    if (path_.back() != '/')
    {
        path_.push_back('/');
    }
    basename_ = path_ + basename_;
}

// void AsyncLogging::setLogBasename(const string& basename) {

// }

void AsyncLogging::append(const char *str, int len)
{
    MutexLockGuard lock(mutex_);
    // printf("join append\n");
    if (currentBuffer_->avail() > len)
    {
        currentBuffer_->append(str, len);
    }
    else
    {
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_)
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else
        {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(str, len);
        cond_.notify();
    }
    // printf("append sucess\n");
}

void AsyncLogging::runInThread()
{
    assert(running_ == true);
    latch_.cutdown();
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();

    BufferPtrList bufferToLog;
    bufferToLog.reserve(12);
    LogFile logfile(basename_, rollSize_, false);

    while (running_)
    {
        // caution there is if not while
        {
            MutexLockGuard lock(mutex_);
            if (buffers_.empty())
            {
                cond_.waitForSeconds(flushInterval_);
                if (DEBUG) printf("waitForSeconds return\n");
            }
            buffers_.push_back(std::move(currentBuffer_));
            bufferToLog.swap(buffers_);
            currentBuffer_ = std::move(newBuffer1);
            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }
        assert(!bufferToLog.empty());
        if (bufferToLog.size() > 25)
        {
            if (DEBUG) printf("too many log, adjust and erase it\n");
            // too many log, adjust and erase it
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
                     Timestamp::now().toFormattedString().c_str(),
                     bufferToLog.size() - 2);
            fputs(buf, stderr);
            logfile.append(buf, static_cast<int>(strlen(buf)));
            bufferToLog.erase(bufferToLog.begin() + 2, bufferToLog.end());
        }
        for (auto &buf : bufferToLog)
        {
            if (DEBUG) printf("thread is writting \n");
            logfile.append(buf->data(), buf->length());
        }
        if (bufferToLog.size() > 2)
            bufferToLog.resize(2);
        auto &&resetBuffer = [&bufferToLog](BufferPtr &newBuffer)
        {
            if (!newBuffer)
            {
                assert(bufferToLog.size() > 0);
                newBuffer = std::move(bufferToLog.back());
                // has bug if last size is 4000, now is 3990, then extra 10bytes ?
                newBuffer->reset();
                bufferToLog.pop_back();
            }
        };
        resetBuffer(newBuffer1);
        resetBuffer(newBuffer2);
        bufferToLog.clear();
        logfile.flush();
    }
    logfile.flush();
}
