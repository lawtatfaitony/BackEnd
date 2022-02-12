#pragma once
#include <string>
#include <map>
#include "CameraSearchDefins.h"


#ifdef CAMERA_SEARCH_LIBRARY_EXPORT
#ifdef _WIN32
#define CAMERA_SEARCH_LIBRARY __declspec(dllexport)
#else
#define CAMERA_SEARCH_LIBRARY  __attribute__((visibility("default")))
#endif
#else
#define CAMERA_SEARCH_LIBRARY __declspec(dllimport)
#endif


/**
* @brief:       search cameras
* @author:      wite_chen
* @create:      20120-02-21
* @para[out]:   mapDevices, the result of cameras
* @return:      string, result of invoking, empty if success, or failed
*/
CAMERA_SEARCH_LIBRARY std::string SearchCameras(std::map<std::string, CameraSearch::DeviceInfo>& mapDevices);

/**
* @brief:       search camera's rtsp
* @author:      wite_chen
* @create:      20120-02-21
* @para[in]:    strIp, camera's ip
* @para[in]:    strUser, user of login
* @para[in]:    strPwd, password of login
* @para[out]:   strRtsp, camera's rtsp
* @return:      string, result of invoking, empty if success, or failed
*/
CAMERA_SEARCH_LIBRARY std::string SearchCameraRtsp(const std::string& strIp,
    const std::string& strUser,
    const std::string& strPwd,
    std::string& strRtsp);
