#include "SystemConfig.h"
#include <soci/soci.h>
#include <easylogging/easylogging++.h>
#include <JsonHelper.h>
#include <Macro.h>
#include <SociHelp.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Config/GlobalConfig.h"
#include "../Cache/session/Session.h"
#include "../Cache/ConnectionPool/DatabaseConnPool.h"


namespace soci
{
    template<>
    struct type_conversion<Service::SystemConfigInfo>
    {
        typedef values base_type;
        static void from_base(const values& v, indicator ind, Service::SystemConfigInfo& config)
        {
            config.strName = v.get<std::string>("name");
            config.strConfig = v.get<std::string>("config");
        }
    };
}

namespace JsonHelper
{
    static void to_json(const Service::SystemConfigInfo& infoPersConfig,
        rapidjson::Value& value,
        rapidjson::Document::AllocatorType &allocator)
    {
        value.AddMember("name", rapidjson::Value(infoPersConfig.strName.c_str(), allocator), allocator);
        value.AddMember("config", rapidjson::Value(infoPersConfig.strConfig.c_str(), allocator), allocator);
    }

}


using namespace Service;
int SystemConfigManagement::UpdateSystemConfig(const std::string& strMsg, std::string& strRst)
{
    SystemConfigInfo infoConfig;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoConfig.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoConfig.strName, doc, String, name, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoConfig.strConfig, doc, String, config, strRst, CG_INVALID_PARA);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(infoConfig.strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(update_system_config(infoConfig));
    return CG_OK;
}

int SystemConfigManagement::GetSystemConfig(const std::string& strMsg, std::string& strRst)
{
    std::string strSession;
    std::string strName;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(strName, doc, String, name, strRst, CG_INVALID_PARA);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    SystemConfigInfo infoConfig;
    VERIFY_CODE_RETURN(get_system_config(strName, infoConfig));
    JsonHelper::GenerateObject(infoConfig, strRst);
    return CG_OK;
}

bool SystemConfigManagement::CheckConfigExist(const std::string& strName, std::string& strConfig)
{
    soci::session conn;
    try
    {
        conn.open(CFG_DATABASE.strConnFrontendBase);
        conn << "select config from ft_config where visible = 1"
            << " and name=:name"
            , soci::into(strConfig)
            , soci::use(strName);
        return conn.got_data();
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Check system config of :" << strName << " exception: " << e.what();
        return false;
    }
    return true;
}

int SystemConfigManagement::InitSystemConfig(const std::string& strName, std::string& strConfig)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "insert into ft_config(name,config)"
            << " values(:name,:config)"
            , soci::use(strName)
            , soci::use(strConfig);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "init system config of " << strName << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int SystemConfigManagement::update_system_config(const SystemConfigInfo& infoConfig)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "select id from ft_config where visible= 1"
            << " and name=:name"
            , soci::use(infoConfig.strName);
        if (conn.got_data())
        {
            conn << "update ft_config set config = :config where visible = 1"
                << " and name=:name"
                , soci::use(infoConfig.strConfig)
                , soci::use(infoConfig.strName);
        }
        else
        {
            conn<<"insert into ft_config(name,config)"
                <<" values(:name,:config)"
                , soci::use(infoConfig.strName)
                , soci::use(infoConfig.strConfig);
        }
        GlobalConfig::Instance().UpdateConfig(infoConfig.strName, infoConfig.strConfig);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Update system config of " << infoConfig.strName << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int SystemConfigManagement::get_system_config(const std::string& strName, SystemConfigInfo& infoConfig)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "select name, config from ft_config where visible = 1"
            << " and name=:name"
            , soci::into(infoConfig.strName)
            , soci::into(infoConfig.strConfig)
            , soci::use(strName);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Get system config of :" << infoConfig.strName << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}
