#ifndef BAMBOO_NET_EVENTLOOP_H
#define BAMBOO_NET_EVENTLOOP_H

#include "Mutex.h"
#include "Timestamp.h"
#include "CurrentThread.h"

#include <memory>
#include <vector>
#include <functional>
#include <atomic>

namespace bamboo
{
    namespace net
    {
        class Channel;
        class Poller;

        //one loop per thread
        class EventLoop
        {
        public:
            typedef std::function<void()> Func;
            typedef std::vector<Channel *> ChannelList;

            EventLoop();
            ~EventLoop();

            void loop();
            void quit();
            void wakeup();
            
            void runInLoop(Func cb);
            void queueInLoop(Func cb);
            bool isInLoopThread() const;
            void assertInLoopThread();

            size_t queueSize() const;

            void updateChannel(Channel *);
            void removeChannel(Channel *);
            bool hasChannel(Channel *);

            static EventLoop *getEventLoopOfCurrentThread();

        private:
            void abortNotInLoopThread();
            void handleRead(); // waked up
            void doPendingFunctors();

            typedef std::vector<Func> FuncList;

            // eventloop state
            bool looping_;
            bool eventHandling_;
            bool pendingHandling_;
            std::atomic<bool> quit_;

            // for async func, ensure threadsafe
            FuncList pendingFunctors_;

            // I/O Multiplexer
            std::unique_ptr<Poller> poller_;

            int iteration_;
            int wakeupFd_; // replace pipe(pipefd) pipefd[2]
            std::unique_ptr<Channel> wakeupChannel_;
            // avoid repeat create
            Timestamp pollReturnTime_;

            // handle activeChannels
            ChannelList activeChannels_;
            Channel *currentActiveChannel_; // What's the use?

            mutable Mutex mutex_;
            const pid_t tid_;
        };
    }
}

#endif