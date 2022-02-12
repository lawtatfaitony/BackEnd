/**************************************************************
*
* @brief:       map·â×°
* @date:        20190705
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <map>
#include <mutex>
#include <memory>
#include "Basic.h"

NAMESPACE_BASIC_BEGIN
template <class Key, class T>
class SafeMap
{
public:
    typedef std::pair<Key, T> PairType;
    typedef std::map<Key, T> MapType;
	typedef std::shared_ptr<std::lock_guard<std::mutex>> LockerPtr;

public:
    SafeMap() {}
    ~SafeMap() {}
    SafeMap(const SafeMap&) = delete;
    SafeMap& operator=(const SafeMap&) = delete;

public:
    MapType& Fetch(LockerPtr& pLocker)
    {
        pLocker.reset(new std::lock_guard<std::mutex>(m_mtLock));
        return m_mapData;
    }

    int Size()
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        return m_mapData.size();
    }
    void Insert(const Key& key, const T& value)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_mapData.insert(PairType(key, value));
    }

    void Erase(const Key& key)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_mapData.erase(key);
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        return m_mapData.empty();
    }

    bool Exist(const Key& key)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        return m_mapData.find(key) != m_mapData.end();
    }

    bool GetValue(const Key& key, T& value)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        auto iter = m_mapData.find(key);
        if (iter == m_mapData.end()) return false;
        value = iter->second;
        return true;
    }

    void UpdateValue(const Key& key, const T& value)
    {
        std::lock_guard<std::mutex> lock (m_mtLock);
        m_mapData[key] = value;
    }

    MapType SnapShot()
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        return m_mapData;
    }

    void Swap(MapType& mapData)
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_mapData.swap(mapData);
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mtLock);
        m_mapData.clear();
    }

private:
    MapType m_mapData;
    std::mutex m_mtLock;

};
NAMESPACE_BASIC_END
