/**************************************************************
* @brief:           cache of camera searching
* @auth:            Wite_Chen
* @date:            20200221
* @update:
* @copyright:
*
***************************************************************/
#pragma once
#include <map>
#include <CameraSearch/CameraSearchDefins.h>
#include <Basic/ThreadObject.h>
#include "../CacheDefine.h"


NAMESPACE_CACHE_BEGIN
const int kSearchInterval = 2 * 60 * 1000;
class DiscoveryCamera
{
    DiscoveryCamera();

public:
    static DiscoveryCamera& Instance();
    ~DiscoveryCamera();
    void Start();
    void Stop();
    void GetSearchCameras(std::map<std::string, CameraSearch::DeviceInfo>& mapDevices);
    std::string SearchCameraRtsp(const std::string& strIp,
        const std::string& strUser,
        const std::string& strPwd,
        std::string& strRtsp);

private:
    void run();

private:
    std::mutex m_mtDevice;
    std::map<std::string, CameraSearch::DeviceInfo> m_mapDevice;
    Basic::ThreadObject m_objThread;
};
NAMESPACE_CACHE_END
