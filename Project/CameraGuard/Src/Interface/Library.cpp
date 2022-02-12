#include "library.h"
#include <easylogging/easylogging++.h>
#include <JsonHelper.h>
#include <Macro.h>
#include <SociHelp.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Config/GlobalConfig.h"
#include "../Cache/session/Session.h"
#include "../Client/CompareClient.h"
#include "../Cache/ConnectionPool/DatabaseConnPool.h"



namespace soci
{
    template<>
    struct type_conversion<Service::LibraryInfo>
    {
        typedef values base_type;
        static void from_base(const values& v, indicator ind, Service::LibraryInfo& library)
        {
            library.nRecordId = v.get<int>("id");
            library.nLibId = v.get<int>("lib_id");
            library.strName = v.get<std::string>("name");
            library.nType = v.get<int>("type");
            library.nPersonCnt = v.get<int>("person_count");
            library.strRemark = v.get<std::string>("remark");
            library.strCreateTime = SociHelper::FetchTime(v, "create_time");
        }
    };
}

namespace JsonHelper
{
    static void to_json(const Service::LibraryInfo& infoLibrary, 
        rapidjson::Value& value, 
        rapidjson::Document::AllocatorType &allocator)
    {
        value.AddMember("lib_id", infoLibrary.nLibId, allocator);
        value.AddMember("name", rapidjson::Value(infoLibrary.strName.c_str(), allocator), allocator);
        value.AddMember("type", infoLibrary.nType, allocator);
        value.AddMember("person_coun", infoLibrary.nPersonCnt, allocator);
        value.AddMember("remark", rapidjson::Value(infoLibrary.strRemark.c_str(), allocator), allocator);
        value.AddMember("create_time", rapidjson::Value(infoLibrary.strCreateTime.c_str(), allocator), allocator);
    }
}


using namespace Service;
int LibraryManagement::AddLibrary(const std::string& strMsg, std::string& strRst)
{
    LibraryInfo infoLibrary;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoLibrary.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoLibrary.strName, doc, String, name, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoLibrary.nType, doc, Int, type, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(infoLibrary.strRemark, doc, String, remark);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(infoLibrary.strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(check_name_exists(infoLibrary.strName));
    VERIFY_RETURN_OPTION(Client::CompareClient::Instance().AddLibrary(infoLibrary.nLibId), CG_ADD_LIBRARY_COMPARE_FAILED);
    VERIFY_CODE_RETURN(add_library(infoLibrary));
    return CG_OK;
}

int LibraryManagement::DeleteLibrary(const std::string& strMsg, std::string& strRst)
{
    std::string strSession;
    int nLibId = 0;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(nLibId, doc, Int, lib_id, strRst, CG_INVALID_PARA);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    Client::CompareClient::Instance().DeleteLibrary(nLibId);

    size_t nPos = 0;
    int nCode = delete_library(nLibId, nPos);
    VERIFY_CODE_RETURN(DatabaseConnPool::Instance().GivebackConn(nCode, nPos));
    return CG_OK;
}

int LibraryManagement::QueryLibraryList(const std::string& strMsg, std::string& strRst)
{
    LibraryQueryCond condQuery;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(condQuery.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(condQuery.nPage, doc, Int, page_no, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(condQuery.nPageSize, doc, Int, page_size, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(condQuery.strName, doc, String, username);
    JS_PARSE_OPTION(condQuery.nType, doc, Int, type);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(condQuery.strSession), CG_INVALID_SESSION);
    int64_t nTotalCnt = 0;
    LibraryList listLibrary;
    VERIFY_CODE_RETURN(query_library_list(condQuery, listLibrary, nTotalCnt));

    JsonHelper::GenerateList(condQuery.nPage, condQuery.nPageSize, nTotalCnt, listLibrary, strRst);
    return CG_OK;
}

int LibraryManagement::check_name_exists(const std::string& strLibName)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);
        conn << "select id from ft_library where visible = 1"
            << " and name=:name"
            , soci::use(strLibName);
        if (conn.got_data())
            return CG_LIBRARY_ALREADY_EXIST;
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Check name of library repeat exception:" << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int LibraryManagement::add_library(const LibraryInfo& infoLibrary)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);
        conn << "insert into ft_library(lib_id,name,type,remark)"
            << " values(:lib_id,:name,:password,:remark)"
            , soci::use(infoLibrary.nLibId)
            , soci::use(infoLibrary.strName)
            , soci::use(infoLibrary.nType)
            , soci::use(infoLibrary.strRemark);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Add library exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int LibraryManagement::delete_library(int nLibId, size_t& nPos)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);
        conn << "update ft_library set visible = 0 where visible = 1"
            << " and lib_id=:id"
            , soci::use(nLibId);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Delete library:" << nLibId << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int LibraryManagement::query_library_list(const LibraryQueryCond& condQuery, 
    LibraryList& listLibrary, 
    int64_t& nCnt)
{
    std::string strSqlCond;
    {
        std::stringstream ss;
        ss << " where visible = 1";
        if (!condQuery.strName.empty())
            ss << " and name like '%" << condQuery.strName << "%'";
        if (condQuery.nType > 0)
            ss << " and type = " << condQuery.nType;
        strSqlCond = ss.str();
    }
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);
        conn << "select count(*) from ft_library"
            << strSqlCond
            , soci::into(nCnt);
        if(0 == nCnt) return CG_OK;
        LibraryInfo infoLibrary;
        soci::statement st = (conn.prepare
            << "select * from ft_library"
            << strSqlCond
            << " limit "<< (condQuery.nPage - 1) * condQuery.nPageSize
            << " ," << condQuery.nPageSize
            , soci::into(infoLibrary));
        st.execute();
        while (st.fetch())
            listLibrary.push_back(infoLibrary);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Query library exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}