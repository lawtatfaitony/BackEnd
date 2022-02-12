#include "DatabaseConnPool.h"
#include "../../Config/GlobalConfig.h"
#include <easylogging/easylogging++.h>


DatabaseConnPool::DatabaseConnPool()
    : m_poolConn(CFG_DATABASE.nDatabasePoolSize)
{
}

DatabaseConnPool& DatabaseConnPool::Instance()
{
    static DatabaseConnPool g_Instance;
    return g_Instance;
}

DatabaseConnPool::~DatabaseConnPool()
{

}

soci::session& DatabaseConnPool::FetchConnInstance(size_t& nPos, int nTimeout)
{
    if (nTimeout > 0)
    {
        if (!m_poolConn.try_lease(nPos, nTimeout))
        {
            nPos = kInvalidConnInstance;
            return soci::session();
        }
    }

    // get conn, it locked at nPos in connection pool
    return m_poolConn.at(nPos);
}

soci::connection_pool& DatabaseConnPool::GetConnPool()
{
    return m_poolConn;
}

int DatabaseConnPool::GivebackConn(int nCode, size_t nPos)
{
    // unlock the nPos of connection pool
    m_poolConn.give_back(nPos);
    return nCode;
}

bool DatabaseConnPool::Init()
{
    // init the connections
    for (int nIndex = 0; nIndex < CFG_DATABASE.nDatabasePoolSize; ++nIndex)
    {
        soci::session& conn = m_poolConn.at(nIndex);
        try
        {
            conn.open(CFG_DATABASE.strConnFrontendBase);
        }
        catch (const std::exception& e)
        {
            LOG(ERROR) << "Init the connection pool of database exception:" << e.what();
            return false;
        }
    }
    LOG(INFO) << "Init connect pool of database success";
    return true;
}
