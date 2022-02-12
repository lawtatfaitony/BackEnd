#pragma once
#include <string>
#include "Basic.h"


NAMESPACE_BASIC_BEGIN
template<class TList>
static void SpiltFromString(const std::string& strData, TList& result, const char szDim = ',')
{
    if (strData.empty()) return;
    int nIndex = 0;
    int nPrePos = 0;
    int nCurrPos = 0;
    do {
        nCurrPos = strData.find(szDim, nPrePos);
        int nCnt = (std::string::npos == nCurrPos) ? std::string::npos : nCurrPos - nPrePos;
        std::string strTmp = strData.substr(nPrePos, nCnt);
        int nData = std::stoi(strTmp);
        result.push_back(nData);
        if (std::string::npos == nCnt)break;
        nPrePos = ++nCurrPos;
    } while (false);
}

template<class TList>
static void ContainerToString(const  TList& input, std::string& result, const char szDim = ',')
{
    std::stringstream ss;
    for (const auto& item : input)
    {
        ss << item << szDim;
    }
    result = ss.str();
    if (!result.empty())
        result.pop_back();
}
NAMESPACE_BASIC_END