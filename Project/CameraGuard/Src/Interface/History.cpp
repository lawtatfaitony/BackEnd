#include "History.h"
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
    struct type_conversion<Service::TaskCaptureRecord>
    {
        typedef values base_type;
        static void from_base(const values& v, indicator ind, Service::TaskCaptureRecord& record)
        {
            record.nRecordId = v.get<int64_t>("id");
            record.nTaskId = v.get<int>("task_id");
            record.strTaskName = v.get<std::string>("task_name");
            record.nCameraId = v.get<int>("camera_id");
            record.strCameraName = v.get<std::string>("camera_name");
            record.nLibId = v.get<int>("lib_id");
            record.strLibName = v.get<std::string>("lib_name");
            record.nPersonId = v.get<int64_t>("person_id");
            record.strPersonName = v.get<std::string>("person_name");
            record.nSex = v.get<int>("category");
            record.strCardNo = v.get<std::string>("card_no");
            record.nCategory = v.get<int>("sex");
            record.nClassify = v.get<int>("classify");
            record.nSimilarity = v.get<double>("similarity");
            record.strPicPath = v.get<std::string>("pic_path");
            if (!record.strPicPath.empty()) {
                record.strPicPath.insert(0, CFG_STORAGE_SERVER.strUrlPrefix);
            }
            record.strCapturePath = v.get<std::string>("capture_path");
            if (!record.strCapturePath.empty()) {
                record.strCapturePath.insert(0, CFG_STORAGE_SERVER.strUrlPrefix);
            }
            record.strCaptureTime = SociHelper::FetchTime(v, "capture_time");
            record.strCreateTime = SociHelper::FetchTime(v, "create_time");
        }
    };
}

namespace JsonHelper
{
    static void to_json(const Service::TaskCaptureRecord& infoRecord,
        rapidjson::Value& value,
        rapidjson::Document::AllocatorType &allocator)
    {
        value.AddMember("record_id", infoRecord.nRecordId, allocator);
        value.AddMember("task_id", infoRecord.nTaskId, allocator);
        value.AddMember("task_name", rapidjson::Value(infoRecord.strTaskName.c_str(), allocator), allocator);
        value.AddMember("camera_id", infoRecord.nCameraId, allocator);
        value.AddMember("camera_name", rapidjson::Value(infoRecord.strCameraName.c_str(), allocator), allocator);
        value.AddMember("lib_id", infoRecord.nLibId, allocator);
        value.AddMember("lib_name", rapidjson::Value(infoRecord.strLibName.c_str(), allocator), allocator);
        value.AddMember("person_id", infoRecord.nPersonId, allocator);
        value.AddMember("person_name", rapidjson::Value(infoRecord.strPersonName.c_str(), allocator), allocator);
        value.AddMember("sex", infoRecord.nSex, allocator);
        value.AddMember("category", infoRecord.nCategory, allocator);
        value.AddMember("card_no", rapidjson::Value(infoRecord.strCardNo.c_str(), allocator), allocator);
        value.AddMember("classify", infoRecord.nClassify, allocator);
        value.AddMember("similarity", infoRecord.nSimilarity, allocator);
        value.AddMember("pic_url", rapidjson::Value(infoRecord.strPicPath.c_str(), allocator), allocator);
        value.AddMember("capture_url", rapidjson::Value(infoRecord.strCapturePath.c_str(), allocator), allocator);
        value.AddMember("capture_time", rapidjson::Value(infoRecord.strCaptureTime.c_str(), allocator), allocator);
        value.AddMember("create_time", rapidjson::Value(infoRecord.strCreateTime.c_str(), allocator), allocator);
    }
}


using namespace Service;
int HistoryManagement::QueryTaskCaptureRecord(const std::string& strCond, std::string& strRst)
{
    TaskCaptueryQueryCond condQuery;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strCond, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(condQuery.strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(condQuery.nPage, doc, Int, page_no, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(condQuery.nPageSize, doc, Int, page_size, strRst, CG_INVALID_PARA);
    // option
    JS_PARSE_OPTION(condQuery.nTaskId, doc, Int, task_id);
    JS_PARSE_OPTION(condQuery.nCameraId, doc, Int, camera_id);
    JS_PARSE_OPTION(condQuery.nLibId, doc, Int, lib_id);
    JS_PARSE_OPTION(condQuery.strPersonName, doc, String, person_name);
    JS_PARSE_OPTION(condQuery.strCardNo, doc, String, card_no);
    JS_PARSE_OPTION(condQuery.nClassify, doc, Int, classify);
    JS_PARSE_NUMBER_OPTION(condQuery.nSimilarity, doc, Float, similarity);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(condQuery.strSession), CG_INVALID_SESSION);
    int64_t nTotalCount = 0;
    TaskCaptureList listRecord;
    VERIFY_CODE_RETURN(query_task_capture_record(condQuery, listRecord, nTotalCount));
    JsonHelper::GenerateList(condQuery.nPage, condQuery.nPageSize, nTotalCount, listRecord, strRst);
    return CG_OK;
}

int HistoryManagement::DeleteTaskCaptureRecord(const std::string& strCond, std::string& strRst)
{
    int64_t nRecordId = 0;
    std::string strSession;
    rapidjson::Document doc;
    JS_PARSE_OBJECT_RETURN(doc, strCond, CG_INVALID_JSON);
    JS_PARSE_REQUIRED_RETURN(strSession, doc, String, session, strRst, CG_INVALID_PARA);
    JS_PARSE_REQUIRED_RETURN(nRecordId, doc, Int, record_id, strRst, CG_INVALID_PARA);

    VERIFY_EXPR_RETURN(SessionMangement::Instance().IsValidSession(strSession), CG_INVALID_SESSION);
    VERIFY_CODE_RETURN(delete_task_capture_record(nRecordId));
    return CG_OK;
}

int HistoryManagement::query_task_capture_record(const TaskCaptueryQueryCond& condQuery,
    TaskCaptureList& listRecord,
    int64_t& nCnt)
{
    std::string strSqlCond;
    {
        std::stringstream ss;
        ss << " where visible = 1";
        if (!condQuery.strPersonName.empty())
            ss << " and person_name like '%" << condQuery.strPersonName << "%'";
        if (!condQuery.strCardNo.empty())
            ss << " and card_no = '" << condQuery.strCardNo << "'";
        if (condQuery.nTaskId > 0)
            ss << " and task_id = " << condQuery.nTaskId;
        if (condQuery.nCameraId > 0)
            ss << " and camera_id = " << condQuery.nCameraId;
        if (condQuery.nLibId > 0)
            ss << " and lib_id = " << condQuery.nLibId;
        if (condQuery.nClassify >= 0)
            ss << " and classify = " << condQuery.nClassify;
        if (condQuery.nSimilarity > 0)
            ss << " and similarity >= " << condQuery.nSimilarity;
        strSqlCond = ss.str();
    }
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strHistoryBase);
        conn << "select count(*) from hist_recognize_record"
            << strSqlCond
            , soci::into(nCnt);
        if (0 == nCnt) return CG_OK;
        TaskCaptureRecord infoRecord;
        soci::statement st = (conn.prepare
            << "select * from hist_recognize_record"
            << strSqlCond
            << " limit " << (condQuery.nPage - 1) * condQuery.nPageSize
            << " ," << condQuery.nPageSize
            , soci::into(infoRecord));
        st.execute();
        while (st.fetch())
            listRecord.push_back(infoRecord);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Query task capture record exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}

int HistoryManagement::delete_task_capture_record(int64_t nRecordId)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strHistoryBase);
        conn << "update hist_recognize_record set visible=0 where visible=1"
            << " and id=:id;"
            , soci::use(nRecordId);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Delete task capture record exception: " << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}