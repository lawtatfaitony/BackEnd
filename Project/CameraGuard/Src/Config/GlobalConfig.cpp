#include "GlobalConfig.h"
#include <sstream>
#include <fstream>
#include <easylogging/easylogging++.h>
#include <soci/soci.h>
#include <rapidjson/document.h>
#include <Basic/Base64.h>
#include <Basic/Encrypt.h>
#include <Basic/CrossPlat.h>
#include "Macro.h"
#include "JsonHelper.h"
#include "../interface/SystemConfig.h"



bool GlobalConfig::Init()
{
    VERIFY_EXPR_RETURN(parse_config(read_file(Config::GetConfigFile())), false);
    load_from_db();
    return true;
}

std::string GlobalConfig::GetConfig(const std::string& strName)
{
    SCOPE_LOCK(mt);
    auto itMatch = m_mapConfig.find(strName);
    if (itMatch == m_mapConfig.end())
        return std::string();
    return itMatch->second;
}

void GlobalConfig::UpdateConfig(const std::string& strName, const std::string& strConfig)
{
    GlobalConfig::BaseServerConfig config;
    {
        rapidjson::Document doc;
        JS_PARSE_OBJECT_VOID(doc, strConfig);
        JS_PARSE_OPTION(config.strIP, doc, String, ip);
        JS_PARSE_OPTION(config.port, doc, Int, ip);
        config.strConfig = strConfig;
        config.strConfigName = strName;
    }
    SCOPE_LOCK(mt);
    m_mapConfig[strName] = strConfig;
    m_mapServerConfig[strName] = config;
}

GlobalConfig::BaseServerConfig GlobalConfig::GetStorageConfig()
{
    SCOPE_LOCK(mt);
    auto itMatch = m_mapServerConfig.find("StorageServer");
    if (itMatch != m_mapServerConfig.end())
        return itMatch->second;
    return GlobalConfig::BaseServerConfig();
}

std::string GlobalConfig::read_file(const std::string& strFile)
{
    if (0 == CG_access(strFile.c_str(), 0))
    {
        std::fstream fin(strFile, std::ios::in);
        if (fin.is_open())
        {
            fin.seekg(0, std::ios::end);
            int64_t nFileSize = fin.tellp();
            fin.seekg(0, std::ios::beg);
            std::vector<char>vecData(nFileSize + 1, 0);
            fin.read(vecData.data(), nFileSize);
            fin.close();
            LOG(INFO) << "Load config file:" << strFile << " success";
            return vecData.data();
        }
    }
    LOG(ERROR) << "Can't find config file:" << strFile;
    return "";
}

bool GlobalConfig::parse_config(const std::string& strJsonData)
{
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strJsonData, false);
    if (doc.HasMember("database") && doc["database"].IsObject())
    {
        config.cfgDatabase.LoadFromJson(doc["database"]);
    }
    if (doc.HasMember("http_server") && doc["http_server"].IsObject())
    {
        config.cfgHttpServer.LoadFromJson(doc["http_server"]);
    }
    if (doc.HasMember("storage_server") && doc["storage_server"].IsObject())
    {
        config.cfgStorageServer.LoadFromJson(doc["storage_server"]);
    }
    if (doc.HasMember("welcome_patten") && doc["welcome_patten"].IsObject())
    {
        auto& j_welcome_patten = doc["welcome_patten"];
        JS_PARSE_OPTION(config.cfgWelcome, j_welcome_patten, String, patten);
    }

    return true;
}

void GlobalConfig::load_from_db()
{
    config.cfgStorageServer.LoadFromDatabase();

}

void GlobalConfig::Database::LoadFromJson(rapidjson::Value& value)
{
    static auto func_decrypt = [] (const std::string& strInput)
    {
        static std::string strKey = "cameraen";
        static std::string strIvValue = "00000000";
        return Basic::Encrypt::DesCBCDecrypt(strKey, strIvValue, Basic::Base64::Decode(strInput));
    };
    static auto func_generate_conn_str = [](std::string& strConn, const std::string& strHost,
        const std::string& strBaseName, const std::string& strUser, const std::string& strPassword) {
            std::stringstream ss;
            ss << "mysql://"
                << "host=" << strHost
                << " port=" << 3306
                << " db=" << strBaseName
                << " user=" << strUser
                << " password=" << strPassword
                << " charset = utf8";
            strConn = ss.str();
    };
    JS_PARSE_OPTION(strIp, value, String, ip);
    JS_PARSE_OPTION(strFrontBase, value, String, base_frontend);
    JS_PARSE_OPTION(strHistoryBase, value, String, base_history);
    JS_PARSE_OPTION(stUser, value, String, user);
    JS_PARSE_OPTION(strPassword, value, String, password);

    stUser = func_decrypt(stUser);
    strPassword = func_decrypt(strPassword);
    func_generate_conn_str(strConnFrontendBase, strIp, strFrontBase, stUser, strPassword);
    func_generate_conn_str(strConnHistoryBase, strIp, strHistoryBase, stUser, strPassword);
    JS_PARSE_OPTION(nDatabasePoolSize, value, Int, pool_count);
}

void GlobalConfig::HttpServer::LoadFromJson(rapidjson::Value& value)
{
    JS_PARSE_OPTION(strIP, value, String, host);
    JS_PARSE_OPTION(port, value, Int, port);
    JS_PARSE_OPTION(root, value, String, root);
    {
        std::stringstream ss;
        ss << "http://" << strIP << ":" << port << "/";
        strUrlPrefix = ss.str();
    }
}

void GlobalConfig::BaseServerConfig::LoadFromDatabase()
{
    if (Service::SystemConfigManagement::CheckConfigExist(strConfigName, strConfig))
    {
        // parse the config
        rapidjson::Document doc;
        JS_PARSE_OBJECT_VOID(doc, strConfig);
        JS_PARSE_OPTION(strIP, doc, String, ip);
        JS_PARSE_OPTION(port, doc, Int, ip);
        {
            std::stringstream ss;
            ss << "http://" << strIP << ":" << port << "/";
            strUrlPrefix = ss.str();
        }
    }
    else
    {
        strConfig = generate_config();
        Service::SystemConfigManagement::InitSystemConfig(strConfigName, strConfig);
    }
    GlobalConfig::Instance().UpdateConfig(strConfigName, strConfig);
    return;
}

void GlobalConfig::BaseServerConfig::LoadFromJson(rapidjson::Value& value)
{
    JS_PARSE_OPTION(strIP, value, String, host);
    JS_PARSE_OPTION(port, value, Int, port);
    {
        std::stringstream ss;
        ss << "http://" << strIP << ":" << port << "/";
        strUrlPrefix = ss.str();
    }
}

std::string GlobalConfig::BaseServerConfig::generate_config()
{
    rapidjson::Document doc(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();

    doc.AddMember("ip", rapidjson::Value(strIP.c_str(), typeAllocate), typeAllocate);
    doc.AddMember("port", port, typeAllocate);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}
