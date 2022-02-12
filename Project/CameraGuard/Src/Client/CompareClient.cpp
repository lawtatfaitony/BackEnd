#include "CompareClient.h"
#include "CompareClientImpl.h"
#include <easylogging/easylogging++.h>
#include <Macro.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Config/GlobalConfig.h"
#include "../Cache/ConnectionPool/DatabaseConnPool.h"



using namespace Client;
CompareClient::CompareClient()
    : m_LibId(1)
{

}

bool CompareClient::Start()
{
    VERIFY_RETURN_OPTION(load_library_info(), false);
#ifdef ICE_CPP11_MAPPING
    auto servant(std::make_shared<ClientImpl>());
    auto funcAddClient = [](const std::shared_ptr<MyIceDemo::PrinterServerPrx>& pPrx, const std::string& strId)
#else
    auto servant(new CompareClientImpl);
    auto funcAddClient = [](const Compare::CompareServerPrx& pPrx, const std::string& strId)
#endif // ICE_CPP11_MAPPING
    {
        return pPrx->RegisterClient(strId);
    };

    return m_pClient.Init(servant, funcAddClient, Config::GetCompareClientConfig(), "Compare.Proxy");

}

void CompareClient::Stop()
{
    m_pClient.Cleanup();
}

#ifdef ICE_CPP11_MAPPING
std::shared_ptr<Compare::CompareServer> ClientManager::GetProxy()
#else
Compare::CompareServerPrx  CompareClient::GetProxy()
#endif // ICE_CPP11_MAPPING
{
    return m_pClient.GetProxy();
}

bool CompareClient::IsConnected()
{
    return m_pClient.CheckConnStatus();
}

std::string CompareClient::GetIdentityName()
{
    return m_pClient.GetIdentity().name;
}

int CompareClient::AddLibrary(int& lib_id)
{
    std::string strResult;
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_COMPARE_INSTANCE;
    lib_id = ++m_LibId;
    int ret = pProxy->AddLibrary(lib_id, strResult);
    CATCH_EXCEPTION_END(AddLibrary);
    // parse the result
    rapidjson::Document docRst;
    JS_PARSE_OBJECT_RETURN(docRst, strResult, CG_ADD_LIBRARY_FAILED);
    int code = CG_OK;
    JS_PARSE_OPTION_RETURN(code, docRst, Int, code, CG_ADD_LIBRARY_FAILED);
    if (CG_OK != code)
    {
        std::string strMsg;
        JS_PARSE_OPTION_RETURN(strMsg, docRst, String, msg, CG_INVALID_PARA);
        LOG(ERROR) << "Add library failed,error:" << strMsg;
        --m_LibId;
        return CG_ADD_LIBRARY_FAILED;
    }
    return CG_OK;
}

int CompareClient::DeleteLibrary(int nLibId)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_COMPARE_INSTANCE;
    return pProxy->DeleteLibrary(nLibId);
    CATCH_EXCEPTION_END(DeleteLibrary);
    return CG_INVAOKE_TIMEOUT;
}

int CompareClient::ListLibrary(std::string& strResult)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_COMPARE_INSTANCE;
    return pProxy->ListLibrary(strResult);
    CATCH_EXCEPTION_END(ListLibrary);
    return CG_INVAOKE_TIMEOUT;
}

int CompareClient::AddPerson(int nLibId, 
    const std::string& strPicUrl,
    std::string& strResult)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_COMPARE_INSTANCE;
    return pProxy->AddPerson(nLibId, strPicUrl, strResult);
    CATCH_EXCEPTION_END(AddPerson);
    return CG_INVAOKE_TIMEOUT;
}

int CompareClient::DeletePerson(int nLibId, int64_t nPersonId)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_COMPARE_INSTANCE;
    return pProxy->DeletePerson(nLibId, nPersonId);
    CATCH_EXCEPTION_END(DeletePerson);
    return CG_INVAOKE_TIMEOUT;
}

int CompareClient::UpdatePerson(int nLibId,
    int64_t nPersonId,
    const std::string& strPicUrl,
    std::string& strResult)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_COMPARE_INSTANCE;
    return pProxy->UpdatePersonPicture(nLibId, nPersonId, strPicUrl, strResult);
    CATCH_EXCEPTION_END(UpdatePerson);
    return CG_INVAOKE_TIMEOUT;
}

int CompareClient::GetFeatureSize()
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_COMPARE_INSTANCE;
    return pProxy->GetFeatureSize();
    CATCH_EXCEPTION_END(GetFeatureSize);
    return CG_INVAOKE_TIMEOUT;
}

int CompareClient::ExtractFeature(const std::string& strPicUrl, std::string& strResult)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_COMPARE_INSTANCE;
    return pProxy->ExtractFeature(strPicUrl, strResult);
    CATCH_EXCEPTION_END(ExtractFeature);
    return CG_INVAOKE_TIMEOUT;
}

int CompareClient::Compare1v1(const std::string& strLPicUrl,
    const std::string& strRPicUrl,
    float& nSimilarity)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_COMPARE_INSTANCE;
    auto nCode = pProxy->Compare1v1(strLPicUrl, strRPicUrl, nSimilarity);
    if (nCode != CG_OK)
    {
        LOG(ERROR) << "Compare 1:1 failed: " << nCode;
        return CG_COMPARE_1V1_FAILED;
    }
    return CG_OK;
    CATCH_EXCEPTION_END(Compare1v1);
    return CG_INVAOKE_TIMEOUT;
}

int CompareClient::CompareWithFeature(const std::string& strLFeature,
    const std::string& strRFeature,
    float& nSimilarity)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_COMPARE_INSTANCE;
    VERIFY_CODE_RETURN(pProxy->CompareWithFeature(strLFeature, strRFeature, nSimilarity));

    CATCH_EXCEPTION_END(CompareWithFeature);
    return CG_INVAOKE_TIMEOUT;
}

int CompareClient::load_library_info()
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        int nLibId = 1;
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);
        conn << "select IFNULL(max(id),0) from ft_library where visible = 1"
            , soci::into(nLibId);
        m_LibId = nLibId;
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Load library info exception:" << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}