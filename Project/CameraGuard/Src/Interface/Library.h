/**************************************************************
* @brief:       library management
* @date:         20200124
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
    struct LibraryInfo
    {
        std::string strSession;
        int nRecordId = 0;
        int nLibId = 0;
        std::string strName;
        int nType = 0;
        int nPersonCnt = 0;
        std::string strRemark;
        std::string strCreateTime;
    };

    struct LibraryQueryCond
    {
        int nPage = 1;
        int nPageSize = 10;
        std::string strSession;
        std::string strName;
        int nType = 0;
    };
    typedef std::list<LibraryInfo> LibraryList;


    class LibraryManagement
    {
    public:
        static int AddLibrary(const std::string& strMsg, std::string& strRst);
        static int DeleteLibrary(const std::string& strMsg, std::string& strRst);
        static int QueryLibraryList(const std::string& strMsg, std::string& strRst);

    private:
        static int check_name_exists(const std::string& strLibName);
        static int add_library(const LibraryInfo& infoLibrary);
        static int delete_library(int nLibId, size_t& nPos);
        static int query_library_list(const LibraryQueryCond& conn, 
            LibraryList& listLibrary, 
            int64_t& nCnt);

    };
}
