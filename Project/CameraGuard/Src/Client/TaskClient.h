/**************************************************************
* @brief:       task client
* @date:         20200214
* @update:
* @auth:         Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
#include <Task.h>
#include <Basic/IceDuplexClient.h>


namespace Client
{
    class TaskClient
    {
    private:
        TaskClient();
    public:
        static TaskClient& Instance()
        {
            static TaskClient g_Instance;
            return g_Instance;
        }
        void Start();

        void Stop();
        bool IsConnected();
        std::string GetIdentityName();
#ifdef ICE_CPP11_MAPPING
        std::shared_ptr<Task::TaskServer> GetProxy();
#else
        Task::TaskServerPrx GetProxy();
#endif // ICE_CPP11_MAPPING

        // operation of task
        int StartTask(const Task::TaskInfo& infoTask);
        int StopTask(int nTaskId);


    private:
        IceClient::IceClientDuplex<Task::TaskServerPrx>m_pClient;


    };
}