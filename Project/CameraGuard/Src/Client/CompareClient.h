/**************************************************************
* @brief:       compare client
* @date:         20200201
* @update:
* @auth:         Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
#include <atomic>
#include <Compare.h>
#include <Basic/IceDuplexClient.h>


namespace Client
{
    class CompareClient
    {
    private:
        CompareClient();
    public:
        static CompareClient& Instance()
        {
            static CompareClient g_Instance;
            return g_Instance;
        }
        bool Start();

        void Stop();
        bool IsConnected();
        std::string GetIdentityName();
#ifdef ICE_CPP11_MAPPING
        std::shared_ptr<Compare::CompareServer> GetProxy();
#else
        Compare::CompareServerPrx GetProxy();
#endif // ICE_CPP11_MAPPING
        // library
        int AddLibrary(int& lib_id);
        int DeleteLibrary(int nLibId);
        int ListLibrary(std::string& strResult);

        // person
        int AddPerson(int nLibId, const std::string& strPicUrl,
            std::string& strResult);
        int DeletePerson(int nLibId, int64_t nPersonId);
        int UpdatePerson(int nLibId, 
            int64_t nPersonId, 
            const std::string& strPicUrl,
            std::string& strResult);

        // compare
        int GetFeatureSize();
        int ExtractFeature(const std::string& strPicUrl, std::string& strResult);
        int Compare1v1(const std::string& strLPicUrl,
            const std::string& strRPicUrl,
            float& nSimilarity);
        int CompareWithFeature(const std::string& strLFeature,
            const std::string& strRFeature,
            float& nSimilarity);

    private:
        int load_library_info();

    private:
        IceClient::IceClientDuplex<Compare::CompareServerPrx> m_pClient;
        std::atomic<int> m_LibId;

    };
}