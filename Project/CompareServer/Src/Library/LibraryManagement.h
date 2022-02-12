/**************************************************************
* @brief:       底库管理
* @date:        20200131
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <atomic>
#include <memory>
#include <list>
#include <condition_variable>
#include "Library.h"
#include <FaceSdk/FaceSdk.h>
#include <Basic/ThreadPool.h>

struct TaskInfo
{
    std::string strIdetityId;  // 客户端连接id
    int nTaskId = 0;
    int nCameraId = 0;
    float nThreshold;
    int nTop;
    std::string strLibList;
    std::string strCaptureTime;
    std::string strPicData;
    std::string strPicPath;
};

class LibraryManagement
{
private:
    LibraryManagement();

public:
    static LibraryManagement& Instance();
    ~LibraryManagement();
    bool Init();
    void Stop();
    // library
    int AddLibrary(int nLibId, std::string& strResult);
    int DeleteLibrary(int nLibId);
    int ListLibrary(std::string& strResult);

    // person
    int AddPerson(int nLibId, const std::string& strPicUrl, std::string& strResult);
    int DeletePerson(int nLibId, int64_t nPersonId);
    int UpdatePerson(int nLibId, int64_t nPersonId, const std::string& strPicUrl, std::string& strResult);

    // compare
    int GetFeatureSize();
    int ExtractFeature(const std::string& strPicUrl, std::string& strResult);
    int Compare1v1(const std::string& strLPicUrl,
        const std::string& strRPicUrl,
        float& nSimilarity);
    int CompareWithFeature(const std::string& strLFerature,
        const std::string& strRFerature,
        float& nSimilarity);

    // interactive with task
    int PushCompareTask(const TaskInfo& task);


private:
    void load_library();
    int extract_feature(const std::string& strPicUrl, std::shared_ptr<float>& pFeature);
    bool download_picture(const std::string& strPicUrl, std::string& strPicData);
    bool detect_face(const ImageData& imgData, std::vector<FaceInfo>& vecFace);
    void do_task();
    void to_json(const std::list<int>& listData, std::string& strResult);
    std::string serialize_feature(const std::shared_ptr<float>& pFeature);
    void deserialize_feature(const std::string& strFeature, std::shared_ptr<float>& pFeature);

private:
    std::atomic<bool> m_bStop;
    // library
    std::mutex m_mtLib;
    std::map<int, std::shared_ptr<LibraryHandle>> m_mapLibHandle;
    // person-id
    std::atomic<int64_t> m_acPerson;
    // FaceSdk
    void* m_pFaceSdk;
    int m_nFeatureize;
    // task
    std::condition_variable m_cvTask;
    std::mutex m_mtTask;
    std::list<TaskInfo> m_listTask;
    Basic::ThreadPool m_poolExcuteTask;
    std::thread m_thDoTask;

};