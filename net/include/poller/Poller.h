#ifndef BAMBOO_NET_POLLER_H
#define BAMBOO_NET_POLLER_H

#include "EventLoop.h"
#include "Timestamp.h"

#include <vector>
#include <memory>
#include <unordered_map>

namespace bamboo
{
    namespace net
    {
        class Channel;

        class Poller
        {
        public:
            typedef std::vector<Channel *> ChannelList;
            Poller(EventLoop*);
            virtual ~Poller();

            virtual Timestamp poll(int timeoutMs, ChannelList* activeChannel) = 0;
            virtual void removeChannel(Channel *) = 0;
            virtual void updateChannel(Channel *) = 0;
            virtual bool hasChannel(Channel*) const;

            //default epoll
            static Poller* newDefaultPoller(EventLoop*);

            void assertInLoopThread() const {
                loop_->assertInLoopThread();
            }

        protected:
            std::unordered_map<int, Channel*> channels_;

        private:
            EventLoop* loop_;
        };
    }
}

#endif