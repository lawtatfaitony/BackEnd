/**************************************************************
* @brief:       connection-pool of database
* @date:         20200409
* @update:
* @auth:         Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <soci/soci.h>

static const int kInvalidConnInstance = -1;
class DatabaseConnPool
{
private:
    DatabaseConnPool();

public:
    static DatabaseConnPool& Instance();
    ~DatabaseConnPool();
    soci::session& FetchConnInstance(size_t& nPos, int nTimeout = 3000);
    soci::connection_pool& GetConnPool();
    int GivebackConn(int nCode ,size_t nPos);
    bool Init();

private:
    soci::connection_pool m_poolConn;

};