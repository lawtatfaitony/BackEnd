#include "CameraState.h"
#include <soci/soci.h>
#include <easylogging\easylogging++.h>
#include <Macro.h>
#ifdef _WIN32
    #include <Basic/WindowsPing.h>
#else
    #include <Basic/LinuxPing.h>
#endif // _WIN32
#include "../../Config/GlobalConfig.h"


USE_NAMESPACE_CACHE;
CameraStateManagement::CameraStateManagement()
{
    m_bStop.store(false);
}

CameraStateManagement& CameraStateManagement::Instance()
{
    static CameraStateManagement g_Instance;
    return g_Instance;
}

CameraStateManagement::~CameraStateManagement()
{

}

void CameraStateManagement::Start()
{
    m_thRun = std::thread(&CameraStateManagement::run, &CameraStateManagement::Instance());
    load_camera();
}

void CameraStateManagement::Stop()
{
    m_bStop.store(true);
    m_cvCamera.notify_all();
    if (m_thRun.joinable())
        m_thRun.join();
}


void CameraStateManagement::AddCamera(const std::string& strIp)
{
    {
        SCOPE_LOCK(m_mtCamera);
        m_mapCamera[strIp] = 0;
    }
    m_cvCamera.notify_one();
}

void CameraStateManagement::DeleteCamera(const std::string& strIp)
{
    SCOPE_LOCK(m_mtCamera);
    m_mapCamera.erase(strIp);
}

void CameraStateManagement::GetAllCamera(std::map<std::string, int>& mapCamera)
{
    SCOPE_LOCK(m_mtCamera);
    mapCamera = m_mapCamera;
}

void CameraStateManagement::load_camera()
{
    // load the cameras
    soci::session conn;
    try
    {
        conn.open(CFG_DATABASE.strConnFrontendBase);
        std::string strIp;
        soci::statement st = (conn.prepare
            << "select ip from ft_camera where visible = 1"
            , soci::into(strIp));
        st.execute();
        SCOPE_LOCK(m_mtCamera);
        while(st.fetch())
            m_mapCamera[strIp] = 0;
        if (!m_mapCamera.empty())
            m_cvCamera.notify_one();
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Load cameras exception:" << e.what();
    }
}

void CameraStateManagement::run()
{
    while (!m_bStop.load())
    {
        if (m_bStop.load()) break;
        std::map<std::string, int> mapCamera;
        {
            std::unique_lock<std::mutex> lock(m_mtCamera);
            m_cvCamera.wait_for(lock, std::chrono::milliseconds(kCheckInterval));
            mapCamera = m_mapCamera;
        }
        if (!mapCamera.empty())
        {
            // check online
            for (const auto& item : mapCamera)
                mapCamera[item.first] = check_online(item.first) ? 1 : 0;
            // update the camera's state
            SCOPE_LOCK(m_mtCamera);
            for (auto& item : m_mapCamera)
            {
                auto itMatch = mapCamera.find(item.first);
                if (itMatch != mapCamera.end())
                    m_mapCamera[item.first] = itMatch->second;
            }
        }
    }
}

bool CameraStateManagement::check_online(const std::string& strIp)
{
#ifdef _WIN32
    WindowsPing onlineCheck;
    return onlineCheck.Ping((char*)strIp.c_str());
#else
    Linuxping onlineCheck;
    return onlineCheck.Ping((char*)strIp.c_str());
#endif // _WIN32
    return true;
}