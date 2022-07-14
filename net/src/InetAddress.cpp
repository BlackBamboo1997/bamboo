#include "InetAddress.h"

#include "Logging.h"
#include "Endian.h"
#include "SocketsOps.h"

#include <cassert>
#include <arpa/inet.h>

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInaddrAny = INADDR_ANY;
static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic error "-Wold-style-cast"

using namespace bamboo;
using namespace bamboo::net;

InetAddress::InetAddress(uint16_t port, bool loopbackonly)
{
    ::bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackonly ? kInaddrLoopback : kInaddrAny;
    addr_.sin_port = sockets::hostToNetwork16(port);
    addr_.sin_addr.s_addr = sockets::hostToNetwork32(ip);
}

InetAddress::InetAddress(uint16_t port, const string &ip)
{
    ::bzero(&addr_, sizeof addr_);
    sockets::fromIpPort(ip.c_str(), port, &addr_);
}

string InetAddress::toIpPort() const
{
  char buf[64] = "";
  sockets::toIpPort(buf, sizeof buf, getSockAddr());
  return buf;
}

string InetAddress::toIp() const
{
  char buf[64] = "";
  sockets::toIp(buf, sizeof buf, getSockAddr());
  return buf;
}

uint16_t InetAddress::port() const
{
  return sockets::networkToHost16(portNetEndian());
}

void InetAddress::setSockAddrInet(const struct sockaddr_in6& addr6) {
  addr_ = *(sockets::sockaddr_in_cast(sockets::sockaddr_cast(&addr6)));
}

uint32_t InetAddress::ipv4NetEndian() const
{
  assert(family() == AF_INET);
  return addr_.sin_addr.s_addr;
}

