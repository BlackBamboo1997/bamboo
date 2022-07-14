#ifndef BAMBOO_NET_CHANNEL_H
#define BAMBOO_NET_CHANNEL_H

#include "Timestamp.h"

#include <functional>
#include <memory>

namespace bamboo
{
    namespace net
    {
        class EventLoop;

        class Channel
        {
        public:
            typedef std::function<void()> CallBack;
            typedef std::function<void(Timestamp)> ReadCallBack;
            Channel(EventLoop *loop, int fd);
            ~Channel();

            bool isWritting() const { return event_ & kEnableWrite; }
            bool isReading() const { return event_ & kEnableRead; }

            void setReadCallBack(ReadCallBack cb)
            {
                readCallBack_ = std::move(cb);
            }
            void setCloseCallBack(CallBack cb)
            {
                closeCallBack_ = std::move(cb);
            }
            void setWriteCallBack(CallBack cb)
            {
                writeCallBack_ = std::move(cb);
            }
            void setErrorCallBack(CallBack cb)
            {
                errorCallBack_ = std::move(cb);
            }

            int index() const { return index_; }
            void setIndex(int idx) { index_ = idx; }

            int fd() const { return fd_; }
            int event() const { return event_; }
            void setRevent(int revent) { revent_ = revent; }
            bool isNoneEvent() { return event_ == kNoneEvent; }

            void enableReading()
            {
                event_ |= kEnableRead;
                update();
            }
            void disableReading()
            {
                event_ &= ~kEnableRead;
                update();
            }
            void enableWritting()
            {
                event_ |= kEnableWrite;
                update();
            }
            void disableWritting()
            {
                event_ &= ~kEnableWrite;
                update();
            }
            void disableAll()
            {
                event_ = kNoneEvent;
                update();
            }

            EventLoop *ownerLoop() const { return loop_; }
            void handleEvent(Timestamp);
            void remove();
            // for toplevel, obj die or alive
            void tie(const std::shared_ptr<void> &);
            string reventsToString() const;
            string eventsToString() const;

        private:
            static string eventsToString(int fd, int ev);
            void update();
            void handleEventWithGuard(Timestamp);

            static const int kEnableRead;
            static const int kNoneEvent;
            static const int kEnableWrite;

            EventLoop *loop_;
            int fd_;
            int event_;
            int revent_;
            int index_; // quickly find for poller
            bool logHup_;

            // for toplevel, obj die or alive
            std::weak_ptr<void> tie_;
            bool tied_;

            bool eventHandling_;
            bool addedtoLoop_;
            ReadCallBack readCallBack_;
            CallBack writeCallBack_;
            CallBack errorCallBack_;
            CallBack closeCallBack_;
        };
    }
}

#endif