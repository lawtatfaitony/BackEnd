#include <iostream>
#include <easylogging/LoggingHelper.h>
#include <easylogging/easylogging++.h>
#include "config/GlobalConfig.h"
#include "Server.h"



bool Init();
void Uninit();

// main
int main()
{
    Init();
    getchar();
    Uninit();

    return 0;
}

bool Init()
{
    if (!LoggingHelper::InitLogging(Config::GetLogConfig()))
    {
        printf("Init logging failed\n");
        return false;
    }
    if (!GlobalConfig::Instance().Init())
    {
        printf("Init global config failed\n");
        return false;
    }
    if (!Compare::CompareServer::Instance().Init())
    {
        printf("Init server failed\n");
        return false;
    }
    LOG(INFO) << "Init successs";
    return true;
}

void Uninit()
{
    Compare::CompareServer::Instance().Uninit();
}