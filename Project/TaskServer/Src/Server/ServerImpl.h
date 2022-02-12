/**************************************************************
* @brief:       任务管理
* @date:        20200212
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <Ice/Ice.h>
#include <Task.h>
#include <map>
#include <memory>
#include <mutex>


class TaskServerImpl : public Task::TaskServer
{
public:
    TaskServerImpl();
    ~TaskServerImpl();

public:
    virtual void RegisterClient(const std::string& strId, const Ice::Current & current);
    virtual int StartTask(const Task::TaskInfo& infoTask,
        const Ice::Current& current);
    virtual int StopTask(const std::string& strId, int nTaskId, const Ice::Current & current);


public:
    static void PushTaskResult(const Task::TaskResult& result);
    static void UpdateCameraState(const std::string& strIdentity, const Task::seqCameraState& stateVideo);

private:
    void hold_heartbeat(const std::string& strId, Ice::ConnectionPtr conn);
    void update_conn_list(const std::string& strId);
    void heartbeat_callback(const ::std::shared_ptr<Ice::Connection>& con);
    void push_task_result(const Task::TaskResult& result);
    void update_camera_state(const std::string& strIdentity, const Task::seqCameraState& stateVideo);

public:
    //client control
    class TaskClient
    {
    public:
        ~TaskClient();
        bool Init(const std::string& strId, const Ice::Current &curr);
        void PushTaskResult(const Task::TaskResult& result);
        void UpdateCameraState(const Task::seqCameraState& stateCamera);

    private:
#ifdef ICE_CPP11_MAPPING
        std::shared_ptr<MyIceDemo::PrinterClientPrx> m_pClient;
#else
        Task::TaskClientPrx m_pClient;
#endif // ICE_CPP11_MAPPING

    };

#ifdef ICE_CPP11_MAPPING
    class ClientCloseCallback
    {
    public:
        ClientCloseCallback() {}
        ClientCloseCallback(const std::string& strId, iceServerImpl* pServer)
            :m_strId(strId)
            ,m_pServer(pServer)
        {
        }
        void closed(const ::Ice::ConnectionPtr& conn)
        {
            std::cout << m_strId << " IceClientDuplex closed.";
            m_pServer->update_conn_list(m_strId);
        }
    private:
        CompareServerImpl* m_pServer
        std::string m_strId;
    };
#else
    //heartbeat
    class ClientHeartbeatCallback : public Ice::HeartbeatCallback
    {
    public:
        ClientHeartbeatCallback() {}
        virtual void heartbeat(const ::Ice::ConnectionPtr& conn) override
        {
#ifdef PRINT_HEARTBEAT
            Ice::IPConnectionInfoPtr ip_info
                = Ice::IPConnectionInfoPtr::dynamicCast(conn->getInfo());
            if (ip_info != NULL) {
                std::cout << time(0) << "[heartbeat]" << ip_info->remoteAddress
                    << ":" << ip_info->remotePort << std::endl;
            }
#endif  // PRINT_HEARTBEAT
        }
    };
    //close callback
    class ClientCloseCallback : public Ice::CloseCallback
    {
    public:
        ClientCloseCallback(const std::string& strId, TaskServerImpl *pServer)
            : m_strId(strId)
            , m_pServer(pServer)
        {
            assert(m_pServer);
        }
        virtual void closed(const ::Ice::ConnectionPtr& conn) override
        {
            std::cout << m_strId << " IceClientDuplex closed.";
            m_pServer->update_conn_list(m_strId);
        }
    private:
        TaskServerImpl *m_pServer;
        std::string m_strId;
    };
#endif // ICE_CPP11_MAPPING

    typedef std::shared_ptr<TaskClient> ClientPtr;

private:
    static TaskServerImpl* m_pThis;
    std::mutex m_mtLock;
    std::map<std::string, ClientPtr>m_mapClient;


};
