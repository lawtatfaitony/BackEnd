#include <iostream>
#include <easylogging/LoggingHelper.h>
#include <easylogging/easylogging++.h>
#include "Config/GlobalConfig.h"
#include "Server/ServerImpl.h"
#include "Client/CompareClient.h"



bool Init();
void Uninit();
void SetupCatchCtrl();
void DoCatchCtrl(int nSignal);
class TaskServer :public Ice::Application
{
public:
    virtual int run(int, char *[])
    {
        try
        {
            Ice::ObjectAdapterPtr pAdapter = communicator()->createObjectAdapter("TaskServer");
#ifdef ICE_CPP11_MAPPING
            auto servant(std::make_shared<iceServerImpl>());
#else
            auto pServant(new TaskServerImpl());
#endif // ICE_CPP11_MAPPING
            pAdapter->add(pServant, Ice::stringToIdentity("Task"));
            pAdapter->activate();

            communicator()->waitForShutdown();
        }
        catch (const Ice::Exception& e)
        {
            std::cout << "Init exception,error:" << e.what();
        }
        return 0;
    }
};

// main
int main(int argc, char *argv[])
{
    if (!Init()) return -1;
    TaskServer server;
    server.main(argc, argv, Config::GetServerConfig().c_str());
    Uninit();

    return 0;
}

bool Init()
{
    if (!LoggingHelper::InitLogging(Config::GetLogConfig()))
    {
        LOG(INFO) << "Init logging failed";
        return false;
    }
    if (!GlobalConfig::Instance().Init())
    {
        LOG(INFO) << "Init global config failed";
        return false;
    }
    SetupCatchCtrl();
    Client::CompareClient::Instance().Start();
    LOG(INFO) << "Init successs";
    return true;
}

void Uninit()
{
    Client::CompareClient::Instance().Stop();
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