/**************************************************************
* @brief:       history management
* @date:         20200725
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
    struct TaskCaptueryQueryCond
    {
        int nPage = 1;
        int nPageSize = 10;
        std::string strSession;
        int nTaskId = 0;
        int nCameraId = 0;
        int nLibId = 0;
        std::string strPersonName;
        std::string strCardNo;
        int nClassify = -1;
        float nSimilarity = 0.0;
    };
    struct TaskCaptureRecord
    {
        int64_t nRecordId;
        int nTaskId = 0;
        std::string strTaskName;
        int nCameraId = 0;
        std::string strCameraName;
        int64_t nPersonId = 0;
        std::string strPersonName;
        int nSex = 0;
        int nCategory = 0;
        std::string strCardNo;
        int nClassify = 0;
        int nLibId = 0;
        std::string strLibName;
        float nSimilarity = 0.0;
        std::string strPicPath;
        std::string strCapturePath;
        std::string strCaptureTime;
        std::string strCreateTime;
    };
    typedef std::list<TaskCaptureRecord> TaskCaptureList;


    class HistoryManagement
    {
    public:
        static int QueryTaskCaptureRecord(const std::string& strCond, std::string& strResult);
        static int DeleteTaskCaptureRecord(const std::string& strCond, std::string& strResult);


    private:
        static int query_task_capture_record(const TaskCaptueryQueryCond& condQuery,
            TaskCaptureList& listRecord,
            int64_t& nCnt);
        static int delete_task_capture_record(int64_t nRecordId);

    };
}