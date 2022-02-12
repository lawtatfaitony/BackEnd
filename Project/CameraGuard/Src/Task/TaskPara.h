/**************************************************************
* @brief:       task result handler
* @date:         20200404
* @update:
* @auth:         Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <memory>
#include <vector>
#include <map>
#include <string>

namespace Task
{
    struct TaskPara
    {
        int nTaskId = 0;
        std::string strName;
        int nType = 0;
        // library
        std::string strLib;
        std::vector<int> vecLib;
        // camera list1
        std::string strCameraList1;
        std::map<int, std::string> mapCam1;
        // camera list2
        std::string strCameraList2;
        std::map<int, std::string> mapCam2;
        float fCompareShreshold = 0.8;
        int nTopN = 1;
        int nInterval = 3;     // the timer to push the same person

        int MakeTaskPara(int nTaskId);

    private:
        int load_task_para();
        int load_camera_info(const std::string& strCameraList, std::map<int,std::string>& mapCamera);

    };

    typedef std::shared_ptr<TaskPara> TaskParaPtr;

}
