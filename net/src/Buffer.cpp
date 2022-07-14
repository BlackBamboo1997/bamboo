#include "Buffer.h"
#include "Socket.h"
#include "SocketsOps.h"

#include <algorithm>
#include <cassert>
#include <sys/uio.h>

using namespace bamboo;
using namespace bamboo::net;

const char Buffer::kCRLF[] = "\r\n";
// we need algorithm of range_find, not use strstr strchr
const char *Buffer::findCRLF() const
{
    // const char *pos = strstr(begin(), kCRLF);
    const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? NULL : crlf;
}

const char *Buffer::Buffer::findCRLF(const char *start) const
{
    assert(peek() <= start);
    assert(start <= beginWrite());
    const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? NULL : crlf;
}

const char *Buffer::findEOL() const
{
    // const char *pos = strchr(begin(), '\n');
    const void *eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char *>(eol);
}

const char *Buffer::findEOL(const char *start) const
{
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void *eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char *>(eol);
}

void Buffer::prepend(const void *data, size_t len)
{
    assert(len <= prependBytes());
    readerIndex_ -= len;
    const char *d = static_cast<const char *>(data);
    std::copy(d, d + len, begin() + readerIndex_);
}

void Buffer::append(const char *data, size_t len)
{
    ensureWritableBytes(len);
    // const char* d = static_cast<const char *>(data);
    // memcpy(begin() + writerIndex_, d, len);
    std::copy(data, data + len, begin() + writerIndex_);
    hasWritten(len);
}

void Buffer::ensureWritableBytes(size_t len)
{
    if (writableBytes() < len)
    {
        makeSpace(len);
    }
}

void Buffer::makeSpace(size_t len)
{
    if (prependBytes() + writableBytes() < len + kCheapPrepend)
    {
        buffer_.resize(buffer_.size() + len);
    }
    else
    { // space enough, adjust
        assert(kCheapPrepend < readerIndex_);
        size_t readable = readableBytes();
        std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;
        assert(readable == readableBytes());
    }
}

void Buffer::hasWritten(size_t len)
{
    assert(len <= writableBytes());
    writerIndex_ += len;
}

void Buffer::retrieveAll()
{
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
}

void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    if (len < readableBytes())
    {
        readerIndex_ += len;
    }
    else
    {
        retrieveAll();
    }
}

void Buffer::retrieveUntil(const char *end)
{
    assert(peek() <= end && end <= beginWrite());
    retrieve(end - peek());
}

string Buffer::retrieveAsString(size_t len)
{

    assert(len <= readableBytes());
    string result(peek(), len);
    retrieve(len);
    return result;
}

int64_t Buffer::peekInt64() const
{
    assert(readableBytes() >= sizeof(int64_t));
    int64_t x = 0;
    memcpy(&x, peek(), sizeof(x));
    return sockets::networkToHost64(x);
}

int32_t Buffer::peekInt32() const
{
    assert(readableBytes() >= sizeof(int32_t));
    int32_t x = 0;
    memcpy(&x, peek(), sizeof(x));
    return sockets::networkToHost32(x);
}

int16_t Buffer::peekInt16() const
{
    assert(readableBytes() >= sizeof(int16_t));
    int16_t x = 0;
    memcpy(&x, peek(), sizeof(x));
    return sockets::networkToHost16(x);
}

int64_t Buffer::readInt64()
{
    int64_t x = peekInt64();
    retrieveInt64();
    return x;
}

int32_t Buffer::readInt32()
{
    int32_t x = peekInt32();
    retrieveInt32();
    return x;
}

int16_t Buffer::readInt16()
{
    int16_t x = peekInt16();
    retrieveInt16();
    return x;
}

ssize_t Buffer::readFd(int fd, int& savederrno)
{
    //use stack space 65536, maybe too big
    char extrabuf[65536];
    const size_t writable = writableBytes();
    //sockets::readv
    struct iovec vec[2];
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    int count = writable >= sizeof(extrabuf) ? 1 : 2;
    ssize_t n = sockets::readv(fd, vec, count);
    if (n < 0) {
        savederrno = errno;
    }
    else if (implicit_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    }
    else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }
    return n;
}