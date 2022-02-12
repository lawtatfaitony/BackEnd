#pragma once
#ifdef _WIN32
    #include <windows.h>
    #include <objbase.h>
#else
    
#endif
#include <string>
#include "Basic.h"


NAMESPACE_BASIC_BEGIN
static std::string GenerateGuuid()
{
    const static int kLen = 36;;
    char szGuuid[kLen] = { 0 };
#ifdef _WIN32
    GUID guid;
    CoCreateGuid(&guid);
    snprintf(szGuuid, kLen,
        "%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2],
        guid.Data4[3], guid.Data4[4], guid.Data4[5],
        guid.Data4[6], guid.Data4[7]);
#else
    
#endif
    return szGuuid;
}
NAMESPACE_BASIC_END
