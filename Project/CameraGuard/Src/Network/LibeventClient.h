/**************************************************************
* @brief:       wrap of libevent
* @auth:         Wite_Chen
* @date:         20200129
* @update:
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
#include <map>
#include <errno.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <event2/util.h>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h> 
#include <ws2tcpip.h>
#endif // _Win32
#include <signal.h>
#include <functional>
#include <Basic/ThreadObject.h>


namespace Network
{
struct ServerInfo
{
    std::string strIP;
    int nPort;
    int nTimeout;
    std::atomic<bool> bConn;
    ServerInfo() { bConn.store(false); }
    ServerInfo(const ServerInfo& infoServer)
    {
        this->strIP = infoServer.strIP;
        this->nPort = infoServer.nPort;
        this->nTimeout = infoServer.nTimeout;
    }
    ServerInfo(const std::string& strIP, int nPort, int nTimeout)
    {
        this->strIP = strIP;
        this->nPort = nPort;
        this->nTimeout = nTimeout;
    }
    ServerInfo& operator=(const ServerInfo& infoServer)
    {
        this->strIP = infoServer.strIP;
        this->nPort = infoServer.nPort;
        this->nTimeout = infoServer.nTimeout;
        return *this;
    }
};
class LibeventClient
{
    using FuncNotify = std::function<void(const std::string&)>;
    // default cache size is 4096
    const static int kMaxData = 4096;
    const static int kCheckInterval = 1000;
public:
    LibeventClient();
    ~LibeventClient();
    void SetNotifyCallback(FuncNotify cbNotify);
    bool InitClient(const std::string& strIP, int nPort, int nTimeout = 60);
    void  Stop();
    bool WriteData(const std::string& strData);

private:
    /************************************************************
    *                   libevent callback                       *
    *************************************************************/
    static void set_tcp_no_delay(evutil_socket_t sock);
    static void server_msg_cb(bufferevent* pEvtBuffer, void* pArg);
    static void write_cb(bufferevent* pEvtBuffer, void * pArg);
    static void event_cb(bufferevent* pEvtBuffer, short nEvent, void* pArg);
    static void signal_cb(evutil_socket_t sock, short nEvent, void* pArg);
    static void timeout_cb(evutil_socket_t sock, short nEvent, void* pArg);

private:
    /************************************************************
    *                   libevent initialize                     *
    *************************************************************/
    bool init_environment();
    bool init_client();
    void run();
    void reconnect();
    void update_conn_state(bool bConnState = true);
    void release_data();
    void release_buffevent();
    void release_eventbase();
    void stop_dispatch();

public:
    FuncNotify m_cbNotify;
    ServerInfo m_infoServer;
    Basic::ThreadObject m_objRun;
    Basic::ThreadObject m_objCheckConn;
    event_base *m_pBase;
    struct event* m_pEvtStop;
    struct event* m_pEvtTimeout;
    struct bufferevent* m_pEvtBuffer;

};
}
