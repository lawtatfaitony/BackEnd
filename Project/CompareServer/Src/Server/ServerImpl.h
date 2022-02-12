/**************************************************************
* @brief:       compare server
* @date:        20200131
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <Ice/Ice.h>
#include <Compare.h>
#include <map>
#include <memory>
#include <mutex>


class CompareServerImpl : public Compare::CompareServer
{
public:
    CompareServerImpl();
    ~CompareServerImpl();

public:
    virtual void RegisterClient(const std::string& strId, const Ice::Current & current);
    // library
    virtual int AddLibrary(int nLibId,
        std::string& strResult, 
        const Ice::Current& current);
    virtual int DeleteLibrary(int nLibId, const Ice::Current & current);
    virtual int ListLibrary(std::string& strResult, const Ice::Current & current);
    // person
    virtual int AddPerson(int nLibId, const std::string& strPicUrl,
        std::string& strResult, 
        const Ice::Current & current);
    virtual int DeletePerson(int nLibId, int64_t nPersonId, const Ice::Current & current);
    virtual int UpdatePersonPicture(int nLibId, 
        int64_t nPersonId, 
        const std::string& strPicUrl,
        std::string& strResult,
        const Ice::Current & current);
    // compare
    virtual int GetFeatureSize(const Ice::Current & current);
    virtual int ExtractFeature(const std::string& strPicUrl, 
        std::string& strFeature, 
        const Ice::Current & current);
    virtual int Compare1v1(const std::string& strLPicUrl,
        const std::string& strRPicUrl,
        float& nSimilarity,
        const Ice::Current & current);
    virtual int CompareWithFeature(const std::string& strLFerature,
        const std::string& strRFerature,
        float& nSimilarity,
        const Ice::Current & current);
    // task
    virtual int PushCompareTask(const Compare::CompareTask& task, const Ice::Current & current);


public:
    static void PushCompareResult(const Compare::CompareResult& result);


private:
    void hold_heartbeat(const std::string& strId, Ice::ConnectionPtr conn);
    void update_conn_list(const std::string& strId);
    void heartbeat_callback(const ::std::shared_ptr<Ice::Connection>& con);
    void push_compare_result(const Compare::CompareResult& result);

public:
    //client control
    class CompareClient
    {
    public:
        ~CompareClient();
        bool Init(const std::string& strId, const Ice::Current &curr);
        void PushCompareResult(const Compare::CompareResult& result);

    private:
#ifdef ICE_CPP11_MAPPING
        std::shared_ptr<MyIceDemo::PrinterClientPrx> m_pClient;
#else
        Compare::CompareClientPrx m_pClient;
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
        ClientCloseCallback(const std::string& strId, CompareServerImpl *pServer)
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
        CompareServerImpl *m_pServer;
        std::string m_strId;
    };
#endif // ICE_CPP11_MAPPING


private:
    static CompareServerImpl* m_pThis;
    std::mutex m_mtLock;
    std::map<std::string, std::shared_ptr<CompareClient>>m_mapConns;


};
