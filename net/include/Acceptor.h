#ifndef BABMBOO_NET_ACCEPTOR_H
#define BABMBOO_NET_ACCEPTOR_H

#include "Timestamp.h"
#include "Logging.h"

// #include "InetAddress.h"
// #include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

namespace bamboo
{
    namespace net
    {

        class EventLoop;
        class InetAddress;

        class Accetpor
        {
        public:
            typedef std::function<void(int, InetAddress)> NewConnCallBack;

            Accetpor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
            ~Accetpor();

            void setNewConnCallBack(NewConnCallBack cb)
            { newConnCallBack_ = std::move(cb); }

            void listen();

        private:
            void handRead();

            NewConnCallBack newConnCallBack_;

            EventLoop *loop_;
            Socket acceptSocket_;
            Channel accpetChannel_;

            bool listenning_;
            //reserve a fd, prevent fd cann't close
            int idleFd_;
        };
    }
}

#endif