/**************************************************************
* @brief:       task result handler
* @date:         20200407
* @update:
* @auth:         Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <Task.h>
#include <Basic/ThreadPool.h>
#include <map>
#include <mutex>



class TaskResult
{
    TaskResult();

public:
    static TaskResult& Instance();
    ~TaskResult();
    void Init();
    void Unint();
    void PushTaskResult(const Task::TaskResult& taskResult);

private:
    void handle_result(const Task::TaskResult& taskResult);
    bool check_invalid_result(Task::TaskResult& taskResult);
    bool filter_duplication(Task::TaskResult& taskResult);
    void filter_stranger(Task::TaskResult& taskResult);
    int fill_person_info(Task::TaskResult& taskResult);
    int save_result(const Task::TaskResult& taskResult);

private:
    Basic::ThreadPool m_poolHandleResult;

    std::mutex m_mtRecord;
    // std::pair<pic_id, std::pair<camera_id, timestamp>>
    std::map<int, std::pair<int, int64_t>> m_mapRecord;


};