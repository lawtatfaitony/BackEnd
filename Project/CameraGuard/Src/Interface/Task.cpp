#include "task.h"
#include <soci/soci.h>
#include <easylogging/easylogging++.h>
#include <JsonHelper.h>
#include <Macro.h>
#include <SociHelp.h>
#include <Basic/Function.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Config/GlobalConfig.h"
#include "../Cache/session/Session.h"
#include "../Cache/ConnectionPool/DatabaseConnPool.h"
#include "../Task/TaskManager.h"
#include "../Client/CompareClient.h"


namespace soci
{
    template<>
    struct type_conversion<Service::TaskInfo>
    {
        typedef values base_type;
        static void from_base(const values& v, indicator ind, Service::TaskInfo& task)
        {
            task.nTaskId = v.get<int>("id");
            task.strName = v.get<std::string>("name");
            task.nType = v.get<int>("type");
            task.strCameraList1 = v.get<std::string>("camera_list1");
            task.strCameraList2 = v.get<std::string>("camera_list2");
            task.strLibList = v.get<std::string>("lib_list");
            task.nInterval = v.get<int>("interval");
            task.fThreshold = v.get<double>("threshold");
            task.nState = v.get<int>("state");
            task.strPlan = v.get<std::string>("plan");
            task.strRemark = v.get<std::string>("remark");
            task.strCreateTime = SociHelper::FetchTime(v, "create_time");
        }
    };
}

namespace JsonHelper
{
    static void to_json(const Service::CameraStateInfo& infoCamera,
        rapidjson::Value& value,
        rapidjson::Document::AllocatorType &allocator)
    {
        value.AddMember("camera_id", infoCamera.nCameraId, allocator);
        value.AddMember("name", rapidjson::Value(infoCamera.strName.c_str(), allocator), allocator);
        value.AddMember("state", infoCamera.nState, allocator);
    }

    static void to_json(const Service::TaskLibraryInfo& infoLibrary,
        rapidjson::Value& value,
        rapidjson::Document::AllocatorType &allocator)
    {
        value.AddMember("lib_id", infoLibrary.nLibId, allocator);
        value.AddMember("name", rapidjson::Value(infoLibrary.strName.c_str(), allocator), allocator);
    }

    static void to_json(const Service::TaskInfo& infoTask,
        rapidjson::Value& value,
        rapidjson::Document::AllocatorType &allocator)
    {
        value.AddMember("task_id", infoTask.nTaskId, allocator);
        value.AddMember("name", rapidjson::Value(infoTask.strName.c_str(), allocator), allocator);
        value.AddMember("type", infoTask.nType, allocator);
        // in
        {
            rapidjson::Value valueArr(rapidjson::kArrayType);
            for (const auto& item : infoTask.listCamera1)
            {
                rapidjson::Value valueData(rapidjson::kObjectType);
                to_json(item, valueData, allocator);
                valueArr.PushBack(valueData, allocator);
            }
            value.AddMember("camera_list1", valueArr, allocator);
        }
        // out
        {
            rapidjson::Value valueArr(rapidjson::kArrayType);
            for (const auto& item : infoTask.listCamera2)
            {
                rapidjson::Value valueData(rapidjson::kObjectType);
                to_json(item, valueData, allocator);
                valueArr.PushBack(valueData, allocator);
            }
            value.AddMember("camera_list2", valueArr, allocator);
        }
        // library info
        {
            rapidjson::Value valueArr(rapidjson::kArrayType);
            for (const auto& item : infoTask.listLib)
            {
                rapidjson::Value valueData(rapidjson::kObjectType);
                to_json(item, valueData, allocator);
                valueArr.PushBack(valueData, allocator);
            }
            value.AddMember("lib_list", valueArr, allocator);
        }
        value.AddMember("interval", infoTask.nInterval, allocator);
        value.AddMember("threshold", infoTask.fThreshold, allocator);
        value.AddMember("state", infoTask.nState, allocator);
        value.AddMember("plan", rapidjson::Value(infoTask.strPlan.c_str(), allocator), allocator);
        value.AddMember("remark", rapidjson::Value(infoTask.strRemark.c_str(), allocator), allocator);
        value.AddMember("create_time", rapidjson::Value(infoTask.strCreateTime.c_str(), allocator), allocator);
    }
}

using namespace Service;
int TaskManagement::AddTask(const std::string& strMsg, std::string& strRst)
{
    TaskInfo infoTask;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoTask.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoTask.strName, doc, String, name, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoTask.nType, doc, Int, type, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoTask.strCameraList1, doc, String, camera_list1, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoTask.strCameraList2, doc, String, camera_list2, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoTask.strLibList, doc, String, lib_list, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoTask.nInterval, doc, Int, interval, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoTask.fThreshold, doc, Double, threshold, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(infoTask.strPlan, doc, String, plan);
    JS_PARSE_OPTION(infoTask.strRemark, doc, String, remark);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(infoTask.strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(add_task(infoTask));
    return CG_OK;
}

int TaskManagement::DeleteTask(const std::string& strMsg, std::string& strRst)
{
    std::string strSession;
    int nTaskId = 0;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(nTaskId, doc, Int, task_id, strRst, CG_INVALID_PARA);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(check_task_running(nTaskId));
    VERIFY_CODE_RETURN(delete_task(nTaskId));
    return CG_OK;
}

int TaskManagement::UpdateTask(const std::string& strMsg, std::string& strRst)
{
    TaskInfo infoTask;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(infoTask.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(infoTask.nTaskId, doc, Int, task_id, strRst, CG_INVALID_PARA);

    std::stringstream ss;
    ss << "update ft_task set visible = 1";
    JS_PARSE_OPTION_UPDATE_STRING(infoTask.strName, doc, String, name, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoTask.strCameraList1, doc, String, camera_list1, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoTask.strCameraList2, doc, String, camera_list2, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoTask.strLibList, doc, String, lib_list, ss);
    JS_PARSE_OPTION_UPDATE_INT(infoTask.nType, doc, Int, type, ss);
    JS_PARSE_OPTION_UPDATE_INT(infoTask.nInterval, doc, Int, interval, ss);
    JS_PARSE_OPTION_UPDATE_INT(infoTask.fThreshold, doc, Double, threshold, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoTask.strPlan, doc, String, plan, ss);
    JS_PARSE_OPTION_UPDATE_STRING(infoTask.strRemark, doc, String, remark, ss);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(infoTask.strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(check_task_running(infoTask.nTaskId));
    VERIFY_CODE_RETURN(update_task(infoTask, ss.str()));
    return CG_OK;
}

int TaskManagement::QueryTaskList(const std::string& strMsg, std::string& strRst)
{
    TaskQueryCond condQuery;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(condQuery.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(condQuery.nPage, doc, Int, page_no, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(condQuery.nPageSize, doc, Int, page_size, strRst, CG_INVALID_PARA);
    JS_PARSE_OPTION(condQuery.strName, doc, String, username);
    JS_PARSE_OPTION(condQuery.nType, doc, Int, type);
    JS_PARSE_OPTION(condQuery.nState, doc, Int, state);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(condQuery.strSession), CG_INVALID_SESSION);
    int64_t nTotalCnt = 0;
    TaskList listTask;
    VERIFY_CODE_RETURN(query_task_list(condQuery, listTask, nTotalCnt));
    fill_camera_state(listTask);
    JsonHelper::GenerateList(condQuery.nPage, condQuery.nPageSize, nTotalCnt, listTask, strRst);
    return CG_OK;
}

int TaskManagement::StartTask(const std::string& strMsg, std::string& strRst)
{
    std::string strSession;
    int nTaskId = 0;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(nTaskId, doc, Int, task_id, strRst, CG_INVALID_PARA);
    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(TaskManager::StartTask(nTaskId));
    return CG_OK;
}

int TaskManagement::StopTask(const std::string& strMsg, std::string& strRst)
{
    std::string strSession;
    int nTaskId = 0;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(nTaskId, doc, Int, task_id, strRst, CG_INVALID_PARA);
    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(TaskManager::StopTask(nTaskId));
    return CG_OK;
}

int TaskManagement::Compare1V1(const std::string& strMsg, std::string& strRst)
{
    std::string strSession;
    std::string strPic1, strPic2;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strMsg, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(strPic1, doc, String, first_pic, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(strPic2, doc, String, second_pic, strRst, CG_INVALID_PARA);
    if (strPic1.empty() || strPic2.empty())
    {
        strRst = "Invalid path of person's photo";
        return CG_INVALID_PARA;
    }
    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    float nSimilarity = 0.0;
    VERIFY_CODE_RETURN(Client::CompareClient::Instance().Compare1v1(strPic1, strPic2, nSimilarity));
    JsonHelper::GenerateValue("similarity", nSimilarity, strRst);
    return CG_OK;
}

int TaskManagement::add_task(const TaskInfo& infoTask)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "select id from ft_task where visible = 1"
            << " and name=:name"
            , soci::use(infoTask.strName);
        if (conn.got_data())
            return CG_TASK_ALREADY_EXIST;
        conn << "insert into ft_task(name,type,camera_list1,camera_list2,lib_list"
            ",`interval`,threshold,plan,remark)"
            << " values(:name,:type,:camera_list1,:camera_list2,:lib_list"
            ",:interval,:threshold,:plan,:remark)"
            , soci::use(infoTask.strName)
            , soci::use(infoTask.nType)
            , soci::use(infoTask.strCameraList1)
            , soci::use(infoTask.strCameraList2)
            , soci::use(infoTask.strLibList)
            , soci::use(infoTask.nInterval)
            , soci::use(static_cast<double>(infoTask.fThreshold))
            , soci::use(infoTask.strPlan)
            , soci::use(infoTask.strRemark);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Add task exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int TaskManagement::delete_task(int nTaskId)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "update ft_task set visible = 0 where visible = 1"
            << " and id=:id"
            , soci::use(nTaskId);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Delete task:" << nTaskId << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int TaskManagement::update_task(const TaskInfo& infoTask, const std::string& strUpdateSql)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        if (!infoTask.strName.empty())
        {
            conn << "select id from ft_task where visible = 1"
                << " and name=:name and id<>:id"
                , soci::use(infoTask.strName)
                , soci::use(infoTask.nTaskId);
            if (conn.got_data())
                return CG_TASK_ALREADY_EXIST;
        }
        conn << strUpdateSql
            << " where id=:id"
            , soci::use(infoTask.nTaskId);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Update task:" << infoTask.nTaskId << " exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int TaskManagement::query_task_list(const TaskQueryCond& condQuery,
    TaskList& listTask,
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
        if (condQuery.nState > 0)
            ss << " and state = " << condQuery.nState;
        strSqlCond = ss.str();
    }
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "select count(*) from ft_task"
            << strSqlCond
            , soci::into(nCnt);
        if (0 == nCnt) return CG_OK;
        TaskInfo infoTask;
        soci::statement st = (conn.prepare
            << "select * from ft_task"
            << strSqlCond
            << " limit " << (condQuery.nPage - 1) * condQuery.nPageSize
            << " ," << condQuery.nPageSize
            , soci::into(infoTask));
        st.execute();
        while (st.fetch())
        {
            // in
            if (infoTask.strCameraList1.empty())
            {
                CameraStateInfo infoCamera;
                soci::statement stCam = (conn.prepare
                    << "select id,name from ft_camera where id in("
                    << infoTask.strCameraList1 << ")"
                    , soci::into(infoCamera.nCameraId)
                    , soci::into(infoCamera.strName));
                stCam.execute();
                while (stCam.fetch())
                    infoTask.listCamera1.push_back(infoCamera);
            }
            // out
            if (infoTask.strCameraList2.empty())
            {
                CameraStateInfo infoCamera;
                soci::statement stCam = (conn.prepare
                    << "select id,name from ft_camera where id in("
                    << infoTask.strCameraList2 << ")"
                    , soci::into(infoCamera.nCameraId)
                    , soci::into(infoCamera.strName));
                stCam.execute();
                while (stCam.fetch())
                    infoTask.listCamera2.push_back(infoCamera);
            }
            if (!infoTask.strLibList.empty())
            {
                TaskLibraryInfo infoLibrary;
                soci::statement stLib = (conn.prepare
                    << "select lib_id,name from ft_library where lib_id in("
                    << infoTask.strLibList << ")"
                    , soci::into(infoLibrary.nLibId)
                    , soci::into(infoLibrary.strName));
                stLib.execute();
                while (stLib.fetch())
                    infoTask.listLib.push_back(infoLibrary);
            }
            listTask.push_back(infoTask);
        }
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Query task list exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

void TaskManagement::fill_camera_state(TaskList& listTask)
{
    if (listTask.empty()) return;
    // TODO: fill the camera's state
}


int TaskManagement::check_task_running(int nTaskId)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "select id from ft_task where visible=1"
            << " and id:id and `state` =1"
            , soci::use(nTaskId);
        if (conn.got_data()) return CG_TASK_IS_RUNNING;
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Check task's state exception:" << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}