/**************************************************************
* @brief:       task result handler
* @date:         20200404
* @update:
* @auth:         Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <Basic/SafeMap.h>
#include "TaskPara.h"


enum TaskState
{
    kStateOff,
    kStateOn
};
class TaskManager
{
public:
    typedef Basic::SafeMap<int, Task::TaskParaPtr> RunningTaskMap;

public:
    static int StartTask(int nTaskId);
    static int StopTask(int nTaskId);

private:
    static int start_task(Task::TaskParaPtr);
    static void update_task_state(int nTaskId, int nState = kStateOn);

private:
    static RunningTaskMap s_mapTask;

};