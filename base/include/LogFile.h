#ifndef BAMBOO_BASE_LOGFILE_H
#define BAMBOO_BASE_LOGFILE_H

#include "Types.h"
#include "Timestamp.h"
#include "Mutex.h"

#include <memory>

namespace bamboo
{
    namespace fileutil {
        class AppendFile;
    }
    class LogFile
    {
    public:
        LogFile(const string& name, int rollSize, bool threadSafe = true, int flushInterval = 3, int checkEveryN = 1024);
        ~LogFile();

        void append(const char* str, int len);
        void flush();
        bool rollFile();

    private:
        void append_unlocked(const char* str, int len);
        static string getLogFileName(const string& name, time_t* now);

        const string basename_;
        const int rollSize_;
        const int flushInterval_;
        const int checkEveryN_;

        int count_;
        time_t startOfPeriod_;
        time_t lastRoll_;
        time_t lastFlush_;

        std::unique_ptr<Mutex> mutex_;
        std::unique_ptr<fileutil::AppendFile> file_;

        const static int kRollPerSeconds_ = 24 * 60 * 60;
    };
}   // bamboo

#endif
