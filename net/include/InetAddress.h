#ifndef BAMBOO_NET_INETADDRESS_H
#define BAMBOO_NET_INETADDRESS_H

#include <netinet/in.h>
#include "Types.h"
#include "SocketsOps.h"

namespace bamboo
{
    namespace net
    {
        class InetAddress
        {
        public:
            explicit InetAddress(uint16_t port = 0, bool loopbackonly = false); // ADDR_ANY
            explicit InetAddress(struct sockaddr_in &addr)
                : addr_(addr)
            {
            }

            InetAddress(uint16_t port, const string &ip);

            sa_family_t family() const { return addr_.sin_family; }
            string toIp() const;
            string toIpPort() const;
            uint16_t port() const;

            const struct sockaddr *getSockAddr() const { return sockets::sockaddr_cast(&addr_); }
            void setSockAddrInet(const struct sockaddr_in6 &addr6);

            uint32_t ipv4NetEndian() const;
            uint16_t portNetEndian() const { return addr_.sin_port; }

        private:
            struct sockaddr_in addr_;
        };
    }
}

#endif