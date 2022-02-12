#include "camera.h"
#include <regex>
#include <soci/soci.h>
#include <easylogging/easylogging++.h>
#include <JsonHelper.h>
#include <Macro.h>
#include <SociHelp.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Config/GlobalConfig.h"
#include "../Cache/session/Session.h"
#include "../Cache/camera/DiscoverCamera.h"
#include "../Cache/camera/CameraState.h"
#include "../Cache/ConnectionPool/DatabaseConnPool.h"


namespace soci
{
    template<>
    struct type_conversion<Service::CameraInfo>
    {
        typedef values base_type;
        static void from_base(const values& v, indicator ind, Service::CameraInfo& camera)
        {
            camera.nCameraId = v.get<int>("id");
            camera.strName = v.get<std::string>("name");
            camera.nType = v.get<int>("type");
            camera.strIp = v.get<std::string>("ip");
            camera.strUser = v.get<std::string>("username");
            camera.strPassword = v.get<std::string>("password");
            camera.strRtsp = v.get<std::string>("rtsp");
            camera.strRemark = v.get<std::string>("remark");
            camera.strCreateTime = SociHelper::FetchTime(v, "create_time");
        }
    };
}

namespace JsonHelper
{
    static void to_json(const Service::CameraInfo& infoCamera,
        rapidjson::Value& value,
        rapidjson::Document::AllocatorType &allocator)
    {
        value.AddMember("camera_id", infoCamera.nCameraId, allocator);
        value.AddMember("name", rapidjson::Value(infoCamera.strName.c_str(), allocator), allocator);
        value.AddMember("type", infoCamera.nType, allocator);
        value.AddMember("rtsp", rapidjson::Value(infoCamera.strRtsp.c_str(), allocator), allocator);
        value.AddMember("online", infoCamera.nOnline, allocator);
        value.AddMember("remark", rapidjson::Value(infoCamera.strRemark.c_str(), allocator), allocator);
        value.AddMember("create_time", rapidjson::Value(infoCamera.strCreateTime.c_str(), allocator), allocator);
    }

    static void to_json(const Service::SearchCamera& infoCamera,
        rapidjson::Value& value,
        rapidjson::Document::AllocatorType &allocator)
    {
        value.AddMember("ip", rapidjson::Value(infoCamera.strIp.c_str(), allocator), allocator);
        value.AddMember("manufacturer", rapidjson::Value(infoCamera.strManufacturer.c_str(), allocator), allocator);
    }

}



using namespace Service;
int CameraManagement::AddCamera(const std::string& strMsg, std::string& strRst)
{
    CameraInfo infoCamera;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoCamera.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoCamera.strName, doc, String, name, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoCamera.strRtsp, doc, String, rtsp, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoCamera.nType, doc, Int, type, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(infoCamera.strRemark, doc, String, remark);

    if(infoCamera.strRtsp.empty())
        return CG_CAMERA_INVALID_RTSP;
    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(infoCamera.strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(parse_camera_rtsp(infoCamera.strRtsp, infoCamera));
    Cache::CameraStateManagement::Instance().AddCamera(infoCamera.strIp);
    VERIFY_CODE_RETURN(add_camera(infoCamera));
    return CG_OK;
}

int CameraManagement::DeleteCamera(const std::string& strMsg, std::string& strRst)
{
    std::string strSession;
    int nCameraId = 0;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(nCameraId, doc, Int, camera_id, strRst, CG_INVALID_PARA);

    VERIFY_RETURN_OPTION(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(delete_camera(nCameraId));
    return CG_OK;
}

int CameraManagement::UpdateCamera(const std::string& strMsg, std::string& strRst)
{
    CameraInfo infoCamera;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoCamera.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoCamera.nCameraId, doc, Int, camera_id, strRst, CG_INVALID_PARA);

    std::stringstream ss;
    ss << "update ft_camera set visible = 1";
    JS_PARSE_OPTION_UPDATE_STRING(infoCamera.strName, doc, String, name, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoCamera.strRtsp, doc, String, rtsp, ss);
    JS_PARSE_OPTION_UPDATE_INT(infoCamera.nType, doc, Int, type, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoCamera.strRemark, doc, String, remark, ss);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(infoCamera.strSession), CG_INVALID_SESSION);
    if (!infoCamera.strRtsp.empty())
    {
        VERIFY_CODE_RETURN(parse_camera_rtsp(infoCamera.strRtsp, infoCamera));
        ss << ",ip = '" << infoCamera.strIp << "'";
        ss << ",username = '" << infoCamera.strUser << "'";
        ss << ",password = '" << infoCamera.strPassword << "'";
    }
    VERIFY_CODE_RETURN(update_camera(infoCamera, ss.str()));
    return CG_OK;
}

int CameraManagement::QueryCameraList(const std::string& strMsg, std::string& strRst)
{
    CameraQueryCond condQuery;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(condQuery.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(condQuery.nPage, doc, Int, page_no, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(condQuery.nPageSize, doc, Int, page_size, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(condQuery.strName, doc, String, username);
    JS_PARSE_OPTION(condQuery.nType, doc, Int, type);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(condQuery.strSession), CG_INVALID_SESSION);
    int nTotalCnt = 0;
    CameraList listCamera;
    VERIFY_CODE_RETURN(query_camera_list(condQuery, listCamera, nTotalCnt));
    JsonHelper::GenerateList(condQuery.nPage, condQuery.nPageSize, nTotalCnt, listCamera, strRst);
    return CG_OK;
}

int CameraManagement::SearchCamera(const std::string& strMsg, std::string& strRst)
{
    std::string strSession;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    int nTotalCnt = 0;
    SearchCameraList listCamera;
    VERIFY_CODE_RETURN(search_camera(listCamera, nTotalCnt));
    JsonHelper::GenerateListResult(nTotalCnt, listCamera, strRst);
    return CG_OK;
}

int CameraManagement::QueryCameraRtsp(const std::string& strMsg, std::string& strRst)
{
    CameraInfo infoCamera;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoCamera.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoCamera.strIp, doc, String, ip, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(infoCamera.strUser, doc, String, user);
    JS_PARSE_OPTION(infoCamera.strPassword, doc, String, password);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(infoCamera.strSession), CG_INVALID_SESSION);
    std::string strRtsp;
    strRst = Cache::DiscoveryCamera::Instance().SearchCameraRtsp(infoCamera.strIp, infoCamera.strUser, infoCamera.strPassword, strRtsp);
    VERIFY_EXPR_RETURN(strRst.empty(), CG_CAMERA_SEARCH_RTSP_ERROR);
    JsonHelper::GenerateStringValue("rtsp", strRtsp, strRst);
    return CG_OK;
}

int CameraManagement::add_camera(const CameraInfo& infoCamera)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);
        conn << "select id from ft_camera where visible = 1"
            << " and name=:name"
            , soci::use(infoCamera.strName);
        if (conn.got_data())
            return CG_CAMERA_ALREADY_EXIST;
        conn << "insert into ft_camera(name,rtsp,ip,username,password,type,remark)"
            << " values(:name,:rtsp,:ip,:username,:password,:type,:remark)"
            , soci::use(infoCamera.strName)
            , soci::use(infoCamera.strRtsp)
            , soci::use(infoCamera.strIp)
            , soci::use(infoCamera.strUser)
            , soci::use(infoCamera.strPassword)
            , soci::use(infoCamera.nType)
            , soci::use(infoCamera.strRemark);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Add camera exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int CameraManagement::delete_camera(int nCameraId)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);
        std::string strIp;
        conn << "select ip from ft_camera where visible = 1"
            << " and id=:id"
            , soci::into(strIp)
            , soci::use(nCameraId);
        if (conn.got_data())
        {
            Cache::CameraStateManagement::Instance().DeleteCamera(strIp);
            conn << "update ft_camera set visible = 0 where visible = 1"
                << " and id=:id"
                , soci::use(nCameraId);
        }
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Delete camera:" << nCameraId << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int CameraManagement::update_camera(const CameraInfo& infoCamera, const std::string& strUpdateSql)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);
        if (!infoCamera.strRtsp.empty())
        {
            std::string strIp;
            conn << "select ip from ft_camera where visible = 1"
                << " and id=:id"
                , soci::into(strIp)
                , soci::use(infoCamera.nCameraId);
            if (conn.got_data())
                Cache::CameraStateManagement::Instance().DeleteCamera(strIp);
            Cache::CameraStateManagement::Instance().DeleteCamera(infoCamera.strIp);
        }
        if (!infoCamera.strName.empty())
        {
            conn << "select id from ft_camera where visible = 1"
                << " and name=:name and id <>:id"
                , soci::use(infoCamera.strName)
                , soci::use(infoCamera.nCameraId);
            if (conn.got_data())
                return CG_CAMERA_ALREADY_EXIST;
        }
        conn << strUpdateSql
            << " where id=:id"
            , soci::use(infoCamera.nCameraId);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Update camera:" << infoCamera.nCameraId << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int CameraManagement::parse_camera_rtsp(const std::string& strRtsp, CameraInfo& info)
{
    std::regex rg("^rt(?:s|m)p://(?:(.+):(.+)@)?([\\.\\w]+)([:\\.\\w]+)?(.*)$");
    std::smatch sm;
    std::regex_search(strRtsp, sm, rg);
    if (sm.size() < 6) return CG_CAMERA_INVALID_RTSP;
    int nIndex = 0;
    info.strUser = sm[++nIndex];
    info.strPassword = sm[++nIndex];
    info.strIp = sm[++nIndex];
    return CG_OK;
}

int CameraManagement::query_camera_list(const CameraQueryCond& condQuery, 
    CameraList& listCamera, int& nCnt)
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
        conn << "select count(*) from ft_camera"
            << strSqlCond
            , soci::into(nCnt);
        if (0 == nCnt) return CG_OK;
        std::map<std::string, int> stateCamera;
        Cache::CameraStateManagement::Instance().GetAllCamera(stateCamera);
        CameraInfo infoCamera;
        soci::statement st = (conn.prepare
            << "select * from ft_camera"
            << strSqlCond
            << " limit " << (condQuery.nPage - 1) * condQuery.nPageSize
            << " ," << condQuery.nPageSize
            , soci::into(infoCamera));
        st.execute();
        while (st.fetch())
        {
            auto itMatch = stateCamera.find(infoCamera.strIp);
            if (itMatch != stateCamera.end())
                infoCamera.nOnline = itMatch->second;
            listCamera.push_back(infoCamera);
        }
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Query camera exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int CameraManagement::search_camera(SearchCameraList& lisrLibrary, int& nCnt)
{
    std::map<std::string, CameraSearch::DeviceInfo> mapCamera;
    Cache::DiscoveryCamera::Instance().GetSearchCameras(mapCamera);
    if (mapCamera.empty())return CG_OK;
    for (const auto& item : mapCamera) {
        Service::SearchCamera camera;
        camera.strIp = item.first;
        camera.strManufacturer = item.second.hardware;
        lisrLibrary.emplace_back(camera);
    }
    return CG_OK;
}