#include "ThreadPool.h"

using namespace bamboo;

ThreadPool::~ThreadPool()
{
  if (running_)
  {
    stop();
  }
}

void ThreadPool::start()
{
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(threadNum);
  for (int i = 0; i < threadNum; ++i)
  {
    char id[32];
    snprintf(id, sizeof id, "%d", i+1);
    threads_.emplace_back(new bamboo::Thread(
          std::bind(&ThreadPool::runInThread, this), basename_+id));
    threads_[i]->start();
  }
  if (threadNum == 0 && threadInitCallback_)
  {
    threadInitCallback_();
  }
}

void ThreadPool::stop()
{
  {
  MutexLockGuard lock(mutex_);
  running_ = false;
  notEmpty_.notifyAll();
  notFull_.notifyAll();
  }
  for (auto& thr : threads_)
  {
    thr->join();
  }
}

void ThreadPool::push(Task task)
{
    if (threads_.empty())
    {
        task();
    }
    else
    {
        MutexLockGuard lock(mutex_);
        while (isFull() && running_)
        {
            notFull_.wait();
        }
        if (!running_) return;
        assert(!isFull());
        queueTask_.push(std::move(task));
        notEmpty_.notify();
    }
}

ThreadPool::Task ThreadPool::take()
{
    MutexLockGuard lock(mutex_);
    while (queueTask_.empty() && running_)
    {
        notEmpty_.wait();
    }
    Task task;
    if (!queueTask_.empty()) {
        task = std::move(queueTask_.front());
        queueTask_.pop();
        if (maxTaskNum_ > 0) {
            notFull_.notify();
        }
    }
    return task;
}

void ThreadPool::runInThread()
{
  try
  {
    if (threadInitCallback_)
    {
      threadInitCallback_();
    }
    while (running_)
    {
      Task task(take());
      if (task)
      {
        task();
      }
    }
  }
  catch (const std::exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", basename_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", basename_.c_str());
    throw; // rethrow
  }
}