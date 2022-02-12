#pragma once
#include <ctime>
#include <string>
#include <chrono>
#include "CrossPlat.h"
#include "Basic.h"


NAMESPACE_BASIC_BEGIN
class Time
{
    static const int kMaxSize = 128;
    static const int kHoursPeyDay = 24;
    static const int kSecondPerHour = 3600;
public:
    static std::string GetCurrentDate()
    {
        return GetCurrentSystemTime("%Y-%m-%d");
    }

    static std::string GetCurrentSystemTime(const std::string& strFormat = "%Y-%m-%d %H:%M:%S")
    {
        return DatetimeToString(time(nullptr), strFormat);
    }

    static void StringToDatetime(const std::string& strTime, time_t& time)
    {
        tm tmTime;
        memset(&tmTime, 0, sizeof(tmTime));

        sscanf_s(strTime.c_str(), "%d-%d-%d %d:%d:%d",
            &(tmTime.tm_year),
            &(tmTime.tm_mon),
            &(tmTime.tm_mday),
            &(tmTime.tm_hour),
            &(tmTime.tm_min),
            &(tmTime.tm_sec));

        tmTime.tm_year -= 1900;
        tmTime.tm_mon -= 1;

        time = mktime(&tmTime);
    }

    static std::string DatetimeToString(time_t time, const std::string& strFormat = "%Y-%m-%d %H:%M:%S")
    {
        tm tmBuffer = { 0 };
        CG_local_time(time, tmBuffer);
        char szTime[kMaxSize] = { 0 };
        strftime(szTime, kMaxSize, strFormat.c_str(), &tmBuffer);
        return std::string(szTime);
    }

    static std::tm DatetimeToTM(time_t time)
    {
        std::tm tmBuffer = { 0 };
        CG_local_time(time, tmBuffer);
        return tmBuffer;
    }

    static bool IsSameDay(time_t timePre, time_t timeNow)
    {
        std::string strTimePre = DatetimeToString(timePre, "%Y-%m-%d");
        std::string strTimeNow = DatetimeToString(timeNow, "%Y-%m-%d");
        return strTimePre == strTimeNow;
    }

    static int DiffDays(time_t timePre, time_t timeNow)
    {
        int64_t nDiffSeconds = timeNow - timePre;
        return nDiffSeconds / (kHoursPeyDay * kSecondPerHour);
    }

    static int64_t GetMilliTimestamp()
    {
        auto curr = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            curr.time_since_epoch());
        return ms.count();
    }
};
NAMESPACE_BASIC_END