#pragma once
#include<string>
#include<locale>
#include <codecvt>
#include "Basic.h"


NAMESPACE_BASIC_BEGIN
// local enviroment tool of wrapper of wstring/wbuffer_convert
template<class Facet>
struct deletable_facet : Facet
{
    template<class ...Args>
    deletable_facet(Args&& ...args)
        : Facet(std::forward<Args>(args)...) {}
    ~deletable_facet() {}
};


inline std::wstring Gbk2Unicode(const std::string &gbk)
{
#ifdef _MSC_VER
    const char* GBK_LOCALE_NAME = ".936";
#else
    const char* GBK_LOCALE_NAME = "zh_CN.GBK";
#endif

    typedef deletable_facet<std::codecvt_byname<wchar_t, char, std::mbstate_t>> gbfacet_t;

    std::wstring_convert<gbfacet_t> conv(new gbfacet_t(GBK_LOCALE_NAME));
    std::wstring utf16 = conv.from_bytes(gbk);
    return std::move(utf16);
}

inline std::string Unicode2Gbk(const std::wstring &utf16)
{
#ifdef _MSC_VER
    const char* GBK_LOCALE_NAME = ".936";
#else
    const char* GBK_LOCALE_NAME = "zh_CN.GBK";
#endif

    typedef deletable_facet<std::codecvt_byname<wchar_t, char, std::mbstate_t>> gbfacet_t;
    std::wstring_convert<gbfacet_t> conv(new gbfacet_t(GBK_LOCALE_NAME));

    std::string gbk = conv.to_bytes(utf16);
    return std::move(gbk);
}

inline std::wstring utf8_to_utf16(const std::string &utf8)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::wstring utf16 = conv.from_bytes(utf8);
    return std::move(utf16);
}

inline std::string utf16_to_utf8(const std::wstring &utf16)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    std::string utf8 = conv.to_bytes(utf16);
    return std::move(utf8);
}

static std::string Utf8ToGbk(const std::string & utf8)
{
    return Unicode2Gbk(utf8_to_utf16(utf8));
}

static std::string Gbk2Utf8(const std::string & gbk)
{
    return utf16_to_utf8(Gbk2Unicode(gbk));
}

NAMESPACE_BASIC_END