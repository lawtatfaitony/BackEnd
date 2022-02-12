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
        void Start();
        void Stop();
        bool IsConnected();
        std::string GetIdentityName();
#ifdef ICE_CPP11_MAPPING
        std::shared_ptr<Compare::CompareServer> GetProxy();
#else
        Compare::CompareServerPrx GetProxy();
#endif // ICE_CPP11_MAPPING
        int PushCompareTask(const Compare::CompareTask& task);

    private:
        IceClient::IceClientDuplex<Compare::CompareServerPrx>m_pClient;


    };
}