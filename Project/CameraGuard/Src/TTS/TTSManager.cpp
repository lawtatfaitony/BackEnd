#include "TTSManager.h"
#include <string>
#include <Basic/Conversion.h>


void TTSManagement::Init()
{
#ifdef WIN32
    m_bInit = false;
    if (SUCCEEDED(::CoInitialize(NULL)))
        m_bInit = true;
#endif
}

void TTSManagement::Uninit()
{
    if(m_bInit)
        ::CoUninitialize();
}

void TTSManagement::PlaySound(const std::string& strInfo)
{
    if (!m_bInit) return;
    ISpVoice* pVoice = NULL;

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    if (SUCCEEDED(hr))
    {
        std::wstring wstrData = Basic::Gbk2Unicode(strInfo);
        pVoice->Speak(wstrData.c_str(), SPF_DEFAULT, NULL);
        pVoice->Release();
        pVoice = NULL;
    }
}

bool TTSManagement::SaveWavFile(const std::string& strInfo, const std::string& strFile)
{
    if(!m_bInit) return m_bInit;
    ISpVoice* pVoice = NULL;

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    CComPtr<ISpStream>cpWavStream;
    CComPtr<ISpStreamFormat>cpOldStream;
    CSpStreamFormat originalFmt;
    pVoice->GetOutputStream(&cpOldStream);
    originalFmt.AssignFormat(cpOldStream);

    std::wstring wstrWavFile = Basic::Gbk2Unicode(strFile);
    hr = SPBindToFile(wstrWavFile.c_str(), SPFM_CREATE_ALWAYS,
        &cpWavStream, &originalFmt.FormatId(),
        originalFmt.WaveFormatExPtr());
    if (SUCCEEDED(hr))
    {
        pVoice->SetOutput(cpWavStream, TRUE);
        std::wstring wstrData = Basic::Gbk2Unicode(strInfo);
        pVoice->Speak(wstrData.c_str(), SPF_IS_XML, NULL);
        pVoice->Release();
        pVoice = NULL;
    }
    return m_bInit;
}
