/**************************************************************
* @brief:       任务管理
* @date:        20200213
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <Compare.h>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <memory>
#include "TaskDefines.h"


class TaskHandle;
class StreamHandle;
typedef std::shared_ptr<TaskHandle> TaskHandlePtr;
typedef std::shared_ptr<StreamHandle> StreamHandlePtr;
class TaskManagement
{
    TaskManagement();
public:
    static TaskManagement& Instance();
    ~TaskManagement();
    int StartTask(const TaskInfo& task);
    int StopTask(const std::string& strIdentity, int nTaskId);
    void PushCompareResult(const Compare::CompareResult& result);


private:
    std::mutex m_mtTask;
    // pair<identity, pair<taskId, handle>>
    std::map<std::string, std::map<int, TaskHandlePtr>> m_mapTaskHandle;
    // pair<identity, pair<videoId, handle>>
    std::map<std::string, std::map<int, StreamHandlePtr>> m_mapStream;
    // pair<identity, pair<taskId, cameras>>
    std::map<std::string, std::map<int, std::vector<int>>> m_mapCamera;
    // pair<taskId, identity>
    std::map<int, std::string> m_mapTask;

};