#ifndef BAMBOO_NET_EPollPoller_H
#define BAMBOO_NET_EPollPoller_H

#include "Poller.h"

#include <vector>

struct epoll_event;

namespace bamboo
{
    namespace net
    {
        class EPollPoller : public Poller
        {
        public:
            explicit EPollPoller(EventLoop* loop);
            ~EPollPoller() override;

            Timestamp poll(int timeoutMs, ChannelList *activeChannel) override;
            void removeChannel(Channel *) override;
            void updateChannel(Channel *) override;

        private:
            static const int kInitEventListSize = 16;
            static const char* operationToString(int op);
            void fillActiveChannel(int, ChannelList*);
            void update(int operation, Channel* channel);

            int epollfd_;
            std::vector<struct epoll_event> events_;
        };
    }
}

#endif