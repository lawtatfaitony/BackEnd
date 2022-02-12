#include "CompareClientImpl.h"
#include "../ErrorInfo/ErrorCode.h"
#include "../Task/TaskManager.h"

using namespace Client;

int CompareClientImpl::PushCompareResult(const Compare::CompareResult& result, const Ice::Current& current)
{
    TaskManagement::Instance().PushCompareResult(result);
    return CTS_OK;
}