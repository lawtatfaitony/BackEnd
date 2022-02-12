#include "FileManager.h"
#include <atomic>
#include <Macro.h>
#include <Basic/File.h>
#include <Basic/Time.h>
#include <easylogging/easylogging++.h>



std::map<FileType, std::string> FileManagement::g_mapFileType
{
    { kFileTypeNone, "" },
    { kFileTypeJpg, ".jpg" },
    { kFileTypeJpg, ".png" },
    { kFileTypeJpg, ".ico" },
    { kFileTypeJpg, ".wave" },
    { kFileTypeZip, ".zip" },
    { kFileTypeText, ".txt" },
};

FileManagement::~FileManagement()
{

}

bool FileManagement::Init()
{
    m_nFile.store(kStartPos);
    // create the root directory to store the files
    if (!Basic::File::CreateSingleDirectory(kRootDir))
    {
        LOG(ERROR) << "Create root directory:" << kRootDir << " failed";
        return false;
    }
    m_objThread.Start(std::bind(&FileManagement::run, this), kCheckDate);

    return true;
}

void FileManagement::Uninit()
{
    m_objThread.Stop();
}

std::string FileManagement::SaveFile(const std::string& strData, FileType nType)
{
    FileInfo infoFile;
    infoFile.nType = nType;
    infoFile.strData = strData;
    infoFile.nDataLen = strData.length();
    infoFile.nSlice = infoFile.nDataLen / kSlice;

    std::string strFileName;
    std::string strPath = generate_path(strFileName, infoFile.nSlice + 1, infoFile.nIndex);
    // push into cache, notify to save data
    push_file(strFileName, infoFile);
    return strPath;
}

std::string FileManagement::SaveSingleFile(const FileInfo& infoFile)
{
    std::string strPath= m_strCurrDate + "/";;
    strPath += std::to_string(Basic::Time::GetMilliTimestamp());
    switch (infoFile.nType)
    {
        case kFileTypeJpg:
        case kFileTypePng:
        case kFileTypeIco:
        case kFileTypeWave:
        case kFileTypeZip:
        case kFileTypeText:
            strPath += g_mapFileType[infoFile.nType];
            break;
        default:
            break;
    }
    // save with file
    std::string strFile = kRootDir + strPath;
    std::fstream fout(strFile.c_str(), std::ios::binary | std::ios::out);
    if (fout.is_open())
    {
        fout.write((char*)infoFile.nType, sizeof(infoFile.nType));
        fout.write((char*)infoFile.nDataLen, sizeof(infoFile.nDataLen));
        fout.write(infoFile.strData.c_str(), infoFile.strData.length());
        fout.close();
    }
    return strPath;
}

void FileManagement::ReadFile(const std::string& strPath, FileInfo& infoFile)
{
    std::string strFile;
    if (parse_path(strPath, strFile, infoFile.nIndex))
    {
        auto pHandle = get_file_handle(strFile);
        if (nullptr == pHandle)
        {
            std::shared_ptr<FileHandle> pNewHandle(new FileHandle(kRootDir, strFile, true));
            m_mapHandle.Insert(m_strFile, pNewHandle);
            pNewHandle->ReadFile(infoFile);
        }
        else
            pHandle->ReadFile(infoFile);
    }
    else
    {
        // single file
        // TODO: 
    }
}

void FileManagement::run()
{
    std::string strCurrDate = Basic::Time::GetCurrentDate();
    if (strCurrDate != m_strCurrDate)
    {
        // reset the index
        m_nFile.store(kStartPos);
        m_strCurrDate = strCurrDate;
        std::string strDateDir = kRootDir + strCurrDate;
        if (!Basic::File::CreateSingleDirectory(strDateDir))
        {
            LOG(ERROR) << "Create directory:" << strDateDir << " failed";
            return;
        }
    }
}

void FileManagement::add_new_handle()
{
    m_strFile = std::to_string(time(nullptr));
    std::string strPath = m_strCurrDate + "/" + m_strFile;
    std::shared_ptr<FileHandle> pHandle(new FileHandle(kRootDir, strPath));
    m_mapHandle.Insert(strPath, pHandle);
    // reset the index
    m_nFile.store(kStartPos);
}

std::string FileManagement::generate_path(std::string& strFile, int nSlice, int& nIndex)
{
    std::string strPath = m_strCurrDate + "/";
    SCOPE_LOCK(m_mtLock);
    nIndex = m_nFile.fetch_add(nSlice);
    if (kStartPos == nIndex || nIndex > kMaxFile)
    {
        add_new_handle();
        nIndex = m_nFile.fetch_add(nSlice);
    }
    strPath += m_strFile;
    strFile = strPath;
    strPath += "/" + std::to_string(nIndex);

    return strPath;
}

std::shared_ptr<FileHandle> FileManagement::get_file_handle(const std::string& strFile)
{
    std::shared_ptr<FileHandle> pHandle(nullptr);
    m_mapHandle.GetValue(strFile, pHandle);
    return pHandle;
}

void FileManagement::push_file(const std::string& strFile, const FileInfo& infoFile)
{
    auto pHandle = get_file_handle(strFile);
    if (pHandle != nullptr)
        pHandle->AddFile(infoFile);
}

bool FileManagement::parse_path(const std::string& strPath, std::string& strFile, int& nFile)
{
    bool bSucc = false;
    int nPos = strPath.find(".");
    if (std::string::npos == nPos)
    {
        int nPos = strPath.find_last_of("/");
        if (std::string::npos == nPos) return false;
        strFile = strPath.substr(0, nPos);
        nFile = std::stoi(strPath.substr(++nPos));
        bSucc = true;
    }
    return bSucc;
}
