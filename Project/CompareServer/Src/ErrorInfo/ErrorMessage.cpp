#include "ErrorMessage.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "ErrorCode.h"


// global error message map
std::map<int, std::string> g_mapErrorInfo
{
    { CS_OK , "" },
    { CS_INVALID_INSTANCE , "invalid face sdk instance" },
    { CS_DOWNLOAD_PICTURE_FAILED , "download picture failed" },
    { CS_DETECT_NO_FACE , "detect no face" },
    { CS_DETECT_MULTI_FACE , "detect multi-face" },
    { CS_EXTRACT_FEATURE_FAILED , "extract feature failed" },

    // user
    { CS_COMPARE_WITH_FEATURE_FAILED , "compare with feature failed" },
    { CS_LIBRARY_ALREADY_EXIST , "name of library already exists" },
    { CS_LIBRARY_NOT_EXIST , "library not exist" }

};


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
        std::lock_guard<std::mutex> lock(m_mtLock);
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