#include "session.h"
#include <Macro.h>

using namespace Service;
SessionMangement::SessionMangement()
{

}

SessionMangement& SessionMangement::Instance()
{
    static SessionMangement g_instance;
    return g_instance;
}

SessionMangement::~SessionMangement()
{

}
bool SessionMangement::IsValidSession(const std::string& strSession)
{
    // for now, return true
    return true;
    SCOPE_LOCK(m_mtSession);
    return m_mapSession.find(strSession)!= m_mapSession.end();
}

void SessionMangement::PushSession(const std::string& strSession)
{
    SCOPE_LOCK(m_mtSession);
    m_mapSession[strSession] = time(nullptr);
}

void SessionMangement::clear_invalid_session()
{
    // for now, do nothing
}