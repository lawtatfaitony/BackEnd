/**************************************************************
* @brief:       �ȶԷ������
* @date:        20200131
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include "network/LibeventServer.h"
#include <mutex>
#include <list>
#include <atomic>
#include <thread>


/*��ɹ���
1���׿����/ɾ��
2���������
3��������ȡ
4�������ȶ�
*/

namespace Compare
{
    typedef struct ClientMessage
    {
        evutil_socket_t fd;
        std::string strData;
    } ClientMsg;

    class CompareServer
    {
        CompareServer();
    public:
        static void HandleRequest(evutil_socket_t fd, const std::string& strData, void* pUser);
        static CompareServer& Instance();
        ~CompareServer();
        bool Init();
        void Uninit();
        void PushMessage(const ClientMsg& msg);
        void PopMessage(ClientMsg& msg);



    private:
        void decode();
        void handle_request(evutil_socket_t fd, const std::string& strData);

    private:
        std::atomic_bool m_bExit;
        std::thread m_thDecode;
        std::mutex m_mtLock;
        std::condition_variable m_cvMsg;
        std::list<ClientMsg> m_listMsg;


    };
}