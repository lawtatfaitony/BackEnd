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
#include "../common/CrossPlat.h"
#include <fstream>
#include <vector>
#include <rapidjson/document.h>

#define SERVER_CONFIG GlobalConfig::Instance().GetConfig()

class GlobalConfig
{
public:
    struct ConfigServer
    {
        int port = 10012;
        void LoadFromJson(rapidjson::Value& value);
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
    ConfigServer GetConfig() { return config; }

private:
    ConfigServer config;
};

