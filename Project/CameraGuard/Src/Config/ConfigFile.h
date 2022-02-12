/**************************************************************
* @brief:       配置文件变量定义
* @date:        20190709
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <string>


namespace Config
{
    //conf
    static std::string kConfigDir = "conf//";
    static std::string kConfigName = "config.json";
    static std::string kLogConfig = "log.conf";
    static std::string kCompareClientConfig = "compare.client";
    static std::string kTaskClientConfig = "task.client";


    /************************************************************************
    *                                   config                              *
    *************************************************************************/
    static std::string GetConfigDir()
    {
        return kConfigDir;
    }

    static std::string GetConfigFile()
    {
        return  GetConfigDir() + kConfigName;
    }

    static std::string GetLogConfig()
    {
        return GetConfigDir() + kLogConfig;
    }

    static std::string GetCompareClientConfig()
    {
        return GetConfigDir() + kCompareClientConfig;
    }

    static std::string GetTaskClientConfig()
    {
        return GetConfigDir() + kTaskClientConfig;
    }

}
