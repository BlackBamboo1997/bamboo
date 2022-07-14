#ifndef BAMBOO_BASE_LOGGING_H
#define BAMBOO_BASE_LOGGING_H

#include "LogStream.h"
#include "Timestamp.h"

#include <cstring>
#include <functional>

namespace bamboo
{

    class Logger
    {
    public:
        typedef std::function<void(const char *str, int)> OutputFunc;
        typedef std::function<void()> FlushFunc;
        enum LogLevel
        {
            TRACE,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL,
            NUM_LOG_LEVELS,
        };

        class SourceFile
        {
        public:
            template <int N>
            SourceFile(const char (&arr)[N]) : data_(arr), length_(N - 1)
            {
                const char *pos = strrchr(data_, '/');
                if (pos)
                {
                    data_ = pos + 1;
                    length_ -= static_cast<int>(data_ - arr);
                }
            }
            explicit SourceFile(const char *str) : data_(str)
            {
                const char *pos = strrchr(data_, '/');
                if (pos)
                {
                    data_ = pos + 1;
                }
                length_ = strlen(data_);
            }

            const char *data_;
            int length_;
        };

        Logger(SourceFile file, int line);
        Logger(SourceFile file, int line, LogLevel level);
        Logger(SourceFile file, int line, LogLevel level, const char *func);
        Logger(SourceFile file, int line, bool toAbort);
        ~Logger();

        LogStream &stream()
        {
            return impl_.stream_;
        }
        static LogLevel logLevel();
        static void setLogLevel(LogLevel);
        static void setOutPut(OutputFunc);
        static void setFlush(FlushFunc);

    private:
        struct Impl
        {
            typedef Logger::LogLevel LogLevel;
            Impl(LogLevel level, int old_errno, const SourceFile &file, int line);
            void formatTime();
            void finish();

            Timestamp time_;
            LogStream stream_;
            LogLevel level_;
            int line_;
            SourceFile basename_;
        };

        Impl impl_;
    };

    extern Logger::LogLevel g_logLevel;

    inline Logger::LogLevel Logger::logLevel()
    {
        return g_logLevel;
    }

#define LOG_TRACE                                            \
    if (bamboo::Logger::logLevel() <= bamboo::Logger::TRACE) \
    bamboo::Logger(__FILE__, __LINE__, bamboo::Logger::TRACE, __func__).stream()
#define LOG_DEBUG                                            \
    if (bamboo::Logger::logLevel() <= bamboo::Logger::DEBUG) \
    bamboo::Logger(__FILE__, __LINE__, bamboo::Logger::DEBUG, __func__).stream()
#define LOG_INFO                                            \
    if (bamboo::Logger::logLevel() <= bamboo::Logger::INFO) \
    bamboo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN \
    bamboo::Logger(__FILE__, __LINE__, bamboo::Logger::WARN).stream()
#define LOG_ERROR \
    bamboo::Logger(__FILE__, __LINE__, bamboo::Logger::ERROR).stream()
#define LOG_FATAL \
    bamboo::Logger(__FILE__, __LINE__, bamboo::Logger::FATAL).stream()
#define LOG_SYSERR \
    bamboo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL \
    bamboo::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);
} // bamboo

#endif