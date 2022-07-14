#ifndef BAMBOO_NET_CALLBACKS_H
#define BAMBOO_NET_CALLBACKS_H

#include "Timestamp.h"

#include <functional>
#include <memory>

namespace bamboo
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    namespace net
    {
        class TcpConnection;
        class Buffer;

        typedef std::function<void(const std::shared_ptr<TcpConnection> &)> ConnectionCallback;
        typedef std::function<void(const std::shared_ptr<TcpConnection> &)> WriteCompleteCallback;
        typedef std::function<void(const std::shared_ptr<TcpConnection> &)> CloseCallback;
        typedef std::function<void(const std::shared_ptr<TcpConnection> &, size_t)> HighWaterMarkCallback;
        typedef std::function<void(const std::shared_ptr<TcpConnection> &, Buffer *, Timestamp)> MessageCallback;
        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

        void defaultConnectionCallback(const TcpConnectionPtr &conn);
        void defaultMessageCallback(const TcpConnectionPtr &conn,
                                    Buffer *buffer,
                                    Timestamp timestamp);
    }
}

#endif