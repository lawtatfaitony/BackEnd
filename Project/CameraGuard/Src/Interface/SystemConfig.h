/**************************************************************
* @brief:       system config management
* @date:         20200128
* @update:
* @auth:         Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
#include <list>


namespace Service
{
    struct SystemConfigInfo
    {
        std::string strSession;
        std::string strName;
        std::string strConfig;
    };

    class SystemConfigManagement
    {
    public:
        static int UpdateSystemConfig(const std::string& strMsg, std::string& strRst);
        static int GetSystemConfig(const std::string& strMsg, std::string& strRst);
        static bool CheckConfigExist(const std::string& strName, std::string& strConfig);
        static int InitSystemConfig(const std::string& strName, std::string& strConfig);

    private:
        static int update_system_config(const SystemConfigInfo& infoConfig);
        static int get_system_config(const std::string& strName, SystemConfigInfo& infoConfig);

    };
};

