#include "OnvifHelper.h"
#include <iostream>
#include <vector>
#include "onvif/wsseapi.h"
#include "onvif/wsaapi.h"
#include "onvif/wsdd.nsmap"

//#include "stdsoap2.c"

#if defined(_WIN32)
#pragma warning(disable:4996)
#include <ObjBase.h>
#include <Iphlpapi.h>
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "ole32.lib")
#else
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif // _WIN32

//tag for searching
const std::string kProfileTag[] = { "Media", "device_service" };
const std::string kOnvifPrefix = "onvif://www.onvif.org/";
const int kMaxMsg = 1024;
const int kTimeout = 3;

#define EXTRACT_DEVICE_INFO(to, key, map) \
    if (map.find(#key) != map.end()) { \
        to = map[#key]; \
    }


OnvifHelper::OnvifHelper()
{
}

OnvifHelper& OnvifHelper::Instance()
{
    static OnvifHelper g_Instance;
    return g_Instance;
}

std::string OnvifHelper::DiscoveryDevice(std::map<std::string, CameraSearch::DeviceInfo>& mapDevices)
{
    std::string strErrorMsg;
    int nDevice = 0;
    int nRst = SOAP_FAULT;
    wsdd__ProbeType req;
    struct __wsdd__ProbeMatches resp;
    wsdd__ScopesType sScope;
    struct soap *soap = nullptr;
    struct SOAP_ENV__Header header;

    const char *was_To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    const char *was_Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
    const char *soap_endpoint = "soap.udp://239.255.255.250:3702/";

    soap = init_soap(&header, was_To, was_Action, kTimeout);
    soap->header = &header;
    soap_default_wsdd__ScopesType(soap, &sScope);
    sScope.__item = "";
    soap_default_wsdd__ProbeType(soap, &req);
    req.Scopes = &sScope;
    req.Types = "tdn:NetworkVideoTransmitter";

    nRst = soap_send___wsdd__Probe(soap, soap_endpoint, nullptr, &req);
    while (SOAP_OK == nRst)
    {
        nRst = soap_recv___wsdd__ProbeMatches(soap, &resp);//这个函数用来接受probe消息，存在resp里面
        if (SOAP_OK == nRst)
        {
            if (soap->error)
            {
                nRst = soap->error;
                strErrorMsg = get_error_msg(soap, nRst);
                std::cout << strErrorMsg << std::endl;
            }
            else
            {
                nDevice++;
                if (resp.wsdd__ProbeMatches->ProbeMatch != nullptr
                    && resp.wsdd__ProbeMatches->ProbeMatch->XAddrs != nullptr)
                { 
                    
                    CameraSearch::DeviceInfo infoDevice;
                    char* pitem = resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item;
                    if (pitem) 
                    {
                        std::map<std::string, std::string> mapDevDetail;
                        std::string strDeviceInfo(pitem);
                        split_device_info(kOnvifPrefix, strDeviceInfo, mapDevDetail);
                        EXTRACT_DEVICE_INFO(infoDevice.name, name, mapDevDetail);
                        EXTRACT_DEVICE_INFO(infoDevice.hardware, hardware, mapDevDetail);
                        EXTRACT_DEVICE_INFO(infoDevice.location, location, mapDevDetail);
                    }
                    auto strIp = get_ip_from_str(resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
                    mapDevices[strIp] = infoDevice;
                }
            }
        }
        else if (soap->error) 
        {
            if (0 == nDevice)
            {
                nRst = soap->error;
                strErrorMsg = get_error_msg(soap, nRst);
                std::cout << strErrorMsg << std::endl;
            }
            break;
        }
    }

    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);
    return strErrorMsg;
}

std::string OnvifHelper::GetCameraRtsp(const std::string& ip, const std::string& user, const std::string& password, std::string& rtsp)
{
    std::string strErrorMsg;
    int nRst = SOAP_OK;
    struct soap *soap = nullptr;
    char *soap_endpoint = (char *)malloc(256);
    const char *soap_action = nullptr;

    struct SOAP_ENV__Header header;

    struct _trt__GetProfiles media_GetProfiles;
    struct _trt__GetProfilesResponse media_GetProfilesResponse;
    struct _trt__GetStreamUri media_GetStreamUri;
    struct _trt__GetStreamUriResponse media_GetStreamUriResponse;

    /* 1 GetProfiles */
    soap = init_soap(&header, nullptr, nullptr, kTimeout);
    int cnt = sizeof(kProfileTag) / sizeof(kProfileTag[0]);
    do
    {
        nRst = SOAP_OK;
        memset(soap_endpoint, '\0', 256);
        sprintf(soap_endpoint, "http://%s/onvif/%s", ip.c_str(), kProfileTag[--cnt].c_str());
        soap_wsse_add_UsernameTokenDigest(soap, "user", user.c_str(), password.c_str());
        soap_call___trt__GetProfiles(soap, soap_endpoint, soap_action, &media_GetProfiles, &media_GetProfilesResponse);
        if (soap->error)
        {
            nRst = soap->error;
            if (SOAP_UDP_ERROR == nRst || SOAP_TCP_ERROR == nRst)
            {
                strErrorMsg = "timeout with query rtsp";
                break;
            }
            strErrorMsg = get_error_msg(soap, nRst);
            std::cout << strErrorMsg << std::endl;
        }
        else
        {
            printf("==== [ Media Profiles Response ] ====\n"
                "> Name  :  %s\n"
                "> token :  %s\n\n", \
                media_GetProfilesResponse.Profiles->Name, \
                media_GetProfilesResponse.Profiles->token);
            break;
        }
    } while (cnt);

    /* 2 GetStreamUri */
    if (SOAP_OK == soap->error)
    {
        media_GetStreamUri.StreamSetup = (struct tt__StreamSetup *)soap_malloc(soap, sizeof(struct tt__StreamSetup));
        media_GetStreamUri.StreamSetup->Transport = (struct tt__Transport *)soap_malloc(soap, sizeof(struct tt__Transport));

        media_GetStreamUri.StreamSetup->Stream = tt__StreamType__RTP_Unicast;
        media_GetStreamUri.StreamSetup->Transport->Protocol = tt__TransportProtocol__UDP;
        media_GetStreamUri.StreamSetup->Transport->Tunnel = 0;
        media_GetStreamUri.StreamSetup->__size = 1;
        media_GetStreamUri.StreamSetup->__any = nullptr;
        media_GetStreamUri.StreamSetup->__anyAttribute = nullptr;
        media_GetStreamUri.ProfileToken = media_GetProfilesResponse.Profiles->token;
        soap_wsse_add_UsernameTokenDigest(soap, "user", user.c_str(), password.c_str());
        do
        {
            soap_call___trt__GetStreamUri(soap, soap_endpoint, soap_action, &media_GetStreamUri, &media_GetStreamUriResponse);
            if (soap->error)
            {
                nRst = soap->error;
                strErrorMsg = get_error_msg(soap, nRst);
                std::cout << strErrorMsg << std::endl;
            }
            else
            {
                printf("==== [ Media Stream Uri Response ] ====\n"
                    "> MediaUri :\n\t%s\n", \
                    media_GetStreamUriResponse.MediaUri->Uri);
                printf("[\033[1;32m success\033[0m ] Get Stream Uri!\n");

                std::string strRtspTmp = media_GetStreamUriResponse.MediaUri->Uri;
                rtsp = strRtspTmp.substr(0, 7);
                rtsp += user + ":" + password + "@" + strRtspTmp.substr(7);
            }
        } while (0);
    }
    free(soap_endpoint);
    soap_endpoint = nullptr;
    soap_destroy(soap);
    soap_end(soap);
    soap_free(soap);
    return strErrorMsg;
}

int OnvifHelper::GetLocalIps(std::map<std::string, std::set<std::string> >& ips)
{
#ifdef _WIN32
    ULONG ulLen = 0;
    PIP_ADAPTER_INFO lpAdapterInfo = nullptr, lpNextData = nullptr;

    GetAdaptersInfo(lpAdapterInfo, &ulLen);
    if (0 == ulLen)
        return -1;

    lpAdapterInfo = (PIP_ADAPTER_INFO)(new CHAR[ulLen]);
    if (nullptr == lpAdapterInfo)
        return -1;

    memset(lpAdapterInfo, 0, ulLen);
    ULONG uRet = GetAdaptersInfo(lpAdapterInfo, &ulLen);
    if (uRet != ERROR_SUCCESS)
    {
        delete[] lpAdapterInfo;
        lpAdapterInfo = nullptr;
        return -1;
    }

    // multiple adapters
    for (lpNextData = lpAdapterInfo; lpNextData != nullptr; lpNextData = lpNextData->Next) {
        // multiple ips for each adapter
        IP_ADDR_STRING *pIpAddrString = &(lpNextData->IpAddressList);
        int IPnumPerNetCard = 0;
        do {
            pIpAddrString = pIpAddrString->Next;
        } while (pIpAddrString);
    }

    delete[] lpAdapterInfo;
    lpAdapterInfo = nullptr;

#else
    struct ifaddrs *ifList = nullptr;
    int iRet = getifaddrs(&ifList);
    if (iRet < 0) { return -1; }

    struct sockaddr_in *sin = nullptr;
    struct ifaddrs *ifa = nullptr;
    for (ifa = ifList; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
            continue;
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            printf("\n>>> interfaceName: %s\n", ifa->ifa_name);
            sin = (struct sockaddr_in *)ifa->ifa_addr;
            printf(">>> ipAddress: %s\n", inet_ntoa(sin->sin_addr));
            ips[ifa->ifa_name].insert(inet_ntoa(sin->sin_addr));
        }
    }
    freeifaddrs(ifList);
#endif
    return 0;
}

// init soap
struct soap* OnvifHelper::init_soap(struct SOAP_ENV__Header *header,
    const char *was_To, const char *was_Action, int timeout)
{
    struct soap *soap = nullptr;
    unsigned char macaddr[6];
    char _HwId[1024];
    unsigned int Flagrand;
    soap = soap_new();
    if (soap == nullptr)
    {
        printf("[%d]soap = nullptr\n", __LINE__);
        return nullptr;
    }
    soap_set_namespaces(soap, namespaces);
    soap->recv_timeout = 10;
    soap->send_timeout = 10;
    soap->connect_timeout = 10;
    soap_default_SOAP_ENV__Header(soap, header);

    if (!m_strLocalIP.empty())
    {
        struct in_addr if_req;
        inet_pton(AF_INET, m_strLocalIP.c_str(), (void*)&if_req.s_addr);
        soap->ipv4_multicast_if = (char*)soap_malloc(soap, sizeof(in_addr));
        memset(soap->ipv4_multicast_if, 0, sizeof(in_addr));
        memcpy(soap->ipv4_multicast_if, (char*)&if_req, sizeof(if_req));
    }

    // 为了保证每次搜索的时候MessageID都是不相同的！因为简单，直接取了随机值
    srand((int)time(0));
    Flagrand = rand() % 9000 + 1000; //保证四位整数
    macaddr[0] = 0x1; macaddr[1] = 0x2; macaddr[2] = 0x3; macaddr[3] = 0x4; macaddr[4] = 0x5; macaddr[5] = 0x6;
    sprintf(_HwId, "urn:uuid:%ud68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X",
        Flagrand, macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    header->wsa__MessageID = (char *)malloc(100);
    memset(header->wsa__MessageID, 0, 100);
    strncpy(header->wsa__MessageID, _HwId, strlen(_HwId));

    if (was_Action != nullptr)
    {
        header->wsa__Action = (char *)malloc(1024);
        memset(header->wsa__Action, '\0', 1024);
        strncpy(header->wsa__Action, was_Action, 1024);//"http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
    }
    else {}

    if (was_To != nullptr)
    {
        header->wsa__To = (char *)malloc(1024);
        memset(header->wsa__To, '\0', 1024);
        strncpy(header->wsa__To, was_To, 1024);//"urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    }
    soap->header = header;
    return soap;
}

int OnvifHelper::get_capabilities(struct __wsdd__ProbeMatches *resp)
{
    struct _tds__GetCapabilities capa_req;
    struct _tds__GetCapabilitiesResponse capa_resp;

    struct soap *soap = nullptr;
    struct SOAP_ENV__Header header;

    int retval = 0;
    soap = init_soap(&header, nullptr, nullptr, 5);
    char *soap_endpoint = (char *)malloc(256);
    memset(soap_endpoint, '\0', 256);
    sprintf(soap_endpoint, resp->wsdd__ProbeMatches->ProbeMatch->XAddrs);
    capa_req.Category = (enum tt__CapabilityCategory *)soap_malloc(soap, sizeof(int));

    capa_req.__sizeCategory = 1;
    *(capa_req.Category) = (enum tt__CapabilityCategory)0;
    const char *soap_action = "http://www.onvif.org/ver10/device/wsdl/GetCapabilities";
    capa_resp.Capabilities = (struct tt__Capabilities*)soap_malloc(soap, sizeof(struct tt__Capabilities));

    soap_wsse_add_UsernameTokenDigest(soap, "user", m_strUsername.c_str(), m_strPassword.c_str());
    do
    {
        retval = soap_call___tds__GetCapabilities(soap, soap_endpoint, soap_action, &capa_req, &capa_resp);
        if (soap->error)
        {
            std::cout << get_error_msg(soap, retval) << std::endl;
            break;
        }
        else
        {
            if (nullptr == capa_resp.Capabilities)
                printf("GetCapabilities failed! result = %d\n", retval);
            else
                get_profiles(soap, &capa_resp);
        }
    } while (0);

    free(soap_endpoint);
    soap_endpoint = nullptr;
    soap_destroy(soap);
    return retval;
}

void OnvifHelper::get_profiles(struct soap *soap, struct _tds__GetCapabilitiesResponse *capa_resp)
{
    struct _trt__GetProfiles trt__GetProfiles;
    struct _trt__GetProfilesResponse trt__GetProfilesResponse;
    int result = SOAP_OK;
    soap_wsse_add_UsernameTokenDigest(soap, "user", m_strUsername.c_str(), m_strPassword.c_str());

    result = soap_call___trt__GetProfiles(soap, capa_resp->Capabilities->Media->XAddr, nullptr, &trt__GetProfiles, &trt__GetProfilesResponse);
    if ( -1 == result)
    {
        result = soap->error;
        std::cout << get_error_msg(soap, result) << std::endl;
    }
    else
    {
        if (trt__GetProfilesResponse.Profiles != nullptr)
        {
            int profile_cnt = trt__GetProfilesResponse.__sizeProfiles;
            get_uri(soap, &trt__GetProfilesResponse, capa_resp);
        }
    }
}

void OnvifHelper::get_uri(struct soap *soap, struct _trt__GetProfilesResponse *trt__GetProfilesResponse,
struct _tds__GetCapabilitiesResponse *capa_resp)
{
    int result = 0;
    struct _trt__GetStreamUri *trt__GetStreamUri = (struct _trt__GetStreamUri *)malloc(sizeof(struct _trt__GetStreamUri));
    struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse =
        (struct _trt__GetStreamUriResponse *)malloc(sizeof(struct _trt__GetStreamUriResponse));

    trt__GetStreamUri->StreamSetup = (struct tt__StreamSetup*)soap_malloc(soap, sizeof(struct tt__StreamSetup));
    trt__GetStreamUri->StreamSetup->Stream = tt__StreamType__RTP_Unicast;
    trt__GetStreamUri->StreamSetup->Transport = (struct tt__Transport *)soap_malloc(soap, sizeof(struct tt__Transport));
    trt__GetStreamUri->StreamSetup->Transport->Protocol = tt__TransportProtocol__UDP;
    trt__GetStreamUri->StreamSetup->Transport->Tunnel = 0;
    trt__GetStreamUri->StreamSetup->__size = 1;
    trt__GetStreamUri->StreamSetup->__any = nullptr;
    trt__GetStreamUri->StreamSetup->__anyAttribute = nullptr;
    trt__GetStreamUri->ProfileToken = trt__GetProfilesResponse->Profiles->token;

    soap_wsse_add_UsernameTokenDigest(soap, "user", m_strUsername.c_str(), m_strPassword.c_str());
    soap_call___trt__GetStreamUri(soap, capa_resp->Capabilities->Media->XAddr, nullptr, trt__GetStreamUri, trt__GetStreamUriResponse);

    if (soap->error)
    {
        result = soap->error;
        //print error msg
        std::cout << get_error_msg(soap, result) << std::endl;
    }
    else
    {
        std::cout << trt__GetStreamUriResponse->MediaUri->Uri << std::endl;
    }
}

void OnvifHelper::split_device_info(const std::string& prefix, std::string& str, std::map<std::string, std::string>& dev_info)
{
    if (str.empty()) return;

    // all chars are ' '
    auto start_pos = str.find_first_not_of(' ');
    if (start_pos == std::string::npos) return;

    try
    {
        std::vector<std::string> vec;
        char* src_str = const_cast<char*>(str.data());
        char* ret_str = strtok(src_str, " ");
        while (ret_str)
        {
            vec.push_back(ret_str);
            ret_str = strtok(nullptr, " ");
        }

        for (auto& it : vec)
        {
            auto pos = it.find(prefix);
            if (pos != std::string::npos)
            {
                it = it.substr(pos + prefix.size());
                auto nPos = it.find("/");
                if (nPos == std::string::npos)
                    continue;
                //cout << "str:" << it << endl;
                dev_info[it.substr(0, nPos)] = it.substr(nPos + 1);
            }
        }
    }
    catch (...) {}
}

std::string OnvifHelper::get_ip_from_str(const std::string& src_str)
{
    std::string ip = src_str;
    std::string prefix = "http://";
    auto pos = src_str.find(prefix);
    if (pos == std::string::npos)
        return ip;
    try
    {
        ip = ip.substr(pos + prefix.size());
        pos = ip.find("/");
        ip = ip.substr(0, pos);
    }
    catch (...) {}
    return ip;
}

std::string OnvifHelper::get_error_msg(struct soap *soap, int nErr)
{
    char szMsg[kMaxMsg];
    soap_sprint_fault(soap, szMsg, kMaxMsg);
    return szMsg;
}