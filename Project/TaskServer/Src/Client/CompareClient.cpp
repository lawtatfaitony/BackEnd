#include "CompareClient.h"
#include "CompareClientImpl.h"
#include <easylogging/easylogging++.h>
#include <Macro.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Config/GlobalConfig.h"



using namespace Client;
CompareClient::CompareClient()
{

}

void CompareClient::Start()
{
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

    m_pClient.Init(servant, funcAddClient, Config::GetCompareClientConfig(), "Compare.Proxy");

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

int CompareClient::PushCompareTask(const Compare::CompareTask& task)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CTS_COMPARE_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CTS_INVALID_COMPARE_INSTANCE;
    return pProxy->PushCompareTask(task);
    CATCH_EXCEPTION_END(PushCompareTask);
    return CTS_OK;
}
