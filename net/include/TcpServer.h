#ifndef BAMBOO_NET_TCPSERVER_H
#define BAMBOO_NET_TCPSERVER_H

#include "EventLoopThreadPool.h"
#include "TcpConnection.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "Buffer.h"

// #include <set>
#include <unordered_map>
#include <atomic>

namespace bamboo
{
    namespace net
    {
        class TcpConnection;
        class InetAddress;

        class TcpServer
        {
        public:
            typedef std::unordered_map<string, TcpConnectionPtr> TcpConnectionMap;

            TcpServer(EventLoop *loop, InetAddress listenAddr, const string& name = string("TcpServer"));
            ~TcpServer();

            void setThreadNum(int num) { if(num > 0) threadPools_.setThreadNum(num); }

            void setConnectionCallback(ConnectionCallback cb)
            { connectionCallback_ = std::move(cb); }
            void setWriteCompleteCallback(WriteCompleteCallback cb)
            { writeCompleteCallback_ = std::move(cb); }
            void setMessageCallback(MessageCallback cb)
            { messageCallback_ = std::move(cb); }
            void setHighWaterMarkCallback(HighWaterMarkCallback cb)
            { highWaterMarkCallback_ = std::move(cb); }

            void start();

        private:
            // for acceptor, package connection
            void newConnection(int fd, InetAddress peerAddr);
            // for handleclose connection
            void removeConnection(const TcpConnectionPtr&);

            void removeConnectionInLoop(const TcpConnectionPtr&);

            std::atomic<bool> started_;
            std::atomic<int> connectNum_;
            const string ipPort_;
            string name_;
            EventLoop *loop_;
            std::unique_ptr<Accetpor> acceptor_;
            EventLoopThreadPool threadPools_;
            TcpConnectionMap connections_;

            ConnectionCallback connectionCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            MessageCallback messageCallback_;
            HighWaterMarkCallback highWaterMarkCallback_;
            // CloseCallback closeCallback_;
        };
    }
}

#endif