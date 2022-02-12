#include "user.h"
#include <rapidjson/document.h>
#include <soci/soci.h>
#include <easylogging/easylogging++.h>
#include <JsonHelper.h>
#include <Macro.h>
#include <Basic/Guuid.h>
#include <Basic/Time.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Config/GlobalConfig.h"
#include "../Cache/session/Session.h"
#include "../Cache/ConnectionPool/DatabaseConnPool.h"


using namespace Service;
int UserManagement::Login(const std::string& strMsg, std::string& strRst)
{
    UserInfo infoUser;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoUser.strName,doc,String, username, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoUser.strPassword, doc, String, password, strRst, CG_INVALID_PARA);

    VERIFY_CODE_RETURN(login(infoUser, strRst));
    return CG_OK;
}

int UserManagement::RegisterUser(const std::string& strMsg, std::string& strRst)
{
    UserInfo infoUser;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoUser.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoUser.strName, doc, String, username, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoUser.strPassword, doc, String, password, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(infoUser.strRemark, doc, String, remark);

    VERIFY_CODE_RETURN(register_user(infoUser));
    return CG_OK;
}

int UserManagement::UpdateUser(const std::string& strMsg, std::string& strRst)
{
    UserInfo infoUser;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoUser.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoUser.nUserId, doc, Int, user_id, strRst, CG_INVALID_PARA);

    std::stringstream ss;
    ss << "update ft_user set visible = 1";
    JS_PARSE_OPTION_UPDATE_STRING(infoUser.strName, doc, String, name, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoUser.strPassword, doc, String, password, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoUser.strRemark, doc, String, remark, ss);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(infoUser.strSession), CG_INVALID_SESSION);
    if (!infoUser.strPassword.empty())
        JS_PARSE_REQUIRED_RETURN(infoUser.strOldPassword, doc, String, old_password, strRst, CG_INVALID_PARA);
    VERIFY_CODE_RETURN(update_user(infoUser, ss.str()));
    return CG_OK;
}

int UserManagement::DeleteUser(const std::string& strMsg, std::string& strRst)
{
    std::string strSession;
    int nUserId = 0;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(nUserId, doc, Int, user_id, strRst, CG_INVALID_PARA);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(nUserId);
    return CG_OK;
}

int UserManagement::login(const UserInfo& infoUser, std::string& strRst)
{
    int nUserId = 0;
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        std::string strPwd;
        conn << "select id,password from ft_user where visible = 1"
            << " and name=:name"
            , soci::into(nUserId)
            , soci::into(strPwd)
            , soci::use(infoUser.strName);
        if (!conn.got_data())
            return CG_INVALID_USER;
        if (strPwd != infoUser.strPassword)
            return CG_PASSWORD_ERROR;
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Login exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    generate_result(nUserId, strRst);
    return CG_OK;
}

int UserManagement::register_user(const UserInfo& infoUser)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "select id from ft_user where visible = 1"
            << " and name=:name"
            , soci::use(infoUser.strName);
        if (conn.got_data())
            return CG_USER_ALREADY_EXIST;
        conn << "insert into ft_user(name,password,remark)"
            << " values(:name,:password,:remark)"
            , soci::use(infoUser.strName)
            , soci::use(infoUser.strPassword)
            , soci::use(infoUser.strRemark);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Register user exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int UserManagement::update_user(const UserInfo& infoUser, const std::string& strUpdateSql)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        if (!infoUser.strName.empty())
        {
            conn << "select id from ft_user where visible = 1"
                << " and name=:name"
                , soci::use(infoUser.strName);
            if (conn.got_data())
                return CG_USER_ALREADY_EXIST;
        }
        if (!infoUser.strPassword.empty())
        {
            conn << "select id from ft_user where visible = 1"
                << " and id=:id and password= :password"
                , soci::use(infoUser.nUserId)
                , soci::use(infoUser.strOldPassword);
            if (!conn.got_data())
                return CG_PASSWORD_ERROR;
        }
        conn << strUpdateSql
            << " where id=:id"
            , soci::use(infoUser.nUserId);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Update user:" << infoUser.nUserId << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int UserManagement::delete_user(int nUserId)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "update ft_user set visible = 0 where visible = 1"
            << " and id=:id"
            , soci::use(nUserId);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Delete user:" << nUserId << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

void UserManagement::generate_result(int nUserId, std::string& strRst)
{
    std::string strSession = Basic::GenerateGuuid();
    SessionMangement::Instance().PushSession(strSession);
    {
        rapidjson::Document doc(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();
        doc.AddMember("code", 0, typeAllocate);
        // info
        {
            rapidjson::Value valueInfo(rapidjson::kObjectType);
            valueInfo.AddMember("user_id", nUserId, typeAllocate);
            valueInfo.AddMember("session", rapidjson::Value(strSession.c_str(), typeAllocate), typeAllocate);

            doc.AddMember("info", valueInfo, typeAllocate);
        }
        doc.AddMember("msg", rapidjson::Value("", typeAllocate), typeAllocate);
        // serialize the data 
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
        doc.Accept(writer);
        strRst = buffer.GetString();
    }
}