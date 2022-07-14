#include "LogStream.h"

#include <cassert>
#include <algorithm>
#include <type_traits>

using namespace bamboo;
using namespace bamboo::detail;

namespace bamboo
{
    namespace detail
    {
        const char digits[] = "9876543210123456789";
        const char *zero = digits + 9;

        const char digitsHex[] = "0123456789ABCDEF";
        static_assert(sizeof digitsHex == 17, "wrong number of digitsHex");

        template <class T>
        int convert(char buf[], T val)
        {
            T i = val;
            char *p = buf;
            do
            {
                int pos = static_cast<int>(i % 10);
                i /= 10;
                *(p++) = zero[pos];
            } while (i != 0);

            if (val < 0)
            {
                *(p++) = '-';
            }
            *p = '\0';
            std::reverse(buf, p);
            return p - buf;
        }

        size_t convertHex(char buf[], uintptr_t value)
        {
            uintptr_t i = value;
            char *p = buf;

            do
            {
                int lsd = static_cast<int>(i % 16);
                i /= 16;
                *p++ = digitsHex[lsd];
            } while (i != 0);

            *p = '\0';
            std::reverse(buf, p);

            return p - buf;
        }
    }
}

template <class T>
void LogStream::formatInteger(T val)
{
    if (buffer_.avail() >= kMaxNumericSize)
    {
        int len = convert(buffer_.current(), val);
        buffer_.add(len);
    }
}

LogStream &LogStream::operator<<(int v)
{
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(unsigned int v)
{
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(long v)
{
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(unsigned long v)
{
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(short v)
{
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(unsigned short v)
{
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(long long v)
{
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(unsigned long long v)
{
    formatInteger(v);
    return *this;
}

LogStream &LogStream::operator<<(double v)
{
    if (buffer_.avail() >= kMaxNumericSize)
    {
        int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
        return *this;
    }
}

LogStream &LogStream::operator<<(const void *p)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (buffer_.avail() >= kMaxNumericSize)
    {
        char *buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = convertHex(buf + 2, v);
        buffer_.add(len + 2);
    }
    return *this;
}

template <class T>
Fmt::Fmt(const char *str, T val)
{
    static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");

    length_ = snprintf(buf_, sizeof buf_, str, val);
    assert(length_ < sizeof buf_);
}

template Fmt::Fmt(const char *str, char);

template Fmt::Fmt(const char *str, short);
template Fmt::Fmt(const char *str, unsigned short);
template Fmt::Fmt(const char *str, int);
template Fmt::Fmt(const char *str, unsigned int);
template Fmt::Fmt(const char *str, long);
template Fmt::Fmt(const char *str, unsigned long);
template Fmt::Fmt(const char *str, long long);
template Fmt::Fmt(const char *str, unsigned long long);

template Fmt::Fmt(const char *str, float);
template Fmt::Fmt(const char *str, double);