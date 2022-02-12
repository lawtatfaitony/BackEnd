#include "TaskClient.h"
#include "TaskClientImpl.h"
#include <easylogging/easylogging++.h>
#include <Macro.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Config/GlobalConfig.h"



using namespace Client;
TaskClient::TaskClient()
{

}

void TaskClient::Start()
{
#ifdef ICE_CPP11_MAPPING
    auto servant(std::make_shared<ClientImpl>());
    auto funcAddClient = [](const std::shared_ptr<MyIceDemo::PrinterServerPrx>& pPrx, const std::string& strId)
#else
    auto servant(new TaskClientImpl);
    auto funcAddClient = [](const Task::TaskServerPrx& pPrx, const std::string& strId)
#endif // ICE_CPP11_MAPPING
    {
        return pPrx->RegisterClient(strId);
    };

    m_pClient.Init(servant, funcAddClient, Config::GetTaskClientConfig(), "Task.Proxy");

}

void TaskClient::Stop()
{
    m_pClient.Cleanup();
}

#ifdef ICE_CPP11_MAPPING
std::shared_ptr<Compare::CompareServer> ClientManager::GetProxy()
#else
Task::TaskServerPrx  TaskClient::GetProxy()
#endif // ICE_CPP11_MAPPING
{
    return m_pClient.GetProxy();
}

bool TaskClient::IsConnected()
{
    return m_pClient.CheckConnStatus();
}

std::string TaskClient::GetIdentityName()
{
    return m_pClient.GetIdentity().name;
}

int TaskClient::StartTask(const Task::TaskInfo& infoTask)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_TASK_SERVER_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_TASK_SERVER_INSTANCE;
    return pProxy->StartTask(infoTask);
    CATCH_EXCEPTION_END(StartTask);
    return CG_OK;
}

int TaskClient::StopTask(int nTaskId)
{
    CATCH_EXCEPTION_BEGIN;
    if (!IsConnected())
        return CG_TASK_SERVER_NOT_CONNECT;
    auto pProxy = GetProxy();
    if (!pProxy)
        CG_INVALID_TASK_SERVER_INSTANCE;
    return pProxy->StopTask(GetIdentityName(), nTaskId);
    CATCH_EXCEPTION_END(StopTask);
    return CG_OK;
}