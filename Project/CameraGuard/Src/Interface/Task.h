/**************************************************************
* @brief:       task management
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
    struct CameraStateInfo
    {
        int nCameraId;
        std::string strName;
        int nState = 0;
    };
    struct TaskLibraryInfo
    {
        int nLibId = 0;
        std::string strName;
    };

    struct TaskInfo
    {
        std::string strSession;
        int nTaskId = 0;
        std::string strName;
        int nType = 0;
        std::string strCameraList1;
        std::string strCameraList2;
        std::list<CameraStateInfo> listCamera1;
        std::list<CameraStateInfo> listCamera2;
        std::string strLibList;
        std::list<TaskLibraryInfo> listLib;
        int nInterval = 0;
        double fThreshold = 0.8;
        int nState = 0;
        std::string strPlan;
        std::string strRemark;
        std::string strCreateTime;
    };

    struct TaskQueryCond
    {
        int nPage = 1;
        int nPageSize = 10;
        std::string strSession;
        std::string strName;
        int nType = 0;
        int nState = -1;
    };
    typedef std::list<TaskInfo> TaskList;

    class TaskManagement
    {
    public:
        static int AddTask(const std::string& strMsg, std::string& strRst);
        static int DeleteTask(const std::string& strMsg, std::string& strRst);
        static int UpdateTask(const std::string& strMsg, std::string& strRst);
        static int QueryTaskList(const std::string& strMsg, std::string& strRst);
        static int StartTask(const std::string& strMsg, std::string& strRst);
        static int StopTask(const std::string& strMsg, std::string& strRst);
        static int Compare1V1(const std::string& strMsg, std::string& strRst);

    private:
        static int add_task(const TaskInfo& infoTask);
        static int delete_task(int nTaskId);
        static int update_task(const TaskInfo& infoTask,
            const std::string& strUpdateSql);
        static int query_task_list(const TaskQueryCond& conn, 
            TaskList& listTask, 
            int64_t& nCnt);
        static void fill_camera_state(TaskList& listTask);
        static int check_task_running(int nTaskId);

    };
};