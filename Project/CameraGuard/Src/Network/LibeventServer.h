/**************************************************************
* @brief:       wrap of libevent server
* @auth:        Wite_Chen
* @date:        20200129
* @update:
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
#include <mutex>
#include <map>
#include <thread>
#include <errno.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include <event2/util.h>
#include <signal.h>


#ifdef WIN32
#include <winsock2.h>
#include <windows.h> 
#include <ws2tcpip.h>
#endif // _Win32

#include <Basic/Singleton.h>


namespace Network
{
class LibeventServer : public Basic::Singleton<LibeventServer>
{
    friend class Basic::Singleton<LibeventServer>;
    // default cache size is 4096
    const static int kMaxData = 4096;

public:
    ~LibeventServer();
    bool InitServer(int nPort, int nTimeout = 60);
    void Stop();

private:
    LibeventServer();
    LibeventServer(const LibeventServer&) = delete;
    LibeventServer& operator=(const LibeventServer&) = delete;
    LibeventServer(const LibeventServer&&) = delete;
    LibeventServer& operator=(const LibeventServer&&) = delete;

private:
    /************************************************************
    *                   libevent initialization                 *
    *************************************************************/
    static void conn_cb(evconnlistener *pListener,
                        evutil_socket_t sock,
                        struct sockaddr *sockAddr,
                        int socklen, 
                        void *pArg);
    static void set_tcp_no_delay(evutil_socket_t sock);
    static void read_cb(bufferevent * pEvtBuffer, void* pArg);
    static void write_cb(bufferevent * pEvtBuffer, void* pArg);
    static void event_cb(bufferevent * pEvtBuffer, short nEvent, void* pArg);
    static void signal_cb(evutil_socket_t sock, short nEvent, void* pArg);
    static void timeout_cb(evutil_socket_t sock, short nEvent, void* pArg);

private:
    /************************************************************
    *                   data interactive                        *
    *************************************************************/
    void run();
    void add_client(evutil_socket_t sock, bufferevent *pEvtBuffer);
    void remove_client(evutil_socket_t sock);

public:
    std::thread m_thRun;
    event_base *m_pBase;
    evconnlistener *m_pListener;
    struct event* m_pEvtStop;
    struct event* m_pEvtTimeout;
    std::mutex m_mtLock;
    std::map<evutil_socket_t, bufferevent*> m_mapEvtBuffer;

};
}
