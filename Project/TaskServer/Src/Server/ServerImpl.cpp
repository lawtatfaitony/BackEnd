#include "ServerImpl.h"
#include <easylogging/easylogging++.h>
#include <Macro.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Task/TaskManager.h"


void convert_task_info(const Task::TaskInfo& infoTask, TaskInfo& task)
{
    task.strIdetityId = infoTask.strIdetityId;
    task.nTaskId = infoTask.nTaskId;
    task.veclib = infoTask.vecLib;
    task.mapCamera1 = infoTask.mapCamera1;
    task.mapCamera2 = infoTask.mapCamera2;
    task.nInterval = infoTask.nInterval;
    task.nThreshold = infoTask.nThreshold;
    task.nTop = infoTask.nTop;
};

TaskServerImpl* TaskServerImpl::m_pThis = nullptr;
TaskServerImpl::TaskServerImpl()
{
    m_pThis = this;
}


TaskServerImpl::~TaskServerImpl()
{
    m_mapClient.clear();
}

void TaskServerImpl::RegisterClient(const std::string& strId, const Ice::Current & current)
{
    ClientPtr pClient(new TaskClient);
    if (pClient->Init(strId, current))
    {
        std::lock_guard<std::mutex>lock(m_mtLock);
        m_mapClient[strId] = pClient;
    }
    LOG(INFO) << "New connection, id:" << strId;
    hold_heartbeat(strId, current.con);
}

int TaskServerImpl::StartTask(const Task::TaskInfo& infoTask,
    const Ice::Current& current)
{
    TaskInfo task;
    convert_task_info(infoTask, task);

    return TaskManagement::Instance().StartTask(task);
}

int TaskServerImpl::StopTask(const std::string& strId, int nTaskId, const Ice::Current & current)
{
    return TaskManagement::Instance().StopTask(strId, nTaskId);
}

void TaskServerImpl::PushTaskResult(const Task::TaskResult& result)
{
    m_pThis->push_task_result(result);
}

void TaskServerImpl::UpdateCameraState(const std::string& strIdentity, const Task::seqCameraState& stateCamera)
{
    m_pThis->update_camera_state(strIdentity, stateCamera);
}

void TaskServerImpl::hold_heartbeat(const std::string& strId, Ice::ConnectionPtr conn)
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

void TaskServerImpl::update_conn_list(const std::string& strId)
{
    std::lock_guard<std::mutex>lock(m_mtLock);
    if (m_mapClient.find(strId) != m_mapClient.end())
    {
        m_mapClient.erase(strId);
    }
}

void TaskServerImpl::heartbeat_callback(const ::std::shared_ptr<Ice::Connection>& con)
{
}

void TaskServerImpl::push_task_result(const Task::TaskResult& result)
{
    std::lock_guard<std::mutex>lock(m_mtLock);
    auto itMatch = m_mapClient.find(result.infoTask.strIdetityId);
    if (itMatch != m_mapClient.end())
        itMatch->second->PushTaskResult(result);
}

void TaskServerImpl::update_camera_state(const std::string& strIdentity, const Task::seqCameraState& stateCamera)
{
    std::lock_guard<std::mutex>lock(m_mtLock);
    auto itMatch = m_mapClient.find(strIdentity);
    if (itMatch != m_mapClient.end())
        itMatch->second->UpdateCameraState(stateCamera);
}



/********************************************************************************
*                           Client controler                                    *
*********************************************************************************/
TaskServerImpl::TaskClient::~TaskClient()
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

bool TaskServerImpl::TaskClient::Init(const std::string& strId, const Ice::Current &current)
{
    bool bSuccess = true;
    try
    {
        Ice::Identity id;
        id.name = strId;
        m_pClient = Ice::uncheckedCast<Task::TaskClientPrx>(current.con->createProxy(id));
        m_pClient->ice_invocationTimeout(500);
    }
    catch (const Ice::Exception& e)
    {
        LOG(ERROR) << "Register client failed,error:" << e.what() << std::endl;
        bSuccess = false;
    }
    return bSuccess;
}

void TaskServerImpl::TaskClient::PushTaskResult(const Task::TaskResult& result)
{
    // push the task's compare result
    CATCH_EXCEPTION_BEGIN;
    m_pClient->PushTaskResult(result);
    CATCH_EXCEPTION_END("PushCompareResult");
}

void TaskServerImpl::TaskClient::UpdateCameraState(const Task::seqCameraState& stateCamera)
{
    // update the camera's state
    CATCH_EXCEPTION_BEGIN;
    m_pClient->UpdateCameraState(stateCamera);
    CATCH_EXCEPTION_END("UpdateCameraState");
}
