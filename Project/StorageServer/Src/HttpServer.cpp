#include "HttpServer.h"
#include <fstream>
#include <easylogging/easylogging++.h>
#include <Macro.h>
#include <Basic/Time.h>
#include <Basic/CrossPlat.h>
#include <Basic/Guuid.h>
#include <Basic/File.h>
#include <Basic/Base64.h>
#include "Config/GlobalConfig.h"
#include "File/FileManager.h"


static std::string kDownloadPrefix = "/download";
std::map<int, std::string> g_mapErrorInfo
{
    { kResponseCodeOk, "Interactive success"},
    { kResponseCodeNoData, "Can't load file" },
    { kResponseCodeInvalidPara, "Invalid para of filename" },
};

std::map<std::string, mg_event_handler_t> HttpServer::m_mapFunc
{
    { "/upload", HttpServer::handle_upload },
    { "/download", handle_download }
};
/***************************************************************************
*                          response http request                           *
****************************************************************************/
void HttpServer::event_handle(mg_connection *pConn, int nEvtType, void *pEvtData)
{
    switch (nEvtType)
    {
    case MG_EV_HTTP_REQUEST://http
    {
        http_message *pHttpReq = (http_message*)pEvtData;
        http_event_handle(pConn, pHttpReq);
        break;
    }
    //websocket
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
    case MG_EV_WEBSOCKET_FRAME:
    case MG_EV_CLOSE:
    {
        websocket_message *pWebMsg = (websocket_message*)pEvtData;
        websocket_event_handle(pConn, nEvtType, pWebMsg);
        break;
    }
    }
}

void HttpServer::handle_upload(struct mg_connection *pConn, int ev, void *pData)
{
    struct FileWriterData *pFileData = (struct FileWriterData *) pConn->user_data;
    struct mg_http_multipart_part *pMultiPart = (struct mg_http_multipart_part *)pData;
    switch (ev)
    {
    case MG_EV_HTTP_PART_BEGIN:
    {
        if (nullptr == pFileData)
        {
            VERIFY_EXPR_RETURN_VOID(send_and_close(pConn->flags));
            pFileData = (FileWriterData*)calloc(1, sizeof(FileWriterData));
            pFileData->nDataLen = 0;
            pConn->user_data = (void *)pFileData;
        }
        break;
    }
    case MG_EV_HTTP_PART_DATA: {
        VERIFY_EXPR_RETURN_VOID(send_and_close(pConn->flags));
        pFileData->strFileData += std::string(pMultiPart->data.p, pMultiPart->data.len);
        pFileData->nDataLen += pMultiPart->data.len;
        break;
    }
    case MG_EV_HTTP_PART_END: {
        VERIFY_EXPR_RETURN_VOID(send_and_close(pConn->flags));
        std::string strFile  = FileManagement::Instance().SaveFile(pFileData->strFileData);
        pConn->flags |= MG_F_SEND_AND_CLOSE;
        std::string strStoragePath = CONFIG_HTTP_SERVER.strServerInfo + strFile;
        mg_printf(pConn,
            "HTTP/1.1 200 OK\r\n"
            "Date:%s\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n\r\n"
            "%s", Basic::Time::GetCurrentSystemTime().c_str(), strStoragePath.c_str());
        free(pFileData);
        pConn->user_data = NULL;
        break;
    }
    case MG_EV_HTTP_REQUEST: {
        LOG(INFO) << "Http request";
        http_message *pHttpReq = (http_message*)pData;
        std::string strBase64 = std::string(pHttpReq->body.p, pHttpReq->body.len);
        std::string strFile = FileManagement::Instance().SaveFile(Basic::Base64::Decode(strBase64));
        // forget it, it need much memory
        //HttpServer::Instance().push_file(strFile, pFileData->strFileData);
        pConn->flags |= MG_F_SEND_AND_CLOSE;
        std::string strStoragePath = CONFIG_HTTP_SERVER.strServerInfo + strFile;
        mg_printf(pConn,
            "HTTP/1.1 200 OK\r\n"
            "Date:%s\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n\r\n"
            "%s", Basic::Time::GetCurrentSystemTime().c_str(), strStoragePath.c_str());
        break;
    }
    }
}

void HttpServer::handle_download(struct mg_connection *pConn, int ev, void *pData)
{
    http_message *pHttpMsg = (http_message*)pData;
    if (nullptr == pHttpMsg)return;
    std::string strRequstStr = std::string(pHttpMsg->message.p, pHttpMsg->message.len);

    std::string strUrl = std::string(pHttpMsg->uri.p, pHttpMsg->uri.len);
    std::string strBody = std::string(pHttpMsg->body.p, pHttpMsg->body.len);
    if (strBody.empty())
    {
        int nPos = strUrl.find(kDownloadPrefix);
        if (nPos != std::string::npos)
        {
            std::string strPath = strUrl.substr(nPos + kDownloadPrefix.length() + 1);
            FileInfo infoFile;
            FileManagement::Instance().ReadFile(strPath, infoFile);
            int nCode = infoFile.strData.empty() ? kResponseCodeNoData : kResponseCodeOk;
            HttpServer::Instance().do_response(pConn, nCode, infoFile.strData);
        }
    }
    else
    {
        char szFile[kMaxPath] = { 0 };
        // get variable filename from http url
        if (0 >= mg_get_http_var(&pHttpMsg->body, "filename", szFile, sizeof(szFile)))
        {
            HttpServer::Instance().do_response(pConn, kResponseCodeInvalidPara);
        }
        else
        {
            FileInfo infoFile;
            FileManagement::Instance().ReadFile(szFile, infoFile);
            int nCode = infoFile.strData.empty() ? kResponseCodeNoData : kResponseCodeOk;
            HttpServer::Instance().do_response(pConn, nCode, infoFile.strData);
        }
    }
}

/***************************************************************************
*                           member function                                *
****************************************************************************/
HttpServer::HttpServer()
    :m_bExit(false)
    , m_pConn(nullptr)
{
}

HttpServer& HttpServer::Instance()
{
    static HttpServer g_Instance;
    return g_Instance;
}

HttpServer::~HttpServer()
{
    Stop();
}

bool HttpServer::Start()
{
    auto nPort = std::to_string(CONFIG_HTTP_SERVER.port);
    mg_mgr_init(&m_httpMgr, NULL);
    m_pConn = mg_bind(&m_httpMgr, nPort.c_str(), event_handle);
    if (nullptr == m_pConn)
    {
        LOG(ERROR) << "Http server bind port "<< nPort << " failed";
        return false;
    }
    // Serve current directory
    m_httpOptions.document_root = CONFIG_HTTP_SERVER.root.c_str();
    init_handle(m_pConn);
    // Set up HTTP server parameters
    mg_set_protocol_http_websocket(m_pConn);
    LOG(INFO) << "Start http server at port " << nPort << " success";
    m_thRun = std::thread(&HttpServer::run, this);
    return true;
}

void HttpServer::Stop()
{
    m_bExit = true;
    if (m_thRun.joinable())
        m_thRun.join();
}

void HttpServer::run()
{
    while (!m_bExit)
        mg_mgr_poll(&m_httpMgr, 500); // ms
    mg_mgr_free(&m_httpMgr);
}

void HttpServer::init_handle(mg_connection* pConn)
{
    for (const auto& item : m_mapFunc)
    {
        auto strUri = item.first;
        auto hFunc = item.second;
        mg_register_http_endpoint(pConn, strUri.c_str(), MG_CB(hFunc, NULL));
    }
}

void HttpServer::http_event_handle(mg_connection *pConn, http_message* pHttpMsg)
{
    if (nullptr == pHttpMsg)return;
    std::string strUrl = std::string(pHttpMsg->uri.p, pHttpMsg->uri.len);
    std::string strBody = std::string(pHttpMsg->body.p, pHttpMsg->body.len);
    mg_printf(
        pConn,
        "%s",
        "HTTP/1.1 501 Not Implemented\r\n" // 1.1, version of http
        "Content-Length: 0\r\n\r\n");
}

bool HttpServer::route_check(const http_message *pHttpMsg, const std::string& strRoutePrefix)
{
    bool bMatch = false;
    if (0 == mg_vcmp(&pHttpMsg->uri, strRoutePrefix.c_str()))
    {
        bMatch = true;
    }
    return bMatch;

    // TODO: check method(GET, POST, PUT, DELTE)
    //mg_vcmp(&http_msg->method, "GET");
    //mg_vcmp(&http_msg->method, "POST");
    //mg_vcmp(&http_msg->method, "PUT");
    //mg_vcmp(&http_msg->method, "DELETE");
}

/*******************************************************
*                       Websocket                      *
********************************************************/
void HttpServer::websocket_event_handle(mg_connection *pConn,
    int nEvtType, websocket_message* pWebsocketMsg)
{
    switch (nEvtType)
    {
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
    {
        printf("client websocket connected\n");
        // get the ip and port of request
        char addr[32];
        mg_sock_addr_to_str(&pConn->sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
        printf("client addr: %s\n", addr);

        break;
    }
    case MG_EV_WEBSOCKET_FRAME:
    {
        mg_str received_msg = {
            (char *)pWebsocketMsg->data, pWebsocketMsg->size
        };
        char buff[1024] = { 0 };
        strncpy_s(buff, received_msg.p, received_msg.len);

        // do sth to process request
        printf("received msg: %s\n", buff);
        break;
    }
    case MG_EV_CLOSE:
    {
        if (pConn->flags & MG_F_IS_WEBSOCKET)
        {
            printf("client websocket closed\n");
        }
        break;
    }
    }
}

bool HttpServer::send_and_close(int nTag)
{
    return MG_F_SEND_AND_CLOSE != nTag;
}

void HttpServer::do_response(mg_connection *pConn, int nErr, const std::string& strRst)
{
    std::string strResponse = strRst;
    if (strRst.empty())
        strResponse = g_mapErrorInfo[nErr];

    mg_printf(pConn,
        "HTTP/1.1 200 OK\r\n"
        "Date:%s\r\n"
        "Transfer-Encoding:chunked\r\n"
        "Connection: keep-alive\r\n"
        "Content-Length: %ld\r\n\r\n",
        Basic::Time::GetCurrentSystemTime().c_str(), strResponse.length());
    mg_send_http_chunk(pConn, strResponse.c_str(), strResponse.length());
    mg_send_http_chunk(pConn, "", 0);
}
