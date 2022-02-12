#define CAMERA_SEARCH_LIBRARY_EXPORT
#include "CameraSearch.h"
#include "OnvifHelper.h"


std::string SearchCameras(std::map<std::string, CameraSearch::DeviceInfo>& mapDevices)
{
    return OnvifHelper::Instance().DiscoveryDevice(mapDevices);
}


std::string SearchCameraRtsp(const std::string& strIp,
     const std::string& strUser, 
     const std::string& strPwd,
     std::string& strRtsp)
{
    return OnvifHelper::Instance().GetCameraRtsp(strIp, strUser, strPwd, strRtsp);
}