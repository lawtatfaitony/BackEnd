#pragma once
#include <map>
#include <string>
#include <mutex>

namespace Service
{
    class SessionMangement
    {
        const int kClearInvalidSession = 60 * 1000;
        SessionMangement();

    public:
        static SessionMangement& Instance();
        ~SessionMangement();
        bool IsValidSession(const std::string& strSession);
        void PushSession(const std::string& strSession);

        private:
            void clear_invalid_session();

    private:
        std::mutex m_mtSession;
        std::map<std::string, time_t> m_mapSession;

    };
}
