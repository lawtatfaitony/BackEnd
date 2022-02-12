//��windowsƽ̨ʹ��ʱ����Ҫʹ�úꡰWIN32_LEAN_AND_MEAN���롰ELPP_WINSOCK2�������SOCKET�����ͻ����
#include "LoggingHelper.h"
#include "easylogging++.h"


INITIALIZE_EASYLOGGINGPP


namespace LoggingHelper
{
    bool InitLogging(const std::string& strLogFile)
    {
        try
        {
            el::Configurations conf(strLogFile);
            //�����趨logger
            el::Loggers::reconfigureLogger("default", conf);
            //�趨���е�looger����
            el::Loggers::reconfigureAllLoggers(conf);
            el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
            el::Loggers::addFlag(el::LoggingFlag::StrictLogFileSizeCheck);
            // time-rolling
            el::Loggers::addFlag(el::LoggingFlag::StrictLogFileTimeCheck);
        }
        catch (const std::exception& e)
        {
            std::cout << "Init logger error:" << e.what();
            return false;
        }
        return true;
    }
}