#include "Logging.h"
#include "ThreadPool.h"

#include <stdio.h>
#include <unistd.h>

using namespace bamboo;


int g_total;
FILE* g_file;

void dummyOutput(const char* msg, int len)
{
  g_total += len;
  if (g_file)
  {
    fwrite(msg, 1, len, g_file);
  }
}

void bench(const char* type)
{
  bamboo::Logger::setOutPut(dummyOutput);
  Timestamp start(Timestamp::now());
  g_total = 0;

  int n = 1000*1000;
  const bool kLongLog = false;
  string empty = " ";
  string longStr(3000, 'X');
  longStr += " ";
  for (int i = 0; i < n; ++i)
  {
    LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz"
             << (kLongLog ? longStr : empty)
             << i;
  }
  Timestamp end(Timestamp::now());
  double seconds = timeDifference(end, start);
  printf("%12s: %f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n",
         type, seconds, g_total, n / seconds, g_total / seconds / (1024 * 1024));
}

void logInThread()
{
  LOG_INFO << "logInThread";
  usleep(1000);
}

int main()
{
  getppid(); // for ltrace and strace
  
  ThreadPool pool("pool");
  pool.setThreadNum(5);
  pool.start();
  pool.push(logInThread);
  pool.push(logInThread);
  pool.push(logInThread);
  pool.push(logInThread);
  pool.push(logInThread);

  LOG_TRACE << "trace";
  LOG_DEBUG << "debug";
  LOG_INFO << "Hello";
  LOG_WARN << "World";
  LOG_ERROR << "Error";
  LOG_INFO << sizeof(Logger);
  LOG_INFO << sizeof(LogStream);
  LOG_INFO << sizeof(Fmt);
  LOG_INFO << sizeof(LogStream::Buffer);

  sleep(1);
  bench("nop");

  char buffer[64*1024];

  g_file = fopen("/dev/null", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench("/dev/null");
  fclose(g_file);

  g_file = fopen("/tmp/log", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench("/tmp/log");
  fclose(g_file);

  bench("timezone nop");
}
