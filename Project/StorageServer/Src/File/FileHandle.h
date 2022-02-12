/**************************************************************
*
* @brief:       file handle
* @date:        20201217
* @update:
* @auth:        Wite_Chen
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
#include <mutex>
#include <fstream>
#include <Basic/SafeMap.h>


enum FileType
{
    kFileTypeNone,
    kFileTypeJpg,
    kFileTypePng,
    kFileTypeIco,
    kFileTypeWave,
    kFileTypeZip,
    kFileTypeText

};

struct FileInfo
{
    FileType nType = kFileTypeNone;
    int64_t nDataLen = 0;
    std::string strData;
    int nSlice = 1;
    int nIndex = 0;
    FileInfo(){}
};

const int64_t kSlice = 128 * 1000;
class FileHandle
{
    const int kSecond = 1000;
public:
    FileHandle();
    FileHandle(const std::string& strPath, const std::string& strFileName, bool bRead = false);
    ~FileHandle();
    void AddFile(const FileInfo& infoFile);
    void ReadFile(FileInfo& infoFile);

 private:

private:
    std::mutex m_mtLock;
    std::fstream m_hFile;
    std::string m_strData;
    Basic::SafeMap<int, FileInfo> m_mapFile;

};