#include "Encrypt.h"
#include <algorithm>


USE_NAMESPACE_BASIC
Encrypt::Encrypt()
{
}


Encrypt::~Encrypt()
{
}

std::string Encrypt::DesCBCEncrypt(const std::string& strKey,
    const std::string& strIvValue,
    const std::string& strInput)
{
    // schedule
    DES_key_schedule schedule;
    const_DES_cblock szKey = { 0 };
    memcpy(szKey, (unsigned char*)strKey.c_str(), std::min((int)strKey.size(), kKeyLen));
    DES_set_key_unchecked(&szKey, &schedule);
    // IV
    DES_cblock szIvValue;
    memset(szIvValue, 0, sizeof(szIvValue));
    memcpy(szIvValue, strIvValue.c_str(), std::min((int)strIvValue.size(), kKeyLen));
    // fill data with Multiples of 8
    int nPadding = kKeyLen - strInput.length() % kKeyLen;
    std::string strSrcTmp;
    strSrcTmp.assign(strInput.length() + nPadding, (char)nPadding);
    memcpy((char*)strSrcTmp.c_str(), strInput.c_str(), strInput.length());
    // encrypt
    std::string strRst;
    strRst.assign(strInput.length() + nPadding, '\0');
    DES_cbc_encrypt((const uint8_t*)strSrcTmp.c_str(),
        (uint8_t*)strRst.c_str(), strRst.size(),
        &schedule,
        &szIvValue,
        DES_ENCRYPT);
    return strRst;
}

std::string Encrypt::DesCBCDecrypt(const std::string& strKey,
    const std::string& strIvValue,
    const std::string& strInput)
{
    // schedule
    DES_key_schedule schedule;
    const_DES_cblock szKey = { 0 };
    memcpy(szKey, (unsigned char*)strKey.c_str(), std::min((int)strKey.size(), kKeyLen));
    DES_set_key_unchecked(&szKey, &schedule);
    // IV
    DES_cblock szIvValue;
    memset(szIvValue, 0, sizeof(szIvValue));
    memcpy(szIvValue, strIvValue.c_str(), std::min((int)strIvValue.size(), kKeyLen));
    // decrypt
    std::string strRst;
    strRst.assign(strInput.size(), '\0');
    DES_cbc_encrypt((const uint8_t*)strInput.c_str(),
        (uint8_t*)strRst.c_str(),
        strRst.length(),
        &schedule,
        &szIvValue,
        DES_DECRYPT);
    strRst.erase(strRst.size() - strRst.back());
    return strRst;
}