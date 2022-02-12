/**************************************************************
* @brief:       camera management
* @date:         20200126
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
    struct CameraInfo
    {
        std::string strSession;
        int nCameraId = 0;
        std::string strIp;
        std::string strName;
        std::string strUser;
        std::string strPassword;
        int nType = 0;
        int nOnline = 0;
        std::string strRtsp;
        std::string strRemark;
        std::string strCreateTime;
    };
    struct SearchCamera
    {
        std::string strIp;
        std::string strManufacturer;
    };

    struct CameraQueryCond
    {
        int nPage = 1;
        int nPageSize = 10;
        std::string strSession;
        std::string strName;
        int nType = 0;
    };
    typedef std::list<CameraInfo> CameraList;
    typedef std::list<SearchCamera> SearchCameraList;

    class CameraManagement
    {
    public:
        static int AddCamera(const std::string& strMsg, std::string& strRst);
        static int DeleteCamera(const std::string& strMsg, std::string& strRst);
        static int UpdateCamera(const std::string& strMsg, std::string& strRst);
        static int QueryCameraList(const std::string& strMsg, std::string& strRst);
        // search cameras of local-network
        static int SearchCamera(const std::string& strMsg, std::string& strRst);
        // get rtsp of specified camera
        static int QueryCameraRtsp(const std::string& strMsg, std::string& strRst);

    private:
        static int add_camera(const CameraInfo& infoCamera);
        static int delete_camera(int nCameraId);
        static int update_camera(const CameraInfo& infoCamera, const std::string& strUpdateSql);
        static int parse_camera_rtsp(const std::string& strRtsp, CameraInfo& info);
        static int query_camera_list(const CameraQueryCond& conn, 
            CameraList& listCamera, 
            int& nCnt);
        static int search_camera(SearchCameraList& listCamera, int& nCnt);

    };
};