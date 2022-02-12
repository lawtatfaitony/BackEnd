#pragma once
#include <string>


namespace CameraSearch
{
    struct DeviceInfo
    {
        std::string uri;
        std::string ip;
        std::string name;
        std::string location;
        std::string hardware;

        bool operator< (const DeviceInfo& info) const
        {
            return this->ip < info.ip;
        }
    };
}
