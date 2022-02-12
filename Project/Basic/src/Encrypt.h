#pragma once
#include "Basic.h"
#include <string>
#include <openssl/des.h>


NAMESPACE_BASIC_BEGIN
class Encrypt
{
    static const int kKeyLen = 8;
private:
    Encrypt();
    ~Encrypt();
    Encrypt(const Encrypt&);

public:
    /*********************************************************
    *
    * @brief:       encrypt with openssl des-cbc
    * @author:      wite_chen
    * @create:      20121-01-08
    * @para[in]:    strKey, key of encrypt
    * @para[in]:    strIvValue, IV value
    * @para[in]:    strInput, input to encrypt
    * @return:      string, result of convert
    *
    *********************************************************/
    static std::string DesCBCEncrypt(const std::string& strKey,
        const std::string& strIvValue,
        const std::string& strInput);

    /*********************************************************
        *
        * @brief:       decrypt with openssl des-cbc
        * @author:      wite_chen
        * @create:      20121-01-08
        * @para[in]:    strKey, key of encrypt
        * @para[in]:    strIvValue, IV value
        * @para[in]:    strInput, input to encrypt
        * @return:      string, result of convert
        *
        *********************************************************/
    static std::string DesCBCDecrypt(const std::string& strKey,
        const std::string& strIvValue,
        const std::string& strInput);
};
NAMESPACE_BASIC_END