/**************************************************************
* @brief:       rtsp¡˜π‹¿Ì
* @date:        20200213
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
#include <mutex>
#include <list>
#include <thread>
#include <atomic>
#include "TaskDefines.h"
#include <Basic/ThreadPool.h>
#include <Basic/ThreadObject.h>
#include "../Rtsp/RtspStreamHelper.h"


class RtspStreamHelper;
typedef std::shared_ptr<RtspStreamHelper> StreamPtr;
class StreamHandle
{
public:
    StreamHandle();
    StreamHandle(const TaskInfo& task,
        int nCameraId,
        const std::string& strRtsp);
    ~StreamHandle();
    int AddTask(const TaskInfo& task);
    void RemoveTask(int nTaskId, bool& bNeedDel);
    void Stop();
    bool GetCameraState();


private:
    void check_camera_state();
    void fetch_frame(const cv::Mat& cvMat);
    void update_video_state(bool bOpen = true);
    void handle_frame(cv::Mat frame);
    bool save_frame(cv::Mat frame,std::string& strPicData, std::string& strPicUrl);

private:
    std::string m_strIdentity;
    int m_nCameraId;
    std::mutex m_mtTask;
    std::list<TaskInfo> m_listTask;

    StreamPtr m_pStream;
    Basic::ThreadObject m_objThread;
    std::atomic<bool> m_bOpen;
    time_t m_nLastInteractive;
    Basic::ThreadPool m_pool;
    std::map<int, time_t> m_mapLastCapture;

};