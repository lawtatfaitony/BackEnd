#include "TaskPara.h"
#include <soci/soci.h>
#include <easylogging/easylogging++.h>
#include <Macro.h>
#include "../Config/GlobalConfig.h"
#include "../ErrorInfo/ErrorCode.h"
#include "../Cache/ConnectionPool/DatabaseConnPool.h"


namespace soci
{
    template<>
    struct type_conversion<Task::TaskPara>
    {
        typedef values base_type;
        static void from_base(const values& v, indicator ind, Task::TaskPara& task)
        {
            task.nTaskId = v.get<int>("id");
            task.strName = v.get<std::string>("name");
            task.nType = v.get<int>("type");
            task.strLib = v.get<std::string>("lib_list");
            task.strCameraList1 = v.get<std::string>("camera_list1");
            task.strCameraList2 = v.get<std::string>("camera_list2");
            task.nInterval = v.get<int>("interval");
            task.fCompareShreshold = v.get<double>("threshold");
        }
    };
}

using namespace Task;
int TaskPara::MakeTaskPara(int nTaskId)
{
    this->nTaskId = nTaskId;
    VERIFY_CODE_RETURN(load_task_para());       // load para of task
    // load info of camera
    VERIFY_CODE_RETURN(load_camera_info(strCameraList1, mapCam1));
    VERIFY_CODE_RETURN(load_camera_info(strCameraList2, mapCam2));
    if (mapCam1.empty() && mapCam2.empty())
    {
        LOG(ERROR) << "Invalid para of camera list";
        return CG_INVALID_PARA;
    }

    return CG_OK;
}

int TaskPara::load_task_para()
{
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);
        conn << "select * from ft_task where visible=1"
            << " and id=:id;"
            , soci::into(*this)
            , soci::use(nTaskId);
        if (!conn.got_data()) return CG_NOT_EXIST;
    }
    catch (const std::exception&e)
    {
        LOG(ERROR) << "Load parameter of task " << nTaskId << " exception:" << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
	return CG_OK;
}

int TaskPara::load_camera_info(const std::string& strCameraList, std::map<int, std::string>& mapCamera)
{
    if (strCameraList.empty()) return CG_OK;
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        int camera_id = 0;
        std::string strRtsp;
        soci::statement st=(conn.prepare 
            << "select id, rtsp from ft_camera where visible = 1"
            << " and id in(" << strCameraList << ")"
            , soci::into(camera_id)
            , soci::into(strRtsp));
        st.execute();
        while (st.fetch())
            mapCamera[camera_id] = strRtsp;
    }
    catch (const std::exception&e)
    {
        LOG(ERROR) << "Load camera info of task " << nTaskId << " exception:" << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}