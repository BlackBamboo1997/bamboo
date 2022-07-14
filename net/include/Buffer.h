#ifndef BAMBOO_NET_BUFFER_H
#define BAMBOO_NET_BUFFER_H

#include "Types.h"

#include "Endian.h"

#include <vector>
#include <cassert>

namespace bamboo
{
    namespace net
    {
        class Buffer
        {
        public:
            static const size_t kCheapPrepend = 8;
            static const size_t kInitialSize = 1024;

            explicit Buffer(size_t initialSize = kInitialSize)
                : readerIndex_(kCheapPrepend),
                writerIndex_(kCheapPrepend),
                buffer_(kCheapPrepend + initialSize)
            {}

            size_t readableBytes() const { return writerIndex_ - readerIndex_; }
            size_t writableBytes() const { return buffer_.size() - writerIndex_; }
            size_t prependBytes() const { return readerIndex_; }

            char *beginWrite() { return begin() + writerIndex_; }
            const char* beginWrite() const { return begin() + writerIndex_;  }
            void cancelWrite(size_t len) { assert(len <= readableBytes()); writerIndex_ -= len; }
            string toString() { return string(peek(), static_cast<int>(readableBytes()));}

            const char* findCRLF() const;
            const char* findCRLF(const char* start) const;
            const char* findEOL() const;
            const char* findEOL(const char* start) const;

            const char *peek() const { return begin() + readerIndex_; }
            int64_t peekInt64() const;
            int32_t peekInt32() const;
            int16_t peekInt16() const;
            int8_t peekInt8() const;

            int64_t readInt64();
            int32_t readInt32();
            int16_t readInt16();
            int8_t readInt8();

            //use network endian, ensure an agreement
            void prepend(const void *data, size_t len);
            void prependInt64(int64_t x) { x = sockets::hostToNetwork64(x); prepend(&x, sizeof(x)); }
            void prependInt32(int32_t x) { x = sockets::hostToNetwork32(x); prepend(&x, sizeof(x)); }
            void prependInt16(int16_t x) { x = sockets::hostToNetwork16(x); prepend(&x, sizeof(x)); }
            void prependInt8(int8_t x) {prepend(&x, sizeof(x)); }

            void append(const char *data, size_t len);
            void append(const void *data, size_t len) { append(static_cast<const char *>(data), len); }
            void append(const string &data) { append(data.c_str(), data.size()); }
            void appendInt64(int64_t x) { x = sockets::hostToNetwork64(x); append(&x, sizeof(x)); }
            void appendInt32(int32_t x) { x = sockets::hostToNetwork32(x); append(&x, sizeof(x)); }
            void appendInt16(int16_t x) { x = sockets::hostToNetwork16(x); append(&x, sizeof(x)); }
            void appendInt8(int8_t x) { append(&x, sizeof(x)); }

            void retrieveAll();
            void retrieve(size_t len);
            void retrieveUntil(const char *end);
            void retrieveInt64() { retrieve(sizeof(int64_t)); }
            void retrieveInt32() { retrieve(sizeof(int32_t)); }
            void retrieveInt16() { retrieve(sizeof(int16_t)); }
            void retrieveInt8() { retrieve(sizeof(int8_t)); }
            string retrieveAsString(size_t len);
            string retrieveAllAsString() { return retrieveAsString(readableBytes()); }   

            void ensureWritableBytes(size_t len);
            void hasWritten(size_t len);
            size_t internalCapacity() const { return buffer_.capacity(); }

            ssize_t readFd(int fd, int& savederrno);

        private:
            char *begin() { return &*buffer_.begin(); }
            const char *begin() const { return &*buffer_.begin(); }
            void makeSpace(size_t len);

            size_t readerIndex_;
            size_t writerIndex_;
            std::vector<char> buffer_;

            const static char kCRLF[];  // \r\n
        };

        inline int8_t Buffer::peekInt8() const
        {
            assert(readableBytes() >= sizeof(int8_t));
            int8_t x = *peek();
            // memcpy(&x, peek(), sizeof(x));
            return x;
        }

        inline int8_t Buffer::readInt8()
        {
            int8_t x = peekInt8();
            retrieveInt8();
            return x;
        }
    }
}

#endif