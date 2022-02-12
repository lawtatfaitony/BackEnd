#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <functional>
#include <map>
#include <mongoose/mongoose.h>
#include "Config/GlobalConfig.h"


namespace Service
{
    class CamGuardService
    {
        typedef std::function<int(const std::string& strInput, std::string& strResult)> INVOKE_FUNC;
    private:
        CamGuardService();
    public:
        static CamGuardService& Instance();
        ~CamGuardService();
        bool Start();
        void Stop();

    private:
        static void event_handle(mg_connection *pConn, int nEvtType, void *pEvtData);
        // user
        static void handle_login(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_register_user(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_update_user(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_delete_user(mg_connection *pConn, int nEvtType, void *pEvtData);
        // library
        static void handle_add_library(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_delete_library(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_query_library_list(mg_connection *pConn, int nEvtType, void *pEvtData);
        // person
        static void handle_add_person(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_delete_person(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_update_person(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_query_person_list(mg_connection *pConn, int nEvtType, void *pEvtData);
        // camera
        static void handle_add_camera(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_delete_camera(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_update_camera(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_query_camera_list(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_search_camera(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_query_camera_rtsp(mg_connection *pConn, int nEvtType, void *pEvtData);
        // task
        static void handle_add_task(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_delete_task(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_update_task(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_query_task_list(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_start_task(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_stop_task(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void handle_compare1v1(mg_connection *pConn, int nEvtType, void *pEvtData);
        // config
        static void get_system_config(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void update_system_config(mg_connection *pConn, int nEvtType, void *pEvtData);
        // history
        static void query_task_capture_record(mg_connection *pConn, int nEvtType, void *pEvtData);
        static void delete_task_capture_record(mg_connection *pConn, int nEvtType, void *pEvtData);

        static void call_wrap(mg_connection *pConn, void* pData, INVOKE_FUNC func);

    private:
        void run();
        void init_handle(mg_connection* pConn);
        void check_keep_alive(struct http_message *pHttpMsg,bool& bKeepAlive);
        static void do_response(mg_connection *pConn,
            bool bKeepAlive, 
            int nErr, const 
            std::string& strRst);

    private:
        // std::pair<uri, func>
        static std::map<std::string, mg_event_handler_t> m_mapFunc;
        mg_serve_http_opts m_optsHttp;
        mg_mgr m_Mgr;   // manager of event
        std::atomic_bool m_bExit;
        std::thread m_thRun;

    };
}