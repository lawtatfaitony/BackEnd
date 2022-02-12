#pragma once
#pragma once
#include <stdint.h>
#include <string>
#include <thread>
#include <functional>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/Connection.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/UUID.h>
#include "ThreadObject.h"


namespace IceClient 
{

    struct DuplexProxyProperty 
    {
        std::string retry_intervals = "-1";
        std::string message_size_max = "20480";
        std::string override_timeout = "10000";
        std::string override_compress = "0";
        std::string override_connect_timeout = "1000";
        std::string acm_m_clientclose = "0";
        std::string acm_m_clientheartbeat = "3";
        std::string acm_m_clienttimeout = "10";
    };


    template<class TIcePrx>
    class IceClientDuplex
    {
    public:
        static const int kReconnectionInterval = 2000;
#ifdef ICE_CPP11_MAPPING
        typedef std::function<void(const std::shared_ptr<TIcePrx>&, const std::string& strId)> AddClientFunc;
        class ClientCloseCallback
        {
        public:
            ClientCloseCallback(IceClientDuplex* pClient)
                :m_pClient(pClient)
            {
            }
            void closed(const ::Ice::ConnectionPtr& conn)
            {
                m_pClient->set_online(false);
                m_pClient->conn_start();
            }
        private:
            IceClientDuplex *m_pClient;
        };
#else
        typedef std::function<void(const TIcePrx&, const std::string& strId)> AddClientFunc;
    private:
        // HeartbeatCallback
        class ClientHeartbeatCallback : public Ice::HeartbeatCallback
        {
        public:
            ClientHeartbeatCallback(IceClientDuplex *client)
                : m_client(client) 
            {
                assert(m_client);
            }
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
        private:
            IceClientDuplex *m_client;
        };

        // CloseCallback
        class ClientCloseCallback : public Ice::CloseCallback
        {
        public:
            ClientCloseCallback(IceClientDuplex *client)
                : m_client(client) 
            {
                assert(m_client);
            }
            virtual void closed(const ::Ice::ConnectionPtr& conn) override 
            {
                std::cout << typeid(TIcePrx).name() << " IceClientDuplex closed.";
                m_client->set_online(false);
                m_client->conn_start();
            }
        private:
            IceClientDuplex *m_client;
        };
#endif // ICE_CPP11_MAPPING

    public:
        // Constructor
        IceClientDuplex()
            : m_add_clientfunc(nullptr)
            , m_pPrx(NULL)
            , m_pCommuictor(NULL)
            , m_pAdapter(NULL)
            , m_bOnLine(false) 
        {
        }
        // Destructor
        virtual ~IceClientDuplex() 
        {
            Cleanup();
        }

    public:
        /* @brief Init
        * @param servant       The servant to add.
        * @param func          The function for add client to server.
        * @param config        The config filename.
        * @param proxy_name    The proxy name in configuration file.
        * @param category      The category of ice client-side.
        */
        bool Init(const Ice::ObjectPtr& servant, AddClientFunc func,
            const std::string& config, const std::string& proxy_name,
            const std::string& category = std::string()) 
        {
            assert(!m_pCommuictor);
            assert(servant && !config.empty() && !proxy_name.empty());
            bool bSuccess(false);
            if (m_pCommuictor) 
            {
                std::cout << typeid(TIcePrx).name()
                    << " IceClientDuplex init error, has been initialized.";
                return bSuccess;
            }
            m_add_clientfunc = func;
            try
            {
                Ice::PropertiesPtr props = Ice::createProperties();
                props->load(config);
                Ice::InitializationData initData;
                initData.properties = props;
                m_pCommuictor = Ice::initialize(initData);
                m_strProxyName = proxy_name;
                m_identity.name = IceUtil::generateUUID();
                m_identity.category = category;
                m_pAdapter = m_pCommuictor->createObjectAdapter("");
                m_pAdapter->add(servant, m_identity);
                m_pAdapter->activate();
                std::cout << typeid(TIcePrx).name() << " IceClientDuplex init success\n";
                // Automatic start connection.
                conn_start();
                bSuccess = true;
            }
            catch (const Ice::Exception& e)
            {
                std::cout << "IceClientDuplex init failed,error:" << e.what() << std::endl;
            }          
            return bSuccess;
        }
        /* @brief Init
        * @param servant   The servant to add.
        * @param func      The function for add client to server.
        * @param ip        The ip of ice server.
        * @param port      The port of ice server.
        * @param identity  The identity of the Ice object that is implemented by the servant.
        * @param config    The property config filename.
        * @param category  The category of ice client-side.
        */
        bool Init(const Ice::ObjectPtr& servant, AddClientFunc func,
            const std::string& ip, uint16_t port, const std::string& identity,
            const std::string& config, const std::string& category = std::string()) 
        {
            assert(!m_pCommuictor);
            assert(servant && !ip.empty() && port != 0 && !identity.empty() && !config.empty());
            bool bSuccess(false);
            if (m_pCommuictor) 
            {
                std::cout << typeid(TIcePrx).name()
                    << " IceClientDuplex init error, has been initialized.";
                return bSuccess;
            }
            m_add_clientfunc = func;
            try
            {
                Ice::PropertiesPtr props = Ice::createProperties();
                props->load(config);
                props->setProperty(pProxyName, get_remote_endpoint(ip, port, identity));
                Ice::InitializationData initData;
                initData.properties = props;
                m_pCommuictor = Ice::initialize(initData);
                m_strProxyName = pProxyName;
                m_identity.name = IceUtil::generateUUID();
                m_identity.category = category;
                m_pAdapter = m_pCommuictor->createObjectAdapter("");
                m_pAdapter->add(servant, m_identity);
                m_pAdapter->activate();
                std::cout << typeid(TIcePrx).name() << " IceClientDuplex init success\n";

                conn_start();
                bSuccess = true;
            }
            catch (const Ice::Exception& e)
            {
                std::cout << "IceClientDuplex init failed,error:" << e.what() << std::endl;
            }
            return bSuccess;           
        }
        /* @brief Init
        * @param servant   The servant to add.
        * @param func      The function for add client to server.
        * @param ip        The ip of ice server.
        * @param port      The port of ice server.
        * @param identity  The identity of the Ice object that is implemented by the servant.
        * @param prop      The connection properties.
        * @param category  The category of ice client-side.
        */
        bool Init(const Ice::ObjectPtr& servant, AddClientFunc func,
            const std::string& ip, uint16_t port, const std::string& identity,
            const DuplexProxyProperty& prop = DuplexProxyProperty(),
            const std::string& category = std::string()) 
        {
            assert(!m_pCommuictor);
            assert(servant && !ip.empty() && port != 0 && !identity.empty());
            bool bSuccess(false);
            if (m_pCommuictor) 
            {
                std::cout << typeid(TIcePrx).name()
                    << " IceClientDuplex init error, has been initialized.";
                return bSuccess;
            }
            m_add_clientfunc = func;
            try
            {
                Ice::PropertiesPtr props = Ice::createProperties();
                props->setProperty("Ice.RetryIntervals", prop.retry_intervals);
                props->setProperty("Ice.MessageSizeMax", prop.message_size_max);
                props->setProperty("Ice.Override.ConnectTimeout", prop.override_connect_timeout);
                props->setProperty("Ice.Override.Timeout", prop.override_timeout);
                props->setProperty("Ice.Override.Compress", prop.override_compress);
                props->setProperty("Ice.ACM.Client.Close", prop.acm_m_clientclose);
                props->setProperty("Ice.ACM.Client.Heartbeat", prop.acm_m_clientheartbeat);
                props->setProperty("Ice.ACM.Client.Timeout", prop.acm_m_clienttimeout);
                props->setProperty(pProxyName, get_remote_endpoint(ip, port, identity));
                Ice::InitializationData initData;
                initData.properties = props;
                m_pCommuictor = Ice::initialize(initData);
                m_strProxyName = pProxyName;
                m_identity.name = IceUtil::generateUUID();
                m_identity.category = category;
                m_pAdapter = m_pCommuictor->createObjectAdapter("");
                m_pAdapter->add(servant, m_identity);
                m_pAdapter->activate();
                ICE_DUPLEX_LOG(INFO) << typeid(TIcePrx).name() << " IceClientDuplex init success\n";
                // Automatic start connection.
                conn_start();
                bSuccess = true;
            }  
            catch (const Ice::Exception& e)
            {
                std::cout << "IceClientDuplex init failed,error:" << e.what() << std::endl;
            }
            return bSuccess;
        }

        /* @brief Cleanup
        */
        void Cleanup() 
        {
            // Stop the connection thread.
            m_thread.Stop();
            // Destruction of resources.
            try
            {
                set_online(false);
                if (m_pPrx) 
                {
                    std::string strConnInfo = m_pPrx->ice_getConnection()->toString();
#ifdef ICE_CPP11_MAPPING // C++11 mapping
                    m_pPrx->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);
#else
                    m_pPrx->ice_getConnection()->close(Ice::ConnectionClose::ConnectionCloseGracefullyWithWait);
#endif                   
                    m_pPrx = NULL;
                    std::cout << typeid(TIcePrx).name()
                        << " IceClientDuplex disconnect success. " << strConnInfo;
                }
                if (m_pAdapter) 
                {
                    m_pAdapter->destroy();
                    m_pAdapter = NULL;
                }
                if (m_pCommuictor) 
                {
                    m_pCommuictor->destroy();
                    m_pCommuictor = NULL;
                }
                std::cout << typeid(TIcePrx).name() << " IceClientDuplex cleanup success.";
            }
            catch (const Ice::Exception& e)
            {
                m_add_clientfunc = nullptr;
                std::cout << "IceClientDuplex cleanup failed,error:" << e.what() << std::endl;
            }       
        }

    public:
        // Get client proxy
#ifdef ICE_CPP11_MAPPING
        std::shared_ptr<TIcePrx> GetProxy()
#else
        TIcePrx GetProxy()
#endif // ICE_CPP11_MAPPING
        {
            assert(m_pCommuictor);
            return m_pPrx;
        }

        // Get servant
        Ice::ObjectPtr GetServant() 
        {
            assert(m_pAdapter);
            return m_pAdapter->find(m_identity);
        }

        // Get ice identity
        const Ice::Identity& GetIdentity() 
        {
            return m_identity;
        }

        // Check connection status
        bool CheckConnStatus() 
        {
            return m_bOnLine;
        }
        void Stop()
        {
            m_thread.Stop();
        }
    private:
        inline void set_online(bool bOnLine) 
        {
            m_bOnLine = bOnLine;
        }

        inline std::string get_remote_endpoint(const std::string& strIp, uint16_t nPort,
            const std::string& strId)
        {
            char szBuffer[128];
            sprintf_s(szBuffer, 128, "%s:default -h %s -p %u", strId.c_str(), strIp.c_str(), nPort);
            return szBuffer;
        }

        bool conn_start()
        {
            assert(m_pCommuictor);
            m_thread.Start([this]()->bool {return connect();}, kReconnectionInterval);
            return true;
        }

        bool connect()
        {
            if (!m_pCommuictor) 
                return false;
            if (m_bOnLine) 
                return true;
            try
            {
                // The proxy objects need to be built only when the connection is first established.
                // After the connection is broken, you need to restore it.
                bool recovery = (m_pPrx != 0);
                if (!m_pPrx) 
                    m_pPrx = Ice::checkedCast<TIcePrx>(m_pCommuictor->propertyToProxy(m_strProxyName));
                m_pPrx->ice_invocationTimeout(500);
                m_pPrx->ice_ping();
                m_pPrx->ice_getConnection()->setAdapter(m_pAdapter);
                if (m_add_clientfunc != nullptr)
                    m_add_clientfunc(m_pPrx, m_identity.name);
                hold_heartbeat(m_pPrx->ice_getConnection());
                set_online(true);
                std::cout << typeid(TIcePrx).name()
                    << (recovery ? " IceClientDuplex recovery connection success. "
                        : " IceClientDuplex connection success. \n")
                    << m_pPrx->ice_getConnection()->toString();
                return true;
            }
            catch (...) 
            {
                set_online(false);
            }
            return false;
        }

        void hold_heartbeat(Ice::ConnectionPtr conn) 
        {
#ifdef ICE_CPP11_MAPPING
            auto heart_beat_func = [](const Ice::ConnectionPtr& conn){};
            conn->setHeartbeatCallback(heart_beat_func);
            std::shared_ptr<ClientCloseCallback>pClientCallback(new ClientCloseCallback(this));
            conn->setCloseCallback(std::bind(&ClientCloseCallback::closed, pClientCallback, conn));
#else
            conn->setHeartbeatCallback(new ClientHeartbeatCallback(this));
            conn->setCloseCallback(new ClientCloseCallback(this));
#endif // ICE_CPP11_MAPPING

            
            // If a configuration exists, the value of the configuration is taken.
            Ice::ACM acm = conn->getACM();
            if (acm.heartbeat != Ice::ACMHeartbeat::HeartbeatAlways
                || acm.close != Ice::ACMClose::CloseOff) 
            {
                // A heartbeat is sent to the server every 10/2 s.
                conn->setACM(
                    IceUtil::Optional<Ice::Int>(10),
                    IceUtil::Optional<Ice::ACMClose>(Ice::ACMClose::CloseOff),
                    IceUtil::Optional<Ice::ACMHeartbeat>(Ice::ACMHeartbeat::HeartbeatAlways));
            }
        }

    private:
        const char *pProxyName = "DefaultDuplexIceServer.Proxy";

    private:
        std::string m_strProxyName;
        AddClientFunc m_add_clientfunc;
        // The communicator and client proxy.
#ifdef ICE_CPP11_MAPPING
        std::shared_ptr<TIcePrx> m_pPrx;
#else
        TIcePrx m_pPrx;
#endif
        Ice::CommunicatorPtr m_pCommuictor;
        // The identity of the Ice object that is implemented by the servant.
        Ice::Identity m_identity;
        // The adapter for duplex communication.
        Ice::ObjectAdapterPtr m_pAdapter;
        // The connection checking thread.
        volatile bool m_bOnLine;
        Basic::ThreadObject m_thread;
    };

}