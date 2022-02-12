#pragma once
#include <mutex>
#include <list>
#include <condition_variable>
#include <functional>
#include <assert.h>
#include "Basic.h"


NAMESPACE_BASIC_BEGIN
template<class TData>
class MessageQueue
{
    using CallbackSignal = std::function<void(const TData& data)>;
    using CallbackTimeout = std::function<void()>;

    const static int kInfinite = -1;
public:
    MessageQueue()
        : m_bStop(false)
    {

    }

    ~MessageQueue()
    {
        Stop();
    }

    void Start(CallbackSignal cbNotify)
    {
        assert(cbNotify);
        Stop();
        m_bStop.store(false);
        m_thRun = std::thread([=] {
            while (!this->m_bStop.load())
            {
                std::mutex mt;
                std::unique_lock<std::mutex> lock(mt);
                m_cvSignal.wait(lock);
                TData tmpData;
                if (cbNotify && PopFront(tmpData))
                {
                    cbNotify(tmpData);
                }
            }
            }
        );
    }

    void Start(CallbackSignal cbSignal, CallbackTimeout cbTimeout, int nWaitMSecond)
    {
        assert(cbSignal);
        Stop();
        m_bStop.store(false);
        m_thRun = std::thread([=] {
            while (!this->m_bStop.load())
            {
                std::mutex mt;
                std::unique_lock<std::mutex> lock(mt);
                bool bSucc = m_cvSignal.wait_for(lock, std::chrono::milliseconds(nWaitMSecond), [=] {
                    return !this->m_listData.empty() || this->m_bStop.load(); }
                );
                if (this->m_bStop.load()) break;
                if (bSucc)
                {
                    TData tmpData;
                    if (cbSignal && PopFront(tmpData))
                    {
                        cbSignal(tmpData);
                    }
                }
                else
                {
                    if (cbTimeout)
                    {
                        cbTimeout();
                    }
                }
            }
            }
        );
    }

    void Stop()
    {
        m_bStop.store(true);
        m_cvSignal.notify_one();
        if (m_thRun.joinable())
        {
            m_thRun.join();
        }
    }

    void PushFront(const TData& data)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_listData.emplace_front(data);

        m_cvSignal.notify_one();
    }

    bool PopFront(TData& data)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        if (m_listData.empty()) return false;
        data = std::move(m_listData.front());
        m_listData.pop_front();

        return true;
    }

    void PushBack(const TData& data)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_listData.emplace_back(data);

        m_cvSignal.notify_one();
    }

    bool PopBack(TData& data)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        if (m_listData.empty()) return false;
        data = std::move(m_listData.back());
        m_listData.pop_back();

        return true;
    }


private:
    MessageQueue(const MessageQueue&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;

    MessageQueue(const MessageQueue&&) = delete;
    MessageQueue& operator=(const MessageQueue&&) = delete;


private:
    std::atomic<bool> m_bStop;
    std::mutex m_mtLock;
    std::list<TData> m_listData;
    std::condition_variable m_cvSignal;
    std::thread m_thRun;


};

NAMESPACE_BASIC_END