#include "TaskManager.h"
#include <rapidjson/document.h>
#include <soci/soci.h>
#include <easylogging/easylogging++.h>
#include <Macro.h>
#include <Basic/Function.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Config/GlobalConfig.h"
#include "../Client/TaskClient.h"
#include "../Cache/ConnectionPool/DatabaseConnPool.h"


TaskManager::RunningTaskMap TaskManager::s_mapTask;
int TaskManager::StartTask(int nTaskId)
{
    RunningTaskMap::LockerPtr lock;
    auto& taskSnap = s_mapTask.Fetch(lock);
    if (taskSnap.find(nTaskId) != taskSnap.end())
        return CG_TASK_IS_RUNNING;
    else
    {
        Task::TaskParaPtr pTask = std::make_shared<Task::TaskPara>();
        VERIFY_CODE_RETURN(pTask->MakeTaskPara(nTaskId));       // init the para
        VERIFY_CODE_RETURN(start_task(pTask));
        update_task_state(nTaskId);
        taskSnap[nTaskId] = pTask;
    }

    return CG_OK;
}

int TaskManager::StopTask(int nTaskId)
{
    {
        RunningTaskMap::LockerPtr lock;
        auto& taskSnap = s_mapTask.Fetch(lock);
        // stop the task and remove it
        VERIFY_CODE_RETURN(Client::TaskClient::Instance().StopTask(nTaskId));

        taskSnap.erase(nTaskId);
    }

    update_task_state(nTaskId, kStateOff);
    return CG_OK;
}

// interactive with task server
int TaskManager::start_task(Task::TaskParaPtr pTask)
{
    Task::TaskInfo infoTask;
    infoTask.nTaskId = pTask->nTaskId;
    infoTask.nInterval = pTask->nInterval;
    infoTask.nThreshold = pTask->fCompareShreshold;
    infoTask.nTop = pTask->nTopN;
    infoTask.strIdetityId = Client::TaskClient::Instance().GetIdentityName();
    Basic::SpiltFromString(pTask->strLib, infoTask.vecLib);
    infoTask.mapCamera1 = pTask->mapCam1;
    infoTask.mapCamera2 = pTask->mapCam2;

    return Client::TaskClient::Instance().StartTask(infoTask);
}

void TaskManager::update_task_state(int nTaskId, int nState)
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        conn << "update ft_task set `state` = :state where visible =1"
            << " and id=:id"
            , soci::use(nState)
            , soci::use(nTaskId);
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Update task's state exception:" << e.what();
    }
}