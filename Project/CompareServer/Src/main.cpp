#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>
#include <easylogging/LoggingHelper.h>
#include <easylogging/easylogging++.h>
#include "Config/GlobalConfig.h"
#include "Server/ServerImpl.h"
#include "Library/LibraryManagement.h"



bool Init();
void Uninit();
void SetupCatchCtrl();
void DoCatchCtrl(int nSignal);
class CompareServer :public Ice::Application
{
public:
    virtual int run(int argc, char *[])
    {
        try
        {
            Ice::ObjectAdapterPtr pAdapter = communicator()->createObjectAdapter("CompareServer");
#ifdef ICE_CPP11_MAPPING
            auto servant(std::make_shared<iceServerImpl>());
#else
            auto pServant(new CompareServerImpl());
#endif // ICE_CPP11_MAPPING
            pAdapter->add(pServant, Ice::stringToIdentity("Compare"));
            pAdapter->activate();

            communicator()->waitForShutdown();
        }
        catch (const Ice::Exception& e)
        {
            std::cout << "Init exception,error:" << e.what();
        }
        //do clean
        Uninit();
        std::cout << "exit\n";
        return 0;
    }
};
// main
int main(int argc, char *argv[])
{
    if (!Init()) return -1;
    CompareServer server;
    return server.main(argc, argv, Config::GetServerConfig().c_str());
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
    if (!LibraryManagement::Instance().Init())
    {
        LOG(INFO) << "Init server failed";
        return false;
    }
    LOG(INFO) << "Init successs";
    return true;
}

void Uninit()
{
    LibraryManagement::Instance().Stop();
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
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    std::cout << "exit";
}
