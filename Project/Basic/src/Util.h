#pragma once
#include "Basic.h"
#include <string>
#include <sstream>


NAMESPACE_BASIC_BEGIN
class Util
{
public:
    /*********************************************************
    *
    * @brief:       convert string to hex
    * @author:      wite_chen
    * @create:      20121-01-08
    * @para[in]:    strInput, input to convert
    * @return:      string, result of convert
    *
    *********************************************************/
    static std::string String2Hex(const std::string& strInput)
    {
        static std::string kHexString = "0123456789ABCDEF";
        std::stringstream ss;
        for (std::string::size_type i = 0; i < strInput.size(); ++i)
            ss << kHexString[(unsigned char)strInput[i] >> 4] << kHexString[(unsigned char)strInput[i] & 0xf];

        return ss.str();
    }
    /*********************************************************
    *
    * @brief:       convert hex to string
    * @author:      wite_chen
    * @create:      20121-01-08
    * @para[in]:    strInput, input to convert
    * @return:      string, result of convert
    *
    *********************************************************/
    static std::string Hex2String(const std::string& strInput)
    {
        std::string strResult;
        for (size_t i = 0; i < strInput.length(); i += 2)
        {
            std::string byte = strInput.substr(i, 2);
            char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
            strResult.push_back(chr);
        }
        return strResult;
    }

};
NAMESPACE_BASIC_END