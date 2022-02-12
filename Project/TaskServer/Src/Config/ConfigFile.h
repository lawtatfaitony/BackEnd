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
#ifdef WIN32
    #include <direct.h>
    #include <windows.h>
#else
    #include <unistd.h> //linux
#endif



namespace Config
{
    //conf
    static std::string kConfigDir = "conf//";
    static std::string kConfigName = "config.json";
    static std::string kLogConfig = "log.conf";
    static std::string kCompareClientConfig = "compare.client";
    static std::string kTaskServerConfig = "task.server";



    /************************************************************************
    *                                   function                            *
    *************************************************************************/
    //获取当前路径
    static std::string GetCurrentPath()
    {
        const int MAX_RESOURCE_PATH = 260;
        char szPath[MAX_RESOURCE_PATH] = { 0 };
        _getcwd(szPath, MAX_RESOURCE_PATH);
        return szPath;
    }

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

    static std::string GetServerConfig()
    {
        return GetConfigDir() + kTaskServerConfig;
    }

};