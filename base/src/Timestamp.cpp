#include "Timestamp.h"
// #include "Types.h" //for string to_string

#include <sys/time.h>
#include <inttypes.h>

using namespace bamboo;

static_assert(sizeof(Timestamp) == sizeof(int64_t), "Timestamp should be same size as int64_t");

string Timestamp::toString() const
{
    string seconds = std::to_string(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    string mircoSeconds = std::to_string(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
    return seconds + "." + mircoSeconds;
}

string Timestamp::toFormattedString(bool showMircoSeconds) const
{
    char buf[64] = {0};
    time_t t = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&t, &tm_time);
    if (showMircoSeconds)
    {
        snprintf(buf, sizeof buf, "%4d%02d%02d %02d:%02d:%02d:%06d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                 static_cast<int>(microSecondsSinceEpoch_% kMicroSecondsPerSecond));
    }
    else {
        snprintf(buf, sizeof buf, "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

Timestamp Timestamp::now(double timeDiff)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp((seconds + timeDiff * 60 * 60) * kMicroSecondsPerSecond + tv.tv_usec);
}