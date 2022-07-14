#include "LogFile.h"
#include "Logging.h"

#include <unistd.h>

std::unique_ptr<bamboo::LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
  g_logFile->append(msg, len);
}

void flushFunc()
{
  g_logFile->flush();
}

int main(int argc, char* argv[])
{
  bamboo::string name("/home/xing/log/testlog");
//   strncpy(name, argv[0], sizeof name - 1);
  g_logFile.reset(new bamboo::LogFile(name, 200*1000));
  bamboo::Logger::setOutPut(outputFunc);
  bamboo::Logger::setFlush(flushFunc);

  bamboo::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

  for (int i = 0; i < 10000; ++i)
  {
    LOG_INFO << line << i;

    usleep(1000);
  }
}
