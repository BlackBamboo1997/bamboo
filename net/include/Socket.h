#ifndef BAMBOO_NET_SOCKETS_H
#define BAMBOO_NET_SOCKETS_H

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace bamboo
{
    namespace net
    {
        class InetAddress;

        class Socket
        {
        public:
            explicit Socket(int fd)
                : sockfd_(fd)
            {    }
            ~Socket();

            int fd() const { return sockfd_; }
            bool getTcpInfo(struct tcp_info *) const;
            bool getTcpInfoString(char *buf, int len) const;

            void bind(const InetAddress &addr);
            void listen();
            int accept(InetAddress *peeraddr);
            void shutdownWrite();

            void setKeepAlive(bool on);
            void setTcpNoDelay(bool on);
            void setReusePort(bool on);
            void setReuseAddr(bool on);

        private:
            const int sockfd_;
        };
    }
}

#endif
