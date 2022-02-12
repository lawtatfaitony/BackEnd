#pragma once
#include <Compare.h>

namespace Client
{
    class CompareClientImpl : public Compare::CompareClient
    {
    public:
        virtual int PushCompareResult(const Compare::CompareResult& result, const Ice::Current& current);

    };
}
