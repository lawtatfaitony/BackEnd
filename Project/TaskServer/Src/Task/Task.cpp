#include "Task.h"
#include "../Server/ServerImpl.h"

TaskHandle::TaskHandle()
{
    m_bExit = false;
}

TaskHandle::TaskHandle(const TaskInfo& infoTask)
    : m_infoTask(infoTask)
    , m_bExit(false)
{
    m_thRun = std::thread(std::bind(&TaskHandle::run, this), kHandleResult);
}

TaskHandle::~TaskHandle()
{
    if (m_thRun.joinable())
        m_thRun.join();
    m_bExit = true;
    m_cvRecord.notify_one();
}

void TaskHandle::PushCompareResult(const Compare::CompareResult& result)
{
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_listResult.push_back(result);
    }
    m_cvRecord.notify_one();
}

void TaskHandle::run()
{
    while (!m_bExit)
    {
        Compare::CompareResult result;
        {
            std::unique_lock<std::mutex> lock(m_mtLock);
            m_cvRecord.wait(lock, [=]()
            {
                return m_bExit || !m_listResult.empty();
            });
            result = m_listResult.front();
            m_listResult.pop_front();
        }
        Task::TaskResult tmpResult;
        {
            auto& infoTask = tmpResult.infoTask;
            infoTask.strIdetityId = m_infoTask.strIdetityId;
            infoTask.nTaskId = m_infoTask.nTaskId;
            infoTask.nInterval = m_infoTask.nInterval;
            infoTask.nThreshold = m_infoTask.nThreshold;
            tmpResult.nCameraId = result.taskInfo.nCameraId;
            tmpResult.strCapturePath = result.strPicPath;
            tmpResult.strCaptureTime = result.taskInfo.strCaptureTime;
            // compare result
            for (const auto& item : result.resultMatch)
            {
                Task::ComparePerson person;
                person.nPicId = item.first;
                person.fScore = item.second;
                tmpResult.seqPerson.push_back(person);
            }
        }
        TaskServerImpl::PushTaskResult(tmpResult);
    }
}