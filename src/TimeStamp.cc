#include "TimeStamp.h"
#include <inttypes.h>
#include <sys/time.h>
#include <chrono>

std::string TimeStamp::toString() const
{
    char buf[32] = {0};
    int seconds = microSecondSinceEpoch_ / kMicroSecondsPerSecond;
    int microSeconds = microSecondSinceEpoch_ % kMicroSecondsPerSecond;
    snprintf(buf,sizeof buf,"%",PRId64,".%06" PRId64 "",seconds,microSeconds);
    return buf;
}
std::string TimeStamp::toFormatString(bool showMicroSeconds) const
{
    char buf[32] = {0};
    time_t seconds = static_cast<time_t>(microSecondSinceEpoch_/ kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    if (showMicroSeconds)
    {
        int microseconds = static_cast<int>(microSecondSinceEpoch_% kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    }
    return buf;
}

TimeStamp TimeStamp::now()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    int64_t seconds = tv.tv_sec;
    return TimeStamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}