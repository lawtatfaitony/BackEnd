#include "StreamHandle.h"
#include "../Server/ServerImpl.h"
#include <Basic/Function.h>
#include <Basic/Base64.h>
#include <Basic/Time.h>
#include "../Client/CompareClient.h"
#include "../Http/LibcurlHelper.h"


StreamHandle::StreamHandle()
    : m_bOpen(false)
    , m_nLastInteractive(0)
{
    m_pool.Start(1);
}

StreamHandle::~StreamHandle()
{
    Stop();
}

StreamHandle::StreamHandle(const TaskInfo& task,
    int nCameraId,
    const std::string& strRtsp)
    : m_nCameraId(nCameraId)
    , m_bOpen(false)
    , m_nLastInteractive(0)
{
    m_pool.Start();
    m_strIdentity = task.strIdetityId;
    m_listTask.push_back(task);
    m_pStream = std::make_shared<RtspStreamHelper>();
    std::string strVideoName = std::to_string(nCameraId) + std::to_string(time(nullptr)) + ".mp4";
    m_pStream->StartRecvStream(strRtsp, strVideoName, std::bind(&StreamHandle::fetch_frame, this, std::placeholders::_1));
    m_objThread.Start(std::bind(&StreamHandle::check_camera_state, this), 2 * 1000);
}

int StreamHandle::AddTask(const TaskInfo& task)
{
    std::lock_guard<std::mutex> lock(m_mtTask);
    m_listTask.push_back(task);

    return 0;
}

void StreamHandle::RemoveTask(int nTaskId, bool& bNeedDel)
{
    bNeedDel = false;
    std::lock_guard<std::mutex> lock(m_mtTask);
    auto iter = m_listTask.begin();
    for (; iter != m_listTask.end(); ++iter)
    {
        if (iter->nTaskId == nTaskId)
        {
            m_listTask.erase(iter);
            if (m_listTask.empty())
            {
                Stop();
                bNeedDel = true;
            }
            return;
        }
    }
}

void StreamHandle::Stop()
{
    m_pStream->StopRecvStream();
    m_objThread.Stop();
    m_pool.Stop();
}

bool StreamHandle::GetCameraState()
{
    return m_bOpen;
}

void StreamHandle::check_camera_state()
{
    if (0 == m_nLastInteractive) return;
    // 每2分钟更新一次状态
    if (time(nullptr) - m_nLastInteractive >= 120)
    {
        m_bOpen = false;
        update_video_state(m_bOpen);
    }
    else
    {
        if (!m_bOpen)
        {
            m_bOpen = true;
            update_video_state(m_bOpen);
        }
    }
}

void StreamHandle::fetch_frame(const cv::Mat& cvMat)
{
    m_nLastInteractive = time(nullptr);
    m_pool.Commit([=]() {
        handle_frame(cvMat);
    });
}

void StreamHandle::update_video_state(bool bOpen)
{
    // update the camera state
    std::list<TaskInfo> listTask;
    {
        std::lock_guard<std::mutex> lock(m_mtTask);
        listTask = m_listTask;
    }
    Task::seqCameraState stateCamera;
    for (const auto& item : listTask)
    {
        Task::CameraState video;
        video.nTaskId = item.nTaskId;
        video.nCameraId = m_nCameraId;
        video.bOpen = bOpen;
        stateCamera.push_back(video);
    }
    TaskServerImpl::UpdateCameraState(m_strIdentity, stateCamera);
}

void StreamHandle::handle_frame(cv::Mat frame)
{
    std::list<TaskInfo> listTask;
    {
        std::lock_guard<std::mutex> lock(m_mtTask);
        listTask = m_listTask;
    }
    std::string strPicData;
    std::string strPicPath;
    if (!save_frame(frame, strPicData, strPicPath))return;
    time_t tmNow = time(nullptr);
    for (const auto& item : listTask)
    {
        bool bDoCompare = false;
        auto iter_match = m_mapLastCapture.find(item.nTaskId);
        if (iter_match == m_mapLastCapture.end() ||
            (tmNow - iter_match->second) * 1000 >= item.nInterval)
        {
            m_mapLastCapture[item.nTaskId] = tmNow;
            bDoCompare = true;
        }
        if (!bDoCompare) continue;

        Compare::CompareTask task;
        task.strIdentity = Client::CompareClient::Instance().GetIdentityName();
        task.nTaskId = item.nTaskId;
        task.nCameraId = m_nCameraId;
        task.nThreshold = item.nThreshold;
        task.nTop = item.nTop;
        Basic::ContainerToString(item.veclib, task.strLibList);
        task.strPicData = strPicData;
        task.strPicPath = strPicPath;
        task.strCaptureTime = Basic::Time::DatetimeToString(tmNow);
        Client::CompareClient::Instance().PushCompareTask(task);
    }
}

bool StreamHandle::save_frame(cv::Mat frame, std::string& strPicData, std::string& strPicPath)
{
    // mat->string
    std::vector<unsigned char> szData;
    cv::imencode(".jpg", frame, szData);
    strPicData.resize(szData.size());
    memcpy(&strPicData[0], szData.data(), szData.size());
    // TODO: 20200721
    // upload the picture to storage-server
    auto extract_url_path = [&](const std::string& strInput, std::string& strResult)
    {
        strResult = "";
        int nPos = strInput.find_last_of(":");
        if (nPos != std::string::npos)
        {
            nPos = strInput.find("/", nPos);
            if (nPos != std::string::npos)
            {
                strResult = strInput.substr(++nPos);
            }
        }
    };
    LibcurlHelper clientCurl;
    HttpPara para;
    para.strUrl = "http://127.0.0.1:10011/upload/";
    std::string strResponse;
    std::string strBase64 = Basic::Base64::Encode((uint8_t*)strPicData.c_str(), strPicData.size());
    int nCode = clientCurl.Post(para, strBase64, strResponse);
    extract_url_path(strResponse, strPicPath);
    return (0 == nCode) && !(strPicPath.empty());
}