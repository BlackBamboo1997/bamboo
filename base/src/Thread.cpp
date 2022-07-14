#include "Thread.h"
#include "CurrentThread.h"
#include "Logging.h"
#include "Timestamp.h"

#include <type_traits>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace bamboo
{

    namespace detail
    {
        pid_t gettid()
        {
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }

        // void afterFork()
        // {
        //     bamboo::currentthread::t_cachedTid = 0;
        //     bamboo::currentthread::t_threadName = "main";
        //     // avoid same tid with parent process
        //     currentthread::tid();
        //     // no need to call pthread_atfork(NULL, NULL, &afterFork);
        // }

        // class ThreadNameInitializer
        // {
        // public:
        //     ThreadNameInitializer()
        //     {
        //         bamboo::currentthread::t_threadName = "main";
        //         currentthread::tid();
        //         pthread_atfork(NULL, NULL, &afterFork);
        //     }
        // };

        // ThreadNameInitializer init;

        struct ThreadData
        {
            typedef bamboo::Thread::ThreadFunc ThreadFunc;
            ThreadFunc func_;
            string name_;
            pid_t *tid_;
            LatchCutDown *latch_;

            ThreadData(ThreadFunc func,
                       const string &name,
                       pid_t *tid,
                       LatchCutDown *latch)
                : func_(std::move(func)),
                  name_(name),
                  tid_(tid),
                  latch_(latch)
            {
            }

            void runInThread()
            {
                *tid_ = bamboo::currentthread::tid();
                tid_ = NULL;
                latch_->cutdown();
                latch_ = NULL;

                bamboo::currentthread::t_threadName = name_.empty() ? "bambooThread" : name_.c_str();
                ::prctl(PR_SET_NAME, bamboo::currentthread::t_threadName);
                try
                {
                    func_();
                    bamboo::currentthread::t_threadName = "finished";
                }
                // catch (const Exception& ex)
                // {
                //   bamboo::currentthread::t_threadName = "crashed";
                //   fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                //   fprintf(stderr, "reason: %s\n", ex.what());
                //   fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
                //   abort();
                // }
                catch (const std::exception &ex)
                {
                    bamboo::currentthread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n", ex.what());
                    abort();
                }
                catch (...)
                {
                    bamboo::currentthread::t_threadName = "crashed";
                    fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
                    throw; // rethrow
                }
            }
        };

        void *startThread(void *obj)
        {
            ThreadData *data = static_cast<ThreadData *>(obj);
            data->runInThread();
            delete data;
            return NULL;
        }
    }

}

using namespace bamboo;

void currentthread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

bool currentthread::isMainThread()
{
  return tid() == ::getpid();
}

void currentthread::sleepUsec(int64_t usec)
{
  struct timespec ts = { 0, 0 };
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, NULL);
}

std::atomic<int> Thread::numCreated_(0);

Thread::Thread(ThreadFunc func, const string& n)
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(std::move(func)),
    name_(n),
    latch_(1)
{
  setDefaultName();
}

Thread::~Thread()
{
  if (started_ && !joined_)
  {
    pthread_detach(pthreadId_);
  }
}

void Thread::setDefaultName()
{
  int num = ++numCreated_;
  if (name_.empty())
  {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  // FIXME: move(func_)
  detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);
  if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
  {
    started_ = false;
    delete data; // or no delete?
    // LOG_SYSFATAL << "Failed in pthread_create";
  }
  else
  {
    latch_.wait();
    assert(tid_ > 0);
  }
}

int Thread::join()
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}