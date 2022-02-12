#pragma once
#include <Ice/Ice.h>
#include <Ice/Initialize.h>
#include <Ice/Application.h>
#include <Ice/Object.h>
#include <Ice/ObjectAdapter.h>
#include <string>


struct DefaultProxyProperty {
    std::string retry_intervals = "0 100";
    std::string message_size_max = "20480";
    std::string override_timeout = "10000";
    std::string override_compress = "0";
    std::string override_connect_timeout = "1000";
    std::string acm_client_close = "3";
    std::string acm_client_heartbeat = "1";
    std::string acm_client_timeout = "60";
};

template <class TIcePrx>
class SimpleIce
{
private:
    const std::string kDefaultProperty = "Default.proxy";
public:
    SimpleIce()
    {
        m_pCommunicator = nullptr;
        m_pPrx = nullptr;
    }

    ~SimpleIce()
    {
        CleanUp();
    }

    void Init(const std::string& strIP, int nPort, const std::string& strIdetity
    ,const DefaultProxyProperty& propProxy)
    {
        assert(!strIP.empty() && nPort != 0 && !strIdetity.empty());
        try
        {
            Ice::PropertiesPtr props = Ice::createProperties();
            props->setProperty("Ice.RetryIntervals", propProxy.retry_intervals);
            props->setProperty("Ice.MessageSizeMax", propProxy.message_size_max);
            props->setProperty("Ice.Override.ConnectTimeout", propProxy.override_connect_timeout);
            props->setProperty("Ice.Override.Timeout", propProxy.override_timeout);
            props->setProperty("Ice.Override.Compress", propProxy.override_compress);
            props->setProperty("Ice.ACM.Client.Close", propProxy.acm_client_close);
            props->setProperty("Ice.ACM.Client.Heartbeat", propProxy.acm_client_heartbeat);
            props->setProperty("Ice.ACM.Client.Timeout", propProxy.acm_client_timeout);
            char szProperty[100] = { 0 };
            sprintf_s(szProperty, 128, "%s:default -h %s -p %u", strIdetity.c_str(), strIP.c_str(), nPort);
            props->setProperty(kDefaultProperty, szProperty);
            Ice::InitializationData initData;
            initData.properties = props;
            comm_ptr_ = Ice::initialize(initData);
            m_strProxyName = kDefaultProperty;
        }
        catch (const Ice::Exception& e)
        {
            std::cout << "Init ice exception:" << e.what;
        }
    }

    void Init(const std::string& strConfigName, const std::string& strProxyName)
    {
        assert(!strProxyName.empty());
        Ice::PropertiesPtr pProper = Ice::createProperties();      
        pProper->load(strConfigName);
        
        Ice::InitializationData initData;
        initData.properties = pProper;
        m_pCommunicator = Ice::initialize(initData);

        m_strProxyName = strProxyName;
    }

#ifdef ICE_CPP11_MAPPING // C++11 mapping
    std::shared_ptr<TIcePrx> GetProxy()
#else
    TIcePrx GetProxy()
#endif
    {
        CheckConnected();
        return m_pPrx;
    }

    bool CheckConnected()
    {
        bool conn= false;
        if (m_pPrx)
        {
            try
            {
                m_pPrx->ice_invocationTimeout(10)->ice_ping();
                conn = true;
            }
            catch (const Ice::Exception& e)
            {
                std::cout << "Check connected exception:" << e;
                conn = false;
            }
        }
        else
        {
            conn = ConnectServer();
        }
        return conn;
    }

    bool ConnectServer()
    {
        bool success = true;
        if (m_pPrx)return success;
        try
        {
            m_pPrx = Ice::checkedCast<TIcePrx>(m_pCommunicator->propertyToProxy(m_strProxyName));
            m_pPrx->ice_ping();
            std::cout << typeid(m_pPrx).name() << " Ice client connect success." 
                << m_pPrx->ice_getConnection()->toString();
        }
        catch (const Ice::Exception& e)
        {
            std::cout << e;
            success = false;
        }
        return success;
    }

    void CleanUp()
    {
        try
        {
            if (m_pPrx) 
            {
                std::string strConnInfo(m_pPrx->ice_getConnection()->toString());
#ifdef ICE_CPP11_MAPPING // C++11 mapping
                m_pPrx->ice_getConnection()->close(Ice::ConnectionClose::GracefullyWithWait);
#else
                m_pPrx->ice_getConnection()->close(Ice::ConnectionClose::ConnectionCloseGracefullyWithWait);
#endif
                m_pPrx = nullptr;
                std::cout << typeid(TIcePrx).name() << " Ice client disconnect success. " << strConnInfo;
            }
            if (m_pCommunicator)
            {
                m_pCommunicator->destroy();
            }
        }
        catch (const Ice::Exception& e)
        {
            std::cout << e;
        }       
    }


private:
    Ice::CommunicatorPtr m_pCommunicator;
    std::string m_strProxyName;
    
#ifdef ICE_CPP11_MAPPING
    std::shared_ptr<TIcePrx>m_pPrx;
#else
    TIcePrx m_pPrx;
#endif // ICE_CPP11_MAPPING
};