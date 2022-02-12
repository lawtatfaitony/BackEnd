//在windows平台使用时，需要使用宏“WIN32_LEAN_AND_MEAN”与“ELPP_WINSOCK2”来解决SOCKET定义冲突问题
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
            //单独设定logger
            el::Loggers::reconfigureLogger("default", conf);
            //设定所有的looger配置
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