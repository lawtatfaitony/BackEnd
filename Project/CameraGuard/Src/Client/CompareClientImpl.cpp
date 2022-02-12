#include "CompareClientImpl.h"
#include "../ErrorInfo/ErrorCode.h"

using namespace Client;

int CompareClientImpl::PushCompareResult(const Compare::CompareResult& result, const Ice::Current& current)
{
    return CG_OK;
}