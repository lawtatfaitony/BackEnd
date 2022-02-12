#include "ErrorMessage.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <Macro.h>
#include "ErrorCode.h"


// global error message map
std::map<int, std::string> g_mapErrorInfo
{
    // common error code
    { CG_OK , "" },
    { CG_UNKNOW_ERROR , "unknow error" },
    { CG_INVALID_JSON , "parse input json error" },
    { CG_INVALID_PARA , "invalid para" },
    { CG_EXCUTE_SQL_ERROR , "excute sql exception" },
    { CG_INVALID_SESSION , "invalid session" },
    { CG_NOT_EXIST , "data not exist" },
    { CG_INVAOKE_TIMEOUT , "invoke with timeout" },
    { CG_INVALID_DATABASE_CONN , "invalid database conntion instance" },

    { CG_COMPARE_NOT_CONNECT , "can't connect compare server" },
    { CG_INVALID_COMPARE_INSTANCE , "invalid compare server instance" },
    { CG_TASK_SERVER_NOT_CONNECT , "can't connect task server" },
    { CG_INVALID_TASK_SERVER_INSTANCE , "invalid task server instance" },

    // user
    { CG_INVALID_USER , "unknow user" },
    { CG_USER_ALREADY_EXIST , "name of user already exists" },
    { CG_PASSWORD_ERROR , "error of password" },

    // library
    { CG_LIBRARY_ALREADY_EXIST , "name of library exists" },
    { CG_LIBRARY_NOT_EXIST , "library not exists" },
    { CG_ADD_LIBRARY_COMPARE_FAILED , "can't connect compare server" },
    { CG_ADD_LIBRARY_FAILED , "add library failed"},

    // camera
    { CG_CAMERA_INVALID_RTSP , "invalid camera's rtsp" },
    { CG_CAMERA_ALREADY_EXIST , "name of camera already exists" },
    { CG_CAMERA_SEARCH_RTSP_ERROR , "search camera's rtsp error" },

    // task
    { CG_TASK_ALREADY_EXIST , "name of task exists" },
    { CG_TASK_SERVER_FAILED , "can't connect task server" },
    { CG_COMPARE_1V1_FAILED,"1:1 compare failed" },

    // person
    { CG_ADD_PERSON_FAILED, "add person failed" },
    { CG_PERSON_ALREADY_EXIST, "identity of person already exists" },
    { CG_EXTRACT_FAILED, "failed to extract feature" },
    { CG_INVALID_PERSON_PHOTO, "invalid person's photo" },
    { CG_PERSON_CARD_NO_EXIST, "person's card-no exist"},

    // business
    { CG_TASK_IS_RUNNING,"task is running"},


};


using namespace Service;
ErrorMsgManagement::ErrorMsgManagement()
{
    init();
}

ErrorMsgManagement& ErrorMsgManagement::Instance()
{
    static ErrorMsgManagement g_Instance;
    return g_Instance;
}

ErrorMsgManagement::~ErrorMsgManagement()
{

}

std::string ErrorMsgManagement::GetErrorMsg(int nError, const std::string& strErrorInfo)
{
    if (strErrorInfo.empty())
    {
        SCOPE_LOCK(m_mtLock);
        auto itMatch = m_mapErrorMsg.find(nError);
        if (itMatch == m_mapErrorMsg.end())
            return std::string();
        return itMatch->second;
    }
    return generate_error_msg(nError, strErrorInfo);
}

void ErrorMsgManagement::init()
{
    for (const auto& item : g_mapErrorInfo)
        m_mapErrorMsg[item.first] = generate_error_msg(item.first, item.second);
}

std::string ErrorMsgManagement::generate_error_msg(int nError, const std::string& strErrorIn)
{
    std::string strErrorMsg;
    {
        rapidjson::Document doc(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();
        doc.AddMember("code", nError, typeAllocate);
        doc.AddMember("msg", rapidjson::Value(strErrorIn.c_str(), typeAllocate), typeAllocate);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
        doc.Accept(writer);
        strErrorMsg = buffer.GetString();
    }
    return strErrorMsg;
}