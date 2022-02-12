/**************************************************************
* @brief:       配置文件管理
* @date:        20190803
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include "ConfigFile.h"
#include <map>
#include <mutex>
#include <rapidjson/document.h>


#define GLOBAL_CONFIG_INSTANCE GlobalConfig::Instance()
#define CAMERA_GUARD_CONFIG     GLOBAL_CONFIG_INSTANCE.GetConfig()
#define CFG_DATABASE            CAMERA_GUARD_CONFIG.cfgDatabase
#define CFG_HTTP_SERVER         CAMERA_GUARD_CONFIG.cfgHttpServer
#define CFG_STORAGE_SERVER      CAMERA_GUARD_CONFIG.cfgStorageServer
#define CFG_WELCOME_PATTEN      CAMERA_GUARD_CONFIG.cfgWelcome


class GlobalConfig
{
public:
    // database
    struct Database
    {
        std::string strIp = "127.0.0.1";
        std::string strFrontBase = "camera_guard_business";
        std::string strHistoryBase = "camera_guard_history";
        std::string stUser = "bsX17n4dQ9s=";
        std::string strPassword = "bsX17n4dQ9s=";
        std::string strConnFrontendBase;
        std::string strConnHistoryBase;
        int nDatabasePoolSize = 20;
        void LoadFromJson(rapidjson::Value& value);
    };
    // http server
    struct HttpServer
    {
        std::string strIP = "127.0.0.1";
        int port = 10009;
        std::string root = ".";
        std::string strUrlPrefix;
        void LoadFromJson(rapidjson::Value& value);
    };

    struct BaseServerConfig
    {
    public:
        std::string strConfigName;
        std::string strIP = "127.0.0.1";
        int port = 10010;
        std::string strConfig;
        std::string strUrlPrefix;
        BaseServerConfig() {};
        BaseServerConfig(const std::string& strName, int nPort)
            : strConfigName(strName)
            , port(nPort) {}
        void LoadFromDatabase();
        void LoadFromJson(rapidjson::Value& value);
        std::string generate_config();
    };
    // storage server
    struct StorageServer
        : public BaseServerConfig
    {
        StorageServer() :BaseServerConfig("StorageServer", 10011){}
    };

    struct ConfigInfo
    {
        Database cfgDatabase;
        HttpServer cfgHttpServer;
        StorageServer cfgStorageServer;
        std::string cfgWelcome;
    };

private:
    GlobalConfig() {}
    std::string read_file(const std::string& strFile);
    bool parse_config(const std::string& strJsonData);
    void load_from_db();

public:
    static GlobalConfig& Instance()
    {
        static GlobalConfig g_Instance;
        return g_Instance;
    }
    bool Init();
    std::string GetConfig(const std::string& strName);
    void UpdateConfig(const std::string& strName, const std::string& strConfig);
    ConfigInfo GetConfig() { return config; }
    BaseServerConfig GetStorageConfig();

private:
    ConfigInfo config;
    std::mutex mt;
    std::map<std::string, std::string> m_mapConfig;
    std::map<std::string, BaseServerConfig> m_mapServerConfig;

};
