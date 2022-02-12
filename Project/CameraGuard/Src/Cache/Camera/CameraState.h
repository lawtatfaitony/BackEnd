#pragma once
#include <string>
#include <mutex>
#include <map>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "../CacheDefine.h"


NAMESPACE_CACHE_BEGIN
class CameraStateManagement
{
    static const int kCheckInterval = 2 * 60 * 1000;
    CameraStateManagement();

public:
    static CameraStateManagement& Instance();
    ~CameraStateManagement();
    void Start();
    void Stop();

    void AddCamera(const std::string& strIp);
    void DeleteCamera(const std::string& strIp);
    void GetAllCamera(std::map<std::string, int>& mapCamera);

private:
    void load_camera();
    void run();
    bool check_online(const std::string& strIp);

private:
    std::atomic<bool> m_bStop;
    std::mutex m_mtCamera;
    std::map<std::string, int> m_mapCamera;
    std::thread m_thRun;
    std::condition_variable m_cvCamera;

};
NAMESPACE_CACHE_END
