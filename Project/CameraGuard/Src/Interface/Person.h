/**************************************************************
* @brief:       person management
* @date:         20200127
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
    struct PersonInfo
    {
        std::string strSession;
        int64_t nPersonId = 0;
        std::string strName;
        std::string strPiny;
        int nLibId = 0;
        int64_t nPicId;
        std::string strLibName;
        int nSex = 0;
        std::string strPicUrl;
        std::string strCardNo;
        std::string strPhone;
        int nCategory = 0;
        std::string strRemark;
        std::string strCreateTime;
    };

    struct PersonQueryCond
    {
        int nPage = 1;
        int nPageSize = 10;
        std::string strSession;
        std::string strName;
        std::string strCardNo;
        int nCategory = 0;
        std::string strPhone;
    };
    typedef std::list<PersonInfo> PersonList;

    class PersonManagement
    {
    public:
        static int AddPerson(const std::string& strMsg, std::string& strRst);
        static int DeletePerson(const std::string& strMsg, std::string& strRst);
        static int UpdatePerson(const std::string& strMsg, std::string& strRst);
        static int QueryPersonList(const std::string& strMsg, std::string& strRst);

    private:
        static int check_person_exist(int nLibId, const std::string& strCardNo, int64_t nPersonId = 0);
        static int add_person(int nLibId, const std::string& strPicUrl, int64_t& nPicId);
        static int add_person(const PersonInfo& infoPerson);
        static int delete_person(int nLibId, int64_t nPersonId);
        static int update_person(const PersonInfo& infoPerson, const std::string& strUpdateSql);
        static int query_person_list(const PersonQueryCond& conn, 
            PersonList& listPerson, 
            int64_t& nCnt);

    };
};