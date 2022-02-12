#include "DiscoverCamera.h"
#include <Macro.h>
#include "CameraSearch/CameraSearch.h"


USE_NAMESPACE_CACHE;
DiscoveryCamera::DiscoveryCamera()
{

}
DiscoveryCamera& DiscoveryCamera::Instance()
{
    static DiscoveryCamera g_Instance;
    return g_Instance;
}

DiscoveryCamera::~DiscoveryCamera()
{
}

void DiscoveryCamera::Start()
{
    m_objThread.Start(std::bind(&DiscoveryCamera::run, &DiscoveryCamera::Instance()), kSearchInterval);
}
void DiscoveryCamera::Stop()
{
    m_objThread.Stop();
}

void DiscoveryCamera::GetSearchCameras(std::map<std::string, CameraSearch::DeviceInfo>& mapDevices)
{
    SCOPE_LOCK(m_mtDevice);
    mapDevices = m_mapDevice;
}

std::string DiscoveryCamera::SearchCameraRtsp(const std::string& strIp,
    const std::string& strUser,
    const std::string& strPwd,
    std::string& strRtsp)
{
    return ::SearchCameraRtsp(strIp, strUser, strPwd, strRtsp);
}

void DiscoveryCamera::run()
{
    std::map<std::string, CameraSearch::DeviceInfo> mapDevices;
    SearchCameras(mapDevices);
    SCOPE_LOCK(m_mtDevice);
    m_mapDevice = mapDevices;
}