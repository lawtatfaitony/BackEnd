/**************************************************************
*
* @brief:       配置文件管理
* @date:        20190803
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include "ConfigFile.h"
#include <fstream>
#include <vector>
#include <rapidjson/document.h>
#include <Basic/CrossPlat.h>


#define SERVER_CONFIG GlobalConfig::Instance().GetConfig()
#define CONFIG_HTTP_SERVER SERVER_CONFIG.cfgHttpServer

class GlobalConfig
{
public:
    struct HttpServer
    {
        std::string strIP = "127.0.0.1";
        int port = 10008;
        std::string root = ".";
        std::string strServerInfo;
        void LoadFromJson(rapidjson::Value& value);
    };

    struct ConfigInfo
    {
        HttpServer cfgHttpServer;
    };
private:
    GlobalConfig() {}
    std::string read_file(const std::string& strFile);
    bool parse_config(const std::string& datrData);

public:
    static GlobalConfig& Instance()
    {
        static GlobalConfig g_Instance;
        return g_Instance;
    }
    bool Init();
    ConfigInfo GetConfig() { return config; }

private:
    ConfigInfo config;
};

