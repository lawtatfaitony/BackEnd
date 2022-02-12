#include <iostream>
#include <signal.h>
#include <easylogging/LoggingHelper.h>
#include <easylogging/easylogging++.h>
#include <Macro.h>
#include "Config/GlobalConfig.h"
#include "Servce.h"
#include "ErrorInfo/ErrorMessage.h"
#include "client/CompareClient.h"
#include "client/TaskClient.h"
#include "cache/camera/CameraState.h"
#include "cache/camera/DiscoverCamera.h"
#include "Cache/ConnectionPool/DatabaseConnPool.h"
#include "Task/Process/TaskResult.h"
#include "TTS/TTSManager.h"


bool Init();
void Uninit();
void SetupCatchCtrl();
void DoCatchCtrl(int nSignal);
int main()
{
    // init the running config
    if (!Init())
    {
        Uninit();
        return -1;
    }
    LOG(INFO) << "Init successs";

    // wait for input
    getchar();

    // uninit
    Uninit();
    return 0;
}

bool Init()
{
    VERIFY_EXPR_RETURN(LoggingHelper::InitLogging(Config::GetLogConfig()), false);
    VERIFY_EXPR_RETURN(GlobalConfig::Instance().Init(), false);
    SetupCatchCtrl();
    VERIFY_EXPR_RETURN(DatabaseConnPool::Instance().Init(), false);
    TTSManagement::Instance().Init();
    TaskResult::Instance().Init();
    Service::ErrorMsgManagement::Instance();
    VERIFY_EXPR_RETURN(Service::CamGuardService::Instance().Start(), false);
    Cache::CameraStateManagement::Instance().Start();
    Cache::DiscoveryCamera::Instance().Start();
    Client::CompareClient::Instance().Start();
    Client::TaskClient::Instance().Start();
    return true;
}

void Uninit()
{
    Service::CamGuardService::Instance().Stop();
    Client::CompareClient::Instance().Stop();
    Client::TaskClient::Instance().Stop();
    Cache::CameraStateManagement::Instance().Stop();
    Cache::DiscoveryCamera::Instance().Stop();
    TTSManagement::Instance().Uninit();
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
