#ifndef BAMBOO_NET_POLLPOLLER_H
#define BAMBOO_NET_POLLPOLLER_H

#include "Poller.h"

#include <vector>

struct pollfd;

namespace bamboo
{
    namespace net
    {
        class PollPoller : public Poller
        {
        public:
            PollPoller(EventLoop *loop);
            ~PollPoller() override;

            Timestamp poll(int timeoutMs, ChannelList *activeChannel) override;
            void removeChannel(Channel *) override;
            void updateChannel(Channel *) override;

        private:
            void fillActiveChannels(int num, ChannelList *activeChannel);

            std::vector<struct pollfd> pollfds_;
        };
    }

} // namespace bamboo

#endif