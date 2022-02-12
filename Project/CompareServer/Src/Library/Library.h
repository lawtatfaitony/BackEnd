/**************************************************************
* @brief:       单个底库管理
* @date:        20200131
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
#include <mutex>
#include <map>
#include <vector>
#include <atomic>



class LibraryHandle
{
public:
    LibraryHandle();
    ~LibraryHandle();
    LibraryHandle(const std::string& strStorageDir, int nLibId);


    void DeleteStorageFile();
    void AddPerson(int64_t nPersonId, const std::shared_ptr<float>& pFeature);
    void DeletePerson(int64_t nPersonId);
    void QueryTopN(const std::shared_ptr<float>& pFeature, float nThreold, std::vector<std::pair<int64_t, float>>& vecMatch, int nTop = 1);
    int64_t GetMaxPersonId();

private:
    void load_library_data();
    void save();

private:
    std::string m_strStorageDir;
    int m_nLibId;
    std::string m_strFileName;
    // person of library
    std::mutex m_mtPerson;
    // pair<person_id, feature>
    std::map<int64_t, std::shared_ptr<float>> m_mapPerson;
    // FaceSdk
    void* m_pFaceSdk;
    int m_nFeatureSize;

};