#ifndef BAMBOO_BASE_LOGSTREAM_H
#define BAMBOO_BASE_LOGSTREAM_H

#include "Types.h"

namespace bamboo
{

    namespace detail
    {
        const int ksmallBuffer = 4000;
        const int kbigBuffer = 4000 * 1000;

        template <int SIZE>
        class FixedBuffer
        {
        public:
            FixedBuffer() : cur_(data_) {}
            void append(const char *str, int len)
            {
                if (avail() > len)
                {
                    memcpy(cur_, str, len);
                    cur_ += len;
                }
            }

            int avail() const
            {
                return static_cast<int>(end() - cur_);
            }
            char *current() const
            {
                return cur_;
            }
            void add(int len)
            {
                cur_ += len;
            }

            const char *data() const
            {
                return data_;
            }

            int length() const
            {
                return static_cast<int>(cur_ - data_);
            }

            void reset()
            {
                cur_ = data_;
            }

            void bzero()
            {
                ::bzero(data_, sizeof data_);
            }

        private:
            const char *end() const
            {
                return data_ + sizeof data_;
            }

            char data_[SIZE];
            char *cur_;
        };
    }

    class LogStream
    {
        typedef LogStream self;

    public:
        typedef detail::FixedBuffer<detail::ksmallBuffer> Buffer;
        self &operator<<(bool v)
        {
            buffer_.append(v ? "1" : "0", 1);
            return *this;
        }
        self &operator<<(char c)
        {
            buffer_.append(&c, 1);
            return *this;
        }
        self &operator<<(const void *);
        self &operator<<(const char *str)
        {
            if (str)
            {
                buffer_.append(str, strlen(str));
            }
            else
            {
                buffer_.append("(null)", 6);
            }
            return *this;
        }
        self &operator<<(const string &s)
        {
            buffer_.append(s.c_str(), s.size());
            return *this;
        }

        self &operator<<(int);
        self &operator<<(unsigned int);
        self &operator<<(short);
        self &operator<<(unsigned short);
        self &operator<<(long);
        self &operator<<(unsigned long);
        self &operator<<(long long);
        self &operator<<(unsigned long long);

        self &operator<<(float f)
        {
            *this << static_cast<double>(f);
            return *this;
        }
        self &operator<<(double);

        void append(const char *str, int len) {
            buffer_.append(str, len);
        }

        const Buffer& buffer() const{
            return buffer_;
        }

        void resetBuffer() {
            buffer_.reset();
        }

    private:
        template <class T>
        void formatInteger(T);

        const int kMaxNumericSize = 48;
        Buffer buffer_;
    };

    class Fmt
    {
    public:
        template <class T>
        Fmt(const char *str, T val);

        int length() const
        {
            return length_;
        }
        const char *data() const
        {
            return buf_;
        }

    private:
        char buf_[32];
        int length_;
    };

    inline LogStream &operator<<(LogStream &stream, Fmt fmt)
    {
        stream.append(fmt.data(), fmt.length());
        return stream;
    }
}
#endif