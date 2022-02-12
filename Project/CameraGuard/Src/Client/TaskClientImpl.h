#pragma once
#include <Task.h>


namespace Client
{
    class TaskClientImpl : public Task::TaskClient
    {
    public:
        virtual int PushTaskResult(const Task::TaskResult& result, const Ice::Current& current);
        virtual int UpdateCameraState(const Task::seqCameraState& state, const Ice::Current& current);
    };
}
