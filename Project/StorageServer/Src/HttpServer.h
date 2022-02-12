#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <map>
#include <fstream>
#include <mongoose/mongoose.h>
#include <Basic/ThreadObject.h>


enum ResponseCode
{
    kResponseCodeOk,            // success
    kResponseCodeNoData,        // no data
    kResponseCodeInvalidPara    // invalid para
};

struct FileWriterData
{
    size_t nDataLen;
    std::string strFileData;
};

class HttpServer
{
    const std::string kDataDir = "data";
    static const int kSlice = 512 * 1024;
    static const int kMaxNode = 100;
    static const int kMaxPath = 260;

    HttpServer();
public:
    static HttpServer& Instance();
    ~HttpServer();
    bool Start();
    void Stop();

private:
    void run();
    void init_handle(mg_connection* pConn);
    // static function to handle http request
    static void event_handle(mg_connection *pConn, int nEvtType, void *pEvtData);
    static void handle_upload(struct mg_connection *pConn, int ev, void *pData);
    static void handle_download(struct mg_connection *pConn, int ev, void *pData);
    static void http_event_handle(mg_connection *pConn, http_message* pHttpMsg);
    static void websocket_event_handle(mg_connection *pConn, int nEvtType, websocket_message* pWebsocketMsg);

    static bool route_check(const http_message *pHttpMsg, const std::string& strRoutePrefix);
    static bool send_and_close(int nTag);

private:
    void do_response(mg_connection *pConn, int nErr, const std::string& strRst = "");

private:
    bool m_bExit;
    // std::pair<uri, func>
    static std::map<std::string, mg_event_handler_t> m_mapFunc;
    // http
    mg_mgr m_httpMgr;
    struct mg_connection *m_pConn;
    mg_serve_http_opts m_httpOptions;
    std::string m_strWebDir;
    std::thread m_thRun;

};

