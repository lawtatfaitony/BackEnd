/**************************************************************
*
* @brief:       list封装
* @date:        20201221
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <list>
#include <mutex>
#include <memory>
#include "Basic.h"

NAMESPACE_BASIC_BEGIN
template <class T>
class SafeList
{
public:
    typedef std::list<Key, T> ListData;
    typedef std::shared_ptr<std::lock_guard<std::mutex>> LockerPtr;

public:
    SafeList() {}
    ~SafeList() {}
    SafeList(const SafeList&) = delete;
    SafeList& operator=(const SafeList&) = delete;

public:
    ListData& Fetch(LockerPtr& pLocker)
    {
        pLocker.reset(new std::lock_guard<std::mutex>(m_mtLock));
        return m_listData;
    }

    int Size()
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        return m_listData.size();
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        return m_listData.empty();
    }

    void PushFront(const T& value)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_listData.push_front(value);
    }

    bool PopFront(T& value)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        if (m_listData.empty()) return false;
        value = m_listData.front();
        m_listData.pop_front();

        return true;
    }

    void PushBack(const T& value)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_listData.push_back(value);
    }

    bool PopBack(T& value)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        if (m_listData.empty()) return false;
        value = m_listData.back();
        m_listData.pop_back();

        return true;
    }

    void Swap(ListData& listData)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_listData.swap(listData);
    }

    ListData SnapShot()
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        return m_listData;
    }

    void Clear()
    {
        std::lock_guard<std::mutex>lock(m_mtLock);
        m_listData.clear();
    }


private:
    ListData m_listData;
    std::mutex m_mtLock;

};
NAMESPACE_BASIC_END
