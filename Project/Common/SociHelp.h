#pragma once
#include <exception>
#include <soci/values.h>



namespace SociHelper
{
    static std::string FetchTime(const soci::values& v, const std::string& strField)
    {
        try
        {
            std::tm timeFetch = v.get<std::tm>(strField);
            char szTime[255] = { 0 };
            strftime(szTime, sizeof(szTime), "%Y-%m-%d %H:%M:%S", &timeFetch);
            return szTime;
        }
        catch (const std::exception& e) {}
        return std::string();
    }
}
