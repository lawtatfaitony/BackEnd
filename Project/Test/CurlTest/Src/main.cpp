#include <iostream>
#include <sstream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "LibcurlHelper.h"
#include <Basic/Conversion.h>
#include <Basic/Base64.h>


std::string LoadFile(const std::string& strFile)
{
    std::fstream fin(strFile.c_str(), std::ios::in | std::ios::binary);
    if (fin.is_open())
    {
        fin.seekg(0, std::ios::end);
        int64_t nFileSize = fin.tellp();
        fin.seekg(0, std::ios::beg);
        std::vector<char>vecData(nFileSize + 1, 0);
        fin.read(vecData.data(), nFileSize);
        fin.close();
        std::string strData;
        strData.resize(vecData.size());
        memcpy(&strData[0], vecData.data(), vecData.size());
        return Basic::Base64::Encode((unsigned char*)strData.c_str(), strData.size());
    }
    return "";
}

void DownloadFile()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    // download file
    //para.strUrl = "https://codeload.github.com/curl/curl/zip/master";
    para.strUrl = "http://127.0.0.1:10008/download/2019-12-13/296b1d7b-6a26-4fde-9ac600604e659700.zip";
    //para.strUrl = "http://127.0.0.1:10008/download/2019-12-12/3c4a4890-93b1-4641-834afe59ce57cd24.jpg";
    //para.strUrl = "https://192.168.2.66/svn/%E7%A5%9E%E7%9B%BE/SDBusiness/branches/WiseEyes_v3_saida/projects/SaaS/src/interface/service/heatmap.cpp";
    ///*para.strUser = "tredy6t";
    //para.strPassword = "wite_chen0";*/
    /*para.strUser = "fei.chen";
    para.strPassword = "fei.chen.00";*/
    //std::string strRstFile = "test.zip";
    clientHttp.DownloadFile(para, strResponse);
}

void DownloadBigFile()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    // download file
    std::string strUrl = "https://127.0.0.1/svn/QQ_Project/ChatServer/ChatServer/SocketServer.cpp";
    para.strUrl = strUrl;
    para.strUser = "Wite";
    para.strPassword = "wite_chen";
    std::string strRstFile = "server.txt";
    std::string strPath = "";
    clientHttp.DownloadBigFile(para, strPath, strRstFile);
}

void LocalTest()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    std::string strUrl;
    int nCode = 0;
    //para.strUrl = "";
    para.strUrl = "https://codeload.github.com/curl/curl/zip/master";
    //para.strUrl = "https://download.hello.com/files/hello/hello.zip";
    //para.strUrl = "https://192.168.2.66/svn/%E7%A5%9E%E7%9B%BE/SDBusiness/branches/WiseEyes_v3_saida/projects/SaaS/src/interface/service/heatmap.cpp";
    //para.strUrl = "ed2k://|file|cn_windows_10_multiple_editions_x64_dvd_6848463.iso|4303300608|94FD861E82458005A9CA8E617379856A|/";
    std::string strPath = "";
    /*para.strUser = "fei.chen";
    para.strPassword = "fei.chen.00";*/
    /*para.strUser = "1058778041@qq.com";
    para.strPassword = "wite_chen0";*/
    para.strUser = "1058778041@qq.com";
    para.strPassword = "wite_chen0";

    //nCode = clientHttp.DownloadFile(para, strFile, strResponse);

    nCode = clientHttp.DownloadBigFile(para);
}

void DownloadMsiFile()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    std::string strUrl;
    int nCode = 0;
    //std::string strFile = "test.txt";
    std::string strFile = "test.msi";
    para.strUrl = "http://slproweb.com/download/Win32OpenSSL_Light-1_1_1d.msi";
    //para.strUrl = "ed2k://|file|cn_windows_10_multiple_editions_x64_dvd_6848463.iso|4303300608|94FD861E82458005A9CA8E617379856A|/";
    std::string strPath = "";
    /*para.strUser = "1058778041@qq.com";
    para.strPassword = "wite_chen";*/
    nCode = clientHttp.DownloadBigFile(para, strPath, strFile, 1);

}

void UploadFile()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    para.strUrl = "http://127.0.0.1:10011/upload";
    //std::string strFile = "1.jpg";
    std::string strFile = "test.jpg";
    //std::string strFile = "2.jpg";
    //std::string strFile = "3.jpg";
    //std::string strFile = "curlDemo.zip";
    int code = clientHttp.UploadFile(para, strFile, strResponse);

    std::cout << "code:" << code << "result:" << strResponse << std::endl;
}

void UploadFileWithData()
{
    LibcurlHelper clientHttp;
    std::string strResponse;
    std::string strUrl = "http://127.0.0.1:10011/upload/";

    std::string strFile = "test.jpg";
    std::string strFileData = LoadFile(strFile);
    HttpPara para;
    para.strUrl = strUrl;

    auto extract_url_path = [=](const std::string& strInput, std::string& strResult)
    {
        if (strInput.empty())return;
        strResult = "";
        int nPos = strInput.find_last_of(":");
        if (nPos != std::string::npos)
        {
            nPos = strInput.find("/", nPos);
            if (nPos != std::string::npos)
            {
                strResult = strInput.substr(nPos);
            }
        }
    };
    int code = clientHttp.Post(para, strFileData, strResponse);
    std::string strPicPath;
    extract_url_path(strResponse, strPicPath);
    std::cout << "code:" << code << "result:" << strResponse << std::endl;
}

void InvokeHttp()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    para.strUrl = "http://127.0.0.1:10009/Login";
    std::string strFile = "{\"username\":\"admin\",\"password\":\"0192023a7bbd73250516f069df18b500\"}";
    //std::string strFile = "curlDemo.zip";
    int code = clientHttp.Post(para, strFile, strResponse);

    std::cout << "code:" << code << "\nresult:" << strResponse << std::endl;
}

std::string WrapCameraInfo()
{
    std::string strName = "摄像头";
    std::string strNameUtf8 = Basic::Gbk2Utf8(strName);
    std::string strRtsp = "rtsp://admin:admin123@172.0.0.1:1935/live/test";
    rapidjson::Document doc(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();
    /*
        {
	    "session": "session",
	    "name": "摄像头8",
	    "rtsp": "rtsp://admin:admin123@172.0.0.1:1935/live/test",
	    "type": 0,
	    "remark": ""
        }
    */
    doc.AddMember("session", rapidjson::Value("", typeAllocate), typeAllocate);
    doc.AddMember("name", rapidjson::Value(strNameUtf8.c_str(), typeAllocate), typeAllocate);
    doc.AddMember("rtsp", rapidjson::Value(strRtsp.c_str(), typeAllocate), typeAllocate);
    doc.AddMember("type", 0, typeAllocate);
    doc.AddMember("remark", rapidjson::Value("", typeAllocate), typeAllocate);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
    doc.Accept(writer);
   return buffer.GetString();
}

std::string WrapPersonInfo()
{
    std::string strName = "person3";
    std::string strNameUtf8 = Basic::Gbk2Utf8(strName);
    rapidjson::Document doc(rapidjson::kObjectType);
    rapidjson::Document::AllocatorType &typeAllocate = doc.GetAllocator();

    doc.AddMember("lib_id", 1, typeAllocate);
    doc.AddMember("session", rapidjson::Value("", typeAllocate), typeAllocate);
    doc.AddMember("name", rapidjson::Value(strNameUtf8.c_str(), typeAllocate), typeAllocate);
    std::string strPicUrl = "download/data/2020-06-14/1592124172/1";

    doc.AddMember("pic_url", rapidjson::Value(strPicUrl.c_str(), typeAllocate), typeAllocate);
    std::string card_no = "123";
    //doc.AddMember("card_no", rapidjson::Value(card_no.c_str(), typeAllocate), typeAllocate);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

void AddLibrary()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    para.strUrl = "http://127.0.0.1:10009/AddLibrary";
    std::string strMsg = "{\"session\":\"\",\"name\":\"test0614\",\"type\":0}";
    int code = clientHttp.Post(para, strMsg, strResponse);

    std::cout << "code:" << code << "\nresult:" << strResponse << std::endl;
}

void AddCamera()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    para.strUrl = "http://127.0.0.1:10009/AddCamera";
    std::string strData = WrapCameraInfo();
    int code = clientHttp.Post(para, strData, strResponse);

    std::cout << "code:" << code << "\nresult:" << strResponse << std::endl;
}

void InvokeXWwwFormUrlEncoded()
{
    HttpPara para;
    std::string strResponse;
    para.vecHeaders.push_back("Content-Type: x-www-form-urlencoded");
    LibcurlHelper clientHttp;
    para.strUrl = "http://127.0.0.1:10009/Login";
    std::string strData;
    {
        // 如果是base64数据，需要url encode
        std::stringstream ss;
        ss << "data=" << "123" << "&";
        ss << "name=" << "234" << "&";
        strData = ss.str();
    }
    //std::string strData = "data=123&name=234";
    int nErr = clientHttp.Post(para, strData, strResponse);
    std::cout << "code:" << nErr << ",msg:" << strResponse;
}

void AddPerson()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    para.strUrl = "http://127.0.0.1:10009/AddPerson";
    std::string strData = WrapPersonInfo();
    int code = clientHttp.Post(para, strData, strResponse);

    std::cout << "code:" << code << "\nresult:" << strResponse << std::endl;
}

void DownloadPic()
{
    HttpPara para;
    LibcurlHelper clientHttp;
    para.strUrl = "http://127.0.0.1:10009/download/data/2020-03-17/1584453382/2";
    std::string strResponse;
    int code = clientHttp.Get(para, strResponse);

    std::cout << "code:" << code << "\nresult:" << strResponse << std::endl;
}

void StartTask()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    para.strUrl = "http://127.0.0.1:10009/StartTask";
    std::string strMsg = "{\"session\":\"\",\"task_id\":1}";
    int code = clientHttp.Post(para, strMsg, strResponse);

    std::cout << "code:" << code << "\nresult:" << strResponse << std::endl;
}

void StopTask()
{
    HttpPara para;
    std::string strResponse;
    LibcurlHelper clientHttp;
    para.strUrl = "http://127.0.0.1:10009/StopTask";
    std::string strMsg = "{\"session\":\"\",\"task_id\":1}";
    int code = clientHttp.Post(para, strMsg, strResponse);

    std::cout << "code:" << code << "\nresult:" << strResponse << std::endl;
}

int main()
{
    //InvokeXWwwFormUrlEncoded();
    //AddCamera();
    //InvokeHttp();
    //DownloadFile();
    //DownloadBigFile();
    //LocalTest();

    //DownloadMsiFile();
    //DownloadPic();
    UploadFile();
    //UploadFileWithData();
    // add person
    //AddLibrary();
    //AddPerson();

    //DownloadFile();
    //UploadFile1();
    //StartTask();
    //StopTask();
    getchar();
    return 0;
}