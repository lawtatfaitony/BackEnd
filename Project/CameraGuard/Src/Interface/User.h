/**************************************************************
* @brief:       user management
* @date:         20200124
* @update:
* @auth:         Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <string>



namespace Service
{
    struct UserInfo
    {
        std::string strSession;
        int nUserId = 0;
        std::string strName;
        std::string strOldPassword;
        std::string strPassword;
        std::string strRemark;
        std::string strCreateTime;
    };

    class UserManagement
    {
    public:
        static int Login(const std::string& strMsg, std::string& strRst);
        static int RegisterUser(const std::string& strMsg, std::string& strRst);
        static int UpdateUser(const std::string& strMsg, std::string& strRst);
        static int DeleteUser(const std::string& strMsg, std::string& strRst);

    private:
        static int login(const UserInfo& infoUser, std::string& strRst);
        static int register_user(const UserInfo& infoUser);
        static int update_user(const UserInfo& infoUser, const std::string& strUpdateSql);
        static int delete_user(int nUserId);
        static void generate_result(int nUserId, std::string& strRst);
    };
}
