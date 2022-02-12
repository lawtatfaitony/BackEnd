#include "TaskClientImpl.h"
#include <easylogging/easylogging++.h>
#include "../ErrorInfo/ErrorCode.h"
#include "../Task/Process/TaskResult.h"

using namespace Client;
int TaskClientImpl::PushTaskResult(const Task::TaskResult& result, const Ice::Current& current)
{
    LOG(INFO) << "Receive compare result:";
    TaskResult::Instance().PushTaskResult(result);
    return CG_OK;
}

int TaskClientImpl::UpdateCameraState(const Task::seqCameraState& state, const Ice::Current& current)
{
    return CG_OK;
}