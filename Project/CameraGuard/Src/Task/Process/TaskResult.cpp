#include "TaskResult.h"
#include <easylogging/easylogging++.h>
#include <Basic/Time.h>
#include <Macro.h>
#include "../../ErrorInfo/ErrorCode.h"
#include "../../Cache/ConnectionPool/DatabaseConnPool.h"
#include "../../Config/GlobalConfig.h"


TaskResult::TaskResult()
{

}

TaskResult& TaskResult::Instance()
{
    static TaskResult g_Instance;
    return g_Instance;
}

TaskResult::~TaskResult()
{

}

void TaskResult::Init()
{
    m_poolHandleResult.Start(1, 1000);
}

void TaskResult::Unint()
{
    m_poolHandleResult.Stop();
}

void TaskResult::PushTaskResult(const Task::TaskResult& taskResult)
{
    m_poolHandleResult.Commit([=]() {
        handle_result(taskResult);
    });
}

void TaskResult::handle_result(const Task::TaskResult& taskResult)
{
    // handle the compare's result
    Task::TaskResult rstCompare = taskResult;
    VERIFY_EXPR_RETURN_VOID(!check_invalid_result(rstCompare));
    VERIFY_EXPR_RETURN_VOID(!filter_duplication(rstCompare));
    filter_stranger(rstCompare);
    VERIFY_CODE_RETURN_VOID(fill_person_info(rstCompare));
    VERIFY_CODE_RETURN_VOID(save_result(rstCompare));
}

bool TaskResult::check_invalid_result(Task::TaskResult& taskResult)
{
    // check valid result
    auto& tmpCompare = taskResult.seqPerson;
    double nThreshold = taskResult.infoTask.nThreshold;
    auto& itBegin = taskResult.seqPerson[0];
    if (itBegin.fScore < taskResult.infoTask.nThreshold)
    {
        taskResult.seqPerson.erase(++taskResult.seqPerson.begin(), taskResult.seqPerson.end());
    }
    else
    {
        tmpCompare.erase(std::remove_if(tmpCompare.begin(), tmpCompare.end(),
            [nThreshold](const Task::ComparePerson& item)
        {
            return item.fScore < nThreshold;
        }), tmpCompare.end());
    }

    return tmpCompare.empty();
}

bool TaskResult::filter_duplication(Task::TaskResult& taskResult)
{
    // filter the duplication with the same person
    double nThreshold = taskResult.infoTask.nThreshold;
    int nRecogInterval = taskResult.infoTask.nInterval;
    int nCameraId = taskResult.nCameraId;
    auto& itBegin = taskResult.seqPerson[0];
    if (itBegin.fScore> nThreshold)
    {
        int64_t nMilliTimestamp = Basic::Time::GetMilliTimestamp();
        SCOPE_LOCK(m_mtRecord);
        auto& itMatch = m_mapRecord.find(itBegin.nPicId);
        if (itMatch == m_mapRecord.end())
        {
            m_mapRecord[itBegin.nPicId].first= nCameraId;
            m_mapRecord[itBegin.nPicId].second = nMilliTimestamp;
        }
        else
        {
            if (nMilliTimestamp - itMatch->second.second >= nRecogInterval)
            {
                itMatch->second.first = taskResult.nCameraId;
                itMatch->second.second = nMilliTimestamp;
            }
            else
            {
                if (itMatch->second.first != taskResult.nCameraId)
                {
                    itMatch->second.first = taskResult.nCameraId;
                    itMatch->second.second = nMilliTimestamp;
                }
                else
                {
                    // record exist in cache
                    return true;
                }
            }
        }
    }
    return false;
}

void TaskResult::filter_stranger(Task::TaskResult& taskResult)
{
    // filter stranger, do nothing now
}

int TaskResult::fill_person_info(Task::TaskResult& taskResult)
{
    if (taskResult.seqPerson.empty()) return CG_OK;
    // fill compare-person's info
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try {
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        for (auto& item : taskResult.seqPerson)
        {
            conn << "select t_per.id,t_per.name as person_name,sex,card_no,category"
                ",pic_url,t_per.lib_id,t_lib.name as lib_name from ft_picture t_pic"
                << " left join ft_person t_per on t_per.id= t_pic.person_id and t_per.visible =1"
                << " left join ft_library t_lib on t_lib.lib_id= t_per.lib_id and t_lib.visible =1"
                << " where t_pic.pic_id=:pic_id;"
                , soci::into(item.nPersonId)
                , soci::into(item.strName)
                , soci::into(item.nSex)
                , soci::into(item.strCardNo)
                , soci::into(item.nCategory)
                , soci::into(item.strPicPath)
                , soci::into(item.nLibId)
                , soci::into(item.strLibName)
                , soci::use(item.nPicId);
        }
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Fill person's info with compare result exception:" << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }

    return CG_OK;
}

int TaskResult::save_result(const Task::TaskResult& taskResult)
{
    if (taskResult.seqPerson.empty()) return CG_OK;
    int nClassify = taskResult.seqPerson.begin()->fScore >= taskResult.infoTask.nThreshold ? 1 : 0;
    // save data to base
    soci::session conn(DatabaseConnPool::Instance().GetConnPool());
    try
    {
        int nTaskId = taskResult.infoTask.nTaskId;
        std::string strTaskName = taskResult.infoTask.strTaskName;
        std::string strCameraName;
        CHANGE_DATABASE(conn, CFG_DATABASE.strFrontBase);;
        soci::indicator ind;
        conn << "select name from ft_task where visible=1"
            << " and id=:id;"
            , soci::into(strTaskName, ind)
            , soci::use(nTaskId);
        conn << "select name from ft_camera where visible=1"
            << " and id=:id;"
            , soci::into(strCameraName, ind)
            , soci::use(taskResult.nCameraId);
        // save the record
        CHANGE_DATABASE(conn, CFG_DATABASE.strHistoryBase);
        DB_BEGIN(conn);
        for (const auto& item : taskResult.seqPerson)
        {
            conn << "insert into hist_recognize_record(task_id,task_name,camera_id,camera_name"
                << ",lib_id,lib_name,person_id,person_name,sex,card_no,category"
                << ",classify,pic_path,capture_path,capture_time,similarity)"
                << " value(:task_id,:task_name,:camera_id,:camera_name,:lib_id,:lib_name"
                << ",:person_id,:person_name,:sex,:card_no,:category"
                << ",:classify,:pic_path,:capture_path,:capture_time,:similarity);"
                , soci::use(nTaskId)
                , soci::use(strTaskName)
                , soci::use(taskResult.nCameraId)
                , soci::use(strCameraName)
                , soci::use(item.nLibId)
                , soci::use(item.strLibName)
                , soci::use(item.nPersonId)
                , soci::use(item.strName)
                , soci::use(item.nSex)
                , soci::use(item.strCardNo)
                , soci::use(item.nCategory)
                , soci::use(nClassify)
                , soci::use(item.strPicPath)
                , soci::use(taskResult.strCapturePath)
                , soci::use(taskResult.strCaptureTime)
                , soci::use(item.fScore);
        }
        DB_COMMIT(conn);
    }
    catch (const std::exception& e)
    {
        DB_ROLLBACK(conn);
        LOG(ERROR) << "Fill person's info with compare result exception:" << e.what();
        return CG_EXCUTE_SQL_ERROR;
    }
    return CG_OK;
}