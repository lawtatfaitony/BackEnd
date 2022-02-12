#include <iostream>
#include <CameraSearch/CameraSearch.h>

int main()
{
    std::string strResult;
    std::map<std::string, CameraSearch::DeviceInfo> mapDevices;
    strResult = SearchCameras(mapDevices);
    std::cout << "Search finished, it records " << mapDevices.size() << "\n";
    for (const auto& item : mapDevices)
        std::cout << item.first << ":" << item.second.location << "\n";
    std::cout << "*********************************\n";
    std::string strIp, strUser, strPassword;
    std::cout << "Please input camera to search:\n";
    std::cout << "ip:";
    std::cin >> strIp;
    std::cout << "user:";
    std::cin >> strUser;
    std::cout << "password:";
    std::cin >> strPassword;
    std::string strRtsp;
    strResult = SearchCameraRtsp(strIp, strUser, strPassword, strRtsp);
    if (strResult.empty())
        std::cout << "Searching success,rtps:\n\t" << strRtsp << "\n";
    else
        std::cout << "Searching failed, error:\n\t" << strResult << "\n";
    std::cout << "Hello World\n";
    return 0;
}