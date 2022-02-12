#pragma once
#include <thread>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include "Basic.h"


NAMESPACE_BASIC_BEGIN
class ThreadObject
{
    using Func = std::function<void()>;
    const static int kInfinite = -1;

public:
    ThreadObject()
    {
        m_bStop.store(false);
    }

    ~ThreadObject()
    {
        Stop();
    }

    void Start(Func func, int nWaitMSeond = kInfinite)
    {
        do_work(func, nWaitMSeond);
    }

    void StartOnce(Func func)
    {
        Stop();
        m_bStop.store(false);
        m_thWork = std::thread([=] {
            if (func)
            {
                func();
            }
            });
    }

    void StartAndWait(Func func, int nWaitMSeond = kInfinite)
    {
        do_work(func, nWaitMSeond, true);
    }

    void Stop()
    {
        m_bStop.store(true);
        m_cvCond.notify_one();
        if (m_thWork.joinable())
            m_thWork.join();
    }

    void SetEvent()
    {
        m_cvCond.notify_one();
    }

    bool IsRunning()
    {
        return (!m_bStop.load());
    }


private:
    ThreadObject(const ThreadObject&) = delete;
    ThreadObject& operator=(const ThreadObject&) = delete;

    ThreadObject(const ThreadObject&&) = delete;
    ThreadObject& operator=(const ThreadObject&&) = delete;


private:
    void do_work(Func func, int nWaitMSeond, bool bWait = false)
    {
        Stop();
        m_bStop.store(false);
        m_bWork = !bWait;
        m_thWork = std::thread([=] {
            while (!this->m_bStop.load())
            {
                if (this->m_bWork.load() && func)
                {
                    func();
                }
                if (!this->m_bWork.load())
                {
                    this->m_bWork = true;
                }
                if (!this->m_bStop.load() && nWaitMSeond != 0)
                {
                    std::unique_lock<std::mutex> lock(m_mtLock);
                    m_cvCond.wait_for(lock, std::chrono::milliseconds(nWaitMSeond));
                }
            }
            }
        );
    }


private:
    std::atomic<bool> m_bStop;
    std::atomic<bool> m_bWork;
    std::condition_variable m_cvCond;
    std::mutex m_mtLock;
    std::thread m_thWork;

};


NAMESPACE_BASIC_END