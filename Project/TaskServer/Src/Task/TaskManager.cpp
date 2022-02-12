#include "TaskManager.h"
#include "Task.h"
#include "../Server/ServerImpl.h"
#include "../ErrorInfo/ErrorCode.h"
#include "StreamHandle.h"



TaskManagement::TaskManagement()
{

}

TaskManagement& TaskManagement::Instance()
{
    static TaskManagement g_Instance;
    return g_Instance;
}

TaskManagement::~TaskManagement()
{

}

int TaskManagement::StartTask(const TaskInfo& task)
{
    std::lock_guard<std::mutex> lock(m_mtTask);
    std::vector<int> vecVideo;
    {
        // check the task exist in cache
        auto itClient = m_mapTaskHandle.find(task.strIdetityId);
        if (itClient != m_mapTaskHandle.end())
        {
            auto itTask = itClient->second.find(task.nTaskId);
            if (itTask != itClient->second.end())
                return CTS_OK;
        }
    }
    {
        // add video stream handle with in
        for (const auto& item : task.mapCamera1)
        {
            auto itClient = m_mapStream.find(task.strIdetityId);
            if (itClient == m_mapStream.end())
            {
                StreamHandlePtr pNewStream(new StreamHandle(task, item.first, item.second));
                m_mapStream[task.strIdetityId][item.first] = pNewStream;
            }
            else
            {
                auto itStream = itClient->second.find(item.first);
                if (itStream == itClient->second.end())
                {
                    StreamHandlePtr pNewStream(new StreamHandle(task, item.first, item.second));
                    itClient->second[item.first] = pNewStream;
                }
                else
                    itStream->second->AddTask(task);
            }
            vecVideo.push_back(item.first);
        }

        // add video stream handle with out
        for (const auto& item : task.mapCamera2)
        {
            auto itClient = m_mapStream.find(task.strIdetityId);
            if (itClient == m_mapStream.end())
            {
                StreamHandlePtr pNewStream(new StreamHandle(task, item.first, item.second));
                m_mapStream[task.strIdetityId][item.first] = pNewStream;
            }
            else
            {
                auto itStream = itClient->second.find(item.first);
                if (itStream == itClient->second.end())
                {
                    StreamHandlePtr pNewStream(new StreamHandle(task, item.first, item.second));
                    itClient->second[item.first] = pNewStream;
                }
                else
                    itStream->second->AddTask(task);
            }
            vecVideo.push_back(item.first);
        }
    }

    TaskHandlePtr pNewHandle(new TaskHandle(task));
    m_mapTaskHandle[task.strIdetityId][task.nTaskId] = pNewHandle;
    m_mapCamera[task.strIdetityId][task.nTaskId] = vecVideo;
    m_mapTask[task.nTaskId] = task.strIdetityId;

    return CTS_OK;
}

int TaskManagement::StopTask(const std::string& strIdentity, int nTaskId)
{
    std::lock_guard<std::mutex> lock(m_mtTask);
    {
        // delete the video cache
        auto itClient = m_mapStream.find(strIdentity);
        auto itCamera = m_mapCamera.find(strIdentity);
        if (itClient != m_mapStream.end()
            && itCamera != m_mapCamera.end())
        {
            auto itTask = itCamera->second.find(nTaskId);
            for (const auto& item : itTask->second)
            {
                auto itStream = itClient->second.find(item);
                if (itStream != itClient->second.end())
                {
                    bool bDel;
                    itStream->second->RemoveTask(nTaskId, bDel);
                    if (bDel)
                    {
                        // delete the instance of stream
                        itClient->second.erase(item);
                    }
                }
            }
            // delete the task cache
            itCamera->second.erase(nTaskId);
        }
    }
    {
        //delete the task cache
        auto itClient = m_mapTaskHandle.find(strIdentity);
        if (itClient != m_mapTaskHandle.end())
        {
            auto itTask = itClient->second.find(nTaskId);
            if (itTask != itClient->second.end())
            {
                itClient->second.erase(itTask);
                return CTS_OK;
            }
        }
    }
    m_mapTask.erase(nTaskId);
    return CTS_TASK_NOT_EXIST;
}

void TaskManagement::PushCompareResult(const Compare::CompareResult& result)
{
    std::lock_guard<std::mutex> lock(m_mtTask);
    const auto& iterMatch = m_mapTask.find(result.taskInfo.nTaskId);
    if (iterMatch == m_mapTask.end()) return;
    auto itClient = m_mapTaskHandle.find(iterMatch->second);
    if (itClient != m_mapTaskHandle.end())
    {
        auto itHandle = itClient->second.find(result.taskInfo.nTaskId);
        if (itHandle != itClient->second.end())
        {
            Compare::CompareResult tmpResult = result;
            tmpResult.taskInfo.strIdentity = iterMatch->second;
            tmpResult.taskInfo.strCaptureTime = result.taskInfo.strCaptureTime;
            tmpResult.strPicPath = result.taskInfo.strPicPath;
            itHandle->second->PushCompareResult(tmpResult);
        }
    }
}