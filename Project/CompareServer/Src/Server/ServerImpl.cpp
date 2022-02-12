#include "ServerImpl.h"
#include <easylogging/easylogging++.h>
#include <Macro.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Library/LibraryManagement.h"



void convert_task_info(const Compare::CompareTask& task, TaskInfo& infoTask)
{
    infoTask.strIdetityId = task.strIdentity;
    infoTask.nTaskId = task.nTaskId;
    infoTask.nCameraId = task.nCameraId;
    infoTask.nThreshold = task.nThreshold;
    infoTask.nTop = task.nTop;
    infoTask.strLibList = task.strLibList;
    infoTask.strCaptureTime = task.strCaptureTime;
    infoTask.strPicData = task.strPicData;
    infoTask.strPicPath = task.strPicPath;
};


CompareServerImpl* CompareServerImpl::m_pThis = nullptr;
CompareServerImpl::CompareServerImpl()
{
    m_pThis = this;
}


CompareServerImpl::~CompareServerImpl()
{
    m_mapConns.clear();
}

void CompareServerImpl::RegisterClient(const std::string& strId, const Ice::Current & current)
{
    std::shared_ptr<CompareClient>pClient(new CompareClient);
    if (pClient->Init(strId, current))
    {
        std::lock_guard<std::mutex>lock(m_mtLock);
        m_mapConns[strId] = pClient;
    }
    LOG(INFO) << "New connection, id:" << strId;
    hold_heartbeat(strId, current.con);
}

int CompareServerImpl::AddLibrary(int nLibId,
    std::string& strResult, 
    const Ice::Current & current)
{
    return LibraryManagement::Instance().AddLibrary(nLibId, strResult);
}

int CompareServerImpl::DeleteLibrary(int nLibId, const Ice::Current & current)
{
    return LibraryManagement::Instance().DeleteLibrary(nLibId);
}

int CompareServerImpl::ListLibrary(std::string& strResult, const Ice::Current & current)
{
    return LibraryManagement::Instance().ListLibrary(strResult);
}

int CompareServerImpl::AddPerson(int nLibId, const std::string& strPicUrl,
    std::string& strResult, 
    const Ice::Current & current)
{
    return LibraryManagement::Instance().AddPerson(nLibId, strPicUrl, strResult);
}

int CompareServerImpl::DeletePerson(int nLibId, int64_t nPersonId, const Ice::Current & current)
{
    return LibraryManagement::Instance().DeletePerson(nLibId, nPersonId);
}

int CompareServerImpl::UpdatePersonPicture(int nLibId,
    int64_t nPersonId,
    const std::string& strPicUrl,
    std::string& strResult,
    const Ice::Current & current)
{
    return LibraryManagement::Instance().UpdatePerson(nLibId, nPersonId, strPicUrl, strResult);
}

int CompareServerImpl::GetFeatureSize(const Ice::Current & current)
{
    return LibraryManagement::Instance().GetFeatureSize();
}

int CompareServerImpl::ExtractFeature(const std::string& strPicUrl, 
    std::string& strResult, 
    const Ice::Current & current)
{
    return LibraryManagement::Instance().ExtractFeature(strPicUrl, strResult);
}

int CompareServerImpl::Compare1v1(const std::string& strLPicUrl,
    const std::string& strRPicUrl,
    float& nSimilarity,
    const Ice::Current & current)
{
    return LibraryManagement::Instance().Compare1v1(strLPicUrl, strRPicUrl, nSimilarity);
}

int CompareServerImpl::CompareWithFeature(const std::string& strLFerature,
    const std::string& strRFerature,
    float& nSimilarity,
    const Ice::Current & current)
{
    return LibraryManagement::Instance().CompareWithFeature(strLFerature, strRFerature, nSimilarity);
}

int CompareServerImpl::PushCompareTask(const Compare::CompareTask& task, const Ice::Current & current)
{
    // the task that client pushed
    TaskInfo infoTask;
    convert_task_info(task, infoTask);
    LibraryManagement::Instance().PushCompareTask(infoTask);
    return CS_OK;
}

void CompareServerImpl::PushCompareResult(const Compare::CompareResult& result)
{
    m_pThis->push_compare_result(result);
}

void CompareServerImpl::hold_heartbeat(const std::string& strId, Ice::ConnectionPtr conn)
{
#ifdef ICE_CPP11_MAPPING
    auto heart_beat_func = [](const Ice::ConnectionPtr& conn) { };
    conn->setHeartbeatCallback(heart_beat_func);
    std::shared_ptr<ClientCloseCallback>pClientCallback(new ClientCloseCallback(strId, this));
    conn->setCloseCallback(std::bind(&iceServerImpl::ClientCloseCallback::closed, pClientCallback, conn));
#else
    conn->setHeartbeatCallback(new ClientHeartbeatCallback());
    conn->setCloseCallback(new ClientCloseCallback(strId, this));
#endif // ICE_CPP11_MAPPING


    // If a configuration exists, the value of the configuration is taken.
    Ice::ACM acm = conn->getACM();
    if (acm.heartbeat != Ice::ACMHeartbeat::HeartbeatAlways
        || acm.close != Ice::ACMClose::CloseOff)
    {
        // A heartbeat is sent to the server every 10/2 s.
        conn->setACM(
            IceUtil::Optional<Ice::Int>(10),
            IceUtil::Optional<Ice::ACMClose>(Ice::ACMClose::CloseOff),
            IceUtil::Optional<Ice::ACMHeartbeat>(Ice::ACMHeartbeat::HeartbeatAlways));
    }
}

void CompareServerImpl::update_conn_list(const std::string& strId)
{
    std::lock_guard<std::mutex>lock(m_mtLock);
    if (m_mapConns.find(strId) != m_mapConns.end())
    {
        m_mapConns.erase(strId);
    }
}

void CompareServerImpl::heartbeat_callback(const ::std::shared_ptr<Ice::Connection>& con)
{
}

void CompareServerImpl::push_compare_result(const Compare::CompareResult& result)
{
    std::lock_guard<std::mutex>lock(m_mtLock);
    auto itMatch = m_mapConns.find(result.taskInfo.strIdentity);
    if (itMatch != m_mapConns.end())
        itMatch->second->PushCompareResult(result);
}



/********************************************************************************
*                           Client controler                                    *
*********************************************************************************/
CompareServerImpl::CompareClient::~CompareClient()
{
    try
    {
#ifdef ICE_CPP11_MAPPING // C++11 mapping
        m_pClient->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);
#else
        m_pClient->ice_getConnection()->close(Ice::ConnectionClose::ConnectionCloseGracefullyWithWait);
#endif
    }
    catch (...) {}

}

bool CompareServerImpl::CompareClient::Init(const std::string& strId, const Ice::Current &current)
{
    bool bSuccess = true;
    try
    {
        Ice::Identity id;
        id.name = strId;
        m_pClient = Ice::uncheckedCast<Compare::CompareClientPrx>(current.con->createProxy(id));
        m_pClient->ice_invocationTimeout(500);
    }
    catch (const Ice::Exception& e)
    {
        LOG(ERROR) << "Register client failed,error:" << e.what() << std::endl;
        bSuccess = false;
    }
    return bSuccess;
}

void CompareServerImpl::CompareClient::PushCompareResult(const Compare::CompareResult& result)
{
    // push the task's compare result
    CATCH_EXCEPTION_BEGIN;
    m_pClient->PushCompareResult(result);
    CATCH_EXCEPTION_END("PushCompareResult");
}
