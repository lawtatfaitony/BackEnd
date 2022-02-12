#pragma once
#include "Basic.h"


NAMESPACE_BASIC_BEGIN
template <class T>
class Singleton
{
public:
    static T& Instance()
    {
        static T g_instance;
        return g_instance;
    }

};
NAMESPACE_BASIC_END