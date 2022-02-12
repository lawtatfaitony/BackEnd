#include "Server.h"
#include <JsonHelper.h>
#include <Macro.h>
#include "config/GlobalConfig.h"




using namespace Compare;
void CompareServer::HandleRequest(evutil_socket_t fd, const std::string& strData, void* pUser)
{
    CompareServer::Instance().handle_request(fd, strData);
}

CompareServer::CompareServer()
    : m_bExit(false)
{

}

CompareServer& CompareServer::Instance()
{
    static CompareServer g_Instance;
    return g_Instance;
}

CompareServer::~CompareServer()
{

}

bool CompareServer::Init()
{
    if (Network::LibeventServer::Instance().InitServer(SERVER_CONFIG.port))
    {
        printf("Init network failed\n");
        return false;
    }
    // register handler of client message
    Network::LibeventServer::Instance().RegisterReadingCallback(CompareServer::HandleRequest, nullptr);
    m_thDecode = std::thread(std::bind(&CompareServer::decode, &CompareServer::Instance()));


    return true;
}

void CompareServer::Uninit()
{
    Network::LibeventServer::Instance().Stop();
}

void CompareServer::PushMessage(const ClientMsg& msg)
{
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_listMsg.push_back(msg);
    }
    m_cvMsg.notify_one();
}


void CompareServer::decode()
{
    while (!m_bExit)
    {
        ClientMsg msg;
        {
            std::unique_lock<std::mutex> lock(m_mtLock);
            m_cvMsg.wait(lock, [=]() {
                if (!m_listMsg.empty() || m_bExit) return true;
                return false;
            });
            if (m_listMsg.empty())
            {
                msg = m_listMsg.front();
                m_listMsg.pop_front();
            }
        }
        
        
    }
}

void CompareServer::handle_request(evutil_socket_t fd, const std::string& strData)
{
    rapidjson::Document doc;
    JS_PARSE_OBJECT_VOID(doc, strData);
}