/**************************************************************
* @brief:       TTS manager
* @auth:         Wite_Chen
* @date:         20210106
* @update:
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
#include <Basic/Singleton.h>

#ifdef WIN32
#include <sphelper.h>


#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"sapi.lib")
#endif

class TTSManagement : public Basic::Singleton<TTSManagement>
{
    const int kSyncTime = 1000;
public:
    void Init();
    void Uninit();
    void PlaySound(const std::string& strInfo);
    bool SaveWavFile(const std::string& strInfo, const std::string& strFile);

private:
    bool m_bInit;
};

