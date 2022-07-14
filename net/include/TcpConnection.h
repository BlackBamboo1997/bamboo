#ifndef BAMBOO_NET_TCPCONNECTION_H
#define BAMBOO_NET_TCPCONNECTION_H

#include "Timestamp.h"
#include "Types.h"

#include "Buffer.h"
#include "Socket.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Callbacks.h"

#include <functional>
#include <memory>

/*

*/

namespace bamboo
{
    namespace net
    {
        class Channel;
        class InetAddress;
        class Socket;

        class TcpConnection : public std::enable_shared_from_this<TcpConnection>
        {
        public:


            TcpConnection(EventLoop* loop ,const string& name, int fd, InetAddress localAddr, InetAddress peerAddr);
            ~TcpConnection();

            EventLoop* getLoop() const { return loop_; }
            const string& name() const { return name_; }
            const InetAddress& localAddress() const { return localAddr_; }
            const InetAddress& peerAddress() const { return peerAddr_; }
            bool connected() const { return state_ == kConnected; }
            bool disconnected() const { return state_ == kDisconnected; }
            bool isReading() const { return reading_; }
            Buffer* inputBuffer() { return &inputBuffer_; }
            Buffer* outputBuffer() { return &outputBuffer_; }

            // return true if success.
            bool getTcpInfo(struct tcp_info*) const;
            string getTcpInfoString() const;

            void setConnectionCallback(ConnectionCallback cb)
            { connectionCallback_ = std::move(cb); }
            void setMessageCallback(MessageCallback cb)
            { messageCallback_ = std::move(cb); }
            void setWriteCompleteCallback(WriteCompleteCallback cb)
            { writeCompleteCallback_ = std::move(cb); }
            void setHighWaterMarkCallback(HighWaterMarkCallback cb)
            { highWaterMarkCallback_ = std::move(cb); }
            void setCloseCallback(CloseCallback cb)
            { closeCallback_ = std::move(cb); }
            void setTcpNoDelay(bool on)
            { socket_->setTcpNoDelay(on); }

            void send(const string& data);
            void send(const std::shared_ptr<string> data);
            void send(Buffer* data);
            void send(const void* data, size_t len);
            void send(const char* data, size_t len);

            void stopRead();
            void startRead();
            void shutdown();
            void forceClose();
            
            void connectEstablished();
            void connectDestroyed();

        private:
            enum StateE {kConnecting, kConnected, kDisconnecting, kDisconnected};
            void setState(StateE state) { state_ = state; }

            void handleRead(Timestamp receiveTime);
            void handleWirte();
            void handleClose();
            void handleError();

            void sendInLoop(const std::shared_ptr<string> strPtr);
            void sendInLoop(const char* data, size_t len);
            void stopReadInLoop();
            void startReadInLoop();
            void shutdownInLoop();
            void forceCloseInLoop();
            const char* stateToString() const;
            // int fd_;
            EventLoop *loop_;
            string name_;
            StateE state_;
            bool reading_;
            InetAddress localAddr_;
            InetAddress peerAddr_;
            std::unique_ptr<Channel> channel_;
            std::unique_ptr<Socket> socket_;

            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            HighWaterMarkCallback highWaterMarkCallback_;
            CloseCallback closeCallback_;

            size_t highWaterMark_;
            Buffer outputBuffer_;
            Buffer inputBuffer_;
        };
    }
}

#endif