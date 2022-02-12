/**************************************************************
* @brief:       单个任务管理实例
* @date:        20200213
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <Compare.h>
#include "TaskDefines.h"
#include <mutex>
#include <list>
#include <condition_variable>
#include <thread>


class TaskHandle
{
    static const int kHandleResult = 200;
public:
    TaskHandle();
    TaskHandle(const TaskInfo& infoTask);
    ~TaskHandle();
    void PushCompareResult(const Compare::CompareResult& result);


private:
    void run();

private:
    bool m_bExit;
    TaskInfo m_infoTask;
    std::mutex m_mtLock;
    std::list<Compare::CompareResult> m_listResult;
    std::condition_variable m_cvRecord;
    std::thread m_thRun;

};