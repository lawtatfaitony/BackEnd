#include "LibeventClient.h"
#include <functional>

using namespace Network;
LibeventClient::LibeventClient()
    : m_pBase(nullptr)
    , m_pEvtStop(nullptr)
    , m_pEvtBuffer(nullptr)
{
}

LibeventClient::~LibeventClient()
{
    Stop();
}


void LibeventClient::SetNotifyCallback(FuncNotify cbNotify)
{
    m_cbNotify = cbNotify;
}

/*
* @brief:       init the client
* @date:        20200129
* @update:
* @param[in]:   strIP, server ip
* @param[in]:   nPort, port to listen
* @param[in]:   nTimeout, time to lost interactive
* @return:      bool, success to return true, or false
*/
bool LibeventClient::InitClient(const std::string& strIP, int nPort, int nTimeout/* = 60*/)
{
    m_infoServer = ServerInfo(strIP, nPort, nTimeout);
    if (!init_environment()) return false;
    if (!init_client()) return false;

    m_objCheckConn.Start(std::bind(&LibeventClient::reconnect, this), kCheckInterval);

    return true;
}

/*
* @brief:       stop to dispatch the message
* @date:        20200129
* @update:
* @return:      void
*/
void LibeventClient::Stop()
{
    release_data();
    m_objRun.Stop();
    m_objCheckConn.Stop();
}

/*
* @brief:       write data
* @date:        20200129
* @update:
* @param[in]:   strData, data to write
* @return:      void
*/
bool LibeventClient::WriteData(const std::string& strData)
{
    if (nullptr == m_pEvtBuffer) return false;
    // 0 to return true, or false
    return 0 == bufferevent_write(m_pEvtBuffer, strData.c_str(), strData.length() + 1);
}

/*
* @brief:       handle read signal, triggerd with writing of server
* @date:        20200129
* @update:
* @param[in]:   pEvtBuffer, the buffer of event
* @param[in]:   pArg, extra para
* @return:      void
*/
void LibeventClient::server_msg_cb(bufferevent *pEvtBuffer, void * pArg)
{
    size_t nLen;
    char szMsg[1024];
    while ((nLen = bufferevent_read(pEvtBuffer, szMsg, sizeof(szMsg)) > 0))
    {
        printf("recv %s from server, %zd length\n", szMsg, nLen);
    }
    auto pThis = (LibeventClient*)pArg;
    if (pThis != nullptr && pThis->m_cbNotify)
    {
        pThis->m_cbNotify(szMsg);
    }
}

/*
* @brief:       handle write gignal, triggered by writing of client
* @date:        20200129
* @update:
* @param[in]:   pEvtBuffer, the buffer of event
* @param[in]:   pArg, extra para
* @return:      void
*/
void LibeventClient::write_cb(bufferevent *pEvtBuffer, void * pArg)
{
}

/*
* @brief:       catch the event of client
* @date:        20200129
* @update:
* @param[in]:   pEvtBuffer, the buffer of event
* @param[in]:   nEvent, event type
* @param[in]:   pArg, extra para
* @return:      void
*/
void LibeventClient::event_cb(bufferevent * pEvtBuffer, short nEvent, void *pArg)
{
    struct evbuffer *pOutputBuff = bufferevent_get_output(pEvtBuffer);
    size_t remain = evbuffer_get_length(pOutputBuff);

    if (nEvent & BEV_EVENT_TIMEOUT)
        printf("Timed out\n");  //if bufferevent_set_timeouts() called.
    else if (nEvent & BEV_EVENT_EOF)
        printf("connection closed, remain %d\n", remain);
    else if (nEvent & BEV_EVENT_ERROR)
        printf("some other error, remain %d\n", remain);
    else if (nEvent & BEV_EVENT_CONNECTED)
    {
        printf("connect server success\n");
        evutil_socket_t sock = bufferevent_getfd(pEvtBuffer);
        set_tcp_no_delay(sock);
        return;
    }

    // it'll close the socket and free the buffer-cache automaticaly
    auto pThis = (LibeventClient*)pArg;
    if (pThis != nullptr)
    {
        pThis->update_conn_state(false);
        pThis->release_buffevent();
        pThis->stop_dispatch();
    }
}

/*
* @brief:       catch the event of socket
* @date:        20200129
* @update:
* @param[in]:   sock, socket id
* @param[in]:   nEvent, event type
* @param[in]:   pArg, extra para
* @return:      void
*/
void LibeventClient::signal_cb(evutil_socket_t sock, short nEvent, void* pArg)
{
    printf("exception: interrupt, stop now!\n");
    auto pThis = (LibeventClient*)pArg;
    if (pThis != nullptr && pThis->m_pBase != nullptr)
    {
        event_base_loopexit(pThis->m_pBase, NULL);
    }
}

/*
* @brief:       catch the timeout of socket
* @date:        20200129
* @update:
* @param[in]:   sock, socket id
* @param[in]:   nEvent, event type
* @param[in]:   pArg, extra para
* @return:      void
*/
void LibeventClient::timeout_cb(evutil_socket_t sock, short nEvent, void *pArg)
{
    printf("error: timeout\n");

    auto pThis = (LibeventClient*)(pArg);
    if (pThis != nullptr)
    {
        pThis->update_conn_state(false);
        if (pThis->m_pBase != nullptr)
        {
            event_base_loopexit(pThis->m_pBase, NULL);
        }
    }
}

/*
* @brief:       set the block state of socket
* @date:        20190830
* @update:
* @param:       sock, scoket id
* @return:      void
*/
void LibeventClient::set_tcp_no_delay(evutil_socket_t sock)
{
    int nLen = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&nLen, sizeof(nLen));
}

/*
* @brief:       initial the environment
* @date:        20200129
* @update:
* @return:      void
*/
bool LibeventClient::init_environment()
{
#ifdef _WIN32
    WSADATA wsaData;
    DWORD nError;
    if ((nError = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
    {
        printf("WSAStartup failed with error %d\n", nError);
        return false;
    }
    evthread_use_windows_threads();
#endif // _WIN32

    return true;
}

/*
* @brief:       initial the client
* @date:        20200129
* @update:
* @return:      void
*/
bool LibeventClient::init_client()
{
    m_pBase = event_base_new();
    if (nullptr == m_pBase)
    {
        printf("couldn't allocate event base!\n");
        return false;
    }
    // signal
    m_pEvtStop = evsignal_new(m_pBase, SIGINT, signal_cb, this);
    evsignal_add(m_pEvtStop, nullptr);

    // timeout
    struct timeval timeVar { m_infoServer.nTimeout, 0 };
    m_pEvtTimeout = evtimer_new(m_pBase, timeout_cb, this);
    evtimer_add(m_pEvtTimeout, &timeVar);
    // address of server
    struct sockaddr_in addrSock;
    memset(&addrSock, 0, sizeof(struct sockaddr_in));
    addrSock.sin_family = AF_INET;
    addrSock.sin_port = htons(m_infoServer.nPort);
    addrSock.sin_addr.s_addr = inet_addr(m_infoServer.strIP.c_str());

    m_pEvtBuffer = bufferevent_socket_new(m_pBase, -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE);
    if (nullptr == m_pEvtBuffer) return false;
    bufferevent_setcb(m_pEvtBuffer, server_msg_cb, nullptr, event_cb, this);
    bufferevent_enable(m_pEvtBuffer, EV_READ | EV_WRITE | EV_PERSIST);

    if (bufferevent_socket_connect(m_pEvtBuffer, (struct sockaddr*)&addrSock,
        sizeof(addrSock)) < 0)
    {
        printf("Error starting connection!\n");
        bufferevent_free(m_pEvtBuffer);
        m_pEvtBuffer = nullptr;
        return false;
    }
    update_conn_state();
    m_objRun.StartOnce(std::bind(&LibeventClient::run, this));
    return true;
}

/*
* @brief:       start dispatch message
* @date:        20200129
* @update:
* @return:      void
*/
void LibeventClient::run()
{
    if (m_pBase != nullptr)
    {
        event_base_dispatch(m_pBase);
    }
}

/*
* @brief:       reconnect server
* @date:        20200129
* @update:
* @return:      void
*/
void LibeventClient::reconnect()
{
    if (!m_infoServer.bConn.load())
    {
        m_objRun.Stop();
        release_data();
        if (init_client())
        {
            printf("Reconnect the server %s, port %d success\n", m_infoServer.strIP.c_str(), m_infoServer.nPort);
        }
    }
}

/*
* @brief:       update state that connect server
* @date:        20200129
* @update:
* @para[in]:    bConnState, the state value
* @return:      void
*/
void LibeventClient::update_conn_state(bool bConnState/* = true*/)
{
    m_infoServer.bConn = bConnState;
}

/*
* @brief:       release data
* @date:        20200129
* @update:
* @return:      void
*/
void LibeventClient::release_data()
{
    if (nullptr != m_pEvtStop)
    {
        event_free(m_pEvtStop);
        m_pEvtStop = nullptr;
    }
    release_buffevent();
    release_eventbase();
}

/*
* @brief:       release buffer event
* @date:        20200129
* @update:
* @return:      void
*/
void LibeventClient::release_buffevent()
{
    if (m_pEvtBuffer)
    {
        bufferevent_free(m_pEvtBuffer);
        m_pEvtBuffer = nullptr;
    }
}

/*
* @brief:       release event base
* @date:        20200129
* @update:
* @return:      void
*/
void LibeventClient::release_eventbase()
{
    if (nullptr != m_pBase)
    {
        event_base_loopexit(m_pBase, nullptr);
        event_base_free(m_pBase);
        m_pBase = nullptr;
    }
}

/*
* @brief:       stop dispatch
* @date:        20200129
* @update:
* @return:      void
*/
void LibeventClient::stop_dispatch()
{
    if (nullptr != m_pBase)
    {
        event_base_loopexit(m_pBase, nullptr);
    }
}
