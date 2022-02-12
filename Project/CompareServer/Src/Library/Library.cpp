#include "Library.h"
#include <algorithm>
#include <FaceSdk/FaceSdk.h>
#include <easylogging/easylogging++.h>
#include <Basic/Stream.h>
#include <Basic/CrossPlat.h>



LibraryHandle::LibraryHandle()
{

}

LibraryHandle::LibraryHandle(const std::string& strStorageDir, int nLibId)
    : m_strStorageDir(strStorageDir)
    , m_nLibId(nLibId)
    , m_pFaceSdk(nullptr)
{
    auto nCode = CreateInstance(m_pFaceSdk);
    if (nCode != kFaceResultOk)
    {
        LOG(ERROR) << "Create facesdk instance:" << nCode;
        return;
    }
    GetFeatureSize(m_pFaceSdk, m_nFeatureSize);
    auto funcGenerateDir = [=]() {
        return m_strStorageDir + std::to_string(m_nLibId) + ".data";
    };
    m_strFileName = funcGenerateDir();
    load_library_data();
}

LibraryHandle::~LibraryHandle()
{
    save();
    DestroyInstance(m_pFaceSdk);
}

void LibraryHandle::DeleteStorageFile()
{
    CG_remove(m_strFileName.c_str());
}

void LibraryHandle::AddPerson(int64_t nPersonId, const std::shared_ptr<float>& pFeature)
{
    std::lock_guard<std::mutex> lock(m_mtPerson);
    m_mapPerson[nPersonId] = pFeature;
}

void LibraryHandle::DeletePerson(int64_t nPersonId)
{
    std::lock_guard<std::mutex> lock(m_mtPerson);
    m_mapPerson.erase(nPersonId);
}

void LibraryHandle::QueryTopN(const std::shared_ptr<float>& pFeature,
    float nThreold, 
    std::vector<std::pair<int64_t, float>>& vecMatch,
    int nTop)
{
    // compare with person in library
    std::vector<std::pair<int64_t, float>> result(m_mapPerson.size());
    {
        std::unique_lock<std::mutex> _locker(m_mtPerson);
        size_t i = 0;
        for (const auto &person : m_mapPerson)
        {
            result[i].first = person.first;
            CompareWithFeature(m_pFaceSdk, person.second.get(), pFeature.get(), result[i++].second);
        }
    }
    std::partial_sort(result.begin(), result.begin() + nTop, result.end(), [](
        const std::pair<int64_t, float> &a, const std::pair<int64_t, float> &b) -> bool
    {
        return a.second > b.second;
    });
    const size_t nResult = nTop > result.size() ? result.size() : nTop;
    for (size_t i = 0; i < nResult; ++i)
        vecMatch.push_back(std::pair<int64_t, float>(result[i].first, result[i].second));
}

int64_t LibraryHandle::GetMaxPersonId()
{
    if (!m_mapPerson.empty()) {
        return m_mapPerson.rbegin()->first;
    }
    return 0;
}

void LibraryHandle::load_library_data()
{
    Basic::FileReader ifile(m_strFileName.c_str(), Basic::FileWriter::Binary);
    if (!ifile.is_opened()) return;
    try
    {
        uint64_t num;
        uint64_t dim;
        Basic::Read(ifile, num);
        Basic::Read(ifile, dim);
        m_nFeatureSize = dim;

        for (size_t i = 0; i < num; ++i)
        {
            int64_t index;
            std::shared_ptr<float> features(new float[size_t(dim)], std::default_delete<float[]>());

            Basic::Read(ifile, index);
            Basic::Read(ifile, features.get(), size_t(dim));

            m_mapPerson.insert(std::make_pair(index, features));
        }
    }
    catch (const std::exception& e)
    {
        LOG(ERROR) << "Read file exception:" << e.what();
    }
}

void LibraryHandle::save()
{
    DeleteStorageFile();
    // save the library info
    Basic::FileWriter ofile(m_strFileName.c_str(), Basic::FileWriter::Binary);
    if (!ofile.is_opened())
    {
        LOG(ERROR) << "Open file failed:" << m_strFileName;
        return;
    }

    std::lock_guard<std::mutex> lock(m_mtPerson);
    const uint64_t num = m_mapPerson.size();
    const uint64_t dim = m_nFeatureSize;

    Basic::Write(ofile, num);
    Basic::Write(ofile, dim);
    for (auto &line : m_mapPerson)
    {
        auto &index = line.first;
        auto &features = line.second;
        // do save
        Basic::Write(ofile, index);
        Basic::Write(ofile, features.get(), size_t(dim));
    }
}
