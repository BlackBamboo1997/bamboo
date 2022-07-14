#include "Logging.h"
#include "CurrentThread.h"

#include <cassert>
#include <errno.h>
#include <stdio.h>
#include <sstream>

namespace bamboo
{

  __thread char t_errnobuf[512];
  __thread char t_time[64];
  __thread time_t t_lastSecond;

  const char *strerror_tl(int savedErrno)
  {
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
  }

  Logger::LogLevel initLogLevel()
  {
    if (::getenv("BAMBOO_LOG_TRACE"))
      return Logger::TRACE;
    else if (::getenv("BAMBOO_LOG_DEBUG"))
      return Logger::DEBUG;
    else
      return Logger::INFO;
  }

  Logger::LogLevel g_logLevel = initLogLevel();

  const char *LogLevelName[Logger::NUM_LOG_LEVELS] =
      {
          "TRACE ",
          "DEBUG ",
          "INFO  ",
          "WARN  ",
          "ERROR ",
          "FATAL ",
  };

  // helper class for known string length at compile time
  class T
  {
  public:
    T(const char *str, unsigned len)
        : str_(str),
          len_(len)
    {
      assert(strlen(str) == len_);
    }

    const char *str_;
    const unsigned len_;
  };

  inline LogStream &operator<<(LogStream &s, T v)
  {
    s.append(v.str_, v.len_);
    return s;
  }

  inline LogStream &operator<<(LogStream &s, const Logger::SourceFile &v)
  {
    s.append(v.data_, v.length_);
    return s;
  }

  void defaultOutput(const char *msg, int len)
  {
    size_t n = fwrite(msg, 1, len, stdout);
    //   if (n <= 0) {
    //       LOG_ERROR << "fwrite fail";
    //   }
  }

  void defaultFlush()
  {
    fflush(stdout);
  }

  Logger::OutputFunc g_output = defaultOutput;
  Logger::FlushFunc g_flush = defaultFlush;

}

using namespace bamboo;

Logger::Logger(SourceFile file, int line) : impl_(INFO, 0, file, line)
{
}
Logger::Logger(SourceFile file, int line, LogLevel level) : impl_(level, 0, file, line)
{
}
Logger::Logger(SourceFile file, int line, LogLevel level, const char *func) : impl_(level, 0, file, line)
{
  impl_.stream_ << func << " ";
}
Logger::Logger(SourceFile file, int line, bool toAbort)
    : impl_(toAbort ? FATAL : ERROR, errno, file, line)
{
}

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile &file, int line)
    : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file)
{
  formatTime();
  currentthread::tid();
  stream_ << ' ';
  stream_ << T(currentthread::tidString(), currentthread::tidStringLength());
  stream_ << T(LogLevelName[level], 6);
  if (savedErrno != 0)
  {
    stream_ << strerror_tl(savedErrno) << "(errno = " << savedErrno << ")";
  }
}

void Logger::Impl::formatTime()
{
  stream_ << time_.toFormattedString();
}

void Logger::Impl::finish()
{
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::~Logger() {
  impl_.finish();
  const LogStream::Buffer& buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if (impl_.level_ == FATAL) {
    g_flush();
    abort();
  }
}

void Logger::setLogLevel(Logger::LogLevel level) {
  g_logLevel = level;
}

void Logger::setOutPut(OutputFunc out) {
  g_output = out;
}

void Logger::setFlush(FlushFunc flush) {
  g_flush = flush;
}