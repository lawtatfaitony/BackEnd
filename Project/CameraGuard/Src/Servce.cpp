#include "Servce.h"
#include <easylogging/easylogging++.h>
#include <Basic/Time.h>
#include <Macro.h>
#include "ErrorInfo/ErrorCode.h"
#include "ErrorInfo/ErrorMessage.h"
#include "interface/Interface.h"
#include "interface/user.h"
#include "interface/library.h"
#include "interface/camera.h"
#include "interface/task.h"
#include "interface/person.h"
#include "interface/SystemConfig.h"
#include "Interface/History.h"


using namespace Service;
std::map<std::string, mg_event_handler_t> CamGuardService::m_mapFunc
{
    // user
    { "/Login", CamGuardService::handle_login },
    { "/RegisterUser", CamGuardService::handle_register_user },
    { "/UpdateUser", CamGuardService::handle_update_user },
    { "/DeleteUser", CamGuardService::handle_delete_user },
    // library
    { "/AddLibrary", CamGuardService::handle_add_library },
    { "/DeleteLibrary", CamGuardService::handle_delete_library },
    { "/QueryLibraryList", CamGuardService::handle_query_library_list },
    // person
    { "/AddPerson", CamGuardService::handle_add_person },
    { "/DeletePerson", CamGuardService::handle_delete_person },
    { "/UpdatePerson", CamGuardService::handle_update_person },
    { "/QueryPersonList", CamGuardService::handle_query_person_list },
    // camera
    { "/AddCamera", CamGuardService::handle_add_camera },
    { "/DeleteCamera", CamGuardService::handle_delete_camera },
    { "/UpdateCamera", CamGuardService::handle_update_camera },
    { "/QueryCameraList", CamGuardService::handle_query_camera_list },
    { "/SearchCamera", CamGuardService::handle_search_camera },
    { "/QueryCameraRtsp", CamGuardService::handle_query_camera_rtsp },
    // task
    { "/AddTask", CamGuardService::handle_add_task },
    { "/DeleteTask", CamGuardService::handle_delete_task },
    { "/UpdateTask", CamGuardService::handle_update_task },
    { "/QueryTaskList", CamGuardService::handle_query_task_list },
    { "/StartTask", CamGuardService::handle_start_task },
    { "/StopTask", CamGuardService::handle_stop_task },
    // config
    { "/UpdateSystemConfig", CamGuardService::update_system_config },
    { "/GetSystemConfig", CamGuardService::get_system_config },
    // history
    { "/QueryTaskCaptureRecord", CamGuardService::query_task_capture_record },
    { "/DeleteTaskCaptureRecord", CamGuardService::delete_task_capture_record }
};

CamGuardService::CamGuardService()
{
    m_bExit.store(false);
}

CamGuardService& CamGuardService::Instance()
{
    static CamGuardService g_Instance;
    return g_Instance;
}

CamGuardService::~CamGuardService()
{
    Stop();
}

bool CamGuardService::Start()
{
    mg_mgr_init(&m_Mgr, NULL);
    std::string strPort= std::to_string(CFG_HTTP_SERVER.port);
    mg_connection *pConn = mg_bind(&m_Mgr, strPort.c_str(), &CamGuardService::event_handle);
    if (nullptr == pConn)
    {
        LOG(INFO) << "Bind http server at port:" << strPort.c_str() << " error";
        return false;
    }
    //for both http and websocket
    mg_set_protocol_http_websocket(pConn);
    init_handle(pConn);
    LOG(INFO) << "starting http server at port:" << strPort.c_str() << " success";
    m_thRun = std::thread(&CamGuardService::run, this);
    return true;
}

void CamGuardService::Stop()
{
    m_bExit.store(true);
    if(m_thRun.joinable())
        m_thRun.join();
}

void CamGuardService::event_handle(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    switch (nEvtType)
    {
    case MG_EV_HTTP_REQUEST: //http
    {
        http_message* pHttpMsg = static_cast<http_message*>(pEvtData);
        mg_printf(
            pConn,
            "%s",
            "HTTP/1.1 501 Not Implemented\r\n"
            "Content-Length: 0\r\n\r\n");
        mg_printf_http_chunk(pConn, "", 0);
        break;
    }
    //websocket
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
    case MG_EV_WEBSOCKET_FRAME:
    case MG_EV_CLOSE:
    {
        websocket_message *pWebMsg = (websocket_message*)pEvtData;
        break;
    }
    }
}

/**************************************************************
*                               user                          *
***************************************************************/
void CamGuardService::handle_login(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return UserManagement::Login(strInput, strResult);
    });
}

void CamGuardService::handle_register_user(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return UserManagement::RegisterUser(strInput, strResult);
    });
}

void CamGuardService::handle_update_user(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return UserManagement::UpdateUser(strInput, strResult);
    });
}

void CamGuardService::handle_delete_user(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return UserManagement::DeleteUser(strInput, strResult);
    });
}

/**************************************************************
*                               library                       *
***************************************************************/
void CamGuardService::handle_add_library(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return LibraryManagement::AddLibrary(strInput, strResult);
    });
}

void CamGuardService::handle_delete_library(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return LibraryManagement::DeleteLibrary(strInput, strResult);
    });
}

void CamGuardService::handle_query_library_list(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return LibraryManagement::QueryLibraryList(strInput, strResult);
    });
}

/**************************************************************
*                               person                        *
***************************************************************/
void CamGuardService::handle_add_person(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return PersonManagement::AddPerson(strInput, strResult);
    });
}

void CamGuardService::handle_delete_person(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return PersonManagement::DeletePerson(strInput, strResult);
    });
}

void CamGuardService::handle_update_person(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return PersonManagement::UpdatePerson(strInput, strResult);
    });
}

void CamGuardService::handle_query_person_list(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return PersonManagement::QueryPersonList(strInput, strResult);
    });
}

/**************************************************************
*                               camera                        *
***************************************************************/
void CamGuardService::handle_add_camera(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return CameraManagement::AddCamera(strInput, strResult);
    });
}

void CamGuardService::handle_delete_camera(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return CameraManagement::DeleteCamera(strInput, strResult);
    });
}

void CamGuardService::handle_update_camera(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return CameraManagement::UpdateCamera(strInput, strResult);
    });
}

void CamGuardService::handle_query_camera_list(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return CameraManagement::QueryCameraList(strInput, strResult);
    });
}

void CamGuardService::handle_search_camera(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return CameraManagement::SearchCamera(strInput, strResult);
    });
}

void CamGuardService::handle_query_camera_rtsp(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return CameraManagement::QueryCameraRtsp(strInput, strResult);
    });
}

/**************************************************************
*                               task                          *
***************************************************************/
void CamGuardService::handle_add_task(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return TaskManagement::AddTask(strInput, strResult);
    });
}

void CamGuardService::handle_delete_task(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return TaskManagement::DeleteTask(strInput, strResult);
    });
}

void CamGuardService::handle_update_task(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return TaskManagement::UpdateTask(strInput, strResult);
    });
}

void CamGuardService::handle_query_task_list(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return TaskManagement::QueryTaskList(strInput, strResult);
    });
}

void CamGuardService::handle_start_task(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return TaskManagement::StartTask(strInput, strResult);
    });
}

void CamGuardService::handle_stop_task(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return TaskManagement::StopTask(strInput, strResult);
    });
}

void CamGuardService::handle_compare1v1(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return TaskManagement::Compare1V1(strInput, strResult);
    });
}

/**************************************************************
*                               config                        *
***************************************************************/
void CamGuardService::get_system_config(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return SystemConfigManagement::GetSystemConfig(strInput, strResult);
    });
}

void CamGuardService::update_system_config(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return SystemConfigManagement::UpdateSystemConfig(strInput, strResult);
    });
}

void CamGuardService::query_task_capture_record(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return HistoryManagement::QueryTaskCaptureRecord(strInput, strResult);
    });
}

void CamGuardService::delete_task_capture_record(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    call_wrap(pConn, pEvtData, [](const std::string& strInput, std::string& strResult) {
        return HistoryManagement::DeleteTaskCaptureRecord(strInput, strResult);
    });
}

void CamGuardService::call_wrap(mg_connection *pConn, void* pData, INVOKE_FUNC func)
{
    http_message* pHttpMsg = static_cast<http_message*>(pData);
    VERIFY_EXPR_RETURN_VOID((pHttpMsg != nullptr));
    bool bKeepAlive = false;
    CamGuardService::Instance().check_keep_alive(pHttpMsg, bKeepAlive);
    std::string strMsg = std::string(pHttpMsg->body.p, pHttpMsg->body.len);
    std::string strRst;
    int nCode = func(strMsg, strRst);
    do_response(pConn, bKeepAlive, nCode, strRst);
}

void CamGuardService::run()
{
    while (!m_bExit.load())
    {
        mg_mgr_poll(&m_Mgr, 500); // ms
    }
    mg_mgr_free(&m_Mgr);

}

void CamGuardService::init_handle(mg_connection* pConn)
{
    for (const auto& item : m_mapFunc)
    {
        auto strUri = item.first;
        auto hFunc = item.second;
        mg_register_http_endpoint(pConn, strUri.c_str(), MG_CB(hFunc, NULL));
    }
}

void CamGuardService::check_keep_alive(struct http_message *pHttpMsg, bool& bKeepAlive)
{
    auto pHeader = mg_get_http_header(pHttpMsg, "Connection");
    if (nullptr == pHeader)
    {
        /*if (0 == mg_vcasecmp(&pHttpMsg->proto, "HTTP/1.1"))
            bKeepAlive = true;*/
    }
    else
    {
        if (0 == mg_vcasecmp(pHeader, "keep-alive"))
            bKeepAlive = true;
    }
}

void CamGuardService::do_response(mg_connection *pConn, 
    bool bKeepAlive, 
    int nErr, 
    const std::string& strRst)
{
    std::string strInvokeResult = strRst;
    if (CG_OK != nErr || strRst.empty())
        strInvokeResult = ErrorMsgManagement::Instance().GetErrorMsg(nErr, strRst);
    mg_printf(pConn,
        "HTTP/1.1 200 OK\r\n"
        "Date:%s\r\n"
        "Content-Type: Application/json\r\n"
        "Transfer-Encoding:chunked\r\n"
        "Connection: %s\r\n"
        "Access-Control-Allow-Orign:*\r\n\r\n",
        Basic::Time::GetCurrentSystemTime().c_str(),
        bKeepAlive ? "keep-alive" : "close");
    mg_printf_http_chunk(pConn, strInvokeResult.c_str());
    mg_printf_http_chunk(pConn, "", 0);
    if (!bKeepAlive)
        pConn->flags |= MG_F_SEND_AND_CLOSE;
}
