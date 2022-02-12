#include <iostream>
#include "HttpServer.h"
#include <easylogging/easylogging++.h>
#include <easylogging/LoggingHelper.h>
#include "Config/GlobalConfig.h"
#include "File/FileManager.h"



bool Init();
void Uninit();
void SetupCatchCtrl();
void DoCatchCtrl(int nSignal);
int main()
{
    if (!Init())
    {
        LOG(ERROR) << "Init failed";
        return -1;
    }
    std::cout << "Press any key to exit";
    getchar();
    Uninit();
	return 0;
}

bool Init()
{
    if (!LoggingHelper::InitLogging(GetLogConfig()))
        return false;
    if (!GlobalConfig::Instance().Init())
        return false;
    SetupCatchCtrl();
    FileManagement::Instance().Init();
    if (!HttpServer::Instance().Start())
        return false;
    return true;
}

void Uninit()
{
    HttpServer::Instance().Stop();
    FileManagement::Instance().Uninit();
}

void SetupCatchCtrl()
{
    signal(SIGINT, DoCatchCtrl); //ctrl+c
    signal(SIGTERM, DoCatchCtrl); //ctrl+b
    signal(SIGBREAK, DoCatchCtrl); //x
    signal(SIGABRT, DoCatchCtrl);
}

void DoCatchCtrl(int nSignal)
{
    std::cout << nSignal << std::endl;
    Uninit();
}
